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

#include "sal/config.h"

#include <cassert>

#include <string.h>
#include <unistd.h>
#include <sys/poll.h>
#include <fcntl.h>

#include <rtl/strbuf.hxx>

#include <rtl/process.h>
#include <osl/security.h>
#include <osl/conditn.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>

#include <unx/sm.hxx>
#include <unx/saldata.hxx>
#include <unx/saldisp.hxx>
#include <unx/salframe.h>
#include <unx/salinst.h>

#include <vcl/svapp.hxx>
#include <vcl/window.hxx>

#include "salsession.hxx"

namespace {

class IceSalSession : public SalSession
{
public:
    IceSalSession() {}

private:
    virtual ~IceSalSession() {}

    virtual void queryInteraction() override;
    virtual void interactionDone() override;
    virtual void saveDone() override;
    virtual bool cancelShutdown() override;
};

}

SalSession* X11SalInstance::CreateSalSession()
{
    SalSession * p = new IceSalSession;
    SessionManagerClient::open(p);
    return p;
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
}

bool IceSalSession::cancelShutdown()
{
    SessionManagerClient::interactionDone( true );
    return false;
}

extern "C" void ICEWatchProc(
    IceConn ice_conn, IcePointer client_data, Bool opening,
    IcePointer * watch_data);

extern "C" void SAL_CALL ICEConnectionWorker(void * data);

class ICEConnectionObserver
{
    friend void ICEWatchProc(IceConn, IcePointer, Bool, IcePointer *);

    friend void ICEConnectionWorker(void *);

    struct pollfd* m_pFilehandles;
    int m_nConnections;
    IceConn* m_pConnections;
    int m_nWakeupFiles[2];
    oslThread m_ICEThread;
    IceIOErrorHandler m_origIOErrorHandler;
    IceErrorHandler m_origErrorHandler;

    void wakeup();

public:
    osl::Mutex m_ICEMutex;

    ICEConnectionObserver()
        : m_pFilehandles(nullptr)
        , m_nConnections(0)
        , m_pConnections(nullptr)
        , m_ICEThread(nullptr)
        , m_origIOErrorHandler(nullptr)
        , m_origErrorHandler(nullptr)
    {
        m_nWakeupFiles[0] = m_nWakeupFiles[1] = 0;
    }

    void activate();
    void deactivate();
    void terminate(oslThread iceThread);
};

SalSession * SessionManagerClient::m_pSession = nullptr;
std::unique_ptr< ICEConnectionObserver >
SessionManagerClient::m_xICEConnectionObserver;
SmcConn SessionManagerClient::m_pSmcConnection = nullptr;
OString SessionManagerClient::m_aClientID;
bool SessionManagerClient::m_bDocSaveDone = false; // HACK

extern "C" {

static void IgnoreIceErrors(
    SAL_UNUSED_PARAMETER IceConn, SAL_UNUSED_PARAMETER Bool,
    SAL_UNUSED_PARAMETER int, SAL_UNUSED_PARAMETER unsigned long,
    SAL_UNUSED_PARAMETER int, SAL_UNUSED_PARAMETER int,
    SAL_UNUSED_PARAMETER IcePointer)
{}

static void IgnoreIceIOErrors(SAL_UNUSED_PARAMETER IceConn) {}

}

static SmProp*  pSmProps = nullptr;
static SmProp** ppSmProps = nullptr;
static int      nSmProps = 0;
static unsigned char   *pSmRestartHint = nullptr;

