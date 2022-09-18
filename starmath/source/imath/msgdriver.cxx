/* **************************************************************************
                          msgdriver.cpp  - Driver class for class message
                             -------------------
    begin               : Thu Jun 5 10:33:44 CEST 2008
    copyright           : (C) 2008 by Jan Rheinlaender
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

//#include "../config/config.h" // *** added in 1.3.1
#include <sstream>
#include "msgdriver.hxx"

// Define static members
message msg::msg_error(t_msg_error, 1);
message msg::msg_warn(t_msg_warn, 1);
message msg::msg_info(t_msg_info, -1);
message msg::devnull(devnullstream, t_msg_info, 0);
std::ofstream msg::devnullstream;

inline message& msg::error(const int priority) {
  if (msg_error.checkprio(priority))
    return msg_error;
  else
    return devnull;
}

inline message& msg::warn(const int priority) {
  if (msg_warn.checkprio(priority))
    return msg_warn;
  else
    return devnull;
}

inline message& msg::info(const int priority) {
  if (msg_info.checkprio(priority))
    return msg_info;
  else
    return devnull;
}

void msg::init() {
  msg_error = message(t_msg_error, 1);
  msg_warn = message(t_msg_warn, 1);
  msg_info = message(t_msg_info, -1);
  devnullstream.open("/dev/null");
  devnull = message(devnullstream, t_msg_info, 0);
}

