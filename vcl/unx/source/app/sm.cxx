/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: sm.cxx,v $
 * $Revision: 1.32 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_vcl.hxx"
#include <string.h>
#include <unistd.h>
#include <sys/poll.h>
#include <fcntl.h>

// [ed] 6/15/02 Use the poll replacement on OS X
// [ericb] 5/7/05 mismatch in Mac OSX 10.4, test is necessary
#if defined (MACOSX) && (BUILD_OS_MAJOR==10) && (BUILD_OS_MINOR<4)
#include <poll.h>
#else
#include <sys/poll.h>
#endif
#include <stdio.h>

#include <osl/process.h>
#include <osl/security.h>

#include <prex.h>
#include <X11/Xatom.h>
#include <postx.h>
#include <sm.hxx>
#include <saldata.hxx>
#include <saldisp.hxx>
#include <salframe.h>
#include <vcl/svapp.hxx>
#include <salinst.h>

#include <osl/conditn.h>

#define USE_SM_EXTENSION

#if OSL_DEBUG_LEVEL > 1
#include <cstdarg>
static bool bFirstAssert = true;
#endif

#if OSL_DEBUG_LEVEL > 1
inline void SMprintf( const char* pFormat, ... )
#else
inline void SMprintf( const char*, ... )
#endif
{
#if OSL_DEBUG_LEVEL > 1
    FILE* fp = fopen( "/tmp/sessionlog.txt", bFirstAssert ? "w" : "a" );
    if(!fp) return;
    bFirstAssert = false;
    va_list ap;
    va_start( ap, pFormat );
    vfprintf( fp, pFormat, ap );
    fclose( fp );
    va_end( ap );
#endif
};

static IceSalSession* pOneInstance = NULL;

SalSession* X11SalInstance::CreateSalSession()
{
    if( ! pOneInstance )
        pOneInstance = new IceSalSession();
    return pOneInstance;
}

/*
 *  class IceSalSession
 */

static X11SalFrame* pOldStyleSaveFrame = NULL;

IceSalSession::IceSalSession()
{
}

IceSalSession::~IceSalSession()
{
    if( pOneInstance == this )
        pOneInstance = NULL;
}

void IceSalSession::queryInteraction()
{
    if( ! SessionManagerClient::queryInteraction() )
    {
        SalSessionInteractionEvent aEvent( false );
        CallCallback( &aEvent );
    }
}

void IceSalSession::interactionDone()
{
    SessionManagerClient::interactionDone();
}

void IceSalSession::saveDone()
{
    SessionManagerClient::saveDone();
    if( pOldStyleSaveFrame )
    {
        // note: does nothing if not running in generic plugin
        X11SalFrame::SaveYourselfDone( pOldStyleSaveFrame );
    }
}

bool IceSalSession::cancelShutdown()
{
    return false;
}

void IceSalSession::handleOldX11SaveYourself( SalFrame* pFrame )
{
    // do this only once
    if( ! pOldStyleSaveFrame )
    {
        pOldStyleSaveFrame = static_cast<X11SalFrame*>(pFrame);
        if( pOneInstance )
        {
            SalSessionSaveRequestEvent aEvent( true, false );
            pOneInstance->CallCallback( &aEvent );
        }
    }
}

extern "C" void SAL_CALL ICEConnectionWorker( void* );

class ICEConnectionObserver
{
    friend void SAL_CALL ICEConnectionWorker(void*);
    static BOOL bIsWatching;
    static void ICEWatchProc( IceConn connection, IcePointer client_data,
                              Bool opening, IcePointer* watch_data );

    static struct pollfd* pFilehandles;
    static IceConn* pConnections;
    static int nConnections;
    static int nWakeupFiles[2];
    static oslMutex ICEMutex;
    static oslThread ICEThread;
public:

    static void activate();
    static void deactivate();
    static void lock();
    static void unlock();
    static void wakeup();
};


SmcConn             SessionManagerClient::aSmcConnection            = NULL;
ByteString          SessionManagerClient::aClientID;
BOOL                ICEConnectionObserver::bIsWatching              = FALSE;
struct pollfd*  ICEConnectionObserver::pFilehandles             = NULL;
IceConn*            ICEConnectionObserver::pConnections             = NULL;
int                 ICEConnectionObserver::nConnections             = 0;
oslMutex            ICEConnectionObserver::ICEMutex                 = NULL;
oslThread           ICEConnectionObserver::ICEThread                = NULL;
int                 ICEConnectionObserver::nWakeupFiles[2]          = { 0, 0 };

