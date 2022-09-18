/*************************************************************************
    extintegral.hxx  -  class extintegral, extending the GiNaC integral class
                             -------------------
    begin                : Sun Jan 26 2014
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

#ifndef EXTINTEGRAL_H
#define EXTINTEGRAL_H

#include <typeinfo>
#include <ginac/integral.h>
#include "printing.hxx"

namespace GiNaC {

class extintegral : public integral
{
  GINAC_DECLARE_REGISTERED_CLASS(extintegral, integral)

  // other constructors
public:
  extintegral(const ex & x_, const ex & a_, const ex & b_, const ex & f_);
  extintegral(const ex & x_, const ex & f_, const ex & C_);
  extintegral(const ex & f__, const ex & C_);
  extintegral(const integral& other);

#ifdef DEBUG_CONSTR_DESTR
  extintegral(const extintegral& other);
  extintegral& operator=(const extintegral& other);
  ~extintegral();
#endif

  // functions overriding virtual functions from base classes
public:
  inline unsigned precedence() const override {return 45;}
#if (((GINACLIB_MAJOR_VERSION == 1) && (GINACLIB_MINOR_VERSION >= 7)) || (GINACLIB_MAJOR_VERSION >= 1))
  ex eval() const override;
  ex evalf() const override;
#else
  ex eval(int level=0) const;
  ex evalf(int level=0) const;
#endif
  ex evalm() const override;
  ex subs(const exmap & m, unsigned options = 0) const override;
  int degree(const ex & s) const override;
  int ldegree(const ex & s) const override;
  ex expand(unsigned options = 0) const override;
  ex conjugate() const override;
  ex eval_integ() const override;
public:
  void set_integration_constant(const symbol& newconstant);
protected:
  ex derivative(const symbol & s) const override;

  // non-virtual functions in this class
protected:
  void do_print(const print_context & c, unsigned level) const;
  void do_print_imath(const imathprint& c, unsigned level) const;
private:
  /// The integral has boundaries defined
  bool hasboundaries;
  /// The integration constant, for integrals without boundaries
  ex C;
};

class extintegral_unarchiver {
public:
  extintegral_unarchiver();
  ~extintegral_unarchiver();
};
static extintegral_unarchiver extintegral_unarchiver;

}

#endif
