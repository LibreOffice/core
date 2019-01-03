
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

#include <sal/config.h>

#include <condition_variable>
#include <list>
#include <mutex>

#include <comphelper/solarmutex.hxx>
#include <osl/conditn.hxx>
#include <osl/thread.hxx>

#ifdef MACOSX
#include <osx/osxvcltypes.h>
#endif
#include <salinst.hxx>

#include <osx/runinmain.hxx>

#include <salusereventlist.hxx>

class AquaSalFrame;
class SalFrame;
class SalObject;
class ApplicationEvent;
class Image;
enum class SalEvent;

typedef void(^RuninmainBlock)(void);

class SalYieldMutex : public comphelper::SolarMutex
{
public:
    OSX_RUNINMAIN_MEMBERS

protected:
    virtual void            doAcquire( sal_uInt32 nLockCount ) override;
    virtual sal_uInt32      doRelease( bool bUnlockAll ) override;

public:
    SalYieldMutex();
    virtual ~SalYieldMutex() override;

    virtual bool IsCurrentThread() const override;
};

class AquaSalInstance : public SalInstance, public SalUserEventList
{
    friend class AquaSalFrame;

    bool RunInMainYield( bool bHandleAllCurrentEvents );

    virtual void ProcessEvent( SalUserEvent aEvent ) override;

public:
    virtual void TriggerUserEventProcessing() override;

    OUString                                maDefaultPrinter;
    oslThreadIdentifier                     maMainThread;
    int                                     mnActivePrintJobs;
    osl::Mutex                              maUserEventListMutex;
    osl::Condition                          maWaitingYieldCond;
    bool                                    mbIsLiveResize;
    bool                                    mbNoYieldLock;
    bool                                    mbTimerProcessed;

    static std::list<const ApplicationEvent*> aAppEventList;

    AquaSalInstance();
    virtual ~AquaSalInstance() override;

    virtual void AfterAppInit() override;
    virtual bool SVMainHook(int *) override;

    virtual SalFrame*       CreateChildFrame( SystemParentData* pParent, SalFrameStyleFlags nStyle ) override;
    virtual SalFrame*       CreateFrame( SalFrame* pParent, SalFrameStyleFlags nStyle ) override;
    virtual void            DestroyFrame( SalFrame* pFrame ) override;
    virtual SalObject*      CreateObject( SalFrame* pParent, SystemWindowData* pWindowData,
                                          bool bShow ) override;
    virtual void            DestroyObject( SalObject* pObject ) override;
    virtual std::unique_ptr<SalVirtualDevice>
                            CreateVirtualDevice( SalGraphics* pGraphics,
                                                   long &nDX, long &nDY,
                                                   DeviceFormat eFormat,
                                                   const SystemGraphicsData *pData = nullptr ) override;
    virtual SalInfoPrinter* CreateInfoPrinter( SalPrinterQueueInfo* pQueueInfo,
                                               ImplJobSetup* pSetupData ) override;
    virtual void            DestroyInfoPrinter( SalInfoPrinter* pPrinter ) override;
    virtual std::unique_ptr<SalPrinter> CreatePrinter( SalInfoPrinter* pInfoPrinter ) override;
    virtual void            GetPrinterQueueInfo( ImplPrnQueueList* pList ) override;
    virtual void            GetPrinterQueueState( SalPrinterQueueInfo* pInfo ) override;
    virtual OUString        GetDefaultPrinter() override;
    virtual SalTimer*       CreateSalTimer() override;
    virtual SalSystem*      CreateSalSystem() override;
    virtual std::shared_ptr<SalBitmap> CreateSalBitmap() override;
    virtual bool            DoYield(bool bWait, bool bHandleAllCurrentEvents) override;
    virtual bool            AnyInput( VclInputFlags nType ) override;
    virtual std::unique_ptr<SalMenu>     CreateMenu( bool bMenuBar, Menu* pVCLMenu ) override;
    virtual std::unique_ptr<SalMenuItem> CreateMenuItem( const SalItemParams & rItemData ) override;
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

    void delayedSettingsChanged( bool bInvalidate );

    // Is this the NSAppThread?
    virtual bool IsMainThread() const override;

    void startedPrintJob() { mnActivePrintJobs++; }
    void endedPrintJob() { mnActivePrintJobs--; }

    // event subtypes for NSApplicationDefined events
    static const short AppExecuteSVMain   = 1;
    static const short AppStartTimerEvent = 10;
    static const short YieldWakeupEvent   = 20;
    static const short DispatchTimerEvent = 30;

    static NSMenu* GetDynamicDockMenu();
};

CGImageRef CreateCGImage( const Image& );
NSImage*   CreateNSImage( const Image& );

#endif // INCLUDED_VCL_INC_OSX_SALINST_H

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
