/***************************************************************************
    eqc.hxx  -  Header file for class eqc
                             -------------------
    begin                : Sun Oct 21 2001
    copyright            : (C) 2016 by Jan Rheinlaender
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

/**
@short Used to store all equations, variables and constants encountered during processing. It is then
possible to request variable values from eqc, which will be derived from the registered equations.
@author Jan Rheinlaender
**/

#ifndef EQC_H
#define EQC_H

#include <iostream>
#include <string>
#include <stdexcept>
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning (disable: 4099 4100 4996)
#endif
#include <ginac/ginac.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif
#include "equation.hxx"
#include "extsymbol.hxx"
#include "utils.hxx"
#include "unitmgr.hxx"

#define VALSYM "__eqc-val-sym__"
#define VALLABEL "__eqc-val-label__"

/// Possible types of symbols
enum symtype {
  t_variable, // A user-defined variable
  t_constant, // A constant, i.e., a variable with a fixed value
  t_function, // A user-defined function
  t_none      // Variable is not defined in the compiler
};
enum symprop {
  p_complex,  // This is the standard for symbols
  p_vector,   // A vector (only used by getsymtype, for iMath)
  p_matrix,   // A matrix/tensor (only used by getsymtype, for iMath)
  p_real,     // A real-valued variable (that is, not complex)
  p_pos       // A positive-valued variable (that is, not complex and greater than zero)
};

/// Storage for equations and the result of the last substitution
struct eqrec {
  /// The equation
  GiNaC::expression eq;

  /// The equation label
  std::string label;
  /// The label of the previous equation
  std::string prevlabel;

  /// Check for automatic label
  bool is_automatic() const;

  /// The result of the last substitution of assignments - lhs
  GiNaC::expression subsed_lhs;
  /// The result of the last substitution of assignments - rhs
  GiNaC::expression subsed_rhs;

  /// Create a new eqrec for the equation e and the label l and the previous label pl
  eqrec(const GiNaC::expression &e, const std::string& l, const std::string& pl);

#ifdef DEBUG_CONSTR_DESTR
  eqrec();
  eqrec(const eqrec& other);
  eqrec& operator=(const eqrec& other);
  ~eqrec();
#endif
};

/**
 * This class is used to store variables used by the equations registered with the compiler. Variables
 * may be constants, in this case the value of the constant is stored as well.
 */
// TODO: Create a subclass variable of class symbol for this purpose!
// Or enhance class func to handle symbols as special functions
class symrec {
private:
  /// The symbol representing the variable
  GiNaC::extsymbol& sym;

  /// Indicates whether the symbol is a variable, constant or function
  symtype type;
  /// Indicates whether the symbol is a vector, matrix, complex number, real number or positive number
  symprop prop;

public:
  /**
  An expression for storing the value of the symbol. This needs to be an expression
  because most values will be quantities.
  **/
  GiNaC::expression val;
  GiNaC::expression aval; // Value not reduced to floats, cf. AVAL etc.

  /// A list of equations that define the value of this symbol
  std::list<eqrec*> assignments;

  /// Create a new symrec
  symrec(const symtype t, const std::string& varname, const symprop p);

  /// Assignment operator
  symrec& operator=(const symrec& other);

#ifdef DEBUG_CONSTR_DESTR
  symrec();
  symrec(const symrec& other);
  ~symrec();
#endif

  /// Change the symbol type
  void setsymtype(const symtype t) { type = t; }

  /// Change the symbol properties
  void setsymprop(const symprop p);

  /// Return the symbol
  inline const GiNaC::extsymbol& getsym() const { return sym; }

  /// Return the symbol type
  inline const symtype& getsymtype() const { return type; }

  /// Return the symbol properties
  inline const symprop& getsymprop() const { return prop; }

  /// Return the name of the symbol
  inline std::string get_name() const { return sym.get_name(); }

  /// Set the value of the symbol to unknown
  void make_unknown();

  /// Return true if the symbol has a value
  bool has_value() const;
};

/// Used to store all equations, variables and constants encountered during processing.
class eqc {
public:
  /// Unit management
  Unitmanager unitmgr;

private:
  /**
  A map which stores all equations. The key is a string containing the label of the equation or the equation number.
  **/
  std::map<const std::string, eqrec> equations;
  typedef std::map<const std::string, eqrec>::iterator eqrec_it;
  typedef std::map<const std::string, eqrec>::const_iterator eqrec_cit;

  /// A map which stores all expression. The key is a string containing the label
  std::map<const std::string, GiNaC::expression> expressions;

  /// Contains the next number to be given to an equation if no label is supplied.
  int nextlabel;

