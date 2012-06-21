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

#include <string.h>
#include <unistd.h>
#include <sys/poll.h>
#include <fcntl.h>

#include <stdio.h>

#include <rtl/strbuf.hxx>

#include <osl/process.h>
#include <osl/security.h>
#include <osl/conditn.h>

#include <tools/prex.h>
#include <X11/Xatom.h>
#include <tools/postx.h>

#include <unx/sm.hxx>
#include <unx/saldata.hxx>
#include <unx/saldisp.hxx>
#include <unx/salframe.h>
#include <unx/salinst.h>

#include <vcl/svapp.hxx>
#include <vcl/window.hxx>

#define USE_SM_EXTENSION

static IceSalSession* pOneInstance = NULL;

SalSession* X11SalInstance::CreateSalSession()
{
    if( ! pOneInstance )
        pOneInstance = new IceSalSession();
    SessionManagerClient::open();
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
    SessionManagerClient::interactionDone( false );
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
    SessionManagerClient::interactionDone( true );
    return false;
}

extern "C" void SAL_CALL ICEConnectionWorker( void* );

class ICEConnectionObserver
{
    friend void SAL_CALL ICEConnectionWorker(void*);
    static sal_Bool bIsWatching;
    static void ICEWatchProc( IceConn connection, IcePointer client_data,
                              Bool opening, IcePointer* watch_data );

    static struct pollfd* pFilehandles;
    static IceConn* pConnections;
    static int nConnections;
    static int nWakeupFiles[2];
    static oslMutex ICEMutex;
    static oslThread ICEThread;
#ifdef USE_SM_EXTENSION
    static IceIOErrorHandler origIOErrorHandler;
    static IceErrorHandler origErrorHandler;
#endif
public:

    static void activate();
    static void deactivate();
    static void lock();
    static void unlock();
    static void wakeup();
};


SmcConn             SessionManagerClient::aSmcConnection            = NULL;
rtl::OString SessionManagerClient::m_aClientID;
sal_Bool                ICEConnectionObserver::bIsWatching              = sal_False;
struct pollfd*  ICEConnectionObserver::pFilehandles             = NULL;
IceConn*            ICEConnectionObserver::pConnections             = NULL;
int                 ICEConnectionObserver::nConnections             = 0;
oslMutex            ICEConnectionObserver::ICEMutex                 = NULL;
oslThread           ICEConnectionObserver::ICEThread                = NULL;
int                 ICEConnectionObserver::nWakeupFiles[2]          = { 0, 0 };

#ifdef USE_SM_EXTENSION
IceIOErrorHandler ICEConnectionObserver::origIOErrorHandler = NULL;
IceErrorHandler ICEConnectionObserver::origErrorHandler = NULL;

static void IgnoreIceErrors(IceConn, Bool, int, unsigned long, int, int, IcePointer)
{
}

static void IgnoreIceIOErrors(IceConn)
{
}
#endif

// HACK
bool SessionManagerClient::bDocSaveDone = false;


static SmProp*  pSmProps = NULL;
static SmProp** ppSmProps = NULL;
static int      nSmProps = 0;
static unsigned char   *pSmRestartHint = NULL;


