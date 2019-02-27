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
#include <osl/conditn.hxx>
#include <salinst.hxx>
#include <salwtype.hxx>
#include <saltimer.hxx>
#include <salusereventlist.hxx>
#include <unx/geninst.h>
#include <unx/genprn.h>

#include <list>
#include <condition_variable>

#include <sys/time.h>
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
    virtual void Start( sal_uInt64 nMS ) override;
    virtual void Stop() override;
};

class SvpSalFrame;
class GenPspGraphics;

enum class SvpRequest
{
    NONE,
    MainThreadDispatchOneEvent,
    MainThreadDispatchAllEvents,
};

class SvpSalYieldMutex : public SalYieldMutex
{
private:
    // note: these members might as well live in SvpSalInstance, but there is
    // at least one subclass of SvpSalInstance (GTK3) that doesn't use them.
    friend class SvpSalInstance;
    // members for communication from main thread to non-main thread
#ifndef IOS
    int                     m_FeedbackFDs[2];
#endif
    osl::Condition          m_NonMainWaitingYieldCond;
    // members for communication from non-main thread to main thread
    bool                    m_bNoYieldLock = false; // accessed only on main thread
    std::mutex              m_WakeUpMainMutex; // guard m_wakeUpMain & m_Request
    std::condition_variable m_WakeUpMainCond;
    bool                    m_wakeUpMain = false;
    SvpRequest              m_Request = SvpRequest::NONE;

protected:
    virtual void            doAcquire( sal_uInt32 nLockCount ) override;
    virtual sal_uInt32      doRelease( bool bUnlockAll ) override;

public:
    SvpSalYieldMutex();
    virtual ~SvpSalYieldMutex() override;

    virtual bool IsCurrentThread() const override;

};

SalInstance* svp_create_SalInstance();

// NOTE: the functions IsMainThread, DoYield and Wakeup *require* the use of
// SvpSalYieldMutex; if a subclass uses something else it must override these
// (Wakeup is only called by SvpSalTimer and SvpSalFrame)
class VCL_DLLPUBLIC SvpSalInstance : public SalGenericInstance, public SalUserEventList
{
    timeval                 m_aTimeout;
    sal_uLong               m_nTimeoutMS;
    oslThreadIdentifier     m_MainThread;

    virtual void            TriggerUserEventProcessing() override;
    virtual void            ProcessEvent( SalUserEvent aEvent ) override;
    void                    Wakeup(SvpRequest request = SvpRequest::NONE);

public:
    static SvpSalInstance*  s_pDefaultInstance;

    SvpSalInstance( std::unique_ptr<SalYieldMutex> pMutex );
    virtual ~SvpSalInstance() override;

    void                    CloseWakeupPipe(bool log);
    void                    CreateWakeupPipe(bool log);

    void                    StartTimer( sal_uInt64 nMS );
    void                    StopTimer();

    inline void             registerFrame( SalFrame* pFrame );
    inline void             deregisterFrame( SalFrame* pFrame );

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
    virtual std::unique_ptr<SalVirtualDevice>
                            CreateVirtualDevice( SalGraphics* pGraphics,
                                                     long &nDX, long &nDY,
                                                     DeviceFormat eFormat, const SystemGraphicsData *pData = nullptr ) override;

    // Printer
    // pSetupData->mpDriverData can be 0
    // pSetupData must be updated with the current
    // JobSetup
    virtual SalInfoPrinter* CreateInfoPrinter( SalPrinterQueueInfo* pQueueInfo,
                                               ImplJobSetup* pSetupData ) override;
    virtual void            DestroyInfoPrinter( SalInfoPrinter* pPrinter ) override;
    virtual std::unique_ptr<SalPrinter> CreatePrinter( SalInfoPrinter* pInfoPrinter ) override;

    virtual void            GetPrinterQueueInfo( ImplPrnQueueList* pList ) override;
    virtual void            GetPrinterQueueState( SalPrinterQueueInfo* pInfo ) override;
    virtual OUString        GetDefaultPrinter() override;
    virtual void            PostPrintersChanged() override;

    // SalTimer
    virtual SalTimer*       CreateSalTimer() override;
    // SalSystem
    virtual SalSystem*      CreateSalSystem() override;
    // SalBitmap
    virtual std::shared_ptr<SalBitmap> CreateSalBitmap() override;

    // wait next event and dispatch
    // must returned by UserEvent (SalFrame::PostEvent)
    // and timer
    virtual bool            DoYield(bool bWait, bool bHandleAllCurrentEvents) override;
    virtual bool            AnyInput( VclInputFlags nType ) override;
    virtual bool            IsMainThread() const override;
    virtual void            updateMainThread() override;

    virtual OpenGLContext*  CreateOpenGLContext() override;

    virtual OUString        GetConnectionIdentifier() override;

    virtual void            AddToRecentDocumentList(const OUString& rFileUrl, const OUString& rMimeType, const OUString& rDocumentService) override;

    virtual GenPspGraphics *CreatePrintGraphics() override;
};

inline void SvpSalInstance::registerFrame( SalFrame* pFrame )
{
    insertFrame( pFrame );
}

inline void SvpSalInstance::deregisterFrame( SalFrame* pFrame )
{
    eraseFrame( pFrame );
}

VCL_DLLPUBLIC cairo_surface_t* get_underlying_cairo_surface(const VirtualDevice& rDevice);

#endif // INCLUDED_VCL_INC_HEADLESS_SVPINST_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
