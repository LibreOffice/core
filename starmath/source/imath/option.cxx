/***************************************************************************
    optstack.cpp  -  Class for managing options
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

#include <sstream>
#include "option.hxx"
#include "msgdriver.hxx"
#include <cmath>

bool option::operator==(const option& other) const {
  if (type != other.type) return false;

  switch (type) {
    case t_str: return *value.str == *other.value.str;
    case t_bool: return value.boolean == other.value.boolean;
    case t_dbl: {
      return (fabs(value.dbl - other.value.dbl) < 1E-10); // TODO: What limit is useful?
    }
    case t_align: return value.align == other.value.align;
    case t_uint: return value.uinteger == other.value.uinteger;
    case t_int: return value.integer == other.value.integer;
    case t_exvec: return (*value.exvec) == (*other.value.exvec);
  default:
    return false;
  }
}

bool option::operator!=(const option& other) const {
  return !(*this == other);
}

void option::print(std::ostream &os) const {
  switch (type) {
    case t_bool: os << (value.boolean ? "true" : "false"); break;
    case t_dbl:  os << value.dbl; break;
    case t_align: os << value.align; break;
    case t_uint: os << value.uinteger; break;
    case t_int: os << value.integer; break;
    case t_str: os << *value.str; break;
    case t_exvec: os << *value.exvec; break;
    default: throw std::invalid_argument("Internal error: Unknown option type");
  }
}

