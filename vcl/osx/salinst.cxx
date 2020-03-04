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

#include <sal/config.h>
#include <sal/log.hxx>
#include <osl/diagnose.h>

#include <algorithm>
#include <condition_variable>
#include <mutex>
#include <utility>

#include <config_features.h>

#include <stdio.h>

#include <comphelper/solarmutex.hxx>
#include <comphelper/lok.hxx>

#include <osl/process.h>

#include <unotools/resmgr.hxx>
#include <vclpluginapi.h>
#include <vcl/svapp.hxx>
#include <vcl/window.hxx>
#include <vcl/idle.hxx>
#include <vcl/svmain.hxx>
#include <vcl/opengl/OpenGLContext.hxx>
#include <vcl/commandevent.hxx>
#include <vcl/event.hxx>
#include <vcl/virdev.hxx>
#include <vcl/quickselectionengine.hxx>
#include <vcl/builder.hxx>
#include <vcl/weld.hxx>
#include <xmlreader/xmlreader.hxx>

#define VCL_INTERNALS
#include <vcl/toolkit/dialog.hxx>

#include <salvtables.hxx>
#include <osx/saldata.hxx>
#include <osx/salinst.h>
#include <osx/salframe.h>
#include <osx/salobj.h>
#include <osx/salsys.h>
#include <quartz/salvd.h>
#include <quartz/salbmp.h>
#include <quartz/utils.h>
#include <osx/salprn.h>
#include <osx/saltimer.h>
#include <osx/vclnsapp.h>
#include <osx/runinmain.hxx>

#include <print.h>

#include <comphelper/processfactory.hxx>

#include <com/sun/star/uri/ExternalUriReferenceTranslator.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

#include <premac.h>
#include <objc/runtime.h>
#include <Foundation/Foundation.h>
#include <ApplicationServices/ApplicationServices.h>
#import "apple_remote/RemoteMainController.h"
#include <apple_remote/RemoteControl.h>
#include <postmac.h>

extern "C" {
#include <crt_externs.h>
}

using namespace std;
using namespace ::com::sun::star;

static int* gpnInit = nullptr;
static NSMenu* pDockMenu = nil;
static bool bLeftMain = false;

namespace {

class AquaDelayedSettingsChanged : public Idle
{
    bool            mbInvalidate;

public:
    AquaDelayedSettingsChanged( bool bInvalidate ) :
        mbInvalidate( bInvalidate )
    {
    }

    virtual void Invoke() override
    {
        AquaSalInstance *pInst = GetSalData()->mpInstance;
        SalFrame *pAnyFrame = pInst->anyFrame();
        if( pAnyFrame )
            pAnyFrame->CallCallback( SalEvent::SettingsChanged, nullptr );

        if( mbInvalidate )
        {
            for( auto pSalFrame : pInst->getFrames() )
            {
                AquaSalFrame* pFrame = static_cast<AquaSalFrame*>( pSalFrame );
                if( pFrame->mbShown )
                    pFrame->SendPaintEvent();
            }
        }
        delete this;
    }
};

}

void AquaSalInstance::delayedSettingsChanged( bool bInvalidate )
{
    osl::Guard< comphelper::SolarMutex > aGuard( *GetYieldMutex() );
    AquaDelayedSettingsChanged* pIdle = new AquaDelayedSettingsChanged( bInvalidate );
    pIdle->SetDebugName( "AquaSalInstance AquaDelayedSettingsChanged" );
    pIdle->Start();
}

// the std::list<const ApplicationEvent*> must be available before any SalData/SalInst/etc. objects are ready
std::list<const ApplicationEvent*> AquaSalInstance::aAppEventList;

NSMenu* AquaSalInstance::GetDynamicDockMenu()
{
    if( ! pDockMenu && ! bLeftMain )
        pDockMenu = [[NSMenu alloc] initWithTitle: @""];
    return pDockMenu;
}

bool AquaSalInstance::isOnCommandLine( const OUString& rArg )
{
    sal_uInt32 nArgs = osl_getCommandArgCount();
    for( sal_uInt32 i = 0; i < nArgs; i++ )
    {
        OUString aArg;
        osl_getCommandArg( i, &aArg.pData );
        if( aArg.equals( rArg ) )
            return true;
    }
    return false;
}

void AquaSalInstance::AfterAppInit()
{
    [[NSNotificationCenter defaultCenter] addObserver: NSApp
                                          selector: @selector(systemColorsChanged:)
                                          name: NSSystemColorsDidChangeNotification
                                          object: nil ];
    [[NSNotificationCenter defaultCenter] addObserver: NSApp
                                          selector: @selector(screenParametersChanged:)
                                          name: NSApplicationDidChangeScreenParametersNotification
                                          object: nil ];
    // add observers for some settings changes that affect vcl's settings
    // scrollbar variant
    [[NSDistributedNotificationCenter defaultCenter] addObserver: NSApp
                                          selector: @selector(scrollbarVariantChanged:)
                                          name: @"AppleAquaScrollBarVariantChanged"
                                          object: nil ];
    // scrollbar page behavior ("jump to here" or not)
    [[NSDistributedNotificationCenter defaultCenter] addObserver: NSApp
                                          selector: @selector(scrollbarSettingsChanged:)
                                          name: @"AppleNoRedisplayAppearancePreferenceChanged"
                                          object: nil ];
#if !HAVE_FEATURE_MACOSX_SANDBOX
    // Initialize Apple Remote
    GetSalData()->mpAppleRemoteMainController = [[AppleRemoteMainController alloc] init];

    [[NSDistributedNotificationCenter defaultCenter] addObserver: NSApp
                                           selector: @selector(applicationWillBecomeActive:)
                                           name: @"AppleRemoteWillBecomeActive"
                                           object: nil ];

    [[NSDistributedNotificationCenter defaultCenter] addObserver: NSApp
                                           selector: @selector(applicationWillResignActive:)
                                           name: @"AppleRemoteWillResignActive"
                                           object: nil ];
#endif
}

void SalAbort( const OUString& rErrorText, bool bDumpCore )
{
    if( rErrorText.isEmpty() )
        fprintf( stderr, "Application Error " );
    else
        fprintf( stderr, "%s ",
            OUStringToOString( rErrorText, osl_getThreadTextEncoding() ).getStr() );
    if( bDumpCore )
        abort();
    else
        _exit(1);
}

SalYieldMutex::SalYieldMutex()
    : m_aCodeBlock( nullptr )
{
}

SalYieldMutex::~SalYieldMutex()
{
}

void SalYieldMutex::doAcquire( sal_uInt32 nLockCount )
{
    AquaSalInstance *pInst = GetSalData()->mpInstance;
    if ( pInst && pInst->IsMainThread() )
    {
        if ( pInst->mbNoYieldLock )
            return;
        do {
            RuninmainBlock block = nullptr;
            {
                std::unique_lock<std::mutex> g(m_runInMainMutex);
                if (m_aMutex.tryToAcquire()) {
                    assert(m_aCodeBlock == nullptr);
                    m_wakeUpMain = false;
                    break;
                }
                // wait for doRelease() or RUNINMAIN_* to set the condition
                m_aInMainCondition.wait(g, [this]() { return m_wakeUpMain; });
                m_wakeUpMain = false;
                std::swap(block, m_aCodeBlock);
            }
            if ( block )
            {
                assert( !pInst->mbNoYieldLock );
                pInst->mbNoYieldLock = true;
                block();
                pInst->mbNoYieldLock = false;
                Block_release( block );
                std::scoped_lock<std::mutex> g(m_runInMainMutex);
                assert(!m_resultReady);
                m_resultReady = true;
                m_aResultCondition.notify_all();
            }
        }
        while ( true );
    }
    else
        m_aMutex.acquire();
    ++m_nCount;
    --nLockCount;

    comphelper::SolarMutex::doAcquire( nLockCount );
}

sal_uInt32 SalYieldMutex::doRelease( const bool bUnlockAll )
{
    AquaSalInstance *pInst = GetSalData()->mpInstance;
    if ( pInst->mbNoYieldLock && pInst->IsMainThread() )
        return 1;
    sal_uInt32 nCount;
    {
        std::scoped_lock<std::mutex> g(m_runInMainMutex);
        // read m_nCount before doRelease
        bool const isReleased(bUnlockAll || m_nCount == 1);
        nCount = comphelper::SolarMutex::doRelease( bUnlockAll );
        if (isReleased && !pInst->IsMainThread()) {
            m_wakeUpMain = true;
            m_aInMainCondition.notify_all();
        }
    }
    return nCount;
}

bool SalYieldMutex::IsCurrentThread() const
{
    if ( !GetSalData()->mpInstance->mbNoYieldLock )
        return comphelper::SolarMutex::IsCurrentThread();
    else
        return GetSalData()->mpInstance->IsMainThread();
}

// some convenience functions regarding the yield mutex, aka solar mutex

bool ImplSalYieldMutexTryToAcquire()
{
    AquaSalInstance* pInst = GetSalData()->mpInstance;
    if ( pInst )
        return pInst->GetYieldMutex()->tryToAcquire();
    else
        return false;
}

void ImplSalYieldMutexRelease()
{
    AquaSalInstance* pInst = GetSalData()->mpInstance;
    if ( pInst )
        pInst->GetYieldMutex()->release();
}

extern "C" {
VCLPLUG_OSX_PUBLIC SalInstance* create_SalInstance()
{
    SalData* pSalData = new SalData;

    NSAutoreleasePool * pool = [ [ NSAutoreleasePool alloc ] init ];
    unlink([[NSString stringWithFormat:@"%@/Library/Saved Application State/%s.savedState/restorecount.plist", NSHomeDirectory(), MACOSX_BUNDLE_IDENTIFIER] UTF8String]);
    unlink([[NSString stringWithFormat:@"%@/Library/Saved Application State/%s.savedState/restorecount.txt", NSHomeDirectory(), MACOSX_BUNDLE_IDENTIFIER] UTF8String]);
    [ pool drain ];

    // create our cocoa NSApplication
    [VCL_NSApplication sharedApplication];

    SalData::ensureThreadAutoreleasePool();

    // put cocoa into multithreaded mode
    [NSThread detachNewThreadSelector:@selector(enableCocoaThreads:) toTarget:[[CocoaThreadEnabler alloc] init] withObject:nil];

    // activate our delegate methods
    [NSApp setDelegate: NSApp];

    SAL_WARN_IF( pSalData->mpInstance != nullptr, "vcl", "more than one instance created" );
    AquaSalInstance* pInst = new AquaSalInstance;

    // init instance (only one instance in this version !!!)
    pSalData->mpInstance = pInst;
    // this one is for outside AquaSalInstance::Yield
    SalData::ensureThreadAutoreleasePool();
    // no focus rects on NWF
    ImplGetSVData()->maNWFData.mbNoFocusRects = true;
    ImplGetSVData()->maNWFData.mbNoActiveTabTextRaise = true;
    ImplGetSVData()->maNWFData.mbCenteredTabs = true;
    ImplGetSVData()->maNWFData.mnStatusBarLowerRightOffset = 10;

    return pInst;
}
}

AquaSalInstance::AquaSalInstance()
    : SalInstance(std::make_unique<SalYieldMutex>())
    , mnActivePrintJobs( 0 )
    , mbIsLiveResize( false )
    , mbNoYieldLock( false )
    , mbTimerProcessed( false )
{
    maMainThread = osl::Thread::getCurrentIdentifier();

    ImplSVData* pSVData = ImplGetSVData();
    pSVData->maAppData.mxToolkitName = OUString("osx");
}

AquaSalInstance::~AquaSalInstance()
{
    [NSApp stop: NSApp];
    bLeftMain = true;
    if( pDockMenu )
    {
        [pDockMenu release];
        pDockMenu = nil;
    }
}

void AquaSalInstance::TriggerUserEventProcessing()
{
    dispatch_async(dispatch_get_main_queue(),^{
        ImplNSAppPostEvent( AquaSalInstance::YieldWakeupEvent, NO );
    });
}

void AquaSalInstance::ProcessEvent( SalUserEvent aEvent )
{
    aEvent.m_pFrame->CallCallback( aEvent.m_nEvent, aEvent.m_pData );
    maWaitingYieldCond.set();
}

bool AquaSalInstance::IsMainThread() const
{
    return osl::Thread::getCurrentIdentifier() == maMainThread;
}

void AquaSalInstance::handleAppDefinedEvent( NSEvent* pEvent )
{
    AquaSalTimer *pTimer = static_cast<AquaSalTimer*>( ImplGetSVData()->maSchedCtx.mpSalTimer );
    int nSubtype = [pEvent subtype];
    switch( nSubtype )
    {
    case AppStartTimerEvent:
        if ( pTimer )
            pTimer->handleStartTimerEvent( pEvent );
        break;
    case AppExecuteSVMain:
    {
        int nRet = ImplSVMain();
        if (gpnInit)
            *gpnInit = nRet;
        [NSApp stop: NSApp];
        break;
    }
    case DispatchTimerEvent:
    {
        AquaSalInstance *pInst = GetSalData()->mpInstance;
        if ( pTimer && pInst )
            pInst->mbTimerProcessed = pTimer->handleDispatchTimerEvent( pEvent );
        break;
    }
#if !HAVE_FEATURE_MACOSX_SANDBOX
    case AppleRemoteControlEvent: // Defined in <apple_remote/RemoteMainController.h>
    {
        MediaCommand nCommand;
        AquaSalInstance *pInst = GetSalData()->mpInstance;
        bool bIsFullScreenMode = false;

        for( auto pSalFrame : pInst->getFrames() )
        {
            const AquaSalFrame* pFrame = static_cast<const AquaSalFrame*>( pSalFrame );
            if ( pFrame->mbFullScreen )
            {
                bIsFullScreenMode = true;
                break;
            }
        }

        switch ([pEvent data1])
        {
            case kRemoteButtonPlay:
                nCommand = bIsFullScreenMode ? MediaCommand::PlayPause : MediaCommand::Play;
                break;

            // kept for experimentation purpose (scheduled for future implementation)
            // case kRemoteButtonMenu:         nCommand = MediaCommand::Menu; break;

            case kRemoteButtonPlus:         nCommand = MediaCommand::VolumeUp; break;

            case kRemoteButtonMinus:        nCommand = MediaCommand::VolumeDown; break;

            case kRemoteButtonRight:        nCommand = MediaCommand::NextTrack; break;

            case kRemoteButtonRight_Hold:   nCommand = MediaCommand::NextTrackHold; break;

            case kRemoteButtonLeft:         nCommand = MediaCommand::PreviousTrack; break;

            case kRemoteButtonLeft_Hold:    nCommand = MediaCommand::Rewind; break;

            case kRemoteButtonPlay_Hold:    nCommand = MediaCommand::PlayHold; break;

            case kRemoteButtonMenu_Hold:    nCommand = MediaCommand::Stop; break;

            // FIXME : not detected
            case kRemoteButtonPlus_Hold:
            case kRemoteButtonMinus_Hold:
                break;

            default:
                break;
        }
        AquaSalFrame* pFrame = static_cast<AquaSalFrame*>( pInst->anyFrame() );
        vcl::Window* pWindow = pFrame ? pFrame->GetWindow() : nullptr;
        if( pWindow )
        {
            const Point aPoint;
            CommandMediaData aMediaData(nCommand);
            CommandEvent aCEvt( aPoint, CommandEventId::Media, false, &aMediaData );
            NotifyEvent aNCmdEvt( MouseNotifyEvent::COMMAND, pWindow, &aCEvt );

            if ( !ImplCallPreNotify( aNCmdEvt ) )
                pWindow->Command( aCEvt );
        }

    }
    break;
#endif

    case YieldWakeupEvent:
        // do nothing, fall out of Yield
        break;

    default:
        OSL_FAIL( "unhandled NSApplicationDefined event" );
        break;
    }
}

bool AquaSalInstance::RunInMainYield( bool bHandleAllCurrentEvents )
{
    OSX_SALDATA_RUNINMAIN_UNION( DoYield( false, bHandleAllCurrentEvents), boolean )
    assert( false && "Don't call this from the main thread!" );
    return false;

}

static bool isWakeupEvent( NSEvent *pEvent )
{
SAL_WNODEPRECATED_DECLARATIONS_PUSH
    return NSApplicationDefined == [pEvent type]
        && AquaSalInstance::YieldWakeupEvent == static_cast<int>([pEvent subtype]);
SAL_WNODEPRECATED_DECLARATIONS_POP
}

bool AquaSalInstance::DoYield(bool bWait, bool bHandleAllCurrentEvents)
{
    // ensure that the per thread autorelease pool is top level and
    // will therefore not be destroyed by cocoa implicitly
    SalData::ensureThreadAutoreleasePool();

    // NSAutoreleasePool documentation suggests we should have
    // an own pool for each yield level
    ReleasePoolHolder aReleasePool;

    // first, process current user events
    bool bHadEvent = DispatchUserEvents( bHandleAllCurrentEvents );
    if ( !bHandleAllCurrentEvents && bHadEvent )
        return true;

    // handle cocoa event queue
    // cocoa events may be only handled in the thread the NSApp was created
    if( IsMainThread() && mnActivePrintJobs == 0 )
    {
        // handle available events
        NSEvent* pEvent = nil;
        NSTimeInterval now = [[NSProcessInfo processInfo] systemUptime];
        mbTimerProcessed = false;

        do
        {
            SolarMutexReleaser aReleaser;

            pEvent = [NSApp nextEventMatchingMask: NSEventMaskAny
                            untilDate: nil
                            inMode: NSDefaultRunLoopMode
                            dequeue: YES];
            if( pEvent )
            {
                [NSApp sendEvent: pEvent];
                if ( isWakeupEvent( pEvent ) )
                    continue;
                bHadEvent = true;
            }

            [NSApp updateWindows];

            if ( !bHandleAllCurrentEvents || !pEvent || now < [pEvent timestamp] )
                break;
        }
        while( true );

        AquaSalTimer *pTimer = static_cast<AquaSalTimer*>( ImplGetSVData()->maSchedCtx.mpSalTimer );
        if ( !mbTimerProcessed && pTimer && pTimer->IsDirectTimeout() )
        {
            pTimer->handleTimerElapsed();
            bHadEvent = true;
        }

        // if we had no event yet, wait for one if requested
        if( bWait && ! bHadEvent )
        {
            SolarMutexReleaser aReleaser;

            pEvent = [NSApp nextEventMatchingMask: NSEventMaskAny
                            untilDate: [NSDate distantFuture]
                            inMode: NSDefaultRunLoopMode
                            dequeue: YES];
            if( pEvent )
            {
                [NSApp sendEvent: pEvent];
                if ( !isWakeupEvent( pEvent ) )
                    bHadEvent = true;
            }
            [NSApp updateWindows];
        }

        // collect update rectangles
        for( auto pSalFrame : GetSalData()->mpInstance->getFrames() )
        {
            AquaSalFrame* pFrame = static_cast<AquaSalFrame*>( pSalFrame );
            if( pFrame->mbShown && ! pFrame->maInvalidRect.IsEmpty() )
            {
                pFrame->Flush( pFrame->maInvalidRect );
                pFrame->maInvalidRect.SetEmpty();
            }
        }

        if ( bHadEvent )
            maWaitingYieldCond.set();
    }
    else
    {
        bHadEvent = RunInMainYield( bHandleAllCurrentEvents );
        if ( !bHadEvent && bWait )
        {
            // #i103162#
            // wait until the main thread has dispatched an event
            maWaitingYieldCond.reset();
            SolarMutexReleaser aReleaser;
            maWaitingYieldCond.wait();
        }
    }

    // we get some apple events way too early
    // before the application is ready to handle them,
    // so their corresponding application events need to be delayed
    // now is a good time to handle at least one of them
    if( bWait && !aAppEventList.empty() && ImplGetSVData()->maAppData.mbInAppExecute )
    {
        // make sure that only one application event is active at a time
        static bool bInAppEvent = false;
        if( !bInAppEvent )
        {
            bInAppEvent = true;
            // get the next delayed application event
            const ApplicationEvent* pAppEvent = aAppEventList.front();
            aAppEventList.pop_front();
            // handle one application event (no recursion)
            const ImplSVData* pSVData = ImplGetSVData();
            pSVData->mpApp->AppEvent( *pAppEvent );
            delete pAppEvent;
            // allow the next delayed application event
            bInAppEvent = false;
        }
    }

    return bHadEvent;
}

