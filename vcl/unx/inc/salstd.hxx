/*************************************************************************
 *
 *  $RCSfile: salstd.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:05:41 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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

// -=-= #defines -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
#ifndef STDAPI
#define STDAPI(Class)   int     operator != ( const Class& ) const; \
                        int     operator == ( const Class& ) const; \
                        Class  &operator = ( const Class& ); \
                                Class( const Class& );
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

