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

#include <config_features.h>

#include <stdio.h>

#include <comphelper/solarmutex.hxx>

#include "comphelper/lok.hxx"

#include "osl/process.h"

#include "rtl/ustrbuf.hxx"

#include <vcl/svapp.hxx>
#include <vcl/window.hxx>
#include <vcl/idle.hxx>
#include <vcl/svmain.hxx>
#include <vcl/opengl/OpenGLContext.hxx>

#include "osx/saldata.hxx"
#include "osx/salinst.h"
#include "osx/salframe.h"
#include "osx/salobj.h"
#include "osx/salsys.h"
#include "quartz/salvd.h"
#include "quartz/salbmp.h"
#include "quartz/utils.h"
#include "osx/salprn.h"
#include "osx/saltimer.h"
#include "osx/vclnsapp.h"

#include "print.h"
#include "impbmp.hxx"
#include "salimestatus.hxx"

#include <comphelper/processfactory.hxx>

#include <com/sun/star/uri/XExternalUriReferenceTranslator.hpp>
#include <com/sun/star/uri/ExternalUriReferenceTranslator.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

#include "premac.h"
#include <Foundation/Foundation.h>
#include <ApplicationServices/ApplicationServices.h>
#import "apple_remote/RemoteMainController.h"
#include "apple_remote/RemoteControl.h"
#include "postmac.h"

extern "C" {
#include <crt_externs.h>
}

using namespace std;
using namespace ::com::sun::star;

static int* gpnInit = nullptr;
static NSMenu* pDockMenu = nil;
static bool bNoSVMain = true;
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
        SalData* pSalData = GetSalData();
        if( ! pSalData->maFrames.empty() )
            pSalData->maFrames.front()->CallCallback( SalEvent::SettingsChanged, nullptr );

        if( mbInvalidate )
        {
            for( std::list< AquaSalFrame* >::iterator it = pSalData->maFrames.begin();
                it != pSalData->maFrames.end(); ++it )
            {
                if( (*it)->mbShown )
                    (*it)->SendPaintEvent();
            }
        }
        Stop();
        delete this;
    }
};

