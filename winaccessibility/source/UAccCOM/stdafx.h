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

#if !defined(AFX_STDAFX_H__5E2F9072_190F_47C7_8003_4D44FD308CB9__INCLUDED_)
#define AFX_STDAFX_H__5E2F9072_190F_47C7_8003_4D44FD308CB9__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//#define _ATL_APARTMENT_THREADED

#include <prewin.h>
#include <windows.h>

#include <atlbase.h>
// You may derive a class from CComModule and use it if you want to override
// something, but do not change the name of _Module
extern CComModule _Module;
#include <atlcom.h>
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

#endif // !defined(AFX_STDAFX_H__5E2F9072_190F_47C7_8003_4D44FD308CB9__INCLUDED)

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