// HACK
bool SessionManagerClient::bDocSaveDone = false;


static SmProp*  pSmProps = NULL;
static SmProp** ppSmProps = NULL;
static int      nSmProps = 0;


static void BuildSmPropertyList()
{
    if( ! pSmProps )
    {
        ByteString aExec( SessionManagerClient::getExecName(), osl_getThreadTextEncoding() );

        nSmProps = 4;
        pSmProps = new SmProp[ nSmProps ];

        pSmProps[ 0 ].name      = const_cast<char*>(SmCloneCommand);
        pSmProps[ 0 ].type      = const_cast<char*>(SmLISTofARRAY8);
        pSmProps[ 0 ].num_vals  = 1;
        pSmProps[ 0 ].vals      = new SmPropValue;
        pSmProps[ 0 ].vals->length  = aExec.Len()+1;
        pSmProps[ 0 ].vals->value   = strdup( aExec.GetBuffer() );

        pSmProps[ 1 ].name      = const_cast<char*>(SmProgram);
        pSmProps[ 1 ].type      = const_cast<char*>(SmARRAY8);
        pSmProps[ 1 ].num_vals  = 1;
        pSmProps[ 1 ].vals      = new SmPropValue;
        pSmProps[ 1 ].vals->length  = aExec.Len()+1;
        pSmProps[ 1 ].vals->value   = strdup( aExec.GetBuffer() );

        pSmProps[ 2 ].name      = const_cast<char*>(SmRestartCommand);
        pSmProps[ 2 ].type      = const_cast<char*>(SmLISTofARRAY8);
        pSmProps[ 2 ].num_vals  = 3;
        pSmProps[ 2 ].vals      = new SmPropValue[3];
        pSmProps[ 2 ].vals[0].length    = aExec.Len()+1;
        pSmProps[ 2 ].vals[0].value = strdup( aExec.GetBuffer() );
            ByteString aRestartOption( "-session=" );
        aRestartOption.Append( SessionManagerClient::getSessionID() );
        pSmProps[ 2 ].vals[1].length    = aRestartOption.Len()+1;
        pSmProps[ 2 ].vals[1].value = strdup( aRestartOption.GetBuffer() );
            ByteString aRestartOptionNoLogo( "-nologo" );
        pSmProps[ 2 ].vals[2].length    = aRestartOptionNoLogo.Len()+1;
        pSmProps[ 2 ].vals[2].value = strdup( aRestartOptionNoLogo.GetBuffer() );

        rtl::OUString aUserName;
        rtl::OString aUser;
        oslSecurity aSec = osl_getCurrentSecurity();
        if( aSec )
        {
            osl_getUserName( aSec, &aUserName.pData );
            aUser = rtl::OUStringToOString( aUserName, osl_getThreadTextEncoding() );
            osl_freeSecurityHandle( aSec );
        }

        pSmProps[ 3 ].name      = const_cast<char*>(SmUserID);
        pSmProps[ 3 ].type      = const_cast<char*>(SmARRAY8);
        pSmProps[ 3 ].num_vals  = 1;
        pSmProps[ 3 ].vals      = new SmPropValue;
        pSmProps[ 3 ].vals->value   = strdup( aUser.getStr() );
        pSmProps[ 3 ].vals->length  = strlen( (char *)pSmProps[ 3 ].vals->value )+1;

        ppSmProps = new SmProp*[ nSmProps ];
        for( int i = 0; i < nSmProps; i++ )
            ppSmProps[ i ] = &pSmProps[i];
    }
}

bool SessionManagerClient::checkDocumentsSaved()
{
    return bDocSaveDone;
}

IMPL_STATIC_LINK( SessionManagerClient, SaveYourselfHdl, void*, EMPTYARG )
{
    SMprintf( "posting save documents event shutdown = %s\n", (pThis!=0) ? "true" : "false" );
    if( pOneInstance )
    {
        SalSessionSaveRequestEvent aEvent( pThis != 0, false );
        pOneInstance->CallCallback( &aEvent );
    }
    else
        saveDone();

    return 0;
}

