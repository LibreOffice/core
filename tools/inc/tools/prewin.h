/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: prewin.h,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2007-09-06 14:14:02 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

/* nicht geschuetzt, darf nur einmal includet werden */

#if defined WNT

#define BOOL         WIN_BOOL
#define BYTE         WIN_BYTE
#ifndef VCL_NEED_BASETSD
#define INT64        WIN_INT64
#define UINT64       WIN_UINT64
#define INT32        WIN_INT32
#define UINT32       WIN_UINT32
#endif

#define Rectangle    BLA_Rectangle
#define Polygon      BLA_Polygon
#define PolyPolygon  BLA_PolyPolygon
#define Region       WIN_Region
#define Folder       WIN_Folder
#define GradientStyle_RECT  WIN_GradientStyle_RECT

#ifdef __cplusplus
extern "C" {
#endif

#define STRICT
#ifdef _MSC_VER
#pragma warning(push, 1)
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
