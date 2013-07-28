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

#ifndef _SVP_SALINST_HXX
#define _SVP_SALINST_HXX

#include <vcl/solarmutex.hxx>

#include <osl/mutex.hxx>
#include <osl/thread.hxx>
#include <salinst.hxx>
#include <salwtype.hxx>
#include <saltimer.hxx>
#include <generic/geninst.h>
#include <generic/genprn.h>

#include <list>

#include <time.h>

#define VIRTUAL_DESKTOP_WIDTH 1024
#define VIRTUAL_DESKTOP_HEIGHT 768

class SvpSalInstance;
class SvpSalTimer : public SalTimer
{
    SvpSalInstance* m_pInstance;
public:
    SvpSalTimer( SvpSalInstance* pInstance ) : m_pInstance( pInstance ) {}
    virtual ~SvpSalTimer();

    // overload all pure virtual methods
    virtual void Start( sal_uLong nMS );
    virtual void Stop();
};

class SvpSalFrame;
class GenPspGraphics;

class SvpSalInstance : public SalGenericInstance
{
    timeval                 m_aTimeout;
    sal_uLong               m_nTimeoutMS;
    int                     m_pTimeoutFDS[2];

    // internal event queue
    struct SalUserEvent
    {
        const SalFrame*     m_pFrame;
        void*               m_pData;
        sal_uInt16          m_nEvent;

        SalUserEvent( const SalFrame* pFrame, void* pData, sal_uInt16 nEvent = SALEVENT_USEREVENT )
                : m_pFrame( pFrame ),
                  m_pData( pData ),
                  m_nEvent( nEvent )
        {}
    };

    oslMutex                m_aEventGuard;
    std::list< SalUserEvent > m_aUserEvents;

    std::list< SalFrame* >  m_aFrames;

    bool                    isFrameAlive( const SalFrame* pFrame ) const;

protected:
    virtual void            DoReleaseYield( int nTimeoutMS );

public:
    static SvpSalInstance*  s_pDefaultInstance;

    SvpSalInstance( SalYieldMutex *pMutex );
    virtual ~SvpSalInstance();

    void                    PostEvent( const SalFrame* pFrame, void* pData, sal_uInt16 nEvent );

    bool                    PostedEventsInQueue();

    void                    StartTimer( sal_uLong nMS );
    void                    StopTimer();
    void                    Wakeup();

    void                    registerFrame( SalFrame* pFrame ) { m_aFrames.push_back( pFrame ); }
    void                    deregisterFrame( SalFrame* pFrame );
    const std::list< SalFrame* >&
                            getFrames() const { return m_aFrames; }

    bool                    CheckTimeout( bool bExecuteTimers = true );

    // Frame
    virtual SalFrame*       CreateChildFrame( SystemParentData* pParent, sal_uLong nStyle );
    virtual SalFrame*       CreateFrame( SalFrame* pParent, sal_uLong nStyle );
    virtual void            DestroyFrame( SalFrame* pFrame );

    // Object (System Child Window)
    virtual SalObject*      CreateObject( SalFrame* pParent, SystemWindowData* pWindowData, sal_Bool bShow = sal_True );
    virtual void            DestroyObject( SalObject* pObject );

    // VirtualDevice
    // nDX and nDY in Pixel
    // nBitCount: 0 == Default(=as window) / 1 == Mono
    // pData allows for using a system dependent graphics or device context
    virtual SalVirtualDevice*   CreateVirtualDevice( SalGraphics* pGraphics,
                                                     long nDX, long nDY,
                                                     sal_uInt16 nBitCount, const SystemGraphicsData *pData = NULL );
    virtual void                DestroyVirtualDevice( SalVirtualDevice* pDevice );

    // Printer
    // pSetupData->mpDriverData can be 0
    // pSetupData must be updatet with the current
    // JobSetup
    virtual SalInfoPrinter* CreateInfoPrinter( SalPrinterQueueInfo* pQueueInfo,
                                               ImplJobSetup* pSetupData );
    virtual void            DestroyInfoPrinter( SalInfoPrinter* pPrinter );
    virtual SalPrinter*     CreatePrinter( SalInfoPrinter* pInfoPrinter );
    virtual void            DestroyPrinter( SalPrinter* pPrinter );

    virtual void            GetPrinterQueueInfo( ImplPrnQueueList* pList );
    virtual void            GetPrinterQueueState( SalPrinterQueueInfo* pInfo );
    virtual void            DeletePrinterQueueInfo( SalPrinterQueueInfo* pInfo );
    virtual OUString   GetDefaultPrinter();
    virtual void            PostPrintersChanged();

    // SalTimer
    virtual SalTimer*       CreateSalTimer();
    // SalI18NImeStatus
    virtual SalI18NImeStatus*   CreateI18NImeStatus();
    // SalSystem
    virtual SalSystem*      CreateSalSystem();
    // SalBitmap
    virtual SalBitmap*      CreateSalBitmap();

    // wait next event and dispatch
    // must returned by UserEvent (SalFrame::PostEvent)
    // and timer
    virtual void            Yield( bool bWait, bool bHandleAllCurrentEvents );
    virtual bool            AnyInput( sal_uInt16 nType );

    // may return NULL to disable session management
    virtual SalSession*     CreateSalSession();

    virtual void*           GetConnectionIdentifier( ConnectionIdentifierType& rReturnedType, int& rReturnedBytes );

    virtual void            AddToRecentDocumentList(const OUString& rFileUrl, const OUString& rMimeType, const OUString& rDocumentService);

    virtual GenPspGraphics *CreatePrintGraphics();
};

#endif // _SV_SALINST_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
