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


#ifndef _MDIEXP_HXX
#define _MDIEXP_HXX

#include <tools/solar.h>
#include <tblenum.hxx>
#include <layout/layout.hxx>
#include <swdllapi.h>

class UniString;
class SwRect;
class Size;
class Dialog;
class ViewShell;
class SwDoc;
class SwDocShell;

extern void ScrollMDI(ViewShell* pVwSh, const SwRect &, sal_uInt16 nRangeX, sal_uInt16 nRangeY);
extern sal_Bool IsScrollMDI(ViewShell* pVwSh, const SwRect &);
extern void SizeNotify(ViewShell* pVwSh, const Size &);

//Update der Statusleiste, waehrend einer Action.
extern void PageNumNotify( ViewShell* pVwSh,
                            sal_uInt16 nPhyNum,
                            sal_uInt16 nVirtNum,
                           const UniString& rPg );

enum FlyMode { FLY_DRAG_START, FLY_DRAG, FLY_DRAG_END };
extern void FrameNotify( ViewShell* pVwSh, FlyMode eMode = FLY_DRAG );

SW_DLLPUBLIC void StartProgress    ( sal_uInt16 nMessId, long nStartVal, long nEndVal, SwDocShell *pDocSh = 0 );
SW_DLLPUBLIC void EndProgress      ( SwDocShell *pDocSh = 0 );
SW_DLLPUBLIC void SetProgressState  ( long nPosition, SwDocShell *pDocShell );
void SetProgressText   ( sal_uInt16 nMessId, SwDocShell *pDocShell );
void RescheduleProgress( SwDocShell *pDocShell );

void EnableCmdInterface(sal_Bool bEnable = sal_True);

LAYOUT_NS Dialog* GetSearchDialog();

void RepaintPagePreview( ViewShell* pVwSh, const SwRect& rRect );

// ndgrf.cxx
// alle QuickDraw-Bitmaps des speziellen Docs loeschen
void DelAllGrfCacheEntries( SwDoc* pDoc );

// ChgMode fuer Tabellen aus der Konfiguration lesen
TblChgMode GetTblChgDefaultMode();

sal_Bool JumpToSwMark( ViewShell* pVwSh, const UniString& rMark );

// IAccessible2 Implementation
extern void AccessibilityScrollMDI(ViewShell* pVwSh, const SwRect &, sal_uInt16 nRangeX, sal_uInt16 nRangeY, sal_Bool isLeftTop);

#endif
