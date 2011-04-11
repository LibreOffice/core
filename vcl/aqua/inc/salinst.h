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

#ifndef _SV_SALINST_H
#define _SV_SALINST_H

#include "vcl/sv.h"
#include "osl/mutex.hxx"
#include "osl/thread.hxx"
#include "vcl/salinst.hxx"
#include "osl/conditn.h"
#include <vcl/solarmutex.hxx>

#include "aquavcltypes.h"

#include <list>

class AquaSalFrame;
class ApplicationEvent;
class Image;

// -----------------
// - SalYieldMutex -
// -----------------

class SalYieldMutex : public vcl::SolarMutexObject
{
    sal_uLong                                       mnCount;
    oslThreadIdentifier                         mnThreadId;

public:
                                                SalYieldMutex();
    virtual void                                acquire();
    virtual void                                release();
    virtual sal_Bool                            tryToAcquire();
    sal_uLong                                       GetAcquireCount() const { return mnCount; }
    oslThreadIdentifier                         GetThreadId() const { return mnThreadId; }
};

#define YIELD_GUARD osl::SolarGuard aGuard( GetSalData()->mpFirstInstance->GetYieldMutex() )


// -------------------
// - SalInstanceData -
// -------------------

//struct SalInstanceData
//{
//public:
//};

// ------------------
// - AquaSalInstance -
// ------------------

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
    rtl::OUString                           maDefaultPrinter;
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

    virtual SalSystem*      CreateSystem();
    virtual void            DestroySystem(SalSystem*);
    virtual SalFrame*       CreateChildFrame( SystemParentData* pParent, sal_uLong nStyle );
    virtual SalFrame*       CreateFrame( SalFrame* pParent, sal_uLong nStyle );
    virtual void            DestroyFrame( SalFrame* pFrame );
    virtual SalObject*      CreateObject( SalFrame* pParent, SystemWindowData* pWindowData, sal_Bool bShow = sal_True );
    virtual void            DestroyObject( SalObject* pObject );
    virtual SalVirtualDevice*   CreateVirtualDevice( SalGraphics* pGraphics,
                                                     long nDX, long nDY,
                                                     sal_uInt16 nBitCount, const SystemGraphicsData *pData );
    virtual void            DestroyVirtualDevice( SalVirtualDevice* pDevice );

    virtual SalInfoPrinter* CreateInfoPrinter( SalPrinterQueueInfo* pQueueInfo,
                                               ImplJobSetup* pSetupData );
    virtual void            DestroyInfoPrinter( SalInfoPrinter* pPrinter );
    virtual SalPrinter*     CreatePrinter( SalInfoPrinter* pInfoPrinter );
    virtual void            DestroyPrinter( SalPrinter* pPrinter );
    virtual void            GetPrinterQueueInfo( ImplPrnQueueList* pList );
    virtual void            GetPrinterQueueState( SalPrinterQueueInfo* pInfo );
    virtual void            DeletePrinterQueueInfo( SalPrinterQueueInfo* pInfo );
    virtual String             GetDefaultPrinter();
    virtual SalTimer*           CreateSalTimer();
    virtual SalI18NImeStatus*   CreateI18NImeStatus();
    virtual SalSystem*          CreateSalSystem();
    virtual SalBitmap*          CreateSalBitmap();
    virtual osl::SolarMutex*    GetYieldMutex();
    virtual sal_uLong               ReleaseYieldMutex();
    virtual void                AcquireYieldMutex( sal_uLong nCount );
    virtual bool                CheckYieldMutex();
    virtual void                Yield( bool bWait, bool bHandleAllCurrentEvents );
    virtual bool                AnyInput( sal_uInt16 nType );
    virtual SalMenu*            CreateMenu( sal_Bool bMenuBar, Menu* pVCLMenu );
    virtual void                DestroyMenu( SalMenu* );
    virtual SalMenuItem*        CreateMenuItem( const SalItemParams* pItemData );
    virtual void                DestroyMenuItem( SalMenuItem* );
    virtual SalSession*         CreateSalSession();
    virtual void*               GetConnectionIdentifier( ConnectionIdentifierType& rReturnedType, int& rReturnedBytes );
    virtual void                AddToRecentDocumentList(const rtl::OUString& rFileUrl, const rtl::OUString& rMimeType);
    virtual void            SetEventCallback( void* pInstance, bool(*pCallback)(void*,void*,int) );
    virtual void            SetErrorEventCallback( void* pInstance, bool(*pCallback)(void*,void*,int) );

    // dtrans implementation
    virtual com::sun::star::uno::Reference< com::sun::star::uno::XInterface >
        CreateClipboard( const com::sun::star::uno::Sequence< com::sun::star::uno::Any >& i_rArguments );
    virtual com::sun::star::uno::Reference< com::sun::star::uno::XInterface > CreateDragSource();
    virtual com::sun::star::uno::Reference< com::sun::star::uno::XInterface > CreateDropTarget();

    static void handleAppDefinedEvent( NSEvent* pEvent );

    // check whether a particular string is passed on the command line
    // this is needed to avoid duplicate open events through a) command line and b) NSApp's openFile
    static bool isOnCommandLine( const rtl::OUString& );

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
    static const short AppleRemoteEvent   = 15;
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

// helper class
rtl::OUString GetOUString( CFStringRef );
rtl::OUString GetOUString( NSString* );
CFStringRef CreateCFString( const rtl::OUString& );
NSString* CreateNSString( const rtl::OUString& );

CGImageRef CreateCGImage( const Image& );
NSImage*   CreateNSImage( const Image& );

#endif // _SV_SALINST_H

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
