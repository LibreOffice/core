/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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

#include <global.hxx>
#include <shlxthdl.hxx>
#include "classfactory.hxx"
#include <utilities.hxx>

#include <shlobj.h>

// Module global

LONG g_DllRefCnt = 0;
HINSTANCE g_hModule = nullptr;

// COM exports

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
    *ppv = nullptr;

    if (rclsid != CLSID_THUMBVIEWER_HANDLER)
        return CLASS_E_CLASSNOTAVAILABLE;

    if ((riid != IID_IUnknown) && (riid != IID_IClassFactory))
        return E_NOINTERFACE;

    OutputDebugStringFormatW( L"DllGetClassObject: Create CLSID_THUMBVIEWER_HANDLER\n" );

    IUnknown* pUnk = new CClassFactory(rclsid);
    *ppv = pUnk;
    return S_OK;
}

STDAPI DllCanUnloadNow()
{
    if (CClassFactory::IsLocked() || g_DllRefCnt > 0)
        return S_FALSE;

    return S_OK;
}

BOOL WINAPI DllMain(HINSTANCE hInst, ULONG /*ul_reason_for_call*/, LPVOID /*lpReserved*/)
{
    g_hModule = hInst;
    return TRUE;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
