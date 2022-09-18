/* **************************************************************************
                          msgdriver.hxx  - Driver class for class message
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

#ifndef _MSGDRIVER_H
#define _MSGDRIVER_H

#include "iostream"
#include "fstream"
#include "message.hxx"

class msg {
  private:
    static message msg_error;
    static message msg_warn;
    static message msg_info;
    static message devnull;
    static std::ofstream devnullstream;
  public:
    static inline message& error() { return msg_error; }
    static inline message& warn() { return msg_warn; }
    static inline message& info() { return msg_info; }
    static  message& error(const int priority);
    static  message& warn(const int priority);
    static  message& info(const int priority);
    static void init();
};

#define MSG_ERROR(priority, output) if (msg::error().checkprio(priority)) msg::error() << output
#define MSG_WARN(priority, output)  if (msg::warn().checkprio(priority)) msg::warn() << output
#define MSG_INFO(priority, output) if (msg::info().checkprio(priority)) msg::info() << output

#endif