bool AquaSalInstance::AnyInput( VclInputFlags nType )
{
    if( nType & VclInputFlags::APPEVENT )
    {
        if( ! aAppEventList.empty() )
            return true;
        if( nType == VclInputFlags::APPEVENT )
            return false;
    }

    OSX_INST_RUNINMAIN_UNION( AnyInput( nType ), boolean )

    if( nType & VclInputFlags::TIMER )
    {
        AquaSalTimer *pTimer = static_cast<AquaSalTimer*>( ImplGetSVData()->maSchedCtx.mpSalTimer );
        if (pTimer && pTimer->IsTimerElapsed())
            return true;
    }

    unsigned/*NSUInteger*/ nEventMask = 0;
SAL_WNODEPRECATED_DECLARATIONS_PUSH
        // 'NSFlagsChangedMask' is deprecated: first deprecated in macOS 10.12
        // 'NSKeyDownMask' is deprecated: first deprecated in macOS 10.12
        // 'NSKeyUpMask' is deprecated: first deprecated in macOS 10.12
        // 'NSLeftMouseDownMask' is deprecated: first deprecated in macOS 10.12
        // 'NSLeftMouseDraggedMask' is deprecated: first deprecated in macOS 10.12
        // 'NSLeftMouseUpMask' is deprecated: first deprecated in macOS 10.12
        // 'NSMouseEnteredMask' is deprecated: first deprecated in macOS 10.12
        // 'NSMouseExitedMask' is deprecated: first deprecated in macOS 10.12
        // 'NSOtherMouseDownMask' is deprecated: first deprecated in macOS 10.12
        // 'NSOtherMouseDraggedMask' is deprecated: first deprecated in macOS 10.12
        // 'NSOtherMouseUpMask' is deprecated: first deprecated in macOS 10.12
        // 'NSRightMouseDownMask' is deprecated: first deprecated in macOS 10.12
        // 'NSRightMouseDraggedMask' is deprecated: first deprecated in macOS 10.12
        // 'NSRightMouseUpMask' is deprecated: first deprecated in macOS 10.12
        // 'NSScrollWheelMask' is deprecated: first deprecated in macOS 10.12
        // 'NSTabletPoint' is deprecated: first deprecated in macOS 10.12
    if( nType & VclInputFlags::MOUSE)
        nEventMask |=
            NSLeftMouseDownMask    | NSRightMouseDownMask    | NSOtherMouseDownMask |
            NSLeftMouseUpMask      | NSRightMouseUpMask      | NSOtherMouseUpMask |
            NSLeftMouseDraggedMask | NSRightMouseDraggedMask | NSOtherMouseDraggedMask |
            NSScrollWheelMask |
            // NSMouseMovedMask |
            NSMouseEnteredMask | NSMouseExitedMask;
    if( nType & VclInputFlags::KEYBOARD)
        nEventMask |= NSKeyDownMask | NSKeyUpMask | NSFlagsChangedMask;
    if( nType & VclInputFlags::OTHER)
        nEventMask |= NSTabletPoint | NSApplicationDefinedMask;
SAL_WNODEPRECATED_DECLARATIONS_POP
    // TODO: VclInputFlags::PAINT / more VclInputFlags::OTHER
    if( !bool(nType) )
        return false;

    NSEvent* pEvent = [NSApp nextEventMatchingMask: nEventMask untilDate: nil
                            inMode: NSDefaultRunLoopMode dequeue: NO];
    return (pEvent != nullptr);
}

SalFrame* AquaSalInstance::CreateChildFrame( SystemParentData*, SalFrameStyleFlags /*nSalFrameStyle*/ )
{
    return nullptr;
}

SalFrame* AquaSalInstance::CreateFrame( SalFrame* pParent, SalFrameStyleFlags nSalFrameStyle )
{
    OSX_INST_RUNINMAIN_POINTER( CreateFrame( pParent, nSalFrameStyle ), SalFrame* )
    return new AquaSalFrame( pParent, nSalFrameStyle );
}

void AquaSalInstance::DestroyFrame( SalFrame* pFrame )
{
    OSX_INST_RUNINMAIN( DestroyFrame( pFrame ) )
    delete pFrame;
}

SalObject* AquaSalInstance::CreateObject( SalFrame* pParent, SystemWindowData* pWindowData, bool /* bShow */ )
{
    if ( !pParent )
        return nullptr;

    OSX_INST_RUNINMAIN_POINTER( CreateObject( pParent, pWindowData, false ), SalObject* )
    return new AquaSalObject( static_cast<AquaSalFrame*>(pParent), pWindowData );
}

void AquaSalInstance::DestroyObject( SalObject* pObject )
{
    OSX_INST_RUNINMAIN( DestroyObject( pObject ) )
    delete pObject;
}

std::unique_ptr<SalPrinter> AquaSalInstance::CreatePrinter( SalInfoPrinter* pInfoPrinter )
{
    return std::unique_ptr<SalPrinter>(new AquaSalPrinter( dynamic_cast<AquaSalInfoPrinter*>(pInfoPrinter) ));
}

void AquaSalInstance::GetPrinterQueueInfo( ImplPrnQueueList* pList )
{
    NSArray* pNames = [NSPrinter printerNames];
    NSArray* pTypes = [NSPrinter printerTypes];
    unsigned int nNameCount = pNames ? [pNames count] : 0;
    unsigned int nTypeCount = pTypes ? [pTypes count] : 0;
    SAL_WARN_IF( nTypeCount != nNameCount, "vcl", "type count not equal to printer count" );
    for( unsigned int i = 0; i < nNameCount; i++ )
    {
        NSString* pName = [pNames objectAtIndex: i];
        NSString* pType = i < nTypeCount ? [pTypes objectAtIndex: i] : nil;
        if( pName )
        {
            std::unique_ptr<SalPrinterQueueInfo> pInfo(new SalPrinterQueueInfo);
            pInfo->maPrinterName    = GetOUString( pName );
            if( pType )
                pInfo->maDriver     = GetOUString( pType );
            pInfo->mnStatus         = PrintQueueFlags::NONE;
            pInfo->mnJobs           = 0;

            pList->Add( std::move(pInfo) );
        }
    }
}

void AquaSalInstance::GetPrinterQueueState( SalPrinterQueueInfo* )
{
}

OUString AquaSalInstance::GetDefaultPrinter()
{
    // #i113170# may not be the main thread if called from UNO API
    SalData::ensureThreadAutoreleasePool();

    if( maDefaultPrinter.isEmpty() )
    {
        NSPrintInfo* pPI = [NSPrintInfo sharedPrintInfo];
        SAL_WARN_IF( !pPI, "vcl", "no print info" );
        if( pPI )
        {
            NSPrinter* pPr = [pPI printer];
            SAL_WARN_IF( !pPr, "vcl", "no printer in default info" );
            if( pPr )
            {
                NSString* pDefName = [pPr name];
                SAL_WARN_IF( !pDefName, "vcl", "printer has no name" );
                maDefaultPrinter = GetOUString( pDefName );
            }
        }
    }
    return maDefaultPrinter;
}

SalInfoPrinter* AquaSalInstance::CreateInfoPrinter( SalPrinterQueueInfo* pQueueInfo,
                                                    ImplJobSetup* pSetupData )
{
    // #i113170# may not be the main thread if called from UNO API
    SalData::ensureThreadAutoreleasePool();

    SalInfoPrinter* pNewInfoPrinter = nullptr;
    if( pQueueInfo )
    {
        pNewInfoPrinter = new AquaSalInfoPrinter( *pQueueInfo );
        if( pSetupData )
            pNewInfoPrinter->SetPrinterData( pSetupData );
    }

    return pNewInfoPrinter;
}

void AquaSalInstance::DestroyInfoPrinter( SalInfoPrinter* pPrinter )
{
    // #i113170# may not be the main thread if called from UNO API
    SalData::ensureThreadAutoreleasePool();

    delete pPrinter;
}

OUString AquaSalInstance::GetConnectionIdentifier()
{
    return "";
}

// We need to re-encode file urls because osl_getFileURLFromSystemPath converts
// to UTF-8 before encoding non ascii characters, which is not what other apps expect.
static OUString translateToExternalUrl(const OUString& internalUrl)
{
    uno::Reference< uno::XComponentContext > context(
        comphelper::getProcessComponentContext());
    return uri::ExternalUriReferenceTranslator::create(context)->translateToExternal(internalUrl);
}

// #i104525# many versions of OSX have problems with some URLs:
// when an app requests OSX to add one of these URLs to the "Recent Items" list
// then this app gets killed (TextEdit, Preview, etc. and also OOo)
static bool isDangerousUrl( const OUString& rUrl )
{
    // use a heuristic that detects all known cases since there is no official comment
    // on the exact impact and root cause of the OSX bug
    const int nLen = rUrl.getLength();
    const sal_Unicode* p = rUrl.getStr();
    for( int i = 0; i < nLen-3; ++i, ++p ) {
        if( p[0] != '%' )
            continue;
        // escaped percent?
        if( (p[1] == '2') && (p[2] == '5') )
            return true;
        // escapes are considered to be UTF-8 encoded
        // => check for invalid UTF-8 leading byte
        if( (p[1] != 'f') && (p[1] != 'F') )
            continue;
        int cLowNibble = p[2];
        if( (cLowNibble >= '0' ) && (cLowNibble <= '9'))
            return false;
        if( cLowNibble >= 'a' )
            cLowNibble -= 'a' - 'A';
        if( (cLowNibble < 'A') || (cLowNibble >= 'C'))
            return true;
    }

    return false;
}

void AquaSalInstance::AddToRecentDocumentList(const OUString& rFileUrl, const OUString& /*rMimeType*/, const OUString& /*rDocumentService*/)
{
    // Convert file URL for external use (see above)
    OUString externalUrl = translateToExternalUrl(rFileUrl);
    if( externalUrl.isEmpty() )
        externalUrl = rFileUrl;

    if( !externalUrl.isEmpty() && !isDangerousUrl( externalUrl ) )
    {
        NSString* pString = CreateNSString( externalUrl );
        NSURL* pURL = [NSURL URLWithString: pString];

        if( pURL )
        {
            NSDocumentController* pCtrl = [NSDocumentController sharedDocumentController];
            [pCtrl noteNewRecentDocumentURL: pURL];
        }
        if( pString )
            [pString release];
    }
}

SalTimer* AquaSalInstance::CreateSalTimer()
{
    return new AquaSalTimer();
}

SalSystem* AquaSalInstance::CreateSalSystem()
{
    return new AquaSalSystem();
}

std::shared_ptr<SalBitmap> AquaSalInstance::CreateSalBitmap()
{
    return std::make_shared<QuartzSalBitmap>();
}

OUString AquaSalInstance::getOSVersion()
{
    NSString * versionString = nullptr;
    NSDictionary * sysVersionDict = [ NSDictionary dictionaryWithContentsOfFile: @"/System/Library/CoreServices/SystemVersion.plist" ];
    if ( sysVersionDict )
        versionString = [ sysVersionDict valueForKey: @"ProductVersion" ];

    OUString aVersion = "Mac OS X ";
    if ( versionString )
        aVersion += OUString::fromUtf8( [ versionString UTF8String ] );
    else
        aVersion += "(unknown)";

    return aVersion;
}

CGImageRef CreateCGImage( const Image& rImage )
{
    BitmapEx aBmpEx( rImage.GetBitmapEx() );
    Bitmap aBmp( aBmpEx.GetBitmap() );

    if( ! aBmp || ! aBmp.ImplGetSalBitmap() )
        return nullptr;

    // simple case, no transparency
    QuartzSalBitmap* pSalBmp = static_cast<QuartzSalBitmap*>(aBmp.ImplGetSalBitmap().get());

    if( ! pSalBmp )
        return nullptr;

    CGImageRef xImage = nullptr;
    if( ! (aBmpEx.IsAlpha() || aBmpEx.IsTransparent() ) )
        xImage = pSalBmp->CreateCroppedImage( 0, 0, pSalBmp->mnWidth, pSalBmp->mnHeight );
    else if( aBmpEx.IsAlpha() )
    {
        AlphaMask aAlphaMask( aBmpEx.GetAlpha() );
        Bitmap aMask( aAlphaMask.GetBitmap() );
        QuartzSalBitmap* pMaskBmp = static_cast<QuartzSalBitmap*>(aMask.ImplGetSalBitmap().get());
        if( pMaskBmp )
            xImage = pSalBmp->CreateWithMask( *pMaskBmp, 0, 0, pSalBmp->mnWidth, pSalBmp->mnHeight );
        else
            xImage = pSalBmp->CreateCroppedImage( 0, 0, pSalBmp->mnWidth, pSalBmp->mnHeight );
    }
    else if( aBmpEx.GetTransparentType() == TransparentType::Bitmap )
    {
        Bitmap aMask( aBmpEx.GetMask() );
        QuartzSalBitmap* pMaskBmp = static_cast<QuartzSalBitmap*>(aMask.ImplGetSalBitmap().get());
        if( pMaskBmp )
            xImage = pSalBmp->CreateWithMask( *pMaskBmp, 0, 0, pSalBmp->mnWidth, pSalBmp->mnHeight );
        else
            xImage = pSalBmp->CreateCroppedImage( 0, 0, pSalBmp->mnWidth, pSalBmp->mnHeight );
    }
    else if( aBmpEx.GetTransparentType() == TransparentType::Color )
    {
        Color aTransColor( aBmpEx.GetTransparentColor() );
        Color nTransColor( aTransColor.GetRed(), aTransColor.GetGreen(), aTransColor.GetBlue() );
        xImage = pSalBmp->CreateColorMask( 0, 0, pSalBmp->mnWidth, pSalBmp->mnHeight, nTransColor );
    }

    return xImage;
}

NSImage* CreateNSImage( const Image& rImage )
{
    CGImageRef xImage = CreateCGImage( rImage );

    if( ! xImage )
        return nil;

    Size aSize( rImage.GetSizePixel() );
    NSImage* pImage = [[NSImage alloc] initWithSize: NSMakeSize( aSize.Width(), aSize.Height() )];
    if( pImage )
    {
        [pImage lockFocusFlipped:YES];
        NSGraphicsContext* pContext = [NSGraphicsContext currentContext];
        CGContextRef rCGContext = [pContext CGContext];

        const CGRect aDstRect = { {0, 0}, { static_cast<CGFloat>(aSize.Width()), static_cast<CGFloat>(aSize.Height()) } };
        CGContextDrawImage( rCGContext, aDstRect, xImage );

        [pImage unlockFocus];
    }

    CGImageRelease( xImage );

    return pImage;
}

bool AquaSalInstance::SVMainHook(int* pnInit)
{
    gpnInit = pnInit;

    OUString aExeURL, aExe;
    osl_getExecutableFile( &aExeURL.pData );
    osl_getSystemPathFromFileURL( aExeURL.pData, &aExe.pData );
    OString aByteExe( OUStringToOString( aExe, osl_getThreadTextEncoding() ) );

#ifdef DEBUG
    aByteExe += OString ( " NSAccessibilityDebugLogLevel 1" );
    const char* pArgv[] = { aByteExe.getStr(), NULL };
    NSApplicationMain( 3, pArgv );
#else
    const char* pArgv[] = { aByteExe.getStr(), nullptr };
    NSApplicationMain( 1, pArgv );
#endif

    return true;
}

// Correspondance between the GTK welding and macOS welding:
//
// GtkWidget         \
// GtkContainer       > NSView
// GtkBin            /
// GtkDialog        - NSPanel
// GtkMessageDialog - NSAlert
// GtkGrid          - NSGridView
// GtkNotebook      - NSTabView
// GtkFrame         - NSBox
// GtkBox           - NSStackView
// GtkButton        - NSButton


@interface NSObject (Cast)
+ (instancetype)castFrom:(id)from;
@end

@implementation NSObject (Cast)
+ (instancetype)castFrom:(id)from {
    if ([from isKindOfClass:self]) {
        return from;
    }
    return nil;
}
@end

class AppKitInstanceBuilder;

namespace
{
    const void* helpIdKey = "helpid";
    const void* leftAttachKey = "leftattach";
    const void* topAttachKey = "topattach";

    void set_help_id(NSView* pView, const OString& rHelpId)
    {
        NSString* helpid = [NSString stringWithUTF8String:rHelpId.getStr()];
        objc_setAssociatedObject(pView, helpIdKey, helpid, OBJC_ASSOCIATION_COPY);
    }

    OString get_help_id(const NSView *pView)
    {
        id pData = objc_getAssociatedObject(pView, helpIdKey);
        if (pData == nil || strcmp(object_getClassName(pData), object_getClassName(@"")) != 0)
            return "";
        NSString* pString = pData;
        return OString([pString UTF8String]);
    }

    void set_left_attach(NSView* pView, int nLeftAttach)
    {
        NSNumber* leftAttach = [NSNumber numberWithInteger:nLeftAttach];
        objc_setAssociatedObject(pView, leftAttachKey, leftAttach, OBJC_ASSOCIATION_COPY);
    }

    int get_left_attach(const NSView *pView)
    {
        id pData = objc_getAssociatedObject(pView, leftAttachKey);
        if (pData == nil || strcmp(object_getClassName(pData), object_getClassName(@0)) != 0)
            return -1;
        NSNumber* pNumber = pData;
        return [pNumber integerValue];
    }

