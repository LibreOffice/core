/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: salstd.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 12:47:04 $
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

#ifndef _SALSTD_HXX
#define _SALSTD_HXX

// -=-= includes -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
#ifndef _REF_HXX
#include <tools/ref.hxx>
#endif
#ifndef _STRING_HXX
#include <tools/string.hxx>
#endif
#ifndef _GEN_HXX
#include <tools/gen.hxx>
#endif
#ifndef _SV_SV_H
#include <sv.h>
#endif

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

