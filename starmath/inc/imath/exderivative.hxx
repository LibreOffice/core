/***************************************************************************
    exderivative.hxx  -  Header file for class exderivative, an extension class to GiNaC
                             -------------------
    begin                : Mon Apr 24 2017
    copyright            : (C) 2017 by Jan Rheinlaender
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

#ifndef EXDERIVATIVE_H
#define EXDERIVATIVE_H

/**
 * @author Jan Rheinlaender
 * @short Extends GiNaC to handle derivatives of arbitrary expressions
 */

#include <ginac/ginac.h>
#include "differential.hxx"
#include "printing.hxx"

namespace GiNaC {

/**
 * Class exderivative
 * Extends GiNaC to handle derivatives of arbitrary expressions
 * This class is intended as a temporary class for proper handling of complete derivatives
 * when differentiating and printing, since handling the muls of differentials does
 * not lead to good results
 */
class exderivative : public basic {
  GINAC_DECLARE_REGISTERED_CLASS(exderivative, basic)

public:
  /**
   * Construct an exderivative
   * @param e The expression that is to be differentiated
   */
  exderivative(const differential& n, const ex& d);
  exderivative(const exderivative& other);

#ifdef DEBUG_CONSTR_DESTR
  exderivative& operator=(const exderivative& other);
  ~exderivative();
#endif

  inline unsigned precedence() const override {return 70;}

  /**
   * Print the exderivative in a GiNaC print context.
   * @param c The print context (e.g., print_latex)
   * @param level Unused, for consistency with GiNaC print methods
   */
  void do_print(const print_context &c, unsigned level = 0) const;
  void do_print_imath(const imathprint &c, unsigned level = 0) const;

  /**
   * Required by GiNac. Returns 1 if denom equals numer
   * @param level Unused, required by GiNaC
   * @returns An expression with the result of the evaluation
   */
#if (((GINACLIB_MAJOR_VERSION == 1) && (GINACLIB_MINOR_VERSION >= 7)) || (GINACLIB_MAJOR_VERSION >= 1))
  ex eval() const override;
#else
  ex eval(int level = 0) const;
#endif

  size_t nops() const override;
  ex op(size_t i) const override;
  ex map(map_function & f) const override;
  ex subs(const exmap & m, unsigned options) const override;

  // Evaluate the exderivative by differentiating the numerator to all differentials in the denominator
  ex eval_diff() const;

  inline bool is_partial() const { return numer.is_partial(); }

  inline const differential& get_numer() const { return numer; }
  inline const ex& get_denom() const { return denom; }

  /// Dissolve the exderivative into single differentials
  inline ex dissolve() const { return numer / denom; }

protected:
  ex derivative(const symbol & s) const override;

  // Calculate the partial derivative of the exderivative with respect to the given variable
  ex pderivative(const ex& v) const;

private:
  /// The numerator of the derivative, e.g. differential(f(x), 2)
  differential numer;
  // The denominator of the derivative, e.g. differential(y) * differential(x)
  ex denom;
};

class exderivative_unarchiver {
public:
  exderivative_unarchiver();
  ~exderivative_unarchiver();
};
static exderivative_unarchiver exderivative_unarchiver_instance;

}
#endif