  /// A list which stores equations that are not assignments
  std::list<eqrec*> other_equations; // Note: This will only take pointers to entries in equations, therefore no deletion on destruction of the eqc is required
  typedef std::list<eqrec*>::iterator eqreclist_it;
  typedef std::list<eqrec*>::const_iterator eqreclist_cit;

  /// An exmap which contains all constants and assignments for variables found so far
  GiNaC::exmap assignments;

  /**
  An exmap which contains newly created constants and assignments, until they are used
  in find_values()
  **/
  GiNaC::exmap recent_assgn; // Cannot use exhashmap because subs() doesn't accept it

  /**
  A map which stores all variables used in the equations. The key is the variable name (string), and the variables are
  stored with their calculated values in the structure symrec, which has the following members:
  @li symbol: A GiNaC symbol that represents the variable
  @li value:  A GiNaC expression that contains the value of the variable (a quantity). This member is
              only used for constants
  @li constant: A boolean that indicates whether the variable is a constant

  Note that variables marked as constants are not deleted with the clear() method.
  **/
  std::map <std::string, symrec> vars;
  typedef std::map <std::string, symrec>::iterator symrec_it;
  typedef std::map <std::string, symrec>::const_iterator symrec_cit;

  /// The current namespace
  std::string current_namespace;

  /// This iterator always points to the previously registered equation
  eqrec_it previous_it;

public:
  /**
  Constructs an empty eqc.
  **/
  eqc();

  /// Prevent shallow copies
  eqc(const eqc& other) = delete;
  eqc(eqc&& other) noexcept = delete;
  eqc& operator=(const eqc& other) = delete;
  eqc& operator=(eqc&& other) noexcept = delete;

  /// Implement deep copy
  // Note: A shared_ptr is returned because it is impossible to create one from a plain eqc object without invoking the move constructor
  std::shared_ptr<eqc> clone();

  /**
  Register this equation with the compiler. An exception is thrown if
  the specified label already exists. It is always made the 'PREVIOUS' equation.
  The equation is always registered so that it can be accessed as the 'PREVIOUS' equation.
  The old 'PREVIOUS' equation is deleted if it is temporary and no assignment

  @param eq The equation to check and register
  @param l (optional) The label for the equation (if none is given one will be created automatically)
  @exception invalid_argument(The equation label already exists), logic_error(equation redefines constant)
  **/
  void check_and_register (const GiNaC::expression &eq, const std::string& l = "");

  /**
  Remove the equation referenced by the given label (this basically has the effect that the
  equation never existed)
  @param which The label of the equation to be removed
  @exception invalid_argument(The equation label does not exist)
  **/
  void deleq(const std::string &which);
  void deleq(eqrec_it which);

  /**
  Register a constant with the eqc.
  The difference to registering an equation is:
  @li \\constant equations are not added to the list of equations
  @li \\constant values are remembered even after \\clearequation
  @li It is not possible to register a symbol as a constant multiple times

  The method throws an exception if
  -# the constant already exists
  -# the value of the constant (right hand side of the equation) is no quantity
  -# the left hand side of the equation is no symbol

  @param eq An equation containing the value of the constant. The expression is evaluated using the current list of
  assignments and then checked to ensure that it represents a quantity.
  @exception invalid_argument
  **/
  void register_constant (const GiNaC::expression &eq);

  /// Register a function with the compiler
  void register_function (const std::string &n, GiNaC::exvector &args, const unsigned hints, const std::string& printname = "");

  /*
   * Register an expression so that it can be retrieved by its label
     @param eq The equation to check and register
     @param l The label for the expression (if none is given one will be created automatically)
   **/
  void register_expression (const GiNaC::expression &ex, const std::string& l);

  /// Begin and end a namespace. Namespaces are prepended to equation labels and variables
  void begin_namespace(const std::string& ns);
  void end_namespace(const std::string& ns);
  /// Prepend the namespace to the variable name
  std::string varname_ns(const std::string& varname) const;
  /// Prepend the namespace to the equation label. If check is true, try the current namespace first and then the global namespace
  std::string label_ns(const std::string& label, const bool check = false) const;
  /// Prepend the namespace to the expression label. If check is true, try the current namespace first and then the global namespace
  std::string exlabel_ns(const std::string& label, const bool check = false) const;

  /**
  Request a new symbol from the eqc. A GiNaC symbol is created with this name. If a variable with this name
  already exists, return the Ginac symbol representing it

  @param varname A string with the name of the variable.
  @param p The symbol properties (vector, matrix, complex, real positive)
  @returns The Ginac symbol created for this variable, or the existing symbol, if this variable already
  exists.
  **/
  // The GiNaC constant Pi is returned directly when the symbol with name %pi is requested.
  // The same is valid for the symbol with name %e and i
  GiNaC::expression getsym (const std::string& varname, const symprop p = p_complex);
  GiNaC::expression* getsymp (const std::string& varname, const symprop p = p_complex);

