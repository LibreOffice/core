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
#include <generic/geninst.h>
#include <generic/genprn.h>
#include <basebmp/scanlineformats.hxx>

#include <list>
#include <map>

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
    virtual ~SvpSalTimer();

    // override all pure virtual methods
    virtual void Start( sal_uLong nMS ) SAL_OVERRIDE;
    virtual void Stop() SAL_OVERRIDE;
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
        sal_uInt16          m_nEvent;

        SalUserEvent( const SalFrame* pFrame, ImplSVEvent* pData, sal_uInt16 nEvent = SALEVENT_USEREVENT )
                : m_pFrame( pFrame ),
                  m_pData( pData ),
                  m_nEvent( nEvent )
        {}
    };

    oslMutex                m_aEventGuard;
    std::list< SalUserEvent > m_aUserEvents;

    std::list< SalFrame* >  m_aFrames;

    bool                    isFrameAlive( const SalFrame* pFrame ) const;

    void                    DoReleaseYield( int nTimeoutMS );

    typedef std::map< sal_uInt16, ::basebmp::Format > BitCountFormatMap;
    BitCountFormatMap m_aBitCountFormatMap;

public:
    static SvpSalInstance*  s_pDefaultInstance;

    SvpSalInstance( SalYieldMutex *pMutex );
    virtual ~SvpSalInstance();

    void                    PostEvent(const SalFrame* pFrame, ImplSVEvent* pData, sal_uInt16 nEvent);

    void                    StartTimer( sal_uLong nMS );
    void                    StopTimer();
    void                    Wakeup();

    void                    registerFrame( SalFrame* pFrame ) { m_aFrames.push_back( pFrame ); }
    void                    deregisterFrame( SalFrame* pFrame );
    const std::list< SalFrame* >&
                            getFrames() const { return m_aFrames; }

    bool                    CheckTimeout( bool bExecuteTimers = true );

    // Frame
    virtual SalFrame*       CreateChildFrame( SystemParentData* pParent, sal_uLong nStyle ) SAL_OVERRIDE;
    virtual SalFrame*       CreateFrame( SalFrame* pParent, sal_uLong nStyle ) SAL_OVERRIDE;
    virtual void            DestroyFrame( SalFrame* pFrame ) SAL_OVERRIDE;

    // Object (System Child Window)
    virtual SalObject*      CreateObject( SalFrame* pParent, SystemWindowData* pWindowData, bool bShow = true ) SAL_OVERRIDE;
    virtual void            DestroyObject( SalObject* pObject ) SAL_OVERRIDE;

    // VirtualDevice
    // nDX and nDY in Pixel
    // nBitCount: 0 == Default(=as window) / 1 == Mono
    // pData allows for using a system dependent graphics or device context
    virtual SalVirtualDevice*   CreateVirtualDevice( SalGraphics* pGraphics,
                                                     long &nDX, long &nDY,
                                                     sal_uInt16 nBitCount, const SystemGraphicsData *pData = NULL ) SAL_OVERRIDE;

    // Printer
    // pSetupData->mpDriverData can be 0
    // pSetupData must be updatet with the current
    // JobSetup
    virtual SalInfoPrinter* CreateInfoPrinter( SalPrinterQueueInfo* pQueueInfo,
                                               ImplJobSetup* pSetupData ) SAL_OVERRIDE;
    virtual void            DestroyInfoPrinter( SalInfoPrinter* pPrinter ) SAL_OVERRIDE;
    virtual SalPrinter*     CreatePrinter( SalInfoPrinter* pInfoPrinter ) SAL_OVERRIDE;
    virtual void            DestroyPrinter( SalPrinter* pPrinter ) SAL_OVERRIDE;

    virtual void            GetPrinterQueueInfo( ImplPrnQueueList* pList ) SAL_OVERRIDE;
    virtual void            GetPrinterQueueState( SalPrinterQueueInfo* pInfo ) SAL_OVERRIDE;
    virtual void            DeletePrinterQueueInfo( SalPrinterQueueInfo* pInfo ) SAL_OVERRIDE;
    virtual OUString        GetDefaultPrinter() SAL_OVERRIDE;
    virtual void            PostPrintersChanged() SAL_OVERRIDE;

    // SalTimer
    virtual SalTimer*       CreateSalTimer() SAL_OVERRIDE;
    // SalI18NImeStatus
    virtual SalI18NImeStatus*   CreateI18NImeStatus() SAL_OVERRIDE;
    // SalSystem
    virtual SalSystem*      CreateSalSystem() SAL_OVERRIDE;
    // SalBitmap
    virtual SalBitmap*      CreateSalBitmap() SAL_OVERRIDE;

    // wait next event and dispatch
    // must returned by UserEvent (SalFrame::PostEvent)
    // and timer
    virtual void            DoYield(bool bWait, bool bHandleAllCurrentEvents, sal_uLong nReleased) SAL_OVERRIDE;
    virtual bool            AnyInput( VclInputFlags nType ) SAL_OVERRIDE;

    // may return NULL to disable session management
    virtual SalSession*     CreateSalSession() SAL_OVERRIDE;

    virtual void*           GetConnectionIdentifier( ConnectionIdentifierType& rReturnedType, int& rReturnedBytes ) SAL_OVERRIDE;

    virtual void            AddToRecentDocumentList(const OUString& rFileUrl, const OUString& rMimeType, const OUString& rDocumentService) SAL_OVERRIDE;

    virtual GenPspGraphics *CreatePrintGraphics() SAL_OVERRIDE;

    // We want to be able to select colourspace, i.e. ARGB vs RGBA vs BGRA etc.
    // -- as the rest of vcl always uses bit depths, it is perhaps simplest
    // to let us simply change the mapping of bitcount to format (which was
    // previously unchangeable).
    void setBitCountFormatMapping( sal_uInt16 nBitCount, ::basebmp::Format aFormat );

    ::basebmp::Format getFormatForBitCount( sal_uInt16 );
};

#endif // INCLUDED_VCL_INC_HEADLESS_SVPINST_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
