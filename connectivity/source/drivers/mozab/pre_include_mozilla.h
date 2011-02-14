/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

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

#if defined __GNUC__
    #pragma GCC system_header
#elif defined __SUNPRO_CC
    #pragma disable_warn
#elif defined _MSC_VER
    #pragma warning(push, 1)
    #pragma warning(disable:4946 4710)
#endif

