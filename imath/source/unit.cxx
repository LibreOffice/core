/*******************************************************
    unit.cpp  -  Class for extending GiNaC to handle physical units
                             -------------------
    begin                : Mon Oct 22 2001
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

#ifdef INSIDE_SM
#include <imath/unit.hxx>
#include <imath/msgdriver.hxx>
#include <imath/printing.hxx>
#else
#include "unit.hxx"
#include "msgdriver.hxx"
#include "printing.hxx"
#endif

namespace GiNaC {

GINAC_IMPLEMENT_REGISTERED_CLASS_OPT(Unit, basic, print_func<print_context>(&Unit::do_print).print_func<imathprint>(&Unit::do_print_imath));

// Required constructors and destructors and other GiNaC-specific methods
  Unit::Unit() {
    MSG_INFO(3, "Constructing empty unit" << endline);
  }

  Unit::Unit(const std::string& n, const std::string& pn, const expression& cex) : name(n), printname(pn), canonical(cex) {
#ifndef _MSC_VER
    // Build with MSVC crashes on outputting cex = numeric(0) when initialising the base SI units
    MSG_INFO(3, "Constructing unit with name " << n << " and printname " << pn << " and expression " << ex(cex) << endline);
#endif
  }

#ifdef DEBUG_CONSTR_DESTR
  Unit::~Unit() {
    MSG_INFO(3, "Destructing unit " << name << endline);
  }

  Unit::Unit(const Unit& other) : name(other.name), printname(other.printname), canonical(other.canonical) {
    MSG_INFO(3, "Copying unit from " << other.name << endline);
  }

  Unit& Unit::operator=(const Unit& other) {
    MSG_INFO(3, "Assigning unit from " << other << endline);
    name = other.name;
    printname = other.printname;
    canonical = other.canonical;
    return *this;
  }
#endif

Unit_unarchiver::Unit_unarchiver() {}
Unit_unarchiver::~Unit_unarchiver() {}

  int Unit::compare_same_type(const basic &other) const {
    const Unit &o = static_cast<const Unit &>(other);
    int cmpval = name.compare(o.name);

     if (cmpval == 0)
      return 0;
    else if (cmpval < 0)
      return -1;
    else
      return 1;
  }

  void Unit::do_print(const print_context &c, unsigned level) const {
    (void)level;
    c.s << "[" << (printname == "" ? name : printname) << "]";
  }

  void Unit::do_print_imath(const imathprint &c, unsigned level) const {
    (void)level;
    std::string thename = (printname == "" ? name : printname);

    if (thename[0] == '{') { // smath unit definition
      c.s << thename;
    } else { // protect the name with quotes
      c.s << "\"" + thename + "\"";
    }
  }

bool Unit::info(unsigned inf) const {
  switch (inf) {
    case info_flags::symbol:
    case info_flags::polynomial:
    case info_flags::integer_polynomial:
    case info_flags::cinteger_polynomial:
    case info_flags::rational_polynomial:
    case info_flags::crational_polynomial:
    case info_flags::rational_function:
    case info_flags::expanded:
            return true;
    case info_flags::real:
    case info_flags::positive:
    case info_flags::nonnegative:
            return true;
    case info_flags::has_indices:
            return false;
  }
  return inherited::info(inf);
}

expression Unit::get_canonical() const {
  if (canonical.is_equal(_expr0))
    return *this;
  else
    return canonical;
}

bool Unit::is_base() const {
  return canonical.is_equal(_expr0);
}

}
