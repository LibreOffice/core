/***************************************************************************
                          expression.hxx  -  Header file for class expression
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

#ifndef EXPRESSION_H
#define EXPRESSION_H

#include <string>
#include <iostream>
#include <stdexcept>
#pragma warning(push)
#pragma warning (disable: 4099 4100 4996)
#include <ginac/ginac.h>
#pragma warning(pop)

namespace GiNaC {

typedef std::vector<relational> unitvec;

class expression;
extern const expression _expr0;
extern const expression _expr1;

/**
  * @short Subclass of GiNaC::ex to enable special features for expressions
  * @author Jan Rheinlaender
  */
class expression : public ex {
private:
  /// Indicates whether a value has been stored in the expression
  bool empty;
public:
  // Return the real nth root as the result of evalf(), whereas ex::evalf() returns
  // the root on the principal branch, e.g. (-1)^(1/3) = 0.5+0.86602540378443864673*I
  static bool evalf_real_roots_flag;

public:
  /// Constructs an empty expression
  expression();
  /// Constructs an expression from another expression
  expression(const expression &e);
  expression(expression&& e);
  /// Constructs an expression from a GiNaC basic object
  expression(const basic &other);
  expression(basic&& other);
  /// Constructs an expression from a GiNaC ex object
  expression(const ex& e);
  expression(ex&& e);
  /// Constructs an expression from a string and a list of symbols
  expression(const std::string &s, const ex &l);

#ifdef DEBUG_CONSTR_DESTR
  ~expression();
#endif

  /// Make the expression be an empty expression
  inline void clear() { empty = true; }

  /// Return true if the expression is empty
  inline bool is_empty() const { return empty; }

  /**
  * Prints the expression on a print context. For print_latex contexts,
  * the GiNaC expression is printed!
  * @param c The GiNaC print context to print on
  * @param level The parenthesis level (required by GiNaC, not used)
  */
  void print(const print_context &c, unsigned level = 0) const;

  /// Evaluate the expression. The difference to ex::evalf() is that we take positive real nth roots instead of roots on the principal branch
#if (((GINACLIB_MAJOR_VERSION == 1) && (GINACLIB_MINOR_VERSION >= 7)) || (GINACLIB_MAJOR_VERSION >= 1))
  expression evalf() const;
#else
  expression evalf(int level = 0) const;
#endif
  /// Evaluate matrices in the expression
  expression evalm() const;
  /**
   * Evaluate the expression, doing unsafe simplifications
   * @returns The evaluated and simplified expression
   */
  expression evalu() const;

  /**
   * Remove units from certain functions, e.g. logarithm
   * This makes evaluating scientific equations easier
   * @returns The simplified expression
   **/
  expression ignore_units() const;

  /**
   * Expand the expression. Over above GiNaC's expansion, this also expands real powers,
   * for example, sqrt(x * y) is expanded to sqrt(x) * sqrt(y)
   * @param options The options for the expansion
   * @returns An expression containing the result of the expansion
   */
  expression expand(unsigned options = 0) const;

  expression normal() const;

  /**
   *  Substitute an expression or a list of expressions inside the object
   * @param e The expression(s) to substitute
   * @param options A flag of options
   * @returns The result of the substitution
   */
  expression subs(const expression &e, unsigned options = 0) const;
  /// Substitute a map of expressions in the object
  expression subs(const exmap &m, unsigned options = 0) const;
  /**
   * Substitute an expression or a list of expressions inside the object, returning
   * a vector if the RHS of the expression being substituted is a vector
   * @param e The expression(s) to substitute
   * @param options A flag of options
   * @returns The result of the substitution
   */
  expression subsv(const expression& e, const bool consecutive = false, unsigned options = 0) const;

  /**
  Same as subs, but the substitutions are done consecutively.
  @param m A GiNaC exmap of substitutions
  @param options A flag of options
  @returns An expression with the substitutions done
  **/
  expression csubs(const exmap &m, unsigned options = 0) const;
  /// Substitute a list of equations consecutively in the equation
  expression csubs(const unitvec &v, unsigned options = 0) const;
  /// Substitute an expression or a list of expressions inside the object
  expression csubs(const ex &e, unsigned options = 0) const;

  /**
  Simplifies the expression according to the list of simplifications given
  @param s A vector of simplifications that are to be performed
  @returns The simplified expression
  **/
  expression simplify(const std::vector<std::string> &s) const;

  /// Try to collect common factors in this expression
  expression collect_common_factors() const;

  /**
  Orders the expression as a polynomial in the given expression
  @param e An expression that is to be the base of the polynomial
  @returns The modified expression
  **/
  expression collect(const expression& e = _expr0) const;

  /// Evaluate all integral objects in the expression
  expression eval_integral();

  /**
  Differentiates the expression with respect to the dependant variable given
  @param var The dependant variable
  @param nth Calculates the nth derivative
  @param toplevel If this is true, then differential objects are matched into exderivative objects
         before differentiation, and afterwards the exderivatives are dissolved again into single differentials
  @returns An expression containing the result of the differentation
  @exception logic_error(Can only differentiate with respect to a symbol or a function)
  **/
  expression diff(const expression &var, const expression& nth = _expr1, bool toplevel = false) const;

  /**
  Differentiates partially the expression with respect to the dependant variable given.
  All other symbols and functions are regarded as constant.
  @param var The dependant variable as a GiNaC::symbol or a function
  @param nth Calculates the nth derivative
  @param toplevel If this is true, then differential objects are matched into exderivative objects
         before differentiation, and afterwards the exderivatives are dissolved again into single differentials
  @returns An equation containing the result of the differentation
  @exception logic_error(Can only differentiate with respect to a symbol or a function)
  **/
  expression pdiff(const expression &var, const expression& nth = _expr1, bool toplevel = false) const;

  /// Evaluate all differential objects in the expression
  // May be necessary in two cases
  // 1. Substitution of a symbolic diff grade with a numeric grade
  // 2. Composure of exderivative objects manually with differential objects (should be avoided wherever possible)
  expression eval_differential() const;

  /**
  Integrates the expression with respect to the dependant variable given. If
  integration is not possible an extintegral object is returned.
  @param var The dependant variable as a symbol or function
  @param integration_constant The symbol to use as integration constant
  @returns An expression containing the result of the integration
  @exception logic_error(Can only integrate with respect to a symbol or a function)
  **/
  expression integrate(const ex &var, const symbol& integration_constant) const;
  expression integrate(const ex &var, const ex& lowerbound, const ex& upperbound) const;

  /// Assign a symbol to the expression
  expression& operator=(const symbol &s);
  /// Assign an expression to the expression
  expression& operator=(const expression &e);
}; // class expression

typedef std::vector<expression> exprvector;

}
#endif


