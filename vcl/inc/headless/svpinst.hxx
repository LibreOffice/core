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

#ifndef INCLUDED_VCL_INC_HEADLESS_SVPINST_HXX
#define INCLUDED_VCL_INC_HEADLESS_SVPINST_HXX

#include <osl/mutex.hxx>
#include <osl/thread.hxx>
#include <salinst.hxx>
#include <salwtype.hxx>
#include <saltimer.hxx>
#include <unx/geninst.h>
#include <unx/genprn.h>

#include <list>

#include <time.h>

#define VIRTUAL_DESKTOP_WIDTH 1024
#define VIRTUAL_DESKTOP_HEIGHT 768

#ifdef IOS
#define SvpSalInstance AquaSalInstance
#endif

class SvpSalInstance;
class SvpSalTimer : public SalTimer
{
    SvpSalInstance* m_pInstance;
public:
    SvpSalTimer( SvpSalInstance* pInstance ) : m_pInstance( pInstance ) {}
    virtual ~SvpSalTimer() override;

    // override all pure virtual methods
    virtual void Start( sal_uLong nMS ) override;
    virtual void Stop() override;
};

class SvpSalFrame;
class GenPspGraphics;

SalInstance* svp_create_SalInstance();

class VCL_DLLPUBLIC SvpSalInstance : public SalGenericInstance
{
    timeval                 m_aTimeout;
    sal_uLong               m_nTimeoutMS;
    int                     m_pTimeoutFDS[2];

    // internal event queue
    struct SalUserEvent
    {
        const SalFrame*     m_pFrame;
        ImplSVEvent*        m_pData;
        SalEvent            m_nEvent;

        SalUserEvent( const SalFrame* pFrame, ImplSVEvent* pData, SalEvent nEvent )
                : m_pFrame( pFrame ),
                  m_pData( pData ),
                  m_nEvent( nEvent )
        {}
    };

    osl::Mutex              m_aEventGuard;
    std::list< SalUserEvent > m_aUserEvents;

    std::list< SalFrame* >  m_aFrames;

    bool                    isFrameAlive( const SalFrame* pFrame ) const;

    void                    DoReleaseYield( int nTimeoutMS );

public:
    static SvpSalInstance*  s_pDefaultInstance;

    SvpSalInstance( SalYieldMutex *pMutex );
    virtual ~SvpSalInstance() override;

    void                    CloseWakeupPipe(bool log);
    void                    CreateWakeupPipe(bool log);

    void                    PostEvent(const SalFrame* pFrame, ImplSVEvent* pData, SalEvent nEvent);

#ifdef ANDROID
    bool                    PostedEventsInQueue();
#endif

    void                    StartTimer( sal_uLong nMS );
    void                    StopTimer();
    void                    Wakeup();

    void                    registerFrame( SalFrame* pFrame ) { m_aFrames.push_back( pFrame ); }
    void                    deregisterFrame( SalFrame* pFrame );
    const std::list< SalFrame* >&
                            getFrames() const { return m_aFrames; }

    bool                    CheckTimeout( bool bExecuteTimers = true );

    // Frame
    virtual SalFrame*       CreateChildFrame( SystemParentData* pParent, SalFrameStyleFlags nStyle ) override;
    virtual SalFrame*       CreateFrame( SalFrame* pParent, SalFrameStyleFlags nStyle ) override;
    virtual void            DestroyFrame( SalFrame* pFrame ) override;

    // Object (System Child Window)
    virtual SalObject*      CreateObject( SalFrame* pParent, SystemWindowData* pWindowData, bool bShow ) override;
    virtual void            DestroyObject( SalObject* pObject ) override;

    // VirtualDevice
    // nDX and nDY in Pixel
    // nBitCount: 0 == Default(=as window) / 1 == Mono
    // pData allows for using a system dependent graphics or device context
    virtual SalVirtualDevice*   CreateVirtualDevice( SalGraphics* pGraphics,
                                                     long &nDX, long &nDY,
                                                     DeviceFormat eFormat, const SystemGraphicsData *pData = nullptr ) override;

    // Printer
    // pSetupData->mpDriverData can be 0
    // pSetupData must be updatet with the current
    // JobSetup
    virtual SalInfoPrinter* CreateInfoPrinter( SalPrinterQueueInfo* pQueueInfo,
                                               ImplJobSetup* pSetupData ) override;
    virtual void            DestroyInfoPrinter( SalInfoPrinter* pPrinter ) override;
    virtual SalPrinter*     CreatePrinter( SalInfoPrinter* pInfoPrinter ) override;
    virtual void            DestroyPrinter( SalPrinter* pPrinter ) override;

    virtual void            GetPrinterQueueInfo( ImplPrnQueueList* pList ) override;
    virtual void            GetPrinterQueueState( SalPrinterQueueInfo* pInfo ) override;
    virtual void            DeletePrinterQueueInfo( SalPrinterQueueInfo* pInfo ) override;
    virtual OUString        GetDefaultPrinter() override;
    virtual void            PostPrintersChanged() override;

    // SalTimer
    virtual SalTimer*       CreateSalTimer() override;
    // SalSystem
    virtual SalSystem*      CreateSalSystem() override;
    // SalBitmap
    virtual SalBitmap*      CreateSalBitmap() override;

    // wait next event and dispatch
    // must returned by UserEvent (SalFrame::PostEvent)
    // and timer
    virtual bool            DoYield(bool bWait, bool bHandleAllCurrentEvents, sal_uLong nReleased) override;
    virtual bool            AnyInput( VclInputFlags nType ) override;

    // may return NULL to disable session management
    virtual SalSession*     CreateSalSession() override;

    virtual OpenGLContext*  CreateOpenGLContext() override;

    virtual void*           GetConnectionIdentifier( ConnectionIdentifierType& rReturnedType, int& rReturnedBytes ) override;

    virtual void            AddToRecentDocumentList(const OUString& rFileUrl, const OUString& rMimeType, const OUString& rDocumentService) override;

    virtual GenPspGraphics *CreatePrintGraphics() override;
};

#endif // INCLUDED_VCL_INC_HEADLESS_SVPINST_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
