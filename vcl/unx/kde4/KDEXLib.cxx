/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include "VCLKDEApplication.hxx"

#include "KDE4FilePicker.hxx"
#include "KDESalInstance.hxx"

#include <kapplication.h>
#include <klocale.h>
#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <kstartupinfo.h>
#include <qabstracteventdispatcher.h>
#include <qclipboard.h>
#include <qthread.h>

#include "KDEXLib.hxx"

#include <unx/i18n_im.hxx>
#include <unx/i18n_xkb.hxx>

#include <unx/saldata.hxx>
#include <osl/process.h>

#include "KDESalDisplay.hxx"

#include <stdio.h>

#include <config_kde4.h>

#if QT_VERSION >= QT_VERSION_CHECK( 4, 9, 0 )
#define QT_UNIX_EVENT_LOOP_SUPPORT 1
#else
#define QT_UNIX_EVENT_LOOP_SUPPORT 0
#endif

#if KDE_HAVE_GLIB
#define GLIB_EVENT_LOOP_SUPPORT 1
#else
#define GLIB_EVENT_LOOP_SUPPORT 0
#endif

#if GLIB_EVENT_LOOP_SUPPORT
#include <glib-2.0/glib.h>
#endif

KDEXLib::KDEXLib() :
    SalXLib(),  m_bStartupDone(false), m_pApplication(0),
    m_pFreeCmdLineArgs(0), m_pAppCmdLineArgs(0), m_nFakeCmdLineArgs( 0 ),
    eventLoopType( LibreOfficeEventLoop )
{
    
    connect( &timeoutTimer, SIGNAL( timeout()), this, SLOT( timeoutActivated()));
    connect( &userEventTimer, SIGNAL( timeout()), this, SLOT( userEventActivated()));

    
    
    connect( this, SIGNAL( startTimeoutTimerSignal()), this, SLOT( startTimeoutTimer()), Qt::QueuedConnection );
    connect( this, SIGNAL( startUserEventTimerSignal()), this, SLOT( startUserEventTimer()), Qt::QueuedConnection );

    
    
    connect( this, SIGNAL( processYieldSignal( bool, bool )), this, SLOT( processYield( bool, bool )),
        Qt::BlockingQueuedConnection );

    
    
    connect( this, SIGNAL( createFilePickerSignal( const com::sun::star::uno::Reference<
                                                   com::sun::star::uno::XComponentContext >&) ),
             this, SLOT( createFilePicker( const com::sun::star::uno::Reference<
                                                 com::sun::star::uno::XComponentContext >&) ),
             Qt::BlockingQueuedConnection );
}

KDEXLib::~KDEXLib()
{
    delete m_pApplication;

    
    for( int i = 0; i < m_nFakeCmdLineArgs; i++ )
    {
        free( m_pFreeCmdLineArgs[i] );
    }

    delete [] m_pFreeCmdLineArgs;
    delete [] m_pAppCmdLineArgs;
}