    void set_top_attach(NSView* pView, int nTopAttach)
    {
        NSNumber* topAttach = [NSNumber numberWithInteger:nTopAttach];
        objc_setAssociatedObject(pView, topAttachKey, topAttach, OBJC_ASSOCIATION_COPY);
    }

    int get_top_attach(const NSView *pView)
    {
        id pData = objc_getAssociatedObject(pView, topAttachKey);
        if (pData == nil || strcmp(object_getClassName(pData), object_getClassName(@0)) != 0)
            return -1;
        NSNumber* pNumber = pData;
        return [pNumber integerValue];
    }

    bool extractOrientation(std::map<OString, OUString> &rMap)
    {
        bool bVertical = false;
        auto aFind = rMap.find("orientation");
        if (aFind != rMap.end())
        {
            bVertical = aFind->second.equalsIgnoreAsciiCase("vertical");
            rMap.erase(aFind);
        }
        return bVertical;
    }
}

class AppKitInstanceWidget : public virtual weld::Widget
{
protected:
    NSView* m_pView;
    AppKitInstanceBuilder* m_pBuilder;

private:
    bool m_bTakeOwnership;

public:
    AppKitInstanceWidget(NSView* pView, AppKitInstanceBuilder* pBuilder, bool bTakeOwnership)
        : m_pView(pView)
        , m_pBuilder(pBuilder)
        , m_bTakeOwnership(bTakeOwnership)
    {
        // SAL_DEBUG("AppKitInstanceWidget CTOR this=" << this);
        if (!bTakeOwnership)
            [m_pView retain];
    }

    virtual ~AppKitInstanceWidget() override
    {
        // SAL_DEBUG("AppKitInstanceWidget DTOR this=" << this);
        if (m_bTakeOwnership)
            [m_pView removeFromSuperview]; // ???
        else
            [m_pView release];
    }

    virtual void set_sensitive(bool sensitive) override
    {
        NSControl* control = [NSControl castFrom:m_pView];
        if (control != nil)
            [control setEnabled:(sensitive ? YES : NO)];
    }

    virtual bool get_sensitive() const override
    {
        NSControl* control = [NSControl castFrom:m_pView];
        if (control != nil)
            return [control isEnabled];
        return true;
    }

    virtual void show() override
    {
        [m_pView setHidden:NO];
    }

    virtual void hide() override
    {
        [m_pView setHidden:YES];
    }

    virtual bool get_visible() const override
    {
        return ![m_pView isHidden];
    }

    virtual bool is_visible() const override
    {
        return ![m_pView isHiddenOrHasHiddenAncestor];
    }

    virtual void set_can_focus(bool bCanFocus) override
    {
        (void) bCanFocus;
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
    }

    virtual void grab_focus() override
    {
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
    }

    virtual bool has_focus() const override
    {
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
        return false;
    }

    virtual bool is_active() const override
    {
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
        return true;
    }

    virtual void set_has_default(bool has_default) override
    {
        (void) has_default;
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
    }

    virtual bool get_has_default() const override
    {
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
        return false;
    }

    virtual void set_size_request(int nWidth, int nHeight) override
    {
        [m_pView setBoundsSize:NSMakeSize(nWidth, nHeight)];
    }

    virtual Size get_size_request() const override
    {
        // ???
        NSRect bounds = [m_pView bounds];
        return Size(bounds.size.width, bounds.size.height);
    }

    virtual Size get_preferred_size() const override
    {
        // ???
        NSRect bounds = [m_pView bounds];
        return Size(bounds.size.width, bounds.size.height);
    }

    virtual float get_approximate_digit_width() const override
    {
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
        return 10;
    }

    virtual int get_text_height() const override
    {
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
        return 10;
    }

    virtual Size get_pixel_size(const OUString& rText) const override
    {
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
        (void) rText;
        return Size(10, 10);
    }

    virtual OString get_buildable_name() const override
    {
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
        return "";
    }

    virtual void set_help_id(const OString& rHelpId) override
    {
        ::set_help_id(m_pView, rHelpId);
    }

    virtual OString get_help_id() const override
    {
        OString sRet = ::get_help_id(m_pView);
        if (sRet.isEmpty())
            sRet = OString("null");
        return sRet;
    }

    virtual void set_grid_left_attach(int nAttach) override
    {
        (void) nAttach;
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
    }

    virtual int get_grid_left_attach() const override
    {
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
        return 0;
    }

    virtual void set_grid_width(int nCols) override
    {
        (void) nCols;
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
    }

    virtual void set_grid_top_attach(int nAttach) override
    {
        (void) nAttach;
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
    }

    virtual int get_grid_top_attach() const override
    {
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
        return 0;
    }

    virtual void set_hexpand(bool bExpand) override
    {
        (void) bExpand;
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
    }

    virtual bool get_hexpand() const override
    {
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
        return false;
    }

    virtual void set_vexpand(bool bExpand) override
    {
        (void) bExpand;
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
    }

    virtual bool get_vexpand() const override
    {
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
        return false;
    }

    virtual void set_secondary(bool bSecondary) override
    {
        (void) bSecondary;
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
    }

    virtual void set_margin_top(int nMargin) override
    {
        (void) nMargin;
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
    }

    virtual void set_margin_bottom(int nMargin) override
    {
        (void) nMargin;
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
    }

    virtual void set_margin_left(int nMargin) override
    {
        (void) nMargin;
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
    }

    virtual void set_margin_right(int nMargin) override
    {
        (void) nMargin;
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
    }

    virtual int get_margin_top() const override
    {
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
        return 0;
    }

    virtual int get_margin_bottom() const override
    {
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
        return 0;
    }

    virtual int get_margin_left() const override
    {
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
        return 0;
    }

    virtual int get_margin_right() const override
    {
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
        return 0;
    }

    virtual bool get_extents_relative_to(weld::Widget& rRelative, int& x, int &y, int& width, int &height) override
    {
        (void) rRelative;
        (void) x;
        (void) y;
        (void) width;
        (void) height;
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
        return false;
    }

    virtual void set_accessible_name(const OUString& rName) override
    {
        (void) rName;
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
    }

    virtual OUString get_accessible_name() const override
    {
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
        return "";
    }

    virtual OUString get_accessible_description() const override
    {
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
        return "";
    }

    virtual void set_accessible_relation_labeled_by(weld::Widget* pLabel) override
    {
        (void) pLabel;
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
    }

    virtual void set_accessible_relation_label_for(weld::Widget* pLabeled) override
    {
        (void) pLabeled;
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
    }

    virtual void add_extra_accessible_relation(const css::accessibility::AccessibleRelation &rRelation) override
    {
        (void) rRelation;
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
    }

    virtual void clear_extra_accessible_relations() override
    {
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
    }

    virtual void set_tooltip_text(const OUString& rTip) override
    {
        [m_pView setToolTip:CreateNSString(rTip)];
    }

    virtual OUString get_tooltip_text() const override
    {
        return GetOUString([m_pView toolTip]);
    }

    virtual void grab_add() override
    {
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
    }

    virtual bool has_grab() const override
    {
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
        return false;
    }

    virtual void grab_remove() override
    {
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
    }

    virtual vcl::Font get_font() override
    {
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
        return vcl::Font();
    }

    virtual bool get_direction() const override
    {
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
        return false;
    }

    virtual void set_direction(bool bRTL) override
    {
        (void) bRTL;
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
    }

    virtual void freeze() override
    {
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
    }

    virtual void thaw() override
    {
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
    }

    virtual std::unique_ptr<weld::Container> weld_parent() const override;

    virtual void help_hierarchy_foreach(const std::function<bool(const OString&)>& func) override
    {
        (void) func;
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
    }

    virtual OUString strip_mnemonic(const OUString &rLabel) const override
    {
        return rLabel.replaceFirst("_", "");
    }

    virtual VclPtr<VirtualDevice> create_virtual_device() const override
    {
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
        return nullptr;
    }

    virtual void set_stack_background() override
    {
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
    }

    virtual void set_highlight_background() override
    {
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
    }

    virtual void set_toolbar_background() override
    {
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
    }

    virtual css::uno::Reference<css::datatransfer::dnd::XDropTarget> get_drop_target() override
    {
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
        return nullptr;
    }

    virtual void connect_get_property_tree(const Link<boost::property_tree::ptree&, void>&) override
    {
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
    }

    NSView* getView()
    {
        return m_pView;
    }

    NSWindow* getWindow()
    {
        return [m_pView window];
    }

    int runModal()
    {
        int ret = RET_CANCEL;
        while (true)
        {
            NSModalResponse response = [NSApp runModalForWindow:getWindow()];
            if (response == NSModalResponseOK)
                ret = RET_OK;
            else if (response == NSModalResponseCancel)
                ret = RET_CANCEL;
            break;
        }
        hide();
        return ret;
    }
};

class AppKitInstanceContainer : public AppKitInstanceWidget, public virtual weld::Container
{
public:
    AppKitInstanceContainer(NSView* pView, AppKitInstanceBuilder* pBuilder, bool bTakeOwnership)
        : AppKitInstanceWidget(pView, pBuilder, bTakeOwnership)
    {
        // SAL_DEBUG("AppKitInstanceContainer CTOR this=" << this);
    }

    virtual ~AppKitInstanceContainer()
    {
        // SAL_DEBUG("AppKitInstanceContainer DTOR this=" << this);
    }

    virtual void move(weld::Widget* pWidget, weld::Container* pNewParent) override
    {
        AppKitInstanceWidget* pAppKitWidget = dynamic_cast<AppKitInstanceWidget*>(pWidget);
        assert(pAppKitWidget);
        AppKitInstanceContainer* pNewAppKitParent = dynamic_cast<AppKitInstanceContainer*>(pNewParent);
        assert(!pNewParent || pNewAppKitParent);
        [pNewAppKitParent->m_pView addSubview:pAppKitWidget->getView()];
    }

    virtual void recursively_unset_default_buttons() override
    {
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
    }

    virtual css::uno::Reference<css::awt::XWindow> CreateChildFrame() override
    {
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
        return nullptr;
    }
};

std::unique_ptr<weld::Container> AppKitInstanceWidget::weld_parent() const
{
    NSView* parent = [m_pView superview];
    if (parent == nil)
        return nullptr;
    return std::make_unique<AppKitInstanceContainer>(parent, m_pBuilder, false);
}

class AppKitInstanceBox : public AppKitInstanceContainer, public virtual weld::Box
{
private:
    NSStackView* const m_pStackView;

public:
    AppKitInstanceBox(NSStackView* pStackView, AppKitInstanceBuilder* pBuilder, bool bTakeOwnership)
        : AppKitInstanceContainer(pStackView, pBuilder, bTakeOwnership)
        , m_pStackView(pStackView)
    {
    }

    virtual void reorder_child(weld::Widget* pWidget, int nNewPosition) override
    {
        AppKitInstanceWidget* pAppKitWidget = dynamic_cast<AppKitInstanceWidget*>(pWidget);
        assert(pAppKitWidget);
        NSView* pChild = pAppKitWidget->getView();
        [pChild removeFromSuperview];
        [m_pStackView insertView:pChild atIndex:nNewPosition inGravity:NSStackViewGravityCenter];
    }
};

class AppKitInstanceWindow : public AppKitInstanceContainer, public virtual weld::Window
{
private:
    NSWindow* m_pWindow;

protected:
    void help();

public:
    AppKitInstanceWindow(NSWindow* pWindow, AppKitInstanceBuilder* pBuilder, bool bTakeOwnership)
        : AppKitInstanceContainer([pWindow contentView], pBuilder, bTakeOwnership)
        , m_pWindow(pWindow)
    {
        // SAL_DEBUG("AppKitInstanceWindow CTOR this=" << this);
    }

    virtual ~AppKitInstanceWindow()
    {
        // SAL_DEBUG("AppKitInstanceWindow DTOR this=" << this);
    }

    virtual void set_title(const OUString& rTitle) override
    {
        [m_pWindow setTitle:CreateNSString(rTitle)];
    }

    virtual OUString get_title() const override
    {
        return GetOUString([m_pWindow title]);
    }

    virtual void set_busy_cursor(bool bBusy) override
    {
        (void) bBusy;
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
    }

    virtual void window_move(int x, int y) override
    {
        (void) x;
        (void) y;
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
    }

    virtual void set_modal(bool bModal) override
    {
        (void) bModal;
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
    }

    virtual bool get_modal() const override
    {
        return [NSApp modalWindow] == m_pWindow;
    }

    virtual bool get_resizable() const override
    {
        return [m_pWindow isResizable];
    }

    virtual Size get_size() const override
    {
        return Size(m_pWindow.frame.size.width, m_pWindow.frame.size.height);
    }

    virtual Point get_position() const override
    {
        return Point(m_pWindow.frame.origin.x, m_pWindow.frame.origin.y);
    }

    virtual tools::Rectangle get_monitor_workarea() const override
    {
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
        return tools::Rectangle();
    }

    virtual void set_centered_on_parent(bool bTrackGeometryRequests) override
    {
        (void) bTrackGeometryRequests;
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
    }

    virtual bool has_toplevel_focus() const override
    {
        return [m_pWindow isKeyWindow];
    }

    virtual void present() override
    {
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
    }

    virtual void set_window_state(const OString& rStr) override
    {
        (void) rStr;
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
    }

    virtual OString get_window_state(WindowStateMask nMask) const override
    {
        if (nMask == WindowStateMask::NONE)
            return "";

        OStringBuffer sBuf(64);

        if (nMask & WindowStateMask::X)
            sBuf.append(OString::number(m_pWindow.frame.origin.x));
        sBuf.append(",");
        if (nMask & WindowStateMask::Y)
            sBuf.append(OString::number(m_pWindow.frame.origin.y));
        sBuf.append(",");
        if (nMask & WindowStateMask::Width)
            sBuf.append(OString::number(m_pWindow.frame.size.width));
        sBuf.append(",");
        if (nMask & WindowStateMask::Height)
            sBuf.append(OString::number(m_pWindow.frame.size.height));
        sBuf.append(";");
        if (nMask & WindowStateMask::State)
        {
            WindowStateState nState = WindowStateState::Normal;
            if ([m_pWindow isMiniaturized])
                nState = WindowStateState::Minimized;
            else if ([m_pWindow isZoomed])
                nState = WindowStateState::Maximized;
            sBuf.append(OString::number((int)nState));
        }
        sBuf.append(";");
        if (nMask & WindowStateMask::MaximizedX)
            sBuf.append("0"); // ???
        sBuf.append(",");
        if (nMask & WindowStateMask::MaximizedY)
            sBuf.append("0"); // ???
        sBuf.append(",");
        if (nMask & WindowStateMask::MaximizedWidth)
            sBuf.append(m_pWindow.maxSize.width); // ???
        sBuf.append(",");
        if (nMask & WindowStateMask::MaximizedHeight)
            sBuf.append(m_pWindow.maxSize.height); // ???
        sBuf.append(";");

        return sBuf.makeStringAndClear();
    }

    virtual css::uno::Reference<css::awt::XWindow> GetXWindow() override
    {
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
        return nullptr;
    }

    virtual SystemEnvData get_system_data() const override
    {
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
        return SystemEnvData();
    }

    virtual void resize_to_request() override
    {
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
    }

    virtual void draw(VirtualDevice& rOutput) override
    {
        (void) rOutput;
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
    }

    virtual weld::ScreenShotCollection collect_screenshot_data() override
    {
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
        return weld::ScreenShotCollection();
    }
};

namespace
{
    class AppKitInstanceDialogCommon : public virtual weld::Dialog
    {
    private:
        virtual bool runAsync(std::shared_ptr<weld::DialogController> rController,
                              const std::function<void(sal_Int32)>& func) override
        {
            (void) rController;
            (void) func;
            SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
            return false;
        }

    public:
        virtual bool runAsync(std::shared_ptr<weld::Dialog> const& rxSelf,
                              const std::function<void(sal_Int32)>& func) override
        {
            (void) rxSelf;
            (void) func;
            SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
            return false;
        }

        virtual void response(int nResponse) override
        {
            NSModalResponse response;
            if (nResponse == RET_OK)
                response = NSModalResponseOK;
            else if (nResponse == RET_CANCEL)
                response = NSModalResponseCancel;
            else
                return;
            SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
        }

        virtual void add_button(const OUString& rText, int response, const OString& rHelpId = OString()) override
        {
            (void) rText;
            (void) response;
            (void) rHelpId;
            SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
        }

        virtual void set_default_response(int response) override
        {
            (void) response;
            SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
        }

        virtual weld::Button* weld_widget_for_response(int response) override
        {
            (void) response;
            SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
            return nullptr;
        }

        virtual weld::Container* weld_content_area() override
        {
            SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
            return nullptr;
        }

        virtual void collapse(weld::Widget* pEdit, weld::Widget* pButton) override
        {
            (void) pEdit;
            (void) pButton;
            SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
        }

        virtual void undo_collapse() override
        {
            SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
        }

        virtual void SetInstallLOKNotifierHdl(const Link<void*, vcl::ILibreOfficeKitNotifier*>& rLink) override
        {
            (void) rLink;
        }
    };
}

class AppKitInstanceDialog : public AppKitInstanceWindow, public virtual AppKitInstanceDialogCommon, public virtual weld::Dialog
{
protected:
    NSPanel* m_pPanel;

public:
    AppKitInstanceDialog(NSPanel* pPanel, AppKitInstanceBuilder* pBuilder, bool bTakeOwnership)
        : AppKitInstanceWindow(pPanel, pBuilder, bTakeOwnership)
        , m_pPanel(pPanel)
    {
        // SAL_DEBUG("AppKitInstanceDialog CTOR this=" << this);
    }

    virtual ~AppKitInstanceDialog()
    {
        // SAL_DEBUG("AppKitInstanceDialog DTOR this=" << this);
    }

    virtual int run() override
    {
        return runModal();
    }
};

class AppKitInstanceMessageDialog final : public AppKitInstanceWindow, public virtual AppKitInstanceDialogCommon, public virtual weld::MessageDialog
{
private:
    NSAlert* m_pAlert;

public:
    AppKitInstanceMessageDialog(NSAlert* pAlert, AppKitInstanceBuilder* pBuilder, bool bTakeOwnership)
        : AppKitInstanceWindow(pAlert.window, pBuilder, bTakeOwnership)
        , m_pAlert(pAlert)
    {
    }

    virtual void set_primary_text(const OUString& rText) override
    {
        [m_pAlert setMessageText:CreateNSString(rText)];
    }

    virtual OUString get_primary_text() const override
    {
        return GetOUString([m_pAlert messageText]);
    }

    virtual void set_secondary_text(const OUString& rText) override
    {
        [m_pAlert setInformativeText:CreateNSString(rText)];
    }

    virtual OUString get_secondary_text() const override
    {
        return GetOUString([m_pAlert informativeText]);
    }

    virtual Container* weld_message_area() override
    {
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
        return nullptr;
    }

    virtual int run() override
    {
        return runModal();
    }
};

