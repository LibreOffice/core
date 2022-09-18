/* **************************************************************************
                          msgdriver.hxx  - Convert iMath messages to SAL messages
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

#ifndef _MSGDRIVER_H
#define _MSGDRIVER_H

#include <sal/log.hxx>

#include "iostream"

namespace GiNaC {
    class equation;
    class relational;
}
class operands;

class msg {
private:
    int level;

    static msg msg_error;
    static msg msg_warn;
    static msg msg_info;

  public:
    /*  Check if messages of a certain priority will be printed. Returns true
        if (p % 8) <= the priority level of the stream.
        p > 7 adds parser debug output to the messages of priority (p % 8)
        @param p The priority
        @returns True or false
    */
    inline bool checkprio(const int p) {
        if ((level >> 3) > 0) // i.e. level > 7
            return ((p % 8) <= (level % 8));
        else
            return (p <= level);
    }

    /// Set priority of message stream
    inline void setlevel(const int p) { level = p; }

    // TODO: This is extremely inefficient
    template<typename T> inline msg& operator<<(const T& element) { SAL_WARN("starmath.imath", element); return *this; }

    static inline msg& error() { return msg_error; }
    static inline msg& warn()  { return msg_warn; }
    static inline msg& info()  { return msg_info; }
    static void init();
};

// Note: SAL_INFO appends a newline anyway, so ignore this
// TODO: Change imath code to make this superfluous
inline std::ostream& endline(std::ostream& os) { return os; }

#define MSG_ERROR(priority, output) SAL_WARN_IF(msg::error().checkprio(priority), "starmath.imath", output)
#define MSG_WARN(priority, output)  SAL_WARN_IF(msg::warn( ).checkprio(priority), "starmath.imath", output)
#define MSG_INFO(priority, output)  SAL_INFO_IF(msg::info( ).checkprio(priority), "starmath.imath", output)

namespace GiNaC {
    std::ostream& operator<<(std::ostream& os, const equation& e);
    std::ostream& operator<<(std::ostream& os, const operands& r);
    std::ostream& operator<<(std::ostream& os, const relational& r);
}

#endif
