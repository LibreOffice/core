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


#include <stdio.h>

#include "tools/fsys.hxx"
#include "tools/getprocessworkingdir.hxx"
#include <tools/solarmutex.hxx>

#include "osl/process.h"

#include "rtl/ustrbuf.hxx"

#include "vcl/svapp.hxx"
#include "vcl/window.hxx"
#include "vcl/timer.hxx"
#include "vcl/solarmutex.hxx"

#include "ios/saldata.hxx"
#include "ios/salinst.h"
#include "ios/salframe.h"
#include "ios/salobj.h"
#include "ios/salsys.h"
#include "ios/salvd.h"
#include "ios/salbmp.h"
#include "ios/salprn.h"
#include "ios/saltimer.h"
#include "ios/vcluiapp.h"

#include "print.h"
#include "impbmp.hxx"
#include "salimestatus.hxx"

#include <comphelper/processfactory.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/uri/XExternalUriReferenceTranslator.hpp>
#include <com/sun/star/uri/ExternalUriReferenceTranslator.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

using namespace std;
using namespace ::com::sun::star;

extern sal_Bool ImplSVMain();

static int* gpnInit = 0;
static bool bNoSVMain = true;
// -----------------------------------------------------------------------

class IosDelayedSettingsChanged : public Timer
{
    bool            mbInvalidate;
    public:
    IosDelayedSettingsChanged( bool bInvalidate ) :
        mbInvalidate( bInvalidate )
    {
    }

    virtual void Timeout()
    {
        SalData* pSalData = GetSalData();
        if( ! pSalData->maFrames.empty() )
            pSalData->maFrames.front()->CallCallback( SALEVENT_SETTINGSCHANGED, NULL );

        if( mbInvalidate )
        {
            for( std::list< IosSalFrame* >::iterator it = pSalData->maFrames.begin();
                it != pSalData->maFrames.end(); ++it )
            {
                if( (*it)->mbShown )
                    (*it)->SendPaintEvent( NULL );
            }
        }
        Stop();
        delete this;
    }
};

void IosSalInstance::delayedSettingsChanged( bool bInvalidate )
{
    osl::SolarGuard aGuard( *mpSalYieldMutex );
    IosDelayedSettingsChanged* pTimer = new IosDelayedSettingsChanged( bInvalidate );
    pTimer->SetTimeout( 50 );
    pTimer->Start();
}


// the AppEventList must be available before any SalData/SalInst/etc. objects are ready
typedef std::list<const ApplicationEvent*> AppEventList;
AppEventList IosSalInstance::aAppEventList;

// initialize the VCL_UIApplication object
static void initUIApp()
{
    [VCL_UIApplication sharedApplication];

    SalData::ensureThreadAutoreleasePool();
}

sal_Bool ImplSVMainHook( int * pnInit )
{
    char sMain[] = "main";
    gpnInit = pnInit;

    bNoSVMain = false;
    initUIApp();

    char* pArgv[] = { sMain, NULL };
    UIApplicationMain( 1, pArgv, NULL, NULL );

    return TRUE;   // indicate that ImplSVMainHook is implemented
}

// =======================================================================

void SalAbort( const rtl::OUString& rErrorText, bool bDumpCore )
{
    if( rErrorText.isEmpty() )
        fprintf( stderr, "Application Error " );
    else
        fprintf( stderr, "%s ",
            rtl::OUStringToOString( rErrorText, osl_getThreadTextEncoding() ).getStr() );
    if( bDumpCore )
        abort();
    else
        _exit(1);
}

// -----------------------------------------------------------------------

void InitSalData()
{
    SalData *pSalData = new SalData;
    SetSalData( pSalData );
}

// -----------------------------------------------------------------------

const ::rtl::OUString& SalGetDesktopEnvironment()
{
    static OUString aDesktopEnvironment(RTL_CONSTASCII_USTRINGPARAM( "CocoaTouch" ));
    return aDesktopEnvironment;
}

// -----------------------------------------------------------------------

void DeInitSalData()
{
    SalData *pSalData = GetSalData();
    delete pSalData;
    SetSalData( NULL );
}

