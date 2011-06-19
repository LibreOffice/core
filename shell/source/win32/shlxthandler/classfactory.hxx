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

#ifndef CLASSFACTORY_HXX_INCLUDED
#define CLASSFACTORY_HXX_INCLUDED

#if defined _MSC_VER
#pragma warning(push, 1)
#endif
#include <objidl.h>
#if defined _MSC_VER
#pragma warning(pop)
#endif

class CClassFactory : public IClassFactory
{
public:
    CClassFactory(const CLSID& clsid);
    virtual ~CClassFactory();

    //-----------------------------
    // IUnknown methods
    //-----------------------------

    virtual HRESULT STDMETHODCALLTYPE QueryInterface(
            REFIID riid,
            void __RPC_FAR *__RPC_FAR *ppvObject);

    virtual ULONG STDMETHODCALLTYPE AddRef(void);

    virtual ULONG STDMETHODCALLTYPE Release(void);

    //-----------------------------
    // IClassFactory methods
    //-----------------------------

    virtual HRESULT STDMETHODCALLTYPE CreateInstance(
            IUnknown __RPC_FAR *pUnkOuter,
            REFIID riid,
            void __RPC_FAR *__RPC_FAR *ppvObject);

    virtual HRESULT STDMETHODCALLTYPE LockServer(BOOL fLock);

    static bool IsLocked();

private:
    long  m_RefCnt;
    CLSID m_Clsid;

    static long  s_ServerLocks;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
