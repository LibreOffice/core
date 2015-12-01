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

// stdafx1.h : include file for standard system include files,
//      or project specific include files that are used frequently,
//      but are changed infrequently

#if !defined(AFX_STDAFX_H__C1799EA0_62CC_44DE_A2DD_C9F0410FF7F1__INCLUDED_)
#define AFX_STDAFX_H__C1799EA0_62CC_44DE_A2DD_C9F0410FF7F1__INCLUDED_

#ifdef _MSC_VER
#pragma once
#endif

#define STRICT
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0502
#endif
#define _ATL_APARTMENT_THREADED
#define _ATL_STATIC_REGISTRY

#pragma warning (push,1)
#pragma warning (disable:4548)
    //  expression before comma has no effect; expected expression with side-effect
#pragma warning (disable:4555)
    //  expression has no effect; expected expression with side-effect
#if defined __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wall"
#pragma clang diagnostic ignored "-Wattributes"
#pragma clang diagnostic ignored "-Wdelete-incomplete"
#pragma clang diagnostic ignored "-Wdynamic-class-memaccess"
#pragma clang diagnostic ignored "-Wint-to-pointer-cast"
#pragma clang diagnostic ignored "-Winvalid-noreturn"
#pragma clang diagnostic ignored "-Wmicrosoft"
#pragma clang diagnostic ignored "-Wnon-pod-varargs"
#pragma clang diagnostic ignored "-Wsequence-point"
#pragma clang diagnostic ignored "-Wtypename-missing"
#endif

#define min(a, b)  (((a) < (b)) ? (a) : (b))
#include <atlbase.h>

//You may derive a class from CComModule and use it if you want to override
//something, but do not change the name of _Module
extern CComModule _Module;
#include <atlcom.h>
#include <atlctl.h>

#if defined __clang__
#pragma clang diagnostic pop
#endif
#pragma warning (pop)

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__C1799EA0_62CC_44DE_A2DD_C9F0410FF7F1__INCLUDED)

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