  /// Return the type of the variable with this name
  symtype getsymtype(const std::string& varname);
  /// Return the properties of the variable with this name
  symprop getsymprop(const std::string& varname);

  /// Allow setting the symbol properties after creation (important for persisted symbols)
  void setsymprop(const std::string& varname, const symprop p);

  /**
    Check whether the string is a valid equation label, that is, an equation with that
    label has been registered.
    @param s The string to check for
    @returns A boolean indicating whether an equation with this label exists
  **/
  bool is_label(const std::string &s)const;
  bool is_expression_label(const std::string &s) const;

  /// Check whether the string is the label of a library equation
  bool is_lib(const std::string &s) const;

  /**
  Check whether a given symbol has a value (i.e., a quantity). Throws an exception if the symbol
  is not registered with the compiler.

  @param s The symbol
  @returns A boolean indicating whether the symbol has a value
  @exception range_error(Symbol is not registered with the compiler)
  **/
  bool has_value(const GiNaC::symbol& v) const;

  /**
  Return the assignment that defines the value of this variable

  @params s The symbol
  @returns An expression containing the assignment
  @exception range_error(Symbol is not registered with the compiler)
  **/
  GiNaC::expression get_assignment(const GiNaC::symbol& s) const;

  /**
  Return the stored value of this symbol without going through all the hassle of find_values()
  This assumes that has_value(s) returned true!

  @param s The symbol
  @returns The value of the symbol
  @exception range_error(Symbol is not registered with the compiler)
  **/
  GiNaC::expression get_value(const GiNaC::symbol &s) const;

  /**
  A wrapper function for find_values(). Searches for an equation defining the symbol, substitutes as many
  known values as possible on the right hand side of this equation, tries to reduce the resulting expression
  to a numeric by calling evalf(), and returns the result. This result can be a quantity
  or any symbolic expression, depending on the success of the substition process. An optional list of assignments
  can be used to find the value.

  For exceptions thrown, see find_values().

  @param var The variable for which the value is requested
  @param assignments An optional expression containing an equation or a list of equations
  @param tofloat If true, evaluates all numbers to floats. If false, things like sqrt{3} or sin(5) are not evaluated
  @returns An expression containing the value
  @exception invalid_argument
  **/
  GiNaC::expression find_value_of(const GiNaC::symbol &var, const GiNaC::lst &assgn = GiNaC::lst(), const bool tofloat = true);

  /** A wrapper function for find_values(). Same as find_value_of(), only that an exception is thrown if
  the value is no quantity.
  @param var The variable for which the quantity is requested
  @param assignments An optional expression containing an equation or a list of equations
  @returns An expression containing the quantity
  @exception invalid_argument
  **/
  GiNaC::expression find_quantity_of(const GiNaC::symbol &var, const GiNaC::lst &assgn = GiNaC::lst());

  /** A wrapper function for find_values(). Same as find_value_of(), only that an exception is thrown if
  the value is no numeric.
  @param var The variable for which the numeric value is requested
  @param assignments An optional expression containing an equation or a list of equations
  @returns A numeric containing the value
  @exception invalid_argument
  **/
  GiNaC::numeric find_numval_of(const GiNaC::symbol &var, const GiNaC::lst &assgn = GiNaC::lst());

  /** A wrapper function for find_values(). Same as find_value_of(), only that an exception is thrown if
  the value is no unit (or a multiplication of units and powers of units).
  @param var The variable for which the units are requested
  @param assignments An optional expression containing an equation or a list of equations
  @returns An expression containing the units
  @exception invalid_argument
  **/
  GiNaC::expression find_units_of(const GiNaC::symbol &var, const GiNaC::lst &assgn = GiNaC::lst());

  /** Scan the expression for symbols and return a map from the symbols to their values
  @param e The expression
  @returns A map from the symbols to their values
  **/
  GiNaC::exhashmap<GiNaC::ex> find_variable_values(const GiNaC::expression& e) const;