class AppKitInstanceFrame final : public AppKitInstanceContainer, public virtual weld::Frame
{
private:
    NSBox* m_pBox;

public:
    AppKitInstanceFrame(NSBox* pBox, AppKitInstanceBuilder* pBuilder, bool bTakeOwnership)
        : AppKitInstanceContainer(pBox, pBuilder, bTakeOwnership)
        , m_pBox(pBox)
    {
        // SAL_DEBUG("AppKitInstanceFrame CTOR this=" << this);
    }

    virtual ~AppKitInstanceFrame()
    {
        // SAL_DEBUG("AppKitInstanceFrame DTOR this=" << this);
    }

    virtual void set_label(const OUString& rText) override
    {
        [m_pBox setTitle:CreateNSString(rText)];
    }

    virtual OUString get_label() const override
    {
        return GetOUString([m_pBox title]);
    }

    virtual std::unique_ptr<weld::Label> weld_label_widget() const override
    {
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
        return nullptr;
    }
};

class AppKitInstanceNotebook final : public AppKitInstanceContainer, public virtual weld::Notebook
{
private:
    NSTabView* m_pTabView;
    mutable std::vector<std::unique_ptr<AppKitInstanceContainer>> m_aPages;

    int get_page_number(const OString& rIdent) const
    {
        NSString* string = [NSString stringWithUTF8String:rIdent.getStr()];
        NSInteger index = [m_pTabView indexOfTabViewItemWithIdentifier:string];
        if (index == NSNotFound)
            return -1;
        return index;
    }

public:
    AppKitInstanceNotebook(NSTabView* pTabView, AppKitInstanceBuilder* pBuilder, bool bTakeOwnership)
        : AppKitInstanceContainer(pTabView, pBuilder, bTakeOwnership)
        , m_pTabView(pTabView)
    {
    }

    virtual int get_current_page() const override
    {
        return [m_pTabView indexOfTabViewItem:[m_pTabView selectedTabViewItem]];
    }

    OString get_page_ident(int nPage) const override
    {
        if (nPage > [m_pTabView numberOfTabViewItems])
            return "";
        NSTabViewItem* item = [m_pTabView tabViewItemAtIndex:nPage];
        id ident = [item identifier];
        if (ident == nil)
            return "";
        NSString* string = [NSString castFrom:ident];
        if (string == nil)
            return "";
        const char* p = [string UTF8String];
        return OString(p, strlen(p));
    }

    virtual OString get_current_page_ident() const override
    {
        return get_page_ident(get_current_page());
    }

    virtual void set_current_page(int nPage) override
    {
        [m_pTabView selectTabViewItemAtIndex:nPage];
    }

    virtual void set_current_page(const OString& rIdent) override
    {
        [m_pTabView selectTabViewItemWithIdentifier:[NSString stringWithUTF8String:rIdent.getStr()]];
    }

    virtual void remove_page(const OString& rIdent) override
    {
        [m_pTabView removeTabViewItem:[m_pTabView tabViewItemAtIndex:[m_pTabView indexOfTabViewItemWithIdentifier:[NSString stringWithUTF8String:rIdent.getStr()]]]];
    }

    virtual void insert_page(const OString& rIdent, const OUString& rLabel, int nPos) override
    {
        NSTabViewItem* item = [[NSTabViewItem alloc] initWithIdentifier:[NSString stringWithUTF8String:rIdent.getStr()]];
        [item setLabel:CreateNSString(rLabel)];
        [m_pTabView insertTabViewItem:item atIndex:nPos];
    }

    virtual void set_tab_label_text(const OString& rIdent, const OUString& rLabel) override
    {
        [[m_pTabView tabViewItemAtIndex:[m_pTabView indexOfTabViewItemWithIdentifier:[NSString stringWithUTF8String:rIdent.getStr()]]] setLabel:CreateNSString(rLabel)];
    }

    virtual OUString get_tab_label_text(const OString& rIdent) const override
    {
        return GetOUString([[m_pTabView tabViewItemAtIndex:[m_pTabView indexOfTabViewItemWithIdentifier:[NSString stringWithUTF8String:rIdent.getStr()]]] label]);
    }

    virtual int get_n_pages() const override
    {
        return [m_pTabView numberOfTabViewItems];
    }

    virtual weld::Container* get_page(const OString& rIdent) const override
    {
        int nPage = get_page_number(rIdent);
        if (nPage < 0)
            return nullptr;
        NSTabViewItem* pChild = [m_pTabView tabViewItemAtIndex:nPage];
        unsigned int nPageIndex = static_cast<unsigned int>(nPage);
        if (m_aPages.size() < nPageIndex + 1)
            m_aPages.resize(nPageIndex + 1);
        if (!m_aPages[nPageIndex])
            m_aPages[nPageIndex].reset(new AppKitInstanceContainer([pChild view], m_pBuilder, false));
        return m_aPages[nPageIndex].get();
    }

    virtual ~AppKitInstanceNotebook() override
    {
    }
};

class AppKitInstanceButton : public AppKitInstanceContainer, public virtual weld::Button
{
protected:
    NSButton* m_pButton;

public:
    AppKitInstanceButton(NSButton* pButton, AppKitInstanceBuilder* pBuilder, bool bTakeOwnership)
        : AppKitInstanceContainer(pButton, pBuilder, bTakeOwnership)
        , m_pButton(pButton)
    {
    }

    virtual void set_label(const OUString& rText) override
    {
        [m_pButton setTitle:CreateNSString(rText)];
    }

    virtual void set_image(VirtualDevice* pDevice) override
    {
        (void) pDevice;
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
    }

    virtual void set_image(const css::uno::Reference<css::graphic::XGraphic>& rImage) override
    {
        (void) rImage;
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
    }

    virtual void set_from_icon_name(const OUString& rIconName) override
    {
        (void) rIconName;
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
    }

    virtual OUString get_label() const override
    {
        return GetOUString([m_pButton title]);
    }

    virtual void set_label_line_wrap(bool wrap) override
    {
        (void) wrap;
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
    }

    virtual ~AppKitInstanceButton() override
    {
    }
};

class AppKitInstanceToggleButton : public AppKitInstanceButton, public virtual weld::ToggleButton
{
public:
    AppKitInstanceToggleButton(NSButton* pButton, AppKitInstanceBuilder* pBuilder, bool bTakeOwnership)
        : AppKitInstanceButton(pButton, pBuilder, bTakeOwnership)
    {
        [m_pButton setButtonType:NSButtonTypeToggle];
    }

    virtual void set_active(bool active) override
    {
        [m_pButton setState:(active ? NSOnState : NSOffState)];
    }

    virtual bool get_active() const override
    {
        return [m_pButton state] == NSOnState;
    }

    virtual void set_inconsistent(bool inconsistent) override
    {
        [m_pButton setAllowsMixedState:inconsistent];
        if (inconsistent)
            [m_pButton setState:NSMixedState];
    }

    virtual bool get_inconsistent() const override
    {
        return [m_pButton state] == NSMixedState;
    }

    virtual ~AppKitInstanceToggleButton() override
    {
    }
};

class AppKitInstanceRadioButton final : public AppKitInstanceToggleButton, public virtual weld::RadioButton
{
public:
    AppKitInstanceRadioButton(NSButton* pButton, AppKitInstanceBuilder* pBuilder, bool bTakeOwnership)
        : AppKitInstanceToggleButton(pButton, pBuilder, bTakeOwnership)
    {
        [m_pButton setButtonType:NSButtonTypeRadio];
    }
};

class AppKitInstanceCheckButton final : public AppKitInstanceToggleButton, public virtual weld::CheckButton
{
public:
    AppKitInstanceCheckButton(NSButton* pButton, AppKitInstanceBuilder* pBuilder, bool bTakeOwnership)
        : AppKitInstanceToggleButton(pButton, pBuilder, bTakeOwnership)
    {
        [m_pButton setButtonType:NSButtonTypeSwitch];
    }
};

class AppKitInstanceEntry : public AppKitInstanceWidget, public virtual weld::Entry
{
private:
    NSTextField* m_pTextField;

public:
    AppKitInstanceEntry(NSTextField* pTextField, AppKitInstanceBuilder* pBuilder, bool bTakeOwnership)
        : AppKitInstanceWidget(pTextField, pBuilder, bTakeOwnership)
        , m_pTextField(pTextField)
    {
    }

    virtual void set_text(const OUString& rText) override
    {
        [m_pTextField setStringValue:CreateNSString(rText)];
    }

    virtual OUString get_text() const override
    {
        return GetOUString([m_pTextField stringValue]);
    }

    virtual void set_width_chars(int nChars) override
    {
        (void) nChars;
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
    }

    virtual int get_width_chars() const override
    {
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
        return 10;
    }

    virtual void set_max_length(int nChars) override
    {
        (void) nChars;
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
    }

    virtual void select_region(int nStartPos, int nEndPos) override
    {
        (void) nStartPos;
        (void) nEndPos;
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
    }

    virtual bool get_selection_bounds(int& rStartPos, int& rEndPos) override
    {
        (void) rStartPos;
        (void) rEndPos;
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
        return true;
    }

    virtual void replace_selection(const OUString& rText) override
    {
        (void) rText;
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
    }

    virtual void set_position(int nCursorPos) override
    {
        (void) nCursorPos;
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
    }

    virtual int get_position() const override
    {
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
        return 0;
    }

    virtual void set_editable(bool bEditable) override
    {
        (void) bEditable;
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
    }

    virtual bool get_editable() const override
    {
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
        return true;
    }

    virtual void set_message_type(weld::EntryMessageType eType) override
    {
        (void) eType;
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
    }

    virtual void set_placeholder_text(const OUString& rText) override
    {
        (void) rText;
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
    }

    virtual void set_font(const vcl::Font& rFont) override
    {
        (void) rFont;
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
    }

    virtual void cut_clipboard() override
    {
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
    }

    virtual void copy_clipboard() override
    {
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
    }

    virtual void paste_clipboard() override
    {
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
    }

    virtual ~AppKitInstanceEntry() override
    {
    }
};

// Just a first rough idea, probably totally wrong

struct AppKitInstanceTreeIter final : public weld::TreeIter
{
    virtual bool equal(const TreeIter& rOther) const override
    {
        const AppKitInstanceTreeIter& b = static_cast<const AppKitInstanceTreeIter&>(rOther);
        return row == b.row && col == b.col;
    }
    NSInteger row, col;
};

class AppKitInstanceTreeView final : public AppKitInstanceContainer, public virtual weld::TreeView
{
private:
    NSOutlineView* m_pOutlineView;

public:
    AppKitInstanceTreeView(NSOutlineView* pOutlineView, AppKitInstanceBuilder* pBuilder, bool bTakeOwnership)
        : AppKitInstanceContainer(pOutlineView, pBuilder, bTakeOwnership)
        , m_pOutlineView(pOutlineView)
    {
    }

    virtual void insert(const weld::TreeIter* pParent, int pos, const OUString* pStr, const OUString* pId,
                        const OUString* pIconName, VirtualDevice* pImageSurface,
                        const OUString* pExpanderName, bool bChildrenOnDemand, weld::TreeIter* pRet) override
    {
        (void) pParent;
        (void) pos;
        (void) pStr;
        (void) pId;
        (void) pIconName;
        (void) pImageSurface;
        (void) pExpanderName;
        (void) bChildrenOnDemand;
        (void) pRet;
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
    }

    virtual OUString get_selected_text() const override
    {
        // ???
        return GetOUString([m_pOutlineView stringValue]);
    }

    virtual OUString get_selected_id() const override
    {
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
        return "";
    }

    virtual int get_selected_index() const override
    {
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
        return 0;
    }

    virtual void select(int pos) override
    {
        (void) pos;
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
    }

    virtual void unselect(int pos) override
    {
        (void) pos;
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
    }

    virtual void remove(int pos) override
    {
        (void) pos;
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
    }

    virtual OUString get_text(int row, int col = -1) const override
    {
        (void) row;
        (void) col;
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
        return "";
    }

    virtual void set_text(int row, const OUString& rText, int col = -1) override
    {
        (void) row;
        (void) rText;
        (void) col;
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
    }

    using AppKitInstanceWidget::set_sensitive;

    virtual void set_sensitive(int row, bool bSensitive, int col = -1) override
    {
        (void) row;
        (void) bSensitive;
        (void) col;
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
    }

    virtual void set_id(int row, const OUString& rId) override
    {
        (void) row;
        (void) rId;
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
    }

    virtual void set_toggle(int row, TriState eState, int col) override
    {
        (void) row;
        (void) eState;
        (void) col;
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
    }

    virtual TriState get_toggle(int row, int col) const override
    {
        (void) row;
        (void) col;
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
        return TRISTATE_FALSE;
    }

    virtual void set_image(int row, const OUString& rImage, int col = -1) override
    {
        (void) row;
        (void) rImage;
        (void) col;
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
    }

    virtual void set_image(int row, VirtualDevice& rImage, int col = -1) override
    {
        (void) row;
        (void) rImage;
        (void) col;
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
    }

    virtual void set_image(int row, const css::uno::Reference<css::graphic::XGraphic>& rImage,
                           int col = -1) override
    {
        (void) row;
        (void) rImage;
        (void) col;
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
    }

    virtual void set_text_emphasis(int row, bool bOn, int col) override
    {
        (void) row;
        (void) bOn;
        (void) col;
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
    }

    virtual bool get_text_emphasis(int row, int col) const override
    {
        (void) row;
        (void) col;
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
        return false;
    }

    virtual void swap(int pos1, int pos2) override
    {
        (void) pos1;
        (void) pos2;
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
    }

    virtual std::vector<int> get_selected_rows() const override
    {
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
        return std::vector<int>();
    }

    virtual void set_font_color(int pos, const Color& rColor) override
    {
        (void) pos;
        (void) rColor;
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
    }

    virtual void scroll_to_row(int pos) override
    {
        (void) pos;
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
    }

    virtual bool is_selected(int pos) const override
    {
        (void) pos;
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
        return false;
    }

    virtual int get_cursor_index() const override
    {
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
        return 0;
    }

    virtual void set_cursor(int pos) override
    {
        (void) pos;
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
    }

    virtual int find_text(const OUString& rText) const override
    {
        (void) rText;
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
        return 0;
    }

    virtual OUString get_id(int pos) const override
    {
        (void) pos;
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
        return "";
    }

    virtual int find_id(const OUString& rId) const override
    {
        (void) rId;
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
        return 0;
    }

    virtual std::unique_ptr<weld::TreeIter> make_iterator(const weld::TreeIter* pOrig = nullptr) const override
    {
        (void) pOrig;
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
        return nullptr;
    }

    virtual void copy_iterator(const weld::TreeIter& rSource, weld::TreeIter& rDest) const override
    {
        (void) rSource;
        (void) rDest;
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
    }

    virtual bool get_selected(weld::TreeIter* pIter) const override
    {
        (void) pIter;
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
        return false;
    }

    virtual bool get_cursor(weld::TreeIter* pIter) const override
    {
        (void) pIter;
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
        return false;
    }

    virtual void set_cursor(const weld::TreeIter& rIter) override
    {
        (void) rIter;
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
    }

    virtual bool get_iter_first(weld::TreeIter& rIter) const override
    {
        (void) rIter;
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
        return false;
    }

    virtual bool iter_previous_sibling(weld::TreeIter& rIter) const override
    {
        (void) rIter;
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
        return false;
    }

    virtual bool iter_next_sibling(weld::TreeIter& rIter) const override
    {
        (void) rIter;
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
        return false;
    }

    virtual bool iter_next(weld::TreeIter& rIter) const override
    {
        (void) rIter;
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
        return false;
    }

    virtual bool iter_previous(weld::TreeIter& rIter) const override
    {
        (void) rIter;
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
        return false;
    }

    virtual bool iter_children(weld::TreeIter& rIter) const override
    {
        (void) rIter;
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
        return false;
    }

    virtual bool iter_parent(weld::TreeIter& rIter) const override
    {
        (void) rIter;
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
        return false;
    }

    virtual int get_iter_depth(const weld::TreeIter& rIter) const override
    {
        (void) rIter;
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
        return 0;
    }

    virtual int get_iter_index_in_parent(const weld::TreeIter& rIter) const override
    {
        (void) rIter;
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
        return 0;
    }

    virtual int iter_compare(const weld::TreeIter& a, const weld::TreeIter& b) const override
    {
        (void) a;
        (void) b;
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
        return 0;
    }

    virtual bool iter_has_child(const weld::TreeIter& rIter) const override
    {
        (void) rIter;
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
        return false;
    }

    virtual int iter_n_children(const weld::TreeIter& rIter) const override
    {
        (void) rIter;
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
        return 0;
    }

    virtual void remove(const weld::TreeIter& rIter) override
    {
        (void) rIter;
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
    }

    virtual void select(const weld::TreeIter& rIter) override
    {
        (void) rIter;
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
    }

    virtual void unselect(const weld::TreeIter& rIter) override
    {
        (void) rIter;
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
    }

    virtual bool get_row_expanded(const weld::TreeIter& rIter) const override
    {
        (void) rIter;
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
        return false;
    }

    virtual bool get_children_on_demand(const weld::TreeIter& rIter) const override
    {
        (void) rIter;
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
        return false;
    }

    virtual void set_children_on_demand(const weld::TreeIter& rIter, bool bChildrenOnDemand) override
    {
        (void) rIter;
        (void) bChildrenOnDemand;
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
    }

    virtual void expand_row(const weld::TreeIter& rIter) override
    {
        (void) rIter;
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
    }

    virtual void collapse_row(const weld::TreeIter& rIter) override
    {
        (void) rIter;
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
    }

    virtual void set_text(const weld::TreeIter& rIter, const OUString& rStr, int col) override
    {
        (void) rIter;
        (void) rStr;
        (void) col;
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
    }

    virtual void set_sensitive(const weld::TreeIter& rIter, bool bSensitive, int col) override
    {
        (void) rIter;
        (void) bSensitive;
        (void) col;
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
    }

    virtual void set_image(const weld::TreeIter& rIter, const OUString& rImage, int col) override
    {
        (void) rIter;
        (void) rImage;
        (void) col;
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
    }

    virtual void set_text_emphasis(const weld::TreeIter& rIter, bool bOn, int col) override
    {
        (void) rIter;
        (void) bOn;
        (void) col;
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
    }

    virtual bool get_text_emphasis(const weld::TreeIter& rIter, int col) const override
    {
        (void) rIter;
        (void) col;
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
        return false;
    }

    virtual void set_toggle(const weld::TreeIter& rIter, TriState bOn, int col) override
    {
        (void) rIter;
        (void) bOn;
        (void) col;
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
    }

    virtual TriState get_toggle(const weld::TreeIter& rIter, int col) const override
    {
        (void) rIter;
        (void) col;
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
        return TRISTATE_FALSE;
    }

    virtual OUString get_text(const weld::TreeIter& rIter, int col = -1) const override
    {
        (void) rIter;
        (void) col;
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
        return "";
    }

