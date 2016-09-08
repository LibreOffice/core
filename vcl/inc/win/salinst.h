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

public:
    WinSalInstance();
    virtual ~WinSalInstance();

    virtual SalFrame*       CreateChildFrame( SystemParentData* pParent, SalFrameStyleFlags nStyle ) override;
    virtual SalFrame*       CreateFrame( SalFrame* pParent, SalFrameStyleFlags nStyle ) override;
    virtual void            DestroyFrame( SalFrame* pFrame ) override;
    virtual SalObject*      CreateObject( SalFrame* pParent, SystemWindowData* pWindowData, bool bShow = true ) override;
    virtual void            DestroyObject( SalObject* pObject ) override;
    virtual SalVirtualDevice*   CreateVirtualDevice( SalGraphics* pGraphics,
                                                     long &nDX, long &nDY,
                                                     DeviceFormat eFormat, const SystemGraphicsData *pData ) override;
    virtual SalInfoPrinter* CreateInfoPrinter( SalPrinterQueueInfo* pQueueInfo,
                                               ImplJobSetup* pSetupData ) override;
    virtual void            DestroyInfoPrinter( SalInfoPrinter* pPrinter ) override;
    virtual SalPrinter*     CreatePrinter( SalInfoPrinter* pInfoPrinter ) override;
    virtual void            DestroyPrinter( SalPrinter* pPrinter ) override;
    virtual void            GetPrinterQueueInfo( ImplPrnQueueList* pList ) override;
    virtual void            GetPrinterQueueState( SalPrinterQueueInfo* pInfo ) override;
    virtual void            DeletePrinterQueueInfo( SalPrinterQueueInfo* pInfo ) override;
    virtual OUString            GetDefaultPrinter() override;
    virtual SalTimer*           CreateSalTimer() override;
    virtual SalSystem*          CreateSalSystem() override;
    virtual SalBitmap*          CreateSalBitmap() override;
    virtual comphelper::SolarMutex* GetYieldMutex() override;
    virtual sal_uIntPtr         ReleaseYieldMutex() override;
    virtual void                AcquireYieldMutex( sal_uIntPtr nCount ) override;
    virtual bool                CheckYieldMutex() override;

    virtual bool                DoYield(bool bWait, bool bHandleAllCurrentEvents, sal_uLong nReleased) override;
    virtual bool                AnyInput( VclInputFlags nType ) override;
    virtual SalMenu*            CreateMenu( bool bMenuBar, Menu* ) override;
    virtual void                DestroyMenu( SalMenu* ) override;
    virtual SalMenuItem*        CreateMenuItem( const SalItemParams* pItemData ) override;
    virtual void                DestroyMenuItem( SalMenuItem* ) override;
    virtual SalSession*         CreateSalSession() override;
    virtual OpenGLContext*      CreateOpenGLContext() override;
    virtual void*               GetConnectionIdentifier( ConnectionIdentifierType& rReturnedType, int& rReturnedBytes ) override;
    virtual void                AddToRecentDocumentList(const OUString& rFileUrl, const OUString& rMimeType, const OUString& rDocumentService) override;

    virtual OUString            getOSVersion() override;

    static int WorkaroundExceptionHandlingInUSER32Lib(int nExcept, LPEXCEPTION_POINTERS pExceptionInfo);
};

SalFrame* ImplSalCreateFrame( WinSalInstance* pInst, HWND hWndParent, SalFrameStyleFlags nSalFrameStyle );
SalObject* ImplSalCreateObject( WinSalInstance* pInst, WinSalFrame* pParent );
HWND ImplSalReCreateHWND( HWND hWndParent, HWND oldhWnd, bool bAsChild );
void ImplSalStartTimer( sal_uIntPtr nMS, bool bMutex = false );
void ImplSalStopTimer();

#endif // INCLUDED_VCL_INC_WIN_SALINST_H

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
