/***************************************************************************
  func.hxx  -  header file for class func, extending the GiNaC function class
                             -------------------
    begin                : Fri Feb 13 2004
    copyright            : (C) 2004 by Jan Rheinlaender
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

#ifndef FUNC_H
#define FUNC_H

/**
 * @author Jan Rheinlaender
 * @short Extends the GiNaC function class
 */

#include <string>
#include <stdexcept>
#include <ginac/ginac.h>
#include "expression.hxx"
#include "utils.hxx"

namespace GiNaC {

/// A map containing the numbers assigned to different hints
#define FUNCHINT_NONE 1       // Default, no effect
#define FUNCHINT_LIB 2        // A library function, will not be deleted by clear()
#define FUNCHINT_TRIG 4       // For special printing of trigonometric functions
#define FUNCHINT_EXPAND 8     // Function is to be expanded immediately (e.g. square)
#define FUNCHINT_NOBRACKET 16 // Function does not need brackets when printed (e.g. sqrt{...})
#define FUNCHINT_DEFDIFF 32   // Differentiate definition, not function itself
#define FUNCHINT_PRINT 64     // Use hard-coded printing function
const std::map<std::string, unsigned> funchint_map = {
  {"none",      FUNCHINT_NONE},
  {"lib",       FUNCHINT_LIB},
  {"trig",      FUNCHINT_TRIG},
  {"expand",    FUNCHINT_EXPAND},
  {"nobracket", FUNCHINT_NOBRACKET},
  {"defdiff",   FUNCHINT_DEFDIFF},
  {"print",     FUNCHINT_PRINT}
};

// Element-wise operations on vectors and matrices
enum hadamard_operation {
  h_product = 0,
  h_division = 1,
  h_power = 2
};

class imathprint;

/// This structure stores all the information about a user defined function
struct funcrec {
  /// The serial number of the function, if it is hard-coded into GiNaC
  unsigned serial;

  /// dependant variables of the function (e.g. (x; y; z)
  exvector vars;

  /// The definition of the function (e.g. for cubic: x^3)
  expression definition;

  /// True if the function is hard-coded into GiNaC
  bool hard;

  /// Contains hints for handling and printing of the function
  unsigned hints;

  /// Optional print name of the function
  std::string printname;

  /// Construct empty funcrec with hints
  funcrec(const unsigned h);

#ifdef DEBUG_CONSTR_DESTR
  funcrec();
  funcrec(const funcrec& other);
  funcrec& operator=(const funcrec& other);
  ~funcrec();
#endif
};

/// Extends GiNaC::function by runtime definition of functions
class func : public exprseq {
  GINAC_DECLARE_REGISTERED_CLASS(func, exprseq)
private:
  /// Helper function for the constructors
  void initAttributes();
  void print_name(const print_context &c) const;
public:
  /**
   * Construct a func with the given name
   * @param n A string with the name of the function
   * @param args An exvector containing the arguments of the function, e.g. (2*x; y^2)
   * @param discardable Required by GiNaC
   */
#if (((GINACLIB_MAJOR_VERSION == 1) && (GINACLIB_MINOR_VERSION >= 7)) || (GINACLIB_MAJOR_VERSION >= 1))
  func(const std::string &n, const exvector &args = exvector());
  func(const std::string &n, exvector && args);
#else
  func(const std::string &n, const exvector &args = exvector(), bool discardable = false);
  func(const std::string &n, exvector &&args, bool discardable = false);
#endif
  func(const std::string &n, const exprseq &args);
  func(const std::string &n, exprseq &&args);

  /// Create a function with a single argument
  func(const std::string &n, const expression &e);

#ifdef DEBUG_CONSTR_DESTR
  func(const func& other);
  func& operator=(const func& other);
  ~func();
#endif

  /// Print the function arguments
  void printseq(const print_context & c, const std::string &openbracket, const std::string& delim, const std::string &closebracket) const;

  /**
   * Print the function in a GiNaC print context.
   * @param c The print context (e.g., print_latex)
   * @param p A power argument for the function, important for printing sin^2 x instead of (sin x)^2
   * @param level Unused, for consistency with GiNaC print methods
   */
  void print_imath(const imathprint &c, const ex &p, unsigned level = 0) const;

  /**
   * Print the function in a GiNaC print context.
   * @param c The print context (e.g., print_latex)
   * @param level Unused, for consistency with GiNaC print methods
   */
  // *** changed name to do_print in 0.8
  void do_print(const print_context &c, unsigned level = 0) const;
  void do_print_imath(const imathprint &c, unsigned level = 0) const;

  /// Print function with difftype line
  void print_diff_line(const ex& g, const int gr, const print_context& c) const;

  // functions overriding virtual functions from base classes, see functions.hxx
  /// Set the precedence for functions
  inline unsigned precedence(void) const override { return 70; }