    virtual void set_id(const weld::TreeIter& rIter, const OUString& rId) override
    {
        (void) rIter;
        (void) rId;
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
    }

    virtual OUString get_id(const weld::TreeIter& rIter) const override
    {
        (void) rIter;
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
        return "";
    }

    virtual void set_image(const weld::TreeIter& rIter, VirtualDevice& rImage, int col) override
    {
        (void) rIter;
        (void) rImage;
        (void) col;
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
    }

    virtual void set_image(const weld::TreeIter& rIter,
                           const css::uno::Reference<css::graphic::XGraphic>& rImage, int col) override
    {
        (void) rIter;
        (void) rImage;
        (void) col;
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
    }

    virtual void set_font_color(const weld::TreeIter& rIter, const Color& rColor) override
    {
        (void) rIter;
        (void) rColor;
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
    }

    virtual void scroll_to_row(const weld::TreeIter& rIter) override
    {
        (void) rIter;
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
    }

    virtual bool is_selected(const weld::TreeIter& rIter) const override
    {
        (void) rIter;
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
        return false;
    }

    virtual void move_subtree(weld::TreeIter& rNode, const weld::TreeIter* pNewParent, int nIndexInNewParent) override
    {
        (void) rNode;
        (void) pNewParent;
        (void) nIndexInNewParent;
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
    }

    virtual void all_foreach(const std::function<bool(weld::TreeIter&)>& func) override
    {
        (void) func;
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
    }

    virtual void selected_foreach(const std::function<bool(weld::TreeIter&)>& func) override
    {
        (void) func;
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
    }

    virtual void visible_foreach(const std::function<bool(weld::TreeIter&)>& func) override
    {
        (void) func;
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
    }

    virtual void bulk_insert_for_each(int nSourceCount,
                                      const std::function<void(weld::TreeIter&, int nSourceIndex)>& func,
                                      const std::vector<int>* pFixedWidths = nullptr) override
    {
        (void) nSourceCount;
        (void) func;
        (void) pFixedWidths;
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
    }

    virtual void start_editing(const weld::TreeIter& rEntry) override
    {
        (void) rEntry;
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
    }

    virtual void end_editing() override
    {
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
    }

    virtual void enable_drag_source(rtl::Reference<TransferDataContainer>& rTransferrable,
                                    sal_uInt8 eDNDConstants) override
    {
        (void) rTransferrable;
        (void) eDNDConstants;
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
    }

    virtual int n_children() const override
    {
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
        return 0;
    }

    virtual void make_sorted() override
    {
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
    }

    virtual void make_unsorted() override
    {
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
    }

    virtual bool get_sort_order() const override
    {
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
        return false;
    }

    virtual void set_sort_order(bool bAscending) override
    {
        (void) bAscending;
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
    }

    virtual void set_sort_indicator(TriState eState, int nColumn = -1) override
    {
        (void) eState;
        (void) nColumn;
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
    }

    virtual TriState get_sort_indicator(int nColumn = -1) const override
    {
        (void) nColumn;
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
        return TRISTATE_FALSE;
    }

    virtual int get_sort_column() const override
    {
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
        return 0;
    }

    virtual void set_sort_column(int nColumn) override
    {
        (void) nColumn;
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
    }

    virtual void clear() override
    {
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
    }

    virtual int get_height_rows(int nRows) const override
    {
        (void) nRows;
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
        return 0;
    }

    virtual void columns_autosize() override
    {
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
    }

    virtual void set_column_fixed_widths(const std::vector<int>& rWidths) override
    {
        (void) rWidths;
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
    }

    virtual void set_column_editables(const std::vector<bool>& rEditables) override
    {
        (void) rEditables;
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
    }

    virtual int get_column_width(int nCol) const override
    {
        (void) nCol;
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
        return 0;
    }

    virtual void set_centered_column(int nCol) override
    {
        (void) nCol;
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
    }

    virtual OUString get_column_title(int nColumn) const override
    {
        (void) nColumn;
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
        return "";
    }

    virtual void set_column_title(int nColumn, const OUString& rTitle) override
    {
        (void) nColumn;
        (void) rTitle;
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
    }

    virtual void set_selection_mode(SelectionMode eMode) override
    {
        (void) eMode;
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
    }

    virtual int count_selected_rows() const override
    {
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
        return 0;
    }

    virtual void remove_selection() override
    {
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
    }

    virtual void vadjustment_set_value(int value) override
    {
        (void) value;
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
    }

    virtual int vadjustment_get_value() const override
    {
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
        return 0;
    }

    virtual bool get_dest_row_at_pos(const Point& rPos, weld::TreeIter* pResult) override
    {
        (void) rPos;
        (void) pResult;
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
        return false;
    }

    virtual tools::Rectangle get_row_area(const weld::TreeIter& rIter) const override
    {
        (void) rIter;
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
        return tools::Rectangle();
    }

    virtual weld::TreeView* get_drag_source() const override
    {
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
        return nullptr;
    }

    virtual ~AppKitInstanceTreeView() override
    {
    }
};

class AppKitInstanceSpinButton final : public AppKitInstanceEntry, public virtual weld::SpinButton
{
private:
    NSStepper* m_pStepper;

public:
    AppKitInstanceSpinButton(NSTextField* pTextField, NSStepper* pStepper, AppKitInstanceBuilder* pBuilder, bool bTakeOwnership)
        : AppKitInstanceEntry(pTextField, pBuilder, bTakeOwnership)
        , m_pStepper(pStepper)
    {
    }

    virtual void set_value(int value) override
    {
        [m_pStepper setIntValue:value];
    }

    virtual int get_value() const override
    {
        return [m_pStepper intValue];
    }

    virtual void set_range(int min, int max) override
    {
        [m_pStepper setMinValue:min];
        [m_pStepper setMaxValue:max];
    }

    virtual void get_range(int& min, int& max) const override
    {
        min = [m_pStepper minValue];
        max = [m_pStepper maxValue];
    }

    virtual void set_increments(int step, int page) override
    {
        (void) page;
        [m_pStepper setIncrement:step];
    }

    virtual void get_increments(int& step, int& page) const override
    {
        step = [m_pStepper increment];
        page = step;
    }

    virtual void set_digits(unsigned int digits) override
    {
        (void) digits;
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
    }

    virtual unsigned int get_digits() const override
    {
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
        return 1;
    }

    virtual ~AppKitInstanceSpinButton() override
    {
    }
};

class AppKitInstanceLabel final : public AppKitInstanceWidget, public virtual weld::Label
{
private:
    NSTextField* m_pTextField;

public:
    AppKitInstanceLabel(NSTextField* pTextField, AppKitInstanceBuilder* pBuilder, bool bTakeOwnership)
        : AppKitInstanceWidget(pTextField, pBuilder, bTakeOwnership)
        , m_pTextField(pTextField)
    {
        // SAL_DEBUG("AppKitInstanceLabel CTOR this=" << this);
    }

    virtual ~AppKitInstanceLabel()
    {
        // SAL_DEBUG("AppKitInstanceLabel DTOR this=" << this);
    }

    virtual void set_label(const OUString& rText) override
    {
        [m_pTextField setStringValue:CreateNSString(rText)];
    }

    virtual OUString get_label() const override
    {
        return GetOUString([m_pTextField stringValue]);
    }

    virtual void set_mnemonic_widget(Widget* pTarget) override
    {
        (void) pTarget;
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
    }

    virtual void set_message_type(weld::EntryMessageType eType) override
    {
        (void) eType;
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
    }

    virtual void set_font(const vcl::Font& rFont) override
    {
        (void) rFont;
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
    }
};

class AppKitInstanceComboBox final : public AppKitInstanceContainer, public vcl::ISearchableStringList, public virtual weld::ComboBox
{
private:
    NSComboBox* m_pComboBox;

public:
    AppKitInstanceComboBox(NSComboBox* pComboBox, AppKitInstanceBuilder* pBuilder, bool bTakeOwnership)
        : AppKitInstanceContainer(pComboBox, pBuilder, bTakeOwnership)
    {
    }

    virtual void insert(int pos, const OUString& rStr, const OUString* pId,
                        const OUString* pIconName, VirtualDevice* pImageSurface) override
    {
        (void) pos;
        (void) rStr;
        (void) pId;
        (void) pIconName;
        (void) pImageSurface;
        (void) m_pComboBox;
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
    }

    virtual void insert_vector(const std::vector<weld::ComboBoxEntry>& rItems, bool bKeepExisting) override
    {
        (void) rItems;
        (void) bKeepExisting;
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
    }

    virtual void insert_separator(int pos, const OUString& rId) override
    {
        (void) pos;
        (void) rId;
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
    }

    virtual int get_count() const override
    {
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
        return 0;
    }

    virtual void make_sorted() override
    {
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
    }

    virtual void clear() override
    {
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
    }

    virtual int get_active() const override
    {
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
        return 0;
    }

    virtual void set_active(int pos) override
    {
        (void) pos;
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
    }

    virtual void remove(int pos) override
    {
        (void) pos;
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
    }

    virtual OUString get_active_text() const override
    {
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
        return "";
    }

    virtual OUString get_text(int pos) const override
    {
        (void) pos;
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
        return "";
    }

    virtual int find_text(const OUString& rStr) const override
    {
        (void) rStr;
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
        return 0;
    }

    virtual OUString get_active_id() const override
    {
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
        return "";
    }

    virtual void set_active_id(const OUString& rStr) override
    {
        (void) rStr;
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
    }

    virtual OUString get_id(int pos) const override
    {
        (void) pos;
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
        return "";
    }

    virtual void set_id(int row, const OUString& rId) override
    {
        (void) row;
        (void) rId;
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
    }

    virtual int find_id(const OUString& rId) const override
    {
        (void) rId;
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
        return 0;
    }

    virtual bool changed_by_direct_pick() const override
    {
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
        return false;
    }

    virtual bool has_entry() const override
    {
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
        return false;
    }

    virtual void set_entry_message_type(weld::EntryMessageType eType) override
    {
        (void) eType;
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
    }

    virtual void set_entry_text(const OUString& rStr) override
    {
        (void) rStr;
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
    }

    virtual void set_entry_width_chars(int nChars) override
    {
        (void) nChars;
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
    }

    virtual void set_entry_max_length(int nChars) override
    {
        (void) nChars;
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
    }

    virtual void select_entry_region(int nStartPos, int nEndPos) override
    {
        (void) nStartPos;
        (void) nEndPos;
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
    }

    virtual bool get_entry_selection_bounds(int& rStartPos, int& rEndPos) override
    {
        (void) rStartPos;
        (void) rEndPos;
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
        return false;
    }

    virtual void set_entry_completion(bool bEnable, bool bCaseSensitive = false) override
    {
        (void) bEnable;
        (void) bCaseSensitive;
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
    }

    virtual void set_entry_placeholder_text(const OUString& rText) override
    {
        (void) rText;
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
    }

    virtual bool get_popup_shown() const override
    {
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
        return false;
    }

    virtual vcl::StringEntryIdentifier CurrentEntry(OUString& _out_entryText) const override
    {
        (void) _out_entryText;
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
        return nullptr;
    }

    virtual vcl::StringEntryIdentifier NextEntry(vcl::StringEntryIdentifier _currentEntry, OUString& _out_entryText) const override
    {
        (void) _currentEntry;
        (void) _out_entryText;
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
        return nullptr;
    }

    virtual void SelectEntry(vcl::StringEntryIdentifier _entry) override
    {
        (void) _entry;
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
    }
};

class AppKitInstanceBuilder : public weld::Builder
{
private:
    NSView* m_pParent;

    ResHookProc const m_pStringReplace;

    std::map<OString, OUString> m_aDeferredProperties;

    AppKitInstanceBuilder(const AppKitInstanceBuilder&) = delete;
    AppKitInstanceBuilder& operator=(const AppKitInstanceBuilder&) = delete;

    struct PackingData
    {
        bool m_bVerticalOrient;
        sal_Int32 m_nPosition;
        PackingData(bool bVerticalOrient = false)
            : m_bVerticalOrient(bVerticalOrient)
            , m_nPosition(-1)
        {
        }
    };

    struct IdAndView
    {
        OString m_sId;
        NSView* m_pView;
        PackingData m_aPackingData;
        IdAndView(const OString &rId, NSView* pView, bool bVertical)
            : m_sId(rId)
            , m_pView(pView)
            , m_aPackingData(bVertical)
        {
        }
    };
    std::vector<IdAndView> m_aChildren;

    struct StringPair
    {
        OString const m_sID;
        OString const m_sValue;
        StringPair(const OString &rId, const OString &rValue)
            : m_sID(rId)
            , m_sValue(rValue)
        {
        }
    };

    struct UStringPair
    {
        OString m_sID;
        OUString m_sValue;
        UStringPair(const OString &rId, const OUString &rValue)
            : m_sID(rId)
            , m_sValue(rValue)
        {
        }
    };

    typedef StringPair RadioButtonGroupMap;
    typedef UStringPair MnemonicWidgetMap;

    std::locale m_aResLocale;

    std::vector<RadioButtonGroupMap> m_aGroupMaps;

    std::map<OString, std::map<OString, OUString>> m_aAdjustments;

    std::vector<MnemonicWidgetMap> m_aMnemonicWidgetMaps;

public:
    AppKitInstanceBuilder(NSView* pParent, const OUString& rUIRoot, const OUString& rUIFile)
        : weld::Builder(rUIFile)
        , m_pParent(pParent)
        , m_pStringReplace(Translate::GetReadStringHook())
    {
        (void) m_pParent;

        OUString sUri(rUIRoot + rUIFile);
        SAL_DEBUG("AppKitInstanceBuilder: " << sUri);
        xmlreader::XmlReader reader(sUri);

        handleChild(pParent, reader);
    }

    virtual std::unique_ptr<weld::MessageDialog> weld_message_dialog(const OString &id, bool bTakeOwnership) override
    {
        (void) id;
        (void) bTakeOwnership;
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
        return nullptr;
    }

    virtual std::unique_ptr<weld::AboutDialog> weld_about_dialog(const OString &id, bool bTakeOwnership) override
    {
        (void) id;
        (void) bTakeOwnership;
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
        return nullptr;
    }

    virtual std::unique_ptr<weld::Assistant> weld_assistant(const OString &id, bool bTakeOwnership) override
    {
        (void) id;
        (void) bTakeOwnership;
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
        return nullptr;
    }

    virtual std::unique_ptr<weld::Dialog> weld_dialog(const OString &rId, bool bTakeOwnership) override
    {
        // This is apparently where we need to parse the .ui file? No,
        // wait, we do that in the AppKitInstanceBuilder ctor.

        for (const auto &i : m_aChildren)
        {
            if (i.m_sId == rId)
                return std::make_unique<AppKitInstanceDialog>(i.m_pView.window, this, bTakeOwnership);
        }

        SAL_WARN("vcl.osx.weld", "returning nullptr: " << OSL_THIS_FUNC);

        return nullptr;

#if 0
        Dialog* pDialog = m_xBuilder->get<Dialog>(id);
        std::unique_ptr<weld::Dialog> pRet(pDialog ? new SalInstanceDialog(pDialog, this, false) : nullptr);
        if (bTakeOwnership && pDialog)
        {
            assert(!m_aOwnedToplevel && "only one toplevel per .ui allowed");
            m_aOwnedToplevel.set(pDialog);
            m_xBuilder->drop_ownership(pDialog);
        }

        return pRet;
#endif
    }

    virtual std::unique_ptr<weld::Window> create_screenshot_window() override
    {
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
        return nullptr;
    }

    virtual std::unique_ptr<weld::Window> weld_window(const OString &id, bool bTakeOwnership) override
    {
        (void) id;
        (void) bTakeOwnership;
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
        return nullptr;
    }

    virtual std::unique_ptr<weld::Widget> weld_widget(const OString &id, bool bTakeOwnership) override
    {
        (void) id;
        (void) bTakeOwnership;
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
        return nullptr;
    }

    virtual std::unique_ptr<weld::Container> weld_container(const OString &id, bool bTakeOwnership) override
    {
        (void) id;
        (void) bTakeOwnership;
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
        return nullptr;
    }

    virtual std::unique_ptr<weld::Box> weld_box(const OString &id, bool bTakeOwnership) override
    {
        (void) id;
        (void) bTakeOwnership;
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
        return nullptr;
    }

    virtual std::unique_ptr<weld::Frame> weld_frame(const OString &id, bool bTakeOwnership) override
    {
        (void) id;
        (void) bTakeOwnership;
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
        return nullptr;
    }

    virtual std::unique_ptr<weld::ScrolledWindow> weld_scrolled_window(const OString &id, bool bTakeOwnership) override
    {
        (void) id;
        (void) bTakeOwnership;
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
        return nullptr;
    }

    virtual std::unique_ptr<weld::Notebook> weld_notebook(const OString &id, bool bTakeOwnership) override
    {
        (void) id;
        (void) bTakeOwnership;
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
        return nullptr;
    }

    virtual std::unique_ptr<weld::Button> weld_button(const OString &id, bool bTakeOwnership) override
    {
        (void) id;
        (void) bTakeOwnership;
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
        return nullptr;
    }

    virtual std::unique_ptr<weld::MenuButton> weld_menu_button(const OString &id, bool bTakeOwnership) override
    {
        (void) id;
        (void) bTakeOwnership;
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
        return nullptr;
    }

    virtual std::unique_ptr<weld::LinkButton> weld_link_button(const OString &id, bool bTakeOwnership) override
    {
        (void) id;
        (void) bTakeOwnership;
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
        return nullptr;
    }

    virtual std::unique_ptr<weld::ToggleButton> weld_toggle_button(const OString &id, bool bTakeOwnership) override
    {
        (void) id;
        (void) bTakeOwnership;
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
        return nullptr;
    }

    virtual std::unique_ptr<weld::RadioButton> weld_radio_button(const OString &id, bool bTakeOwnership) override
    {
        (void) id;
        (void) bTakeOwnership;
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
        return nullptr;
    }

    virtual std::unique_ptr<weld::CheckButton> weld_check_button(const OString &id, bool bTakeOwnership) override
    {
        (void) id;
        (void) bTakeOwnership;
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
        return nullptr;
    }

    virtual std::unique_ptr<weld::Scale> weld_scale(const OString &id, bool bTakeOwnership) override
    {
        (void) id;
        (void) bTakeOwnership;
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
        return nullptr;
    }

    virtual std::unique_ptr<weld::ProgressBar> weld_progress_bar(const OString &id, bool bTakeOwnership) override
    {
        (void) id;
        (void) bTakeOwnership;
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
        return nullptr;
    }

    virtual std::unique_ptr<weld::Spinner> weld_spinner(const OString &id, bool bTakeOwnership) override
    {
        (void) id;
        (void) bTakeOwnership;
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
        return nullptr;
    }

