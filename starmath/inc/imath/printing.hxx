/**************************************************************************
    printing.hxx  -  header file for pretty-printing expressions in Latex format
                             -------------------
    begin                : Sat Mar 2 2002
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
/**
@short This file contains routines for pretty-printing expressions in iMath format.
@author Jan Rheinlaender

The routines in this file are required because setting of parentheses in GiNaC is solved by using precedence()
and level. This approach does not give nice output, because it is impossible to put brackets in the case of
x * (3+4i) but not in the case of x^{3+4i}. In the latter case the {}-brackets are completely sufficient and
x^{(3+4i)} looks ugly. Therefore another approach to bracketing is implemented in this code.
**/

#ifndef PRINTING_H
#define PRINTING_H

#include <iostream>
#include <string>
#include <stdexcept>
#pragma warning(push)
#pragma warning(disable: 4099 4100 4996)
#include <ginac/ginac.h>
#pragma warning(pop)
#include "option.hxx"

namespace GiNaC {

typedef std::map<option_name, option> optionmap;

/// Printing class for iMath
class imathprint : public print_dflt {
  GINAC_DECLARE_PRINT_CONTEXT(imathprint, print_dflt)
public:
  optionmap* poptions; // iMath-specific print options
  bool add_turn_around; // iMath-specific flag
  imathprint(std::ostream & os, optionmap* popt) : print_dflt(os), poptions(popt), add_turn_around(false) {};
  imathprint(std::ostream & os, const imathprint& c);
  void enter_fraction() const; // Reduce the font size for continued fractions
  void exit_fraction() const;
  static void init();
  static std::string decimalpoint;
};

// iMath printing functions
void imathprint_add(const add& a, const imathprint& c, unsigned level);
void imathprint_constant(const constant& cn, const imathprint& c, unsigned level);
void imathprint_exprseq(const exprseq& es, const imathprint& c, unsigned level);
void imathprint_function(const function& f, const imathprint& c, unsigned level);
void imathprint_matrix(const matrix& m, const imathprint& c, unsigned level);
void imathprint_mul(const mul &m, const imathprint& c, unsigned level);
void imathprint_ncmul(const ncmul &m, const imathprint& c, unsigned level);
void imathprint_numeric(const numeric& m, const imathprint& c, unsigned level);
void imathprint_power(const power& p, const imathprint& c, unsigned level);
void imathprint_relational(const relational& r, const imathprint& c, unsigned level);
void imathprint_symbol(const symbol& s, const imathprint& c, unsigned level);
void imathprint_wildcard(const wildcard& w, const imathprint& c, unsigned level);
void imathprint_generic(const basic& b, const imathprint& c, unsigned level);

}
#endif
