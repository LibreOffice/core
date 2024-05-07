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

#include <condition_variable>
#include <mutex>
#include <utility>

#include <config_features.h>

#include <stdio.h>

#include <comphelper/solarmutex.hxx>

#include <comphelper/lok.hxx>

#include <osl/process.h>

#include <rtl/ustrbuf.hxx>
#include <vclpluginapi.h>
#include <vcl/QueueInfo.hxx>
#include <vcl/svapp.hxx>
#include <vcl/window.hxx>
#include <vcl/idle.hxx>
#include <vcl/svmain.hxx>
#include <vcl/opengl/OpenGLContext.hxx>
#include <vcl/commandevent.hxx>
#include <vcl/event.hxx>

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
#include <strings.hrc>

#include <comphelper/processfactory.hxx>

#include <com/sun/star/uri/ExternalUriReferenceTranslator.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

#include <premac.h>
#include <Foundation/Foundation.h>
#include <ApplicationServices/ApplicationServices.h>
#import "apple_remote/RemoteMainController.h"
#include <apple_remote/RemoteControl.h>
#include <postmac.h>

#if HAVE_FEATURE_SKIA
#include <vcl/skia/SkiaHelper.hxx>
#include <skia/salbmp.hxx>
#include <skia/osx/gdiimpl.hxx>
#include <skia/osx/bitmap.hxx>
#endif

extern "C" {
#include <crt_externs.h>
}

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
        Idle("AquaSalInstance AquaDelayedSettingsChanged"),
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

static OUString& getFallbackPrinterName()
{
    static OUString aFallbackPrinter;

    if ( aFallbackPrinter.isEmpty() )
    {
        aFallbackPrinter = VclResId( SV_PRINT_DEFPRT_TXT );
        if ( aFallbackPrinter.isEmpty() )
            aFallbackPrinter = "Printer";
    }

    return aFallbackPrinter;
}

void AquaSalInstance::delayedSettingsChanged( bool bInvalidate )
{
    osl::Guard< comphelper::SolarMutex > aGuard( *GetYieldMutex() );
    AquaDelayedSettingsChanged* pIdle = new AquaDelayedSettingsChanged( bInvalidate );
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

    // HACK: When the first call to [NSSpellChecker sharedSpellChecker] (in
    // lingucomponent/source/spellcheck/macosxspell/macspellimp.mm) is done both on a thread other
    // than the main thread and with the SolarMutex erroneously locked, then that can lead to
    // deadlock as [NSSpellChecker sharedSpellChecker] internally calls
    //   AppKit`-[NSSpellChecker init] ->
    //   AppKit`-[NSSpellChecker _fillSpellCheckerPopupButton:] ->
    //   AppKit`-[NSApplication(NSServicesMenuPrivate) _fillSpellCheckerPopupButton:] ->
    //   AppKit`-[NSMenu insertItem:atIndex:] ->
    //   Foundation`-[NSNotificationCenter postNotificationName:object:userInfo:] ->
    //   CoreFoundation`_CFXNotificationPost ->
    //   Foundation`-[NSOperation waitUntilFinished]
    // waiting for work to be done on the main thread, but the main thread is typically already
    // blocked (in some event handling loop) waiting to acquire the SolarMutex.  The real solution
    // would be to fix all the cases where a call to [NSSpellChecker sharedSpellChecker] in
    // lingucomponent/source/spellcheck/macosxspell/macspellimp.mm is done while the SolarMutex is
    // locked (somewhere up the call chain), but that appears to be rather difficult (see e.g.
    // <https://bugs.documentfoundation.org/show_bug.cgi?id=151894> "FILEOPEN a Base Document with
    // customized event for open a startform by 'open document' LO stuck").  So, at least for now,
    // chicken out and do that first call to [NSSpellChecker sharedSpellChecker] upfront in a
    // controlled environment:
    [NSSpellChecker sharedSpellChecker];
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
    , mbNoYieldLock( false )
    , mbTimerProcessed( false )
{
    maMainThread = osl::Thread::getCurrentIdentifier();

    ImplSVData* pSVData = ImplGetSVData();
    pSVData->maAppData.mxToolkitName = OUString("osx");
    m_bSupportsOpenGL = true;

    mpButtonCell = [[NSButtonCell alloc] init];
    mpCheckCell = [[NSButtonCell alloc] init];
    mpRadioCell = [[NSButtonCell alloc] init];
    mpTextFieldCell = [[NSTextFieldCell alloc] initTextCell:@""];
    mpComboBoxCell = [[NSComboBoxCell alloc] initTextCell:@""];
    mpPopUpButtonCell = [[NSPopUpButtonCell alloc] init];
    mpStepperCell = [[NSStepperCell alloc] init];
    mpListNodeCell = [[NSButtonCell alloc] init];

#if HAVE_FEATURE_SKIA
    AquaSkiaSalGraphicsImpl::prepareSkia();
#endif
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

    [mpListNodeCell release];
    [mpStepperCell release];
    [mpPopUpButtonCell release];
    [mpComboBoxCell release];
    [mpTextFieldCell release];
    [mpRadioCell release];
    [mpCheckCell release];
    [mpButtonCell release];

#if HAVE_FEATURE_SKIA
    SkiaHelper::cleanup();
#endif
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
            NotifyEvent aNCmdEvt( NotifyEventType::COMMAND, pWindow, &aCEvt );

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
        OSL_FAIL( "unhandled NSEventTypeApplicationDefined event" );
        break;
    }
}

