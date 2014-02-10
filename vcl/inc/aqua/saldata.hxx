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



#ifndef _SV_SALDATA_HXX
#define _SV_SALDATA_HXX

#include "premac.h"
#include <Cocoa/Cocoa.h>
#include "postmac.h"

#include "com/sun/star/uno/Reference.hxx"

#include "vcl/ptrstyle.hxx"

#include "svdata.hxx"
#include "salwtype.hxx"

#include <list>
#include <vector>
#include <map>
#include <hash_set>

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

#define OSX_VER_TIGER     0x1040
#define OSX_VER_LEOPARD   0x1050
#define OSX_VER_SNOW_LEO  0x1060
#define OSX_VER_LION      0x1070
#define OSX_VER_MOUNTLION 0x1080
#define OSX_VER_MAVERICKS 0x1070

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
    std::list<AquaSalFrame*>                      maFrames;         // list of all frames
    std::hash_set<const AquaSalFrame*,FrameHash>  maFrameCheck;     // for fast check of frame existance
    std::list<AquaSalFrame*>                      maPresentationFrames;  // list of frames in presentation mode
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
    AppleRemoteMainController*                    mpAppleRemoteMainController;

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
