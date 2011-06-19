/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
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

#ifndef PROPERTYHDL_HXX_INCLUDED
#define PROPERTYHDL_HXX_INCLUDED

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

    //-----------------------------
    // IUnknown methods
    //-----------------------------
    virtual HRESULT STDMETHODCALLTYPE QueryInterface(
            REFIID riid,
            void __RPC_FAR *__RPC_FAR *ppvObject );
    virtual ULONG STDMETHODCALLTYPE AddRef( void );
    virtual ULONG STDMETHODCALLTYPE Release( void );

    //-----------------------------
    // IPropertyStore
    //-----------------------------
    virtual HRESULT STDMETHODCALLTYPE GetCount( DWORD *pcProps );
    virtual HRESULT STDMETHODCALLTYPE GetAt( DWORD iProp, PROPERTYKEY *pkey );
    virtual HRESULT STDMETHODCALLTYPE GetValue( REFPROPERTYKEY key, PROPVARIANT *pPropVar );
    virtual HRESULT STDMETHODCALLTYPE SetValue( REFPROPERTYKEY key, REFPROPVARIANT propVar );
    virtual HRESULT STDMETHODCALLTYPE Commit();

    //-----------------------------
    // IPropertyStoreCapabilities
    //-----------------------------
    virtual HRESULT STDMETHODCALLTYPE IsPropertyWritable( REFPROPERTYKEY key );

    //-----------------------------
    // IInitializeWithStream
    //-----------------------------
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

    //-----------------------------
    // IUnknown methods
    //-----------------------------
    virtual HRESULT STDMETHODCALLTYPE QueryInterface(
            REFIID riid,
            void __RPC_FAR *__RPC_FAR *ppvObject);
    virtual ULONG STDMETHODCALLTYPE AddRef( void );
    virtual ULONG STDMETHODCALLTYPE Release( void );

    //-----------------------------
    // IClassFactory methods
    //-----------------------------
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
