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
#include <salimestatus.hxx>

#include <comphelper/processfactory.hxx>

#include <com/sun/star/uri/ExternalUriReferenceTranslator.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

#include <premac.h>
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
                std::unique_lock<std::mutex> g(m_runInMainMutex);
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
        std::unique_lock<std::mutex> g(m_runInMainMutex);
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
        return FALSE;
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
    ImplGetSVData()->maNWFData.mbProgressNeedsErase = true;
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
            CommandEvent aCEvt( aPoint, CommandEventId::Media, FALSE, &aMediaData );
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
    };
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

    return true; // indicate that ImplSVMainHook is implemented
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
