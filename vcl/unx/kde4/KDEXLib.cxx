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
#include <qthread.h>

#undef Region

#include "KDEXLib.hxx"

#include <i18n_im.hxx>
#include <i18n_xkb.hxx>

#include <saldata.hxx>
#include <vos/process.hxx>

#include "KDESalDisplay.hxx"

#if OSL_DEBUG_LEVEL > 1
#include <stdio.h>
#endif

KDEXLib::KDEXLib() :
    SalXLib(),  m_bStartupDone(false), m_pApplication(0),
    m_pFreeCmdLineArgs(0), m_pAppCmdLineArgs(0), m_nFakeCmdLineArgs( 0 )
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

    KAboutData *kAboutData = new KAboutData("OpenOffice.org",
            "kdelibs4",
            ki18n( "OpenOffice.org" ),
            "3.0.0",
            ki18n( "OpenOffice.org with KDE Native Widget Support." ),
            KAboutData::License_LGPL,
            ki18n( "Copyright (c) 2003, 2004, 2005, 2006, 2007, 2008, 2009 Novell, Inc"),
            ki18n( "OpenOffice.org is an office suite.\n" ),
            "http://kde.openoffice.org/index.html",
            "dev@kde.openoffice.org" );

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

    m_nFakeCmdLineArgs = 1;
    USHORT nIdx;
    vos::OExtCommandLine aCommandLine;
    int nParams = aCommandLine.getCommandArgCount();
    rtl::OString aDisplay;
    rtl::OUString aParam, aBin;

    for ( nIdx = 0; nIdx < nParams; ++nIdx )
    {
        aCommandLine.getCommandArg( nIdx, aParam );
        if ( !m_pFreeCmdLineArgs && aParam.equalsAscii( "-display" ) && nIdx + 1 < nParams )
        {
            aCommandLine.getCommandArg( nIdx + 1, aParam );
            aDisplay = rtl::OUStringToOString( aParam, osl_getThreadTextEncoding() );

            m_nFakeCmdLineArgs = 3;
            m_pFreeCmdLineArgs = new char*[ m_nFakeCmdLineArgs ];
            m_pFreeCmdLineArgs[ 1 ] = strdup( "-display" );
            m_pFreeCmdLineArgs[ 2 ] = strdup( aDisplay.getStr() );
        }
    }
    if ( !m_pFreeCmdLineArgs )
        m_pFreeCmdLineArgs = new char*[ m_nFakeCmdLineArgs ];

    osl_getExecutableFile( &aParam.pData );
    osl_getSystemPathFromFileURL( aParam.pData, &aBin.pData );
    rtl::OString aExec = rtl::OUStringToOString( aBin, osl_getThreadTextEncoding() );
    m_pFreeCmdLineArgs[0] = strdup( aExec.getStr() );

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

    Display* pDisp = QX11Info::display();
    SalKDEDisplay *pSalDisplay = new SalKDEDisplay(pDisp);

    pInputMethod->CreateMethod( pDisp );
    pInputMethod->AddConnectionWatch( pDisp, (void*)this );
    pSalDisplay->SetInputMethod( pInputMethod );

    PushXErrorLevel( true );
    SalI18N_KeyboardExtension *pKbdExtension = new SalI18N_KeyboardExtension( pDisp );
    XSync( pDisp, False );

    pKbdExtension->UseExtension( ! HasXErrorOccured() );
    PopXErrorLevel();

    pSalDisplay->SetKbdExtension( pKbdExtension );
}

void KDEXLib::Insert( int fd, void* data, YieldFunc pending, YieldFunc queued, YieldFunc handle )
{
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
    SocketData sdata = socketData.take( fd );// according to SalXLib::Remove() this should be safe
    delete sdata.notifier;
}

void KDEXLib::socketNotifierActivated( int fd )
{
    SalData *pSalData = GetSalData();
    const SocketData& sdata = socketData[ fd ];
    pSalData->m_pInstance->GetYieldMutex()->acquire();
    sdata.handle( fd, sdata.data );
    pSalData->m_pInstance->GetYieldMutex()->release();
}

void KDEXLib::Yield( bool bWait, bool bHandleAllCurrentEvents )
{
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

void KDEXLib::processYield( bool bWait, bool bHandleAllCurrentEvents )
{
    YieldMutexReleaser aReleaser;
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

void KDEXLib::StartTimer( ULONG nMS )
{
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
    timeoutTimer.stop();
}

void KDEXLib::timeoutActivated()
{
    SalData *pSalData = GetSalData();
    pSalData->m_pInstance->GetYieldMutex()->acquire();
    GetX11SalData()->Timeout();
    // QTimer is not single shot, so will be restarted immediatelly
    pSalData->m_pInstance->GetYieldMutex()->release();
}

void KDEXLib::Wakeup()
{
    QAbstractEventDispatcher::instance( qApp->thread())->wakeUp(); // main thread event loop
}

void KDEXLib::PostUserEvent()
{
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
    SalData *pSalData = GetSalData();
    pSalData->m_pInstance->GetYieldMutex()->acquire();
    SalKDEDisplay::self()->EventGuardAcquire();
    if( SalKDEDisplay::self()->userEventsCount() <= 1 )
        userEventTimer.stop();
    SalKDEDisplay::self()->EventGuardRelease();
    SalKDEDisplay::self()->DispatchInternalEvent();
    pSalData->m_pInstance->GetYieldMutex()->release();
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
