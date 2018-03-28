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


#ifndef _GLOBALS_HXX_
#define _GLOBALS_HXX_

#include <svpm.h>

#include <rtl/unload.h>

#define OS2_DNDSOURCE_SERVICE_NAME  "com.sun.star.datatransfer.dnd.OleDragSource"
#define OS2_DNDSOURCE_IMPL_NAME  "com.sun.star.comp.datatransfer.dnd.OleDragSource_V1"

#define OS2_DNDTARGET_SERVICE_NAME  "com.sun.star.datatransfer.dnd.OleDropTarget"
#define OS2_DNDTARGET_IMPL_NAME  "com.sun.star.comp.datatransfer.dnd.OleDropTarget_V1"

// from saldata.hxx: these offsets are hardcoded in dnd/globals.hxx too to avoid vcl dependancies
#define SAL_FRAME_DROPTARGET            sizeof(ULONG)
#define SAL_FRAME_DRAGSOURCE            (sizeof(ULONG)*2)
// from saldata.hxx: drag&drop internal messages (see dnd/globals.hxx)
#define DM_AOO_ENDCONVERSATION      (WM_USER+170)

extern rtl_StandardModuleCount g_moduleCount;

#if OSL_DEBUG_LEVEL>0
extern "C" int debug_printf(const char *f, ...);
#else
#define debug_printf( ...)
#endif

MRESULT OfficeToSystemDragActions( sal_Int8 dragActions);
sal_Int8 SystemToOfficeDragActions( USHORT usOperation);

extern "C" MRESULT EXPENTRY dndFrameProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);

// private exports from saldata.hxx
void SetWindowDropTargetPtr( HWND hWnd, void* dt);
void* GetWindowDropTargetPtr( HWND hWnd);
void SetWindowDragSourcePtr( HWND hWnd, void* ds);
void* GetWindowDragSourcePtr( HWND hWnd);

void MapWindowPoint( HWND, PDRAGINFO, PPOINTL);

#endif
