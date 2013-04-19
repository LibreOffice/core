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

#ifndef _SAL_CONFIG_H_
#define _SAL_CONFIG_H_

#if defined LIBO_INTERNAL_ONLY
#include "config_global.h"
#endif

#include <stdlib.h>

#ifdef WIN32
#define SAL_W32
#define SAL_DLLEXTENSION ".dll"
#define SAL_PRGEXTENSION ".exe"
#define SAL_PATHSEPARATOR ';'
#define SAL_PATHDELIMITER '\\'
#define SAL_CONFIGFILE( name ) name ".ini"

#ifdef _MSC_VER

#ifndef _USE_MATH_DEFINES
#define _USE_MATH_DEFINES // needed by Visual C++ for math constants
#endif

#endif /* defined _MSC_VER */

/* Provide ISO C99 compatible versions of snprint and vsnprintf */
#ifdef __MINGW32__
#define _SNPRINTF_DLLIMPORT
#endif
#ifndef _SNPRINTF_H
#include <systools/win32/snprintf.h>
#endif

#endif /* defined WIN32 */

#if defined(SOLARIS) || defined(LINUX) || defined(NETBSD) || defined(FREEBSD) || \
    defined(AIX) || defined(OPENBSD) || defined(DRAGONFLY) || defined(ANDROID)
#define SAL_UNX
#define SAL_DLLEXTENSION ".so"
#define SAL_DLLPREFIX "lib"
#define SAL_PRGEXTENSION ".bin"
#define SAL_PATHSEPARATOR ':'
#define SAL_PATHDELIMITER '/'
#define SAL_CONFIGFILE( name ) name "rc"
#endif

#ifdef MACOSX
#define SAL_UNX
#define SAL_DLLEXTENSION ".dylib"
#define SAL_DLLPREFIX "lib"
#define SAL_PRGEXTENSION ".bin"
#define SAL_PATHSEPARATOR ':'
#define SAL_PATHDELIMITER '/'
#define SAL_CONFIGFILE( name ) name "rc"
#endif

#ifdef IOS
#define SAL_UNX
/* SAL_DLLEXTENSION should not really be used on iOS, as iOS apps are
 * not allowed to load own dynamic libraries.
 */
#define SAL_DLLEXTENSION ".dylib"
#define SAL_DLLPREFIX "lib"
/* This is fairly pointless too, an iOS app consists of a single
 * executable (plus data files).
 */
#define SAL_PRGEXTENSION ".bin"
#define SAL_PATHSEPARATOR ':'
#define SAL_PATHDELIMITER '/'
#define SAL_CONFIGFILE( name ) name "rc"
#endif

#ifdef sun
#undef sun
#define sun sun
#endif

#endif /*_SAL_CONFIG_H_ */


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
