/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef INCLUDED_SAL_CONFIG_H
#define INCLUDED_SAL_CONFIG_H

#if defined LIBO_INTERNAL_ONLY && defined ANDROID && defined __cplusplus
#include <android/compatibility.hxx>
#endif

#ifdef _WIN32
#define SAL_W32
#define SAL_DLLEXTENSION ".dll"
#define SAL_EXEEXTENSION ".exe"
#define SAL_PATHSEPARATOR ';'
#define SAL_PATHDELIMITER '\\'
#define SAL_NEWLINE_STRING "\r\n"
#define SAL_CONFIGFILE( name ) name ".ini"

#ifdef _MSC_VER

#ifndef _USE_MATH_DEFINES
#define _USE_MATH_DEFINES // needed by Visual C++ for math constants
#endif

#endif /* defined _MSC_VER */

#endif /* defined _WIN32 */

#if defined(__sun) || defined(LINUX) || defined(NETBSD) || defined(FREEBSD) || \
    defined(AIX) || defined(OPENBSD) || defined(DRAGONFLY) || defined(ANDROID) || \
    defined(HAIKU)
#define SAL_UNX
#define SAL_DLLEXTENSION ".so"
#define SAL_EXEEXTENSION ""
#define SAL_DLLPREFIX "lib"
#define SAL_PATHSEPARATOR ':'
#define SAL_PATHDELIMITER '/'
#define SAL_NEWLINE_STRING "\n"
#define SAL_CONFIGFILE( name ) name "rc"
#endif

#ifdef MACOSX
#define SAL_UNX
#define SAL_DLLEXTENSION ".dylib"
#define SAL_EXEEXTENSION ""
#define SAL_DLLPREFIX "lib"
#define SAL_PATHSEPARATOR ':'
#define SAL_PATHDELIMITER '/'
#define SAL_NEWLINE_STRING "\n"
#define SAL_CONFIGFILE( name ) name "rc"
#endif

#ifdef IOS
#define SAL_UNX
/* SAL_DLLEXTENSION should not really be used on iOS, as iOS apps are
 * not allowed to load own dynamic libraries.
 */
#define SAL_DLLEXTENSION ".dylib"
#define SAL_DLLPREFIX "lib"
#define SAL_PATHSEPARATOR ':'
#define SAL_PATHDELIMITER '/'
#define SAL_NEWLINE_STRING "\n"
#define SAL_CONFIGFILE( name ) name "rc"
#endif

/* The following spell is for Solaris and its descendants.
 * See the "Solaris" section of
 * <http://sourceforge.net/p/predef/wiki/OperatingSystems/>, and
 * <http://stackoverflow.com/questions/16618604/solaris-and-preprocessor-macros>.
 */
#ifdef sun
#undef sun
#define sun sun
#endif

#if defined __clang__
#if __has_warning("-Wpotentially-evaluated-expression")
#pragma GCC diagnostic ignored "-Wpotentially-evaluated-expression"
#endif
#endif

#endif // INCLUDED_SAL_CONFIG_H

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
