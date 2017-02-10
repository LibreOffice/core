/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
#if !defined(AFX_STDAFX_H_)
#define AFX_STDAFX_H_

#ifdef _MSC_VER
#pragma once
#endif

#define STRICT
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0502
#endif
#define _ATL_APARTMENT_THREADED
// #define _ATL_STATIC_REGISTRY

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(push, 1)
#pragma warning(disable: 4548)
#if defined __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wall"
#pragma clang diagnostic ignored "-Wattributes"
#pragma clang diagnostic ignored "-Wdelete-incomplete"
#pragma clang diagnostic ignored "-Wdynamic-class-memaccess"
#pragma clang diagnostic ignored "-Wextra"
#pragma clang diagnostic ignored "-Wint-to-pointer-cast"
#pragma clang diagnostic ignored "-Winvalid-noreturn"
#pragma clang diagnostic ignored "-Wmicrosoft"
#pragma clang diagnostic ignored "-Wnon-pod-varargs"
#pragma clang diagnostic ignored "-Wnon-virtual-dtor"
#pragma clang diagnostic ignored "-Wnonportable-include-path"
#pragma clang diagnostic ignored "-Wsequence-point"
#pragma clang diagnostic ignored "-Wtypename-missing"
#endif
#endif
#include <atlbase.h>
//You may derive a class from CComModule and use it if you want to override
//something, but do not change the name of _Module
extern CComModule _Module;
#include <atlcom.h>
#include <atlctl.h>

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#ifdef _MSC_VER
#if defined __clang__
#pragma clang diagnostic pop
#endif
#pragma warning(pop)
#pragma warning(pop)
#endif

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
