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

class SalYieldMutex : public comphelper::SolarMutex
{
    osl::Mutex m_mutex;
    sal_uLong                                   mnCount;
    oslThreadIdentifier                         mnThreadId;

public:
                                                SalYieldMutex();
    virtual void                                acquire() SAL_OVERRIDE;
    virtual void                                release() SAL_OVERRIDE;
    virtual bool                                tryToAcquire() SAL_OVERRIDE;
    sal_uLong                                   GetAcquireCount() const { return mnCount; }
    oslThreadIdentifier                         GetThreadId() const { return mnThreadId; }
};

class AquaSalInstance : public SalInstance
{
    struct SalUserEvent
    {
        AquaSalFrame*   mpFrame;
        void*           mpData;
        sal_uInt16          mnType;

        SalUserEvent( AquaSalFrame* pFrame, void* pData, sal_uInt16 nType ) :
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
    oslMutex                                maUserEventListMutex;
    oslCondition                            maWaitingYieldCond;

    typedef std::list<const ApplicationEvent*> AppEventList;
    static AppEventList aAppEventList;

public:
    AquaSalInstance();
    virtual ~AquaSalInstance();

    virtual SalFrame*       CreateChildFrame( SystemParentData* pParent, SalFrameStyleFlags nStyle ) SAL_OVERRIDE;
    virtual SalFrame*       CreateFrame( SalFrame* pParent, SalFrameStyleFlags nStyle ) SAL_OVERRIDE;
    virtual void            DestroyFrame( SalFrame* pFrame ) SAL_OVERRIDE;
    virtual SalObject*      CreateObject( SalFrame* pParent, SystemWindowData* pWindowData,
                                          bool bShow = true ) SAL_OVERRIDE;
    virtual void            DestroyObject( SalObject* pObject ) SAL_OVERRIDE;
    virtual SalVirtualDevice* CreateVirtualDevice( SalGraphics* pGraphics,
                                                   long &nDX, long &nDY,
                                                   sal_uInt16 nBitCount,
                                                   const SystemGraphicsData *pData ) SAL_OVERRIDE;
    virtual SalInfoPrinter* CreateInfoPrinter( SalPrinterQueueInfo* pQueueInfo,
                                               ImplJobSetup* pSetupData ) SAL_OVERRIDE;
    virtual void            DestroyInfoPrinter( SalInfoPrinter* pPrinter ) SAL_OVERRIDE;
    virtual SalPrinter*     CreatePrinter( SalInfoPrinter* pInfoPrinter ) SAL_OVERRIDE;
    virtual void            DestroyPrinter( SalPrinter* pPrinter ) SAL_OVERRIDE;
    virtual void            GetPrinterQueueInfo( ImplPrnQueueList* pList ) SAL_OVERRIDE;
    virtual void            GetPrinterQueueState( SalPrinterQueueInfo* pInfo ) SAL_OVERRIDE;
    virtual void            DeletePrinterQueueInfo( SalPrinterQueueInfo* pInfo ) SAL_OVERRIDE;
    virtual OUString        GetDefaultPrinter() SAL_OVERRIDE;
    virtual SalTimer*       CreateSalTimer() SAL_OVERRIDE;
    virtual SalI18NImeStatus* CreateI18NImeStatus() SAL_OVERRIDE;
    virtual SalSystem*      CreateSalSystem() SAL_OVERRIDE;
    virtual SalBitmap*      CreateSalBitmap() SAL_OVERRIDE;
    virtual comphelper::SolarMutex* GetYieldMutex() SAL_OVERRIDE;
    virtual sal_uLong       ReleaseYieldMutex() SAL_OVERRIDE;
    virtual void            AcquireYieldMutex( sal_uLong nCount ) SAL_OVERRIDE;
    virtual bool            CheckYieldMutex() SAL_OVERRIDE;
    virtual void            DoYield(bool bWait, bool bHandleAllCurrentEvents,
                                    sal_uLong nReleased) SAL_OVERRIDE;
    virtual bool            AnyInput( VclInputFlags nType ) SAL_OVERRIDE;
    virtual SalMenu*        CreateMenu( bool bMenuBar, Menu* pVCLMenu ) SAL_OVERRIDE;
    virtual void            DestroyMenu( SalMenu* ) SAL_OVERRIDE;
    virtual SalMenuItem*    CreateMenuItem( const SalItemParams* pItemData ) SAL_OVERRIDE;
    virtual void            DestroyMenuItem( SalMenuItem* ) SAL_OVERRIDE;
    virtual SalSession*     CreateSalSession() SAL_OVERRIDE;
    virtual void*           GetConnectionIdentifier( ConnectionIdentifierType& rReturnedType,
                                                     int& rReturnedBytes ) SAL_OVERRIDE;
    virtual void            AddToRecentDocumentList(const OUString& rFileUrl, const OUString& rMimeType,
                                                    const OUString& rDocumentService) SAL_OVERRIDE;

    // dtrans implementation
    virtual css::uno::Reference< css::uno::XInterface > CreateClipboard(
            const css::uno::Sequence< css::uno::Any >& i_rArguments ) SAL_OVERRIDE;
    virtual css::uno::Reference< css::uno::XInterface > CreateDragSource() SAL_OVERRIDE;
    virtual css::uno::Reference< css::uno::XInterface > CreateDropTarget() SAL_OVERRIDE;

    static void handleAppDefinedEvent( NSEvent* pEvent );

    // check whether a particular string is passed on the command line
    // this is needed to avoid duplicate open events through a) command line and b) NSApp's openFile
    static bool isOnCommandLine( const OUString& );

    void wakeupYield();

 public:
    friend class AquaSalFrame;

    void PostUserEvent( AquaSalFrame* pFrame, sal_uInt16 nType, void* pData );
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
