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
