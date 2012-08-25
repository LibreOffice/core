/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
// stdafx1.h : include file for standard system include files,
//      or project specific include files that are used frequently,
//      but are changed infrequently

#if !defined(AFX_STDAFX_H__C1799EA0_62CC_44DE_A2DD_C9F0410FF7F1__INCLUDED_)
#define AFX_STDAFX_H__C1799EA0_62CC_44DE_A2DD_C9F0410FF7F1__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define STRICT
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0403
#endif
#define _ATL_APARTMENT_THREADED
#define _ATL_STATIC_REGISTRY

#pragma warning (disable:4505)
    //  globally disable "unreferenced local function has been removed"

#pragma warning (push,1)
#pragma warning (disable:4548)
    //  expression before comma has no effect; expected expression with side-effect
#pragma warning (disable:4555)
    //  expression has no effect; expected expression with side-effect

#define min(a, b)  (((a) < (b)) ? (a) : (b))
#include <atlbase.h>

//You may derive a class from CComModule and use it if you want to override
//something, but do not change the name of _Module
extern CComModule _Module;
#include <atlcom.h>
#include <atlctl.h>

#pragma warning (pop)

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__C1799EA0_62CC_44DE_A2DD_C9F0410FF7F1__INCLUDED)

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
