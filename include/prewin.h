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

/* nicht geschuetzt, darf nur einmal includet werden */

#if defined WNT

#define Rectangle    BLA_Rectangle
#define Polygon      BLA_Polygon
#define PolyPolygon  BLA_PolyPolygon
#define Region       WIN_Region
#define Folder       WIN_Folder
#define GradientStyle_RECT  WIN_GradientStyle_RECT

#ifdef __cplusplus
extern "C" {
#endif

#if !defined STRICT
#define STRICT
#endif
#ifdef _MSC_VER
#pragma warning(push, 1)
#pragma warning (disable: 4005)
#endif
#include <windows.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif
#include <shellapi.h>
#include <commdlg.h>
#include <dlgs.h>
#ifdef USE_TOOLHELP
#include <tlhelp32.h>
#endif
#ifdef USE_IMAGEHLP
#include <imagehlp.h>
#endif
#ifdef INCLUDE_MMSYSTEM
#include <mmsystem.h>
#endif
#ifdef _MSC_VER
#pragma warning(push, 1)
#endif
#include <commctrl.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
