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

// stdafx.h : include file for standard system include files,
//      or project specific include files that are used frequently,
//      but are changed infrequently

#pragma once

#pragma once

// this turns off ATL's locking in the COM component implementations
// (we don't need it since we use SolarMutex instead)
#define _ATL_APARTMENT_THREADED

#include <prewin.h>
#if !defined WIN32_LEAN_AND_MEAN
# define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

#if defined __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wall"
#pragma clang diagnostic ignored "-Wattributes"
#pragma clang diagnostic ignored "-Wdelete-incomplete"
#pragma clang diagnostic ignored "-Wextra"
#pragma clang diagnostic ignored "-Wint-to-pointer-cast"
#pragma clang diagnostic ignored "-Winvalid-noreturn"
#pragma clang diagnostic ignored "-Wmicrosoft"
#pragma clang diagnostic ignored "-Wnon-pod-varargs"
#pragma clang diagnostic ignored "-Wnon-virtual-dtor"
#endif
#include <atlbase.h>
// You may derive a class from CComModule and use it if you want to override
// something, but do not change the name of _Module
extern CComModule _Module;
#include <atlcom.h>
#if defined __clang__
#pragma clang diagnostic pop
#endif
#include <postwin.h>
#undef OPAQUE


#define ENTER_PROTECTED_BLOCK   \
    try                         \
    {

#define LEAVE_PROTECTED_BLOCK   \
    }                           \
    catch(...)                  \
    {                           \
        return E_FAIL;          \
    }

#define SAFE_SYSFREESTRING(x) { ::SysFreeString(x); x=NULL; }

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
