/*******************************************************
    stringex.cpp  -  Class for extending GiNaC to handle string "expressions"
                             -------------------
    begin                : Fri 06 Apr 2012
    copyright            : (C) 2012 by Jan Rheinlaender
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

#ifdef INSIDE_SM
#include <imath/stringex.hxx>
#include <imath/msgdriver.hxx>
#else
#include "stringex.hxx"
#include "msgdriver.hxx"
#endif

namespace GiNaC {

GINAC_IMPLEMENT_REGISTERED_CLASS_OPT(stringex, basic,
  print_func<print_context>(&stringex::do_print).print_func<imathprint>(&stringex::do_print_imath));

// Required constructors and destructors and other GiNaC-specific methods
  stringex::stringex() {
    MSG_INFO(3, "Constructing empty stringex" << endline);
  }

  stringex::stringex(const std::string &s) : value(s) {
    MSG_INFO(3, "Constructing stringex from " << s << endline);
  }

#ifdef DEBUG_CONSTR_DESTR
  stringex::~stringex() {
    MSG_INFO(3, "Destructing stringex " << *this << endline);
  }

  stringex::stringex(const stringex& other) : value(other.value) {
    MSG_INFO(3, "Copying stringex from " << other << endline);
  }

  stringex& stringex::operator=(const stringex& other) {
    MSG_INFO(3, "Assigning stringex from " << other << endline);
    value = other.value;
    return *this;
  }
#endif

stringex_unarchiver::stringex_unarchiver() {}
stringex_unarchiver::~stringex_unarchiver() {}

  int stringex::compare_same_type(const basic &other) const {
    const stringex &o = static_cast<const stringex &>(other);
    int cmpval = value.compare(o.value);

     if (cmpval == 0)
      return 0;
    else if (cmpval < 0)
      return -1;
    else
      return 1;
  }

  void stringex::do_print(const print_context &c, unsigned level) const {
    (void)level;
    c.s << "'" << value << "'";
  }

  void stringex::do_print_imath(const imathprint &c, unsigned level) const {
    (void)level;
    c.s << "\"" << value << "\"";
  }
}