static void BuildSmPropertyList()
{
    if( ! pSmProps )
    {
        rtl::OString aExec(rtl::OUStringToOString(SessionManagerClient::getExecName(), osl_getThreadTextEncoding()));

        nSmProps = 5;
        pSmProps = new SmProp[ nSmProps ];

        pSmProps[ 0 ].name      = const_cast<char*>(SmCloneCommand);
        pSmProps[ 0 ].type      = const_cast<char*>(SmLISTofARRAY8);
        pSmProps[ 0 ].num_vals  = 1;
        pSmProps[ 0 ].vals      = new SmPropValue;
        pSmProps[ 0 ].vals->length  = aExec.getLength()+1;
        pSmProps[ 0 ].vals->value   = strdup( aExec.getStr() );

        pSmProps[ 1 ].name      = const_cast<char*>(SmProgram);
        pSmProps[ 1 ].type      = const_cast<char*>(SmARRAY8);
        pSmProps[ 1 ].num_vals  = 1;
        pSmProps[ 1 ].vals      = new SmPropValue;
        pSmProps[ 1 ].vals->length  = aExec.getLength()+1;
        pSmProps[ 1 ].vals->value   = strdup( aExec.getStr() );

        pSmProps[ 2 ].name      = const_cast<char*>(SmRestartCommand);
        pSmProps[ 2 ].type      = const_cast<char*>(SmLISTofARRAY8);
        pSmProps[ 2 ].num_vals  = 3;
        pSmProps[ 2 ].vals      = new SmPropValue[3];
        pSmProps[ 2 ].vals[0].length    = aExec.getLength()+1;
        pSmProps[ 2 ].vals[0].value = strdup( aExec.getStr() );
        rtl::OStringBuffer aRestartOption;
        aRestartOption.append(RTL_CONSTASCII_STRINGPARAM("--session="));
        aRestartOption.append(SessionManagerClient::getSessionID());
        pSmProps[ 2 ].vals[1].length    = aRestartOption.getLength()+1;
        pSmProps[ 2 ].vals[1].value = strdup(aRestartOption.getStr());
        rtl::OString aRestartOptionNoLogo(RTL_CONSTASCII_STRINGPARAM("--nologo"));
        pSmProps[ 2 ].vals[2].length    = aRestartOptionNoLogo.getLength()+1;
        pSmProps[ 2 ].vals[2].value = strdup(aRestartOptionNoLogo.getStr());

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

        pSmProps[ 4 ].name      = const_cast<char*>(SmRestartStyleHint);
        pSmProps[ 4 ].type      = const_cast<char*>(SmCARD8);
        pSmProps[ 4 ].num_vals  = 1;
        pSmProps[ 4 ].vals      = new SmPropValue;
        pSmProps[ 4 ].vals->value   = malloc(1);
        pSmRestartHint = (unsigned char *)pSmProps[ 4 ].vals->value;
        *pSmRestartHint = SmRestartIfRunning;
        pSmProps[ 4 ].vals->length  = 1;

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
    //decode argument smuggled in by abusing pThis member of SessionManagerClient
    sal_uIntPtr nStateVal = (sal_uIntPtr)pThis;
    Bool shutdown = nStateVal != 0;

    SAL_INFO("vcl.sm", "posting save documents event shutdown = " << (shutdown ? "true" : "false" ));

    static bool bFirstShutdown=true;
    if (shutdown && bFirstShutdown) //first shutdown request
    {
        bFirstShutdown = false;
        /*
          If we have no actual frames open, e.g. we launched a quickstarter,
          and then shutdown all our frames leaving just a quickstarter running,
          then we don't want to launch an empty toplevel frame on the next
          start. (The job of scheduling the restart of the quick-starter is a
          task of the quick-starter)
        */
        *pSmRestartHint = SmRestartNever;
        const std::list< SalFrame* >& rFrames = GetGenericData()->GetSalDisplay()->getFrames();
        for( std::list< SalFrame* >::const_iterator it = rFrames.begin(); it != rFrames.end(); ++it )
        {
            Window *pWindow = (*it)->GetWindow();
            if (pWindow && pWindow->IsVisible())
            {
                *pSmRestartHint = SmRestartIfRunning;
                break;
            }
        }
    }

    if( pOneInstance )
    {
        SalSessionSaveRequestEvent aEvent( shutdown, false );
        pOneInstance->CallCallback( &aEvent );
    }
    else
        saveDone();

    return 0;
}

IMPL_STATIC_LINK_NOINSTANCE( SessionManagerClient, InteractionHdl, void*, EMPTYARG )
{
    SAL_INFO("vcl.sm", "interaction link");
    if( pOneInstance )
    {
        SalSessionInteractionEvent aEvent( true );
        pOneInstance->CallCallback( &aEvent );
    }

    return 0;
}

IMPL_STATIC_LINK_NOINSTANCE( SessionManagerClient, ShutDownCancelHdl, void*, EMPTYARG )
{
    SAL_INFO("vcl.sm", "shutdown cancel");
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
    SAL_INFO("vcl.sm", "Session: save yourself," <<
        "save_type " <<
            " local: " << (save_type == SmSaveLocal) <<
            " global: " << (save_type == SmSaveGlobal) <<
            " both: " << (save_type == SmSaveBoth) <<
        " shutdown: " << shutdown <<
        " interact_style: " <<
            " SmInteractStyleNone: " << (interact_style == SmInteractStyleNone) <<
            " SmInteractStyleErrors: " << (interact_style == SmInteractStyleErrors) <<
            " SmInteractStyleErrors: " << (interact_style == SmInteractStyleAny));
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
    //Smuggle argument in by abusing pThis member of SessionManagerClient
    sal_uIntPtr nStateVal = shutdown ? 0xffffffff : 0x0;
    Application::PostUserEvent( STATIC_LINK( (void*)nStateVal, SessionManagerClient, SaveYourselfHdl ) );
    SAL_INFO("vcl.sm", "waiting for save yourself event to be processed" );
#endif
}

IMPL_STATIC_LINK_NOINSTANCE( SessionManagerClient, ShutDownHdl, void*, EMPTYARG )
{
    if( pOneInstance )
    {
        SalSessionQuitEvent aEvent;
        pOneInstance->CallCallback( &aEvent );
    }

    const std::list< SalFrame* >& rFrames = GetGenericData()->GetSalDisplay()->getFrames();
    SAL_INFO("vcl.sm", (rFrames.begin() != rFrames.end() ? "shutdown on first frame" : "shutdown event but no frame"));
    if( rFrames.begin() != rFrames.end() )
        rFrames.front()->CallCallback( SALEVENT_SHUTDOWN, 0 );
    return 0;
}

void SessionManagerClient::DieProc(
    SmcConn connection,
    SmPointer
    )
{
    SAL_INFO("vcl.sm", "Session: die");
    if( connection == aSmcConnection )
    {
        Application::PostUserEvent( STATIC_LINK( NULL, SessionManagerClient, ShutDownHdl ) );
        SAL_INFO("vcl.sm", "waiting for shutdown event to be processed" );
    }
}

void SessionManagerClient::SaveCompleteProc(
    SmcConn,
    SmPointer
    )
{
    SAL_INFO("vcl.sm", "Session: save complete");
}

void SessionManagerClient::ShutdownCanceledProc(
    SmcConn connection,
    SmPointer )
{
    SAL_INFO("vcl.sm", "Session: shutdown canceled" );
    if( connection == aSmcConnection )
        Application::PostUserEvent( STATIC_LINK( NULL, SessionManagerClient, ShutDownCancelHdl ) );
}

void SessionManagerClient::InteractProc(
                                        SmcConn connection,
                                        SmPointer )
{
    SAL_INFO("vcl.sm", "Session: interaction request completed" );
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
        SAL_INFO("vcl.sm", "sent SaveYourselfDone SmRestartHint of " << *pSmRestartHint );
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
        const rtl::OString& rPrevId(getPreviousSessionID());

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
                                            rPrevId.isEmpty() ? NULL : const_cast<char*>(rPrevId.getStr()),
                                            &pClientID,
                                            sizeof( aErrBuf ),
                                            aErrBuf );
        if( ! aSmcConnection )
            SAL_INFO("vcl.sm", "SmcOpenConnection failed: " << aErrBuf);
        else
            SAL_INFO("vcl.sm", "SmcOpenConnection succeeded, client ID is " << pClientID );
        m_aClientID = rtl::OString(pClientID);
        free( pClientID );
        pClientID = NULL;
        ICEConnectionObserver::unlock();

        SalDisplay* pDisp = GetGenericData()->GetSalDisplay();
        if( pDisp->GetDrawable(pDisp->GetDefaultXScreen()) && !m_aClientID.isEmpty() )
        {
            XChangeProperty( pDisp->GetDisplay(),
                             pDisp->GetDrawable( pDisp->GetDefaultXScreen() ),
                             XInternAtom( pDisp->GetDisplay(), "SM_CLIENT_ID", False ),
                             XA_STRING,
                             8,
                             PropModeReplace,
                             (unsigned char*)m_aClientID.getStr(),
                             m_aClientID.getLength()
                             );
        }
    }
    else if( ! aSmcConnection )
        SAL_INFO("vcl.sm", "no SESSION_MANAGER");