// -----------------------------------------------------------------------

void InitSalMain()
{
    rtl::OUString urlWorkDir;
    rtl_uString *sysWorkDir = NULL;
    if (tools::getProcessWorkingDir(urlWorkDir))
    {
        oslFileError err2 = osl_getSystemPathFromFileURL(urlWorkDir.pData, &sysWorkDir);
        if (err2 == osl_File_E_None)
        {
            rtl::OString aPath( getenv( "PATH" ) );
            rtl::OString aResPath( getenv( "STAR_RESOURCEPATH" ) );
            rtl::OString aCmdPath( OUStringToOString(OUString(sysWorkDir), RTL_TEXTENCODING_UTF8).getStr() );
            rtl::OString aTmpPath;
            // Get absolute path of command's directory
            if ( !aCmdPath.isEmpty() )
            {
                DirEntry aCmdDirEntry( aCmdPath );
                aCmdDirEntry.ToAbs();
                aCmdPath = rtl::OUStringToOString( aCmdDirEntry.GetPath().GetFull(), RTL_TEXTENCODING_ASCII_US );
            }
            // Assign to PATH environment variable
            if ( !aCmdPath.isEmpty() )
            {
                aTmpPath = rtl::OString( "PATH=" );
                aTmpPath += aCmdPath;
                if ( !aPath.isEmpty() )
                    aTmpPath += rtl::OUStringToOString( DirEntry::GetSearchDelimiter(), RTL_TEXTENCODING_ASCII_US );
                aTmpPath += aPath;
                putenv( (char*)aTmpPath.getStr() );
            }
            // Assign to STAR_RESOURCEPATH environment variable
            if ( !aCmdPath.isEmpty() )
            {
                aTmpPath = rtl::OString( "STAR_RESOURCEPATH=" );
                aTmpPath += aCmdPath;
                if ( !aResPath.isEmpty() )
                    aTmpPath += rtl::OUStringToOString( DirEntry::GetSearchDelimiter(), RTL_TEXTENCODING_ASCII_US );
                aTmpPath += aResPath;
                putenv( (char*)aTmpPath.getStr() );
            }
        }
    }
}

// -----------------------------------------------------------------------

void DeInitSalMain()
{
}

// =======================================================================

SalYieldMutex::SalYieldMutex()
{
    mnCount  = 0;
    mnThreadId  = 0;
}

void SalYieldMutex::acquire()
{
    SolarMutexObject::acquire();
    mnThreadId = osl::Thread::getCurrentIdentifier();
    mnCount++;
}

void SalYieldMutex::release()
{
    if ( mnThreadId == osl::Thread::getCurrentIdentifier() )
    {
        if ( mnCount == 1 )
            mnThreadId = 0;
        mnCount--;
    }
    SolarMutexObject::release();
}

sal_Bool SalYieldMutex::tryToAcquire()
{
    if ( SolarMutexObject::tryToAcquire() )
    {
        mnThreadId = osl::Thread::getCurrentIdentifier();
        mnCount++;
        return sal_True;
    }
    else
        return sal_False;
}

// -----------------------------------------------------------------------

// some convenience functions regarding the yield mutex, aka solar mutex

sal_Bool ImplSalYieldMutexTryToAcquire()
{
    IosSalInstance* pInst = (IosSalInstance*) GetSalData()->mpFirstInstance;
    if ( pInst )
        return pInst->mpSalYieldMutex->tryToAcquire();
    else
        return FALSE;
}

void ImplSalYieldMutexAcquire()
{
    IosSalInstance* pInst = (IosSalInstance*) GetSalData()->mpFirstInstance;
    if ( pInst )
        pInst->mpSalYieldMutex->acquire();
}

void ImplSalYieldMutexRelease()
{
    IosSalInstance* pInst = (IosSalInstance*) GetSalData()->mpFirstInstance;
    if ( pInst )
        pInst->mpSalYieldMutex->release();
}

// =======================================================================