  /// expand the function, using its definition on the arguments
  static const unsigned no_expand_function_definition = 0x8000; // Don't expand the function definition. Extends GiNaC::expand_options
  ex expand(unsigned options = 0) const override;

  /// Override these functions because the default applies them to the arguments
  ex conjugate() const override;
  ex imag_part() const override;
  ex real_part() const override;

  /// symbolic integration (implemented for simple cases only)
  bool find_integral(const ex& sym, ex& result) const;

  /// Evaluate the function to a numeric value, if possible
#if (((GINACLIB_MAJOR_VERSION == 1) && (GINACLIB_MINOR_VERSION >= 7)) || (GINACLIB_MAJOR_VERSION >= 1))
  ex eval() const override;
#else
  ex eval(int level=0) const;
#endif

  /// Substitute the exmap in the function arguments and (for pure functions) in the variables
  ex subs(const exmap & m, unsigned options) const override;

  // Map the function arguments and (for pure functions) the variables
  ex map(map_function &f) const override;

  /// Do unsafe evaluations of the function
  struct reduce_double_funcs : public map_function {
    /// Map the expression
    ex operator()(const ex &e);
  };

  /// Replace all GiNaC functions by EQC funcs
  struct replace_function_by_func : public map_function {
    /// Map the expression
    // The expression e must already be fully evaluated
    ex operator()(const ex &e);
  };

  /// Expand all sum functions (by adding up)
  struct expand_sum : public map_function {
    /// Map the expression
    ex operator()(const ex &e);
  };

  bool has(const ex & other, unsigned options = 0) const override;

  /**
   * Evaluate the function without arguments
   * @param level An argument to control the recursion depth. Use level=1 to evaluate only the top level
   * @returns A GiNaC::ex with the result of the evaluation
   * @exception runtime_error(Max recursion level reached)
   **/
#if (((GINACLIB_MAJOR_VERSION == 1) && (GINACLIB_MINOR_VERSION >= 7)) || (GINACLIB_MAJOR_VERSION >= 1))
  ex evalf() const override;
#else
  ex evalf(int level = 0) const;
#endif
  /// Evaluate matrices in the function arguments
  ex evalm() const override;

  /// Calculate the hash value of the function
  unsigned calchash(void) const override;

  /// Create a series expansion of the function
  ex series(const relational &r, int order, unsigned options = 0) const override;

  /// Return this container with v
  // Required for container::subs() to work properly!
  ex thiscontainer(const exvector &v) const override;
#if (((GINACLIB_MAJOR_VERSION == 1) && (GINACLIB_MINOR_VERSION >= 7)) || (GINACLIB_MAJOR_VERSION >= 1))
  ex thiscontainer(exvector &&v) const override;
#else
  ex thiscontainer(std::auto_ptr<exvector> vp) const override;
#endif

protected:
  /// Calculate the (total) derivative of the function with respect to s
  ex derivative(const symbol & s) const override;

  /**
   * Apply the function definition on the function arguments
   * An exception is thrown if the function is not registered, or has no definition
   * @exception runtime_error(Function is not registered)
   * @exception logic_error(Function has no definition)
   **/
  expression expand_definition() const;

  /// Equality checking functions required by GiNaC
  bool is_equal_same_type(const basic & other) const override;
  /// Type matching function required by GiNaC
  bool match_same_type(const basic & other) const override;
  /// Return type of the function
  unsigned return_type(void) const override;

  /**
   * Calculate the partial derivative of the function with respect to the nth dependant variable
   * @param diff_param The number of the dependant variable
   * @returns An expression containing the partial derivative
   * @exception logic_error(Parameter does not exist)
   **/
  ex pderivative(unsigned diff_param) const;

// static (class) functions----------------------------------------------------
private:
  static bool initialized;

public:
  /// Initialize the map of functions with the GiNaC hard-coded functions
  static void init();

  /// Delete the function
  static void remove(const std::string& fname);

  /// Return the function table to the state after reading 'mathconstants.tex'
  static void clear();

  /// Return the function table to the state after init()
  static void clearall();

  /**
   * Register a user-defined function so that the scanner will be able to look it up.
   * @param n    The Latex name of the function which the scanner finds in the input file
   * @param args The arguments of the function. This defines number and name of the default arguments.
   *             The arguments are moved into the funcrec, therefore args will become an empty exvector
   * @param h A flag giving hints for printing and processing the function
   * @param printname An optional name to be printed, instead of the function name
   * @exception invalid_argument(Function already exists, argument is no symbol)
   **/
  static void registr(const std::string &n, exvector &args, const unsigned h = 0, const std::string& printname = "");

  /// Return the number assigned to this hint
  static unsigned hint(const std::string &s);

  /**
   * Define a user-defined function
   * @param n    The name of the function
   * @param def  An expression defining how to evaluate the function
   * @exception invalid_argument (Function does not exist)
   **/
  static void define(const std::string &n, const expression &def);

