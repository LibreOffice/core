/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef _SALSTD_HXX
#define _SALSTD_HXX

#include <tools/gen.hxx>
#include <tools/solar.h>

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
