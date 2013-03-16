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

#ifndef _SV_SALDATA_HXX
#define _SV_SALDATA_HXX

#include "premac.h"
#include <UIKit/UIKit.h>
#include "postmac.h"

#include "com/sun/star/uno/Reference.hxx"

#include "vcl/ptrstyle.hxx"

#include "svdata.hxx"
#include "salwtype.hxx"

#include <list>
#include <vector>
#include <map>
#include <boost/unordered_set.hpp>

#include <cstdio>
#include <cstdarg>

class IosSalInstance;
class SalObject;
class SalFrame;
class SalVirtualDevice;
class SalPrinter;
class SystemFontList;

// ------------------
// - Some constants -
// ------------------

#define SAL_CLIPRECT_COUNT 16

// -----------
// - SalData -
// -----------

class IosSalFrame;
struct FrameHash : public boost::hash<sal_IntPtr>
{
    size_t operator()(const IosSalFrame* frame) const
    { return boost::hash<sal_IntPtr>::operator()( reinterpret_cast<const sal_IntPtr>(frame) ); }
};

class SalData
{
public:
    SALTIMERPROC                                 mpTimerProc;       // timer callback proc
    IosSalInstance                              *mpFirstInstance;   // pointer of first instance
    std::list<IosSalFrame*>                      maFrames;          // pointer of first frame
    boost::unordered_set<const IosSalFrame*,FrameHash>  maFrameCheck;     // for fast check of frame existance
    SalObject                                    *mpFirstObject;    // pointer of first object window
    SalVirtualDevice                             *mpFirstVD;        // first VirDev
    SalPrinter                                   *mpFirstPrinter;   // first printing printer
    SystemFontList                               *mpFontList;

    CGColorSpaceRef                               mxRGBSpace;
    CGColorSpaceRef                               mxGraySpace;
    CGColorSpaceRef                               mxP50Space;
    CGPatternRef                                  mxP50Pattern;

    std::vector< UIMenuItem* >                    maFallbackMenu;

    static oslThreadKey                           s_aAutoReleaseKey;

    long                                          mnDPIX;           // #i100617# read DPI only once per office life
    long                                          mnDPIY;           // #i100617# read DPI only once per office life

    com::sun::star::uno::Reference< com::sun::star::uno::XInterface >
                                                  mxClipboard;

    SalData();
    ~SalData();

    static void ensureThreadAutoreleasePool();
    static void drainThreadAutoreleasePool();
};

inline void SetSalData( SalData* pData ) { ImplGetSVData()->mpSalData = pData; }
inline SalData *GetSalData() { return ImplGetSVData()->mpSalData; }

// --- Prototypes ---

sal_Bool ImplSalYieldMutexTryToAcquire();
void ImplSalYieldMutexAcquire();
void ImplSalYieldMutexRelease();

#endif  // _SV_SALDATA_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
