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

#ifndef _SVP_SALINST_HXX
#define _SVP_SALINST_HXX

#include <vcl/salinst.hxx>
#include <vcl/salwtype.hxx>
#include <vcl/saltimer.hxx>
#include <vcl/solarmutex.hxx>

#include <osl/mutex.hxx>
#include <osl/thread.hxx>

#include <list>

#include <time.h>  // timeval

#define VIRTUAL_DESKTOP_WIDTH 1024
#define VIRTUAL_DESKTOP_HEIGHT 768
#define VIRTUAL_DESKTOP_DEPTH 24

// -------------------------------------------------------------------------
// SalYieldMutex
// -------------------------------------------------------------------------

class SvpSalYieldMutex : public ::vcl::SolarMutexObject
{
protected:
    sal_uLong                                       mnCount;
    oslThreadIdentifier mnThreadId;

public:
                                                SvpSalYieldMutex();

    virtual void                                acquire();
    virtual void                                release();
    virtual sal_Bool                            tryToAcquire();

    sal_uLong                                       GetAcquireCount() const { return mnCount; }
    oslThreadIdentifier GetThreadId() const { return mnThreadId; }
};

// ---------------
// - SalTimer -
// ---------------
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

// ---------------
// - SalInstance -
// ---------------
class SvpSalFrame;
class SvpSalInstance : public SalInstance
{
    timeval             m_aTimeout;
    sal_uLong               m_nTimeoutMS;
    int                 m_pTimeoutFDS[2];
    SvpSalYieldMutex    m_aYieldMutex;

    // internal event queue
    struct SalUserEvent
    {
        const SalFrame*     m_pFrame;
        void*               m_pData;
        sal_uInt16         m_nEvent;

        SalUserEvent( const SalFrame* pFrame, void* pData, sal_uInt16 nEvent = SALEVENT_USEREVENT )
                : m_pFrame( pFrame ),
                  m_pData( pData ),
                  m_nEvent( nEvent )
        {}
    };

    oslMutex        m_aEventGuard;
    std::list< SalUserEvent > m_aUserEvents;

    std::list< SalFrame* > m_aFrames;

    bool isFrameAlive( const SalFrame* pFrame ) const;

public:
    static SvpSalInstance* s_pDefaultInstance;

    SvpSalInstance();
    virtual ~SvpSalInstance();

    void PostEvent( const SalFrame* pFrame, void* pData, sal_uInt16 nEvent );
    void CancelEvent( const SalFrame* pFrame, void* pData, sal_uInt16 nEvent );

    void StartTimer( sal_uLong nMS );
    void StopTimer();
    void Wakeup();

    void registerFrame( SalFrame* pFrame ) { m_aFrames.push_back( pFrame ); }
    void deregisterFrame( SalFrame* pFrame );
    const std::list< SalFrame* >& getFrames() const { return m_aFrames; }

    bool            CheckTimeout( bool bExecuteTimers = true );

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
    virtual String          GetDefaultPrinter();

    // SalTimer
    virtual SalTimer*       CreateSalTimer();
    // SalI18NImeStatus
    virtual SalI18NImeStatus*   CreateI18NImeStatus();
    // SalSystem
    virtual SalSystem*      CreateSalSystem();
    // SalBitmap
    virtual SalBitmap*      CreateSalBitmap();

    // YieldMutex
    virtual osl::SolarMutex* GetYieldMutex();
    virtual sal_uLong           ReleaseYieldMutex();
    virtual void            AcquireYieldMutex( sal_uLong nCount );
    virtual bool            CheckYieldMutex();

    // wait next event and dispatch
    // must returned by UserEvent (SalFrame::PostEvent)
    // and timer
    virtual void            Yield( bool bWait, bool bHandleAllCurrentEvents );
    virtual bool            AnyInput( sal_uInt16 nType );

    // may return NULL to disable session management
    virtual SalSession*     CreateSalSession();

    virtual void*           GetConnectionIdentifier( ConnectionIdentifierType& rReturnedType, int& rReturnedBytes );

    virtual void            AddToRecentDocumentList(const rtl::OUString& rFileUrl, const rtl::OUString& rMimeType);

    virtual void updatePrinterUpdate();
    virtual void jobStartedPrinterUpdate();
    virtual void jobEndedPrinterUpdate();
};

#endif // _SV_SALINST_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