static void BuildSmPropertyList()
{
    if( ! pSmProps )
    {
        OString aExec(OUStringToOString(SessionManagerClient::getExecName(), osl_getThreadTextEncoding()));

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
        OStringBuffer aRestartOption;
        aRestartOption.append("--session=");
        aRestartOption.append(SessionManagerClient::getSessionID());
        pSmProps[ 2 ].vals[1].length    = aRestartOption.getLength()+1;
        pSmProps[ 2 ].vals[1].value = strdup(aRestartOption.getStr());
        OString aRestartOptionNoLogo("--nologo");
        pSmProps[ 2 ].vals[2].length    = aRestartOptionNoLogo.getLength()+1;
        pSmProps[ 2 ].vals[2].value = strdup(aRestartOptionNoLogo.getStr());

        OUString aUserName;
        OString aUser;
        oslSecurity aSec = osl_getCurrentSecurity();
        if( aSec )
        {
            osl_getUserName( aSec, &aUserName.pData );
            aUser = OUStringToOString( aUserName, osl_getThreadTextEncoding() );
            osl_freeSecurityHandle( aSec );
        }

        pSmProps[ 3 ].name      = const_cast<char*>(SmUserID);
        pSmProps[ 3 ].type      = const_cast<char*>(SmARRAY8);
        pSmProps[ 3 ].num_vals  = 1;
        pSmProps[ 3 ].vals      = new SmPropValue;
        pSmProps[ 3 ].vals->value   = strdup( aUser.getStr() );
        pSmProps[ 3 ].vals->length  = rtl_str_getLength( static_cast<char *>(pSmProps[ 3 ].vals->value) )+1;

        pSmProps[ 4 ].name      = const_cast<char*>(SmRestartStyleHint);
        pSmProps[ 4 ].type      = const_cast<char*>(SmCARD8);
        pSmProps[ 4 ].num_vals  = 1;
        pSmProps[ 4 ].vals      = new SmPropValue;
        pSmProps[ 4 ].vals->value   = malloc(1);
        pSmRestartHint = static_cast<unsigned char *>(pSmProps[ 4 ].vals->value);
        *pSmRestartHint = SmRestartIfRunning;
        pSmProps[ 4 ].vals->length  = 1;

        ppSmProps = new SmProp*[ nSmProps ];
        for( int i = 0; i < nSmProps; i++ )
            ppSmProps[ i ] = &pSmProps[i];
    }
}

bool SessionManagerClient::checkDocumentsSaved()
{
    return m_bDocSaveDone;
}

IMPL_STATIC_LINK_TYPED( SessionManagerClient, SaveYourselfHdl, void*, pStateVal, void )
{
    // Decode argument smuggled in as void*:
    sal_uIntPtr nStateVal = reinterpret_cast< sal_uIntPtr >(pStateVal);
    bool shutdown = nStateVal != 0;

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
        const std::list< SalFrame* >& rFrames = vcl_sal::getSalDisplay(GetGenericData())->getFrames();
        for( std::list< SalFrame* >::const_iterator it = rFrames.begin(); it != rFrames.end(); ++it )
        {
            vcl::Window *pWindow = (*it)->GetWindow();
            if (pWindow && pWindow->IsVisible())
            {
                *pSmRestartHint = SmRestartIfRunning;
                break;
            }
        }
    }

    if( m_pSession )
    {
        SalSessionSaveRequestEvent aEvent( shutdown );
        m_pSession->CallCallback( &aEvent );
    }
    else
        saveDone();
}

IMPL_STATIC_LINK_NOARG_TYPED( SessionManagerClient, InteractionHdl, void*, void )
{
    SAL_INFO("vcl.sm", "interaction link");
    if( m_pSession )
    {
        SalSessionInteractionEvent aEvent( true );
        m_pSession->CallCallback( &aEvent );
    }
}

IMPL_STATIC_LINK_NOARG_TYPED( SessionManagerClient, ShutDownCancelHdl, void*, void )
{
    SAL_INFO("vcl.sm", "shutdown cancel");
    if( m_pSession )
    {
        SalSessionShutdownCancelEvent aEvent;
        m_pSession->CallCallback( &aEvent );
    }
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
    SAL_INFO("vcl.sm", "Session: save yourself, "
        "save_type "
            " local: " << (save_type == SmSaveLocal) <<
            " global: " << (save_type == SmSaveGlobal) <<
            " both: " << (save_type == SmSaveBoth) <<
        " shutdown: " << shutdown <<
        " interact_style: "
            " SmInteractStyleNone: " << (interact_style == SmInteractStyleNone) <<
            " SmInteractStyleErrors: " << (interact_style == SmInteractStyleErrors) <<
            " SmInteractStyleErrors: " << (interact_style == SmInteractStyleAny));
    BuildSmPropertyList();
    m_bDocSaveDone = false;
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
    // Smuggle argument in as void*:
    sal_uIntPtr nStateVal = shutdown;
    Application::PostUserEvent( LINK( nullptr, SessionManagerClient, SaveYourselfHdl ), reinterpret_cast< void * >(nStateVal) );
    SAL_INFO("vcl.sm", "waiting for save yourself event to be processed" );
}

