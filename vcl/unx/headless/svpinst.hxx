/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: svpinst.hxx,v $
 * $Revision: 1.5 $
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
#include <vos/mutex.hxx>
#include <vos/thread.hxx>

#include <list>

#define VIRTUAL_DESKTOP_WIDTH 1024
#define VIRTUAL_DESKTOP_HEIGHT 768
#define VIRTUAL_DESKTOP_DEPTH 24

// -------------------------------------------------------------------------
// SalYieldMutex
// -------------------------------------------------------------------------

class SvpSalYieldMutex : public NAMESPACE_VOS(OMutex)
{
protected:
    ULONG                                       mnCount;
    NAMESPACE_VOS(OThread)::TThreadIdentifier   mnThreadId;

public:
                                                SvpSalYieldMutex();

    virtual void                                acquire();
    virtual void                                release();
    virtual sal_Bool                            tryToAcquire();

    ULONG                                       GetAcquireCount() const { return mnCount; }
    NAMESPACE_VOS(OThread)::TThreadIdentifier   GetThreadId() const { return mnThreadId; }
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
    virtual void Start( ULONG nMS );
    virtual void Stop();
};

// ---------------
// - SalInstance -
// ---------------
class SvpSalFrame;
class SvpSalInstance : public SalInstance
{
    timeval             m_aTimeout;
    ULONG               m_nTimeoutMS;
    int                 m_pTimeoutFDS[2];
    SvpSalYieldMutex    m_aYieldMutex;

    // internal event queue
    struct SalUserEvent
    {
        const SalFrame*     m_pFrame;
        void*               m_pData;
        USHORT             m_nEvent;

        SalUserEvent( const SalFrame* pFrame, void* pData, USHORT nEvent = SALEVENT_USEREVENT )
                : m_pFrame( pFrame ),
                  m_pData( pData ),
                  m_nEvent( nEvent )
        {}
    };

    oslMutex        m_aEventGuard;
    std::list< SalUserEvent > m_aUserEvents;

    std::list< SalFrame* > m_aFrames;
public:
    static SvpSalInstance* s_pDefaultInstance;

    SvpSalInstance();
    virtual ~SvpSalInstance();

    void PostEvent( const SalFrame* pFrame, void* pData, USHORT nEvent );
    void CancelEvent( const SalFrame* pFrame, void* pData, USHORT nEvent );

    void StartTimer( ULONG nMS );
    void StopTimer();
    void Wakeup();

    void registerFrame( SalFrame* pFrame ) { m_aFrames.push_back( pFrame ); }
    void deregisterFrame( SalFrame* pFrame );
    const std::list< SalFrame* >& getFrames() const { return m_aFrames; }

    bool            CheckTimeout( bool bExecuteTimers = true );

    // Frame
    virtual SalFrame*       CreateChildFrame( SystemParentData* pParent, ULONG nStyle );
    virtual SalFrame*       CreateFrame( SalFrame* pParent, ULONG nStyle );
    virtual void            DestroyFrame( SalFrame* pFrame );

    // Object (System Child Window)
    virtual SalObject*      CreateObject( SalFrame* pParent, SystemWindowData* pWindowData, BOOL bShow = TRUE );
    virtual void            DestroyObject( SalObject* pObject );

    // VirtualDevice
    // nDX and nDY in Pixel
    // nBitCount: 0 == Default(=as window) / 1 == Mono
    // pData allows for using a system dependent graphics or device context
    virtual SalVirtualDevice*   CreateVirtualDevice( SalGraphics* pGraphics,
                                                     long nDX, long nDY,
                                                     USHORT nBitCount, const SystemGraphicsData *pData = NULL );
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
    // SalOpenGL
    virtual SalOpenGL*      CreateSalOpenGL( SalGraphics* pGraphics );
    // SalI18NImeStatus
    virtual SalI18NImeStatus*   CreateI18NImeStatus();
    // SalSystem
    virtual SalSystem*      CreateSalSystem();
    // SalBitmap
    virtual SalBitmap*      CreateSalBitmap();

    // YieldMutex
    virtual vos::IMutex*    GetYieldMutex();
    virtual ULONG           ReleaseYieldMutex();
    virtual void            AcquireYieldMutex( ULONG nCount );

    // wait next event and dispatch
    // must returned by UserEvent (SalFrame::PostEvent)
    // and timer
    virtual void            Yield( bool bWait, bool bHandleAllCurrentEvents );
    virtual bool            AnyInput( USHORT nType );

                            // Menues
    virtual SalMenu*        CreateMenu( BOOL bMenuBar );
    virtual void            DestroyMenu( SalMenu* pMenu);
    virtual SalMenuItem*    CreateMenuItem( const SalItemParams* pItemData );
    virtual void            DestroyMenuItem( SalMenuItem* pItem );

    // may return NULL to disable session management
    virtual SalSession*     CreateSalSession();

    virtual void*           GetConnectionIdentifier( ConnectionIdentifierType& rReturnedType, int& rReturnedBytes );
};

#endif // _SV_SALINST_HXX
