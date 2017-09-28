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

#include <QtCore/QAbstractEventDispatcher>
#include <QtCore/QThread>
#include <QtWidgets/QApplication>
#include <QtWidgets/QFrame>
#include <QtGui/QClipboard>
#include <QtGui/QFrame>
#include <QtX11Extras/QX11Info>


#include "config_kde5.h"

#include "KDE5XLib.hxx"
#include "VCLKDE5Application.hxx"
#include "KDE5SalInstance.hxx"

#include <KAboutData>
#include <KLocalizedString>

#include "unx/i18n_im.hxx"
#include "unx/i18n_xkb.hxx"
#include "unx/saldata.hxx"
#include <o3tl/make_unique.hxx>
#include "osl/process.h"

#include "KDE5SalDisplay.hxx"

#undef Bool

#if KF5_HAVE_GLIB
//#include "KDE4FilePicker.hxx"
#include "tst_exclude_socket_notifiers.moc"
#include "tst_exclude_posted_events.moc"
#endif

KDE5XLib::KDE5XLib() :
    SalXLib(),  m_bStartupDone(false),
    m_nFakeCmdLineArgs( 0 ),
    m_isGlibEventLoopType(false), m_allowKdeDialogs(false),
    m_timerEventId( -1 ), m_postUserEventId( -1 )
{
    m_timerEventId = QEvent::registerEventType();
    m_postUserEventId = QEvent::registerEventType();

    // the timers created here means they belong to the main thread.
    // As the timeoutTimer runs the LO event queue, which may block on a dialog,
    // the timer has to use a Qt::QueuedConnection, otherwise the nested event
    // loop will detect the blocking timer and drop it from the polling
    // freezing LO X11 processing.
    timeoutTimer.setSingleShot( true );
    connect( &timeoutTimer, SIGNAL( timeout()), this, SLOT( timeoutActivated()), Qt::QueuedConnection );

    // QTimer::start() can be called only in its (here main) thread, so this will
    // forward between threads if needed
    connect( this, SIGNAL( startTimeoutTimerSignal()), this, SLOT( startTimeoutTimer()), Qt::QueuedConnection );

    // this one needs to be blocking, so that the handling in main thread is processed before
    // the thread emitting the signal continues
    connect( this, SIGNAL( processYieldSignal( bool, bool )), this, SLOT( processYield( bool, bool )),
        Qt::BlockingQueuedConnection );

    // Create the File picker in the main / GUI thread and block the calling thread until
    // the FilePicker is created.
    connect( this, SIGNAL( createFilePickerSignal( const css::uno::Reference< css::uno::XComponentContext >&) ),
             this, SLOT( createFilePicker( const css::uno::Reference< css::uno::XComponentContext >&) ),
             Qt::BlockingQueuedConnection );
}

KDE5XLib::~KDE5XLib()
{

    // free the faked cmdline arguments no longer needed by KApplication
    for( int i = 0; i < m_nFakeCmdLineArgs; i++ )
    {
        free( m_pFreeCmdLineArgs[i] );
    }
}

