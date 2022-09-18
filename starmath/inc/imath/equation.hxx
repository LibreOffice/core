/**************************************************************************
                          equation.hxx  -  header file for class equation
                             -------------------
    begin                : Sun Oct 21 2001
    copyright            : (C) 2001 by Jan Rheinlaender
    email                : jrheinlaender@users.sourceforge.net
 ***************************************************************************/

/**************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

/**
@short Used to store, print and evaluate equations
@author Jan Rheinlaender

Class equation is used to store, print and evaluate equations
**/

#ifndef EQUATION_H
#define EQUATION_H

#include <iostream>
#include <string>
#include <stdexcept>
#include <ginac/ginac.h>
#include "expression.hxx"
#include "option.hxx"
#include "printing.hxx"
#include "utils.hxx"

namespace GiNaC {

extern const expression _expr1;

/// Used to store, print and evaluate equations
class equation : public relational {
  GINAC_DECLARE_REGISTERED_CLASS(equation, relational)

  /// The modulus for modular arithmetic. A value of zero indicates no modulus.
  expression mod;

public:
  /**
  Constructs an equation from two expressions representing the left and the right hand side.
  @param le The left hand side of the equation
  @param ri The right hand side of the equation
  @param op The operator of the equation, e.g. equals, less
  @param m  The modulus, zero if no modulus
  **/
  equation(const expression &le, const expression &ri, const operators op = equal, const expression& m = _expr0);

  /**
  Constructs an equation from another equation.
  @param eq The equation that is used to construct the new object
  **/
  equation(const equation &eq);

  /**
  Constructs an equation from a relational.
  @param eq The equation that is used to construct the new object
  **/
  equation(const relational &r, const expression& m = _expr0);

  /// Assigns an equation to another equation
  equation &operator=(const equation &other);

#ifdef DEBUG_CONSTR_DESTR
  ~equation();
#endif

  /// Return type. Returns non_commutative if either side is non-commutative
  // Note that this is different from the behaviour of relational, which asserts that both sides
  // have the same return_type. But an equation could have a scalar product of two vectors on one side
  // (non-commutative) which after evaluation results in a scalar (commutative)
  unsigned return_type() const override;

  /**
  Return the operator sign of the equation
  @returns The operator of the equation
  **/
  inline const operators& getop() const { return o; }

  /// Return the modulus
  inline const expression& getmod() const { return mod; }

  /**
   Set the operator sign of the equation
   @param op An operator type, e.g. relational::equal
   @returns The equation with the changed operator sign
  **/
  inline equation &setop(const operators op) { o = op; return *this; }
  // *** added in 0.8, changed in 0.9

  /**
  Checks whether the equation is an assignment. This is considered to be the case when the left hand side is a
  symbol and the right hand side is a quantity.
  @returns A boolean indicating whether the equation is an assignment.
  **/
  bool is_assignment() const;

  /**
  Prints the equation on a GiNac print context. The output format depends on the print context type
  @param c The print context
  @param level Required for compatibility with GiNaC
  **/
  void do_print(const print_context &c, unsigned level = 0) const;

  /// Print the equation on a print context in iMath format
  void do_print_imath(const imathprint &c, unsigned level = 0) const;

  /**
  Simplifies both sides of the equation according to the list of simplifications given
  @param s A vector of simplifications that are to be performed
  @returns The simplified equation
  **/
  expression simplify(const std::vector<std::string> &s) const;

  /**
  Orders both sides as a polynomial in the given expression
  @param e An expression that is to be the base of the polynomial
  @returns The modified equation
  **/
  expression collect(const expression &e = _expr0) const;

  /**
  Solve the equation for the given variable
  @param e The variable to solve for
  @param n The number of the solution to return (in case there is more than one solution)
  @returns An expression with the resulting equation
  @exception invalid_argument(Argument must be a symbol)
  **/
  expression solve(const expression &e, const expression &n = _expr1) const;

  /**
  Substitute a list of equations in the equation.
  Note that GiNaC does not do intelligent substitution yet, i.e. in x+y+z it is not possible to
  substitute x+y==4 because GiNaC only substitutes objects, in this case the whole "add" object.
  @param e An expression containing a list of substitutions
  @param options A flag of options
  @returns An expression with the substitutions done
  @exception invalid_argument(Argument must be a relational or a list of relationals)
  **/
  ex subs(const ex & e, unsigned options = 0) const;
  /// Substitute a map of expressions in the object
  ex subs(const exmap &m, unsigned options = 0) const override;
  /**
   * Substitute an equation or a list of equations inside the object, returning
   * a vector if the RHS of the expression being substituted is a vector
   * @param e The equation(s) to substitute
   * @param options A flag of options
   * @returns The result of the substitution
   */
  expression subsv(const expression &e, unsigned options = 0) const;

  /**
  Same as subs, but the substitutions are done consecutively.
  @param m A GiNaC exmap of substitutions
  @param options A flag of options
  @returns An equation with the substitutions done
  **/
  expression csubs(const exmap &m, unsigned options = 0) const;
  /// Substitute a list of equations consecutively in the equation
  expression csubs(const ex &e, unsigned options = 0) const;
  /// Substitute consecutively and return a vector
  expression csubsv(const expression &e, unsigned options = 0) const;

  /// Expand both sides of the equation
  ex expand(unsigned options = 0) const override;

  /// Apply the modulus to both sides of the equation (if it is numeric and not zero)
#if (((GINACLIB_MAJOR_VERSION == 1) && (GINACLIB_MINOR_VERSION >= 7)) || (GINACLIB_MAJOR_VERSION >= 1))
  ex eval() const override;
#else
  ex eval(int level=0) const;
#endif