    virtual std::unique_ptr<weld::Image> weld_image(const OString &id, bool bTakeOwnership) override
    {
        (void) id;
        (void) bTakeOwnership;
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
        return nullptr;
    }

    virtual std::unique_ptr<weld::Calendar> weld_calendar(const OString &id, bool bTakeOwnership) override
    {
        (void) id;
        (void) bTakeOwnership;
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
        return nullptr;
    }

    virtual std::unique_ptr<weld::Entry> weld_entry(const OString &id, bool bTakeOwnership) override
    {
        (void) id;
        (void) bTakeOwnership;
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
        return nullptr;
    }

    virtual std::unique_ptr<weld::SpinButton> weld_spin_button(const OString &id, bool bTakeOwnership) override
    {
        (void) id;
        (void) bTakeOwnership;
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
        return nullptr;
    }

    virtual std::unique_ptr<weld::MetricSpinButton> weld_metric_spin_button(const OString& id, FieldUnit eUnit,
                                                                      bool bTakeOwnership) override
    {
        return std::make_unique<weld::MetricSpinButton>(weld_spin_button(id, bTakeOwnership), eUnit);
    }

    virtual std::unique_ptr<weld::FormattedSpinButton> weld_formatted_spin_button(const OString &id, bool bTakeOwnership) override
    {
        (void) id;
        (void) bTakeOwnership;
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
        return nullptr;
    }

    virtual std::unique_ptr<weld::TimeSpinButton> weld_time_spin_button(const OString& id, TimeFieldFormat eFormat,
                                                        bool bTakeOwnership) override
    {
        return std::make_unique<weld::TimeSpinButton>(weld_spin_button(id, bTakeOwnership), eFormat);
    }

    virtual std::unique_ptr<weld::ComboBox> weld_combo_box(const OString &id, bool bTakeOwnership) override
    {
        (void) id;
        (void) bTakeOwnership;
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
        return nullptr;
    }

    virtual std::unique_ptr<weld::TreeView> weld_tree_view(const OString &id, bool bTakeOwnership) override
    {
        (void) id;
        (void) bTakeOwnership;
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
        return nullptr;
    }

    virtual std::unique_ptr<weld::IconView> weld_icon_view(const OString &id, bool bTakeOwnership) override
    {
        (void) id;
        (void) bTakeOwnership;
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
        return nullptr;
    }

    virtual std::unique_ptr<weld::EntryTreeView> weld_entry_tree_view(const OString& containerid, const OString& entryid, const OString& treeviewid, bool bTakeOwnership) override
    {
        (void) containerid;
        (void) entryid;
        (void) treeviewid;
        (void) bTakeOwnership;
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
        return nullptr;
    }

    virtual std::unique_ptr<weld::Label> weld_label(const OString &rId, bool bTakeOwnership) override
    {
        (void) bTakeOwnership;
        for (const auto& i : m_aChildren)
        {
            if (i.m_sId == rId)
                return std::make_unique<AppKitInstanceLabel>((NSTextField*) i.m_pView, this, bTakeOwnership);
        }

        SAL_WARN("vcl.osx.weld", "AppKitInstanceBuilder::weld_label(" << rId << "): not found");
        return nullptr;
    }

    virtual std::unique_ptr<weld::TextView> weld_text_view(const OString &id, bool bTakeOwnership) override
    {
        (void) id;
        (void) bTakeOwnership;
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
        return nullptr;
    }

    virtual std::unique_ptr<weld::Expander> weld_expander(const OString &id, bool bTakeOwnership) override
    {
        (void) id;
        (void) bTakeOwnership;
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
        return nullptr;
    }

    virtual std::unique_ptr<weld::DrawingArea> weld_drawing_area(const OString &id, const a11yref& rA11y,
            FactoryFunction /*pUITestFactoryFunction*/, void* /*pUserData*/, bool bTakeOwnership) override
    {
        (void) id;
        (void) rA11y;
        (void) bTakeOwnership;
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
        return nullptr;
    }

    virtual std::unique_ptr<weld::Menu> weld_menu(const OString &id, bool bTakeOwnership) override
    {
        (void) id;
        (void) bTakeOwnership;
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
        return nullptr;
    }

    virtual std::unique_ptr<weld::Toolbar> weld_toolbar(const OString &id, bool bTakeOwnership) override
    {
        (void) id;
        (void) bTakeOwnership;
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
        return nullptr;
    }

    virtual std::unique_ptr<weld::SizeGroup> create_size_group() override
    {
        SAL_WARN("vcl.osx.weld", "Not yet implemented: " << OSL_THIS_FUNC);
        return nullptr;
    }

    virtual ~AppKitInstanceBuilder() override
    {
    }

private:
    void applyPackingProperty(NSView* pCurrent, NSView* pParent, xmlreader::XmlReader &reader)
    {
        (void) pParent;

        if (!pCurrent)
            return;

#if 0
        //ToolBoxItems are not true widgets just elements
        //of the ToolBox itself
        ToolBox *pToolBoxParent = nullptr;
        if (pCurrent == pParent)
            pToolBoxParent = dynamic_cast<ToolBox*>(pParent);
#endif

        xmlreader::Span name;
        int nsId;

#if 0
        if (pCurrent->GetType() == WindowType::SCROLLWINDOW)
        {
            auto aFind = m_pParserState->m_aRedundantParentWidgets.find(VclPtr<vcl::Window>(pCurrent));
            if (aFind != m_pParserState->m_aRedundantParentWidgets.end())
            {
                pCurrent = aFind->second;
                assert(pCurrent);
            }
        }
#endif
        NSGridView* gridView = [NSGridView castFrom:pParent];
        if (gridView == nil)
            return;

        while (reader.nextAttribute(&nsId, &name))
        {
            if (name == "name")
            {
                name = reader.getAttributeValue(false);
                OString sKey(name.begin, name.length);
                sKey = sKey.replace('_', '-');
                (void)reader.nextItem(xmlreader::XmlReader::Text::Raw, &name, &nsId);
                OString sValue(name.begin, name.length);

                SAL_DEBUG("apply packing: " << sKey << " " << sValue);

                if (sKey == "expand" || sKey == "resize")
                {
                    bool bTrue = (!sValue.isEmpty() && (sValue[0] == 't' || sValue[0] == 'T' || sValue[0] == '1'));
                    (void) bTrue;
#if 0
                    if (pToolBoxParent)
                        pToolBoxParent->SetItemExpand(m_pParserState->m_nLastToolbarId, bTrue);
                    else
                        pCurrent->set_expand(bTrue);
#endif
                    continue;
                }

#if 0
                if (pToolBoxParent)
                    continue;
#endif
                if (sKey == "fill")
                {
                    // bool bTrue = (!sValue.isEmpty() && (sValue[0] == 't' || sValue[0] == 'T' || sValue[0] == '1'));
                    // pCurrent->set_fill(bTrue);
                }
                else if (sKey == "pack-type")
                {
                    // VclPackType ePackType = (!sValue.isEmpty() && (sValue[0] == 'e' || sValue[0] == 'E')) ? VclPackType::End : VclPackType::Start;
                    // pCurrent->set_pack_type(ePackType);
                }
                else if (sKey == "left-attach")
                {
                    set_left_attach(pCurrent, sValue.toInt32());
                }
                else if (sKey == "top-attach")
                {
                    set_top_attach(pCurrent, sValue.toInt32());
                }
                else if (sKey == "width")
                {
                    // pCurrent->set_grid_width(sValue.toInt32());
                }
                else if (sKey == "height")
                {
                    // pCurrent->set_grid_height(sValue.toInt32());
                }
                else if (sKey == "padding")
                {
                    // pCurrent->set_padding(sValue.toInt32());
                }
                else if (sKey == "position")
                {
                    // set_window_packing_position(pCurrent, sValue.toInt32());
                }
                else if (sKey == "secondary")
                {
                    // pCurrent->set_secondary(toBool(sValue));
                }
                else if (sKey == "non-homogeneous")
                {
                    // pCurrent->set_non_homogeneous(toBool(sValue));
                }
                else if (sKey == "homogeneous")
                {
                    // pCurrent->set_non_homogeneous(!toBool(sValue));
                }
                else
                {
                    SAL_WARN("vcl.osx.weld", "unknown packing: " << sKey);
                }
            }
        }

        long leftAttach = get_left_attach(pCurrent);
        long topAttach = get_top_attach(pCurrent);
        if (leftAttach >= 0 && topAttach >= 0)
        {
            if (leftAttach >= gridView.numberOfColumns)
            {
                for (int i = gridView.numberOfColumns; i <= leftAttach; i++)
                    [gridView addColumnWithViews:@[]];
            }
            NSGridColumn* oldColumn = [gridView columnAtIndex:leftAttach];
            NSMutableArray<NSView*>* newColumn = [NSMutableArray arrayWithCapacity:std::max(topAttach+1, oldColumn.numberOfCells)];
            for (int j = 0; j < topAttach; j++)
            {
                if (j < oldColumn.numberOfCells)
                {
                    id oldCellContent = [[oldColumn cellAtIndex:j] contentView];
                    if (oldCellContent != nil)
                        newColumn[j] = [[oldColumn cellAtIndex:j] contentView];
                    else
                        newColumn[j] = [NSGridCell emptyContentView];
                }
                else
                    newColumn[j] = [NSGridCell emptyContentView];
            }
            newColumn[topAttach] = pCurrent;
            for (int j = topAttach+1; j < oldColumn.numberOfCells; j++)
            {
                id oldCellContent = [[oldColumn cellAtIndex:j] contentView];
                if (oldCellContent != nil)
                    newColumn[j] = oldCellContent;
                else
                    newColumn[j] = [NSGridCell emptyContentView];
            }
            [gridView removeColumnAtIndex:leftAttach];
            [gridView insertColumnAtIndex:leftAttach withViews:newColumn];
        }
    }

    void handlePacking(NSView* pCurrent, NSView* pParent, xmlreader::XmlReader &reader)
    {
        xmlreader::Span name;
        int nsId;

        int nLevel = 1;

        while(true)
        {
            xmlreader::XmlReader::Result res = reader.nextItem(xmlreader::XmlReader::Text::NONE, &name, &nsId);

            if (res == xmlreader::XmlReader::Result::Done)
                break;

            if (res == xmlreader::XmlReader::Result::Begin)
            {
                ++nLevel;
                if (name == "property")
                    applyPackingProperty(pCurrent, pParent, reader);
            }

            if (res == xmlreader::XmlReader::Result::End)
            {
                --nLevel;
            }

            if (!nLevel)
                break;
        }
    }

    void handleTabChild(NSView* pParent, xmlreader::XmlReader &reader)
    {
        std::vector<OString> sIDs;

        int nLevel = 1;
        std::map<OString, OUString> aProperties;
        // std::vector<vcl::EnumContext::Context> context;

        while (true)
        {
            xmlreader::Span name;
            int nsId;

            xmlreader::XmlReader::Result res = reader.nextItem(
                xmlreader::XmlReader::Text::NONE, &name, &nsId);

            if (res == xmlreader::XmlReader::Result::Begin)
            {
                ++nLevel;
                if (name == "object")
                {
                    while (reader.nextAttribute(&nsId, &name))
                    {
                        if (name == "id")
                        {
                            name = reader.getAttributeValue(false);
                            OString sID(name.begin, name.length);
                            sal_Int32 nDelim = sID.indexOf(':');
                            if (nDelim != -1)
                            {
                                OString sPattern = sID.copy(nDelim+1);
                                aProperties[OString("customproperty")] = OUString::fromUtf8(sPattern);
                                sID = sID.copy(0, nDelim);
                            }
                            sIDs.push_back(sID);
                        }
                    }
                }
                else if (name == "style")
                {
                    // int nPriority = 0;
                    // context = handleStyle(reader, nPriority);
                    --nLevel;
                }
                else if (name == "property")
                    collectProperty(reader, aProperties);
            }

            if (res == xmlreader::XmlReader::Result::End)
                --nLevel;

            if (!nLevel)
                break;

            if (res == xmlreader::XmlReader::Result::Done)
                break;
        }

        if (!pParent)
            return;

#if 0
        TabControl *pTabControl = pParent->GetType() == WindowType::TABCONTROL ?
            static_cast<TabControl*>(pParent) : nullptr;
        VerticalTabControl *pVerticalTabControl = pParent->GetType() == WindowType::VERTICALTABCONTROL ?
            static_cast<VerticalTabControl*>(pParent) : nullptr;
        assert(pTabControl || pVerticalTabControl);
        VclBuilder::stringmap::iterator aFind = aProperties.find(OString("label"));
        if (aFind != aProperties.end())
        {
            if (pTabControl)
            {
                sal_uInt16 nPageId = pTabControl->GetCurPageId();
                pTabControl->SetPageText(nPageId, aFind->second);
                pTabControl->SetPageName(nPageId, sIDs.back());
                if (!context.empty())
                {
                    TabPage* pPage = pTabControl->GetTabPage(nPageId);
                    pPage->SetContext(context);
                }
            }
            else
            {
                OUString sLabel(aFind->second);
                OUString sIconName(extractIconName(aProperties));
                OUString sTooltip(extractTooltipText(aProperties));
                pVerticalTabControl->InsertPage(sIDs.front(), sLabel, FixedImage::loadThemeImage(sIconName), sTooltip,
                                                pVerticalTabControl->GetPageParent()->GetWindow(GetWindowType::LastChild));
            }
        }
        else
        {
            if (pTabControl)
                pTabControl->RemovePage(pTabControl->GetCurPageId());
        }
#endif
    }

    void handleChild(NSView* pParent, xmlreader::XmlReader &reader)
    {
        NSView* pCurrentChild = nil;
        xmlreader::Span name;
        int nsId;
        OString sType, sInternalChild;

        while (reader.nextAttribute(&nsId, &name))
        {
            if (name == "type")
            {
                name = reader.getAttributeValue(false);
                sType = OString(name.begin, name.length);
            }
            else if (name == "internal-child")
            {
                name = reader.getAttributeValue(false);
                sInternalChild = OString(name.begin, name.length);
            }
        }

        if (sType == "tab")
        {
            handleTabChild(pParent, reader);
            return;
        }

        int nLevel = 1;
        while (true)
        {
            xmlreader::XmlReader::Result res = reader.nextItem(xmlreader::XmlReader::Text::NONE, &name, &nsId);

            if (res == xmlreader::XmlReader::Result::Begin)
            {
                SAL_DEBUG("handleChild: type='" << sType << "' name='" << OString(name.begin, name.length) << "'");

                if (name == "object" || name == "placeholder")
                {
                    pCurrentChild = handleObject(pParent, reader);

                    bool bObjectInserted = pCurrentChild && pParent != pCurrentChild;

                    if (bObjectInserted)
                    {
                        //Internal-children default in glade to not having their visible bits set
                        //even though they are visible (generally anyway)
                        if (!sInternalChild.isEmpty())
                            [pCurrentChild setHidden:NO];

#if 0
                        //Select the first page if it's a notebook
                        if (pCurrentChild->GetType() == WindowType::TABCONTROL)
                        {
                            TabControl *pTabControl = static_cast<TabControl*>(pCurrentChild);
                            pTabControl->SetCurPageId(pTabControl->GetPageId(0));

                            //To-Do add reorder capability to the TabControl
                        }
                        else
#endif
                        {
                            // We want to sort labels before contents of frames
                            // for keyboard traversal, especially if there
                            // are multiple widgets using the same mnemonic
                            if (sType == "label")
                            {
#if 0
                                if (VclFrame *pFrameParent = dynamic_cast<VclFrame*>(pParent))
                                    pFrameParent->designate_label(pCurrentChild);
#endif
                            }
                            if (sInternalChild.startsWith("vbox") || sInternalChild.startsWith("messagedialog-vbox"))
                            {
#if 0
                                if (Dialog *pBoxParent = dynamic_cast<Dialog*>(pParent))
                                    pBoxParent->set_content_area(static_cast<VclBox*>(pCurrentChild));
#endif
                            }
                            else if (sInternalChild.startsWith("action_area") || sInternalChild.startsWith("messagedialog-action_area"))
                            {
#if 0
                                vcl::Window *pContentArea = pCurrentChild->GetParent();
                                if (Dialog *pBoxParent = dynamic_cast<Dialog*>(pContentArea ? pContentArea->GetParent() : nullptr))
                                {
                                    pBoxParent->set_action_area(static_cast<VclButtonBox*>(pCurrentChild));
                                }
#endif
                            }

#if 0
                            bool bIsButtonBox = dynamic_cast<VclButtonBox*>(pCurrentChild) != nullptr;

                            //To-Do make reorder a virtual in Window, move this foo
                            //there and see above
                            std::vector<vcl::Window*> aChilds;
                            for (vcl::Window* pChild = pCurrentChild->GetWindow(GetWindowType::FirstChild); pChild;
                                pChild = pChild->GetWindow(GetWindowType::Next))
                            {
                                if (bIsButtonBox)
                                {
                                    if (PushButton* pPushButton = dynamic_cast<PushButton*>(pChild))
                                        pPushButton->setAction(true);
                                }

                                aChilds.push_back(pChild);
                            }

                            //sort child order within parent so that tabbing
                            //between controls goes in a visually sensible sequence
                            std::stable_sort(aChilds.begin(), aChilds.end(), sortIntoBestTabTraversalOrder(this));
                            BuilderUtils::reorderWithinParent(aChilds, bIsButtonBox);
#endif
                        }
                    }
                }
                else if (name == "packing")
                {
                    handlePacking(pCurrentChild, pParent, reader);
                }
                else if (name == "interface")
                {
                    while (reader.nextAttribute(&nsId, &name))
                    {
                        if (name == "domain")
                        {
                            name = reader.getAttributeValue(false);
                            sType = OString(name.begin, name.length);
                            m_aResLocale = Translate::Create(sType.getStr());
                        }
                    }
                    ++nLevel;
                }
                else
                    ++nLevel;
            }

            if (res == xmlreader::XmlReader::Result::End)
                --nLevel;

            if (!nLevel)
                break;

            if (res == xmlreader::XmlReader::Result::Done)
                break;
        }
    }