SalInstance* CreateSalInstance()
{
    // this is the case for not using SVMain
    // not so good
    if( bNoSVMain )
        initUIApp();

    SalData* pSalData = GetSalData();
    DBG_ASSERT( pSalData->mpFirstInstance == NULL, "more than one instance created" );
    IosSalInstance* pInst = new IosSalInstance;

    // init instance (only one instance in this version !!!)
    pSalData->mpFirstInstance = pInst;
    // this one is for outside IosSalInstance::Yield
    SalData::ensureThreadAutoreleasePool();
    // no focus rects on NWF ios
    ImplGetSVData()->maNWFData.mbNoFocusRects = true;
    ImplGetSVData()->maNWFData.mbNoActiveTabTextRaise = true;
    ImplGetSVData()->maNWFData.mbCenteredTabs = true;
    ImplGetSVData()->maNWFData.mbProgressNeedsErase = true;
    ImplGetSVData()->maNWFData.mbCheckBoxNeedsErase = true;
    ImplGetSVData()->maNWFData.mnStatusBarLowerRightOffset = 10;
    ImplGetSVData()->maGDIData.mbNoXORClipping = true;
    ImplGetSVData()->maWinData.mbNoSaveBackground = true;

    return pInst;
}

// -----------------------------------------------------------------------

void DestroySalInstance( SalInstance* pInst )
{
    delete pInst;
}

// -----------------------------------------------------------------------

IosSalInstance::IosSalInstance()
{
    mpSalYieldMutex = new SalYieldMutex;
    mpSalYieldMutex->acquire();
    ::tools::SolarMutex::SetSolarMutex( mpSalYieldMutex );
    maMainThread = osl::Thread::getCurrentIdentifier();
    mbWaitingYield = false;
    maUserEventListMutex = osl_createMutex();
    mnActivePrintJobs = 0;
    maWaitingYieldCond = osl_createCondition();
}

// -----------------------------------------------------------------------

IosSalInstance::~IosSalInstance()
{
    ::tools::SolarMutex::SetSolarMutex( 0 );
    mpSalYieldMutex->release();
    delete mpSalYieldMutex;
    osl_destroyMutex( maUserEventListMutex );
    osl_destroyCondition( maWaitingYieldCond );
}

// -----------------------------------------------------------------------

void IosSalInstance::wakeupYield()
{
}

// -----------------------------------------------------------------------

void IosSalInstance::PostUserEvent( IosSalFrame* pFrame, sal_uInt16 nType, void* pData )
{
    osl_acquireMutex( maUserEventListMutex );
    maUserEvents.push_back( SalUserEvent( pFrame, pData, nType ) );
    osl_releaseMutex( maUserEventListMutex );

    // notify main loop that an event has arrived
    wakeupYield();
}

// -----------------------------------------------------------------------

osl::SolarMutex* IosSalInstance::GetYieldMutex()
{
    return mpSalYieldMutex;
}

// -----------------------------------------------------------------------

sal_uLong IosSalInstance::ReleaseYieldMutex()
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

// -----------------------------------------------------------------------

void IosSalInstance::AcquireYieldMutex( sal_uLong nCount )
{
    SalYieldMutex* pYieldMutex = mpSalYieldMutex;
    while ( nCount )
    {
        pYieldMutex->acquire();
        nCount--;
    }
}

// -----------------------------------------------------------------------

bool IosSalInstance::CheckYieldMutex()
{
    bool bRet = true;

    SalYieldMutex* pYieldMutex = mpSalYieldMutex;
    if ( pYieldMutex->GetThreadId() != osl::Thread::getCurrentIdentifier())
    {
        bRet = false;
    }

    return bRet;
}

// -----------------------------------------------------------------------

bool IosSalInstance::isUIAppThread() const
{
    return osl::Thread::getCurrentIdentifier() == maMainThread;
}

// -----------------------------------------------------------------------

class ReleasePoolHolder
{
    NSAutoreleasePool* mpPool;
    public:
    ReleasePoolHolder() : mpPool( [[NSAutoreleasePool alloc] init] ) {}
    ~ReleasePoolHolder() { [mpPool release]; }
};

