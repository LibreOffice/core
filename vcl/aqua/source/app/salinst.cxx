/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: salinst.cxx,v $
 *
 *  $Revision: 1.41 $
 *
 *  last change: $Author: vg $ $Date: 2007-12-07 11:49:10 $
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

#include "tools/fsys.hxx"
#include "osl/process.h"
#include "rtl/ustrbuf.hxx"
#include "vcl/svapp.hxx"
#include "vcl/print.h"
#include "vcl/salimestatus.hxx"
#include "vcl/window.hxx"

#include "saldata.hxx"
#include "salinst.h"
#include "salframe.h"
#include "salobj.h"
#include "salsys.h"
#include "salvd.h"
#include "salsound.h"
#include "salbmp.h"
#include "salprn.h"
#include "salogl.h"
#include "saltimer.h"

#include "premac.h"
#include <ApplicationServices/ApplicationServices.h>
#include "postmac.h"

using namespace std;

extern BOOL ImplSVMain();

static BOOL* gpbInit = 0;
static NSMenu* pDockMenu = nil;
static bool bNoSVMain = true;
static bool bLeftMain = false;

// -----------------------------------------------------------------------

// the AppEventList must be available before any SalData/SalInst/etc. objects are ready
typedef std::list<const ApplicationEvent*> AppEventList;
static AppEventList aAppEventList;


NSMenu* AquaSalInstance::GetDynamicDockMenu()
{
    if( ! pDockMenu && ! bLeftMain )
        pDockMenu = [[NSMenu alloc] initWithTitle: @""];
    return pDockMenu;
}

@interface CocoaThreadEnabler : NSObject
{
}
-(void)enableCocoaThreads:(id)param;
@end

@implementation CocoaThreadEnabler
-(void)enableCocoaThreads:(id)param
{
    // do nothing, this is just to start an NSThread and therefore put
    // Cocoa into multithread mode
}
@end

// our very own application
@interface VCL_NSApplication : NSApplication
{
}
-(void)sendEvent:(NSEvent*)pEvent;
-(NSMenu*)applicationDockMenu:(NSApplication *)sender;
-(MacOSBOOL)application: (NSApplication*) app openFile: (NSString*)file;
-(void)application: (NSApplication*) app openFiles: (NSArray*)files;
-(MacOSBOOL)application: (NSApplication*) app printFile: (NSString*)file;
-(NSApplicationPrintReply)application: (NSApplication *) app printFiles:(NSArray *)files withSettings: (NSDictionary *)printSettings showPrintPanels:(BOOL)bShowPrintPanels;
-(NSApplicationTerminateReply)applicationShouldTerminate: (NSApplication *) app;
@end

@implementation VCL_NSApplication
-(void)sendEvent:(NSEvent*)pEvent
{
    NSEventType eType = [pEvent type];
    if( eType == NSApplicationDefined )
        GetSalData()->mpFirstInstance->handleAppDefinedEvent( pEvent );
    else if( eType == NSKeyDown && ([pEvent modifierFlags] & NSCommandKeyMask) != 0 )
    {
        NSWindow* pKeyWin = [NSApp keyWindow];
        if( pKeyWin && [pKeyWin isKindOfClass: [SalFrameWindow class]] )
        {
            AquaSalFrame* pFrame = [(SalFrameWindow*)pKeyWin getSalFrame];
            if( pFrame->mpParent && pFrame->mpParent->GetWindow()->IsInModalMode() )
            {
                // dispatch to view directly to avoid the key event being consumed by the menubar
                [[pKeyWin contentView] keyDown: pEvent];
                return;
            }
        }

    }
    [super sendEvent: pEvent];
}

-(NSMenu*)applicationDockMenu:(NSApplication *)sender
{
    return AquaSalInstance::GetDynamicDockMenu();
}
-(MacOSBOOL)application: (NSApplication*)app openFile: (NSString*)pFile
{
    const rtl::OUString aFile( GetOUString( pFile ) );
    const ApplicationEvent* pAppEvent = new ApplicationEvent( String(), ApplicationAddress(),
                                                APPEVENT_OPEN_STRING, aFile );
    aAppEventList.push_back( pAppEvent );
    return YES;
}