#endif
}

const rtl::OString& SessionManagerClient::getSessionID()
{
    return m_aClientID;
}

void SessionManagerClient::close()
{
    if( aSmcConnection )
    {
#ifdef USE_SM_EXTENSION
        ICEConnectionObserver::lock();
        SAL_INFO("vcl.sm", "attempting SmcCloseConnection");
        SmcCloseConnection( aSmcConnection, 0, NULL );
        SAL_INFO("vcl.sm", "SmcConnection closed");
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

void SessionManagerClient::interactionDone( bool bCancelShutdown )
{
    if( aSmcConnection )
    {
        ICEConnectionObserver::lock();
        SmcInteractDone( aSmcConnection, bCancelShutdown ? True : False );
        ICEConnectionObserver::unlock();
    }
}


rtl::OUString SessionManagerClient::getExecName()
{
    rtl::OUString aExec, aSysExec;
    osl_getExecutableFile( &aExec.pData );
    osl_getSystemPathFromFileURL( aExec.pData, &aSysExec.pData );

    int nPos = aSysExec.indexOf( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(".bin")) );
    if( nPos != -1 )
        aSysExec = aSysExec.copy( 0, nPos );
    return aSysExec;
}


const rtl::OString& SessionManagerClient::getPreviousSessionID()
{
    static rtl::OString aPrevId;

    int nCommands = osl_getCommandArgCount();
    for (int i = 0; i < nCommands; ++i)
    {
        ::rtl::OUString aArg;
        osl_getCommandArg( i, &aArg.pData );
        if(aArg.matchAsciiL(RTL_CONSTASCII_STRINGPARAM("--session=")))
        {
            aPrevId = rtl::OUStringToOString(
                aArg.copy(RTL_CONSTASCII_LENGTH("--session=")),
                osl_getThreadTextEncoding());
            break;
        }
    }

    SAL_INFO("vcl.sm", "previous ID = " << aPrevId.getStr());
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
        bIsWatching = sal_True;
#ifdef USE_SM_EXTENSION
        /*
         * Default handlers call exit, we don't care that strongly if something
         * happens to fail
         */
        origIOErrorHandler = IceSetIOErrorHandler( IgnoreIceIOErrors );
        origErrorHandler = IceSetErrorHandler( IgnoreIceErrors );
        IceAddConnectionWatch( ICEWatchProc, NULL );
#endif
    }
}

