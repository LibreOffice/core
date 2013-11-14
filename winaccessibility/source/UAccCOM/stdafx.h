/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/

// stdafx.h : include file for standard system include files,
//      or project specific include files that are used frequently,
//      but are changed infrequently

#if !defined(AFX_STDAFX_H__5E2F9072_190F_47C7_8003_4D44FD308CB9__INCLUDED_)
#define AFX_STDAFX_H__5E2F9072_190F_47C7_8003_4D44FD308CB9__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define STRICT
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0400
#endif
//#define _ATL_APARTMENT_THREADED

#include <atlbase.h>
// You may derive a class from CComModule and use it if you want to override
// something, but do not change the name of _Module
extern CComModule _Module;
#include <atlcom.h>

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

#include <windows.h>
#undef OPAQUE
#include "CheckEnableAccessible.h"
//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__5E2F9072_190F_47C7_8003_4D44FD308CB9__INCLUDED)
