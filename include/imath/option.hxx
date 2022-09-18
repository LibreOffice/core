/***************************************************************************
    option.hxx  -  Header file for class option, for managing option values
                             -------------------
    begin                : Mon May 15 2006
    copyright            : (C) 2006 by Jan Rheinlaender
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
#ifndef OPTSTACK_H
#define OPTSTACK_H

#include <string>
#include <vector>
#include "expression.hxx"
// TODO: How to pull in sal_Bool directly?
#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif

/// Alignment types of equations
enum aligntype {
  none,     // No alignment
  onlyleft, // Ampersand only before operator
  onlyright,// Ampersand only after operator
  both,     // Ampersand on both sides of operator
  left,     // Ampersand on the left, used for addalign()
  right     // Ampersand on the right, used for addalign()
};

/// All the known options
enum option_name {
  o_eqchain,
  o_eqraw,
  o_eqparse,
  o_eqsplit,
  o_eqsplittext,
  o_vecautosize,
  o_eqalign,
  o_precision,
  o_fixeddigits,
  o_exponent,
  o_lowsclimit,
  o_highsclimit,
  o_units, // Required internally by parser
  o_unitstr,
  o_difftype,
  o_underflow,
  o_suppress_units,
  o_evalf_real_roots,
  o_showlabels,
  o_autofraction,
  o_minimumtextsize,
  o_autotextmode,
  o_basefontheight, // Required internally by imathprint
  o_fractionlevel,   // Required internally by imathprint
  o_formulafont,
  o_echoformula,
  o_forceautoformat, // Required internally by parser
  o_implicitmul,
  o_externaleditor
};

/// Possible types of options
enum option_type {
  t_str,
  t_bool,
  t_dbl,
  t_align,
  t_uint,
  t_int,
  t_exvec
};

// struct with union to store the different option values
// TODO: The options values that are pointers don't have proper memory handling yet and may introduce leaks
// Better change the struct to a class structure with subclasses for each type? Or a template class?
typedef union option_value {
  /// String
  std::string* str;
  /// Alignment type
  aligntype align;
  /// Boolean
  bool boolean;
  /// Double
  double dbl;
  /// Unsigned integer
  unsigned uinteger;
  /// Integer
  int integer;
  /// List
  GiNaC::exvector* exvec;
} option_value;

typedef struct option {
  /// The option type, for integrity checking
  option_type type;
  /// The value
  option_value value;
  /// Create from different types
  option()               { value.boolean = false; type = t_bool; };
  option(std::string v)  { value.str = new std::string(v); type = t_str; };
  option(std::string* v) { value.str = v; type = t_str; };
  option(aligntype v)    { value.align = v; type = t_align; };
  option(sal_Bool v)     { value.boolean = v; type = t_bool; };
  option(bool v)         { value.boolean = v; type = t_bool; };
  option(double v)       { value.dbl = v; type = t_dbl; };
  option(unsigned v)     { value.uinteger = v; type = t_uint; };
  option(int v)          { value.integer = v; type = t_int; };
  option(GiNaC::exvector v)     { value.exvec = new GiNaC::exvector(v); type = t_exvec; };
  option(GiNaC::exvector* v)    { value.exvec = v; type = t_exvec; };

  /// Compare two options
  bool operator==(const option& other) const;
  bool operator!=(const option& other) const;
  /// Output the value (of the given type) onto a stream
  void print(std::ostream &os) const;
} option;

#endif