    NSView* handleObject(NSView* pParent, xmlreader::XmlReader &reader)
    {
        OString sClass;
        OString sID;
        OUString sCustomProperty;

        xmlreader::Span name;
        int nsId;

        while (reader.nextAttribute(&nsId, &name))
        {
            if (name == "class")
            {
                name = reader.getAttributeValue(false);
                sClass = OString(name.begin, name.length);
            }
            else if (name == "id")
            {
                name = reader.getAttributeValue(false);
                sID = OString(name.begin, name.length);
            }
        }

        if (sClass == "GtkListStore" || sClass == "GtkTreeStore")
        {
            // handleListStore(reader, sID, sClass);
            return nullptr;
        }
        else if (sClass == "GtkMenu")
        {
            // handleMenu(reader, sID, false);
            return nullptr;
        }
        else if (sClass == "GtkMenuBar")
        {
            // 
            return nullptr;
        }
        else if (sClass == "GtkSizeGroup")
        {
            // handleSizeGroup(reader);
            return nullptr;
        }
        else if (sClass == "AtkObject")
        {
            // handleAtkObject(reader, pParent);
            return nullptr;
        }

        int nLevel = 1;

        std::map<OString, OUString> aProperties;

        if (!sCustomProperty.isEmpty())
            aProperties[OString("customproperty")] = sCustomProperty;

        NSView* pCurrentChild = nil;
        while (true)
        {
            xmlreader::XmlReader::Result res = reader.nextItem(xmlreader::XmlReader::Text::NONE, &name, &nsId);

            if (res == xmlreader::XmlReader::Result::Done)
                break;

            if (res == xmlreader::XmlReader::Result::Begin)
            {
                if (name == "child")
                {
                    if (!pCurrentChild)
                    {
                        pCurrentChild = insertObject(pParent, sClass, sID, aProperties);
                    }
                    handleChild(pCurrentChild, reader);
                }
                else if (name == "items")
                {
                    // aItems = handleItems(reader);
                }
                else if (name == "style")
                {
                }
                else
                {
                    ++nLevel;
                    if (name == "property")
                        collectProperty(reader, aProperties);
                }
            }

            if (res == xmlreader::XmlReader::Result::End)
            {
                --nLevel;
            }

            if (!nLevel)
                break;
        }

        if (sClass == "GtkAdjustment")
        {
            m_aAdjustments[sID] = aProperties;
            return nullptr;
        }
        else if (sClass == "GtkTextBuffer")
        {
            // m_pParserState->m_aTextBuffers[sID] = aProperties;
            return nullptr;
        }

        if (!pCurrentChild)
        {
            pCurrentChild = insertObject(pParent, sClass, sID, aProperties);
        }
#if 0
        if (!aItems.empty())
        {
            // try to fill-in the items
            if (!insertItems<ComboBox>(pCurrentChild, aProperties, m_aUserData, aItems))
                insertItems<ListBox>(pCurrentChild, aProperties, m_aUserData, aItems);
        }
#endif
        return pCurrentChild;
    }

    void collectProperty(xmlreader::XmlReader &reader, std::map<OString, OUString> &rMap) const
    {
        xmlreader::Span name;
        int nsId;

        OString sProperty, sContext;

        bool bTranslated = false;

        while (reader.nextAttribute(&nsId, &name))
        {
            if (name == "name")
            {
                name = reader.getAttributeValue(false);
                sProperty = OString(name.begin, name.length);
            }
            else if (name == "context")
            {
                name = reader.getAttributeValue(false);
                sContext = OString(name.begin, name.length);
            }
            else if (name == "translatable" && reader.getAttributeValue(false) == "yes")
            {
                bTranslated = true;
            }
        }

        reader.nextItem(xmlreader::XmlReader::Text::Raw, &name, &nsId);
        OString sValue(name.begin, name.length);
        OUString sFinalValue;
        if (bTranslated)
        {
            if (!sContext.isEmpty())
                sValue = sContext + "\004" + sValue;
            sFinalValue = Translate::get(sValue.getStr(), m_aResLocale);
        }
        else
            sFinalValue = OUString::fromUtf8(sValue);

        if (!sProperty.isEmpty())
        {
            sProperty = sProperty.replace('_', '-');
            if (m_pStringReplace)
                sFinalValue = (*m_pStringReplace)(sFinalValue);
            rMap[sProperty] = sFinalValue;
        }
    }

    NSView* insertObject(NSView* pParent, const OString &rClass,
        const OString &rID, std::map<OString, OUString> &rProps)
    {
        NSView* pCurrentChild;

        pCurrentChild = makeObject(pParent, rClass, rID, rProps);

#if 0
        if (pCurrentChild)
        {
            pCurrentChild->set_id(OStringToOUString(rID, RTL_TEXTENCODING_UTF8));
            if (pCurrentChild == m_pParent.get() && m_bToplevelHasDeferredProperties)
                m_aDeferredProperties = rProps;
            else
                BuilderUtils::set_properties(pCurrentChild, rProps);

            for (auto const& elem : rPango)
            {
                const OString &rKey = elem.first;
                const OUString &rValue = elem.second;
                pCurrentChild->set_font_attribute(rKey, rValue);
            }
        }

        rProps.clear();
#endif
        return pCurrentChild;
    }

    void extractMnemonicWidget(const OString &rLabelID, std::map<OString, OUString> &rMap)
    {
        auto aFind = rMap.find(OString("mnemonic-widget"));
        if (aFind != rMap.end())
        {
            OUString sID = aFind->second;
            sal_Int32 nDelim = sID.indexOf(':');
            if (nDelim != -1)
                sID = sID.copy(0, nDelim);
            m_aMnemonicWidgetMaps.emplace_back(rLabelID, sID);
            rMap.erase(aFind);
        }
    }

