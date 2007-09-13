/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: salinst.cxx,v $
 *
 *  $Revision: 1.39 $
 *
 *  last change: $Author: ihi $ $Date: 2007-09-13 16:31:03 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_vcl.hxx"

#include <stdio.h>

#ifndef _SV_SALDATA_HXX
#include <saldata.hxx>
#endif
#ifndef _SV_SALINST_HXX
#include <vcl/salinst.hxx>
#endif
#ifndef _SV_SALINST_H
#include <salinst.h>
#endif
#ifndef _SV_SALFRAME_H
#include <salframe.h>
#endif
#ifndef _SV_SALOBJ_HXX
#include <vcl/salobj.hxx>
#endif
#ifndef _SV_SALOBJ_H
#include <salobj.h>
#endif
#ifndef _SV_SALSYS_HXX
#include <vcl/salsys.hxx>
#endif
#ifndef _SV_SALSYS_H
#include <salsys.h>
#endif
#ifndef _SV_SALVD_HXX
#include <vcl/salvd.hxx>
#endif
#ifndef _SV_SALVD_H
#include <salvd.h>
#endif
#ifndef _SV_DIALOG_HXX
#include <vcl/dialog.hxx>
#endif
#ifndef _FSYS_HXX
#include <tools/fsys.hxx>
#endif
#ifndef _SV_SALIMESTATUS_HXX
#include <vcl/salimestatus.hxx>
#endif

#include <vcl/svapp.hxx>

#include <salprn.h>
#include <vcl/print.h>

#include <salogl.h>
#include <saltimer.h>

#include <osl/process.h>

#include <aquavclevents.hxx>

#include <premac.h>
#include <ApplicationServices/ApplicationServices.h>
#include <postmac.h>

using namespace std;

// -----------------------------------------------------------------------

// the AppEventList must be available before any SalData/SalInst/etc. objects are ready
typedef std::list<const ApplicationEvent*> AppEventList;
static AppEventList aAppEventList;

// -----------------------------------------------------------------------

FILE* SalData::s_pLog = NULL;

void AquaLog( const char* pFormat, ... )
{
    if( ! SalData::s_pLog )
        return;

    va_list ap;
    va_start( ap, pFormat );
    vfprintf( SalData::s_pLog, pFormat, ap );
    va_end( ap );
}

// =======================================================================

