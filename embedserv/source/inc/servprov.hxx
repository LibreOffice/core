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



#ifndef _SERVPROV_HXX
#define _SERVPROV_HXX

#if defined(_MSC_VER) && (_MSC_VER >= 1300)
#undef _DEBUG
#endif

#include "common.h"
#include <oleidl.h>
#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/uno/Sequence.h>
#include <com/sun/star/uno/XInterface.hpp>
#ifndef __COM_SUN_STAR_LANG_XTYPEPROVIDER_HPP_
#include <com/sun/star/lang/XTypeProvider.hpp>
#endif
#include <cppuhelper/weak.hxx>


class EmbedProviderFactory_Impl;

class EmbedServer_Impl : public ::cppu::OWeakObject, ::com::sun::star::lang::XTypeProvider
{
public:
    EmbedServer_Impl( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > &xFactory );
    virtual ~EmbedServer_Impl();

    // XInterface
    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type& aType )
        throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL acquire() throw ();
    virtual void SAL_CALL release() throw ();

    // XTypeProvider
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes( )
        throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId()
        throw(::com::sun::star::uno::RuntimeException);

protected:

    CComPtr< EmbedProviderFactory_Impl > m_pOLEFactories[ SUPPORTED_FACTORIES_NUM ];
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > m_xFactory;
};

class EmbedProviderFactory_Impl : public IClassFactory
{
public:

    EmbedProviderFactory_Impl( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xFactory, const GUID* pGuid);
    virtual ~EmbedProviderFactory_Impl();

    sal_Bool registerClass();
    sal_Bool deregisterClass();

    /* IUnknown methods */
    STDMETHOD(QueryInterface)(REFIID riid, LPVOID FAR * ppvObj);
    STDMETHOD_(ULONG, AddRef)();
    STDMETHOD_(ULONG, Release)();

    /* IClassFactory methods */
    STDMETHOD(CreateInstance)(IUnknown FAR* punkOuter, REFIID riid, void FAR* FAR* ppv);
    STDMETHOD(LockServer)(int fLock);

protected:

    oslInterlockedCount m_refCount;
    GUID                m_guid;
    DWORD               m_factoryHandle;

    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > m_xFactory;
};

#endif