  /// Evaluate both sides of the equation
#if (((GINACLIB_MAJOR_VERSION == 1) && (GINACLIB_MINOR_VERSION >= 7)) || (GINACLIB_MAJOR_VERSION >= 1))
  ex evalf() const override;
#else
  ex evalf(int level=0) const;
#endif

  /// Evaluate matrices on both sides of the equation
  ex evalm() const override;

  /// Add an expression or equation to the equation
  equation &eqadd(const expression &add);

  /// Multiply the equation with an expression
  equation &eqmul(const expression &mul);

  /**
  Applies a function to both sides of the equation. Beware: The user must handle any
  mathematical problems resulting from this, for example, if the function is sqrt().
  @param e An expression containing the function
  @returns An equation containing the result of the operation
  **/
  expression apply_func(const expression &e) const;
  expression apply_func(const std::string& fname) const;

  /// Raises both sides to the power of e
  expression apply_power(const expression &e) const;

  /**
  Differentiates both sides of the equation with respect to the dependant variable given
  @param var The dependant variable as a GiNaC::symbol or (pure) function
  @param nth Calculates the nth derivative
  @param toplevel If this is true, then differential objects are matched into exderivative objects
          before differentiation, and afterwards the exderivatives are dissolved again into single differentials
  @returns An equation containing the result of the differentation
  @exception logic_error(Can only differentiate with respect to a symbol or pure function)
  **/
  expression diff(const ex &var, const expression& nth = _expr1, const bool toplevel = false) const;

  /**
  Differentiates partially both sides of the equation with respect to the dependant variable given.
  All other symbols and functions are considered constant.
  @param var The dependant variable as a GiNaC::symbol or (pure) function
  @param nth Calculates the nth derivative
  @param toplevel If this is true, then differential objects are matched into exderivative objects
         before differentiation, and afterwards the exderivatives are dissolved again into single differentials
  @returns An equation containing the result of the differentation
  @exception logic_error(Can only differentiate with respect to a symbol or pure function)
  **/
  expression pdiff(const ex &var, const expression& nth = _expr1, const bool toplevel = false) const;

  /**
  Integrates both sides of the equation with respect to the dependant variable given.
  The integral is not evaluated at all. Use simplify for that
  @param var The dependant variable as a symbol or function
  @param integration_constant The symbol to use as integration constant
  @returns An equation containing the result of the integration
  @exception logic_error(Can only integrate with respect to a symbol)
  **/
  expression integrate(const ex &var, const symbol& integration_constant) const;
  expression integrate(const ex &var, const ex& lowerbound, const ex& upperbound) const;
  expression integrate(const ex &lvar, const symbol& l_integration_constant, const ex &rvar, const symbol& r_integration_constant) const;
  expression integrate(const ex &lvar, const ex& l_lowerbound, const ex& l_upperbound, const ex &rvar, const ex& r_lowerbound, const ex& r_upperbound) const;

  /**
  Reverses right hand side and left hand side of the equation
  @returns The reversed equation
  **/
  expression reverse() const;

protected:
  // Implementation of derivative for equations
  virtual ex derivative(const symbol& s) const override;

private:
    using basic::collect; // Shut up warning about our collect() hiding the base classes' collect
};

class equation_unarchiver {
public:
  equation_unarchiver();
  ~equation_unarchiver();
};
static equation_unarchiver equation_unarchiver_instance;

/// These operators are defined in analogy to operators.hxx. They take care of equations

/**
  Adds two expressions.
  @param e The first expression
  @param add The second expression
  @returns An ex containing the result of the addition
  @exception invalid_argument
  **/
const expression operator+(const expression &e, const expression &add);
const expression operator+(const equation &e, const equation &add);
const expression operator+(const expression &e, const equation &add);
const expression operator+(const equation &e, const expression &add);

/**
  Subtracts an expression from another expression.
  @param e The expression
  @param sub The expression that should be subtracted
  @returns An ex containing the result of the subtraction
  @exception invalid_argument
  **/
const expression operator-(const expression &e, const expression &sub);
const expression operator-(const equation &e, const equation &sub);
const expression operator-(const expression &e, const equation &sub);
const expression operator-(const equation &e, const expression &sub);

/**
  Multiplies two expressions. Throws an exception if the multiplicator is zero.
  @param e The first expression
  @param mul The second expression
  @returns An ex containing the result of the multiplication
  @exception invalid_argument(Multiplication with zero)
  @exception invalid_argument(Cannot multiply two equations)
  **/
const expression operator*(const expression &e, const expression &mul);
const expression operator*(const equation &e, const equation &mul);
const expression operator*(const expression &e, const equation &mul);
const expression operator*(const equation &e, const expression &mul);

/**
  Divides an expression by an expression. Throws an exception if the divisor is zero.
  @param e The first expression
  @param divisor The expression by which e should be divided
  @returns An expression containing the result of the division
  @exception invalid_argument(Division by zero)
  **/
const expression operator/(const expression &e, const expression &divisor);
const expression operator/(const equation &e, const equation &divisor);
const expression operator/(const expression &e, const equation &divisor);
const expression operator/(const equation &e, const expression &divisor);

/**
  Takes the expression to the power of another expression, taking care of equations.
  Beware: This does NOT handle
  the ambiguity if, for example, taking the square root of x^2 (could be +x or -x)!
  @param e The expression
  @param exponent The exponent
  @returns An expression containing the result of the operation
  @exception invalid_argument(Cannot calculate power to exponent that is an equation)
  **/
expression pow(const expression &e, const expression &exponent);

/**
  Negate an expression.
  @param e The expression
  @returns The negated expression
  **/
const expression operator-(const expression &e);

}
#endif

