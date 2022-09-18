/* **************************************************************************
                          msgdriver.cxx  - Convert iMath messages to SAL messages
                             -------------------
    begin               : Sat Oct 23 17:00:00 CEST 2021
    copyright           : (C) 2021 by Jan Rheinlaender
    email               : jrheinlaender@users.sourceforge.net
 ***************************************************************************/

/* **************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify *
 *   it under the terms of the GNU General Public License as published by *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <sstream>
#ifdef INSIDE_SM
#include <imath/msgdriver.hxx>
#include <imath/equation.hxx>
#else
#include "msgdriver.hxx"
#include "equation.hxx"
#endif
#include "operands.hxx"

// Define static members
msg msg::msg_error = msg();
msg msg::msg_warn = msg();
msg msg::msg_info = msg();

void msg::init() {
  msg_error.level = 1;
  msg_warn.level = 1;
  msg_info.level = -1;
}

namespace GiNaC {
    std::ostream& operator<<(std::ostream& os, const equation& e) {
        e.print(print_dflt(os));
        return os;
    }

    std::ostream& operator<<(std::ostream& os, const operands& o) {
        o.print(os);
        return os;
    }

    std::ostream& operator<<(std::ostream& os, const relational& r) {
        r.print(print_dflt(os));
        return os;
    }
}
