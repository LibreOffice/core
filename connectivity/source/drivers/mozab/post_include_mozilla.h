/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#if defined __SUNPRO_CC
#pragma enable_warn
#elif defined _MSC_VER
#pragma warning(pop)
#endif

#ifdef MOZ_BOOL
# undef BOOL
# undef Bool
#endif

#ifdef DEBUG_WAS_DEFINED
    #define DEBUG DEBUG_WAS_DEFINED
#endif

#ifdef _DEBUG_WAS_DEFINED
    #define _DEBUG _DEBUG_WAS_DEFINED
#endif

#ifndef _CONNECTIVITY_MOZILLA_REINTERPRET_CAST_MINGW_ONLY_
#define _CONNECTIVITY_MOZILLA_REINTERPRET_CAST_MINGW_ONLY_

#ifdef __cplusplus
#ifdef __MINGW32__
template<class T1, class T2> T1 inline reinterpret_cast_mingw_only(T2 p) { return reinterpret_cast<T1>(p); }
#else
template<class T1, class T2> T1 inline reinterpret_cast_mingw_only(T2 p) { return p; }
#endif
#endif

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