void KDE5XLib::Init()
{
    m_pInputMethod = new SalI18N_InputMethod;
    m_pInputMethod->SetLocale();
    XrmInitialize();

    KAboutData *kAboutData = new KAboutData( I18N_NOOP("LibreOffice"),
            i18n( "LibreOffice" ),
            "5.1.0",
            i18n( "LibreOffice with KDE Native Widget Support." ),
            KAboutLicense::File,
            i18n("Copyright (c) 2000, 2014 LibreOffice contributors" ),
            i18n( "LibreOffice is an office suite.\n" ),
            "http://libreoffice.org",
            QLatin1String("libreoffice@lists.freedesktop.org"));

    kAboutData->addAuthor( i18n( "Jan Holesovsky" ),
            i18n( "Original author and maintainer of the KDE NWF." ),
            "kendy@artax.karlin.mff.cuni.cz",
            "http://artax.karlin.mff.cuni.cz/~kendy" );
    kAboutData->addAuthor( i18n("Roman Shtylman"),
            i18n( "Porting to KDE 4." ),
            "shtylman@gmail.com", "http://shtylman.com" );
    kAboutData->addAuthor( i18n("Eric Bischoff"),
            i18n( "Accessibility fixes, porting to KDE 4." ),
            "bischoff@kde.org" );

    m_nFakeCmdLineArgs = 2;

    sal_uInt32 nParams = osl_getCommandArgCount();
    OString aDisplay;
    OUString aParam, aBin;

    for ( sal_uInt32 nIdx = 0; nIdx < nParams; ++nIdx )
    {
        osl_getCommandArg( nIdx, &aParam.pData );
        if ( !m_pFreeCmdLineArgs && aParam == "-display" && nIdx + 1 < nParams )
        {
            osl_getCommandArg( nIdx + 1, &aParam.pData );
            aDisplay = OUStringToOString( aParam, osl_getThreadTextEncoding() );

            m_pFreeCmdLineArgs = o3tl::make_unique<char*[]>(m_nFakeCmdLineArgs + 2);
            m_pFreeCmdLineArgs[ m_nFakeCmdLineArgs + 0 ] = strdup( "-display" );
            m_pFreeCmdLineArgs[ m_nFakeCmdLineArgs + 1 ] = strdup( aDisplay.getStr() );
            m_nFakeCmdLineArgs += 2;
        }
    }
    if ( !m_pFreeCmdLineArgs )
        m_pFreeCmdLineArgs = o3tl::make_unique<char*[]>(m_nFakeCmdLineArgs);

    osl_getExecutableFile( &aParam.pData );
    osl_getSystemPathFromFileURL( aParam.pData, &aBin.pData );
    OString aExec = OUStringToOString( aBin, osl_getThreadTextEncoding() );
    m_pFreeCmdLineArgs[0] = strdup( aExec.getStr() );
    m_pFreeCmdLineArgs[1] = strdup( "--nocrashhandler" );

    // make a copy of the string list for freeing it since
    // KApplication manipulates the pointers inside the argument vector
    // note: KApplication bad !
    m_pAppCmdLineArgs = o3tl::make_unique<char*[]>(m_nFakeCmdLineArgs);
    for( int i = 0; i < m_nFakeCmdLineArgs; i++ )
        m_pAppCmdLineArgs[i] = m_pFreeCmdLineArgs[i];

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
    //m_pApplication.reset( new VCLKDEApplication() );
    if( session_manager != nullptr )
    {
        // coverity[tainted_string] - trusted source for setenv
        setenv( "SESSION_MANAGER", session_manager, 1 );
        free( session_manager );
    }

    //KApplication::setQuitOnLastWindowClosed(false);

#if KF5_HAVE_GLIB
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

    m_pDisplay = QX11Info::display();
}

// When we use Qt event loop, it can actually use its own event loop handling, or wrap
// the Glib event loop (the latter is the default is Qt is built with Glib support
// and $QT_NO_GLIB is not set). We mostly do not care which one it is, as QSocketNotifier's
// and QTimer's can handle it transparently, but it matters for the SolarMutex, which
// needs to be unlocked shortly before entering the main sleep (e.g. select()) and locked
// immediately after. So we need to know which event loop implementation is used and
// hook accordingly.
#if KF5_HAVE_GLIB
#include <glib.h>

static GPollFunc old_gpoll = nullptr;

static gint gpoll_wrapper( GPollFD* ufds, guint nfds, gint timeout )
{
    SalYieldMutexReleaser release; // release YieldMutex (and re-acquire at block end)
    return old_gpoll( ufds, nfds, timeout );
}
#endif

/*static bool ( *old_qt_event_filter )( void* );
static bool qt_event_filter( void* m )
{
    if( old_qt_event_filter != nullptr && old_qt_event_filter( m ))
        return true;
    if( SalKDE5Display::self() && SalKDE5Display::self()->checkDirectInputEvent( static_cast< XEvent* >( m )))
        return true;
    return false;
}*/