void IosSalInstance::Yield( bool bWait, bool bHandleAllCurrentEvents )
{
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
        osl_acquireMutex( maUserEventListMutex );
        SalUserEvent aEvent( NULL, NULL, 0 );
        if( ! maUserEvents.empty() )
        {
            aEvent = maUserEvents.front();
            maUserEvents.pop_front();
        }
        else
            bDispatchUser = false;
        osl_releaseMutex( maUserEventListMutex );

        AcquireYieldMutex( nCount );

        // dispatch it
        if( aEvent.mpFrame && IosSalFrame::isAlive( aEvent.mpFrame ) )
        {
            aEvent.mpFrame->CallCallback( aEvent.mnType, aEvent.mpData );
            osl_setCondition( maWaitingYieldCond );
            // return if only one event is asked for
            if( ! bHandleAllCurrentEvents )
                return;
        }
    }

    // handle event queue
    // events mye be only handled in the thread the app was created
    if( mnActivePrintJobs == 0 )
    {
        // we need to be woken up by a cocoa-event
        // if a user event should be posted by the event handling below
        bool bOldWaitingYield = mbWaitingYield;
        mbWaitingYield = bWait;

        mbWaitingYield = bOldWaitingYield;

        // collect update rectangles
        const std::list< IosSalFrame* > rFrames( GetSalData()->maFrames );
        for( std::list< IosSalFrame* >::const_iterator it = rFrames.begin(); it != rFrames.end(); ++it )
        {
            if( (*it)->mbShown && ! (*it)->maInvalidRect.IsEmpty() )
            {
                (*it)->Flush( (*it)->maInvalidRect );
                (*it)->maInvalidRect.SetEmpty();
            }
        }
        osl_setCondition( maWaitingYieldCond );
    }
}

// -----------------------------------------------------------------------

bool IosSalInstance::AnyInput( sal_uInt16 nType )
{
    if( nType & VCL_INPUT_APPEVENT )
    {
        if( ! aAppEventList.empty() )
            return true;
        if( nType == VCL_INPUT_APPEVENT )
            return false;
    }

    if( nType & VCL_INPUT_TIMER )
    {
        if( IosSalTimer::pRunningTimer )
        {
            NSDate* pDt = [IosSalTimer::pRunningTimer fireDate];
            if( pDt && [pDt timeIntervalSinceNow] < 0 )
            {
                return true;
            }
        }
    }
    return false;
}

// -----------------------------------------------------------------------

SalFrame* IosSalInstance::CreateChildFrame( SystemParentData*, sal_uLong /*nSalFrameStyle*/ )
{
    return NULL;
}

// -----------------------------------------------------------------------

SalFrame* IosSalInstance::CreateFrame( SalFrame* pParent, sal_uLong nSalFrameStyle )
{
    SalData::ensureThreadAutoreleasePool();

    SalFrame* pFrame = new IosSalFrame( pParent, nSalFrameStyle );
    return pFrame;
}

// -----------------------------------------------------------------------

void IosSalInstance::DestroyFrame( SalFrame* pFrame )
{
    delete pFrame;
}

// -----------------------------------------------------------------------

SalObject* IosSalInstance::CreateObject( SalFrame* pParent, SystemWindowData* /* pWindowData */, sal_Bool /* bShow */ )
{
    // SystemWindowData is meaningless on Mac OS X
    IosSalObject *pObject = NULL;

    if ( pParent )
        pObject = new IosSalObject( static_cast<IosSalFrame*>(pParent) );

    return pObject;
}

// -----------------------------------------------------------------------

void IosSalInstance::DestroyObject( SalObject* pObject )
{
    delete ( pObject );
}

// -----------------------------------------------------------------------

SalPrinter* IosSalInstance::CreatePrinter( SalInfoPrinter* pInfoPrinter )
{
    (void) pInfoPrinter;
    return NULL;
}

// -----------------------------------------------------------------------

void IosSalInstance::DestroyPrinter( SalPrinter* pPrinter )
{
    delete pPrinter;
}

// -----------------------------------------------------------------------

