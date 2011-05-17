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

#include <vcl/sv.h>
#include <vcl/salinst.hxx>
#include <vcl/solarmutex.hxx>

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
    osl::Mutex*             mpSalWaitMutex;         // Sal-Wait-Mutex
    USHORT              mnYieldWaitCount;       // Wait-Count

public:
    Os2SalInstance();
    virtual ~Os2SalInstance();

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
    virtual SalI18NImeStatus*   CreateI18NImeStatus();
    virtual SalSystem*          CreateSalSystem();
    virtual SalBitmap*          CreateSalBitmap();
    virtual osl::SolarMutex*    GetYieldMutex();
    virtual ULONG               ReleaseYieldMutex();
    virtual void                AcquireYieldMutex( ULONG nCount );
    virtual bool                CheckYieldMutex();
    virtual void                Yield( bool, bool );
    virtual bool                AnyInput( USHORT nType );
    virtual SalSession*                         CreateSalSession();
    virtual void*               GetConnectionIdentifier( ConnectionIdentifierType& rReturnedType, int& rReturnedBytes );
    virtual void                AddToRecentDocumentList(const rtl::OUString& rFileUrl, const rtl::OUString& rMimeType);
};

SalFrame* ImplSalCreateFrame( Os2SalInstance* pInst, HWND hWndParent, ULONG nSalFrameStyle );
SalObject* ImplSalCreateObject( Os2SalInstance* pInst, Os2SalFrame* pParent );
void ImplSalStartTimer( ULONG nMS, BOOL bMutex = FALSE );

#endif // _SV_SALINST_H

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
