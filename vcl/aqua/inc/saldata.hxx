/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

#include "com/sun/star/uno/Reference.hxx"

#include "vcl/sv.h"
#include "vcl/svdata.hxx"
#include "vcl/salwtype.hxx"
#include "vcl/ptrstyle.hxx"

#include <list>
#include <vector>
#include <map>
#include <boost/unordered_set.hpp>

#include <cstdio>
#include <cstdarg>

#include "apple_remote/RemoteMainController.h"

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

#define VER_TIGER   0x1040
#define VER_LEOPARD 0x1050

// -----------
// - SalData -
// -----------

class AquaSalFrame;
struct FrameHash : public boost::hash<sal_IntPtr>
{
    size_t operator()(const AquaSalFrame* frame) const
    { return boost::hash<sal_IntPtr>::operator()( reinterpret_cast<const sal_IntPtr>(frame) ); }
};

#define INVALID_CURSOR_PTR (NSCursor*)0xdeadbeef

struct SalData
{

    SALTIMERPROC                                  mpTimerProc;      // timer callback proc
    AquaSalInstance                              *mpFirstInstance;  // pointer of first instance
    std::list<AquaSalFrame*>                      maFrames;         // pointer of first frame
    boost::unordered_set<const AquaSalFrame*,FrameHash>  maFrameCheck;     // for fast check of frame existance
    SalObject                                    *mpFirstObject;    // pointer of first object window
    SalVirtualDevice                             *mpFirstVD;        // first VirDev
    SalPrinter                                   *mpFirstPrinter;   // first printing printer
    SystemFontList                               *mpFontList;
    NSStatusItem*                                 mpStatusItem;     // one status item that draws all our stati
                                                                    // at the moment this is only one add menu button

    CGColorSpaceRef                               mxRGBSpace;
    CGColorSpaceRef                               mxGraySpace;
    CGColorSpaceRef                               mxP50Space;
    CGPatternRef                                  mxP50Pattern;

    std::vector< NSCursor* >                      maCursors;
    std::vector< NSMenuItem* >                    maFallbackMenu;
    std::map< NSEvent*, bool >                    maKeyEventAnswer;

    static oslThreadKey                           s_aAutoReleaseKey;

    bool                                          mbIsScrollbarDoubleMax;   // TODO: support DoubleMin and DoubleBoth too
    SInt32                                        mnSystemVersion;          // Store System Version
    MainController*                               mpMainController;         // Apple Remote

    NSObject*                                     mpDockIconClickHandler;
    long                                          mnDPIX;           // #i100617# read DPI only once per office life
    long                                          mnDPIY;           // #i100617# read DPI only once per office life

    com::sun::star::uno::Reference< com::sun::star::uno::XInterface >
                                                  mxClipboard;

    SalData();
    ~SalData();

    NSCursor* getCursor( PointerStyle i_eStyle );

    static void ensureThreadAutoreleasePool();
    static void drainThreadAutoreleasePool();

    static NSStatusItem* getStatusItem();
};

inline void SetSalData( SalData* pData ) { ImplGetSVData()->mpSalData = (void*)pData; }
inline SalData *GetSalData() { return (SalData*)ImplGetSVData()->mpSalData; }
inline SalData *GetAppSalData() { return (SalData*)ImplGetAppSVData()->mpSalData; }

// --- Prototypes ---

sal_Bool ImplSalYieldMutexTryToAcquire();
void ImplSalYieldMutexAcquire();
void ImplSalYieldMutexRelease();

#endif  // _SV_SALDATA_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
