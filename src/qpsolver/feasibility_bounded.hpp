#ifndef __SRC_LIB_FEASIBILITYBOUNDED_HPP__
#define __SRC_LIB_FEASIBILITYBOUNDED_HPP__

#include "Highs.h"
#include "qpsolver/a_asm.hpp"
#include "qpsolver/crashsolution.hpp"

static void computestartingpoint_bounded(Instance& instance, Settings& settings, Statistics& stats, QpModelStatus& modelstatus, QpHotstartInformation& result, HighsTimer& timer) {
  // compute initial feasible point for problems with bounds only (no general linear constraints)

  // compute  Qx + c = 0 --> x = Q^-1c
  std::vector<double> L;
  L.resize(instance.num_var * instance.num_var);

  // compute cholesky factorization of Q
  for (size_t col = 0; col < instance.num_var; col++) {
    for (size_t idx = instance.Q.mat.start[col]; idx < instance.Q.mat.start[col+1]; idx++) {
      double sum = 0;
      size_t row = instance.Q.mat.index[idx]; 
      if (row == col) {
        for (size_t k = 0; k < row; k++)
          sum += L[k * instance.num_var + row] * L[k * instance.num_var + row];
        L[row * instance.num_var + row] = sqrt(instance.Q.mat.value[idx] - sum);
      } else {
        for (size_t k = 0; k < row; k++)
          sum += (L[k * instance.num_var + col] * L[k * instance.num_var + row]);
        L[row * instance.num_var + col] =
            (instance.Q.mat.value[idx] - sum) / L[row * instance.num_var + row];
      }
    }
  }

  // solve for c
  Vector res = instance.c;
  for (HighsInt r = 0; r <res.dim; r++) {
    for (HighsInt j = 0; j < r; j++) {
      res.value[r] -= res.value[j] * L[j * instance.num_var + r];
    }
    res.value[r] /= L[r * instance.num_var + r];
  }

  for (HighsInt i = res.dim - 1; i >= 0; i--) {
    double sum = 0.0;
    for (HighsInt j = res.dim - 1; j > i; j--) {
      sum += res.value[j] * L[i * instance.num_var + j];
    }
    res.value[i] = (res.value[i] - sum) / L[i * instance.num_var + i];
  }

  // project solution to bounds and collect active bounds
  Vector x0(instance.num_var);
  Vector ra(instance.num_con);
  std::vector<HighsInt> initialactive;
  std::vector<HighsInt> initialinactive;
  std::vector<BasisStatus> atlower;

  for (int i=0; i<instance.num_var; i++) {
    if (res.value[i] <= instance.var_lo[i]) {
        res.value[i] = instance.var_lo[i];
        initialactive.push_back(i + instance.num_con);
        atlower.push_back(BasisStatus::ActiveAtLower);
    } else if (res.value[i] >= instance.var_up[i]) {
        res.value[i] = instance.var_up[i];
        initialactive.push_back(i + instance.num_con);
        atlower.push_back(BasisStatus::ActiveAtUpper);
    } else {
        initialinactive.push_back(i + instance.num_con);
    }
    if (fabs(res.value[i]) > 10E-5) {
      x0.value[i] = res.value[i];
      x0.index[x0.num_nz++] = i;
    }
  }

  // if no bounds are active, solution lies in the interior -> optimal
  if (initialactive.size() == 0) {
    modelstatus = QpModelStatus::OPTIMAL;
  }

  assert((HighsInt)(initialactive.size() + initialinactive.size()) ==
         instance.num_var);

  result.status = atlower;
  result.active = initialactive;
  result.inactive = initialinactive;
  result.primal = x0;
  result.rowact = ra;
}

#endif