void SalAbort( const XubString& rErrorText )
{
    if( !rErrorText.Len() )
        AquaLog( "Application Error " );
    else
        AquaLog( "%s ",
            ByteString( rErrorText, gsl_getSystemTextEncoding() ).GetBuffer() );
    abort();
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
    static OUString aDesktopEnvironment(RTL_CONSTASCII_USTRINGPARAM( "MacOSX" ));
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

extern "C" {
#include <crt_externs.h>
}

// -----------------------------------------------------------------------

void InitSalMain()
{
    rtl_uString *urlWorkDir = NULL;
    rtl_uString *sysWorkDir = NULL;

    oslProcessError err1 = osl_getProcessWorkingDir(&urlWorkDir);

    if (err1 == osl_Process_E_None)
    {
        oslFileError err2 = osl_getSystemPathFromFileURL(urlWorkDir, &sysWorkDir);
        if (err2 == osl_File_E_None)
        {
            ByteString aPath( getenv( "PATH" ) );
            ByteString aResPath( getenv( "STAR_RESOURCEPATH" ) );
            ByteString aLibPath( getenv( "DYLD_LIBRARY_PATH" ) );
            ByteString aCmdPath( OUStringToOString(OUString(sysWorkDir), RTL_TEXTENCODING_UTF8).getStr() );
            ByteString aTmpPath;
            // Get absolute path of command's directory
            if ( aCmdPath.Len() ) {
                DirEntry aCmdDirEntry( aCmdPath );
                aCmdDirEntry.ToAbs();
                aCmdPath = ByteString( aCmdDirEntry.GetPath().GetFull(), RTL_TEXTENCODING_ASCII_US );
            }
            // Assign to PATH environment variable
            if ( aCmdPath.Len() )
            {
                aTmpPath = ByteString( "PATH=" );
                aTmpPath += aCmdPath;
                if ( aPath.Len() )
                    aTmpPath += ByteString( DirEntry::GetSearchDelimiter(), RTL_TEXTENCODING_ASCII_US );
                aTmpPath += aPath;
                putenv( (char*)aTmpPath.GetBuffer() );
            }
            // Assign to STAR_RESOURCEPATH environment variable
            if ( aCmdPath.Len() )
            {
                aTmpPath = ByteString( "STAR_RESOURCEPATH=" );
                aTmpPath += aCmdPath;
                if ( aResPath.Len() )
                    aTmpPath += ByteString( DirEntry::GetSearchDelimiter(), RTL_TEXTENCODING_ASCII_US );
                aTmpPath += aResPath;
                putenv( (char*)aTmpPath.GetBuffer() );
            }
            // Assign to DYLD_LIBRARY_PATH environment variable
            if ( aCmdPath.Len() )
            {
                aTmpPath = ByteString( "DYLD_LIBRARY_PATH=" );
                aTmpPath += aCmdPath;
                if ( aLibPath.Len() )
                    aTmpPath += ByteString( DirEntry::GetSearchDelimiter(), RTL_TEXTENCODING_ASCII_US );
                aTmpPath += aLibPath;
                putenv( (char*)aTmpPath.GetBuffer() );
            }
        }
    }
}

// -----------------------------------------------------------------------

void DeInitSalMain()
{
    // Release autorelease pool
    //VCLAutoreleasePool_Release( hMainAutoreleasePool );

}

// -----------------------------------------------------------------------

void SetFilterCallback( void* pCallback, void* pInst )
{
    // SalData *pSalData = GetSalData();
    //[fheckl]pSalData->mpFirstInstance->mpFilterCallback = pCallback;
    //[fheckl]pSalData->mpFirstInstance->mpFilterInst = pInst;
}

// =======================================================================

SalYieldMutex::SalYieldMutex()
{
    mnCount  = 0;
    mnThreadId  = 0;
}

void SalYieldMutex::acquire()
{
    OMutex::acquire();
    mnThreadId = NAMESPACE_VOS(OThread)::getCurrentIdentifier();
    mnCount++;
}

void SalYieldMutex::release()
{
    if ( mnThreadId == NAMESPACE_VOS(OThread)::getCurrentIdentifier() )
    {
        if ( mnCount == 1 )
            mnThreadId = 0;
        mnCount--;
    }
    OMutex::release();
}

sal_Bool SalYieldMutex::tryToAcquire()
{
    if ( OMutex::tryToAcquire() )
    {
        mnThreadId = NAMESPACE_VOS(OThread)::getCurrentIdentifier();
        mnCount++;
        return sal_True;
    }
    else
        return sal_False;
}

// -----------------------------------------------------------------------

// some convenience functions regarding the yield mutex, aka solar mutex

BOOL ImplSalYieldMutexTryToAcquire()
{
    AquaSalInstance* pInst = (AquaSalInstance*) GetSalData()->mpFirstInstance;
    if ( pInst )
        return pInst->mpSalYieldMutex->tryToAcquire();
    else
        return FALSE;
}

void ImplSalYieldMutexAcquire()
{
    AquaSalInstance* pInst = (AquaSalInstance*) GetSalData()->mpFirstInstance;
    if ( pInst )
        pInst->mpSalYieldMutex->acquire();
}

void ImplSalYieldMutexRelease()
{
    AquaSalInstance* pInst = (AquaSalInstance*) GetSalData()->mpFirstInstance;
    if ( pInst )
    {
        //GdiFlush();
        pInst->mpSalYieldMutex->release();
    }
}

// =======================================================================

SalInstance* CreateSalInstance()
{
    // FIXME: before integration: conditionalize debugging for DEBUG only
    const char* pLogEnv = getenv( "AQUA_LOG" );
    if( ! pLogEnv || ! strcmp( pLogEnv, "off" ) )
        SalData::s_pLog = NULL;
    else if( ! strcmp( pLogEnv, "stderr" ) )
        SalData::s_pLog = stderr;
    else
        SalData::s_pLog = fopen( pLogEnv, "w" );

    AquaSalInstance* pInst = new AquaSalInstance;

    EventLoopTimerUPP eventLoopTimer = NewEventLoopTimerUPP(AquaSalInstance::TimerEventHandler);
    InstallEventLoopTimer(GetMainEventLoop(), 1, 0, eventLoopTimer, pInst, &pInst->mEventLoopTimerRef);

    // init instance (only one instance in this version !!!)
    SalData* pSalData = GetSalData();
    pSalData->mpFirstInstance = pInst;
    ImplGetSVData()->maNWFData.mbNoFocusRects = true;
    ImplGetSVData()->maNWFData.mbNoBoldTabFocus = true;
    ImplGetSVData()->maNWFData.mbCenteredTabs = true;
    ImplGetSVData()->maNWFData.mbProgressNeedsErase = true;

    return pInst;
}

// -----------------------------------------------------------------------

void DestroySalInstance( SalInstance* pInst )
{
    delete pInst;

    if( SalData::s_pLog && SalData::s_pLog != stderr )
        fclose( SalData::s_pLog );
}

// -----------------------------------------------------------------------

AquaSalInstance::AquaSalInstance()
{
    mpFilterCallback = NULL;
    mpFilterInst    = NULL;
    mpSalYieldMutex = new SalYieldMutex;
    mEventLoopTimerRef = NULL;
    mbForceDispatchPaintEvents = false;
    mpSalYieldMutex->acquire();

    // In order to receive events, we put the application in foreground
    ProcessSerialNumber psn;
    GetCurrentProcess(&psn);
    TransformProcessType(&psn, kProcessTransformToForegroundApplication);
    SetFrontProcess(&psn);
}

// -----------------------------------------------------------------------

AquaSalInstance::~AquaSalInstance()
{
    mpSalYieldMutex->release();
    delete mpSalYieldMutex;
}

// -----------------------------------------------------------------------

void AquaSalInstance::TimerEventHandler(EventLoopTimerRef inTimer, void* pData)
{
  AquaSalInstance* pInst = reinterpret_cast<AquaSalInstance*>(pData);

  if (pInst->mbForceDispatchPaintEvents)
    {
      ULONG nCount = 0;

      // Release all locks so that we don't deadlock when we pull pending
      // events from the event queue
      nCount = pInst->ReleaseYieldMutex();

      EventRef theEvent;
      EventTargetRef theTarget = GetEventDispatcherTarget();

      if (ReceiveNextEvent(1, &cOOoSalTimerEvent, 0, true, &theEvent) == noErr)
    {
      SendEventToEventTarget(theEvent, theTarget);
      ReleaseEvent(theEvent);
    }

      // Reset all locks
      pInst->AcquireYieldMutex(nCount);

      SetEventLoopTimerNextFireTime(inTimer, 1); // restart timer
    }
}

// -----------------------------------------------------------------------

vos::IMutex* AquaSalInstance::GetYieldMutex()
{
    return mpSalYieldMutex;
}

// -----------------------------------------------------------------------

ULONG AquaSalInstance::ReleaseYieldMutex()
{
    SalYieldMutex* pYieldMutex = mpSalYieldMutex;
    if ( pYieldMutex->GetThreadId() ==
         NAMESPACE_VOS(OThread)::getCurrentIdentifier() )
    {
        ULONG nCount = pYieldMutex->GetAcquireCount();
        ULONG n = nCount;
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

void AquaSalInstance::AcquireYieldMutex( ULONG nCount )
{
    SalYieldMutex* pYieldMutex = mpSalYieldMutex;
    while ( nCount )
    {
        pYieldMutex->acquire();
        nCount--;
    }
}

// -----------------------------------------------------------------------

void AquaSalInstance::StartForceDispatchingPaintEvents()
{
  SetEventLoopTimerNextFireTime(mEventLoopTimerRef, 1);
  mbForceDispatchPaintEvents = true;
}

void AquaSalInstance::StopForceDispatchingPaintEvents()
{
  mbForceDispatchPaintEvents = false;
}

void AquaSalInstance::Yield( bool bWait, bool bHandleAllCurrentEvents )
{
    ULONG nCount = 0;
    // Release all locks so that we don't deadlock when we pull pending
    // events from the event queue
    nCount = ReleaseYieldMutex();

    EventTargetRef theTarget = GetEventDispatcherTarget();
    EventTimeout timeout = bWait ? kEventDurationForever : 0;

    EventRef aLastTimerEvent = NULL;
    for(;;)
    {
        EventRef theEvent;
        OSStatus eStatus = ReceiveNextEvent( 0, NULL, timeout, kEventRemoveFromQueue, &theEvent );
        if( eStatus != noErr)
            break;
        if( bHandleAllCurrentEvents
        &&  GetEventClass(theEvent) == cOOoSalUserEventClass
        &&  GetEventKind(theEvent) == cOOoSalEventTimer )
        {
            // ignore any timer event except the last one
            if( aLastTimerEvent )
                ReleaseEvent( aLastTimerEvent );
            aLastTimerEvent = theEvent;
        }
        else
        {
            //ImplSalYieldMutexAcquire();
            eStatus = SendEventToEventTarget(theEvent, theTarget);
            //ImplSalYieldMutexRelease();

            ReleaseEvent(theEvent);
        }

        if( !bHandleAllCurrentEvents )
            break;
    }

    // send only the latest timer event if any
    if( aLastTimerEvent )
    {
        SendEventToEventTarget( aLastTimerEvent, theTarget );
        ReleaseEvent( aLastTimerEvent );
    }

    // Reset all locks
    AcquireYieldMutex( nCount );

    // we get some apple events way too early
    // before the application is ready to handle them,
    // so their corresponding application events need to be delayed
    // now is a good time to handle at least one of them
    if( bWait && !aAppEventList.empty() )
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
}

// -----------------------------------------------------------------------

bool AquaSalInstance::AnyInput( USHORT nType )
{
  // xxx TODO
    return FALSE;
}

// -----------------------------------------------------------------------

SalFrame* AquaSalInstance::CreateChildFrame( SystemParentData* pSystemParentData, ULONG nSalFrameStyle )
{
    return NULL;
}

// -----------------------------------------------------------------------

SalFrame* AquaSalInstance::CreateFrame( SalFrame* pParent, ULONG nSalFrameStyle )
{
    SalFrame* pFrame = NULL;

    try
    {
        pFrame = new AquaSalFrame(pParent, nSalFrameStyle, this);
    }
    catch(runtime_error&)
    {
        // frame creation failed
    }
    return pFrame;
}

// -----------------------------------------------------------------------

void AquaSalInstance::DestroyFrame( SalFrame* pFrame )
{
    delete pFrame;
}

// -----------------------------------------------------------------------

SalObject* AquaSalInstance::CreateObject( SalFrame* pParent, SystemWindowData* /* pWindowData */ )
{
        // SystemWindowData is meaningless on Mac OS X
    AquaSalObject *pObject = NULL;

    if ( pParent )
    {
        pObject = new AquaSalObject();
        pObject->mpFrame = (AquaSalFrame*)pParent;
    }

    return pObject;
}

// -----------------------------------------------------------------------

void AquaSalInstance::DestroyObject( SalObject* pObject )
{
    delete ( pObject );
}

// -----------------------------------------------------------------------

SalPrinter* AquaSalInstance::CreatePrinter( SalInfoPrinter* pInfoPrinter )
{
    return new AquaSalPrinter( dynamic_cast<AquaSalInfoPrinter*>(pInfoPrinter) );
}

// -----------------------------------------------------------------------

void AquaSalInstance::DestroyPrinter( SalPrinter* pPrinter )
{
    delete pPrinter;
}

// -----------------------------------------------------------------------

void AquaSalInstance::GetPrinterQueueInfo( ImplPrnQueueList* pList )
{
    CFArrayRef  rPrinterList(0);
    if( PMServerCreatePrinterList( kPMServerLocal, &rPrinterList ) == noErr )
    {
        CFIndex nPrinters = CFArrayGetCount( rPrinterList );
        for( CFIndex n = 0; n < nPrinters; n++ )
        {
            PMPrinter aPrinter = reinterpret_cast<PMPrinter>(const_cast<void*>(CFArrayGetValueAtIndex( rPrinterList, n )));

            SalPrinterQueueInfo* pNewPrinter = new SalPrinterQueueInfo();
            pNewPrinter->maPrinterName  = GetOUString( PMPrinterGetName( aPrinter ) );
            pNewPrinter->mnStatus       = 0;
            pNewPrinter->mnJobs         = 0;
            // note CFStringRef is a const __CFString *
            // so void* is appropriate to hold it
            // however we need a little casting to convince the compiler
            CFStringRef rID = CFStringCreateCopy( NULL, PMPrinterGetID( aPrinter ) );
            pNewPrinter->mpSysData =
                const_cast<void*>(reinterpret_cast<const void*>( rID ));

            pList->Add( pNewPrinter );
            if( PMPrinterIsDefault( aPrinter ) )
                maDefaultPrinter = pNewPrinter->maPrinterName;
        }
        if ( rPrinterList )
            CFRelease( rPrinterList );
    }
}

// -----------------------------------------------------------------------

void AquaSalInstance::GetPrinterQueueState( SalPrinterQueueInfo* pInfo )
{
}

// -----------------------------------------------------------------------

void AquaSalInstance::DeletePrinterQueueInfo( SalPrinterQueueInfo* pInfo )
{
    if( pInfo->mpSysData )
        CFRelease( reinterpret_cast<CFStringRef>(pInfo->mpSysData) );
    delete pInfo;
}

// -----------------------------------------------------------------------

XubString AquaSalInstance::GetDefaultPrinter()
{
    if( ! maDefaultPrinter.getLength() )
    {
        CFArrayRef  rPrinterList(0);
        if( PMServerCreatePrinterList( kPMServerLocal, &rPrinterList ) == noErr )
        {
            CFIndex nPrinters = CFArrayGetCount( rPrinterList );
            for( CFIndex n = 0; n < nPrinters; n++ )
            {
                PMPrinter aPrinter = reinterpret_cast<PMPrinter>(const_cast<void*>(CFArrayGetValueAtIndex( rPrinterList, n )));
                if( PMPrinterIsDefault( aPrinter ) )
                    maDefaultPrinter = GetOUString( PMPrinterGetName( aPrinter ) );
            }
            if ( rPrinterList )
                CFRelease( rPrinterList );
        }
    }
    return maDefaultPrinter;
}

// -----------------------------------------------------------------------

SalInfoPrinter* AquaSalInstance::CreateInfoPrinter( SalPrinterQueueInfo* pQueueInfo,
                                                ImplJobSetup* pSetupData )
{
    SalInfoPrinter* pNewInfoPrinter = NULL;
    if( pQueueInfo )
    {
        pNewInfoPrinter = new AquaSalInfoPrinter( *pQueueInfo );
        if( pSetupData )
            pNewInfoPrinter->SetPrinterData( pSetupData );
    }

    return pNewInfoPrinter;
}

// -----------------------------------------------------------------------

void AquaSalInstance::DestroyInfoPrinter( SalInfoPrinter* pPrinter )
{
    delete pPrinter;
}

// -----------------------------------------------------------------------

SalSystem* AquaSalInstance::CreateSystem()
{
    return new AquaSalSystem();
}

// -----------------------------------------------------------------------

void AquaSalInstance::DestroySystem( SalSystem* pSystem )
{
    delete pSystem;
}

// -----------------------------------------------------------------------

void AquaSalInstance::SetEventCallback( void* pInstance, bool(*pCallback)(void*,void*,int) )
{
}

// -----------------------------------------------------------------------

void AquaSalInstance::SetErrorEventCallback( void* pInstance, bool(*pCallback)(void*,void*,int) )
{
}

// -----------------------------------------------------------------------

void* AquaSalInstance::GetConnectionIdentifier( ConnectionIdentifierType& rReturnedType, int& rReturnedBytes )
{
    rReturnedBytes  = 1;
    rReturnedType   = AsciiCString;
    return (void*)"";
}

// -----------------------------------------------------------------------

SalSound* AquaSalInstance::CreateSalSound()
{
    return new AquaSalSound();
}

// -----------------------------------------------------------------------

SalTimer* AquaSalInstance::CreateSalTimer()
{
    return new AquaSalTimer();
}

// -----------------------------------------------------------------------

SalOpenGL* AquaSalInstance::CreateSalOpenGL( SalGraphics* pGraphics )
{
    return new AquaSalOpenGL( pGraphics );
}

// -----------------------------------------------------------------------

SalSystem* AquaSalInstance::CreateSalSystem()
{
    return new AquaSalSystem();
}

// -----------------------------------------------------------------------

SalBitmap* AquaSalInstance::CreateSalBitmap()
{
    return new AquaSalBitmap();
}

// -----------------------------------------------------------------------

SalSession* AquaSalInstance::CreateSalSession()
{
    return NULL;
}

// -----------------------------------------------------------------------

class MacImeStatus : public SalI18NImeStatus
{
public:
    MacImeStatus() {}
    virtual ~MacImeStatus() {}

    // asks whether there is a status window available
    // to toggle into menubar
    virtual bool canToggle() { return false; }
    virtual void toggle() {}
};

// -----------------------------------------------------------------------

SalI18NImeStatus* AquaSalInstance::CreateI18NImeStatus()
{
    return new MacImeStatus();
}

// =======================================================================

static OSErr OpenOrPrintDoc( const AppleEvent* pAEvent, bool bPrint )
{
    // get the list of document URLs
    AEDescList aAEDocList;
    OSErr eErr = AEGetParamDesc( pAEvent, keyDirectObject, typeAEList, &aAEDocList );
    if( eErr != noErr )
        return errAEEventNotHandled;

    // prepare the URL-names string for the ApplicationEvent parameter
    long nFullLength = 0;
    std::vector<char> aNameBuffer;
    for( int i = 1;; ++i )
    {
        AEKeyword aAEKeyword;
        DescType aDescType;
        // get the URL's encoded name length
        long nNeedLength = 0;
        if( AEGetNthPtr( &aAEDocList, i, typeFileURL, &aAEKeyword, &aDescType,
            NULL, 0, &nNeedLength ) != noErr )
            break;
        // resize the name buffer for the new URL+delimiter
        if( nFullLength )
            aNameBuffer[ nFullLength++ ] = APPEVENT_PARAM_DELIMITER;
        aNameBuffer.resize( nFullLength + nNeedLength + 1 );
        // get the URL's encoded name data
        long nNewLength = 0;
        if( AEGetNthPtr( &aAEDocList, i, typeFileURL, &aAEKeyword, &aDescType,
            (void*)&aNameBuffer[nFullLength], nNeedLength, &nNewLength ) != noErr )
            break;
        nFullLength += nNewLength;
        aNameBuffer[ nFullLength ] = '\0';
    }

    // the AEdescriptor is no longer needed
    AEDisposeDesc( &aAEDocList );

    // convert the URL names to unicode
    // TODO: is the original encoding always UTF8?
    const String aUrlNames( (sal_Char*)&aNameBuffer[0], RTL_TEXTENCODING_UTF8 );
    // create and send the event to the application
    // send an open or print event to the application
    const String aEmptySender;
    const ApplicationAddress aEmptyAddr;
    const char* pEvtType = bPrint ? APPEVENT_PRINT_STRING : APPEVENT_OPEN_STRING;
    const ApplicationEvent* pAppEvent = new ApplicationEvent( aEmptySender, aEmptyAddr, pEvtType, aUrlNames );
    aAppEventList.push_back( pAppEvent );

    return noErr;
}

// -----------------------------------------------------------------------

OSErr HandleAEventOpenDoc( const AppleEvent* pAEvent,
    AppleEvent* /*pAEReply*/, long /*nHandlerRefConstant*/ )
{
    return OpenOrPrintDoc( pAEvent, false );
}

// -----------------------------------------------------------------------

OSErr HandleAEventPrintDoc( const AppleEvent* pAEvent,
    AppleEvent* /*pAEReply*/, long /*nHandlerRefConstant*/ )
{
    return OpenOrPrintDoc( pAEvent, true );
}

// -----------------------------------------------------------------------

OSErr HandleAEventOpenApp( const AppleEvent* /*pAEvent*/,
    AppleEvent* /*pAEReply*/, long /*nHandlerRefConstant*/ )
{
    const SalData* pSalData = GetSalData();
    if( !pSalData->maFrames.empty() )
        pSalData->maFrames.front()->CallCallback( SALEVENT_SHUTDOWN, NULL );
    return noErr;
}

// -----------------------------------------------------------------------

OSErr HandleAEventQuitApp( const AppleEvent* pAEvent,
    AppleEvent* /*pAEReply*/, long /*nHandlerRefConstant*/ )
{
    const SalData* pSalData = GetSalData();
    if( !pSalData->maFrames.empty() )
        pSalData->maFrames.front()->CallCallback( SALEVENT_SHUTDOWN, NULL );
    return noErr;
}

// -----------------------------------------------------------------------

extern void InstallAEHandlers()
{
    // check if it is possible to install AppleEvent handlers
    long nGestaltAttrs;
    OSStatus eStatus = Gestalt( gestaltAppleEventsAttr, &nGestaltAttrs );
    if( eStatus != noErr )
        return;
    if( ((nGestaltAttrs >> gestaltAppleEventsPresent) & 1) == 0 )
        return;

    // prepare to handle "open document" events
    AEEventHandlerUPP aAEHandler = NewAEEventHandlerUPP( HandleAEventOpenDoc );
    AEInstallEventHandler( kCoreEventClass, kAEOpenDocuments, aAEHandler, 0L, false );

    // prepare to handle "print document" events
    aAEHandler = NewAEEventHandlerUPP( HandleAEventPrintDoc );
    AEInstallEventHandler( kCoreEventClass, kAEPrintDocuments, aAEHandler, 0L, false );

    // prepare to handle "open application" events
    aAEHandler = NewAEEventHandlerUPP( HandleAEventOpenApp );
    AEInstallEventHandler( kCoreEventClass, kAEOpenApplication, aAEHandler, 0L, false );

    // prepare to handle "quit application" events
    aAEHandler = NewAEEventHandlerUPP( HandleAEventQuitApp );
    AEInstallEventHandler( kCoreEventClass, kAEQuitApplication, aAEHandler, 0L, false );

    // TODO: handle other events from the AppleEventManager too
}

//////////////////////////////////////////////////////////////
// TODO: are these debug-convenience functions still needed?
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

CFStringRef CreateCFString( const rtl::OUString& rStr )
{
    return CFStringCreateWithCharacters(kCFAllocatorDefault, rStr.getStr(), rStr.getLength() );
}