IMPL_STATIC_LINK_NOINSTANCE( SessionManagerClient, InteractionHdl, void*, EMPTYARG )
{
    SMprintf( "interaction link\n" );
    if( pOneInstance )
    {
        SalSessionInteractionEvent aEvent( true );
        pOneInstance->CallCallback( &aEvent );
    }

    return 0;
}

IMPL_STATIC_LINK_NOINSTANCE( SessionManagerClient, ShutDownCancelHdl, void*, EMPTYARG )
{
    SMprintf( "shutdown cancel\n" );
    if( pOneInstance )
    {
        SalSessionShutdownCancelEvent aEvent;
        pOneInstance->CallCallback( &aEvent );
    }

    return 0;
}

void SessionManagerClient::SaveYourselfProc(
    SmcConn,
    SmPointer,
    int save_type,
    Bool shutdown,
    int interact_style,
    Bool
    )
{
    SMprintf( "Session: save yourself, save_type = %s, shutdown = %s, interact_style = %s, fast = %s\n",
              save_type == SmSaveLocal ? "SmcSaveLocal" :
              ( save_type == SmSaveGlobal ? "SmcSaveGlobal" :
                ( save_type == SmSaveBoth ? "SmcSaveBoth" : "<unknown>" ) ),
              shutdown ? "true" : "false",
              interact_style == SmInteractStyleNone ? "SmInteractStyleNone" :
              ( interact_style == SmInteractStyleErrors ? "SmInteractStyleErrors" :
                ( interact_style == SmInteractStyleAny ? "SmInteractStyleAny" : "<unknown>" ) ),
              false ? "true" : "false"
              );
    BuildSmPropertyList();
#ifdef USE_SM_EXTENSION
    bDocSaveDone = false;
    /* #i49875# some session managers send a "die" message if the
     * saveDone does not come early enough for their convenience
     * this can occasionally happen on startup, especially the first
     * startup. So shortcut the "not shutting down" case since the
     * upper layers are currently not interested in that event anyway.
     */
    if( ! shutdown )
    {
        SessionManagerClient::saveDone();
        return;
    }
    Application::PostUserEvent( STATIC_LINK( (void*)(shutdown ? 0xffffffff : 0x0), SessionManagerClient, SaveYourselfHdl ) );
    SMprintf( "waiting for save yourself event to be processed\n" );
#endif
}

IMPL_STATIC_LINK_NOINSTANCE( SessionManagerClient, ShutDownHdl, void*, EMPTYARG )
{
    const std::list< SalFrame* >& rFrames = GetX11SalData()->GetDisplay()->getFrames();
    SMprintf( rFrames.begin() != rFrames.end() ? "shutdown on first frame\n" : "shutdown event but no frame\n" );
    if( rFrames.begin() != rFrames.end() )
        rFrames.front()->CallCallback( SALEVENT_SHUTDOWN, 0 );
    return 0;
}

void SessionManagerClient::DieProc(
    SmcConn connection,
    SmPointer
    )
{
    SMprintf( "Session: die\n" );
    if( connection == aSmcConnection )
    {
        Application::PostUserEvent( STATIC_LINK( NULL, SessionManagerClient, ShutDownHdl ) );
        SMprintf( "waiting for shutdown event to be processed\n" );
    }
}

void SessionManagerClient::SaveCompleteProc(
    SmcConn,
    SmPointer
    )
{
    SMprintf( "Session: save complete\n" );
}

void SessionManagerClient::ShutdownCanceledProc(
    SmcConn connection,
    SmPointer )
{
    SMprintf( "Session: shutdown canceled\n" );
    if( connection == aSmcConnection )
        Application::PostUserEvent( STATIC_LINK( NULL, SessionManagerClient, ShutDownCancelHdl ) );
}

void SessionManagerClient::InteractProc(
                                        SmcConn connection,
                                        SmPointer )
{
    SMprintf( "Session: interaction request completed\n" );
    if( connection == aSmcConnection )
        Application::PostUserEvent( STATIC_LINK( NULL, SessionManagerClient, InteractionHdl ) );
}

