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
    virtual ~IceSalSession() override {}

    virtual void queryInteraction() override;
    virtual void interactionDone() override;
    virtual void saveDone() override;
    virtual bool cancelShutdown() override;
};

}

SalSession* X11SalInstance::CreateSalSession()
{
    SAL_INFO("vcl.sm", "X11SalInstance::CreateSalSession");

    SalSession * p = new IceSalSession;
    SessionManagerClient::open(p);
    return p;
}

void IceSalSession::queryInteraction()
{
    SAL_INFO("vcl.sm", "IceSalSession::queryInteraction");

    if( ! SessionManagerClient::queryInteraction() )
    {
        SAL_INFO("vcl.sm.debug", "  call SalSessionInteractionEvent");
        SalSessionInteractionEvent aEvent( false );
        CallCallback( &aEvent );
    }
}

void IceSalSession::interactionDone()
{
    SAL_INFO("vcl.sm", "IceSalSession::interactionDone");

    SessionManagerClient::interactionDone( false );
}

void IceSalSession::saveDone()
{
    SAL_INFO("vcl.sm", "IceSalSession::saveDone");

    SessionManagerClient::saveDone();
}

bool IceSalSession::cancelShutdown()
{
    SAL_INFO("vcl.sm", "IceSalSession::cancelShutdown");

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
        SAL_INFO("vcl.sm", "ICEConnectionObserver::ICEConnectionObserver");

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
OString SessionManagerClient::m_aClientID = "";
OString SessionManagerClient::m_aTimeID = "";
OString SessionManagerClient::m_aClientTimeID = "";
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
static char  ** ppSmDel  = nullptr;

static int      nSmProps = 0;
static int      nSmDel   = 0;
static unsigned char   *pSmRestartHint = nullptr;


enum { eCloneCommand, eProgram, eRestartCommand, eUserId, eRestartStyleHint };
enum { eDiscardCommand };


static void BuildSmPropertyList()
{
    SAL_INFO("vcl.sm", "BuildSmPropertyList");

    if( ! pSmProps )
    {
        nSmProps  = 5;
        nSmDel    = 1;
        pSmProps  = new SmProp[ nSmProps ];
        ppSmProps = new SmProp*[ nSmProps ];
        ppSmDel   = new char*[ nSmDel ];
    }

    OString aExec(OUStringToOString(SessionManagerClient::getExecName(), osl_getThreadTextEncoding()));

    pSmProps[ eCloneCommand ].name      = const_cast<char*>(SmCloneCommand);
    pSmProps[ eCloneCommand ].type      = const_cast<char*>(SmLISTofARRAY8);
    pSmProps[ eCloneCommand ].num_vals  = 1;
    pSmProps[ eCloneCommand ].vals      = new SmPropValue;
    pSmProps[ eCloneCommand ].vals->length  = aExec.getLength()+1;
    pSmProps[ eCloneCommand ].vals->value   = strdup( aExec.getStr() );

    pSmProps[ eProgram ].name      = const_cast<char*>(SmProgram);
    pSmProps[ eProgram ].type      = const_cast<char*>(SmARRAY8);
    pSmProps[ eProgram ].num_vals  = 1;
    pSmProps[ eProgram ].vals      = new SmPropValue;
    pSmProps[ eProgram ].vals->length  = aExec.getLength()+1;
    pSmProps[ eProgram ].vals->value   = strdup( aExec.getStr() );

    pSmProps[ eRestartCommand ].name      = const_cast<char*>(SmRestartCommand);
    pSmProps[ eRestartCommand ].type      = const_cast<char*>(SmLISTofARRAY8);
    pSmProps[ eRestartCommand ].num_vals  = 3;
    pSmProps[ eRestartCommand ].vals      = new SmPropValue[3];
    pSmProps[ eRestartCommand ].vals[0].length    = aExec.getLength()+1;
    pSmProps[ eRestartCommand ].vals[0].value = strdup( aExec.getStr() );
    OStringBuffer aRestartOption;
    aRestartOption.append("--session=");
    aRestartOption.append(SessionManagerClient::getSessionID());
    pSmProps[ eRestartCommand ].vals[1].length    = aRestartOption.getLength()+1;
    pSmProps[ eRestartCommand ].vals[1].value = strdup(aRestartOption.getStr());
    OString aRestartOptionNoLogo("--nologo");
    pSmProps[ eRestartCommand ].vals[2].length    = aRestartOptionNoLogo.getLength()+1;
    pSmProps[ eRestartCommand ].vals[2].value = strdup(aRestartOptionNoLogo.getStr());

    OUString aUserName;
    OString aUser;
    oslSecurity aSec = osl_getCurrentSecurity();
    if( aSec )
    {
        osl_getUserName( aSec, &aUserName.pData );
        aUser = OUStringToOString( aUserName, osl_getThreadTextEncoding() );
        osl_freeSecurityHandle( aSec );
    }

    pSmProps[ eUserId ].name      = const_cast<char*>(SmUserID);
    pSmProps[ eUserId ].type      = const_cast<char*>(SmARRAY8);
    pSmProps[ eUserId ].num_vals  = 1;
    pSmProps[ eUserId ].vals      = new SmPropValue;
    pSmProps[ eUserId ].vals->value   = strdup( aUser.getStr() );
    pSmProps[ eUserId ].vals->length  = rtl_str_getLength( static_cast<char *>(pSmProps[ 3 ].vals->value) )+1;

    pSmProps[ eRestartStyleHint ].name      = const_cast<char*>(SmRestartStyleHint);
    pSmProps[ eRestartStyleHint ].type      = const_cast<char*>(SmCARD8);
    pSmProps[ eRestartStyleHint ].num_vals  = 1;
    pSmProps[ eRestartStyleHint ].vals      = new SmPropValue;
    pSmProps[ eRestartStyleHint ].vals->value   = malloc(1);
    pSmRestartHint = static_cast<unsigned char *>(pSmProps[ 4 ].vals->value);
    *pSmRestartHint = SmRestartIfRunning;
    pSmProps[ eRestartStyleHint ].vals->length  = 1;

    for( int i = 0; i < nSmProps; i++ )
        ppSmProps[ i ] = &pSmProps[i];

    ppSmDel[eDiscardCommand] = const_cast<char*>(SmDiscardCommand);
}

bool SessionManagerClient::checkDocumentsSaved()
{
    SAL_INFO("vcl.sm", "SessionManagerClient::checkDocumentsSaved");

    SAL_INFO("vcl.sm.debug", "  m_bcheckDocumentsSaved = " << (m_bDocSaveDone ? "true" : "false" ));
    return m_bDocSaveDone;
}

IMPL_STATIC_LINK( SessionManagerClient, SaveYourselfHdl, void*, pStateVal, void )
{
    SAL_INFO("vcl.sm", "SessionManagerClient, SaveYourselfHdl");

    // Decode argument smuggled in as void*:
    sal_uIntPtr nStateVal = reinterpret_cast< sal_uIntPtr >(pStateVal);
    bool shutdown = nStateVal != 0;

    static bool bFirstShutdown=true;

    SAL_INFO("vcl.sm.debug", "  shutdown  = " << (shutdown ? "true" : "false" ) <<
                             ", bFirstShutdown = " << (bFirstShutdown ? "true" : "false" ));
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
                SAL_INFO("vcl.sm.debug", "  pSmRestartHint = SmRestartIfRunning");
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

IMPL_STATIC_LINK_NOARG( SessionManagerClient, InteractionHdl, void*, void )
{
    SAL_INFO("vcl.sm", "SessionManagerClient, InteractionHdl");

    if( m_pSession )
    {
        SalSessionInteractionEvent aEvent( true );
        m_pSession->CallCallback( &aEvent );
    }
}

IMPL_STATIC_LINK_NOARG( SessionManagerClient, ShutDownCancelHdl, void*, void )
{
    SAL_INFO("vcl.sm", "SessionManagerClient, ShutDownCancelHdl");

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
    SAL_INFO("vcl.sm", "SessionManagerClient::SaveYourselfProc");

    TimeValue now;
    osl_getSystemTime(&now);

    SAL_INFO("vcl.sm", "  save_type = "   <<    ((save_type == SmSaveLocal ) ? "local"  :
                                                 (save_type == SmSaveGlobal) ? "global" : "both") <<
                       ", shutdown = " <<        (shutdown ? "true" : "false" ) <<
                       ", interact_style = " << ((interact_style == SmInteractStyleNone)   ? "SmInteractStyleNone"   :
                                                 (interact_style == SmInteractStyleErrors) ? "SmInteractStyleErrors" :
                                                                                             "SmInteractStyleAny"));
    char num[100];
    snprintf(num, sizeof(num), "_%d_%d", now.Seconds, (now.Nanosec / 1000));
    m_aTimeID = OString(num);

    BuildSmPropertyList();

    SmcSetProperties( m_pSmcConnection, 1, &ppSmProps[ eProgram ] );
    SmcSetProperties( m_pSmcConnection, 1, &ppSmProps[ eUserId ] );


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
}

IMPL_STATIC_LINK_NOARG( SessionManagerClient, ShutDownHdl, void*, void )
{
    SAL_INFO("vcl.sm", "SessionManagerClient, ShutDownHdl");

    if( m_pSession )
    {
        SalSessionQuitEvent aEvent;
        m_pSession->CallCallback( &aEvent );
    }

    const std::list< SalFrame* >& rFrames = vcl_sal::getSalDisplay(GetGenericData())->getFrames();

    SAL_INFO("vcl.sm.debug", "  rFrames.empty() = " << (rFrames.empty() ? "true" : "false"));
    if( !rFrames.empty() )
        rFrames.front()->CallCallback( SalEvent::Shutdown, nullptr );
}

void SessionManagerClient::DieProc(
    SmcConn connection,
    SmPointer
    )
{
    SAL_INFO("vcl.sm", "SessionManagerClient::DieProc");

    if( connection == m_pSmcConnection )
    {
        SAL_INFO("vcl.sm.debug", "  connection == m_pSmcConnection" );
        Application::PostUserEvent( LINK( nullptr, SessionManagerClient, ShutDownHdl ) );
    }
}

void SessionManagerClient::SaveCompleteProc(
    SmcConn,
    SmPointer
    )
{
    SAL_INFO("vcl.sm", "SessionManagerClient::SaveCompleteProc");
}

void SessionManagerClient::ShutdownCanceledProc(
    SmcConn connection,
    SmPointer )
{
    SAL_INFO("vcl.sm", "SessionManagerClient::ShutdownCanceledProc" );

    SAL_INFO("vcl.sm.debug", "  connection == m_pSmcConnection = " <<  (( connection == m_pSmcConnection ) ? "true" : "false"));
    if( connection == m_pSmcConnection )
        Application::PostUserEvent( LINK( nullptr, SessionManagerClient, ShutDownCancelHdl ) );
}

void SessionManagerClient::InteractProc(
                                        SmcConn connection,
                                        SmPointer )
{
    SAL_INFO("vcl.sm", "SessionManagerClient::InteractProc" );

    SAL_INFO("vcl.sm.debug", "  connection == m_pSmcConnection = " <<  (( connection == m_pSmcConnection ) ? "true" : "false"));
    if( connection == m_pSmcConnection )
        Application::PostUserEvent( LINK( nullptr, SessionManagerClient, InteractionHdl ) );
}

void SessionManagerClient::saveDone()
{
    SAL_INFO("vcl.sm", "SessionManagerClient::saveDone");

    if( m_pSmcConnection )
    {
        assert(m_xICEConnectionObserver);
        osl::MutexGuard g(m_xICEConnectionObserver->m_ICEMutex);
        //SmcSetProperties( m_pSmcConnection, 1, &ppSmProps[ eCloneCommand ] );
        // this message-handling is now equal to kate and plasma desktop
        SmcSetProperties( m_pSmcConnection, 1, &ppSmProps[ eRestartCommand ] );
        SmcDeleteProperties( m_pSmcConnection, 1, &ppSmDel[ eDiscardCommand ] );
        SmcSetProperties( m_pSmcConnection, 1, &ppSmProps[ eRestartStyleHint ] );

        SmcSaveYourselfDone( m_pSmcConnection, True );
        SAL_INFO("vcl.sm.debug", "  sent SmRestartHint = " << (*pSmRestartHint) );
        m_bDocSaveDone = true;
    }
}

void SessionManagerClient::open(SalSession * pSession)
{
    SAL_INFO("vcl.sm", "SessionManagerClient::open");

    assert(!m_pSession && !m_xICEConnectionObserver && !m_pSmcConnection);
        // must only be called once
    m_pSession = pSession;
    // This is the way Xt does it, so we can too:
    if( getenv( "SESSION_MANAGER" ) )
    {
        SAL_INFO("vcl.sm.debug", "  getenv( SESSION_MANAGER ) = true");
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
                SAL_INFO("vcl.sm.debug", "  SmcOpenConnection failed: " << aErrBuf);
            else
                SAL_INFO("vcl.sm.debug", "  SmcOpenConnection succeeded, client ID is " << pClientID );
            m_aClientID = OString(pClientID);
            free( pClientID );
            pClientID = nullptr;
        }

        SalDisplay* pDisp = vcl_sal::getSalDisplay(GetGenericData());
        if( pDisp->GetDrawable(pDisp->GetDefaultXScreen()) && !m_aClientID.isEmpty() )
        {
            SAL_INFO("vcl.sm.debug", "  SmcOpenConnection open: pDisp->GetDrawable = true");
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
        SAL_INFO("vcl.sm.debug", "  getenv( SESSION_MANAGER ) = false");
    }
}

const OString& SessionManagerClient::getSessionID()
{
    SAL_INFO("vcl.sm", "SessionManagerClient::getSessionID");

    m_aClientTimeID = m_aClientID + m_aTimeID;

    SAL_INFO("vcl.sm", "  SessionID = " << str);

    return m_aClientTimeID;
}

void SessionManagerClient::close()
{
    SAL_INFO("vcl.sm", "SessionManagerClient::close");

    if( m_pSmcConnection )
    {
        SAL_INFO("vcl.sm.debug", "  attempting SmcCloseConnection");
        assert(m_xICEConnectionObserver);
        {
            osl::MutexGuard g(m_xICEConnectionObserver->m_ICEMutex);
            SmcCloseConnection( m_pSmcConnection, 0, nullptr );
            SAL_INFO("vcl.sm", "  SmcCloseConnection closed");
        }
        m_xICEConnectionObserver->deactivate();
        m_xICEConnectionObserver.reset();
        m_pSmcConnection = nullptr;
    }
}

bool SessionManagerClient::queryInteraction()
{
    SAL_INFO("vcl.sm", "SessionManagerClient::queryInteraction");

    bool bRet = false;
    if( m_pSmcConnection )
    {
        assert(m_xICEConnectionObserver);
        osl::MutexGuard g(m_xICEConnectionObserver->m_ICEMutex);
        SAL_INFO("vcl.sm.debug", "  SmcInteractRequest" );
        if( SmcInteractRequest( m_pSmcConnection, SmDialogNormal, InteractProc, nullptr ) )
            bRet = true;
    }
    return bRet;
}

void SessionManagerClient::interactionDone( bool bCancelShutdown )
{
    SAL_INFO("vcl.sm", "SessionManagerClient::interactionDone");

    if( m_pSmcConnection )
    {
        assert(m_xICEConnectionObserver);
        osl::MutexGuard g(m_xICEConnectionObserver->m_ICEMutex);
        SAL_INFO("vcl.sm.debug", "  SmcInteractDone = " << (bCancelShutdown ? "true" : "false") );
        SmcInteractDone( m_pSmcConnection, bCancelShutdown ? True : False );
    }
}

OUString SessionManagerClient::getExecName()
{
    SAL_INFO("vcl.sm", "SessionManagerClient::getExecName");

    OUString aExec, aSysExec;
    osl_getExecutableFile( &aExec.pData );
    osl_getSystemPathFromFileURL( aExec.pData, &aSysExec.pData );

    if( aSysExec.endsWith(".bin") )
        aSysExec = aSysExec.copy( 0, aSysExec.getLength() - RTL_CONSTASCII_LENGTH(".bin") );

    SAL_INFO("vcl.sm.debug", "  aSysExec = " << aSysExec);
    return aSysExec;
}

OString SessionManagerClient::getPreviousSessionID()
{
    SAL_INFO("vcl.sm", "SessionManagerClient::getPreviousSessionID");

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

    SAL_INFO("vcl.sm.debug", "  previous ID = " << aPrevId.getStr());
    return aPrevId;
}

void ICEConnectionObserver::activate()
{
    SAL_INFO("vcl.sm", "ICEConnectionObserver::activate");

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
    SAL_INFO("vcl.sm", "ICEConnectionObserver::deactivate");

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
        SAL_INFO("vcl.sm.debug", "  terminate");
        terminate(t);
    }
}

void ICEConnectionObserver::wakeup()
{
    SAL_INFO("vcl.sm", "ICEConnectionObserver::wakeup");

    char cChar = 'w';
    OSL_VERIFY(write(m_nWakeupFiles[1], &cChar, 1) == 1);
}

void ICEConnectionObserver::terminate(oslThread iceThread)
{
    SAL_INFO("vcl.sm", "ICEConnectionObserver::terminate");

    osl_terminateThread(iceThread);
    wakeup();
    osl_joinWithThread(iceThread);
    osl_destroyThread(iceThread);
    close(m_nWakeupFiles[1]);
    close(m_nWakeupFiles[0]);
}

void ICEConnectionWorker(void * data)
{
    SAL_INFO("vcl.sm", "ICEConnectionWorker");

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
            SAL_INFO("vcl.sm.debug", "  file handles active in wakeup: " << nRet);
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
                SAL_INFO("vcl.sm.debug", "  IceProcessMessages");
                Bool bReply;
                for( int i = 0; i < pThis->m_nConnections; i++ )
                    if( pThis->m_pFilehandles[i+1].revents & POLLIN )
                        IceProcessMessages( pThis->m_pConnections[i], nullptr, &bReply );
            }
        }
    }

    SAL_INFO("vcl.sm.debug", "  shutting down ICE dispatch thread");
}

