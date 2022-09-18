/***************************************************************************
                          operands.hxx  -  Header file for class operands
                             -------------------
    begin                : Wed Jun 26 2002
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

#ifndef OPERANDS_H
#define OPERANDS_H

/**
  @short  A helper class for the Latex printing routines. With the methods of this class, all elements of a
  GiNaC expression can be stored separately (i.e., functions, numerics, symbols etc.) Class operands assumes that
  any expression is either an add or a mul. The class offers functions for checking whether an operand is a quantity,
  a numeric etc., and for pretty-printing in Latex syntax.
  @author Jan Rheinlaender
**/

#include <stdexcept>

#ifdef INSIDE_SM
#include <imath/expression.hxx>
#include <imath/utils.hxx>
#else
#include "expression.hxx"
#include "utils.hxx"
#endif

// Possible value for ops_type
#define GINAC_ADD _ex0
#define GINAC_MUL _ex1

/// A helper class for the Latex printing routines
class operands {
  /**
  Definition of the function used to connect the operands (addition or multiplication)
  **/
  typedef GiNaC::ex (*op_func)(const GiNaC::ex&, const GiNaC::ex&);

  /**
  An integer defining the type of the operands (GiNaC::add or GiNaC::mul)
  **/
  GiNaC::ex type;

  /**
  An expression containing all the functions.
  This includes GiNaC's functions and our functions
  **/
  GiNaC::ex functions;

  /// An expression containing all the integrals
  GiNaC::ex integrals;

  /// An expression containing all the differentials
  GiNaC::ex differentials;

  // An expression containing all the derivatives
  GiNaC::ex derivatives;

  /**
  An expression containing all the symbols.
  **/
  GiNaC::ex symbols;

  /// An expression containing all the constants.
  GiNaC::ex constants;

  /**
  An expression containing all the units.
  **/
  GiNaC::ex units;

  /**
  An expression containing all the adds. This is empty if the oper_func is &add_ops.
  **/
  GiNaC::ex adds;

  /**
  An expression containing all the muls. This is empty if the oper_func is &mul_ops.
  **/
  GiNaC::ex muls;

  /**
  An expression containing all the powers.
  **/
  GiNaC::ex powers;

  /**
  An expression containing the numeric coefficient.
  **/
  GiNaC::ex coefficient;

  /// An expression containing the matrices
  GiNaC::ex matrices;

  /**
  An expression containing all the GiNaC types that might occur.
  **/
  // TODO: Implement them and get rid of others.
  GiNaC::ex others;

  /**
  A pointer to the function (either add_ops() or mul_ops()) that is to be used to connect the operands.
  **/
  op_func oper;

public:
  /**
  Create a new instance of class operands. Throws an exception if ops_type is neither GINAC_ADD nor
  GINAC_MUL. All operand types (functions, powers, etc.) are initialized to the value of ops_type.
  @param ops_type An integer indicating whether the operands are part of an add or a mul
  @exception invalid_argument
  **/
  operands(const GiNaC::ex& ops_type);

#ifdef DEBUG_CONSTR_DESTR
  ~operands();
  operands(const operands& other);
  operands& operator=(const operands& other);
#endif

  /**
   * This is a kind of constructor, too. It takes an expression and splits it into operands. If any
   * unknown GiNaC types are encountered, the method throws an exception.
   * -# If the expression is an add, it is split into positive and negative operands
   * -# If the expression is a mul, it is split into positive and negative powers
   @param e The GiNaC expression that is to be split into operands
   @param o1 The positive operands or positive powers
   @param o2 The negative operands or negative powers
   @exception runtime_error
   */
  static void split_ex(const GiNaC::expression &e, operands &o1, operands &o2);

  /**
  @returns The numeric coefficient of the operands.
  **/
  inline const GiNaC::ex& get_coefficient() const { return coefficient; }

  /**
  @returns The symbols of the operand.
  **/
  inline const GiNaC::ex& get_symbols() const { return symbols; }