void SessionManagerClient::saveDone()
{
    if( aSmcConnection )
    {
        ICEConnectionObserver::lock();
        SmcSetProperties( aSmcConnection, nSmProps, ppSmProps );
        SmcSaveYourselfDone( aSmcConnection, True );
        SMprintf( "sent SaveYourselfDone\n" );
        bDocSaveDone = true;
        ICEConnectionObserver::unlock();
    }
}


void SessionManagerClient::open()
{
    static SmcCallbacks aCallbacks;

#ifdef USE_SM_EXTENSION
    // this is the way Xt does it, so we can too
    if( ! aSmcConnection && getenv( "SESSION_MANAGER" ) )
    {
        char aErrBuf[1024];
        ICEConnectionObserver::activate();
        ICEConnectionObserver::lock();

        char* pClientID = NULL;
        const ByteString& rPrevId( getPreviousSessionID() );

        aCallbacks.save_yourself.callback           = SaveYourselfProc;
        aCallbacks.save_yourself.client_data        = NULL;
        aCallbacks.die.callback                     = DieProc;
        aCallbacks.die.client_data                  = NULL;
        aCallbacks.save_complete.callback           = SaveCompleteProc;
        aCallbacks.save_complete.client_data        = NULL;
        aCallbacks.shutdown_cancelled.callback      = ShutdownCanceledProc;
        aCallbacks.shutdown_cancelled.client_data   = NULL;
        aSmcConnection = SmcOpenConnection( NULL,
                                            NULL,
                                            SmProtoMajor,
                                            SmProtoMinor,
                                            SmcSaveYourselfProcMask         |
                                            SmcDieProcMask                  |
                                            SmcSaveCompleteProcMask         |
                                            SmcShutdownCancelledProcMask    ,
                                            &aCallbacks,
                                            rPrevId.Len() ? const_cast<char*>(rPrevId.GetBuffer()) : NULL,
                                            &pClientID,
                                            sizeof( aErrBuf ),
                                            aErrBuf );
        if( ! aSmcConnection )
            SMprintf( "SmcOpenConnection failed: %s\n", aErrBuf );
        else
            SMprintf( "SmcOpenConnection succeeded, client ID is \"%s\"\n", pClientID );
        aClientID = ByteString( pClientID );
        free( pClientID );
        pClientID = NULL;
        ICEConnectionObserver::unlock();

        SalDisplay* pDisp = GetX11SalData()->GetDisplay();
        if( pDisp->GetDrawable( pDisp->GetDefaultScreenNumber() ) && aClientID.Len() )
        {
            XChangeProperty( pDisp->GetDisplay(),
                             pDisp->GetDrawable( pDisp->GetDefaultScreenNumber() ),
                             XInternAtom( pDisp->GetDisplay(), "SM_CLIENT_ID", False ),
                             XA_STRING,
                             8,
                             PropModeReplace,
                             (unsigned char*)aClientID.GetBuffer(),
                             aClientID.Len()
                             );
        }
    }
    else if( ! aSmcConnection )
        SMprintf( "no SESSION_MANAGER\n" );
#endif
}

const ByteString& SessionManagerClient::getSessionID()
{
    return aClientID;
}

void SessionManagerClient::close()
{
    if( aSmcConnection )
    {
#ifdef USE_SM_EXTENSION
        ICEConnectionObserver::lock();
        SMprintf( "attempting SmcCloseConnection\n" );
        SmcCloseConnection( aSmcConnection, 0, NULL );
        SMprintf( "SmcConnection closed\n" );
        ICEConnectionObserver::unlock();
        ICEConnectionObserver::deactivate();
#endif
        aSmcConnection = NULL;
    }
}

bool SessionManagerClient::queryInteraction()
{
    bool bRet = false;
    if( aSmcConnection )
    {
        ICEConnectionObserver::lock();
        if( SmcInteractRequest( aSmcConnection, SmDialogNormal, InteractProc, NULL ) )
            bRet = true;
        ICEConnectionObserver::unlock();
    }
    return bRet;
}

void SessionManagerClient::interactionDone()
{
    if( aSmcConnection )
    {
        ICEConnectionObserver::lock();
        SmcInteractDone( aSmcConnection, False );
        ICEConnectionObserver::unlock();
    }
}


