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

#include "VCLKDEApplication.hxx"

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

#include <config_kde4.h>

#if KDE_HAVE_GLIB
#include "KDE4FilePicker.hxx"
#include "tst_exclude_socket_notifiers.moc"
#include "tst_exclude_posted_events.moc"
#endif

KDEXLib::KDEXLib() :
    SalXLib(),  m_bStartupDone(false),
    m_pFreeCmdLineArgs(nullptr), m_pAppCmdLineArgs(nullptr), m_nFakeCmdLineArgs( 0 ),
    m_frameWidth( -1 ), m_isGlibEventLoopType(false),
    m_allowKdeDialogs(false), blockIdleTimeout(false)
{
    // the timers created here means they belong to the main thread.
    // As the timeoutTimer runs the LO event queue, which may block on a dialog,
    // the timer has to use a Qt::QueuedConnection, otherwise the nested event
    // loop will detect the blocking timer and drop it from the polling
    // freezing LO X11 processing.
    connect( &timeoutTimer, SIGNAL( timeout()), this, SLOT( timeoutActivated()), Qt::QueuedConnection );
    connect( &userEventTimer, SIGNAL( timeout()), this, SLOT( userEventActivated()), Qt::QueuedConnection );

    // QTimer::start() can be called only in its (here main) thread, so this will
    // forward between threads if needed
    connect( this, SIGNAL( startTimeoutTimerSignal()), this, SLOT( startTimeoutTimer()), Qt::QueuedConnection );
    connect( this, SIGNAL( startUserEventTimerSignal()), this, SLOT( startUserEventTimer()), Qt::QueuedConnection );

    // this one needs to be blocking, so that the handling in main thread is processed before
    // the thread emitting the signal continues
    connect( this, SIGNAL( processYieldSignal( bool, bool )), this, SLOT( processYield( bool, bool )),
        Qt::BlockingQueuedConnection );

    // Create the File picker in the main / GUI thread and block the calling thread until
    // the FilePicker is created.
    connect( this, SIGNAL( createFilePickerSignal( const css::uno::Reference< css::uno::XComponentContext >&) ),
             this, SLOT( createFilePicker( const css::uno::Reference< css::uno::XComponentContext >&) ),
             Qt::BlockingQueuedConnection );

    connect( this, SIGNAL( getFrameWidthSignal() ),
             this, SLOT( getFrameWidth() ), Qt::BlockingQueuedConnection );
}

KDEXLib::~KDEXLib()
{

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

    KAboutData *kAboutData = new KAboutData( "LibreOffice",
            "kdelibs4",
            ki18n( "LibreOffice" ),
            "3.6.0",
            ki18n( "LibreOffice with KDE Native Widget Support." ),
            KAboutData::License_File,
            ki18n("Copyright (c) 2000, 2014 LibreOffice contributors" ),
            ki18n( "LibreOffice is an office suite.\n" ),
            "http://libreoffice.org",
            "libreoffice@lists.freedesktop.org");

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

    // make a copy of the string list for freeing it since
    // KApplication manipulates the pointers inside the argument vector
    // note: KApplication bad !
    m_pAppCmdLineArgs = new char*[ m_nFakeCmdLineArgs ];
    for( int i = 0; i < m_nFakeCmdLineArgs; i++ )
        m_pAppCmdLineArgs[i] = m_pFreeCmdLineArgs[i];

    KCmdLineArgs::init( m_nFakeCmdLineArgs, m_pAppCmdLineArgs, kAboutData );

    // LO does its own session management, so prevent KDE/Qt from interfering
    // (QApplication::disableSessionManagement(false) wouldn't quite do,
    // since that still actually connects to the session manager, it just
    // won't save the application data on session shutdown).
    char* session_manager = nullptr;
    if( getenv( "SESSION_MANAGER" ) != nullptr )
    {
        session_manager = strdup( getenv( "SESSION_MANAGER" ));
        unsetenv( "SESSION_MANAGER" );
    }
    m_pApplication.reset( new VCLKDEApplication() );
    if( session_manager != nullptr )
    {
        // coverity[tainted_string] - trusted source for setenv
        setenv( "SESSION_MANAGER", session_manager, 1 );
        free( session_manager );
    }

    KApplication::setQuitOnLastWindowClosed(false);

#if KDE_HAVE_GLIB
    m_isGlibEventLoopType = QAbstractEventDispatcher::instance()->inherits( "QEventDispatcherGlib" );
    // Using KDE dialogs (and their nested event loops) works only with a proper event loop integration
    // that will release SolarMutex when waiting for more events.
    // Moreover there are bugs in Qt event loop code that allow QClipboard recursing because the event
    // loop processes also events that it should not at that point, so no dialogs in that case either.
    // https://bugreports.qt-project.org/browse/QTBUG-37380
    // https://bugreports.qt-project.org/browse/QTBUG-34614
    if (m_isGlibEventLoopType && (0 == tst_processEventsExcludeSocket()) && tst_excludePostedEvents() == 0 )
        m_allowKdeDialogs = true;
#endif

    setupEventLoop();

    Display* pDisp = QX11Info::display();
    SalKDEDisplay *pSalDisplay = new SalKDEDisplay(pDisp);

    pInputMethod->CreateMethod( pDisp );
    pSalDisplay->SetupInput( pInputMethod );
}

