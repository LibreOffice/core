/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: salinst.h,v $
 *
 *  $Revision: 1.17 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-07 16:38:00 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _SV_SALINST_H
#define _SV_SALINST_H

#include "vcl/sv.h"
#include "vos/mutex.hxx"
#include "vos/thread.hxx"
#include "vcl/salinst.hxx"

#include "aquavcltypes.h"

#include <list>

class AquaSalFrame;
class ApplicationEvent;

// -----------------
// - SalYieldMutex -
// -----------------

class SalYieldMutex : public vos::OMutex
{
    ULONG                                       mnCount;
    vos::OThread::TThreadIdentifier             mnThreadId;

public:
                                                SalYieldMutex();
    virtual void                                acquire();
    virtual void                                release();
    virtual sal_Bool                            tryToAcquire();
    ULONG                                       GetAcquireCount() const { return mnCount; }
    vos::OThread::TThreadIdentifier             GetThreadId() const { return mnThreadId; }
};

#define YIELD_GUARD vos::OGuard aGuard( GetSalData()->mpFirstInstance->GetYieldMutex() )


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
        USHORT          mnType;

        SalUserEvent( AquaSalFrame* pFrame, void* pData, USHORT nType ) :
            mpFrame( pFrame ), mpData( pData ), mnType( nType )
        {}
    };

public:
    SalYieldMutex*                          mpSalYieldMutex;        // Sal-Yield-Mutex
    rtl::OUString                           maDefaultPrinter;
    vos::OThread::TThreadIdentifier         maMainThread;
    bool                                    mbWaitingYield;
    std::list< SalUserEvent >               maUserEvents;
    oslMutex                                maUserEventListMutex;

    typedef std::list<const ApplicationEvent*> AppEventList;
    static AppEventList aAppEventList;

public:
    AquaSalInstance();
    virtual ~AquaSalInstance();

    virtual SalSystem*      CreateSystem();
    virtual void            DestroySystem(SalSystem*);
    virtual SalFrame*       CreateChildFrame( SystemParentData* pParent, ULONG nStyle );
    virtual SalFrame*       CreateFrame( SalFrame* pParent, ULONG nStyle );
    virtual void            DestroyFrame( SalFrame* pFrame );
    virtual SalObject*      CreateObject( SalFrame* pParent, SystemWindowData* pWindowData, BOOL bShow = TRUE );
    virtual void            DestroyObject( SalObject* pObject );
    virtual SalVirtualDevice*   CreateVirtualDevice( SalGraphics* pGraphics,
                                                     long nDX, long nDY,
                                                     USHORT nBitCount, const SystemGraphicsData *pData );
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
    virtual SalOpenGL*          CreateSalOpenGL( SalGraphics* pGraphics );
    virtual SalI18NImeStatus*   CreateI18NImeStatus();
    virtual SalSystem*          CreateSalSystem();
    virtual SalBitmap*          CreateSalBitmap();
    virtual vos::IMutex*        GetYieldMutex();
    virtual ULONG               ReleaseYieldMutex();
    virtual void                AcquireYieldMutex( ULONG nCount );
    virtual void                Yield( bool bWait, bool bHandleAllCurrentEvents );
    virtual bool                AnyInput( USHORT nType );
    virtual SalMenu*            CreateMenu( BOOL bMenuBar );
    virtual void                DestroyMenu( SalMenu* );
    virtual SalMenuItem*        CreateMenuItem( const SalItemParams* pItemData );
    virtual void                DestroyMenuItem( SalMenuItem* );
    virtual SalSession*         CreateSalSession();
    virtual void*               GetConnectionIdentifier( ConnectionIdentifierType& rReturnedType, int& rReturnedBytes );
    virtual void            SetEventCallback( void* pInstance, bool(*pCallback)(void*,void*,int) );
    virtual void            SetErrorEventCallback( void* pInstance, bool(*pCallback)(void*,void*,int) );

    static void handleAppDefinedEvent( NSEvent* pEvent );

    // check whether a particular string is passed on the command line
    // this is needed to avoid duplicate open events through a) command line and b) NSApp's openFile
    static bool isOnCommandLine( const rtl::OUString& );

    void wakeupYield();

 public:
    friend class AquaSalFrame;

    void PostUserEvent( AquaSalFrame* pFrame, USHORT nType, void* pData );
    void delayedSettingsChanged( bool bInvalidate );

    bool isNSAppThread() const;

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
    ULONG mnCount;
    public:
    YieldMutexReleaser();
    ~YieldMutexReleaser();
};

// helper class
rtl::OUString GetOUString( CFStringRef );
rtl::OUString GetOUString( NSString* );
CFStringRef CreateCFString( const rtl::OUString& );
NSString* CreateNSString( const rtl::OUString& );

#endif // _SV_SALINST_H