void IosSalInstance::GetPrinterQueueInfo( ImplPrnQueueList* pList )
{
    // ???
    (void) pList;
}

// -----------------------------------------------------------------------

void IosSalInstance::GetPrinterQueueState( SalPrinterQueueInfo* )
{
    // ???
}

// -----------------------------------------------------------------------

void IosSalInstance::DeletePrinterQueueInfo( SalPrinterQueueInfo* pInfo )
{
    delete pInfo;
}

// -----------------------------------------------------------------------

rtl::OUString IosSalInstance::GetDefaultPrinter()
{
    // #i113170# may not be the main thread if called from UNO API
    SalData::ensureThreadAutoreleasePool();

    // ???
    return maDefaultPrinter;
}

// -----------------------------------------------------------------------

SalInfoPrinter* IosSalInstance::CreateInfoPrinter( SalPrinterQueueInfo* pQueueInfo,
                                                   ImplJobSetup* pSetupData )
{
    (void) pQueueInfo;
    (void) pSetupData;

    // #i113170# may not be the main thread if called from UNO API
    SalData::ensureThreadAutoreleasePool();

    SalInfoPrinter* pNewInfoPrinter = NULL;

    // ???

    return pNewInfoPrinter;
}

// -----------------------------------------------------------------------

void IosSalInstance::DestroyInfoPrinter( SalInfoPrinter* pPrinter )
{
    // #i113170# may not be the main thread if called from UNO API
    SalData::ensureThreadAutoreleasePool();

    delete pPrinter;
}

// -----------------------------------------------------------------------

SalSystem* IosSalInstance::CreateSystem()
{
    return new IosSalSystem();
}

// -----------------------------------------------------------------------

void IosSalInstance::DestroySystem( SalSystem* pSystem )
{
    delete pSystem;
}

// -----------------------------------------------------------------------

void IosSalInstance::SetEventCallback( void*, bool(*)(void*,void*,int) )
{
}

// -----------------------------------------------------------------------

void IosSalInstance::SetErrorEventCallback( void*, bool(*)(void*,void*,int) )
{
}

// -----------------------------------------------------------------------

void* IosSalInstance::GetConnectionIdentifier( ConnectionIdentifierType& rReturnedType, int& rReturnedBytes )
{
    rReturnedBytes  = 1;
    rReturnedType   = AsciiCString;
    return (void*)"";
}

void IosSalInstance::AddToRecentDocumentList(const rtl::OUString& /*rFileUrl*/, const rtl::OUString& /*rMimeType*/)
{
}

// -----------------------------------------------------------------------

SalTimer* IosSalInstance::CreateSalTimer()
{
    return new IosSalTimer();
}

// -----------------------------------------------------------------------

SalSystem* IosSalInstance::CreateSalSystem()
{
    return new IosSalSystem();
}

// -----------------------------------------------------------------------

SalBitmap* IosSalInstance::CreateSalBitmap()
{
    return new IosSalBitmap();
}

// -----------------------------------------------------------------------

SalSession* IosSalInstance::CreateSalSession()
{
    return NULL;
}

// -----------------------------------------------------------------------

class IOsImeStatus : public SalI18NImeStatus
{
public:
    IOsImeStatus() {}
    virtual ~IOsImeStatus() {}

    // asks whether there is a status window available
    // to toggle into menubar
    virtual bool canToggle() { return false; }
    virtual void toggle() {}
};

// -----------------------------------------------------------------------

SalI18NImeStatus* IosSalInstance::CreateI18NImeStatus()
{
    return new IOsImeStatus();
}

// -----------------------------------------------------------------------