  /**
   * Check whether fname is a function
   * @param fname A string containing the name of the function
   * @returns True if the function exists
   **/
  static bool is_a_func(const std::string &fname);

  /// Return true if hint expand is set for this function name
  static bool is_expand(const std::string& fname);

  /// Return true if this function name refers to a library function
  static bool is_lib(const std::string& fname);

// member functions-------------------------------------------
  /**
   * Return the name of the function
   * @returns A string with the name of the function - not the Latex name!
   */
  inline const std::string& get_name() const { return(name); }

  /// Return the number of arguments of the function
  inline size_t get_numargs() const { return vars.size(); }

  // Return arguments if there are any, else the variables
  exvector get_args() const;

  /**
  * Return true if this is a trigonometric function, else false
  * @exception runtime_error(Function is not registered)
  **/
  inline bool is_trig() const { return (hints & FUNCHINT_TRIG); }

  /// Returns true if this function should be printed without brackets
  inline bool is_nobracket() const { return (hints & FUNCHINT_NOBRACKET); }

  /**
  * Return true if this is a library function, else false
  * @exception runtime_error(Function is not registered)
  **/
  inline bool is_lib() const { return (hints & FUNCHINT_LIB); }

  /// Return true if this function is expanded immediately when it is created
  inline bool is_expand() const { return (hints & FUNCHINT_EXPAND); }

  /// Return true if the function has no arguments (useful for printing)
  inline bool is_pure() const { return(seq.empty()); }

// member variables -------------------------------------------------
protected:
  /// The name of the function, this is used to look it up in the map of registered functions
  std::string name;

  /// The serial number of the function, if it is hard-coded into GiNaC
  unsigned serial;

  /// dependant variables of the function (e.g. (x; y; z)
  exvector vars;

  /// The definition of the function (e.g. for cubic: x^3)
  expression definition;

  /// True if the function is hard-coded into GiNaC
  bool hard;

  /// Contains hints for handling and printing of the function
  unsigned hints;

  /// The name for printing the function (optional)
  std::string printname;

// class variables ------------------------------------------------
  /**
   * A map containing a list of function names and pointers to the corresponding func objects
   **/
  static std::map<const std::string, funcrec> functions;
  typedef std::map<const std::string, funcrec>::const_iterator funcrec_cit;
  typedef std::map<const std::string, funcrec>::iterator funcrec_it;

  /// A map containing the names of the functions hard-coded in GiNaC and their Latex equivalents
  static std::map<const std::string, std::string> hard_names;
  /// The reverse map, for fast lookup in the opposite direction
  static std::map<const std::string, std::string> hard_names_rev;

  /// A map containing functions and their inverse functions
  static std::map<std::string, std::string> func_inv;

private:
    using exprseq::printseq; // Shut up warning about our printseq() hiding the base classes' printseq()
};

// Fake the GiNaC unarchiver functionality because it breaks unopkg with error "Class ... is already registered" in ginac/archive.cpp
class func_unarchiver {
public:
  func_unarchiver();
  ~func_unarchiver();
};
static func_unarchiver func_unarchiver_instance;

// hardcoded functions
// Note: The order here should correspond to the order of REGISTER_FUNCTION
// and VERY IMPORTANT round must be first, because of eval() using round_SERIAL::serial
DECLARE_FUNCTION_2P(round)
DECLARE_FUNCTION_2P(floor)
DECLARE_FUNCTION_2P(ceil)
DECLARE_FUNCTION_3P(sum)
DECLARE_FUNCTION_3P(mindex)
DECLARE_FUNCTION_3P(hadamard)
DECLARE_FUNCTION_1P(transpose)
DECLARE_FUNCTION_2P(vecprod)
DECLARE_FUNCTION_2P(scalprod)
DECLARE_FUNCTION_3P(ifelse)
DECLARE_FUNCTION_1P(vmax)
DECLARE_FUNCTION_1P(vmin)
DECLARE_FUNCTION_2P(concat)
// Matrix functions (as defined in GiNaC matrix.h). Note that diag_matrix() etc. are already declared in namespace GiNaC in matrix.h
DECLARE_FUNCTION_1P(diagmatrix)
DECLARE_FUNCTION_2P(identmatrix)
DECLARE_FUNCTION_2P(onesmatrix)
DECLARE_FUNCTION_5P(submatrix)
DECLARE_FUNCTION_3P(reducematrix)
DECLARE_FUNCTION_1P(determinant)
DECLARE_FUNCTION_1P(trace)
DECLARE_FUNCTION_2P(charpoly)
DECLARE_FUNCTION_1P(rank)
DECLARE_FUNCTION_3P(solvematrix)
DECLARE_FUNCTION_1P(invertmatrix)
DECLARE_FUNCTION_1P(matrixrows)
DECLARE_FUNCTION_1P(matrixcols)

}
#endif
