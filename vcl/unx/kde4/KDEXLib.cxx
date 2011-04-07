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

#include "VCLKDEApplication.hxx"

#define Region QtXRegion

#include <kapplication.h>
#include <klocale.h>
#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <kstartupinfo.h>
#include <qabstracteventdispatcher.h>
#include <qclipboard.h>
#include <qthread.h>

#undef Region

#include "KDEXLib.hxx"

#include <i18n_im.hxx>
#include <i18n_xkb.hxx>

#include <saldata.hxx>
#include <osl/process.h>

#include "KDESalDisplay.hxx"

#if OSL_DEBUG_LEVEL > 1
#include <stdio.h>
#endif

#include <stdio.h>

#if QT_VERSION >= QT_VERSION_CHECK( 4, 8, 0 )
#define QT_UNIX_EVENT_LOOP_SUPPORT
#ifdef KDE_HAVE_GLIB
#define GLIB_EVENT_LOOP_SUPPORT
#endif
#endif

#ifdef GLIB_EVENT_LOOP_SUPPORT
#include <glib-2.0/glib.h>
#endif

KDEXLib::KDEXLib() :
    SalXLib(),  m_bStartupDone(false), m_pApplication(0),
    m_pFreeCmdLineArgs(0), m_pAppCmdLineArgs(0), m_nFakeCmdLineArgs( 0 ),
    eventLoopType( LibreOfficeEventLoop )
{
    // the timers created here means they belong to the main thread
    connect( &timeoutTimer, SIGNAL( timeout()), this, SLOT( timeoutActivated()));
    connect( &userEventTimer, SIGNAL( timeout()), this, SLOT( userEventActivated()));
    // QTimer::start() can be called only in its (here main) thread, so this will
    // forward between threads if needed
    connect( this, SIGNAL( startTimeoutTimerSignal()), this, SLOT( startTimeoutTimer()), Qt::QueuedConnection );
    connect( this, SIGNAL( startUserEventTimerSignal()), this, SLOT( startUserEventTimer()), Qt::QueuedConnection );
    // this one needs to be blocking, so that the handling in main thread is processed before
    // the thread emitting the signal continues
    connect( this, SIGNAL( processYieldSignal( bool, bool )), this, SLOT( processYield( bool, bool )),
        Qt::BlockingQueuedConnection );
}

KDEXLib::~KDEXLib()
{
    delete m_pApplication;

    // free the faked cmdline arguments no longer needed by KApplication
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

    KAboutData *kAboutData = new KAboutData("LibreOffice",
            "kdelibs4",
            ki18n( "LibreOffice" ),
            "3.3.0",
            ki18n( "LibreOffice with KDE Native Widget Support." ),
            KAboutData::License_LGPL,
            ki18n( "Copyright (c) 2003, 2004, 2005, 2006, 2007, 2008, 2009 Novell, Inc"),
            ki18n( "LibreOffice is an office suite.\n" ),
            "http://libreoffice.org",
            "libreoffice@lists.freedesktop.org" );

    kAboutData->addAuthor( ki18n( "Jan Holesovsky" ),
            ki18n( "Original author and maintainer of the KDE NWF." ),
            "kendy@artax.karlin.mff.cuni.cz",
            "http://artax.karlin.mff.cuni.cz/~kendy" );
    kAboutData->addAuthor( ki18n("Roman Shtylman"),
            ki18n( "Porting to KDE 4." ),
            "shtylman@gmail.com", "http://shtylman.com" );
    kAboutData->addAuthor( ki18n("Eric Bischoff"),
            ki18n( "Accessibility fixes, porting to KDE 4." ),
            "bischoff@kde.org" );

    //kAboutData->setProgramIconName("OpenOffice");

    m_nFakeCmdLineArgs = 2;
    sal_uInt16 nIdx;

    int nParams = osl_getCommandArgCount();
    rtl::OString aDisplay;
    rtl::OUString aParam, aBin;

    for ( nIdx = 0; nIdx < nParams; ++nIdx )
    {
        osl_getCommandArg( nIdx, &aParam.pData );
        if ( !m_pFreeCmdLineArgs && aParam.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "-display" ) ) && nIdx + 1 < nParams )
        {
            osl_getCommandArg( nIdx + 1, &aParam.pData );
            aDisplay = rtl::OUStringToOString( aParam, osl_getThreadTextEncoding() );

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
    rtl::OString aExec = rtl::OUStringToOString( aBin, osl_getThreadTextEncoding() );
    m_pFreeCmdLineArgs[0] = strdup( aExec.getStr() );
    m_pFreeCmdLineArgs[1] = strdup( "--nocrashhandler" );

    // make a copy of the string list for freeing it since
    // KApplication manipulates the pointers inside the argument vector
    // note: KApplication bad !
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
    pInputMethod->AddConnectionWatch( pDisp, (void*)this );
    pSalDisplay->SetInputMethod( pInputMethod );

    PushXErrorLevel( true );
    SalI18N_KeyboardExtension *pKbdExtension = new SalI18N_KeyboardExtension( pDisp );
    XSync( pDisp, False );

    pKbdExtension->UseExtension( ! HasXErrorOccurred() );
    PopXErrorLevel();

    pSalDisplay->SetKbdExtension( pKbdExtension );
}

