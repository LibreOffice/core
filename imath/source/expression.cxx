/**************************************************************************
        expression.cpp  -  Class for encapsulating the GiNaC ex class
                             -------------------
    begin                : Wed Nov 6 2002
    copyright            : (C) 2002 by Jan Rheinlaender
    email                : jrheinlaender@users.sourceforge.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <sstream>
#ifdef INSIDE_SM
#include <imath/expression.hxx>
#include <imath/unit.hxx>
#include <imath/func.hxx>
#include <imath/differential.hxx>
#include <imath/extintegral.hxx>
#include <imath/msgdriver.hxx>
#include <imath/utils.hxx>
#include <imath/printing.hxx>
#else
#include "expression.hxx"
#include "unit.hxx"
#include "func.hxx"
#include "differential.hxx"
#include "extintegral.hxx"
#include "msgdriver.hxx"
#include "utils.hxx"
#include "printing.hxx"
#endif
#include "exderivative.hxx"

namespace GiNaC {

bool expression::evalf_real_roots_flag = false;

expression::expression() : empty(true) {
  MSG_INFO(3, "Constructing empty expression" << endline);
}

expression::expression(const expression &e) : ex(e), empty(e.empty) {
  MSG_INFO(3, "Copying expression from expression " << ex(e) << endline);
}
expression::expression(expression&& e) : ex(std::move(e)), empty(e.empty) {
  MSG_INFO(3, "Moving expression from expression " << ex(*this) << endline);
}

expression::expression(const basic &other) : ex(other), empty(false) {
  // ex(other) can NOT be printed here because
  // - It was dynallocated
  // - It was eval'ed
  // - eval() created a new instance
  // - The old instance (other) was disposed of
  MSG_INFO(3, "Constructing expression from basic" << endline);
}
expression::expression(basic&& other) : ex(std::move(other)), empty(false) {
  MSG_INFO(3, "Moving expression from basic " << ex(*this) << endline);
}

expression::expression(const ex& e) : ex(e), empty(false) {
  MSG_INFO(3, "Constructing expression from ex " << e << endline);
}
expression::expression(ex&& e) : ex(std::move(e)), empty(false) {
  MSG_INFO(3, "Moving expression from ex " << ex(*this) << endline);
}

expression::expression(const std::string &s, const ex &l) : ex(s, l), empty(false) {
  MSG_INFO(3, "Constructing expression from " << s << endline);
}

#ifdef DEBUG_CONSTR_DESTR
  expression::~expression() {
    MSG_INFO(3, "Destructing expression " << ex(*this) << endline);
  }
#endif

#if (((GINACLIB_MAJOR_VERSION == 1) && (GINACLIB_MINOR_VERSION >= 7)) || (GINACLIB_MAJOR_VERSION >= 1))
expression expression::evalf() const {
  MSG_INFO(2, "expression: Applying evalf() on " << ex(*this) << endline);
  if (evalf_real_roots_flag) {
    evalf_real_roots eval_roots;
    return eval_roots(*this).evalf();
  } else {
    return ex::evalf();
  }
}
#else
expression expression::evalf(int level) const {
  MSG_INFO(2, "expression: Applying evalf() on " << ex(*this) << endline);
  if (evalf_real_roots_flag) {
    evalf_real_roots eval_roots;
    return eval_roots(*this).evalf(level);
  } else {
    return ex::evalf(level);
  }
}
#endif

expression expression::evalm() const {
  MSG_INFO(3, "expression: Applying evalm() on " << ex(*this) << endline);
  try {
    expression result(ex::evalm()); // Evaluate matrices

    if (is_a<matrix>(result)) {
      // Sanitize the matrix
      // 1. Return a scalar instead of a 1x1 matrix
      const matrix& m = ex_to<matrix>(result);
      if ((m.rows() == 1) && (m.cols() == 1))
        return m(0,0);

      // 2. Convert matrix with a single row of column vectors or a single column of row vectors to real matrix
      if (m.rows() == 1) {
        for (unsigned c = 0; c < m.cols(); ++c)
          if (!is_a<matrix>(m(0,c)) || (ex_to<matrix>(m(0,c)).cols() > 1))
            return result;

        unsigned rows = ex_to<matrix>(m(0,0)).rows();
        matrix newm(rows, m.cols());
        MSG_INFO(1, "2a. Creating new matrix with " << rows << " rows and " << m.cols() << " columns" << endline);

        for (unsigned c = 0; c < m.cols(); ++c) {
          const matrix& col = ex_to<matrix>(m(0,c));
          if (col.rows() != rows) return result; // Column vectors have different lengths
          for (unsigned r = 0; r < col.rows(); ++r)
            newm(r,c) = col(r,0);
        }

        MSG_INFO(1, "Generated new matrix " << newm << endline);
        return newm;
      } else if (m.cols() == 1) {
        for (unsigned r = 0; r < m.rows(); ++r)
          if (!is_a<matrix>(m(r,0)) || (ex_to<matrix>(m(r,0)).rows() > 1))
            return result;

        unsigned cols = ex_to<matrix>(m(0,0)).cols();
        matrix newm(m.rows(), cols);
        MSG_INFO(1, "2b. Creating new matrix with " << m.rows() << " rows and " << cols << " columns" << endline);

        for (unsigned r = 0; r < m.rows(); ++r) {
          const matrix& row = ex_to<matrix>(m(r,0));
          if (row.cols() != cols) return result; // Row vectors have different lengths
          for (unsigned c = 0; c < row.cols(); ++c)
            newm(r,c) = row(0,c);
        }

        MSG_INFO(1, "Generated new matrix " << newm << endline);
        return newm;
      }
    }

    return result;
  } catch (std::exception& e) {
    (void)e;
    // Do nothing if matrices are not compatible etc.
    return *this;
  }
}

expression expression::evalu() const {
  MSG_INFO(2, "expression: Applying evalu() on " << ex(*this) << endline);
  expand_real_powers expand_powers;
  reduce_double_powers reduce_powers;
  func::reduce_double_funcs reduce_funcs;
  return (reduce_powers(expand_powers(reduce_funcs(*this))));
}

expression expression::ignore_units() const {
  MSG_INFO(2, "expression: Removing units from certain functions in " << ex(*this) << endline);
  special_ignore_units ignore_u;
  return ignore_u(*this);
}

expression expression::expand(const unsigned options) const {
  MSG_INFO(3, "Expanding " << ex(*this) << endline);
  expand_real_powers expand_powers;
  return (expand_powers(ex::expand(options)));
}

expression expression::normal() const {
  MSG_INFO(3, "Normal " << ex(*this) << endline);
  return ex::normal();
}

// Note: All subs() methods must call expression::subs(... exmap ...) because of enhanced substitution that takes place there
expression expression::subs(const expression& e, unsigned options) const {
  MSG_INFO(2, "Substituting " << ex(e) << " in " << ex(*this) << endline);
  exmap m;

  // Note: this is largely copied from ex::subs()
  if (e.info(info_flags::relation_equal)) {
    const ex & s = e.op(0);
    m.insert(std::make_pair(s, e.op(1)));
    if (is_exactly_a<mul>(s) || is_exactly_a<power>(s))
      options |= subs_options::pattern_is_product;
    else
      options |= subs_options::pattern_is_not_product;
  } else if (e.info(info_flags::list)) {
    for (auto & r : ex_to<lst>(e)) {
      if (!r.info(info_flags::relation_equal))
        throw std::invalid_argument("The expression to substitute must be a relational or a list of relationals.");
      const ex & s = r.op(0);
      m.insert(std::make_pair(s, r.op(1)));
      if (is_exactly_a<mul>(s) || is_exactly_a<power>(s))
        options |= subs_options::pattern_is_product;
    }
    if (!(options & subs_options::pattern_is_product))
      options |= subs_options::pattern_is_not_product;
  } else {
     throw std::invalid_argument("The expression to substitute must be a relational or a list of relationals.");
  }
  return subs(m, options);
}

expression expression::subs(const exmap &m, unsigned options) const {
  MSG_INFO(2, "Substituting exmap " << m << " in " << ex(*this) << endline);
  ex result(ex::subs(m, options | subs_options::algebraic));
  MSG_INFO(2, "First result: " << result << endline);

  if (result.return_type() == return_types::commutative) {
    // Try harder because of deficiencies in subs_options::algebraic
    // Note: If the expression is non-commutative (contains matrices), the '+ wild(j)' will throw an exception about incompatible types
    exmap ltemp;
    unsigned j = 1000; // Avoid collision with user-defined wild card!
    for (const auto& i : m) {
      if (is_a<add>(i.first)) {
        // Handle subs(x + y + z, x + y == a)
        ltemp.emplace(i.first + wild(j), i.second + wild(j));
      } else if (is_a<mul>(i.first)) {
        // Handle subs(sin(x)/cos(x), sin(x)/cos(x) == tan(x))
        ltemp.emplace(i.first * wild(j), i.second * wild(j));

        if (!i.first.op(i.first.nops()).is_equal(_ex1)) {
          // Handle subs(-2x, 2x == g)
          j++;
          ltemp.emplace(_ex_1 * i.first * wild(j), _ex_1 * i.second * wild(j));
        }
      }
      j++;
    }
    if (!ltemp.empty()) {
      MSG_INFO(3, "Substituting exmap " << ltemp << " in " << result << endline);
      result = result.subs(ltemp, options | subs_options::algebraic);
      MSG_INFO(3, "Second result: " << result << endline);
    }
  }

  return expression(result);
}

expression expression::subsv(const expression& e, const bool consecutive, unsigned options) const {
  // The expression must have form {var1 = vector1; var2 = vector2; ... }
  unsigned rows = 0;
  std::vector<exmap> substitutions; // will have form { {var1 = vector1[0]; var2 = vector2[0]; ... }; {var1 = vector1[1]; var2 = vector2[1]; ... }; ... }

  for (const auto& eq : e) {
    if (!eq.info(info_flags::relation_equal))
      throw std::invalid_argument("The expression to substitute must be a relational or a list of relationals.");
    if (!is_a<matrix>(eq.op(1)))
      throw std::runtime_error("The expression to substitute must have a column vector on the right-hand side");

    const matrix& m = ex_to<matrix>(eq.op(1));
    if (m.cols() > 1)
      throw std::runtime_error("The expression to substitute must have a column vector on the right-hand side");

    if (rows == 0) {
      // First equation in the list
      rows = m.rows();
      for (unsigned i = 0; i < rows; ++i)
#if (((GINACLIB_MAJOR_VERSION == 1) && (GINACLIB_MINOR_VERSION >= 7)) || (GINACLIB_MAJOR_VERSION >= 1))
        substitutions.push_back(exmap{{eq.op(0), m[i]}}); // { {var1 = vector1[0]}; {var1 = vector1[1]}; ... }
#else
        exmap mm;
        mm[eq.op(0)] = m[i];
        substitutions.push_back(mm);
#endif
    } else {
      if (m.rows() != rows)
        throw std::runtime_error("All expressions to substitute must have the same number of vector elements");
      unsigned i = 0;
      for (std::vector<exmap>::iterator s = substitutions.begin(); s != substitutions.end(); ++s) {
        s->insert(std::make_pair(eq.op(0), m[i]));
        ++i;
      }
    }
  }

  matrix result(rows, 1);
  unsigned i = 0;
  for (const auto& s : substitutions) {
    if (consecutive)
      result(i, 0) = csubs(s, options);
    else
      result(i, 0) = subs(s, options);
    ++i;
  }

  return result;
}

expression expression::csubs(const exmap &m, unsigned options) const {
  MSG_INFO(1, "Substituting consecutively " << m << " in " << ex(*this) << endline);
  if (m.size() == 1) return subs(m, options);

  expression result = *this;

  for (const auto& i : m) {
    exmap mm;
    const ex & s = i.first;
    mm.insert(std::make_pair(s, i.second));
    if (is_exactly_a<mul>(s) || is_exactly_a<power>(s))
      options |= subs_options::pattern_is_product;
    else
      options |= subs_options::pattern_is_not_product;

    result = result.subs(mm, options);
    MSG_INFO(1, "Substituted " << mm << ", result: " << result << endline);
  }

  return result;
}

expression expression::csubs(const unitvec &v, unsigned options) const {
  MSG_INFO(1, "Substituting consecutively unitvec in " << ex(*this) << endline);
  expression result = *this;

  for (const auto& i : v) {
    exmap m;
    const ex & s = i.op(0);
    m.insert(std::make_pair(s, i.op(1)));
    if (is_exactly_a<mul>(s) || is_exactly_a<power>(s))
      options |= subs_options::pattern_is_product;
    else
      options |= subs_options::pattern_is_not_product;

    result = result.subs(m, options);
    MSG_INFO(1, "Substituted " << m << ", result: " << ex(result) << endline);
  }
  return result;
}

expression expression::csubs(const ex &e, unsigned options) const {
  MSG_INFO(1, "Substituting consecutively in " << ex(*this) << ":" << endline);

  if (e.info(info_flags::relation_equal)) {
    return subs(e);
  } else if (e.info(info_flags::list)) {
    expression result = *this;
    for (const auto& i : ex_to<lst>(e)) {
      if (!i.info(info_flags::relation_equal))
        throw std::invalid_argument("The expression to substitute must be a relational or a list of relationals.");
      exmap m;
      const ex & s = i.op(0);
      m.insert(std::make_pair(s, i.op(1)));
      if (is_exactly_a<mul>(s) || is_exactly_a<power>(s))
        options |= subs_options::pattern_is_product;
      else
        options |= subs_options::pattern_is_not_product;

      result = result.subs(m, options);
      MSG_INFO(1, "Substituted " << m << ", result: " << ex(result) << endline);
    }
    return result;
  } else
    throw std::invalid_argument("Argument to csubs() must be a relational or a list of relationals");
}

// Helper functions for simplification cancel-diff
/// Dissolve exderivative objects into single differentials
struct expand_exderivatives : public map_function {
  ex operator()(const ex& e);
};
ex expand_exderivatives::operator()(const ex& e) {
  MSG_INFO(3, "expand_exderivatives() for " << e << endline);
  if (is_a<exderivative>(e))
    return ex_to<exderivative>(e).dissolve().map(*this);

  return e.map(*this);
}

/// Ignore numerator / denominator flag in differentials so that they can be cancelled against one another
struct reduce_differentials : public map_function {
  ex operator()(const ex& e);
};
ex reduce_differentials::operator()(const ex& e) {
  MSG_INFO(3, "reduce_differentials() for " << e << endline);
  if (is_a<differential>(e)) {
    differential d = ex_to<differential>(e);
    d.set_numerator(false);
    return ex(d).map(*this);
  }

  return e.map(*this);
}

expression expression::simplify(const std::vector<std::string> &s) const {
    expression result(*this);

    for (const auto& i : s) {
      if (i == "expand") { // Full expansion
        MSG_INFO(1, "Full expanding " << result << endline);
        result = result.expand(expand_options::expand_function_args);
      } else if (i == "expandf") { // Do not expand function args
        MSG_INFO(1, "Expanding " << result << endline);
        result = result.expand();
      } else if (i == "eval") { // Evaluation
        MSG_INFO(1, "Evaluating " << result << endline);
        result = result.evalf();
      } else if (i == "normal") { // Normalization
        MSG_INFO(1, "Normalizing " << result << endline);
        result = result.normal();
      } else if (i == "collect-common") { // Collecting common factors
        MSG_INFO(1, "Collecting common factors in " << result << endline);
        result = result.collect_common_factors();
      } else if (i == "unsafe") { // Unsafe simplifications
        MSG_INFO(1, "Doing unsafe simplifications in " << result << endline);
        result = result.evalu();
      } else if (i == "diff") { // Evalute differential objects
        // Note: Unevaluated objects may occur after substitution, e.g. of a symbolic grade with a numeric grade
        result = result.eval_differential();
      } else if (i == "cancel-diff") {
        expand_exderivatives expand_devs;
        reduce_differentials reduce_diffs;
        match_differentials match_diffs;
        result = match_diffs(reduce_diffs(expand_devs(result)));
      } else if (i == "sum") { // Evalute \sum function objects
        func::expand_sum expand_s;
        result = expand_s(result);
      } else if (i == "gather-sqrt") {
        MSG_INFO(1, "Gathering square roots in " << result << endline);
        gather_sqrt gather_sqrts;
        result = gather_sqrts(result);
      } else if (i == "integrate") {
        MSG_INFO(1, "Symbolically integrating " << result << endline);
        result = result.eval_integral();
      } else if (i == "ignore-units") {
        MSG_INFO(1, "Ignoring units in certain functions " << result << endline);
        result = result.ignore_units();
      } else { // Nothing of the above fitted
        MSG_WARN(0, "Warning: Unknown simplification of type " << i << endline);
      }
    }

    return result;
  }

expression expression::collect_common_factors() const {
  ex result = GiNaC::collect_common_factors(*this);

  if (result.is_equal(*this))
    // Try harder
    result = collect();

  return result;
}

expression expression::collect(const expression& e) const {
    MSG_INFO(2, "Collecting expression " << ex(*this) << " to symbol " << e << endline);
    ex sym = e;
    if (e.is_zero()) {
      // Try to guess variable
      if (!check_polynomial(*this, sym, false))
        return *this;
    }

    return expression(ex(*this).collect(sym));
}

expression expression::diff(const expression &var, const expression& nth, bool toplevel) const {
  MSG_INFO(1, "Calculating " << nth << "-th derivative of " << ex(*this) << " to " << var << endline);

  // No differentiation possible
  if (!nth.info(info_flags::nonnegint)) {
    // TODO: How to determine when to use partial?
    if (toplevel)
      return expression(dynallocate<differential>(*this, false, nth, _ex0, true)) / expression(dynallocate<differential>(var, false, nth, *this));
    else
      return dynallocate<exderivative>(dynallocate<differential>(*this, false, nth),
                                       dynallocate<differential>(var, false, nth, *this));
  }

  // Handle trivial cases
  int i_nth = numeric_to_int(ex_to<numeric>(nth));
  if (i_nth == 0) return *this;
  if (are_ex_trivially_equal(*this, var)) {
    if (i_nth == 1)
      return _ex1;
    else
      return _ex0;
  }

  expression result;

  // Handle differentiation to a function
  if (is_a<symbol>(var)) {
    if (toplevel) {
      match_differentials match_diffs;
      result = match_diffs(*this).diff(ex_to<symbol>(var), i_nth);
    } else {
      result = ex::diff(ex_to<symbol>(var), i_nth);
    }
  } else if (is_a<func>(var)) {
    const func& v = ex_to<func>(var);
    symbol diffsym(v.get_name()); // Create a temporary symbol
    MSG_INFO(2, "Differentiating to function " << v << endline);
    if (toplevel) {
      match_differentials match_diffs;
      result = match_diffs(this->subs(var == diffsym)).diff(diffsym, i_nth);
    } else {
      result = ex(this->subs(var == diffsym)).diff(diffsym, i_nth);
    }
    MSG_INFO(2, "Intermediate result " << ex(result) << endline);
    result = result.subs(diffsym == var);
  } else {
    throw std::logic_error("Can only differentiate with respect to a variable or a function!");
  }

  MSG_INFO(2, "Result of differentiation: " << result << endline);

  // Differentiating powers might have introduced the GiNaC log() function
  func::replace_function_by_func replace_functions;
  return replace_functions(result);
}

/// Map function for use in partial differentiation
// Functions that do not have an explicit argument which is identical to the differentiation variable are replaced by symbols
// e.g. diffvar is x
// f(x) -> f(x)
// f(y) -> symbol
// f() -> f() if f() is a function of x
// f() -> symbol if f() is not a function of x
// f(a x + b) -> f(a x + b)
struct replace_pure_funcs_by_symbols : public map_function {
  ex diffvar;
  bool toplevel;
  exmap repl;

  replace_pure_funcs_by_symbols(const ex& dv, const bool tl) : diffvar(dv), toplevel(tl) {}
  ex operator()(const ex &e);
  exmap get_replacements() { return repl; }
};

ex replace_pure_funcs_by_symbols::operator()(const ex &e) {
  if (!toplevel && is_a<func>(e) && ex_to<func>(e).is_pure() && (!e.is_equal(diffvar))) {
    // TODO: Isn't it more correct to check whether the function has the diffvar as an argument, even if it is pure?
    // Currently we define our behaviour as a feature
    MSG_INFO(2, "Replacing pure func " << e << " with symbol" << endline);
    ex r = dynallocate<symbol>();
    repl.emplace(r, e);
    toplevel = false;
    return r;
  }

  toplevel = false;
  return e.map(*this);
}

expression expression::pdiff(const expression &var, const expression& nth, bool toplevel) const {
  MSG_INFO(1, "Calculating " << nth << "-th partial derivative of " << ex(*this) << " to " << var << endline);
  exmap repl;
  replace_pure_funcs_by_symbols repl_funcs(var, is_a<func>(*this) && ex_to<func>(*this).is_pure());
  expression result = repl_funcs(*this);
  return expression(result).diff(var, nth, toplevel).subs(repl_funcs.get_replacements());
}

struct eval_differentials : public map_function {
  ex operator()(const ex &e);
};

ex eval_differentials::operator()(const ex& e) {
  MSG_INFO(3, "eval_differentials() for " << e << endline);
  if (is_a<exderivative>(e)) {
    ex mapped_e = e.map(*this); // First map subexpressions
    return ex_to<exderivative>(mapped_e).eval_diff();
  } else {
    // TODO: Also evaluate differential objects?
    return e.map(*this);
  }
}

expression expression::eval_differential() const {
  match_differentials match_diffs;
  eval_differentials eval_diffs;
  return eval_diffs(match_diffs(*this));
}

expression expression::eval_integral() {
  MSG_INFO(1, "Evaluating integrals for " << ex(*this) << endline);
  if (is_a<add>(*this)) {
    expression result(_expr0);
    for (const auto& i : *this)
      result = result + expression(i).eval_integral();
    return result;
  }

  expression integrals(_expr1);
  expression rest(_expr1);

  MSG_INFO(2, "Examining " << ex(*this) << endline);

  // Find the integral(s)
  for (const auto& i : (is_a<mul>(*this) ? *this : lst{*this})) {
    if (is_a<extintegral>(i))
      integrals = integrals * expression(i);
    else if (is_a<add>(i))
      rest = rest * expression(i).eval_integral();
    else
      rest = rest * expression(i);
  }

  MSG_INFO(2, "Integrals: " << integrals << endline);
  MSG_INFO(2, "Rest: " << rest << endline);
  if (integrals.is_equal(_expr1)) return *this; // no integrals found

  // Evaluate the integrals
  expression result(_expr1);
  for (const auto& i : (is_a<mul>(integrals) ? integrals : lst{integrals}))
    result = result * expression(ex_to<extintegral>(i).eval_integ());

  return rest * result;
}

expression expression::integrate(const ex& var, const symbol& integration_constant) const {
  extintegral result(var, *this, integration_constant);
  return result.eval_integ();
}

expression expression::integrate(const ex& var, const ex& lowerbound, const ex& upperbound) const {
  MSG_INFO(1, "expression::integrate() with bounds for " << ex(*this) << endline);
  return extintegral(var, lowerbound, upperbound, *this).eval_integ();
}

expression& expression::operator=(const symbol &s) {
  MSG_INFO(3, "Assigning expression from symbol " << s << endline);
  ex::operator=(s);
  empty = false;
  return *this;
}

expression& expression::operator=(const expression &e) {
  MSG_INFO(3, "Assigning expression from expression " << ex(e) << endline);
  ex::operator=(e);
  empty = e.empty;
  return *this;
}

void expression::print(const print_context &c, unsigned level) const {
  MSG_INFO(4, "expression::print()" << endline);
  if (is_a<imathprint>(c)) {
    if (is_empty()) {
      c.s << "[empty expression]";
    } else {
      ex::print(c, level);
    }
  } else {
    if (is_empty()) {
      c.s << "-EMPTY-";
    } else {
      ex::print(c, level);
    }
  }
} // expression::print()

}
