/*************************************************************************
 *
 *  $RCSfile: salinst.h,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: vg $ $Date: 2007-09-20 16:02:24 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _SV_SALINST_H
#define _SV_SALINST_H

#ifndef _SV_SV_H
#include <sv.h>
#endif

#ifndef _SV_SALINST_HXX
#include <salinst.hxx>
#endif

namespace vos { class OMutex; }

// -------------------
// - SalInstanceData -
// -------------------

class SalYieldMutex;
//class SalInstance;
//class SalFrame;
//class SalObject;

#define SAL_COMMANDLINENOINIT   ((USHORT)0xFFFF)
#define SAL_MAXPARAM            40

class Os2SalInstance : public SalInstance
{
public:
    HAB                     mhAB;                   // anchor block handle
    HMQ                     mhMQ;                   // handle of os2 message queue
    HPOINTER                mhAppIcon;              // app icon
    int                     mnArgc;                 // commandline param count
    char**                  mpArgv;                 // commandline
    HWND                    mhComWnd;               // window, for communication (between threads and the main thread)
    //void*                 mpFilterInst;           // hack for clipboard
    //void*                 mpFilterCallback;       // hack for clipboard
    SalYieldMutex*          mpSalYieldMutex;        // Sal-Yield-Mutex
    vos::OMutex*            mpSalWaitMutex;         // Sal-Wait-Mutex
    USHORT              mnYieldWaitCount;       // Wait-Count

public:
    Os2SalInstance();
    virtual ~Os2SalInstance();

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
    virtual void                Yield( bool, bool );
    virtual bool                AnyInput( USHORT nType );
    virtual SalMenu*                CreateMenu( BOOL bMenuBar );
    virtual void                DestroyMenu( SalMenu* );
    virtual SalMenuItem*            CreateMenuItem( const SalItemParams* pItemData );
    virtual void                DestroyMenuItem( SalMenuItem* );
    virtual SalSession*                         CreateSalSession();
    virtual void*               GetConnectionIdentifier( ConnectionIdentifierType& rReturnedType, int& rReturnedBytes );
};

SalFrame* ImplSalCreateFrame( Os2SalInstance* pInst, HWND hWndParent, ULONG nSalFrameStyle );
SalObject* ImplSalCreateObject( Os2SalInstance* pInst, Os2SalFrame* pParent );
void ImplSalStartTimer( ULONG nMS, BOOL bMutex = FALSE );

#endif // _SV_SALINST_H