// When we use Qt event loop, it can actually use its own event loop handling, or wrap
// the Glib event loop (the latter is the default is Qt is built with Glib support
// and $QT_NO_GLIB is not set). We mostly do not care which one it is, as QSocketNotifier's
// and QTimer's can handle it transparently, but it matters for the SolarMutex, which
// needs to be unlocked shortly before entering the main sleep (e.g. select()) and locked
// immediatelly after. So we need to know which event loop implementation is used and
// hook accordingly.
#ifdef GLIB_EVENT_LOOP_SUPPORT
static GPollFunc old_gpoll = NULL;
static gint gpoll_wrapper( GPollFD*, guint, gint );
#endif
#ifdef QT_UNIX_EVENT_LOOP_SUPPORT
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
#ifdef GLIB_EVENT_LOOP_SUPPORT
// Glib is simple, it has g_main_context_set_poll_func() for wrapping the sleep call.
// The catch is that Qt has a bug that allows triggering timers even when they should
// not be, leading to crashes caused by QClipboard re-entering the event loop.
// (http://bugreports.qt.nokia.com/browse/QTBUG-14461), so enable only with Qt>=4.8.0,
// where it is(?) fixed.
    if( QAbstractEventDispatcher::instance()->inherits( "QEventDispatcherGlib" ))
    {
        eventLoopType = GlibEventLoop;
        old_gpoll = g_main_context_get_poll_func( NULL );
        g_main_context_set_poll_func( NULL, gpoll_wrapper );
        // set QClipboard to use event loop, otherwise the main thread will hold
        // SolarMutex locked, which will prevent the clipboard thread from answering
        m_pApplication->clipboard()->setProperty( "useEventLoopWhenWaiting", true );
        return;
    }
#endif
#ifdef QT_UNIX_EVENT_LOOP_SUPPORT
// When Qt does not use Glib support, it uses its own Unix event dispatcher.
// That one has aboutToBlock() and awake() signals, but they are broken (either
// functionality or semantics), as e.g. awake() is not emitted right after the dispatcher
// is woken up from sleep again, but only later (which is too late for re-acquiring SolarMutex).
// This should be fixed with Qt-4.8.0 (?) where support for adding custom select() function
// has been added too (http://bugreports.qt.nokia.com/browse/QTBUG-16934).
    if( QAbstractEventDispatcher::instance()->inherits( "QEventDispatcherUNIX" ))
    {
        eventLoopType = QtUnixEventLoop;
        QInternal::callFunction( QInternal::GetUnixSelectFunction, reinterpret_cast< void** >( &qt_select ));
        QInternal::callFunction( QInternal::SetUnixSelectFunction, reinterpret_cast< void** >( lo_select ));
        // set QClipboard to use event loop, otherwise the main thread will hold
        // SolarMutex locked, which will prevent the clipboard thread from answering
        m_pApplication->clipboard()->setProperty( "useEventLoopWhenWaiting", true );
        return;
    }
#endif
}