String SessionManagerClient::getExecName()
{
    rtl::OUString aExec, aSysExec;
    osl_getExecutableFile( &aExec.pData );
    osl_getSystemPathFromFileURL( aExec.pData, &aSysExec.pData );

    int nPos = aSysExec.indexOf( rtl::OUString::createFromAscii( ".bin" ) );
    if( nPos != -1 )
        aSysExec = aSysExec.copy( 0, nPos );
    return aSysExec;
}


const ByteString& SessionManagerClient::getPreviousSessionID()
{
    static ByteString aPrevId;

    int nCommands = osl_getCommandArgCount();
    for( int i = 0; i < nCommands; i++ )
    {
        ::rtl::OUString aArg;
        osl_getCommandArg( i, &aArg.pData );
        if( aArg.compareToAscii( "-session=", 9 ) == 0 )
        {
            aPrevId = ByteString( ::rtl::OUStringToOString( aArg.copy( 9 ), osl_getThreadTextEncoding() ) );
            break;
        }
    }
    SMprintf( "previous ID = \"%s\"\n", aPrevId.GetBuffer() );
    return aPrevId;
}

void ICEConnectionObserver::lock()
{
    osl_acquireMutex( ICEMutex );
}

void ICEConnectionObserver::unlock()
{
    osl_releaseMutex( ICEMutex );
}

void ICEConnectionObserver::activate()
{
    if( ! bIsWatching )
    {
        nWakeupFiles[0] = nWakeupFiles[1] = 0;
        ICEMutex = osl_createMutex();
        bIsWatching = TRUE;
#ifdef USE_SM_EXTENSION
        IceAddConnectionWatch( ICEWatchProc, NULL );
#endif
    }
}

void ICEConnectionObserver::deactivate()
{
    if( bIsWatching )
    {
        lock();
        bIsWatching = FALSE;
#ifdef USE_SM_EXTENSION
        IceRemoveConnectionWatch( ICEWatchProc, NULL );
#endif
        nConnections = 0;
        if( ICEThread )
        {
            osl_terminateThread( ICEThread );
            wakeup();
        }
        unlock();
        if( ICEThread )
        {
            osl_joinWithThread( ICEThread );
            osl_destroyThread( ICEThread );
            close( nWakeupFiles[1] );
            close( nWakeupFiles[0] );
            ICEThread = NULL;
        }
        osl_destroyMutex( ICEMutex );
        ICEMutex = NULL;
    }
}

void ICEConnectionObserver::wakeup()
{
    char cChar = 'w';
    write( nWakeupFiles[1], &cChar, 1 );
}

void ICEConnectionWorker( void* )
{
#ifdef USE_SM_EXTENSION
    while( osl_scheduleThread(ICEConnectionObserver::ICEThread) && ICEConnectionObserver::nConnections )
    {
        ICEConnectionObserver::lock();
        int nConnectionsBefore = ICEConnectionObserver::nConnections;
        int nBytes = sizeof( struct pollfd )*(nConnectionsBefore+1);
        struct pollfd* pLocalFD = (struct pollfd*)rtl_allocateMemory( nBytes );
        rtl_copyMemory( pLocalFD, ICEConnectionObserver::pFilehandles, nBytes );
        ICEConnectionObserver::unlock();

        int nRet = poll( pLocalFD,nConnectionsBefore+1,-1 );
        bool bWakeup = (pLocalFD[0].revents & POLLIN);
        rtl_freeMemory( pLocalFD );

        if( nRet < 1 )
            continue;

        // clear wakeup pipe
        if( bWakeup )
        {
            char buf[4];
            while( read( ICEConnectionObserver::nWakeupFiles[0], buf, sizeof( buf ) ) > 0 )
                ;
            SMprintf( "file handles active in wakeup: %d\n", nRet );
            if( nRet == 1 )
                continue;
        }

        // check fd's after we obtained the lock
        ICEConnectionObserver::lock();
        if( ICEConnectionObserver::nConnections > 0 && ICEConnectionObserver::nConnections == nConnectionsBefore )
        {
            nRet = poll( ICEConnectionObserver::pFilehandles+1, ICEConnectionObserver::nConnections, 0 );
            if( nRet > 0 )
            {
                SMprintf( "IceProcessMessages\n" );
                Bool bReply;
                for( int i = 0; i < ICEConnectionObserver::nConnections; i++ )
                    if( ICEConnectionObserver::pFilehandles[i+1].revents & POLLIN )
                        IceProcessMessages( ICEConnectionObserver::pConnections[i], NULL, &bReply );
            }
        }
        ICEConnectionObserver::unlock();
    }
#endif
    SMprintf( "shutting donw ICE dispatch thread\n" );
}

