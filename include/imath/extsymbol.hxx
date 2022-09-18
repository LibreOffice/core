/***************************************************************************
    extsymbol.hxx  -  Header file for class extsymbol
                             -------------------
    begin                : Sun May 08 2016
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
@short Extended symbol class that allows to change domain and commutativity of a symbol
@author Jan Rheinlaender
**/

#ifndef EXTSYMBOL_H
#define EXTSYMBOL_H

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning (disable: 4099 4100 4996)
#endif
#include <ginac/symbol.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif

namespace GiNaC {

class extsymbol: public symbol {
private:
  unsigned _domain;
  unsigned _return_type;
public:
  extsymbol();
  explicit extsymbol(const std::string & initname);

#ifdef DEBUG_CONSTR_DESTR
  extsymbol(const extsymbol& other);
  extsymbol& operator=(const extsymbol& other);
  ~extsymbol();
#endif

  inline unsigned get_domain() const { return _domain; }
  inline void set_domain(const unsigned d) { _domain = d; }
  inline void make_complex() { _domain = domain::complex; }
  inline void make_real()    { _domain = domain::real; }
  inline void make_pos()     { _domain = domain::positive; }

  ex conjugate() const;
  ex real_part() const;
  ex imag_part() const;
  using symbol::normal; // Fixes weird linking error on LO internal build

  extsymbol* duplicate() const { return new extsymbol(*this); }

  inline unsigned return_type() const { return _return_type; }
  inline void set_return_type(const unsigned r) { _return_type = r; }
  inline void make_c()  { _return_type = return_types::commutative; }
  inline void make_nc() { _return_type = return_types::noncommutative_composite; }
};

class extsymbol_unarchiver {
public:
  extsymbol_unarchiver();
  ~extsymbol_unarchiver();
};
static extsymbol_unarchiver extsymbol_unarchiver_instance;

}
#endif