void KDEXLib::Init()
{
    SalI18N_InputMethod* pInputMethod = new SalI18N_InputMethod;
    pInputMethod->SetLocale();
    XrmInitialize();

    KAboutData *kAboutData = new KAboutData( "LibreOffice",
            "kdelibs4",
            ki18n( "LibreOffice" ),
            "3.6.0",
            ki18n( "LibreOffice with KDE Native Widget Support." ),
            KAboutData::License_File,
            ki18n("Copyright (c) 2000, 2014 LibreOffice contributors" ),
            ki18n( "LibreOffice is an office suite.\n" ),
            "http:
            "libreoffice@lists.freedesktop.org");

    kAboutData->addAuthor( ki18n( "Jan Holesovsky" ),
            ki18n( "Original author and maintainer of the KDE NWF." ),
            "kendy@artax.karlin.mff.cuni.cz",
            "http:
    kAboutData->addAuthor( ki18n("Roman Shtylman"),
            ki18n( "Porting to KDE 4." ),
            "shtylman@gmail.com", "http:
    kAboutData->addAuthor( ki18n("Eric Bischoff"),
            ki18n( "Accessibility fixes, porting to KDE 4." ),
            "bischoff@kde.org" );

    m_nFakeCmdLineArgs = 2;
    sal_uInt16 nIdx;

    int nParams = osl_getCommandArgCount();
    OString aDisplay;
    OUString aParam, aBin;

    for ( nIdx = 0; nIdx < nParams; ++nIdx )
    {
        osl_getCommandArg( nIdx, &aParam.pData );
        if ( !m_pFreeCmdLineArgs && aParam == "-display" && nIdx + 1 < nParams )
        {
            osl_getCommandArg( nIdx + 1, &aParam.pData );
            aDisplay = OUStringToOString( aParam, osl_getThreadTextEncoding() );

            m_pFreeCmdLineArgs = new char*[ m_nFakeCmdLineArgs + 2 ];
            m_pFreeCmdLineArgs[ m_nFakeCmdLineArgs + 0 ] = strdup( "-display" );
            m_pFreeCmdLineArgs[ m_nFakeCmdLineArgs + 1 ] = strdup( aDisplay.getStr() );
            m_nFakeCmdLineArgs += 2;
        }
    }
    if ( !m_pFreeCmdLineArgs )
        m_pFreeCmdLineArgs = new char*[ m_nFakeCmdLineArgs ];

    osl_getExecutableFile( &aParam.pData );
    osl_getSystemPathFromFileURL( aParam.pData, &aBin.pData );
    OString aExec = OUStringToOString( aBin, osl_getThreadTextEncoding() );
    m_pFreeCmdLineArgs[0] = strdup( aExec.getStr() );
    m_pFreeCmdLineArgs[1] = strdup( "--nocrashhandler" );

    
    
    
    m_pAppCmdLineArgs = new char*[ m_nFakeCmdLineArgs ];
    for( int i = 0; i < m_nFakeCmdLineArgs; i++ )
        m_pAppCmdLineArgs[i] = m_pFreeCmdLineArgs[i];

    KCmdLineArgs::init( m_nFakeCmdLineArgs, m_pAppCmdLineArgs, kAboutData );

    m_pApplication = new VCLKDEApplication();
    kapp->disableSessionManagement();
    KApplication::setQuitOnLastWindowClosed(false);
    setupEventLoop();

    Display* pDisp = QX11Info::display();
    SalKDEDisplay *pSalDisplay = new SalKDEDisplay(pDisp);

    pInputMethod->CreateMethod( pDisp );
    pSalDisplay->SetupInput( pInputMethod );
}








#if GLIB_EVENT_LOOP_SUPPORT
static GPollFunc old_gpoll = NULL;
static gint gpoll_wrapper( GPollFD*, guint, gint );
#endif
#if QT_UNIX_EVENT_LOOP_SUPPORT
static int (*qt_select)(int nfds, fd_set *fdread, fd_set *fdwrite, fd_set *fdexcept,
   const struct timeval *orig_timeout);
static int lo_select(int nfds, fd_set *fdread, fd_set *fdwrite, fd_set *fdexcept,
   const struct timeval *orig_timeout);
#endif

static bool ( *old_qt_event_filter )( void* );
static bool qt_event_filter( void* m )
{
    if( old_qt_event_filter != NULL && old_qt_event_filter( m ))
        return true;
    if( SalKDEDisplay::self() && SalKDEDisplay::self()->checkDirectInputEvent( static_cast< XEvent* >( m )))
        return true;
    return false;
}

void KDEXLib::setupEventLoop()
{
    old_qt_event_filter = QAbstractEventDispatcher::instance()->setEventFilter( qt_event_filter );
#if GLIB_EVENT_LOOP_SUPPORT





#if QT_VERSION >= QT_VERSION_CHECK( 4, 8, 0 )
    if( QAbstractEventDispatcher::instance()->inherits( "QEventDispatcherGlib" ))
    {
        eventLoopType = GlibEventLoop;
        old_gpoll = g_main_context_get_poll_func( NULL );
        g_main_context_set_poll_func( NULL, gpoll_wrapper );
        return;
    }
#endif
#endif
#if QT_UNIX_EVENT_LOOP_SUPPORT






    if( QAbstractEventDispatcher::instance()->inherits( "QEventDispatcherUNIX" ))
    {
        eventLoopType = QtUnixEventLoop;
        QInternal::callFunction( QInternal::GetUnixSelectFunction, reinterpret_cast< void** >( &qt_select ));
        QInternal::callFunction( QInternal::SetUnixSelectFunction, reinterpret_cast< void** >( lo_select ));
        return;
    }
#endif
}

#if GLIB_EVENT_LOOP_SUPPORT
gint gpoll_wrapper( GPollFD* ufds, guint nfds, gint timeout )
{
    SalYieldMutexReleaser release; 
    return old_gpoll( ufds, nfds, timeout );
}
#endif

#if QT_UNIX_EVENT_LOOP_SUPPORT
int lo_select(int nfds, fd_set *fdread, fd_set *fdwrite, fd_set *fdexcept,
   const struct timeval *orig_timeout)
{
    SalYieldMutexReleaser release; 
    return qt_select( nfds, fdread, fdwrite, fdexcept, orig_timeout );
}
#endif

void KDEXLib::Insert( int fd, void* data, YieldFunc pending, YieldFunc queued, YieldFunc handle )
{
    if( eventLoopType == LibreOfficeEventLoop )
        return SalXLib::Insert( fd, data, pending, queued, handle );
    SocketData sdata;
    sdata.data = data;
    sdata.pending = pending;
    sdata.queued = queued;
    sdata.handle = handle;
    
    sdata.notifier = new QSocketNotifier( fd, QSocketNotifier::Read, qApp );
    connect( sdata.notifier, SIGNAL( activated( int )), this, SLOT( socketNotifierActivated( int )));
    socketData[ fd ] = sdata;
}

void KDEXLib::Remove( int fd )
{
    if( eventLoopType == LibreOfficeEventLoop )
        return SalXLib::Remove( fd );
    SocketData sdata = socketData.take( fd );
    delete sdata.notifier;
}

void KDEXLib::socketNotifierActivated( int fd )
{
    const SocketData& sdata = socketData[ fd ];
    sdata.handle( fd, sdata.data );
}

void KDEXLib::Yield( bool bWait, bool bHandleAllCurrentEvents )
{
    
    static int loop_depth = 0;

    if( eventLoopType == LibreOfficeEventLoop )
    {
        if( qApp->thread() == QThread::currentThread())
        {
            
            
            processYield( false, bHandleAllCurrentEvents );
        }
        return SalXLib::Yield( bWait, bHandleAllCurrentEvents );
    }

    
    
    if( qApp->thread() == QThread::currentThread()) {
        
        if (loop_depth > 0)
            SalYieldMutexReleaser aReleaser;
        loop_depth++;
        processYield( bWait, bHandleAllCurrentEvents );
        loop_depth--;
    }
    else {
        
        
        SalYieldMutexReleaser aReleaser;

        
        
        Q_EMIT processYieldSignal( bWait, bHandleAllCurrentEvents );
    }
}

void KDEXLib::processYield( bool bWait, bool bHandleAllCurrentEvents )
{
    QAbstractEventDispatcher* dispatcher = QAbstractEventDispatcher::instance( qApp->thread());
    bool wasEvent = false;
    for( int cnt = bHandleAllCurrentEvents ? 100 : 1;
         cnt > 0;
         --cnt )
    {
        if( !dispatcher->processEvents( QEventLoop::AllEvents ))
            break;
        wasEvent = true;
    }
    if( bWait && !wasEvent )
        dispatcher->processEvents( QEventLoop::WaitForMoreEvents );
}

void KDEXLib::StartTimer( sal_uLong nMS )
{
    if( eventLoopType == LibreOfficeEventLoop )
        return SalXLib::StartTimer( nMS );
    timeoutTimer.setInterval( nMS );
    
    if( qApp->thread() == QThread::currentThread())
        startTimeoutTimer();
    else
        Q_EMIT startTimeoutTimerSignal();
}

void KDEXLib::startTimeoutTimer()
{
    timeoutTimer.start();
}

void KDEXLib::StopTimer()
{
    if( eventLoopType == LibreOfficeEventLoop )
        return SalXLib::StopTimer();
    timeoutTimer.stop();
}

void KDEXLib::timeoutActivated()
{
    X11SalData *pData = (X11SalData*)ImplGetSVData()->mpSalData;
    pData->Timeout();
    
}

void KDEXLib::Wakeup()
{
    if( eventLoopType == LibreOfficeEventLoop )
        return SalXLib::Wakeup();
    QAbstractEventDispatcher::instance( qApp->thread())->wakeUp(); 
}

void KDEXLib::PostUserEvent()
{
    if( eventLoopType == LibreOfficeEventLoop )
        return SalXLib::PostUserEvent();
    if( qApp->thread() == QThread::currentThread())
        startUserEventTimer();
    else
        Q_EMIT startUserEventTimerSignal();
}

void KDEXLib::startUserEventTimer()
{
    userEventTimer.start( 0 );
}

void KDEXLib::userEventActivated()
{
    if( ! SalKDEDisplay::self()->HasUserEvents() )
        userEventTimer.stop();
    SalKDEDisplay::self()->DispatchInternalEvent();
    
}

void KDEXLib::doStartup()
{
    if( ! m_bStartupDone )
    {
        KStartupInfo::appStarted();
        m_bStartupDone = true;
        SAL_INFO( "vcl.kde4", "called KStartupInfo::appStarted()" );
    }
}

using namespace com::sun::star;

uno::Reference< ui::dialogs::XFilePicker2 > KDEXLib::createFilePicker(
        const uno::Reference< uno::XComponentContext >& xMSF )
{
    if( qApp->thread() != QThread::currentThread()) {
        SalYieldMutexReleaser aReleaser;
        return Q_EMIT createFilePickerSignal( xMSF );
    }
    return uno::Reference< ui::dialogs::XFilePicker2 >( new KDE4FilePicker( xMSF ) );
}

#include "KDEXLib.moc"

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
