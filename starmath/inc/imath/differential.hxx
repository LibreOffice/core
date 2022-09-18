/***************************************************************************
    differential.hxx  -  Header file for class differential, an extension class to GiNaC
                             -------------------
    begin                : Sat Feb 22 2014
    copyright            : (C) 2014 by Jan Rheinlaender
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

#ifndef DIFFERENTIAL_H
#define DIFFERENTIAL_H

/**
 * @author Jan Rheinlaender
 * @short Extends GiNaC to handle string "expressions"
 */

#include <stdexcept>
#include <ginac/ginac.h>
#include "printing.hxx"
#include "utils.hxx"

namespace GiNaC {

/// Extends GiNaC to handle differentials as stand-alone objects
class differential : public basic {
  GINAC_DECLARE_REGISTERED_CLASS(differential, basic)

public:
  /**
   * Construct a differential
   * @param e The expression that is to be differentiated
   */
  differential(const ex& e_, const bool partial_ = false, const ex& grade_ = _ex1, const ex& parent_ = _ex0, const bool numerator_ = false);
  differential(const differential& other);
  differential& operator=(const differential& other);

#ifdef DEBUG_CONSTR_DESTR
  ~differential();
#endif

  inline unsigned precedence() const override {return 70;}

  /**
   * Print the differential in a GiNaC print context.
   * @param c The print context (e.g., print_latex)
   * @param level Unused, for consistency with GiNaC print methods
   * @param ndiff Power of the differential
   * @param is_numer If this is true, a power of 2 will be printed as d(d(x)). Otherwise as dx^2
   * @param is_complete If this is false, only dfdt style printing is possible
   */
  void do_print(const print_context &c, unsigned level = 0) const;
  void do_print_imath(const imathprint &c, unsigned level = 0) const;
  void do_print_imath(const imathprint &c, unsigned level = 0, bool is_complete = false, const ex& pdiffto = _ex0) const;

  /// GiNaC internal comparison function, more efficient than compare_same_type()
  bool match_same_type(const basic & other) const override;

  unsigned calchash() const override;

  /**
   * Required by GiNac, does nothing. It would be possible to define simplification or
   * canonicalization rules here, but the whole point of having a differential object
   * is to delay the differentiation until a user request.
   * @param level Unused, required by GiNaC
   * @returns An expression with the result of the evaluation
   */
#if (((GINACLIB_MAJOR_VERSION == 1) && (GINACLIB_MINOR_VERSION >= 7)) || (GINACLIB_MAJOR_VERSION >= 1))
  ex eval() const override;
#else
  ex eval(int level = 0) const;
#endif

  /// Try to evaluate the differential to a number. Only possible if the expression evaluates to a numeric, then the result is zero
#if (((GINACLIB_MAJOR_VERSION == 1) && (GINACLIB_MINOR_VERSION >= 7)) || (GINACLIB_MAJOR_VERSION >= 1))
  ex evalf() const override;
#else
  ex evalf(int level = 0) const;
#endif

  /// Evaluate matrices in the arguments
  ex evalm() const override;

  size_t nops() const override;
  ex op(size_t i) const override;

  ex map(map_function & f) const override;
  ex subs(const exmap & m, unsigned options) const override;
  bool has(const ex & other, unsigned options) const override;

  /// Allow accessing the argument
  inline const ex& argument() const { return e;};
  /// Check whether this is a partial differential
  inline const bool& is_partial() const { return partial; };
  /// Return the grade of the differential
  inline const ex& get_grade() const { return grade; }
  /// Return the numeric grade, or -1
  int get_ngrade() const;
  /// Return the parent
  inline const ex& get_parent() const { return parent; }
  /// Return true if this differential appears in the numerator
  inline bool is_numerator() const { return numerator; }

  /// Set the partial flag
  inline void set_partial(const bool& p) { partial = p; };
  /// Set the grade
  inline void set_grade(const ex& g) { grade = g; }
  /// Set the numerator flag
  inline void set_numerator(const bool& n) { numerator = n; }

  /// Expand the argument
  ex expand(unsigned options) const override;

protected:
  ex derivative(const symbol & s) const override;

private:
  /// The expression to be differentiated
  ex e;
  /// Partial differential (only relevant for multi-variable expressions)
  bool partial;
  /// The grade of the differential
  ex grade;
  /// The parent, e.g. for d(f)/d(x) 'f' is the parent of d(x).
  // Used to match differentials into exderivative objects for differentiating and printing
  ex parent;
  /// This differential is the numerator of an exderivative object
  // Used to prevent GiNaC::mul from automatically cancelling differential objects
  bool numerator;
};

class differential_unarchiver {
public:
  differential_unarchiver();
  ~differential_unarchiver();
};
static differential_unarchiver differential_unarchiver_instance;
}
#endif