// When we use Qt event loop, it can actually use its own event loop handling, or wrap
// the Glib event loop (the latter is the default is Qt is built with Glib support
// and $QT_NO_GLIB is not set). We mostly do not care which one it is, as QSocketNotifier's
// and QTimer's can handle it transparently, but it matters for the SolarMutex, which
// needs to be unlocked shortly before entering the main sleep (e.g. select()) and locked
// immediately after. So we need to know which event loop implementation is used and
// hook accordingly.
#if KDE_HAVE_GLIB
#include <glib.h>

static GPollFunc old_gpoll = nullptr;

static gint gpoll_wrapper( GPollFD* ufds, guint nfds, gint timeout )
{
    SalYieldMutexReleaser release; // release YieldMutex (and re-acquire at block end)
    return old_gpoll( ufds, nfds, timeout );
}
#endif

static bool ( *old_qt_event_filter )( void* );
static bool qt_event_filter( void* m )
{
    if( old_qt_event_filter != nullptr && old_qt_event_filter( m ))
        return true;
    if( SalKDEDisplay::self() && SalKDEDisplay::self()->checkDirectInputEvent( static_cast< XEvent* >( m )))
        return true;
    return false;
}

void KDEXLib::setupEventLoop()
{
    old_qt_event_filter = QAbstractEventDispatcher::instance()->setEventFilter( qt_event_filter );
#if KDE_HAVE_GLIB
    if( m_isGlibEventLoopType )
    {
        old_gpoll = g_main_context_get_poll_func( nullptr );
        g_main_context_set_poll_func( nullptr, gpoll_wrapper );
        if( m_allowKdeDialogs )
            QApplication::clipboard()->setProperty( "useEventLoopWhenWaiting", true );
        return;
    }
#endif
}

void KDEXLib::Insert( int fd, void* data, YieldFunc pending, YieldFunc queued, YieldFunc handle )
{
    if( !m_isGlibEventLoopType )
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
    if( !m_isGlibEventLoopType )
        return SalXLib::Remove( fd );
    SocketData sdata = socketData.take( fd );// according to SalXLib::Remove() this should be safe
    delete sdata.notifier;
}

void KDEXLib::socketNotifierActivated( int fd )
{
    const SocketData& sdata = socketData[ fd ];
    sdata.handle( fd, sdata.data );
}

