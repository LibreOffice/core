/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: prex.h,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2008-04-02 09:46:03 $
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

#ifndef _PREX_H
#define _PREX_H

#define Window      XLIB_Window
#define BYTE        XLIB_BYTE
#define INT8        XLIB_INT8
#define INT64       XLIB_INT64
#define BOOL        XLIB_BOOL
#define Font        XLIB_Font
#define Cursor      XLIB_Cursor
#define String      XLIB_String
#define KeyCode     XLIB_KeyCode
#define Region      XLIB_Region
#define Icon        XLIB_Icon
#define Time        XLIB_Time
#define Region      XLIB_Region
#define Boolean     XLIB_Boolean

#if defined __cplusplus
extern "C" {
#endif

#if defined(LINUX) || defined(FREEBSD) || defined(MACOSX) // should really check for xfree86 or for X11R6.1 and higher
#define __XKeyboardExtension__ 1
#else
#define __XKeyboardExtension__ 0
#endif

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/StringDefs.h>
#include <X11/extensions/Xrender.h>
#if __XKeyboardExtension__
#include <X11/XKBlib.h>
#endif
typedef unsigned long Pixel;

#undef  DestroyAll
#define DestroyAll      XLIB_DestroyAll
#define XLIB_DestroyAll 0
#undef  String
#define String          XLIB_String

#undef  KeyCode
#define KeyCode         XLIB_KeyCode //undef in intrinsics

#define __Ol_OlXlibExt_h__

#endif

