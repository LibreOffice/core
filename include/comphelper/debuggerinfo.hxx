/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_COMPHELPER_DEBUGINFO_HXX
#define INCLUDED_COMPHELPER_DEBUGINFO_HXX

#include <comphelper/comphelperdllapi.h>

namespace comphelper
{
#if defined DBG_UTIL && !defined NDEBUG
/// Returns true if the process is running with a debugger attached.
///
/// This is useful to disable watchdogs or increase timeouts if they trigger
/// too soon while using a debugger.
COMPHELPER_DLLPUBLIC bool isDebuggerAttached();
#endif

// For detecting whether Valgrind is used, add valgrind to gb_Library_use_externals
// and then in code use:
// #if defined HAVE_VALGRIND_HEADERS
// #include <valgrind/memcheck.h>
// #endif
// ...
// #if defined HAVE_VALGRIND_HEADERS
//        if( RUNNING_ON_VALGRIND )

} // namespace comphelper

#endif // INCLUDED_COMPHELPER_DEBUGINFO_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
