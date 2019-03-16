#ifndef MIP_SOLVEMIP_H_
#define MIP_SOLVEMIP_H_

#include <memory>
#include <stack>

#include "lp_data/HighsLp.h"
#include "lp_data/HighsStatus.h"

struct Node {
  int id;
  int parent_id;
  int level;

  Node(int parent, int index, int depth) :
    id(index), parent_id(parent), level(depth) {}

  std::vector<int> integer_variables;
  std::vector<double> primal_solution;

  // Minimal information about changes. Just bounds for the moment.
  std::vector<double> cow_lower_bound;
  std::vector<double> col_upper_bound;

  std::unique_ptr<Node> left_child;
  std::unique_ptr<Node> right_child;
};

using NodeIndex = int;
constexpr NodeIndex kNoNodeIndex = -1;
constexpr NodeIndex kNodeIndexError = -2;


class NodeStack {
public:
  NodeIndex chooseBranchingVariable(const Node& node,
                                    const std::vector<int>& integer_variables);

private:
  std::vector<int> integer_variables_;
  std::vector<double> best_solution_;

  std::stack<Node> stack_;
};

#endif