void ICEWatchProc(
    IceConn ice_conn, IcePointer client_data, Bool opening,
    SAL_UNUSED_PARAMETER IcePointer *)
{
    SAL_INFO("vcl.sm", "ICEWatchProc");

    // Note: This is a callback function for ICE; this implicitly means that a
    // call into ICE lib is calling this, so the m_ICEMutex MUST already be
    // locked by the caller.
    ICEConnectionObserver * pThis = static_cast< ICEConnectionObserver * >(
        client_data);
    if( opening )
    {
        SAL_INFO("vcl.sm.debug", "  opening");
        int fd = IceConnectionNumber( ice_conn );
        pThis->m_nConnections++;
        pThis->m_pConnections = static_cast<IceConn*>(rtl_reallocateMemory( pThis->m_pConnections, sizeof( IceConn )*pThis->m_nConnections ));
        pThis->m_pFilehandles = static_cast<struct pollfd*>(rtl_reallocateMemory( pThis->m_pFilehandles, sizeof( struct pollfd )*(pThis->m_nConnections+1) ));
        pThis->m_pConnections[ pThis->m_nConnections-1 ]      = ice_conn;
        pThis->m_pFilehandles[ pThis->m_nConnections ].fd     = fd;
        pThis->m_pFilehandles[ pThis->m_nConnections ].events = POLLIN;
        if( pThis->m_nConnections == 1 )
        {
            SAL_INFO("vcl.sm.debug", "  First connection");
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
        SAL_INFO("vcl.sm.debug", "  closing");
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
            SAL_INFO("vcl.sm.debug", "  terminating ICEThread");
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

    SAL_INFO( "vcl.sm.debug", "  ICE connection     on " << IceConnectionNumber( ice_conn ) );
    SAL_INFO( "vcl.sm.debug", "  Display connection is " << ConnectionNumber( vcl_sal::getSalDisplay(GetGenericData())->GetDisplay() ) );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