#ifdef GLIB_EVENT_LOOP_SUPPORT
gint gpoll_wrapper( GPollFD* ufds, guint nfds, gint timeout )
{
    YieldMutexReleaser release; // release YieldMutex (and re-acquire at block end)
    return old_gpoll( ufds, nfds, timeout );
}
#endif

#ifdef QT_UNIX_EVENT_LOOP_SUPPORT
int lo_select(int nfds, fd_set *fdread, fd_set *fdwrite, fd_set *fdexcept,
   const struct timeval *orig_timeout)
{
    YieldMutexReleaser release; // release YieldMutex (and re-acquire at block end)
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
    // qApp as parent to make sure it uses the main thread event loop
    sdata.notifier = new QSocketNotifier( fd, QSocketNotifier::Read, qApp );
    connect( sdata.notifier, SIGNAL( activated( int )), this, SLOT( socketNotifierActivated( int )));
    socketData[ fd ] = sdata;
}

void KDEXLib::Remove( int fd )
{
    if( eventLoopType == LibreOfficeEventLoop )
        return SalXLib::Remove( fd );
    SocketData sdata = socketData.take( fd );// according to SalXLib::Remove() this should be safe
    delete sdata.notifier;
}

void KDEXLib::socketNotifierActivated( int fd )
{
    const SocketData& sdata = socketData[ fd ];
    sdata.handle( fd, sdata.data );
}

void KDEXLib::Yield( bool bWait, bool bHandleAllCurrentEvents )
{
    if( eventLoopType == LibreOfficeEventLoop )
    {
        if( qApp->thread() == QThread::currentThread())
        {
            // even if we use the LO event loop, still process Qt's events,
            // otherwise they can remain unhandled for quite a long while
            processYield( false, bHandleAllCurrentEvents );
        }
        return SalXLib::Yield( bWait, bHandleAllCurrentEvents );
    }
    // if we are the main thread (which is where the event processing is done),
    // good, just do it
    if( qApp->thread() == QThread::currentThread())
        processYield( bWait, bHandleAllCurrentEvents );
    else
    { // if this deadlocks, event processing needs to go into a separate thread
      // or some other solution needs to be found
        emit processYieldSignal( bWait, bHandleAllCurrentEvents );
    }
}

bool KDEXLib::processYield( bool bWait, bool bHandleAllCurrentEvents )
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
        wasEvent = dispatcher->processEvents( QEventLoop::WaitForMoreEvents );
    return wasEvent;
}

void KDEXLib::StartTimer( sal_uLong nMS )
{
    if( eventLoopType == LibreOfficeEventLoop )
        return SalXLib::StartTimer( nMS );
    timeoutTimer.setInterval( nMS );
    // QTimer's can be started only in their thread (main thread here)
    if( qApp->thread() == QThread::currentThread())
        startTimeoutTimer();
    else
        emit startTimeoutTimerSignal();
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
    GetX11SalData()->Timeout();
    // QTimer is not single shot, so will be restarted immediatelly
}

void KDEXLib::Wakeup()
{
    if( eventLoopType == LibreOfficeEventLoop )
        return SalXLib::Wakeup();
    QAbstractEventDispatcher::instance( qApp->thread())->wakeUp(); // main thread event loop
}

void KDEXLib::PostUserEvent()
{
    if( eventLoopType == LibreOfficeEventLoop )
        return SalXLib::PostUserEvent();
    if( qApp->thread() == QThread::currentThread())
        startUserEventTimer();
    else
        emit startUserEventTimerSignal();
}

void KDEXLib::startUserEventTimer()
{
    userEventTimer.start( 0 );
}

void KDEXLib::userEventActivated()
{
    SalKDEDisplay::self()->EventGuardAcquire();
    if( SalKDEDisplay::self()->userEventsCount() <= 1 )
        userEventTimer.stop();
    SalKDEDisplay::self()->EventGuardRelease();
    SalKDEDisplay::self()->DispatchInternalEvent();
    // QTimer is not single shot, so will be restarted immediatelly
}

void KDEXLib::doStartup()
{
    if( ! m_bStartupDone )
    {
        KStartupInfo::appStarted();
        m_bStartupDone = true;
        #if OSL_DEBUG_LEVEL > 1
        fprintf( stderr, "called KStartupInfo::appStarted()\n" );
        #endif
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
