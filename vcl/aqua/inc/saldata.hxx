/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: saldata.hxx,v $
 * $Revision: 1.20 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef _SV_SALDATA_HXX
#define _SV_SALDATA_HXX

#include "premac.h"
#include <Cocoa/Cocoa.h>
#include "postmac.h"

#include "vcl/sv.h"
#include "vcl/svdata.hxx"
#include "vcl/salwtype.hxx"
#include "vcl/ptrstyle.hxx"

#include <list>
#include <vector>
#include <hash_set>

#include <cstdio>
#include <cstdarg>


class AquaSalInstance;
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

class AquaSalFrame;
struct FrameHash : public std::hash<sal_IntPtr>
{
    size_t operator()(const AquaSalFrame* frame) const
    { return std::hash<sal_IntPtr>::operator()( reinterpret_cast<const sal_IntPtr>(frame) ); }
};

#define INVALID_CURSOR_PTR (NSCursor*)0xdeadbeef

struct SalData
{

    SALTIMERPROC                                  mpTimerProc;      // timer callback proc
    AquaSalInstance                              *mpFirstInstance;  // pointer of first instance
    std::list<AquaSalFrame*>                      maFrames;         // pointer of first frame
    std::hash_set<const AquaSalFrame*,FrameHash>  maFrameCheck;     // for fast check of frame existance
    SalObject                                    *mpFirstObject;    // pointer of first object window
    SalVirtualDevice                             *mpFirstVD;        // first VirDev
    SalPrinter                                   *mpFirstPrinter;   // first printing printer
    SystemFontList                               *mpFontList;

    CGColorSpaceRef                               mxRGBSpace;
    CGColorSpaceRef                               mxGraySpace;

    std::vector< NSCursor* >                      maCursors;
    std::vector< NSMenuItem* >                    maFallbackMenu;

    static oslThreadKey                           s_aAutoReleaseKey;

    bool              mbIsScrollbarDoubleMax;   // TODO: support DoubleMin and DoubleBoth too

    SalData();
    ~SalData();

    NSCursor* getCursor( PointerStyle i_eStyle );

    static void ensureThreadAutoreleasePool();
    static void drainThreadAutoreleasePool();
};

inline void SetSalData( SalData* pData ) { ImplGetSVData()->mpSalData = (void*)pData; }
inline SalData *GetSalData() { return (SalData*)ImplGetSVData()->mpSalData; }
inline SalData *GetAppSalData() { return (SalData*)ImplGetAppSVData()->mpSalData; }

// --- Prototypes ---

BOOL ImplSalYieldMutexTryToAcquire();
void ImplSalYieldMutexAcquire();
void ImplSalYieldMutexRelease();

#endif  // _SV_SALDATA_HXX