    NSView* makeObject(NSView* pParent, const OString &name, const OString &id,
        std::map<OString, OUString> &rMap)
    {
        SAL_DEBUG("makeObject " << name << " " << id);

        (void) pParent;

        bool bIsPlaceHolder = name.isEmpty();
        bool bVertical = false;

#if 0
        if (pParent && (pParent->GetType() == WindowType::TABCONTROL ||
                        pParent->GetType() == WindowType::VERTICALTABCONTROL))
        {
            bool bTopLevel(name == "GtkDialog" || name == "GtkMessageDialog" ||
                           name == "GtkWindow" || name == "GtkPopover" || name == "GtkAssistant");
            if (!bTopLevel)
            {
                if (pParent->GetType() == WindowType::TABCONTROL)
                {
                    //We have to add a page
                    //make default pageid == position
                    TabControl *pTabControl = static_cast<TabControl*>(pParent);
                    sal_uInt16 nNewPageCount = pTabControl->GetPageCount()+1;
                    sal_uInt16 nNewPageId = nNewPageCount;
                    pTabControl->InsertPage(nNewPageId, OUString());
                    pTabControl->SetCurPageId(nNewPageId);
                    SAL_WARN_IF(bIsPlaceHolder, "vcl.layout", "we should have no placeholders for tabpages");
                    if (!bIsPlaceHolder)
                    {
                        VclPtrInstance<TabPage> pPage(pTabControl);
                        pPage->Show();

                        //Make up a name for it
                        OString sTabPageId = get_by_window(pParent) +
                            "-page" +
                            OString::number(nNewPageCount);
                        m_aChildren.emplace_back(sTabPageId, pPage, false);
                        pPage->SetHelpId(m_sHelpRoot + sTabPageId);

                        pParent = pPage;

                        pTabControl->SetTabPage(nNewPageId, pPage);
                    }
                }
                else
                {
                    VerticalTabControl *pTabControl = static_cast<VerticalTabControl*>(pParent);
                    SAL_WARN_IF(bIsPlaceHolder, "vcl.layout", "we should have no placeholders for tabpages");
                    if (!bIsPlaceHolder)
                        pParent = pTabControl->GetPageParent();
                }
            }
        }
#endif
        if (bIsPlaceHolder || name == "GtkTreeSelection")
            return nullptr;

        // extractButtonImage(id, rMap, name == "GtkRadioButton");

        NSView* result = nil;
        if (name == "GtkDialog" || name == "GtkAboutDialog" || name == "GtkAssistant")
        {
            // WB_ALLOWMENUBAR because we don't know in advance if we will encounter
            // a menubar, and menubars need a BorderWindow in the toplevel, and
            // such border windows need to be in created during the dialog ctor
            WinBits nBits = WB_MOVEABLE|WB_3DLOOK|WB_ALLOWMENUBAR;
            if (extractResizable(rMap))
                nBits |= WB_SIZEABLE;
            if (extractCloseable(rMap))
                nBits |= WB_CLOSEABLE;
            NSPanel* panel = [[NSPanel alloc] init];
            [panel orderFrontRegardless];
            SAL_DEBUG("NSPanel created for dialog: " << panel << ", contentView: " << panel.contentView);
            result = panel.contentView;
        }
#if 0
        else if (name == "GtkMessageDialog")
        {
            WinBits nBits = WB_MOVEABLE|WB_3DLOOK|WB_CLOSEABLE;
            if (extractResizable(rMap))
                nBits |= WB_SIZEABLE;
            VclPtr<MessageDialog> xDialog(VclPtr<MessageDialog>::Create(pParent, nBits));
            m_pParserState->m_aMessageDialogs.push_back(xDialog);
            xWindow = xDialog;
            xWindow->set_border_width(12);
        }
#endif
        else if (name == "GtkBox" || name == "GtkButtonBox" || name == "GtkStatusbar")
        {
            bVertical = extractOrientation(rMap);
            NSStackView* stackView = [NSStackView stackViewWithViews:@[]];
            if (bVertical)
                [stackView setOrientation:NSUserInterfaceLayoutOrientationVertical];
            assert(pParent);
            [pParent addSubview:stackView];
            result = stackView;
        }
#if 0
        else if (name == "GtkPaned")
        {
            bVertical = extractOrientation(rMap);
            if (bVertical)
                xWindow = VclPtr<VclVPaned>::Create(pParent);
            else
                xWindow = VclPtr<VclHPaned>::Create(pParent);
        }
#endif
        else if (name == "GtkHBox" || name == "GtkHButtonBox")
        {
            NSStackView* stackView = [NSStackView stackViewWithViews:@[]];
            assert(pParent);
            [pParent addSubview:stackView];
            result = stackView;
        }
        else if (name == "GtkVBox" || name == "GtkVButtonBox")
        {
            NSStackView* stackView = [NSStackView stackViewWithViews:@[]];
            [stackView setOrientation:NSUserInterfaceLayoutOrientationVertical];
            assert(pParent);
            [pParent addSubview:stackView];
            result = stackView;
        }
        else if (name == "GtkGrid")
        {
            NSGridView* gridView = [[NSGridView alloc] init];
            assert(pParent);
            [pParent addSubview:gridView];
            result = gridView;
        }
#if 0
        else if (name == "GtkFrame")
            xWindow = VclPtr<VclFrame>::Create(pParent);
        else if (name == "GtkExpander")
        {
            VclPtrInstance<VclExpander> pExpander(pParent);
            m_pParserState->m_aExpanderWidgets.push_back(pExpander);
            xWindow = pExpander;
        }
        else if (name == "GtkAlignment")
            xWindow = VclPtr<VclAlignment>::Create(pParent);
#endif
        else if (name == "GtkButton")
        {
            NSButton* button = [NSButton buttonWithTitle:[NSString stringWithUTF8String:id.getStr()] target:nil action:nil];
            OUString sMenu = BuilderUtils::extractCustomProperty(rMap);
            (void) sMenu;
#if 0
            if (sMenu.isEmpty())
                xButton = extractStockAndBuildPushButton(pParent, rMap, name == "GtkToggleButton", m_bLegacy);
            else
            {
                assert(m_bLegacy && "use GtkMenuButton");
                xButton = extractStockAndBuildMenuButton(pParent, rMap);
                m_pParserState->m_aButtonMenuMaps.emplace_back(id, sMenu);
            }
            xButton->SetImageAlign(ImageAlign::Left); //default to left
            setupFromActionName(xButton, rMap, m_xFrame);
#endif
            assert(pParent);
            [pParent addSubview:button];
            result = button;
        }
#if 0
        else if (name == "GtkMenuButton")
        {
            VclPtr<MenuButton> xButton = extractStockAndBuildMenuButton(pParent, rMap);
            OUString sMenu = extractPopupMenu(rMap);
            if (!sMenu.isEmpty())
                m_pParserState->m_aButtonMenuMaps.emplace_back(id, sMenu);
            xButton->SetImageAlign(ImageAlign::Left); //default to left
            xButton->SetAccessibleRole(css::accessibility::AccessibleRole::BUTTON_MENU);

            if (!extractDrawIndicator(rMap))
                xButton->SetDropDown(PushButtonDropdownStyle::NONE);

            setupFromActionName(xButton, rMap, m_xFrame);
            xWindow = xButton;
        }
        else if (name == "GtkToggleButton" && m_bLegacy)
        {
            VclPtr<Button> xButton;
            OUString sMenu = BuilderUtils::extractCustomProperty(rMap);
            assert(sMenu.getLength() && "not implemented yet");
            xButton = extractStockAndBuildMenuToggleButton(pParent, rMap);
            m_pParserState->m_aButtonMenuMaps.emplace_back(id, sMenu);
            xButton->SetImageAlign(ImageAlign::Left); //default to left
            setupFromActionName(xButton, rMap, m_xFrame);
            xWindow = xButton;
        }
        else if (name == "GtkRadioButton")
        {
            extractGroup(id, rMap);
            WinBits nBits = WB_CLIPCHILDREN|WB_CENTER|WB_VCENTER|WB_3DLOOK;
            OUString sWrap = BuilderUtils::extractCustomProperty(rMap);
            if (!sWrap.isEmpty())
                nBits |= WB_WORDBREAK;
            VclPtr<RadioButton> xButton = VclPtr<RadioButton>::Create(pParent, nBits);
            xButton->SetImageAlign(ImageAlign::Left); //default to left
            xWindow = xButton;

            if (::extractStock(rMap))
            {
                xWindow->SetText(getStockText(extractLabel(rMap)));
            }
        }
        else if (name == "GtkCheckButton")
        {
            WinBits nBits = WB_CLIPCHILDREN|WB_CENTER|WB_VCENTER|WB_3DLOOK;
            OUString sWrap = BuilderUtils::extractCustomProperty(rMap);
            if (!sWrap.isEmpty())
                nBits |= WB_WORDBREAK;
            //maybe always import as TriStateBox and enable/disable tristate
            bool bIsTriState = extractInconsistent(rMap);
            VclPtr<CheckBox> xCheckBox;
            if (bIsTriState && m_bLegacy)
                xCheckBox = VclPtr<TriStateBox>::Create(pParent, nBits);
            else
                xCheckBox = VclPtr<CheckBox>::Create(pParent, nBits);
            if (bIsTriState)
            {
                xCheckBox->EnableTriState(true);
                xCheckBox->SetState(TRISTATE_INDET);
            }
            xCheckBox->SetImageAlign(ImageAlign::Left); //default to left

            xWindow = xCheckBox;

            if (::extractStock(rMap))
            {
                xWindow->SetText(getStockText(extractLabel(rMap)));
            }
        }
        else if (name == "GtkSpinButton")
        {
            OUString sAdjustment = extractAdjustment(rMap);
            OUString sPattern = BuilderUtils::extractCustomProperty(rMap);
            OUString sUnit = extractUnit(sPattern);

            WinBits nBits = WB_CLIPCHILDREN|WB_LEFT|WB_BORDER|WB_3DLOOK;
            if (!id.endsWith("-nospin"))
                nBits |= WB_SPIN | WB_REPEAT;

            if (sPattern.isEmpty())
            {
                SAL_INFO("vcl.layout", "making numeric field for " << name << " " << sUnit);
                if (m_bLegacy)
                {
                    connectNumericFormatterAdjustment(id, sAdjustment);
                    xWindow = VclPtr<NumericField>::Create(pParent, nBits);
                }
                else
                {
                    connectFormattedFormatterAdjustment(id, sAdjustment);
                    VclPtrInstance<FormattedField> xField(pParent, nBits);
                    xField->SetMinValue(0);
                    xWindow = xField;
                }
            }
            else
            {
                if (sPattern == "hh:mm")
                {
                    connectTimeFormatterAdjustment(id, sAdjustment);
                    SAL_INFO("vcl.layout", "making time field for " << name << " " << sUnit);
                    xWindow = VclPtr<TimeField>::Create(pParent, nBits);
                }
                else if (sPattern == "yy:mm:dd")
                {
                    connectDateFormatterAdjustment(id, sAdjustment);
                    SAL_INFO("vcl.layout", "making date field for " << name << " " << sUnit);
                    xWindow = VclPtr<DateField>::Create(pParent, nBits);
                }
                else
                {
                    connectNumericFormatterAdjustment(id, sAdjustment);
                    FieldUnit eUnit = detectMetricUnit(sUnit);
                    SAL_INFO("vcl.layout", "making metric field for " << name << " " << sUnit);
                    VclPtrInstance<MetricField> xField(pParent, nBits);
                    xField->SetUnit(eUnit);
                    if (eUnit == FieldUnit::CUSTOM)
                        xField->SetCustomUnitText(sUnit);
                    xWindow = xField;
                }
            }
        }
        else if (name == "GtkLinkButton")
            xWindow = VclPtr<FixedHyperlink>::Create(pParent, WB_CENTER|WB_VCENTER|WB_3DLOOK|WB_NOLABEL);
        else if (name == "GtkComboBox" || name == "GtkComboBoxText")
        {
            OUString sPattern = BuilderUtils::extractCustomProperty(rMap);
            extractModel(id, rMap);

            WinBits nBits = WB_CLIPCHILDREN|WB_LEFT|WB_VCENTER|WB_3DLOOK;

            bool bDropdown = BuilderUtils::extractDropdown(rMap);

            if (bDropdown)
                nBits |= WB_DROPDOWN;

            if (!sPattern.isEmpty())
            {
                OUString sAdjustment = extractAdjustment(rMap);
                connectNumericFormatterAdjustment(id, sAdjustment);
                OUString sUnit = extractUnit(sPattern);
                FieldUnit eUnit = detectMetricUnit(sUnit);
                SAL_WARN("vcl.layout", "making metric box for type: " << name
                    << " unit: " << sUnit
                    << " name: " << id
                    << " use a VclComboBoxNumeric instead");
                VclPtrInstance<MetricBox> xBox(pParent, nBits);
                xBox->EnableAutoSize(true);
                xBox->SetUnit(eUnit);
                xBox->SetDecimalDigits(extractDecimalDigits(sPattern));
                if (eUnit == FieldUnit::CUSTOM)
                    xBox->SetCustomUnitText(sUnit);
                xWindow = xBox;
            }
            else if (extractEntry(rMap))
            {
                VclPtrInstance<ComboBox> xComboBox(pParent, nBits);
                xComboBox->EnableAutoSize(true);
                xWindow = xComboBox;
            }
            else
            {
                VclPtrInstance<ListBox> xListBox(pParent, nBits|WB_SIMPLEMODE);
                xListBox->EnableAutoSize(true);
                xWindow = xListBox;
            }
        }
        else if (name == "VclComboBoxNumeric")
        {
            OUString sPattern = BuilderUtils::extractCustomProperty(rMap);
            OUString sAdjustment = extractAdjustment(rMap);
            extractModel(id, rMap);

            WinBits nBits = WB_CLIPCHILDREN|WB_LEFT|WB_VCENTER|WB_3DLOOK;

            bool bDropdown = BuilderUtils::extractDropdown(rMap);

            if (bDropdown)
                nBits |= WB_DROPDOWN;

            if (!sPattern.isEmpty())
            {
                connectNumericFormatterAdjustment(id, sAdjustment);
                OUString sUnit = extractUnit(sPattern);
                FieldUnit eUnit = detectMetricUnit(sUnit);
                SAL_INFO("vcl.layout", "making metric box for " << name << " " << sUnit);
                VclPtrInstance<MetricBox> xBox(pParent, nBits);
                xBox->EnableAutoSize(true);
                xBox->SetUnit(eUnit);
                xBox->SetDecimalDigits(extractDecimalDigits(sPattern));
                if (eUnit == FieldUnit::CUSTOM)
                    xBox->SetCustomUnitText(sUnit);
                xWindow = xBox;
            }
            else
            {
                SAL_INFO("vcl.layout", "making numeric box for " << name);
                connectNumericFormatterAdjustment(id, sAdjustment);
                VclPtrInstance<NumericBox> xBox(pParent, nBits);
                if (bDropdown)
                    xBox->EnableAutoSize(true);
                xWindow = xBox;
            }
        }
        else if (name == "GtkIconView")
        {
            assert(rMap.find(OString("model")) != rMap.end() && "GtkIconView must have a model");

            //window we want to apply the packing props for this GtkIconView to
            VclPtr<vcl::Window> xWindowForPackingProps;
            extractModel(id, rMap);
            WinBits nWinStyle = WB_CLIPCHILDREN|WB_LEFT|WB_VCENTER|WB_3DLOOK|WB_HIDESELECTION;
            //IconView manages its own scrolling,
            vcl::Window *pRealParent = prepareWidgetOwnScrolling(pParent, nWinStyle);
            if (pRealParent != pParent)
                nWinStyle |= WB_BORDER;

            VclPtr<IconView> xBox = VclPtr<IconView>::Create(pRealParent, nWinStyle);
            xWindowForPackingProps = xBox;

            xWindow = xBox;
            xBox->SetNoAutoCurEntry(true);
            xBox->SetQuickSearch(true);

            if (pRealParent != pParent)
                cleanupWidgetOwnScrolling(pParent, xWindowForPackingProps, rMap);
        }
        else if (name == "GtkTreeView")
        {
            if (!m_bLegacy)
            {
                assert(rMap.find(OString("model")) != rMap.end() && "GtkTreeView must have a model");
            }

            //window we want to apply the packing props for this GtkTreeView to
            VclPtr<vcl::Window> xWindowForPackingProps;
            //To-Do
            //a) make SvHeaderTabListBox/SvTabListBox the default target for GtkTreeView
            //b) remove the non-drop down mode of ListBox and convert
            //   everything over to SvHeaderTabListBox/SvTabListBox
            //c) remove the users of makeSvTabListBox and makeSvTreeListBox
            extractModel(id, rMap);
            WinBits nWinStyle = WB_CLIPCHILDREN|WB_LEFT|WB_VCENTER|WB_3DLOOK|WB_HIDESELECTION;
            if (m_bLegacy)
            {
                OUString sBorder = BuilderUtils::extractCustomProperty(rMap);
                if (!sBorder.isEmpty())
                    nWinStyle |= WB_BORDER;
            }
            else
            {
                nWinStyle |= WB_HASBUTTONS | WB_HASBUTTONSATROOT;
            }
            //ListBox/SvHeaderTabListBox manages its own scrolling,
            vcl::Window *pRealParent = prepareWidgetOwnScrolling(pParent, nWinStyle);
            if (pRealParent != pParent)
                nWinStyle |= WB_BORDER;
            if (m_bLegacy)
            {
                xWindow = VclPtr<ListBox>::Create(pRealParent, nWinStyle | WB_SIMPLEMODE);
                xWindowForPackingProps = xWindow;
            }
            else
            {
                VclPtr<SvTabListBox> xBox;
                bool bHeadersVisible = extractHeadersVisible(rMap);
                if (bHeadersVisible)
                {
                    VclPtr<VclVBox> xContainer = VclPtr<VclVBox>::Create(pRealParent);
                    OString containerid(id + "-container");
                    xContainer->SetHelpId(m_sHelpRoot + containerid);
                    m_aChildren.emplace_back(containerid, xContainer, true);

                    VclPtrInstance<HeaderBar> xHeader(xContainer, WB_BUTTONSTYLE | WB_BORDER | WB_TABSTOP | WB_3DLOOK);
                    xHeader->set_width_request(0); // let the headerbar width not affect the size request
                    OString headerid(id + "-header");
                    xHeader->SetHelpId(m_sHelpRoot + headerid);
                    m_aChildren.emplace_back(headerid, xHeader, true);

                    VclPtr<LclHeaderTabListBox> xHeaderBox = VclPtr<LclHeaderTabListBox>::Create(xContainer, nWinStyle);
                    xHeaderBox->InitHeaderBar(xHeader);
                    xContainer->set_expand(true);
                    xHeader->Show();
                    xContainer->Show();
                    xBox = xHeaderBox;
                    xWindowForPackingProps = xContainer;
                }
                else
                {
                    xBox = VclPtr<LclTabListBox>::Create(pRealParent, nWinStyle);
                    xWindowForPackingProps = xBox;
                }
                xWindow = xBox;
                xBox->SetNoAutoCurEntry(true);
                xBox->SetQuickSearch(true);
                xBox->SetSpaceBetweenEntries(3);
                xBox->SetEntryHeight(16);
                xBox->SetHighlightRange(); // select over the whole width
            }
            if (pRealParent != pParent)
                cleanupWidgetOwnScrolling(pParent, xWindowForPackingProps, rMap);
        }
        else if (name == "GtkTreeViewColumn")
        {
            if (!m_bLegacy)
            {
                SvHeaderTabListBox* pTreeView = dynamic_cast<SvHeaderTabListBox*>(pParent);
                if (HeaderBar* pHeaderBar = pTreeView ? pTreeView->GetHeaderBar() : nullptr)
                {
                    HeaderBarItemBits nBits = HeaderBarItemBits::LEFTIMAGE;
                    if (extractClickable(rMap))
                        nBits |= HeaderBarItemBits::CLICKABLE;
                    if (extractSortIndicator(rMap))
                        nBits |= HeaderBarItemBits::DOWNARROW;
                    float fAlign = extractAlignment(rMap);
                    if (fAlign == 0.0)
                        nBits |= HeaderBarItemBits::LEFT;
                    else if (fAlign == 1.0)
                        nBits |= HeaderBarItemBits::RIGHT;
                    else if (fAlign == 0.5)
                        nBits |= HeaderBarItemBits::CENTER;
                    auto nItemId = pHeaderBar->GetItemCount() + 1;
                    OUString sTitle(extractTitle(rMap));
                    pHeaderBar->InsertItem(nItemId, sTitle, 100, nBits);
                }
            }
        }
#endif
        else if (name == "GtkLabel")
        {
            WinBits nWinStyle = WB_CENTER|WB_VCENTER|WB_3DLOOK;
            OUString sBorder = BuilderUtils::extractCustomProperty(rMap);
            if (!sBorder.isEmpty())
                nWinStyle |= WB_BORDER;
            extractMnemonicWidget(id, rMap);
            NSTextField* textField = [NSTextField labelWithString:[NSString stringWithUTF8String:id.getStr()]];
            assert(pParent);
            if (![pParent isKindOfClass:NSGridView.class])
                [pParent addSubview:textField];
            result = textField;
        }
#if 0
        else if (name == "GtkImage")
        {
            extractStock(id, rMap);
            xWindow = VclPtr<FixedImage>::Create(pParent, WB_CENTER|WB_VCENTER|WB_3DLOOK|WB_SCALE);
            //such parentless GtkImages are temps used to set icons on buttons
            //default them to hidden to stop e.g. insert->index entry flicking temp
            //full screen windows
            if (!pParent)
            {
                rMap["visible"] = "false";
            }
        }
        else if (name == "GtkSeparator")
        {
            bVertical = extractOrientation(rMap);
            xWindow = VclPtr<FixedLine>::Create(pParent, bVertical ? WB_VERT : WB_HORZ);
        }
        else if (name == "GtkScrollbar")
        {
            extractAdjustmentToMap(id, rMap, m_pParserState->m_aScrollAdjustmentMaps);
            bVertical = extractOrientation(rMap);
            xWindow = VclPtr<ScrollBar>::Create(pParent, bVertical ? WB_VERT : WB_HORZ);
        }
        else if (name == "GtkProgressBar")
        {
            extractAdjustmentToMap(id, rMap, m_pParserState->m_aScrollAdjustmentMaps);
            bVertical = extractOrientation(rMap);
            xWindow = VclPtr<ProgressBar>::Create(pParent, bVertical ? WB_VERT : WB_HORZ);
        }
        else if (name == "GtkScrolledWindow")
        {
            xWindow = VclPtr<VclScrolledWindow>::Create(pParent);
        }
        else if (name == "GtkViewport")
        {
            xWindow = VclPtr<VclViewport>::Create(pParent);
        }
        else if (name == "GtkEventBox")
        {
            xWindow = VclPtr<VclEventBox>::Create(pParent);
        }
        else if (name == "GtkEntry")
        {
            xWindow = VclPtr<Edit>::Create(pParent, WB_LEFT|WB_VCENTER|WB_BORDER|WB_3DLOOK);
            BuilderUtils::ensureDefaultWidthChars(rMap);
        }
        else if (name == "GtkNotebook")
        {
            if (!extractVerticalTabPos(rMap))
                xWindow = VclPtr<TabControl>::Create(pParent, WB_STDTABCONTROL|WB_3DLOOK);
            else
                xWindow = VclPtr<VerticalTabControl>::Create(pParent);
        }
        else if (name == "GtkDrawingArea")
        {
            OUString sBorder = BuilderUtils::extractCustomProperty(rMap);
            xWindow = VclPtr<VclDrawingArea>::Create(pParent, sBorder.isEmpty() ? WB_TABSTOP : WB_BORDER | WB_TABSTOP);
        }
        else if (name == "GtkTextView")
        {
            extractBuffer(id, rMap);

            WinBits nWinStyle = WB_CLIPCHILDREN|WB_LEFT;
            if (m_bLegacy)
            {
                OUString sBorder = BuilderUtils::extractCustomProperty(rMap);
                if (!sBorder.isEmpty())
                    nWinStyle |= WB_BORDER;
            }
            //VclMultiLineEdit manages its own scrolling,
            vcl::Window *pRealParent = prepareWidgetOwnScrolling(pParent, nWinStyle);
            if (pRealParent != pParent)
                nWinStyle |= WB_BORDER;
            xWindow = VclPtr<VclMultiLineEdit>::Create(pRealParent, nWinStyle);
            if (pRealParent != pParent)
                cleanupWidgetOwnScrolling(pParent, xWindow, rMap);
        }
        else if (name == "GtkSpinner")
        {
            xWindow = VclPtr<Throbber>::Create(pParent, WB_3DLOOK);
        }
        else if (name == "GtkScale")
        {
            extractAdjustmentToMap(id, rMap, m_pParserState->m_aSliderAdjustmentMaps);
            bool bDrawValue = extractDrawValue(rMap);
            if (bDrawValue)
            {
                OUString sValuePos = extractValuePos(rMap);
                (void)sValuePos;
            }
            bVertical = extractOrientation(rMap);

            WinBits nWinStyle = bVertical ? WB_VERT : WB_HORZ;

            xWindow = VclPtr<Slider>::Create(pParent, nWinStyle);
        }
        else if (name == "GtkToolbar")
        {
            xWindow = VclPtr<ToolBox>::Create(pParent, WB_3DLOOK | WB_TABSTOP);
        }
        else if(name == "NotebookBarAddonsToolMergePoint")
        {
            customMakeWidget pFunction = GetCustomMakeWidget("sfxlo-NotebookbarToolBox");
            if(pFunction != nullptr)
                NotebookBarAddonsMerger::MergeNotebookBarAddons(pParent, pFunction, m_xFrame, *m_pNotebookBarAddonsItem, rMap);
            return nullptr;
        }
        else if (name == "GtkToolButton" || name == "GtkMenuToolButton" ||
                 name == "GtkToggleToolButton" || name == "GtkRadioToolButton")
        {
            ToolBox *pToolBox = dynamic_cast<ToolBox*>(pParent);
            if (pToolBox)
            {
                OUString aCommand(extractActionName(rMap));

                sal_uInt16 nItemId = 0;
                ToolBoxItemBits nBits = ToolBoxItemBits::NONE;
                if (name == "GtkMenuToolButton")
                    nBits |= ToolBoxItemBits::DROPDOWN;
                else if (name == "GtkToggleToolButton")
                    nBits |= ToolBoxItemBits::AUTOCHECK | ToolBoxItemBits::CHECKABLE;
                else if (name == "GtkRadioToolButton")
                    nBits |= ToolBoxItemBits::AUTOCHECK | ToolBoxItemBits::RADIOCHECK;

                if (!aCommand.isEmpty() && m_xFrame.is())
                {
                    pToolBox->InsertItem(aCommand, m_xFrame, nBits, extractSizeRequest(rMap));
                    nItemId = pToolBox->GetItemId(aCommand);
                }
                else
                {
                    nItemId = pToolBox->GetItemCount() + 1;
                        //TODO: ImplToolItems::size_type -> sal_uInt16!
                    pToolBox->InsertItem(nItemId, extractLabel(rMap), nBits);
                    if (aCommand.isEmpty() && !m_bLegacy)
                        aCommand = OUString::fromUtf8(id);
                    pToolBox->SetItemCommand(nItemId, aCommand);
                }

                pToolBox->SetHelpId(nItemId, m_sHelpRoot + id);
                OUString sTooltip(extractTooltipText(rMap));
                if (!sTooltip.isEmpty())
                    pToolBox->SetQuickHelpText(nItemId, sTooltip);

                OUString sIconName(extractIconName(rMap));
                if (!sIconName.isEmpty())
                    pToolBox->SetItemImage(nItemId, FixedImage::loadThemeImage(sIconName));

                if (!extractVisible(rMap))
                    pToolBox->HideItem(nItemId);

                m_pParserState->m_nLastToolbarId = nItemId;

                return nullptr; // no widget to be created
            }
        }
        else if (name == "GtkSeparatorToolItem")
        {
            ToolBox *pToolBox = dynamic_cast<ToolBox*>(pParent);
            if (pToolBox)
            {
                pToolBox->InsertSeparator();
                return nullptr; // no widget to be created
            }
        }
        else if (name == "GtkWindow")
        {
            WinBits nBits = extractDeferredBits(rMap);
            if (nBits & WB_DOCKABLE)
                xWindow = VclPtr<DockingWindow>::Create(pParent, nBits|WB_MOVEABLE);
            else
                xWindow = VclPtr<FloatingWindow>::Create(pParent, nBits|WB_MOVEABLE);
        }
        else if (name == "GtkPopover")
        {
            WinBits nBits = extractDeferredBits(rMap);
            xWindow = VclPtr<DockingWindow>::Create(pParent, nBits|WB_DOCKABLE|WB_MOVEABLE);
        }
        else if (name == "GtkCalendar")
        {
            WinBits nBits = extractDeferredBits(rMap);
            xWindow = VclPtr<Calendar>::Create(pParent, nBits);
        }
        else
        {
            if (customMakeWidget pFunction = GetCustomMakeWidget(name))
            {
                pFunction(xWindow, pParent, rMap);
                if (xWindow->GetType() == WindowType::PUSHBUTTON)
                    setupFromActionName(static_cast<Button*>(xWindow.get()), rMap, m_xFrame);
                else if (xWindow->GetType() == WindowType::MENUBUTTON)
                {
                    OUString sMenu = BuilderUtils::extractCustomProperty(rMap);
                    if (!sMenu.isEmpty())
                        m_pParserState->m_aButtonMenuMaps.emplace_back(id, sMenu);
                    setupFromActionName(static_cast<Button*>(xWindow.get()), rMap, m_xFrame);
                }
            }
        }
#endif

        if (!result)
            SAL_WARN("vcl.osx.weld", "Nothing created for " << name);

        if (result)
        {
            m_aChildren.emplace_back(id, result, bVertical);
        }

        return result;
    }

    bool extractResizable(std::map<OString, OUString> &rMap)
    {
        bool bResizable = true;
        auto aFind = rMap.find("resizable");
        if (aFind != rMap.end())
        {
            bResizable = toBool(aFind->second);
            rMap.erase(aFind);
        }
        return bResizable;
    }

    bool extractCloseable(std::map<OString, OUString> &rMap)
    {
        bool bCloseable = true;
        auto aFind = rMap.find("deletable");
        if (aFind != rMap.end())
        {
            bCloseable = toBool(aFind->second);
            rMap.erase(aFind);
        }
        return bCloseable;
    }
};

weld::Builder* AquaSalInstance::CreateBuilder(weld::Widget* pParent, const OUString& rUIRoot, const OUString& rUIFile)
{
    AppKitInstanceWidget* pParentWidget = dynamic_cast<AppKitInstanceWidget*>(pParent);
    if (pParent && !pParentWidget) //remove when complete
        return SalInstance::CreateBuilder(pParent, rUIRoot, rUIFile);
    return new AppKitInstanceBuilder(pParentWidget->getView(), rUIRoot, rUIFile);
}

weld::MessageDialog* AquaSalInstance::CreateMessageDialog(weld::Widget* pParent, VclMessageType eMessageType, VclButtonsType eButtonsType, const OUString &rPrimaryMessage)
{
    SAL_DEBUG("AquaSalInstance::CreateMessageDialog");
    (void) eMessageType;
    (void) eButtonsType;
    AppKitInstanceWidget* pParentInstance = dynamic_cast<AppKitInstanceWidget*>(pParent);
    NSWindow* pParentWindow = pParentInstance ? pParentInstance->getWindow() : nullptr;
    NSAlert* pAlert = [[NSAlert alloc] init];
    [pAlert setMessageText:CreateNSString(rPrimaryMessage)];
    [pAlert setInformativeText:@"Informative Text"];
    [pAlert beginSheetModalForWindow:pParentWindow completionHandler:^(NSModalResponse returnCode) {
            (void) returnCode;
        }];
    return new AppKitInstanceMessageDialog(pAlert, nullptr, true);
}

weld::Window* AquaSalInstance::GetFrameWeld(const css::uno::Reference<css::awt::XWindow>& rWindow)
{
    SAL_DEBUG("AquaSalInstance::GetFrameWeld");
    if (SalAppKitXWindow* pAppKitXWindow = dynamic_cast<SalAppKitXWindow*>(rWindow.get()))
        return pAppKitXWindow->getFrameWeld();
    return SalInstance::GetFrameWeld(rWindow);
}

weld::Window* AquaSalFrame::GetFrameWeld() const
{
    SAL_DEBUG("AquaSalFrame::GetFrameWeld");
    if (!m_xFrameWeld)
        m_xFrameWeld.reset(new AppKitInstanceWindow(getNSWindow(), nullptr, false));
    return m_xFrameWeld.get();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
