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

#ifndef _PREX_H
#define _PREX_H

/* Types from <X11/X.h> that clash with LO's identifiers
 * and we don't need.
 */
#define Boolean     HIDE_XLIB_Boolean
#define Font        HIDE_XLIB_Font
#define Icon        HIDE_XLIB_Icon
#define String      HIDE_XLIB_String

/* Types from <X11/X.h> that clash, but we do use. */
#define Cursor      XLIB_Cursor
#define KeyCode     XLIB_KeyCode
#define Region      XLIB_Region
#define Time        XLIB_Time
#define Window      XLIB_Window

#if defined __cplusplus
extern "C" {
#endif

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/StringDefs.h>
#include <X11/extensions/Xrender.h>
#include <X11/XKBlib.h>

/* From <X11/Intrinsic.h> */
typedef unsigned long Pixel;

#undef  String
#define String      XLIB_String

#undef  KeyCode
#define KeyCode         XLIB_KeyCode //undef in intrinsics

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
