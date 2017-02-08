
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

#ifndef INCLUDED_VCL_INC_OSX_SALINST_H
#define INCLUDED_VCL_INC_OSX_SALINST_H

#include <list>

#include <comphelper/solarmutex.hxx>
#include <osl/conditn.h>
#include <osl/thread.hxx>

#ifdef MACOSX
#include "osx/osxvcltypes.h"
#endif
#include "salinst.hxx"

class AquaSalFrame;
class ApplicationEvent;
class Image;
enum class SalEvent;

class SalYieldMutex : public comphelper::SolarMutex
{
    osl::Mutex m_mutex;
    sal_uLong                                   mnCount;
    oslThreadIdentifier                         mnThreadId;

public:
                                                SalYieldMutex();
    virtual void                                acquire() override;
    virtual void                                release() override;
    virtual bool                                tryToAcquire() override;
    sal_uLong                                   GetAcquireCount() const { return mnCount; }
    oslThreadIdentifier                         GetThreadId() const { return mnThreadId; }
};

class AquaSalInstance : public SalInstance
{
    struct SalUserEvent
    {
        AquaSalFrame*   mpFrame;
        void*           mpData;
        SalEvent        mnType;

        SalUserEvent( AquaSalFrame* pFrame, void* pData, SalEvent nType ) :
            mpFrame( pFrame ), mpData( pData ), mnType( nType )
        {}
    };

public:
    SalYieldMutex*                          mpSalYieldMutex;        // Sal-Yield-Mutex
    OUString                                maDefaultPrinter;
    oslThreadIdentifier                     maMainThread;
    bool                                    mbWaitingYield;
    int                                     mnActivePrintJobs;
    std::list< SalUserEvent >               maUserEvents;
    osl::Mutex                              maUserEventListMutex;
    oslCondition                            maWaitingYieldCond;

    static std::list<const ApplicationEvent*> aAppEventList;

public:
    AquaSalInstance();
    virtual ~AquaSalInstance() override;

    virtual SalFrame*       CreateChildFrame( SystemParentData* pParent, SalFrameStyleFlags nStyle ) override;
    virtual SalFrame*       CreateFrame( SalFrame* pParent, SalFrameStyleFlags nStyle ) override;
    virtual void            DestroyFrame( SalFrame* pFrame ) override;
    virtual SalObject*      CreateObject( SalFrame* pParent, SystemWindowData* pWindowData,
                                          bool bShow ) override;
    virtual void            DestroyObject( SalObject* pObject ) override;
    virtual SalVirtualDevice* CreateVirtualDevice( SalGraphics* pGraphics,
                                                   long &nDX, long &nDY,
                                                   DeviceFormat eFormat,
                                                   const SystemGraphicsData *pData = nullptr ) override;
    virtual SalInfoPrinter* CreateInfoPrinter( SalPrinterQueueInfo* pQueueInfo,
                                               ImplJobSetup* pSetupData ) override;
    virtual void            DestroyInfoPrinter( SalInfoPrinter* pPrinter ) override;
    virtual SalPrinter*     CreatePrinter( SalInfoPrinter* pInfoPrinter ) override;
    virtual void            DestroyPrinter( SalPrinter* pPrinter ) override;
    virtual void            GetPrinterQueueInfo( ImplPrnQueueList* pList ) override;
    virtual void            GetPrinterQueueState( SalPrinterQueueInfo* pInfo ) override;
    virtual void            DeletePrinterQueueInfo( SalPrinterQueueInfo* pInfo ) override;
    virtual OUString        GetDefaultPrinter() override;
    virtual SalTimer*       CreateSalTimer() override;
    virtual SalSystem*      CreateSalSystem() override;
    virtual SalBitmap*      CreateSalBitmap() override;
    virtual comphelper::SolarMutex* GetYieldMutex() override;
    virtual sal_uLong       ReleaseYieldMutex() override;
    virtual void            AcquireYieldMutex( sal_uLong nCount ) override;
    virtual bool            CheckYieldMutex() override;
    virtual SalYieldResult  DoYield(bool bWait, bool bHandleAllCurrentEvents,
                                    sal_uLong nReleased) override;
    virtual bool            AnyInput( VclInputFlags nType ) override;
    virtual SalMenu*        CreateMenu( bool bMenuBar, Menu* pVCLMenu ) override;
    virtual void            DestroyMenu( SalMenu* ) override;
    virtual SalMenuItem*    CreateMenuItem( const SalItemParams* pItemData ) override;
    virtual void            DestroyMenuItem( SalMenuItem* ) override;
    virtual SalSession*     CreateSalSession() override;
    virtual OpenGLContext*  CreateOpenGLContext() override;
    virtual OUString        GetConnectionIdentifier() override;
    virtual void            AddToRecentDocumentList(const OUString& rFileUrl, const OUString& rMimeType,
                                                    const OUString& rDocumentService) override;

    virtual OUString        getOSVersion() override;

    // dtrans implementation
    virtual css::uno::Reference< css::uno::XInterface > CreateClipboard(
            const css::uno::Sequence< css::uno::Any >& i_rArguments ) override;
    virtual css::uno::Reference< css::uno::XInterface > CreateDragSource() override;
    virtual css::uno::Reference< css::uno::XInterface > CreateDropTarget() override;

    static void handleAppDefinedEvent( NSEvent* pEvent );

    // check whether a particular string is passed on the command line
    // this is needed to avoid duplicate open events through a) command line and b) NSApp's openFile
    static bool isOnCommandLine( const OUString& );

    void wakeupYield();

 public:
    friend class AquaSalFrame;

    void PostUserEvent( AquaSalFrame* pFrame, SalEvent nType, void* pData );
    void delayedSettingsChanged( bool bInvalidate );

    bool isNSAppThread() const;

    void startedPrintJob() { mnActivePrintJobs++; }
    void endedPrintJob() { mnActivePrintJobs--; }

    // event subtypes for NSApplicationDefined events
    static const short AppExecuteSVMain   = 0x7fff;
    static const short AppEndLoopEvent    = 1;
    static const short AppStartTimerEvent = 10;
    static const short YieldWakeupEvent   = 20;

    static NSMenu* GetDynamicDockMenu();
};

// helper class: inverted solar guard
class YieldMutexReleaser
{
    sal_uLong mnCount;
    public:
    YieldMutexReleaser();
    ~YieldMutexReleaser();
};

CGImageRef CreateCGImage( const Image& );
NSImage*   CreateNSImage( const Image& );

#endif // INCLUDED_VCL_INC_OSX_SALINST_H

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