void AquaSalInstance::delayedSettingsChanged( bool bInvalidate )
{
    osl::Guard< comphelper::SolarMutex > aGuard( *mpSalYieldMutex );
    AquaDelayedSettingsChanged* pIdle = new AquaDelayedSettingsChanged( bInvalidate );
    pIdle->SetPriority( TaskPriority::MEDIUM );
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

// initialize the cocoa VCL_NSApplication object
// returns an NSAutoreleasePool that must be released when the event loop begins
static void initNSApp()
{
    // create our cocoa NSApplication
    [VCL_NSApplication sharedApplication];

    SalData::ensureThreadAutoreleasePool();

    // put cocoa into multithreaded mode
    [NSThread detachNewThreadSelector:@selector(enableCocoaThreads:) toTarget:[[CocoaThreadEnabler alloc] init] withObject:nil];

    // activate our delegate methods
    [NSApp setDelegate: NSApp];

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

bool ImplSVMainHook( int * pnInit )
{
    if (comphelper::LibreOfficeKit::isActive())
        return false;

    NSAutoreleasePool * pool = [ [ NSAutoreleasePool alloc ] init ];
    unlink([[NSString stringWithFormat:@"%@/Library/Saved Application State/%s.savedState/restorecount.plist", NSHomeDirectory(), MACOSX_BUNDLE_IDENTIFIER] UTF8String]);
    unlink([[NSString stringWithFormat:@"%@/Library/Saved Application State/%s.savedState/restorecount.txt", NSHomeDirectory(), MACOSX_BUNDLE_IDENTIFIER] UTF8String]);
    [ pool drain ];

    gpnInit = pnInit;

    bNoSVMain = false;
    initNSApp();

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

    return TRUE;   // indicate that ImplSVMainHook is implemented
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

void InitSalData()
{
    SalData *pSalData = new SalData;
    SetSalData( pSalData );
}

const OUString& SalGetDesktopEnvironment()
{
    static OUString aDesktopEnvironment( "MacOSX" );
    return aDesktopEnvironment;
}

void DeInitSalData()
{
    SalData *pSalData = GetSalData();
    if( pSalData->mpStatusItem )
    {
        [pSalData->mpStatusItem release];
        pSalData->mpStatusItem = nil;
    }
    delete pSalData;
    SetSalData( nullptr );
}

void InitSalMain()
{
}

SalYieldMutex::SalYieldMutex()
{
    mnCount  = 0;
    mnThreadId  = 0;
}

void SalYieldMutex::acquire()
{
    m_mutex.acquire();
    mnThreadId = osl::Thread::getCurrentIdentifier();
    mnCount++;
}

void SalYieldMutex::release()
{
    if ( mnThreadId == osl::Thread::getCurrentIdentifier() )
    {
        if ( mnCount == 1 )
        {
            // TODO: add OpenGLContext::prepareForYield with vcl OpenGL support
            mnThreadId = 0;
        }
        mnCount--;
    }
    m_mutex.release();
}

bool SalYieldMutex::tryToAcquire()
{
    if ( m_mutex.tryToAcquire() )
    {
        mnThreadId = osl::Thread::getCurrentIdentifier();
        mnCount++;
        return true;
    }
    else
        return false;
}

// some convenience functions regarding the yield mutex, aka solar mutex

bool ImplSalYieldMutexTryToAcquire()
{
    AquaSalInstance* pInst = GetSalData()->mpFirstInstance;
    if ( pInst )
        return pInst->mpSalYieldMutex->tryToAcquire();
    else
        return FALSE;
}

void ImplSalYieldMutexRelease()
{
    AquaSalInstance* pInst = GetSalData()->mpFirstInstance;
    if ( pInst )
        pInst->mpSalYieldMutex->release();
}

SalInstance* CreateSalInstance()
{
    // this is the case for not using SVMain
    // not so good
    if( bNoSVMain )
        initNSApp();

    SalData* pSalData = GetSalData();
    SAL_WARN_IF( pSalData->mpFirstInstance != nullptr, "vcl", "more than one instance created" );
    AquaSalInstance* pInst = new AquaSalInstance;

    // init instance (only one instance in this version !!!)
    pSalData->mpFirstInstance = pInst;
    // this one is for outside AquaSalInstance::Yield
    SalData::ensureThreadAutoreleasePool();
    // no focus rects on NWF
    ImplGetSVData()->maNWFData.mbNoFocusRects = true;
    ImplGetSVData()->maNWFData.mbNoActiveTabTextRaise = true;
    ImplGetSVData()->maNWFData.mbCenteredTabs = true;
    ImplGetSVData()->maNWFData.mbProgressNeedsErase = true;
    ImplGetSVData()->maNWFData.mbCheckBoxNeedsErase = true;
    ImplGetSVData()->maNWFData.mnStatusBarLowerRightOffset = 10;

    return pInst;
}

void DestroySalInstance( SalInstance* pInst )
{
    delete pInst;
}

AquaSalInstance::AquaSalInstance()
 : maUserEventListMutex()
{
    mpSalYieldMutex = new SalYieldMutex;
    mpSalYieldMutex->acquire();
    ::comphelper::SolarMutex::setSolarMutex( mpSalYieldMutex );
    maMainThread = osl::Thread::getCurrentIdentifier();
    mbWaitingYield = false;
    mnActivePrintJobs = 0;
    maWaitingYieldCond = osl_createCondition();
}

AquaSalInstance::~AquaSalInstance()
{
    ::comphelper::SolarMutex::setSolarMutex( nullptr );
    mpSalYieldMutex->release();
    delete mpSalYieldMutex;
    osl_destroyCondition( maWaitingYieldCond );
}

void AquaSalInstance::wakeupYield()
{
    // wakeup :Yield
    if( mbWaitingYield )
    {
        SalData::ensureThreadAutoreleasePool();
SAL_WNODEPRECATED_DECLARATIONS_PUSH
    // 'NSApplicationDefined' is deprecated: first deprecated in macOS 10.12
        NSEvent* pEvent = [NSEvent otherEventWithType: NSApplicationDefined
                                   location: NSZeroPoint
                                   modifierFlags: 0
                                   timestamp: 0
                                   windowNumber: 0
                                   context: nil
                                   subtype: AquaSalInstance::YieldWakeupEvent
                                   data1: 0
                                   data2: 0 ];
SAL_WNODEPRECATED_DECLARATIONS_POP
        if( pEvent )
            [NSApp postEvent: pEvent atStart: NO];
    }
}

void AquaSalInstance::PostUserEvent( AquaSalFrame* pFrame, SalEvent nType, void* pData )
{
    {
        osl::MutexGuard g( maUserEventListMutex );
        maUserEvents.push_back( SalUserEvent( pFrame, pData, nType ) );
    }
    // notify main loop that an event has arrived
    wakeupYield();
}

comphelper::SolarMutex* AquaSalInstance::GetYieldMutex()
{
    return mpSalYieldMutex;
}

sal_uLong AquaSalInstance::ReleaseYieldMutex()
{
    SalYieldMutex* pYieldMutex = mpSalYieldMutex;
    if ( pYieldMutex->GetThreadId() ==
         osl::Thread::getCurrentIdentifier() )
    {
        sal_uLong nCount = pYieldMutex->GetAcquireCount();
        sal_uLong n = nCount;
        while ( n )
        {
            pYieldMutex->release();
            n--;
        }

        return nCount;
    }
    else
        return 0;
}

void AquaSalInstance::AcquireYieldMutex( sal_uLong nCount )
{
    SalYieldMutex* pYieldMutex = mpSalYieldMutex;
    while ( nCount )
    {
        pYieldMutex->acquire();
        nCount--;
    }
}

bool AquaSalInstance::CheckYieldMutex()
{
    bool bRet = true;

    SalYieldMutex* pYieldMutex = mpSalYieldMutex;
    if ( pYieldMutex->GetThreadId() != osl::Thread::getCurrentIdentifier())
    {
        bRet = false;
    }

    return bRet;
}

bool AquaSalInstance::isNSAppThread() const
{
    return osl::Thread::getCurrentIdentifier() == maMainThread;
}

void AquaSalInstance::handleAppDefinedEvent( NSEvent* pEvent )
{
    int nSubtype = [pEvent subtype];
    switch( nSubtype )
    {
    case AppStartTimerEvent:
        AquaSalTimer::handleStartTimerEvent( pEvent );
        break;
    case AppEndLoopEvent:
        [NSApp stop: NSApp];
        break;
    case AppExecuteSVMain:
    {
        int nResult = ImplSVMain();
        if( gpnInit )
            *gpnInit = nResult;
        [NSApp stop: NSApp];
        bLeftMain = true;
        if( pDockMenu )
        {
            [pDockMenu release];
            pDockMenu = nil;
        }
    }
    break;
#if !HAVE_FEATURE_MACOSX_SANDBOX
    case AppleRemoteControlEvent: // Defined in <apple_remote/RemoteMainController.h>
    {
        MediaCommand nCommand;
        SalData* pSalData = GetSalData();
        bool bIsFullScreenMode = false;

        std::list<AquaSalFrame*>::iterator it = pSalData->maFrames.begin();
        while( it != pSalData->maFrames.end() )
        {
            if ( (*it) && (*it)->mbFullScreen )
                bIsFullScreenMode = true;
            ++it;
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
        AquaSalFrame* pFrame = pSalData->maFrames.front();
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

class ReleasePoolHolder
{
    NSAutoreleasePool* mpPool;
    public:
    ReleasePoolHolder() : mpPool( [[NSAutoreleasePool alloc] init] ) {}
    ~ReleasePoolHolder() { [mpPool release]; }
};

SalYieldResult AquaSalInstance::DoYield(bool bWait, bool bHandleAllCurrentEvents, sal_uLong const nReleased)
{
    (void) nReleased;
    assert(nReleased == 0); // not implemented

    // ensure that the per thread autorelease pool is top level and
    // will therefore not be destroyed by cocoa implicitly
    SalData::ensureThreadAutoreleasePool();

    // NSAutoreleasePool documentation suggests we should have
    // an own pool for each yield level
    ReleasePoolHolder aReleasePool;

    // Release all locks so that we don't deadlock when we pull pending
    // events from the event queue
    bool bDispatchUser = true;
    while( bDispatchUser )
    {
        sal_uLong nCount = ReleaseYieldMutex();

        // get one user event
        SalUserEvent aEvent( nullptr, nullptr, SalEvent::NONE );
        {
            osl::MutexGuard g( maUserEventListMutex );
            if( ! maUserEvents.empty() )
            {
                aEvent = maUserEvents.front();
                maUserEvents.pop_front();
            }
            else
                bDispatchUser = false;
        }
        AcquireYieldMutex( nCount );

        // dispatch it
        if( aEvent.mpFrame && AquaSalFrame::isAlive( aEvent.mpFrame ) )
        {
            aEvent.mpFrame->CallCallback( aEvent.mnType, aEvent.mpData );
            osl_setCondition( maWaitingYieldCond );
            // return if only one event is asked for
            if( ! bHandleAllCurrentEvents )
                return SalYieldResult::EVENT;
        }
    }

    // handle cocoa event queue
    // cocoa events may be only handled in the thread the NSApp was created
    bool bHadEvent = false;
    if( isNSAppThread() && mnActivePrintJobs == 0 )
    {
        // we need to be woken up by a cocoa-event
        // if a user event should be posted by the event handling below
        bool bOldWaitingYield = mbWaitingYield;
        mbWaitingYield = bWait;

        // handle available events
        NSEvent* pEvent = nil;
        do
        {
            sal_uLong nCount = ReleaseYieldMutex();

SAL_WNODEPRECATED_DECLARATIONS_PUSH
    // 'NSAnyEventMask' is deprecated: first deprecated in macOS 10.12
            pEvent = [NSApp nextEventMatchingMask: NSAnyEventMask untilDate: nil
SAL_WNODEPRECATED_DECLARATIONS_POP
                            inMode: NSDefaultRunLoopMode dequeue: YES];
            if( pEvent )
            {
                [NSApp sendEvent: pEvent];
                bHadEvent = true;
            }
            [NSApp updateWindows];

            AcquireYieldMutex( nCount );
        } while( bHandleAllCurrentEvents && pEvent );

        // if we had no event yet, wait for one if requested
        if( bWait && ! bHadEvent )
        {
            sal_uLong nCount = ReleaseYieldMutex();

            NSDate* pDt = AquaSalTimer::pRunningTimer ? [AquaSalTimer::pRunningTimer fireDate] : [NSDate distantFuture];
SAL_WNODEPRECATED_DECLARATIONS_PUSH
    // 'NSAnyEventMask' is deprecated: first deprecated in macOS 10.12
            pEvent = [NSApp nextEventMatchingMask: NSAnyEventMask untilDate: pDt
SAL_WNODEPRECATED_DECLARATIONS_POP
                            inMode: NSDefaultRunLoopMode dequeue: YES];
            if( pEvent )
                [NSApp sendEvent: pEvent];
            [NSApp updateWindows];

            AcquireYieldMutex( nCount );

            // #i86581#
            // FIXME: sometimes the NSTimer will never fire. Firing it by hand then
            // fixes the problem even seems to set the correct next firing date
            // Why oh why?
            if( ! pEvent && AquaSalTimer::pRunningTimer )
            {
                // this cause crashes on MacOSX 10.4
                // [AquaSalTimer::pRunningTimer fire];
                if (ImplGetSVData()->mpSalTimer != nullptr)
                {
                    bool idle = true; // TODO
                    ImplGetSVData()->mpSalTimer->CallCallback( idle );
                }
            }
        }

        mbWaitingYield = bOldWaitingYield;

        // collect update rectangles
        const std::list< AquaSalFrame* > rFrames( GetSalData()->maFrames );
        for( std::list< AquaSalFrame* >::const_iterator it = rFrames.begin(); it != rFrames.end(); ++it )
        {
            if( (*it)->mbShown && ! (*it)->maInvalidRect.IsEmpty() )
            {
                (*it)->Flush( (*it)->maInvalidRect );
                (*it)->maInvalidRect.SetEmpty();
            }
        }
        osl_setCondition( maWaitingYieldCond );
    }
    else if( bWait )
    {
        // #i103162#
        // wait until any thread (most likely the main thread)
        // has dispatched an event, cop out at 200 ms
        osl_resetCondition( maWaitingYieldCond );
        TimeValue aVal = { 0, 200000000 };
        sal_uLong nCount = ReleaseYieldMutex();
        osl_waitCondition( maWaitingYieldCond, &aVal );
        AcquireYieldMutex( nCount );
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

    return bHadEvent ? SalYieldResult::EVENT : SalYieldResult::TIMEOUT;
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

    if( nType & VclInputFlags::TIMER )
    {
        if( AquaSalTimer::pRunningTimer )
        {
            NSDate* pDt = [AquaSalTimer::pRunningTimer fireDate];
            if( pDt && [pDt timeIntervalSinceNow] < 0 )
            {
                return true;
            }
        }
    }

    if (![NSThread isMainThread])
        return false;

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
        nEventMask |= NSTabletPoint;
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
    SalData::ensureThreadAutoreleasePool();

    SalFrame* pFrame = new AquaSalFrame( pParent, nSalFrameStyle );
    return pFrame;
}

void AquaSalInstance::DestroyFrame( SalFrame* pFrame )
{
    delete pFrame;
}

SalObject* AquaSalInstance::CreateObject( SalFrame* pParent, SystemWindowData* pWindowData, bool /* bShow */ )
{
    AquaSalObject *pObject = nullptr;

    if ( pParent )
        pObject = new AquaSalObject( static_cast<AquaSalFrame*>(pParent), pWindowData );

    return pObject;
}

void AquaSalInstance::DestroyObject( SalObject* pObject )
{
    delete ( pObject );
}

SalPrinter* AquaSalInstance::CreatePrinter( SalInfoPrinter* pInfoPrinter )
{
    return new AquaSalPrinter( dynamic_cast<AquaSalInfoPrinter*>(pInfoPrinter) );
}

void AquaSalInstance::DestroyPrinter( SalPrinter* pPrinter )
{
    delete pPrinter;
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
            SalPrinterQueueInfo* pInfo = new SalPrinterQueueInfo;
            pInfo->maPrinterName    = GetOUString( pName );
            if( pType )
                pInfo->maDriver     = GetOUString( pType );
            pInfo->mnStatus         = PrintQueueFlags::NONE;
            pInfo->mnJobs           = 0;
            pInfo->mpSysData        = nullptr;

            pList->Add( pInfo );
        }
    }
}

void AquaSalInstance::GetPrinterQueueState( SalPrinterQueueInfo* )
{
}

void AquaSalInstance::DeletePrinterQueueInfo( SalPrinterQueueInfo* pInfo )
{
    delete pInfo;
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
    return OUString("");
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

SalBitmap* AquaSalInstance::CreateSalBitmap()
{
    return new QuartzSalBitmap();
}

SalSession* AquaSalInstance::CreateSalSession()
{
    return nullptr;
}

OUString AquaSalInstance::getOSVersion()
{
    NSString * versionString = nullptr;
    NSString * sysVersionDictionaryPath = @"/System/Library/CoreServices/SystemVersion.plist";
    NSDictionary * sysVersionDict = [ NSDictionary dictionaryWithContentsOfFile: sysVersionDictionaryPath ];
    if ( sysVersionDict )
        versionString = [ sysVersionDict valueForKey: @"ProductVersion" ];

    OUString aVersion = "Mac OS X ";
    if ( versionString )
        aVersion += OUString::fromUtf8( [ versionString UTF8String ] );
    else
        aVersion += "(unknown)";

    return aVersion;
}

// YieldMutexReleaser
YieldMutexReleaser::YieldMutexReleaser() : mnCount( 0 )
{
    SalData* pSalData = GetSalData();
    if( ! pSalData->mpFirstInstance->isNSAppThread() )
    {
        SalData::ensureThreadAutoreleasePool();
        mnCount = pSalData->mpFirstInstance->ReleaseYieldMutex();
    }
}

YieldMutexReleaser::~YieldMutexReleaser()
{
    if( mnCount != 0 )
        GetSalData()->mpFirstInstance->AcquireYieldMutex( mnCount );
}

CGImageRef CreateCGImage( const Image& rImage )
{
    BitmapEx aBmpEx( rImage.GetBitmapEx() );
    Bitmap aBmp( aBmpEx.GetBitmap() );

    if( ! aBmp || ! aBmp.ImplGetImpBitmap() )
        return nullptr;

    // simple case, no transparency
    QuartzSalBitmap* pSalBmp = static_cast<QuartzSalBitmap*>(aBmp.ImplGetImpBitmap()->ImplGetSalBitmap());

    if( ! pSalBmp )
        return nullptr;

    CGImageRef xImage = nullptr;
    if( ! (aBmpEx.IsAlpha() || aBmpEx.IsTransparent() ) )
        xImage = pSalBmp->CreateCroppedImage( 0, 0, pSalBmp->mnWidth, pSalBmp->mnHeight );
    else if( aBmpEx.IsAlpha() )
    {
        AlphaMask aAlphaMask( aBmpEx.GetAlpha() );
        Bitmap aMask( aAlphaMask.GetBitmap() );
        QuartzSalBitmap* pMaskBmp = static_cast<QuartzSalBitmap*>(aMask.ImplGetImpBitmap()->ImplGetSalBitmap());
        if( pMaskBmp )
            xImage = pSalBmp->CreateWithMask( *pMaskBmp, 0, 0, pSalBmp->mnWidth, pSalBmp->mnHeight );
        else
            xImage = pSalBmp->CreateCroppedImage( 0, 0, pSalBmp->mnWidth, pSalBmp->mnHeight );
    }
    else if( aBmpEx.GetTransparentType() == TransparentType::Bitmap )
    {
        Bitmap aMask( aBmpEx.GetMask() );
        QuartzSalBitmap* pMaskBmp = static_cast<QuartzSalBitmap*>(aMask.ImplGetImpBitmap()->ImplGetSalBitmap());
        if( pMaskBmp )
            xImage = pSalBmp->CreateWithMask( *pMaskBmp, 0, 0, pSalBmp->mnWidth, pSalBmp->mnHeight );
        else
            xImage = pSalBmp->CreateCroppedImage( 0, 0, pSalBmp->mnWidth, pSalBmp->mnHeight );
    }
    else if( aBmpEx.GetTransparentType() == TransparentType::Color )
    {
        Color aTransColor( aBmpEx.GetTransparentColor() );
        SalColor nTransColor = MAKE_SALCOLOR( aTransColor.GetRed(), aTransColor.GetGreen(), aTransColor.GetBlue() );
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
        CGContextRef rCGContext = static_cast<CGContextRef>([pContext graphicsPort]);

        const CGRect aDstRect = { {0, 0}, { static_cast<CGFloat>(aSize.Width()), static_cast<CGFloat>(aSize.Height()) } };
        CGContextDrawImage( rCGContext, aDstRect, xImage );

        [pImage unlockFocus];
    }

    CGImageRelease( xImage );

    return pImage;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
