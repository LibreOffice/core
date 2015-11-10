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
#ifndef INCLUDED_SW_INC_MDIEXP_HXX
#define INCLUDED_SW_INC_MDIEXP_HXX

#include <rtl/ustring.hxx>
#include <tools/solar.h>
#include <tblenum.hxx>
#include <swdllapi.h>

class SwRect;
class Size;
class SwViewShell;
class SwDoc;
class SwDocShell;

extern void ScrollMDI(SwViewShell* pVwSh, const SwRect &, sal_uInt16 nRangeX, sal_uInt16 nRangeY);
extern bool IsScrollMDI(SwViewShell* pVwSh, const SwRect &);
extern void SizeNotify(SwViewShell* pVwSh, const Size &);

// Update of status bar during an action.
extern void PageNumNotify( SwViewShell* pVwSh,
                            sal_uInt16 nPhyNum,
                            sal_uInt16 nVirtNum,
                           const OUString& rPg );

enum FlyMode { FLY_DRAG_START, FLY_DRAG, FLY_DRAG_END };
extern void FrameNotify( SwViewShell* pVwSh, FlyMode eMode = FLY_DRAG );

SW_DLLPUBLIC void StartProgress    ( sal_uInt16 nMessId, long nStartVal, long nEndVal, SwDocShell *pDocSh = nullptr );
SW_DLLPUBLIC void EndProgress      ( SwDocShell *pDocSh = nullptr );
SW_DLLPUBLIC void SetProgressState  ( long nPosition, SwDocShell *pDocShell );
void SetProgressText   ( sal_uInt16 nMessId, SwDocShell *pDocShell );
void RescheduleProgress( SwDocShell *pDocShell );

void RepaintPagePreview( SwViewShell* pVwSh, const SwRect& rRect );

// Read ChgMode for tables from configuration.
TableChgMode GetTableChgDefaultMode();

bool JumpToSwMark( SwViewShell* pVwSh, const OUString& rMark );

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
