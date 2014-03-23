/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#ifndef _PREX_H
#define _PREX_H

#define Window      XLIB_Window
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

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/StringDefs.h>
#include <X11/extensions/Xrender.h>
#include <X11/XKBlib.h>

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