bool KDE5XLib::nativeEventFilter(const QByteArray &eventType, void *message, long *)
{
    if (eventType == "xcb_generic_event_t") {
        xcb_generic_event_t* ev = static_cast<xcb_generic_event_t *>(message);
        if( SalKDE5Display::self() && SalKDE5Display::self()->checkDirectInputEvent( ev ))
            return true;
    }
    return false;
}

void KDE5XLib::setupEventLoop()
{
    QAbstractEventDispatcher *qDispatcher = QAbstractEventDispatcher::instance();

    //qDispatcher->installNativeEventFilter( this );
#if KF5_HAVE_GLIB
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

void KDE5XLib::Insert( int fd, void* data, YieldFunc pending, YieldFunc queued, YieldFunc handle )
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

void KDE5XLib::Remove( int fd )
{
    if( !m_isGlibEventLoopType )
        return SalXLib::Remove( fd );
    SocketData sdata = socketData.take( fd );// according to SalXLib::Remove() this should be safe
    delete sdata.notifier;
}

void KDE5XLib::socketNotifierActivated( int fd )
{
    const SocketData& sdata = socketData[ fd ];
    sdata.handle( fd, sdata.data );
}

bool KDE5XLib::Yield( bool bWait, bool bHandleAllCurrentEvents )
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
        return SalXLib::Yield(bWait, bHandleAllCurrentEvents) || wasEvent;
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
        return false;
    }
}

/**
 * Quoting the Qt docs: [QAbstractEventDispatcher::processEvents] processes
 * pending events that match flags until there are no more events to process.
 */
bool KDE5XLib::processYield( bool bWait, bool )
{
    QAbstractEventDispatcher* dispatcher = QAbstractEventDispatcher::instance( qApp->thread());
    bool wasEvent = false;
    if ( bWait )
        wasEvent = dispatcher->processEvents( QEventLoop::WaitForMoreEvents );
    else
        wasEvent = dispatcher->processEvents( QEventLoop::AllEvents );
    return wasEvent;
}

void KDE5XLib::StartTimer( sal_uLong nMS )
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

void KDE5XLib::startTimeoutTimer()
{
    timeoutTimer.start();
}

void KDE5XLib::StopTimer()
{
    if( !m_isGlibEventLoopType )
        return SalXLib::StopTimer();
    timeoutTimer.stop();
}

void KDE5XLib::timeoutActivated()
{
    // don't potentially wait in timeout, as QTimer is non-recursive
    QApplication::postEvent(this, new QEvent(QEvent::Type( m_timerEventId )));
}

void KDE5XLib::customEvent(QEvent* e)
{
    if( e->type() == m_timerEventId )
        X11SalData::Timeout();
    else if( e->type() == m_postUserEventId )
        SalKDE5Display::self()->DispatchInternalEvent();
}

void KDE5XLib::Wakeup()
{
    if( !m_isGlibEventLoopType )
        return SalXLib::Wakeup();
    QAbstractEventDispatcher::instance( qApp->thread())->wakeUp(); // main thread event loop
}

void KDE5XLib::PostUserEvent()
{
    if( !m_isGlibEventLoopType )
        return SalXLib::PostUserEvent();
    QApplication::postEvent(this, new QEvent(QEvent::Type( m_postUserEventId )));
}

void KDE5XLib::doStartup()
{
    if( ! m_bStartupDone )
    {
        //KStartupInfo::appStarted();
        m_bStartupDone = true;
        SAL_INFO( "vcl.kde4", "called KStartupInfo::appStarted()" );
    }
}

using namespace com::sun::star;

uno::Reference< ui::dialogs::XFilePicker2 > KDE5XLib::createFilePicker(
        const uno::Reference< uno::XComponentContext >& xMSF )
{
#if KF5_HAVE_GLIB
    if( qApp->thread() != QThread::currentThread()) {
        SalYieldMutexReleaser aReleaser;
        return Q_EMIT createFilePickerSignal( xMSF );
    }
    return uno::Reference< ui::dialogs::XFilePicker2 >( new KDE4FilePicker( xMSF ) );
#else
    (void)xMSF;
    return NULL;
#endif
}

#include "KDE5XLib.moc"

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
