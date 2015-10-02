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

#ifndef INCLUDED_VCL_INC_WIN_SALINST_H
#define INCLUDED_VCL_INC_WIN_SALINST_H

#include <salinst.hxx>

class SalYieldMutex;

class WinSalInstance : public SalInstance
{
public:
    /// Instance Handle
    HINSTANCE           mhInst;
    /// invisible Window so non-main threads can SendMessage() the main thread
    HWND                mhComWnd;
    /// The Yield mutex ensures that only one thread calls into VCL
    SalYieldMutex*      mpSalYieldMutex;
    /// The Wait mutex ensures increment of mnYieldWaitCount and acquisition
    /// or release of mpSalYieldMutex is atomic
    osl::Mutex*         mpSalWaitMutex;
    /// count main thread's pending ImplSalYieldMutexAcquireWithWait() calls
    /// (it's not clear to me if this will be > 1 in practice; it would be
    /// possible if main thread's handling of SAL_MSG_* sent by other threads
    /// via SendMessage() ends up calling ImplSalYieldMutexAcquireWithWait())
    sal_uInt16          mnYieldWaitCount;

public:
    WinSalInstance();
    virtual ~WinSalInstance();

    virtual SalFrame*       CreateChildFrame( SystemParentData* pParent, SalFrameStyleFlags nStyle ) SAL_OVERRIDE;
    virtual SalFrame*       CreateFrame( SalFrame* pParent, SalFrameStyleFlags nStyle ) SAL_OVERRIDE;
    virtual void            DestroyFrame( SalFrame* pFrame ) SAL_OVERRIDE;
    virtual SalObject*      CreateObject( SalFrame* pParent, SystemWindowData* pWindowData, bool bShow = true ) SAL_OVERRIDE;
    virtual void            DestroyObject( SalObject* pObject ) SAL_OVERRIDE;
    virtual SalVirtualDevice*   CreateVirtualDevice( SalGraphics* pGraphics,
                                                     long &nDX, long &nDY,
                                                     sal_uInt16 nBitCount, const SystemGraphicsData *pData ) SAL_OVERRIDE;
    virtual SalInfoPrinter* CreateInfoPrinter( SalPrinterQueueInfo* pQueueInfo,
                                               ImplJobSetup* pSetupData ) SAL_OVERRIDE;
    virtual void            DestroyInfoPrinter( SalInfoPrinter* pPrinter ) SAL_OVERRIDE;
    virtual SalPrinter*     CreatePrinter( SalInfoPrinter* pInfoPrinter ) SAL_OVERRIDE;
    virtual void            DestroyPrinter( SalPrinter* pPrinter ) SAL_OVERRIDE;
    virtual void            GetPrinterQueueInfo( ImplPrnQueueList* pList ) SAL_OVERRIDE;
    virtual void            GetPrinterQueueState( SalPrinterQueueInfo* pInfo ) SAL_OVERRIDE;
    virtual void            DeletePrinterQueueInfo( SalPrinterQueueInfo* pInfo ) SAL_OVERRIDE;
    virtual OUString            GetDefaultPrinter() SAL_OVERRIDE;
    virtual SalTimer*           CreateSalTimer() SAL_OVERRIDE;
    virtual SalI18NImeStatus*   CreateI18NImeStatus() SAL_OVERRIDE;
    virtual SalSystem*          CreateSalSystem() SAL_OVERRIDE;
    virtual SalBitmap*          CreateSalBitmap() SAL_OVERRIDE;
    virtual comphelper::SolarMutex* GetYieldMutex() SAL_OVERRIDE;
    virtual sal_uIntPtr         ReleaseYieldMutex() SAL_OVERRIDE;
    virtual void                AcquireYieldMutex( sal_uIntPtr nCount ) SAL_OVERRIDE;
    virtual bool                CheckYieldMutex() SAL_OVERRIDE;

    virtual void                DoYield(bool bWait, bool bHandleAllCurrentEvents, sal_uLong nReleased) SAL_OVERRIDE;
    virtual bool                AnyInput( VclInputFlags nType ) SAL_OVERRIDE;
    virtual SalMenu*            CreateMenu( bool bMenuBar, Menu* ) SAL_OVERRIDE;
    virtual void                DestroyMenu( SalMenu* ) SAL_OVERRIDE;
    virtual SalMenuItem*        CreateMenuItem( const SalItemParams* pItemData ) SAL_OVERRIDE;
    virtual void                DestroyMenuItem( SalMenuItem* ) SAL_OVERRIDE;
    virtual SalSession*         CreateSalSession() SAL_OVERRIDE;
    virtual void*               GetConnectionIdentifier( ConnectionIdentifierType& rReturnedType, int& rReturnedBytes ) SAL_OVERRIDE;
    virtual void                AddToRecentDocumentList(const OUString& rFileUrl, const OUString& rMimeType, const OUString& rDocumentService) SAL_OVERRIDE;

    static int WorkaroundExceptionHandlingInUSER32Lib(int nExcept, LPEXCEPTION_POINTERS pExceptionInfo);
};

SalFrame* ImplSalCreateFrame( WinSalInstance* pInst, HWND hWndParent, SalFrameStyleFlags nSalFrameStyle );
SalObject* ImplSalCreateObject( WinSalInstance* pInst, WinSalFrame* pParent );
HWND ImplSalReCreateHWND( HWND hWndParent, HWND oldhWnd, bool bAsChild );
void ImplSalStartTimer( sal_uIntPtr nMS, bool bMutex = false );

#endif // INCLUDED_VCL_INC_WIN_SALINST_H

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
