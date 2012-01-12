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



#ifndef _SV_SALINST_H
#define _SV_SALINST_H

#include <salinst.hxx>

namespace vos { class OMutex; }

// -------------------
// - SalInstanceData -
// -------------------

class SalYieldMutex;

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
    SalYieldMutex*          mpSalYieldMutex;        // Sal-Yield-Mutex
    vos::OMutex*            mpSalWaitMutex;         // Sal-Wait-Mutex
    USHORT              mnYieldWaitCount;       // Wait-Count

public:
    Os2SalInstance();
    virtual ~Os2SalInstance();

    virtual SalFrame*       CreateChildFrame( SystemParentData* pParent, ULONG nStyle );
    virtual SalFrame*       CreateFrame( SalFrame* pParent, ULONG nStyle );
    virtual void            DestroyFrame( SalFrame* pFrame );
    virtual SalObject*      CreateObject( SalFrame* pParent, SystemWindowData* pWindowData, sal_Bool bShow = TRUE );
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
    virtual SalI18NImeStatus*   CreateI18NImeStatus();
    virtual SalSystem*          CreateSalSystem();
    virtual SalBitmap*          CreateSalBitmap();
    virtual vos::IMutex*        GetYieldMutex();
    virtual ULONG               ReleaseYieldMutex();
    virtual void                AcquireYieldMutex( ULONG nCount );
    virtual bool                CheckYieldMutex();

    virtual void                Yield( bool, bool );
    virtual bool                AnyInput( USHORT nType );
    virtual SalMenu*            CreateMenu( sal_Bool bMenuBar, Menu* );
    virtual void                DestroyMenu( SalMenu* );
    virtual SalMenuItem*            CreateMenuItem( const SalItemParams* pItemData );
    virtual void                DestroyMenuItem( SalMenuItem* );
    virtual SalSession*                         CreateSalSession();
    virtual void*               GetConnectionIdentifier( ConnectionIdentifierType& rReturnedType, int& rReturnedBytes );
    virtual void                AddToRecentDocumentList(const rtl::OUString& rFileUrl, const rtl::OUString& rMimeType);
};

SalFrame* ImplSalCreateFrame( Os2SalInstance* pInst, HWND hWndParent, ULONG nSalFrameStyle );
SalObject* ImplSalCreateObject( Os2SalInstance* pInst, Os2SalFrame* pParent );
void ImplSalStartTimer( ULONG nMS, sal_Bool bMutex = FALSE );

#endif // _SV_SALINST_H
