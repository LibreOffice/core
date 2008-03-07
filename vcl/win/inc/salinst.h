/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: salinst.h,v $
 *
 *  $Revision: 1.15 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-07 16:47:32 $
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

#ifndef _SV_SALINST_HXX
#include <vcl/salinst.hxx>
#endif

namespace vos { class OMutex; }

// -------------------
// - SalInstanceData -
// -------------------

class SalYieldMutex;

class WinSalInstance : public SalInstance
{
public:
    HINSTANCE           mhInst;                 // Instance Handle
    HWND                mhComWnd;               // window, for communication (between threads and the main thread)
    SalYieldMutex*      mpSalYieldMutex;        // Sal-Yield-Mutex
    vos::OMutex*        mpSalWaitMutex;         // Sal-Wait-Mutex
    USHORT              mnYieldWaitCount;       // Wait-Count
public:
    WinSalInstance();
    virtual ~WinSalInstance();

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
    virtual SalMenu*                CreateMenu( BOOL bMenuBar );
    virtual void                DestroyMenu( SalMenu* );
    virtual SalMenuItem*            CreateMenuItem( const SalItemParams* pItemData );
    virtual void                DestroyMenuItem( SalMenuItem* );
    virtual SalSession*                         CreateSalSession();
    virtual void*               GetConnectionIdentifier( ConnectionIdentifierType& rReturnedType, int& rReturnedBytes );

    static int WorkaroundExceptionHandlingInUSER32Lib(int nExcept, LPEXCEPTION_POINTERS pExceptionInfo);
};

// --------------
// - Prototypen -
// --------------

SalFrame* ImplSalCreateFrame( WinSalInstance* pInst, HWND hWndParent, ULONG nSalFrameStyle );
SalObject* ImplSalCreateObject( WinSalInstance* pInst, WinSalFrame* pParent );
HWND ImplSalReCreateHWND( HWND hWndParent, HWND oldhWnd, BOOL bAsChild );
void ImplSalStartTimer( ULONG nMS, BOOL bMutex = FALSE );
void ImplSalPrinterAbortJobAsync( HDC hPrnDC );

#endif // _SV_SALINST_H
