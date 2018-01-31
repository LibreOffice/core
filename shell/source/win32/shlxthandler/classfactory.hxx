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

#ifndef INCLUDED_SHELL_SOURCE_WIN32_SHLXTHANDLER_CLASSFACTORY_HXX
#define INCLUDED_SHELL_SOURCE_WIN32_SHLXTHANDLER_CLASSFACTORY_HXX

#include <objidl.h>

class CClassFactory : public IClassFactory
{
public:
    explicit CClassFactory(const CLSID& clsid);
    virtual ~CClassFactory();


    // IUnknown methods


    virtual HRESULT STDMETHODCALLTYPE QueryInterface(
            REFIID riid,
            void __RPC_FAR *__RPC_FAR *ppvObject) override;

    virtual ULONG STDMETHODCALLTYPE AddRef() override;

    virtual ULONG STDMETHODCALLTYPE Release() override;


    // IClassFactory methods


    virtual HRESULT STDMETHODCALLTYPE CreateInstance(
            IUnknown __RPC_FAR *pUnkOuter,
            REFIID riid,
            void __RPC_FAR *__RPC_FAR *ppvObject) override;

    virtual HRESULT STDMETHODCALLTYPE LockServer(BOOL fLock) override;

    static bool IsLocked();

private:
    long  m_RefCnt;
    CLSID m_Clsid;

    static long  s_ServerLocks;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