SalYieldResult KDEXLib::Yield( bool bWait, bool bHandleAllCurrentEvents )
{
    if( !m_isGlibEventLoopType )
    {
        bool wasEvent = false;
        if( qApp->thread() == QThread::currentThread())
        {
            // even if we use the LO event loop, still process Qt's events,
            // otherwise they can remain unhandled for quite a long while
            wasEvent = processYield( false, bHandleAllCurrentEvents );
        }
        SalYieldResult aResult = SalXLib::Yield(bWait, bHandleAllCurrentEvents);
        return (aResult == SalYieldResult::EVENT || wasEvent) ?
            SalYieldResult::EVENT : SalYieldResult::TIMEOUT;
    }
    // if we are the main thread (which is where the event processing is done),
    // good, just do it
    if( qApp->thread() == QThread::currentThread())
    {
        return processYield( bWait, bHandleAllCurrentEvents );
    }
    else
    {
        // we were called from another thread;
        // release the yield lock to prevent deadlock with the main thread
        // (it's ok to release it here, since even normal processYield() would
        // temporarily do it while checking for new events)
        SalYieldMutexReleaser aReleaser;
        Q_EMIT processYieldSignal( bWait, bHandleAllCurrentEvents );
        return SalYieldResult::TIMEOUT;
    }
}

SalYieldResult KDEXLib::processYield( bool bWait, bool bHandleAllCurrentEvents )
{
    blockIdleTimeout = !bWait;
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
    blockIdleTimeout = false;
    return wasEvent ? SalYieldResult::EVENT
                    : SalYieldResult::TIMEOUT;
}

void KDEXLib::StartTimer( sal_uLong nMS )
{
    if( !m_isGlibEventLoopType )
        return SalXLib::StartTimer( nMS );
    timeoutTimer.setInterval( nMS );
    // QTimer's can be started only in their thread (main thread here)
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
    if( !m_isGlibEventLoopType )
        return SalXLib::StopTimer();
    timeoutTimer.stop();
}

void KDEXLib::timeoutActivated()
{
    // HACK? Always process posted events before timer timeouts.
    // There are places that may watch both both (for example, there's a posted
    // event about change of the current active window and there's a timeout
    // event informing that a document has finished loading). This is of course
    // racy, but both generic and gtk event loops manage to deliver posted events
    // first, so it's at least consistent, and it probably kind of makes at least
    // some sense (timeouts should be more ok to wait and be triggered somewhen).
    while( SalKDEDisplay::self()->HasUserEvents() )
        SalKDEDisplay::self()->DispatchInternalEvent();

    // QGuiEventDispatcherGlib makes glib watch also X11 fd, but its hasPendingEvents()
    // doesn't check X11, so explicitly check XPending() here.
    bool idle = QApplication::hasPendingEvents() && !blockIdleTimeout && !XPending( QX11Info::display());
    X11SalData::Timeout( idle );
    // QTimer is not single shot, so will be restarted immediately
}

void KDEXLib::Wakeup()
{
    if( !m_isGlibEventLoopType )
        return SalXLib::Wakeup();
    QAbstractEventDispatcher::instance( qApp->thread())->wakeUp(); // main thread event loop
}

void KDEXLib::PostUserEvent()
{
    if( !m_isGlibEventLoopType )
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
    // QTimer is not single shot, so will be restarted immediately
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
#if KDE_HAVE_GLIB
    if( qApp->thread() != QThread::currentThread()) {
        SalYieldMutexReleaser aReleaser;
        return Q_EMIT createFilePickerSignal( xMSF );
    }
    return uno::Reference< ui::dialogs::XFilePicker2 >( new KDE4FilePicker( xMSF ) );
#else
    return NULL;
#endif
}

#include <qframe.h>

int KDEXLib::getFrameWidth()
{
    if( m_frameWidth >= 0 )
        return m_frameWidth;
    if( qApp->thread() != QThread::currentThread()) {
        SalYieldMutexReleaser aReleaser;
        return Q_EMIT getFrameWidthSignal();
    }

    // fill in a default
    QFrame aFrame( nullptr );
    aFrame.setFrameStyle( QFrame::StyledPanel | QFrame::Sunken );
    aFrame.ensurePolished();
    m_frameWidth = aFrame.frameWidth();
    return m_frameWidth;
}

#include "KDEXLib.moc"

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
