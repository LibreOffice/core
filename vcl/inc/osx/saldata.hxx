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

#ifndef INCLUDED_VCL_INC_OSX_SALDATA_HXX
#define INCLUDED_VCL_INC_OSX_SALDATA_HXX

#include <config_features.h>

#include "premac.h"
#include <Cocoa/Cocoa.h>
#include "postmac.h"

#include "com/sun/star/uno/Reference.hxx"

#include <vcl/ptrstyle.hxx>

#include "svdata.hxx"
#include "salwtype.hxx"

#include <functional>
#include <list>
#include <map>
#include <unordered_set>
#include <vector>
#include <o3tl/enumarray.hxx>

#include <cstdio>
#include <cstdarg>

#include "apple_remote/RemoteMainController.h"

class AquaSalFrame;
class AquaSalInstance;
class SalObject;
class SalFrame;
class SalVirtualDevice;
class SalPrinter;
class SystemFontList;

#define SAL_CLIPRECT_COUNT 16

struct FrameHash : public std::hash<sal_IntPtr>
{
    size_t operator()(const AquaSalFrame* frame) const
    { return std::hash<sal_IntPtr>::operator()( reinterpret_cast<const sal_IntPtr>(frame) ); }
};

#define INVALID_CURSOR_PTR reinterpret_cast<NSCursor*>(0xdeadbeef)

// Singleton, instantiated from Application::Application() in
// vcl/source/app/svapp.cxx through InitSalData().

class SalData
{
public:
    SALTIMERPROC                                  mpTimerProc;      // timer callback proc
    AquaSalInstance                              *mpFirstInstance;  // pointer of first instance
    std::list<AquaSalFrame*>                      maPresentationFrames;  // list of frames in presentation mode
    SalObject                                    *mpFirstObject;    // pointer of first object window
    SalVirtualDevice                             *mpFirstVD;        // first VirDev
    SalPrinter                                   *mpFirstPrinter;   // first printing printer
    SystemFontList                               *mpFontList;
    NSStatusItem*                                 mpStatusItem;     // one status item that draws all our statuses
                                                                    // at the moment this is only one add menu button
    CGColorSpaceRef                               mxRGBSpace;
    CGColorSpaceRef                               mxGraySpace;

    o3tl::enumarray< PointerStyle, NSCursor* >    maCursors;
    std::vector< NSMenuItem* >                    maFallbackMenu;
    std::map< NSEvent*, bool >                    maKeyEventAnswer;

    static oslThreadKey                           s_aAutoReleaseKey;

    bool                                          mbIsScrollbarDoubleMax;   // TODO: support DoubleMin and DoubleBoth too
#if !HAVE_FEATURE_MACOSX_SANDBOX
    AppleRemoteMainController*                    mpAppleRemoteMainController;
#endif
    NSObject*                                     mpDockIconClickHandler;
    long                                          mnDPIX;           // #i100617# read DPI only once per office life
    long                                          mnDPIY;           // #i100617# read DPI only once per office life

    css::uno::Reference< css::uno::XInterface >   mxClipboard;

    SalData();
    ~SalData();

    NSCursor* getCursor( PointerStyle i_eStyle );

    static void ensureThreadAutoreleasePool();

    static NSStatusItem* getStatusItem();
};

inline void SetSalData( SalData* pData ) { ImplGetSVData()->mpSalData = pData; }
inline SalData *GetSalData() { return ImplGetSVData()->mpSalData; }

bool ImplSalYieldMutexTryToAcquire();
void ImplSalYieldMutexRelease();

#endif // INCLUDED_VCL_INC_OSX_SALDATA_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
