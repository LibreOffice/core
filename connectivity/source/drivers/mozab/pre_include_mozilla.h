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

#include "sal/config.h"

#ifndef MINIMAL_PROFILEDISCOVER
    // Turn off DEBUG Assertions
    #ifdef _DEBUG
        #define _DEBUG_WAS_DEFINED _DEBUG
        #ifndef MOZILLA_ENABLE_DEBUG
            #undef _DEBUG
        #endif
    #else
        #undef _DEBUG_WAS_DEFINED
        #ifdef MOZILLA_ENABLE_DEBUG
            #define _DEBUG 1
        #endif
    #endif

    // and turn off the additional virtual methods which are part of some interfaces when compiled
    // with debug
    #ifdef DEBUG
        #define DEBUG_WAS_DEFINED DEBUG
        #ifndef MOZILLA_ENABLE_DEBUG
            #undef DEBUG
        #endif
    #else
        #undef DEBUG_WAS_DEFINED
        #ifdef MOZILLA_ENABLE_DEBUG
            #define DEBUG 1
        #endif
    #endif
#endif

#if HAVE_GCC_PRAGMA_DIAGNOSTIC_MODIFY && HAVE_GCC_PRAGMA_DIAGNOSTIC_SCOPE \
    && !defined __clang__
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wnon-virtual-dtor"
#elif defined __SUNPRO_CC
    #pragma disable_warn
#elif defined _MSC_VER
    #pragma warning(push, 1)
    #pragma warning(disable:4946 4710)
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
