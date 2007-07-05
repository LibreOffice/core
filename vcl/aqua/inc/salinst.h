/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: salinst.h,v $
 *
 *  $Revision: 1.12 $
 *
 *  last change: $Author: rt $ $Date: 2007-07-05 08:13:10 $
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

#ifndef _SV_SV_H
#include <vcl/sv.h>
#endif
#ifndef _VOS_MUTEX_HXX
#include <vos/mutex.hxx>
#endif
#ifndef _VOS_THREAD_HXX
#include <vos/thread.hxx>
#endif

#include <salsys.h>
#include <salobj.h>
#include <salvd.h>
#include <salsound.h>
#include <saltimer.h>
#include <salbmp.h>

#ifdef __cplusplus

class SalYieldMutex;

#else // __cplusplus

#define SalYieldMutex void

#endif // __cplusplus

// -----------------
// - SalYieldMutex -
// -----------------

class SalYieldMutex : public vos::OMutex
{
    ULONG                                       mnCount;
    NAMESPACE_VOS(OThread)::TThreadIdentifier   mnThreadId;

public:
                                                SalYieldMutex();
    virtual void                                acquire();
    virtual void                                release();
    virtual sal_Bool                            tryToAcquire();
    ULONG                                       GetAcquireCount() const { return mnCount; }
    NAMESPACE_VOS(OThread)::TThreadIdentifier   GetThreadId() const { return mnThreadId; }
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
public:
    void*               mpFilterInst;
    void*               mpFilterCallback;
    SalYieldMutex*      mpSalYieldMutex;        // Sal-Yield-Mutex
    rtl::OUString       maDefaultPrinter;
public:
    AquaSalInstance();
    virtual ~AquaSalInstance();

    virtual SalSystem*      CreateSystem();
    virtual void            DestroySystem(SalSystem*);
    virtual SalFrame*       CreateChildFrame( SystemParentData* pParent, ULONG nStyle );
    virtual SalFrame*       CreateFrame( SalFrame* pParent, ULONG nStyle );
    virtual void            DestroyFrame( SalFrame* pFrame );
    virtual SalObject*      CreateObject( SalFrame* pParent, SystemWindowData* pWindowData );
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
    virtual SalSound*           CreateSalSound();
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

    static void TimerEventHandler(EventLoopTimerRef inTimer, void* pData);

 public:
    /* During window resizing the standard event handler does
       not dispatch VCL timer messages which (for some strange
       reasons) trigger VCL painting. So when live resizing of
       windows is enabled the window content will not be painted
       at all especially when the user doesn't move the mouse
       anymore but still holds the left mouse button pressed on
       the resize area.
       So to get timer messages delivered nevertheless we setup
       a message loop timer. Events fired by this timer will also
       be delivered during window resizing.
    */
    void StartForceDispatchingPaintEvents();
    void StopForceDispatchingPaintEvents();
    EventLoopTimerRef mEventLoopTimerRef;
    bool mbForceDispatchPaintEvents;

    friend class AquaSalFrame;
};


// helper class
rtl::OUString GetOUString( CFStringRef );
CFStringRef CreateCFString( const rtl::OUString& );

#endif // _SV_SALINST_H