bool AquaSalInstance::RunInMainYield( bool bHandleAllCurrentEvents )
{
    OSX_SALDATA_RUNINMAIN_UNION( DoYield( false, bHandleAllCurrentEvents), boolean )

    // PrinterController::removeTransparencies() calls this frequently on the
    // main thread so reduce the severity from an assert so that printing still
    // works in a debug builds
    SAL_WARN_IF( true, "vcl", "Don't call this from the main thread!" );
    return false;

}

static bool isWakeupEvent( NSEvent *pEvent )
{
    return NSEventTypeApplicationDefined == [pEvent type]
        && AquaSalInstance::YieldWakeupEvent == static_cast<int>([pEvent subtype]);
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
    // Related: tdf#152703 Eliminate potential blocking during live resize
    // Only native events and timers need to be dispatched to redraw
    // the window so skip dispatching user events when a window is in
    // live resize
    bool bHadEvent = ( !ImplGetSVData()->mpWinData->mbIsLiveResize && DispatchUserEvents( bHandleAllCurrentEvents ) );
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

        int noLoops = 0;
        do
        {
            SolarMutexReleaser aReleaser;

            pEvent = [NSApp nextEventMatchingMask: NSEventMaskAny
                            untilDate: [NSDate distantPast]
                            inMode: NSDefaultRunLoopMode
                            dequeue: YES];
            if( pEvent )
            {
                // tdf#155092 don't dispatch left mouse up events during live resizing
                // If this is a left mouse up event, dispatching this event
                // will trigger tdf#155092 to occur in the next mouse down
                // event. So do not dispatch this event and push it back onto
                // the front of the event queue so no more events will be
                // dispatched until live resizing ends. Surprisingly, live
                // resizing appears to end in the next mouse down event.
                if ( ImplGetSVData()->mpWinData->mbIsLiveResize && [pEvent type] == NSEventTypeLeftMouseUp )
                {
                    [NSApp postEvent: pEvent atStart: YES];
                    return false;
                }

                [NSApp sendEvent: pEvent];
                if ( isWakeupEvent( pEvent ) )
                    continue;
                bHadEvent = true;
            }

            [NSApp updateWindows];

            if ( !bHandleAllCurrentEvents || !pEvent || now < [pEvent timestamp] )
                break;
            // noelgrandin: I see sporadic hangs on the macos jenkins boxes, and the backtrace
            // points to the this loop - let us see if breaking out of here after too many
            // trips around helps.
            noLoops++;
            if (noLoops == 100)
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
        // Related: tdf#152703 Eliminate potential blocking during live resize
        // Some events and timers call Application::Reschedule() or
        // Application::Yield() so don't block and wait for events when a
        // window is in live resize
        if( bWait && ! bHadEvent && !ImplGetSVData()->mpWinData->mbIsLiveResize )
        {
            SolarMutexReleaser aReleaser;

            // attempt to fix macos jenkins hangs - part 3
            // oox::xls::WorkbookFragment::finalizeImport() calls
            // AquaSalInstance::DoYield() with bWait set to true. But
            // since unit tests generally have no expected user generated
            // events, we can end up blocking and waiting forever so
            // don't block and wait when running unit tests.
            pEvent = [NSApp nextEventMatchingMask: NSEventMaskAny
                            untilDate: SalInstance::IsRunningUnitTest() ? [NSDate distantPast] : [NSDate distantFuture]
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
    if( nType & VclInputFlags::MOUSE)
    {
        nEventMask |=
            NSEventMaskLeftMouseDown    | NSEventMaskRightMouseDown    | NSEventMaskOtherMouseDown    |
            NSEventMaskLeftMouseUp      | NSEventMaskRightMouseUp      | NSEventMaskOtherMouseUp      |
            NSEventMaskLeftMouseDragged | NSEventMaskRightMouseDragged | NSEventMaskOtherMouseDragged |
            NSEventMaskScrollWheel      |
            // NSEventMaskMouseMoved    |
            NSEventMaskMouseEntered     | NSEventMaskMouseExited;

        // Related: tdf#155266 stop delaying painting timer while swiping
        // After fixing several flushing issues in tdf#155266, scrollbars
        // still will not redraw until swiping has ended or paused when
        // using Skia/Raster or Skia disabled. So, stop the delay by only
        // including NSEventMaskScrollWheel if the current event type is
        // not NSEventTypeScrollWheel.
        NSEvent* pCurrentEvent = [NSApp currentEvent];
        if( pCurrentEvent && [pCurrentEvent type] == NSEventTypeScrollWheel )
        {
            // tdf#160767 skip fix for tdf#155266 when the event hasn't changed
            // When scrolling in Writer with automatic spellchecking enabled,
            // the current event never changes because the fix for tdf#155266
            // causes Writer to get stuck in a loop. So, if the current event
            // has not changed since the last pass through this code, skip
            // the fix for tdf#155266.
            static NSEvent *pLastCurrentEvent = nil;
            if( pLastCurrentEvent != pCurrentEvent )
            {
                if( pLastCurrentEvent )
                    [pLastCurrentEvent release];
                pLastCurrentEvent = [pCurrentEvent retain];
                nEventMask &= ~NSEventMaskScrollWheel;
            }
        }
    }

    if( nType & VclInputFlags::KEYBOARD)
        nEventMask |= NSEventMaskKeyDown | NSEventMaskKeyUp | NSEventMaskFlagsChanged;
    if( nType & VclInputFlags::OTHER)
        nEventMask |= NSEventMaskTabletPoint | NSEventMaskApplicationDefined;
    // TODO: VclInputFlags::PAINT / more VclInputFlags::OTHER
    if( !bool(nType) )
        return false;

    NSEvent* pEvent = [NSApp nextEventMatchingMask: nEventMask untilDate: [NSDate distantPast]
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

    // tdf#151700 Prevent the non-native LibreOffice PrintDialog from
    // displaying by creating a fake printer if there are no printers. This
    // will allow the LibreOffice printing code to proceed with native
    // NSPrintOperation which will display the native print panel.
    if ( !nNameCount )
    {
        std::unique_ptr<SalPrinterQueueInfo> pInfo(new SalPrinterQueueInfo);
        pInfo->maPrinterName    = getFallbackPrinterName();
        pInfo->mnStatus         = PrintQueueFlags::NONE;
        pInfo->mnJobs           = 0;

        pList->Add( std::move(pInfo) );
    }
}

void AquaSalInstance::GetPrinterQueueState( SalPrinterQueueInfo* )
{
}

OUString AquaSalInstance::GetDefaultPrinter()
{
    // #i113170# may not be the main thread if called from UNO API
    SalData::ensureThreadAutoreleasePool();

    // WinSalInstance::GetDefaultPrinter() fetches current default printer
    // on every call so do the same here
    OUString aDefaultPrinter;
    {
        NSPrintInfo* pPI = [NSPrintInfo sharedPrintInfo];
        SAL_WARN_IF( !pPI, "vcl", "no print info" );
        if( pPI )
        {
            NSPrinter* pPr = [pPI printer];
            SAL_WARN_IF( !pPr, "vcl", "no printer in default info" );
            if( pPr )
            {
                // Related: tdf#151700 Return the name of the fake printer if
                // there are no printers so that the LibreOffice printing code
                // will be able to find the fake printer returned by
                // AquaSalInstance::GetPrinterQueueInfo()
                NSString* pDefName = [pPr name];
                SAL_WARN_IF( !pDefName, "vcl", "printer has no name" );
                if ( pDefName && [pDefName length])
                    aDefaultPrinter = GetOUString( pDefName );
                else
                    aDefaultPrinter = getFallbackPrinterName();
            }
        }
    }
    return aDefaultPrinter;
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
    return OUString();
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
#if HAVE_FEATURE_SKIA
    if (SkiaHelper::isVCLSkiaEnabled())
        return std::make_shared<SkiaSalBitmap>();
    else
#endif
        return std::make_shared<QuartzSalBitmap>();
}

OUString AquaSalInstance::getOSVersion()
{
    NSString * versionString = nullptr;
    NSDictionary * sysVersionDict = [ NSDictionary dictionaryWithContentsOfFile: @"/System/Library/CoreServices/SystemVersion.plist" ];
    if ( sysVersionDict )
        versionString = [ sysVersionDict valueForKey: @"ProductVersion" ];

    OUString aVersion = "macOS ";
    if ( versionString )
        aVersion += OUString::fromUtf8( [ versionString UTF8String ] );
    else
        aVersion += "(unknown)";

    return aVersion;
}

CGImageRef CreateCGImage( const Image& rImage )
{
#if HAVE_FEATURE_SKIA
    if (SkiaHelper::isVCLSkiaEnabled())
        return SkiaHelper::createCGImage( rImage );
#endif

    BitmapEx aBmpEx( rImage.GetBitmapEx() );
    Bitmap aBmp( aBmpEx.GetBitmap() );

    if( aBmp.IsEmpty() || ! aBmp.ImplGetSalBitmap() )
        return nullptr;

    // simple case, no transparency
    QuartzSalBitmap* pSalBmp = static_cast<QuartzSalBitmap*>(aBmp.ImplGetSalBitmap().get());

    if( ! pSalBmp )
        return nullptr;

    CGImageRef xImage = nullptr;
    if( !aBmpEx.IsAlpha() )
        xImage = pSalBmp->CreateCroppedImage( 0, 0, pSalBmp->mnWidth, pSalBmp->mnHeight );
    else
    {
        AlphaMask aAlphaMask( aBmpEx.GetAlphaMask() );
        Bitmap aMask( aAlphaMask.GetBitmap() );
        QuartzSalBitmap* pMaskBmp = static_cast<QuartzSalBitmap*>(aMask.ImplGetSalBitmap().get());
        if( pMaskBmp )
            xImage = pSalBmp->CreateWithMask( *pMaskBmp, 0, 0, pSalBmp->mnWidth, pSalBmp->mnHeight );
        else
            xImage = pSalBmp->CreateCroppedImage( 0, 0, pSalBmp->mnWidth, pSalBmp->mnHeight );
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

#if OSL_DEBUG_LEVEL >= 2
    aByteExe += OString ( " NSAccessibilityDebugLogLevel 1" );
    const char* pArgv[] = { aByteExe.getStr(), NULL };
    NSApplicationMain( 3, pArgv );
#else
    const char* pArgv[] = { aByteExe.getStr(), nullptr };
    NSApplicationMain( 1, pArgv );
#endif

    return true;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