  /**
  @returns The constants of the operand.
  **/
  inline const GiNaC::ex& get_constants() const { return constants; }

  /**
  @returns The units of the operand.
  **/
  inline const GiNaC::ex& get_units() const { return units; }

  /**
  @returns The functions of the operand.
  **/
  inline const GiNaC::ex& get_functions() const { return functions; }

  /**
  @returns The integrals of the operand
  **/
  inline const GiNaC::ex& get_integrals() const { return integrals; }

  /**
  @returns The differentials of the operand
  **/
  inline const GiNaC::ex& get_differentials() const { return differentials; }
  inline void set_differentials(const GiNaC::ex& d) { differentials = d; }

  /**
  @returns The derivatives of the operand
  **/
  inline const GiNaC::ex& get_derivatives() const { return derivatives; }

  /**
  @returns The adds of the operand. If the receiver is of type GINAC_ADD, this returns 0.
  **/
  inline const GiNaC::ex& get_adds() const { return adds; }

  /**
  @returns The muls of the operand. If the receiver is of type GINAC_MUL, this returns 1.
  **/
  inline const GiNaC::ex& get_muls() const { return muls; }

  /**
  @returns The matrices of the operand.
  **/
  inline const GiNaC::ex& get_matrices() const { return matrices; }

  /// Checks if there are any functions stored in the operand
  bool check_functions(const GiNaC::ex& t) const;

  /// Checks if there are any matrices stored in the operand
  bool check_matrices(const GiNaC::ex& t) const;

  /// Checks if there are any symbols stored in the operand
  bool check_symbols(const GiNaC::ex& t) const;

  /**
  @returns The powers of the operand.
  **/
  inline const GiNaC::ex& get_powers() const { return powers; }

  /**
  @returns Any other GiNaC data types.
  **/
  inline const GiNaC::ex& get_others() const { return others; }

  /**
  Includes an expression in the receiver. Note that any complex expressions (adds or muls) that are
  passed to this method will just be added to the adds or muls field respectively. They are not split
  into their subparts and included according to type.
  @param what The expression to include
  **/
  void include(const GiNaC::ex &what);

  /**
  Excludes an expression from the receiver by multiplying the contents of the field corresponding to
  its type with 1 / what. Note that this is NOT possible with include(1/what) because the type of this
  is always a power!

  @param what The expression to exclude
  **/
  void exclude(const GiNaC::ex &what);

  /// Remove all elements of a certain type
  inline void clear_diffs() { differentials = type; }
  inline void clear_derivatives() { derivatives = type; }
  inline void clear_symbols() { symbols = type; }
  inline void clear_constants() { constants = type; }
  inline void clear_units() { units = type; }

  /**
  @returns True if the receiver is a quantity.
  **/
  bool is_quantity() const;

 /**
  @returns True if the receiver is a numeric.
  **/
  bool is_number() const;

  /**
  @returns True if the receiver is trivial (0 or 1, depending on the op_type).
  **/
  bool is_trivial() const;

  /**
  @returns True if the receiver consists of only one symbol
  **/
  bool is_symbol() const; // *** added in 1.2

  /**
  @returns True if the receiver is of type GINAC_ADD.
  **/
  bool is_add() const;

  /**
  @returns True if the receiver is of type GINAC_MUL.
  **/
  bool is_mul() const;

  /// Print the receiver onto a stream
  void print(std::ostream &os) const;
};

/// Helper structure for use in remember_split
struct exrec {
  /// The expression
  GiNaC::expression e;
  /// The first operands
  operands o1;
  /// The second operands
  operands o2;
  /// Just for statistical purposes...
  unsigned hits;
  /// Create a new instance
  exrec(const GiNaC::expression &exp, const operands &op1, const operands &op2);
  ~exrec();
#ifdef DEBUG_CONSTR_DESTR
  exrec(const exrec& other);
  exrec& operator=(const exrec& other);
#endif
};

/// Split the equation at this position if requested by the user
void checksplit(const bool toplevel, const int opnum, std::ostream &os);
#endif
