#if !defined(AFX_STDAFX_H_)
#define AFX_STDAFX_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define STRICT
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0403
#endif
#define _ATL_APARTMENT_THREADED
// #define _ATL_STATIC_REGISTRY

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(push, 1)
#pragma warning(disable: 4548)
#pragma warning(disable: 4505)
#endif
#include <atlbase.h>
//You may derive a class from CComModule and use it if you want to override
//something, but do not change the name of _Module
extern CComModule _Module;
#ifdef __MINGW32__
#include <algorithm>
using ::std::min;
using ::std::max;
#endif
#include <atlcom.h>
#include <atlctl.h>

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#ifdef _MSC_VER
#pragma warning(pop)
#pragma warning(pop)
#endif

#endif