void ICEConnectionObserver::deactivate()
{
    if( bIsWatching )
    {
        lock();
        bIsWatching = sal_False;
#ifdef USE_SM_EXTENSION
        IceRemoveConnectionWatch( ICEWatchProc, NULL );
        IceSetErrorHandler( origErrorHandler );
        IceSetIOErrorHandler( origIOErrorHandler );
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
    OSL_VERIFY(write( nWakeupFiles[1], &cChar, 1 ) == 1);
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
            SAL_INFO("vcl.sm", "file handles active in wakeup: " << nRet);
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
                SAL_INFO("vcl.sm", "IceProcessMessages");
                Bool bReply;
                for( int i = 0; i < ICEConnectionObserver::nConnections; i++ )
                    if( ICEConnectionObserver::pFilehandles[i+1].revents & POLLIN )
                        IceProcessMessages( ICEConnectionObserver::pConnections[i], NULL, &bReply );
            }
        }
        ICEConnectionObserver::unlock();
    }
#endif
    SAL_INFO("vcl.sm", "shutting donw ICE dispatch thread");
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
            SAL_INFO("vcl.sm", "terminating ICEThread");
            osl_terminateThread( ICEThread );
            wakeup();
            // must release the mutex here
            osl_releaseMutex( ICEMutex );
            osl_joinWithThread( ICEThread );
            osl_destroyThread( ICEThread );
            close( nWakeupFiles[1] );
            close( nWakeupFiles[0] );
            ICEThread = NULL;
            // acquire the mutex again, because the caller does not expect
            // it to be released when calling into SM
            osl_acquireMutex( ICEMutex );
        }
    }
    SAL_INFO( "vcl.sm", "ICE connection on " << IceConnectionNumber( connection ) << " " << (opening ? "inserted" : "removed"));
    SAL_INFO( "vcl.sm", "Display connection is " << ConnectionNumber( GetGenericData()->GetSalDisplay()->GetDisplay() ) );
#endif
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