-(void)application: (NSApplication*) app openFiles: (NSArray*)files
{
    rtl::OUStringBuffer aFileList( 256 );

    NSEnumerator* it = [files objectEnumerator];
    NSString* pFile = nil;

    while( (pFile = [it nextObject]) != nil )
    {
        if( aFileList.getLength() > 0 )
            aFileList.append( sal_Unicode( APPEVENT_PARAM_DELIMITER ) );
        aFileList.append( GetOUString( pFile ) );
    }
    // we have no back channel here, we have to assume success, in which case
    // replyToOpenOrPrint does not need to be called according to documentation
    // [app replyToOpenOrPrint: NSApplicationDelegateReplySuccess];
    const ApplicationEvent* pAppEvent = new ApplicationEvent( String(), ApplicationAddress(),
                                                APPEVENT_OPEN_STRING, aFileList.makeStringAndClear() );
    aAppEventList.push_back( pAppEvent );
}

-(MacOSBOOL)application: (NSApplication*)app printFile: (NSString*)pFile
{
    const rtl::OUString aFile( GetOUString( pFile ) );
    const ApplicationEvent* pAppEvent = new ApplicationEvent( String(), ApplicationAddress(),
                                                APPEVENT_PRINT_STRING, aFile );
    aAppEventList.push_back( pAppEvent );
    return YES;
}
-(NSApplicationPrintReply)application: (NSApplication *) app printFiles:(NSArray *)files withSettings: (NSDictionary *)printSettings showPrintPanels:(BOOL)bShowPrintPanels
{
    // currently ignores print settings an bShowPrintPanels
    rtl::OUStringBuffer aFileList( 256 );

    NSEnumerator* it = [files objectEnumerator];
    NSString* pFile = nil;

    while( (pFile = [it nextObject]) != nil )
    {
        if( aFileList.getLength() > 0 )
            aFileList.append( sal_Unicode( APPEVENT_PARAM_DELIMITER ) );
        aFileList.append( GetOUString( pFile ) );
    }
    const ApplicationEvent* pAppEvent = new ApplicationEvent( String(), ApplicationAddress(),
                                                APPEVENT_PRINT_STRING, aFileList.makeStringAndClear() );
    aAppEventList.push_back( pAppEvent );
    // we have no back channel here, we have to assume success
    // correct handling would be NSPrintingReplyLater and then send [app replyToOpenOrPrint]
    return NSPrintingSuccess;
}

-(NSApplicationTerminateReply)applicationShouldTerminate: (NSApplication *) app
{
    const SalData* pSalData = GetSalData();
    if( !pSalData->maFrames.empty() )
    {
        // we forward the signal; alas we have no answer
        // we cancel the request; OOo will decide whether is shuts down or not
        pSalData->maFrames.front()->CallCallback( SALEVENT_SHUTDOWN, NULL );
        return NSTerminateCancel;
    }
    else
        // no frame left to talk to -> we really should terminate
        return NSTerminateNow;
}
@end

// initialize the cocoa VCL_NSApplication object
// returns an NSAutoreleasePool that must be released when the event loop begins
static NSAutoreleasePool* initNSApp()
{
    // create our cocoa NSApplication
    [VCL_NSApplication sharedApplication];

    // put cocoa into multithreaded mode
    [NSThread detachNewThreadSelector:@selector(enableCocoaThreads:) toTarget:[[CocoaThreadEnabler alloc] init] withObject:nil];

    NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];

    // activate our delegate methods
    [NSApp setDelegate: NSApp];

    return pool;
}

