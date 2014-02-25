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

#ifndef INCLUDED_SHELL_INC_INTERNAL_PROPERTYHDL_HXX
#define INCLUDED_SHELL_INC_INTERNAL_PROPERTYHDL_HXX

#if defined _MSC_VER
#pragma warning(push, 1)
#pragma warning(disable:4917)
#endif
#include <shlobj.h>
#if defined _MSC_VER
#pragma warning(pop)
#endif

// {AE424E85-F6DF-4910-A6A9-438797986431}
const CLSID CLSID_PROPERTY_HANDLER =
{ 0xae424e85, 0xf6df, 0x4910, { 0xa6, 0xa9, 0x43, 0x87, 0x97, 0x98, 0x64, 0x31 } };

class CMetaInfoReader;

class CPropertyHdl : public IPropertyStore,
                     public IPropertyStoreCapabilities,
                     public IInitializeWithStream
{
public:
             CPropertyHdl(long RefCnt = 1);
    virtual ~CPropertyHdl();


    // IUnknown methods

    virtual HRESULT STDMETHODCALLTYPE QueryInterface(
            REFIID riid,
            void __RPC_FAR *__RPC_FAR *ppvObject );
    virtual ULONG STDMETHODCALLTYPE AddRef( void );
    virtual ULONG STDMETHODCALLTYPE Release( void );


    // IPropertyStore

    virtual HRESULT STDMETHODCALLTYPE GetCount( DWORD *pcProps );
    virtual HRESULT STDMETHODCALLTYPE GetAt( DWORD iProp, PROPERTYKEY *pkey );
    virtual HRESULT STDMETHODCALLTYPE GetValue( REFPROPERTYKEY key, PROPVARIANT *pPropVar );
    virtual HRESULT STDMETHODCALLTYPE SetValue( REFPROPERTYKEY key, REFPROPVARIANT propVar );
    virtual HRESULT STDMETHODCALLTYPE Commit();


    // IPropertyStoreCapabilities

    virtual HRESULT STDMETHODCALLTYPE IsPropertyWritable( REFPROPERTYKEY key );


    // IInitializeWithStream

    virtual HRESULT STDMETHODCALLTYPE Initialize(IStream *pStream, DWORD grfMode);

private:
    void        LoadProperties( CMetaInfoReader *pMetaInfoReader );
    HRESULT     GetItemData( CMetaInfoReader *pMetaInfoReader, UINT nIndex, PROPVARIANT *pVarData );

private:
    long m_RefCnt;
    IPropertyStoreCache* m_pCache;
};

class CClassFactory : public IClassFactory
{
public:
    CClassFactory( const CLSID& clsid );
    virtual ~CClassFactory();


    // IUnknown methods

    virtual HRESULT STDMETHODCALLTYPE QueryInterface(
            REFIID riid,
            void __RPC_FAR *__RPC_FAR *ppvObject);
    virtual ULONG STDMETHODCALLTYPE AddRef( void );
    virtual ULONG STDMETHODCALLTYPE Release( void );


    // IClassFactory methods

    virtual HRESULT STDMETHODCALLTYPE CreateInstance(
            IUnknown __RPC_FAR *pUnkOuter,
            REFIID riid,
            void __RPC_FAR *__RPC_FAR *ppvObject);

    virtual HRESULT STDMETHODCALLTYPE LockServer( BOOL fLock );
    static bool IsLocked();

private:
    long  m_RefCnt;
    CLSID m_Clsid;
    static long  s_ServerLocks;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