void ICEConnectionObserver::ICEWatchProc(
    IceConn connection,
    IcePointer,
    Bool opening,
    IcePointer*
    )
{
    // note: this is a callback function for ICE
    // this implicitly means that a call into ICE lib is calling this
    // so the ICEMutex MUST already be locked by the caller

#ifdef USE_SM_EXTENSION
    if( opening )
    {
        int fd = IceConnectionNumber( connection );
        nConnections++;
        pConnections = (IceConn*)rtl_reallocateMemory( pConnections, sizeof( IceConn )*nConnections );
        pFilehandles = (struct pollfd*)rtl_reallocateMemory( pFilehandles, sizeof( struct pollfd )*(nConnections+1) );
        pConnections[ nConnections-1 ]      = connection;
        pFilehandles[ nConnections ].fd     = fd;
        pFilehandles[ nConnections ].events = POLLIN;
        if( nConnections == 1 )
        {
            if( ! pipe( nWakeupFiles ) )
            {
                int flags;
                pFilehandles[0].fd      = nWakeupFiles[0];
                pFilehandles[0].events  = POLLIN;
                // set close-on-exec and nonblock descriptor flag.
                if ((flags = fcntl (nWakeupFiles[0], F_GETFD)) != -1)
                {
                    flags |= FD_CLOEXEC;
                    fcntl (nWakeupFiles[0], F_SETFD, flags);
                }
                if ((flags = fcntl (nWakeupFiles[0], F_GETFL)) != -1)
                {
                    flags |= O_NONBLOCK;
                    fcntl (nWakeupFiles[0], F_SETFL, flags);
                }
                // set close-on-exec and nonblock descriptor flag.
                if ((flags = fcntl (nWakeupFiles[1], F_GETFD)) != -1)
                {
                    flags |= FD_CLOEXEC;
                    fcntl (nWakeupFiles[1], F_SETFD, flags);
                }
                if ((flags = fcntl (nWakeupFiles[1], F_GETFL)) != -1)
                {
                    flags |= O_NONBLOCK;
                    fcntl (nWakeupFiles[1], F_SETFL, flags);
                }
                ICEThread = osl_createSuspendedThread( ICEConnectionWorker, NULL );
                osl_resumeThread( ICEThread );
            }
        }
    }
    else
    {
        for( int i = 0; i < nConnections; i++ )
        {
            if( pConnections[i] == connection )
            {
                if( i < nConnections-1 )
                {
                    rtl_moveMemory( pConnections+i, pConnections+i+1, sizeof( IceConn )*(nConnections-i-1) );
                    rtl_moveMemory( pFilehandles+i+1, pFilehandles+i+2, sizeof( struct pollfd )*(nConnections-i-1) );
                }
                nConnections--;
                pConnections = (IceConn*)rtl_reallocateMemory( pConnections, sizeof( IceConn )*nConnections );
                pFilehandles = (struct pollfd*)rtl_reallocateMemory( pFilehandles, sizeof( struct pollfd )*(nConnections+1) );
                break;
            }
        }
        if( nConnections == 0 && ICEThread )
        {
            SMprintf( "terminating ICEThread\n" );
            osl_terminateThread( ICEThread );
            wakeup();
            // must release the mutex here
            osl_releaseMutex( ICEMutex );
            osl_joinWithThread( ICEThread );
            osl_destroyThread( ICEThread );
            close( nWakeupFiles[1] );
            close( nWakeupFiles[0] );
            ICEThread = NULL;
        }
    }
    SMprintf( "ICE connection on %d %s\n",
              IceConnectionNumber( connection ),
              opening ? "inserted" : "removed" );
    SMprintf( "Display connection is %d\n", ConnectionNumber( GetX11SalData()->GetDisplay()->GetDisplay() ) );
#endif
}