BOOL ImplSVMainHook( BOOL * pbInit )
{
    gpbInit = pbInit;

    bNoSVMain = false;
    NSAutoreleasePool* pool = initNSApp();

    NSPoint aPt = { 0, 0 };
    NSEvent* pEvent = [NSEvent otherEventWithType: NSApplicationDefined
                               location: aPt
                               modifierFlags: 0
                               timestamp: 0
                               windowNumber: 0
                               context: nil
                               subtype: AquaSalInstance::AppExecuteSVMain
                               data1: 0
                               data2: 0 ];
    if( pEvent )
    {
        [NSApp postEvent: pEvent atStart: NO];
        [pool release];

        rtl::OUString aExeURL, aExe;
        osl_getExecutableFile( &aExeURL.pData );
        osl_getSystemPathFromFileURL( aExeURL.pData, &aExe.pData );
        rtl::OString aByteExe( rtl::OUStringToOString( aExe, osl_getThreadTextEncoding() ) );

        const char* pArgv[] = { aByteExe.getStr(), NULL };
        NSApplicationMain( 1, pArgv );
    }
    else
    {
        [pool release];
        DBG_ERROR( "NSApplication initialization could not be done" );
    }

    return TRUE;   // indicate that ImplSVMainHook is implemented
}

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
        pInst->mpSalYieldMutex->release();
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

    // this is the case for not using SVMain
    // not so good
    if( bNoSVMain )
    {
        NSAutoreleasePool* pool = initNSApp();
        [pool release];
    }

    SalData* pSalData = GetSalData();
    DBG_ASSERT( pSalData->mpFirstInstance == NULL, "more than one instance created" );
    AquaSalInstance* pInst = new AquaSalInstance;

    // init instance (only one instance in this version !!!)
    pSalData->mpFirstInstance = pInst;
    // this one is for outside AquaSalInstance::Yield
    pInst->mpAutoreleasePool = [[NSAutoreleasePool alloc] init];
    // no focus rects on NWF aqua
    ImplGetSVData()->maNWFData.mbNoFocusRects = true;
    ImplGetSVData()->maNWFData.mbNoBoldTabFocus = true;
    ImplGetSVData()->maNWFData.mbCenteredTabs = true;
    ImplGetSVData()->maNWFData.mbProgressNeedsErase = true;
    ImplGetSVData()->maGDIData.mbPrinterPullModel = true;

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
    mpSalYieldMutex = new SalYieldMutex;
    mpSalYieldMutex->acquire();
    maMainThread = vos::OThread::getCurrentIdentifier();
    mnMainThreadLevel = 0;
    mpAutoreleasePool = nil;
    maUserEventListMutex = osl_createMutex();
}

// -----------------------------------------------------------------------

AquaSalInstance::~AquaSalInstance()
{
    mpSalYieldMutex->release();
    delete mpSalYieldMutex;
    osl_destroyMutex( maUserEventListMutex );
}

// -----------------------------------------------------------------------

