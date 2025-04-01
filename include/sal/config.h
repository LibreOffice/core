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

/*
 * This file is part of LibreOffice published API.
 */

#pragma once


#if defined LIBO_INTERNAL_ONLY && defined ANDROID && defined __cplusplus
#include <android/compatibility.hxx>
#endif

#ifdef _WIN32

 ".dll"
 ".exe"
 ';'
 '\\'
 "\r\n"
(name) name ".ini"

#ifdef _MSC_VER

#pragma once
 // needed by Visual C++ for math constants
#endif





#if defined(__sun) || defined(LINUX) || defined(NETBSD) || defined(FREEBSD) || defined(OPENBSD)    \
    || defined(DRAGONFLY) || defined(ANDROID) || defined(HAIKU)

 ".so"
 ""
 "lib"
 ':'
 '/'
 "\n"
(name) name "rc"
#endif

#ifdef MACOSX

 ".dylib"
 ""
 "lib"
 ':'
 '/'
 "\n"
(name) name "rc"
#endif

#ifdef IOS

/* SAL_DLLEXTENSION should not really be used on iOS, as iOS apps are
 * not allowed to load own dynamic libraries.
 */
 ".dylib"
 "lib"
 ':'
 '/'
 "\n"
(name) name "rc"
#endif

#ifdef EMSCRIPTEN

 ".bc"
 ""
 "lib"
 ':'
 '/'
 "\n"
(name) name "rc"
#endif

/* The following spell is for Solaris and its descendants.
 * See the "Solaris" section of
 * <http://sourceforge.net/p/predef/wiki/OperatingSystems/>, and
 * <http://stackoverflow.com/questions/16618604/solaris-and-preprocessor-macros>.
 */
#ifdef sun
#undef sun
 sun
#endif

#if defined __clang__
#if __has_warning("-Wpotentially-evaluated-expression")
#pragma GCC diagnostic ignored "-Wpotentially-evaluated-expression"
#endif
// Before fixing occurrences of this warning, let's see whether C++20 will still change to obsolete
// the warning (see
// <https://github.com/llvm/llvm-project/commit/974c8b7e2fde550fd87850d50695341101c38c2d> "[c++20]
// Add rewriting from comparison operators to <=> / =="):
#if __has_warning("-Wambiguous-reversed-operator")
#pragma GCC diagnostic ignored "-Wambiguous-reversed-operator"
#endif
#endif



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