// YieldMutexReleaser
YieldMutexReleaser::YieldMutexReleaser() : mnCount( 0 )
{
    SalData* pSalData = GetSalData();
    if( ! pSalData->mpFirstInstance->isUIAppThread() )
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

//////////////////////////////////////////////////////////////
rtl::OUString GetOUString( CFStringRef rStr )
{
    if( rStr == 0 )
        return rtl::OUString();
    CFIndex nLength = CFStringGetLength( rStr );
    if( nLength == 0 )
        return rtl::OUString();
    const UniChar* pConstStr = CFStringGetCharactersPtr( rStr );
    if( pConstStr )
        return rtl::OUString( pConstStr, nLength );
    UniChar* pStr = reinterpret_cast<UniChar*>( rtl_allocateMemory( sizeof(UniChar)*nLength ) );
    CFRange aRange = { 0, nLength };
    CFStringGetCharacters( rStr, aRange, pStr );
    rtl::OUString aRet( pStr, nLength );
    rtl_freeMemory( pStr );
    return aRet;
}

rtl::OUString GetOUString( NSString* pStr )
{
    if( ! pStr )
        return rtl::OUString();
    int nLen = [pStr length];
    if( nLen == 0 )
        return rtl::OUString();

    rtl::OUStringBuffer aBuf( nLen+1 );
    aBuf.setLength( nLen );
    [pStr getCharacters: const_cast<sal_Unicode*>(aBuf.getStr())];
    return aBuf.makeStringAndClear();
}

CFStringRef CreateCFString( const rtl::OUString& rStr )
{
    return CFStringCreateWithCharacters(kCFAllocatorDefault, rStr.getStr(), rStr.getLength() );
}

NSString* CreateNSString( const rtl::OUString& rStr )
{
    return [[NSString alloc] initWithCharacters: rStr.getStr() length: rStr.getLength()];
}

CGImageRef CreateCGImage( const Image& rImage )
{
    BitmapEx aBmpEx( rImage.GetBitmapEx() );
    Bitmap aBmp( aBmpEx.GetBitmap() );

    if( ! aBmp || ! aBmp.ImplGetImpBitmap() )
        return NULL;

    // simple case, no transparency
    IosSalBitmap* pSalBmp = static_cast<IosSalBitmap*>(aBmp.ImplGetImpBitmap()->ImplGetSalBitmap());

    if( ! pSalBmp )
        return NULL;

    CGImageRef xImage = NULL;
    if( ! (aBmpEx.IsAlpha() || aBmpEx.IsTransparent() ) )
        xImage = pSalBmp->CreateCroppedImage( 0, 0, pSalBmp->mnWidth, pSalBmp->mnHeight );
    else if( aBmpEx.IsAlpha() )
    {
        AlphaMask aAlphaMask( aBmpEx.GetAlpha() );
        Bitmap aMask( aAlphaMask.GetBitmap() );
        IosSalBitmap* pMaskBmp = static_cast<IosSalBitmap*>(aMask.ImplGetImpBitmap()->ImplGetSalBitmap());
        if( pMaskBmp )
            xImage = pSalBmp->CreateWithMask( *pMaskBmp, 0, 0, pSalBmp->mnWidth, pSalBmp->mnHeight );
        else
            xImage = pSalBmp->CreateCroppedImage( 0, 0, pSalBmp->mnWidth, pSalBmp->mnHeight );
    }
    else if( aBmpEx.GetTransparentType() == TRANSPARENT_BITMAP )
    {
        Bitmap aMask( aBmpEx.GetMask() );
        IosSalBitmap* pMaskBmp = static_cast<IosSalBitmap*>(aMask.ImplGetImpBitmap()->ImplGetSalBitmap());
        if( pMaskBmp )
            xImage = pSalBmp->CreateWithMask( *pMaskBmp, 0, 0, pSalBmp->mnWidth, pSalBmp->mnHeight );
        else
            xImage = pSalBmp->CreateCroppedImage( 0, 0, pSalBmp->mnWidth, pSalBmp->mnHeight );
    }
    else if( aBmpEx.GetTransparentType() == TRANSPARENT_COLOR )
    {
        Color aTransColor( aBmpEx.GetTransparentColor() );
        SalColor nTransColor = MAKE_SALCOLOR( aTransColor.GetRed(), aTransColor.GetGreen(), aTransColor.GetBlue() );
        xImage = pSalBmp->CreateColorMask( 0, 0, pSalBmp->mnWidth, pSalBmp->mnHeight, nTransColor );
    }

    return xImage;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
