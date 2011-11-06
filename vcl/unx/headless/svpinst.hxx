/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#ifndef _SVP_SALINST_HXX
#define _SVP_SALINST_HXX

#include <vos/mutex.hxx>
#include <vos/thread.hxx>

#include <salinst.hxx>
#include <salwtype.hxx>
#include <saltimer.hxx>

#include <list>

#include <time.h>  // timeval

#define VIRTUAL_DESKTOP_WIDTH 1024
#define VIRTUAL_DESKTOP_HEIGHT 768
#define VIRTUAL_DESKTOP_DEPTH 24

// -------------------------------------------------------------------------
// SalYieldMutex
// -------------------------------------------------------------------------

class SvpSalYieldMutex : public vos::OMutex
{
protected:
    sal_uLong                                       mnCount;
    vos::OThread::TThreadIdentifier mnThreadId;

public:
                                                SvpSalYieldMutex();

    virtual void                                acquire();
    virtual void                                release();
    virtual sal_Bool                            tryToAcquire();

    sal_uLong                                       GetAcquireCount() const { return mnCount; }
    vos::OThread::TThreadIdentifier GetThreadId() const { return mnThreadId; }
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
    virtual vos::IMutex*    GetYieldMutex();
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
};

#endif // _SV_SALINST_HXX