IMPL_STATIC_LINK_NOARG_TYPED( SessionManagerClient, ShutDownHdl, void*, void )
{
    if( m_pSession )
    {
        SalSessionQuitEvent aEvent;
        m_pSession->CallCallback( &aEvent );
    }

    const std::list< SalFrame* >& rFrames = vcl_sal::getSalDisplay(GetGenericData())->getFrames();
    SAL_INFO("vcl.sm", (!rFrames.empty() ? "shutdown on first frame" : "shutdown event but no frame"));
    if( !rFrames.empty() )
        rFrames.front()->CallCallback( SalEvent::Shutdown, nullptr );
}

void SessionManagerClient::DieProc(
    SmcConn connection,
    SmPointer
    )
{
    SAL_INFO("vcl.sm", "Session: die");
    if( connection == m_pSmcConnection )
    {
        Application::PostUserEvent( LINK( nullptr, SessionManagerClient, ShutDownHdl ) );
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
    if( connection == m_pSmcConnection )
        Application::PostUserEvent( LINK( nullptr, SessionManagerClient, ShutDownCancelHdl ) );
}

void SessionManagerClient::InteractProc(
                                        SmcConn connection,
                                        SmPointer )
{
    SAL_INFO("vcl.sm", "Session: interaction request completed" );
    if( connection == m_pSmcConnection )
        Application::PostUserEvent( LINK( nullptr, SessionManagerClient, InteractionHdl ) );
}

void SessionManagerClient::saveDone()
{
    if( m_pSmcConnection )
    {
        assert(m_xICEConnectionObserver);
        osl::MutexGuard g(m_xICEConnectionObserver->m_ICEMutex);
        SmcSetProperties( m_pSmcConnection, nSmProps, ppSmProps );
        SmcSaveYourselfDone( m_pSmcConnection, True );
        SAL_INFO("vcl.sm", "sent SaveYourselfDone SmRestartHint of " << *pSmRestartHint );
        m_bDocSaveDone = true;
    }
}

void SessionManagerClient::open(SalSession * pSession)
{
    assert(!m_pSession && !m_xICEConnectionObserver && !m_pSmcConnection);
        // must only be called once
    m_pSession = pSession;
    // This is the way Xt does it, so we can too:
    if( getenv( "SESSION_MANAGER" ) )
    {
        m_xICEConnectionObserver.reset(new ICEConnectionObserver);
        m_xICEConnectionObserver->activate();

        {
            osl::MutexGuard g(m_xICEConnectionObserver->m_ICEMutex);

            static SmcCallbacks aCallbacks; // does this need to be static?
            aCallbacks.save_yourself.callback           = SaveYourselfProc;
            aCallbacks.save_yourself.client_data        = nullptr;
            aCallbacks.die.callback                     = DieProc;
            aCallbacks.die.client_data                  = nullptr;
            aCallbacks.save_complete.callback           = SaveCompleteProc;
            aCallbacks.save_complete.client_data        = nullptr;
            aCallbacks.shutdown_cancelled.callback      = ShutdownCanceledProc;
            aCallbacks.shutdown_cancelled.client_data   = nullptr;
            OString aPrevId(getPreviousSessionID());
            char* pClientID = nullptr;
            char aErrBuf[1024];
            m_pSmcConnection = SmcOpenConnection( nullptr,
                                                  nullptr,
                                                  SmProtoMajor,
                                                  SmProtoMinor,
                                                  SmcSaveYourselfProcMask         |
                                                  SmcDieProcMask                  |
                                                  SmcSaveCompleteProcMask         |
                                                  SmcShutdownCancelledProcMask    ,
                                                  &aCallbacks,
                                                  aPrevId.isEmpty() ? nullptr : const_cast<char*>(aPrevId.getStr()),
                                                  &pClientID,
                                                  sizeof( aErrBuf ),
                                                  aErrBuf );
            if( !m_pSmcConnection )
                SAL_INFO("vcl.sm", "SmcOpenConnection failed: " << aErrBuf);
            else
                SAL_INFO("vcl.sm", "SmcOpenConnection succeeded, client ID is " << pClientID );
            m_aClientID = OString(pClientID);
            free( pClientID );
            pClientID = nullptr;
        }

        SalDisplay* pDisp = vcl_sal::getSalDisplay(GetGenericData());
        if( pDisp->GetDrawable(pDisp->GetDefaultXScreen()) && !m_aClientID.isEmpty() )
        {
            XChangeProperty( pDisp->GetDisplay(),
                             pDisp->GetDrawable( pDisp->GetDefaultXScreen() ),
                             XInternAtom( pDisp->GetDisplay(), "SM_CLIENT_ID", False ),
                             XA_STRING,
                             8,
                             PropModeReplace,
                             reinterpret_cast<unsigned char const *>(m_aClientID.getStr()),
                             m_aClientID.getLength()
                             );
        }
    }
    else
    {
        SAL_INFO("vcl.sm", "no SESSION_MANAGER");
    }
}

const OString& SessionManagerClient::getSessionID()
{
    return m_aClientID;
}

void SessionManagerClient::close()
{
    if( m_pSmcConnection )
    {
        assert(m_xICEConnectionObserver);
        {
            osl::MutexGuard g(m_xICEConnectionObserver->m_ICEMutex);
            SAL_INFO("vcl.sm", "attempting SmcCloseConnection");
            SmcCloseConnection( m_pSmcConnection, 0, nullptr );
            SAL_INFO("vcl.sm", "SmcConnection closed");
        }
        m_xICEConnectionObserver->deactivate();
        m_xICEConnectionObserver.reset();
        m_pSmcConnection = nullptr;
    }
}

bool SessionManagerClient::queryInteraction()
{
    bool bRet = false;
    if( m_pSmcConnection )
    {
        assert(m_xICEConnectionObserver);
        osl::MutexGuard g(m_xICEConnectionObserver->m_ICEMutex);
        if( SmcInteractRequest( m_pSmcConnection, SmDialogNormal, InteractProc, nullptr ) )
            bRet = true;
    }
    return bRet;
}

void SessionManagerClient::interactionDone( bool bCancelShutdown )
{
    if( m_pSmcConnection )
    {
        assert(m_xICEConnectionObserver);
        osl::MutexGuard g(m_xICEConnectionObserver->m_ICEMutex);
        SmcInteractDone( m_pSmcConnection, bCancelShutdown ? True : False );
    }
}

OUString SessionManagerClient::getExecName()
{
    OUString aExec, aSysExec;
    osl_getExecutableFile( &aExec.pData );
    osl_getSystemPathFromFileURL( aExec.pData, &aSysExec.pData );

    if( aSysExec.endsWith(".bin") )
        aSysExec = aSysExec.copy( 0, aSysExec.getLength() - RTL_CONSTASCII_LENGTH(".bin") );
    return aSysExec;
}

OString SessionManagerClient::getPreviousSessionID()
{
    OString aPrevId;

    sal_uInt32 n = rtl_getAppCommandArgCount();
    for (sal_uInt32 i = 0; i != n; ++i)
    {
        OUString aArg;
        rtl_getAppCommandArg( i, &aArg.pData );
        if(aArg.match("--session="))
        {
            aPrevId = OUStringToOString(
                aArg.copy(RTL_CONSTASCII_LENGTH("--session=")),
                osl_getThreadTextEncoding());
            break;
        }
    }

    SAL_INFO("vcl.sm", "previous ID = " << aPrevId.getStr());
    return aPrevId;
}

void ICEConnectionObserver::activate()
{
    /*
     * Default handlers call exit, we don't care that strongly if something
     * happens to fail
     */
    m_origIOErrorHandler = IceSetIOErrorHandler( IgnoreIceIOErrors );
    m_origErrorHandler = IceSetErrorHandler( IgnoreIceErrors );
    IceAddConnectionWatch( ICEWatchProc, this );
}

void ICEConnectionObserver::deactivate()
{
    oslThread t;
    {
        osl::MutexGuard g(m_ICEMutex);
        IceRemoveConnectionWatch( ICEWatchProc, this );
        IceSetErrorHandler( m_origErrorHandler );
        IceSetIOErrorHandler( m_origIOErrorHandler );
        m_nConnections = 0;
        t = m_ICEThread;
        m_ICEThread = nullptr;
    }
    if (t)
    {
        terminate(t);
    }
}

void ICEConnectionObserver::wakeup()
{
    char cChar = 'w';
    OSL_VERIFY(write(m_nWakeupFiles[1], &cChar, 1) == 1);
}

void ICEConnectionObserver::terminate(oslThread iceThread)
{
    osl_terminateThread(iceThread);
    wakeup();
    osl_joinWithThread(iceThread);
    osl_destroyThread(iceThread);
    close(m_nWakeupFiles[1]);
    close(m_nWakeupFiles[0]);
}

void ICEConnectionWorker(void * data)
{
    osl::Thread::setName("ICEConnectionWorker");
    ICEConnectionObserver * pThis = static_cast< ICEConnectionObserver * >(
        data);
    for (;;)
    {
        oslThread t;
        {
            osl::MutexGuard g(pThis->m_ICEMutex);
            if (pThis->m_ICEThread == nullptr || pThis->m_nConnections == 0)
            {
                break;
            }
            t = pThis->m_ICEThread;
        }
        if (!osl_scheduleThread(t))
        {
            break;
        }

        int nConnectionsBefore;
        struct pollfd* pLocalFD;
        {
            osl::MutexGuard g(pThis->m_ICEMutex);
            nConnectionsBefore = pThis->m_nConnections;
            int nBytes = sizeof( struct pollfd )*(nConnectionsBefore+1);
            pLocalFD = static_cast<struct pollfd*>(rtl_allocateMemory( nBytes ));
            memcpy( pLocalFD, pThis->m_pFilehandles, nBytes );
        }

        int nRet = poll( pLocalFD,nConnectionsBefore+1,-1 );
        bool bWakeup = (pLocalFD[0].revents & POLLIN);
        rtl_freeMemory( pLocalFD );

        if( nRet < 1 )
            continue;

        // clear wakeup pipe
        if( bWakeup )
        {
            char buf[4];
            while( read( pThis->m_nWakeupFiles[0], buf, sizeof( buf ) ) > 0 )
                ;
            SAL_INFO("vcl.sm", "file handles active in wakeup: " << nRet);
            if( nRet == 1 )
                continue;
        }

        // check fd's after we obtained the lock
        osl::MutexGuard g(pThis->m_ICEMutex);
        if( pThis->m_nConnections > 0 && pThis->m_nConnections == nConnectionsBefore )
        {
            nRet = poll( pThis->m_pFilehandles+1, pThis->m_nConnections, 0 );
            if( nRet > 0 )
            {
                SAL_INFO("vcl.sm", "IceProcessMessages");
                Bool bReply;
                for( int i = 0; i < pThis->m_nConnections; i++ )
                    if( pThis->m_pFilehandles[i+1].revents & POLLIN )
                        IceProcessMessages( pThis->m_pConnections[i], nullptr, &bReply );
            }
        }
    }
    SAL_INFO("vcl.sm", "shutting down ICE dispatch thread");
}

void ICEWatchProc(
    IceConn ice_conn, IcePointer client_data, Bool opening,
    SAL_UNUSED_PARAMETER IcePointer *)
{
    // Note: This is a callback function for ICE; this implicitly means that a
    // call into ICE lib is calling this, so the m_ICEMutex MUST already be
    // locked by the caller.
    ICEConnectionObserver * pThis = static_cast< ICEConnectionObserver * >(
        client_data);
    if( opening )
    {
        int fd = IceConnectionNumber( ice_conn );
        pThis->m_nConnections++;
        pThis->m_pConnections = static_cast<IceConn*>(rtl_reallocateMemory( pThis->m_pConnections, sizeof( IceConn )*pThis->m_nConnections ));
        pThis->m_pFilehandles = static_cast<struct pollfd*>(rtl_reallocateMemory( pThis->m_pFilehandles, sizeof( struct pollfd )*(pThis->m_nConnections+1) ));
        pThis->m_pConnections[ pThis->m_nConnections-1 ]      = ice_conn;
        pThis->m_pFilehandles[ pThis->m_nConnections ].fd     = fd;
        pThis->m_pFilehandles[ pThis->m_nConnections ].events = POLLIN;
        if( pThis->m_nConnections == 1 )
        {
            if (!pipe(pThis->m_nWakeupFiles))
            {
                int flags;
                pThis->m_pFilehandles[0].fd      = pThis->m_nWakeupFiles[0];
                pThis->m_pFilehandles[0].events  = POLLIN;
                // set close-on-exec and nonblock descriptor flag.
                if ((flags = fcntl(pThis->m_nWakeupFiles[0], F_GETFD)) != -1)
                {
                    flags |= FD_CLOEXEC;
                    (void)fcntl(pThis->m_nWakeupFiles[0], F_SETFD, flags);
                }
                if ((flags = fcntl(pThis->m_nWakeupFiles[0], F_GETFL)) != -1)
                {
                    flags |= O_NONBLOCK;
                    (void)fcntl(pThis->m_nWakeupFiles[0], F_SETFL, flags);
                }
                // set close-on-exec and nonblock descriptor flag.
                if ((flags = fcntl(pThis->m_nWakeupFiles[1], F_GETFD)) != -1)
                {
                    flags |= FD_CLOEXEC;
                    (void)fcntl(pThis->m_nWakeupFiles[1], F_SETFD, flags);
                }
                if ((flags = fcntl(pThis->m_nWakeupFiles[1], F_GETFL)) != -1)
                {
                    flags |= O_NONBLOCK;
                    (void)fcntl(pThis->m_nWakeupFiles[1], F_SETFL, flags);
                }
                pThis->m_ICEThread = osl_createThread(
                    ICEConnectionWorker, pThis);
            }
        }
    }
    else // closing
    {
        for( int i = 0; i < pThis->m_nConnections; i++ )
        {
            if( pThis->m_pConnections[i] == ice_conn )
            {
                if( i < pThis->m_nConnections-1 )
                {
                    memmove( pThis->m_pConnections+i, pThis->m_pConnections+i+1, sizeof( IceConn )*(pThis->m_nConnections-i-1) );
                    memmove( pThis->m_pFilehandles+i+1, pThis->m_pFilehandles+i+2, sizeof( struct pollfd )*(pThis->m_nConnections-i-1) );
                }
                pThis->m_nConnections--;
                pThis->m_pConnections = static_cast<IceConn*>(rtl_reallocateMemory( pThis->m_pConnections, sizeof( IceConn )*pThis->m_nConnections ));
                pThis->m_pFilehandles = static_cast<struct pollfd*>(rtl_reallocateMemory( pThis->m_pFilehandles, sizeof( struct pollfd )*(pThis->m_nConnections+1) ));
                break;
            }
        }
        if( pThis->m_nConnections == 0 && pThis->m_ICEThread )
        {
            SAL_INFO("vcl.sm", "terminating ICEThread");
            oslThread t = pThis->m_ICEThread;
            pThis->m_ICEThread = nullptr;

            // must release the mutex here
            pThis->m_ICEMutex.release();

            pThis->terminate(t);

            // acquire the mutex again, because the caller does not expect
            // it to be released when calling into SM
            pThis->m_ICEMutex.acquire();
        }
    }
    SAL_INFO( "vcl.sm", "ICE connection on " << IceConnectionNumber( ice_conn ) << " " << (opening ? "inserted" : "removed"));
    SAL_INFO( "vcl.sm", "Display connection is " << ConnectionNumber( vcl_sal::getSalDisplay(GetGenericData())->GetDisplay() ) );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