void AquaSalInstance::PostUserEvent( AquaSalFrame* pFrame, USHORT nType, void* pData )
{
    osl_acquireMutex( maUserEventListMutex );
    maUserEvents.push_back( SalUserEvent( pFrame, pData, nType ) );
    osl_releaseMutex( maUserEventListMutex );
    // wakeup :Yield
    NSPoint aPt = { 0, 0 };
    NSEvent* pEvent = [NSEvent otherEventWithType: NSApplicationDefined
                               location: aPt
                               modifierFlags: 0
                               timestamp: 0
                               windowNumber: 0
                               context: nil
                               subtype: AquaSalInstance::YieldWakeupEvent
                               data1: 0
                               data2: 0 ];
    if( pEvent )
        [NSApp postEvent: pEvent atStart: NO];

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

bool AquaSalInstance::isNSAppThread() const
{
    return vos::OThread::getCurrentIdentifier() == maMainThread;
}

// -----------------------------------------------------------------------

void AquaSalInstance::handleAppDefinedEvent( NSEvent* pEvent )
{
    switch( [pEvent subtype] )
    {
    case AppStartTimerEvent:
        AquaSalTimer::handleStartTimerEvent( pEvent );
        break;
    case AppEndLoopEvent:
        [NSApp stop: NSApp];
        break;
    case AppExecuteSVMain:
    {
        BOOL bResult = ImplSVMain();
        if( gpbInit )
            *gpbInit = bResult;
        [NSApp stop: NSApp];
        bLeftMain = true;
        if( pDockMenu )
        {
            [pDockMenu release];
            pDockMenu = nil;
        }
    }
    break;
    case YieldWakeupEvent:
        // do nothing, fall out of Yield
    break;
    default:
        DBG_ERROR( "unhandled NSApplicationDefined event" );
        break;
    };
}

// -----------------------------------------------------------------------

void AquaSalInstance::Yield( bool bWait, bool bHandleAllCurrentEvents )
{
    // Release all locks so that we don't deadlock when we pull pending
    // events from the event queue
    bool bDispatchUser = true;
    while( bDispatchUser )
    {
        ULONG nCount = ReleaseYieldMutex();

        // get one user event
        osl_acquireMutex( maUserEventListMutex );
        SalUserEvent aEvent( NULL, 0, NULL );
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
        if( aEvent.mpFrame && AquaSalFrame::isAlive( aEvent.mpFrame ) )
        {
            aEvent.mpFrame->CallCallback( aEvent.mnType, aEvent.mpData );
            // return if only one event is asked for
            if( ! bHandleAllCurrentEvents )
                return;
        }
    }

    // handle cocoa event queue
    // cocoa events mye be only handled in the thread the NSApp was created
    if( isNSAppThread() )
    {
        mnMainThreadLevel++;

        // handle available events
        NSEvent* pEvent = nil;
        bool bHadEvent = false;
        do
        {
            ULONG nCount = ReleaseYieldMutex();

            pEvent = [NSApp nextEventMatchingMask: NSAnyEventMask untilDate: nil
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
            ULONG nCount = ReleaseYieldMutex();

            pEvent = [NSApp nextEventMatchingMask: NSAnyEventMask untilDate: [NSDate distantFuture]
                            inMode: NSDefaultRunLoopMode dequeue: YES];
            if( pEvent )
                [NSApp sendEvent: pEvent];
            [NSApp updateWindows];

            AcquireYieldMutex( nCount );
        }

        mnMainThreadLevel--;

        if( mnMainThreadLevel == 0 )
        {
            // flush the autorelease pool
            [mpAutoreleasePool release];
            mpAutoreleasePool = [[NSAutoreleasePool alloc] init];
        }
    }

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

    pFrame = new AquaSalFrame( pParent, nSalFrameStyle );
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
        pObject = new AquaSalObject( static_cast<AquaSalFrame*>(pParent) );

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
    NSArray* pNames = [NSPrinter printerNames];
    NSArray* pTypes = [NSPrinter printerTypes];
    unsigned int nNameCount = pNames ? [pNames count] : 0;
    unsigned int nTypeCount = pTypes ? [pTypes count] : 0;
    DBG_ASSERT( nTypeCount == nNameCount, "type count not equal to printer count" );
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
            pInfo->mnStatus         = 0;
            pInfo->mnJobs           = 0;
            pInfo->mpSysData        = NULL;

            pList->Add( pInfo );
        }
    }
}

// -----------------------------------------------------------------------

void AquaSalInstance::GetPrinterQueueState( SalPrinterQueueInfo* pInfo )
{
}

// -----------------------------------------------------------------------

void AquaSalInstance::DeletePrinterQueueInfo( SalPrinterQueueInfo* pInfo )
{
    delete pInfo;
}

// -----------------------------------------------------------------------

XubString AquaSalInstance::GetDefaultPrinter()
{
    if( ! maDefaultPrinter.getLength() )
    {
        NSPrintInfo* pPI = [NSPrintInfo sharedPrintInfo];
        DBG_ASSERT( pPI, "no print info" );
        if( pPI )
        {
            NSPrinter* pPr = [pPI printer];
            DBG_ASSERT( pPr, "no printer in default info" );
            if( pPr )
            {
                NSString* pDefName = [pPr name];
                DBG_ASSERT( pDefName, "printer has no name" );
                maDefaultPrinter = GetOUString( pDefName );
            }
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
