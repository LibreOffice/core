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

#include "Kf5Instance.hxx"
#include <Kf5Instance.moc>

#include <QtCore/QThread>
#include <QtWidgets/QApplication>
#include <QtCore/QAbstractEventDispatcher>

#include "Kf5Frame.hxx"
#include "Kf5Data.hxx"
#include "Kf5Timer.hxx"
#include "Kf5VirtualDevice.hxx"

#include <vclpluginapi.h>
#include <sal/log.hxx>

#include <headless/svpdummies.hxx>
#include <headless/svpbmp.hxx>

Kf5Instance::Kf5Instance( SalYieldMutex* pMutex )
    : SalGenericInstance( pMutex )
    , m_postUserEventId( -1 )
{
    m_postUserEventId = QEvent::registerEventType();

    // this one needs to be blocking, so that the handling in main thread
    // is processed before the thread emitting the signal continues
    connect( this, SIGNAL( ImplYieldSignal( bool, bool )),
             this, SLOT( ImplYield( bool, bool )), Qt::BlockingQueuedConnection );
}

Kf5Instance::~Kf5Instance()
{
}

SalFrame* Kf5Instance::CreateChildFrame( SystemParentData* /*pParent*/, SalFrameStyleFlags nStyle )
{
    return new Kf5Frame( this, nullptr, nStyle );
}

SalFrame* Kf5Instance::CreateFrame( SalFrame* pParent, SalFrameStyleFlags nStyle )
{
    return new Kf5Frame( this, pParent, nStyle );
}

void Kf5Instance::DestroyFrame( SalFrame* pFrame )
{
    delete pFrame;
}

SalObject* Kf5Instance::CreateObject( SalFrame*, SystemWindowData*, bool )
{
    return new SvpSalObject();
}

void Kf5Instance::DestroyObject( SalObject* pObject )
{
    delete pObject;
}

SalVirtualDevice* Kf5Instance::CreateVirtualDevice( SalGraphics* /* pGraphics */,
                                                    long &nDX, long &nDY,
                                                    DeviceFormat eFormat,
                                                    const SystemGraphicsData* /* pData */ )
{
    Kf5VirtualDevice* pVD = new Kf5VirtualDevice( eFormat, 1 );
    pVD->SetSize( nDX, nDY );
    return pVD;
}

SalTimer* Kf5Instance::CreateSalTimer()
{
    return new Kf5Timer();
}

SalSystem* Kf5Instance::CreateSalSystem()
{
    return new SvpSalSystem();
}

SalBitmap* Kf5Instance::CreateSalBitmap()
{
    return new SvpSalBitmap();
}

bool Kf5Instance::ImplYield( bool bWait, bool bHandleAllCurrentEvents )
{
    bool wasEvent = DispatchUserEvents( bHandleAllCurrentEvents );
    if ( !bHandleAllCurrentEvents && wasEvent )
        return true;

    /**
     * Quoting the Qt docs: [QAbstractEventDispatcher::processEvents] processes
     * pending events that match flags until there are no more events to process.
     */
    QAbstractEventDispatcher* dispatcher = QAbstractEventDispatcher::instance( qApp->thread());
    if ( bWait && !wasEvent )
        wasEvent = dispatcher->processEvents( QEventLoop::WaitForMoreEvents );
    else
        wasEvent = dispatcher->processEvents( QEventLoop::AllEvents ) || wasEvent;
    return wasEvent;
}

bool Kf5Instance::DoYield(bool bWait, bool bHandleAllCurrentEvents)
{
    bool bWasEvent = false;
    if( qApp->thread() == QThread::currentThread() )
    {
        bWasEvent = ImplYield( bWait, bHandleAllCurrentEvents );
        if ( bWasEvent )
            m_aWaitingYieldCond.set();
    }
    else
    {
        {
            SolarMutexReleaser aReleaser;
            bWasEvent = Q_EMIT ImplYieldSignal( false, bHandleAllCurrentEvents );
        }
        if ( !bWasEvent && bWait )
        {
            m_aWaitingYieldCond.reset();
            SolarMutexReleaser aReleaser;
            m_aWaitingYieldCond.wait();
            bWasEvent = true;
        }
    }
    return bWasEvent;
}

bool Kf5Instance::AnyInput( VclInputFlags nType )
{
    return false;
}

SalSession* Kf5Instance::CreateSalSession()
{
    return nullptr;
}

OUString Kf5Instance::GetConnectionIdentifier()
{
    return OUString();
}

void Kf5Instance::AddToRecentDocumentList(const OUString&, const OUString&, const OUString&)
{
}

OpenGLContext* Kf5Instance::CreateOpenGLContext()
{
    return nullptr;
}

bool Kf5Instance::IsMainThread() const
{
    return qApp->thread() != QThread::currentThread();
}

void Kf5Instance::TriggerUserEventProcessing()
{
    QApplication::postEvent(this, new QEvent(QEvent::Type( m_postUserEventId )));
}

void Kf5Instance::ProcessEvent( SalUserEvent aEvent )
{
    aEvent.m_pFrame->CallCallback( aEvent.m_nEvent, aEvent.m_pData );
}

extern "C" {
    VCLPLUG_KF5_PUBLIC SalInstance* create_SalInstance()
    {
        OString aVersion( qVersion() );
        SAL_INFO( "vcl.kf5", "qt version string is " << aVersion );

        Kf5Instance* pInstance = new Kf5Instance( new SalYieldMutex() );

        // initialize SalData
        new Kf5Data( pInstance );

        return pInstance;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