  /** Iterate an expression until it converges. The user is responsible to ensure convergence. Also, it is highly recommended to iterate
  numeric expressions only (that is, expressions that always evaluate to a numeric).
  @param syms The symbols (variables) to iterate
  @param exprs The expressions (expressed in the symbols) that are used to find the value of the symbols for the next iteration
  @param start The start values of the symbols (iteration 0)
  @param conv The convergence criteria. Iteration stops when v_n - v_n-1 < conv for all symbols
  @param maxiter The maximum number of iterations, if no convergence happens
  All matrix parameters are assumed to have one column only and to have the same number of rows. All convergence criteria are assumed to be real numbers
  **/
  GiNaC::matrix iterate(const GiNaC::matrix& syms, const GiNaC::matrix& exprs, const GiNaC::matrix& start, const GiNaC::matrix& conv, const unsigned maxiter = 10);

  /**
  Deletes all equations and all variables that are not constants.
  **/
  void clear();

  /**
  Deletes all equations and all variables. This is equivalent to destroying the eqc object and constructing
  a new one.
  @param persist_symbols If true, symbols accessed with getsym() will be consistent across compilation runs.
  This allows caching of compiled equations
  **/
  void clearall(const bool persist_symbols = false);

  /// Print information about the compiler onto a stream
  void print(std::ostream &os) const;

  /// Dump information about variables and their values onto the stream
  void dumpvars(std::ostream & os);

  /**
  Looks up an equation by its label. Throws an exception if the label does not exist.
  @param label The equation label
  @returns The equation registered under this label
  @exception range_error
  **/
  const GiNaC::expression& at(const std::string &label) const;
  /**
  Looks up an expression by its label. Throws an exception if the label does not exist.
  @param label The expression label
  @returns The expression registered under this label
  @exception range_error
  **/
  const GiNaC::expression& expression_at(const std::string &label) const;

  /// Find the nth previous equation and return its label
  std::string getPreviousEquationLabel(const unsigned n) const;
private:
  /**
  Investigate all equations that influence the value of a variable, and try to calculate the
  value from these equations. Up to now, only equations that have this variable on the left hand side
  are investigated. Throws an exception if the variable has no value at all.

  @param var A GiNac symbol representing the variable of which the value is wanted
  @param v A numeric which will contain the numeric part of the value of the variable. If the value
  cannot be separated into a numeric part and its units, the contents of this parameter are undefined
  @param u An expression which will contain the units of the value of this variable.
  @param value An expression which contains the value (numeric part and units) of the variable. If the
  value cannot be split into a numeric part and units, this variable contains the right hand side of
  the last equation that has the variable on the left hand side
  @param assignments An optional expression containing an equation or a list of equations
  @returns A boolean indicating whether this variable's value is a quantity
  @exception invalid_argument
  **/
  bool find_values(const GiNaC::symbol &var, GiNaC::numeric &v, GiNaC::expression &u, GiNaC::expression &value, const GiNaC::lst &assgn = GiNaC::lst(), const bool tofloat = true);

  /**
  Helper function: Check if the equation is an assignment, using all previously found
  assignments

  @param eqr The equation record of the equation to check
  @param names If any one side becomes a symbol or a column vector of symbols, the names of these symbols (empty vector if  check_eq() returns false)
  @param rhs The rhs of the evaluated equation (always a column vector)
  @param arhs The rhs, without reduction to floats of things like sqrt{3}, sin(1/2) (cf. AVAL etc.)
  @returns True if the lhs is a symbol or a column vector of symbols, that is, the equation is an assignment
  **/
  bool check_eq(eqrec* eqr, std::vector<std::string> &names, GiNaC::expression &rhs, GiNaC::expression &arhs);

  /**
  Helper function: Store an assignment sym = rhs, which was derived from equation label.
  This function makes sure that the assignments which defines the value of a variable
  always is the first in vars[...]->assignments.

  @param names The names of the symbols (lhs of the assignment)
  @param rhs The rhs of the assignment (always a column vector)
  @param arhs The rhs of the assignment (without reduction to floats)
  @param eqr The equation record of the equation that yielded the assignment
  @returns True if the rhs is a quantity, false otherwise
  @throws logic_error(equation redefines constant)
  **/
  bool store_assgn(const std::vector<std::string> &names, const GiNaC::expression &rhs, const GiNaC::expression &arhs, eqrec* eqr);

  /**
  Helper function: Adds sym to the list syms. Checks if the value of any variables depends on
  the value of sym. If yes, the value of that variable is removed and the function is called
  again with that variable as sym.

  @param name The name of the variable that has no value any more
  @param names A list of such variables that is recursively collected
  **/
  void remove_assignment(const GiNaC::ex sym, std::list<GiNaC::ex>& syms);

  /**
  Helper function: Go through all the registered equations, and remove the stored lhs and
  rhs substituted values, if the equation contains the symbol.
  @param name The name of the variable that has no value any more
  **/
  void remove_subsed(const GiNaC::ex& sym);
};
#endif
