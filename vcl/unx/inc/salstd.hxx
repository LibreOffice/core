/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: salstd.hxx,v $
 * $Revision: 1.6 $
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

#ifndef _SALSTD_HXX
#define _SALSTD_HXX

// -=-= includes -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
#include <tools/ref.hxx>
#include <tools/string.hxx>
#include <tools/gen.hxx>
#include <vcl/sv.h>

// -=-= X-Lib forwards -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
#ifndef _SVUNX_H
typedef unsigned long       Pixel;
typedef unsigned long       XID;
typedef unsigned long       XLIB_Time;
typedef unsigned long       XtIntervalId;

typedef XID                 Colormap;
typedef XID                 Drawable;
typedef XID                 Pixmap;
typedef XID                 XLIB_Cursor;
typedef XID                 XLIB_Font;
typedef XID                 XLIB_Window;

typedef struct  _XDisplay   Display;
typedef struct  _XGC       *GC;
typedef struct  _XImage     XImage;
typedef struct  _XRegion   *XLIB_Region;

typedef union   _XEvent     XEvent;

typedef struct  _XConfigureEvent    XConfigureEvent;
typedef struct  _XReparentEvent     XReparentEvent;
typedef struct  _XClientMessageEvent        XClientMessageEvent;
typedef struct  _XErrorEvent        XErrorEvent;

struct  Screen;
struct  Visual;
struct  XColormapEvent;
struct  XFocusChangeEvent;
struct  XFontStruct;
struct  XKeyEvent;
struct  XPropertyEvent;
struct  XTextItem;
struct  XWindowChanges;

#define None    0L
#endif

#endif

