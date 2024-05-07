/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 *  The Contents of this file are made available subject to the terms of
 *  the BSD license.
 *
 *  Copyright 2000, 2010 Oracle and/or its affiliates.
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *  3. Neither the name of Sun Microsystems, Inc. nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 *  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 *  COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 *  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 *  OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 *  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
 *  TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 *  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *************************************************************************/

/*
 * service implementation:   foo.Counter
 * exported interfaces:      foo.XCounter
 *
 * simple example component implementing a counter
 */

#include <iostream>
#include <rtl/ustring.hxx>
#include <uno/lbnames.h>
#include <cppuhelper/queryinterface.hxx>
#include <cppuhelper/factory.hxx>
#include <cppuhelper/supportsservice.hxx>
// generated c++ interfaces
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/registry/XRegistryKey.hpp>
#include <foo/XCountable.hpp>

#define SERVICENAME "foo.Counter"
#define IMPLNAME    "com.sun.star.comp.example.cpp.Counter"

using namespace ::rtl;
using namespace ::osl;
using namespace ::cppu;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::registry;
using namespace ::foo;

class MyCounterImpl
    : public XCountable
    , public XServiceInfo
{
    // to obtain other services if needed
    Reference< XMultiServiceFactory > m_xServiceManager;

    sal_Int32 m_nRefCount;
    sal_Int32 m_nCount;

public:
    MyCounterImpl( const Reference< XMultiServiceFactory > & xServiceManager )
        : m_xServiceManager( xServiceManager ), m_nRefCount( 0 )
        { std::cout << "< MyCounterImpl ctor called >" << std::endl; }
    ~MyCounterImpl()
        { std::cout << "< MyCounterImpl dtor called >" << std::endl; }

    // XInterface implementation
    virtual void SAL_CALL acquire() SAL_NOEXCEPT
        { ++m_nRefCount; }
    virtual void SAL_CALL release() SAL_NOEXCEPT
        { if (! --m_nRefCount) delete this; }
    virtual Any SAL_CALL queryInterface( const Type & rType )
        { return cppu::queryInterface(rType,
                                      static_cast< XInterface* >( static_cast< XServiceInfo* >( this ) ),
                                      static_cast< XCountable* >( this ),
                                      static_cast< XServiceInfo* >( this ) ); }

    // XServiceInfo implementation
    virtual OUString SAL_CALL getImplementationName(  );
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName );
    virtual Sequence< OUString > SAL_CALL getSupportedServiceNames(  );
    static Sequence< OUString > SAL_CALL getSupportedServiceNames_Static(  );

    // XCountable implementation
    virtual sal_Int32 SAL_CALL getCount()
        { return m_nCount; }
    virtual void SAL_CALL setCount( sal_Int32 nCount )
        { m_nCount = nCount; }
    virtual sal_Int32 SAL_CALL increment()
        { return (++m_nCount); }
    virtual sal_Int32 SAL_CALL decrement()
        { return (--m_nCount); }
};

OUString SAL_CALL MyCounterImpl::getImplementationName(  )
{
    return OUString( IMPLNAME );
}

sal_Bool SAL_CALL MyCounterImpl::supportsService( const OUString& ServiceName )
{
    return cppu::supportsService(this, ServiceName);
}

Sequence<OUString> SAL_CALL MyCounterImpl::getSupportedServiceNames(  )
{
    return getSupportedServiceNames_Static();
}

Sequence<OUString> SAL_CALL MyCounterImpl::getSupportedServiceNames_Static(  )
{
    OUString aName( SERVICENAME );
    return Sequence< OUString >( &aName, 1 );
}

/**
 * Function to create a new component instance; is needed by factory helper implementation.
 * @param xMgr service manager to if the components needs other component instances
 */
Reference< XInterface > SAL_CALL MyCounterImpl_create(
    const Reference< XMultiServiceFactory > & xMgr )
{
    return Reference<XInterface>(static_cast<XCountable*>(new MyCounterImpl(xMgr)));
}

/**
 * This function is called to get service factories for an implementation.
 *
 * @param pImplName       name of implementation
 * @param pServiceManager a service manager, need for component creation
 * @param pRegistryKey    the registry key for this component, need for persistent data
 * @return a component factory
 */
extern "C" SAL_DLLPUBLIC_EXPORT void * SAL_CALL component_getFactory(const char * pImplName, void * pServiceManager, void * pRegistryKey)
{
    void * pRet = 0;

    if (rtl_str_compare( pImplName, IMPLNAME ) == 0)
    {
        Reference< XSingleServiceFactory > xFactory( createSingleFactory(
            reinterpret_cast< XMultiServiceFactory * >( pServiceManager ),
            OUString( IMPLNAME ),
            MyCounterImpl_create,
            MyCounterImpl::getSupportedServiceNames_Static() ) );

        if (xFactory.is())
        {
            xFactory->acquire();
            pRet = xFactory.get();
        }
    }

    return pRet;
}

extern "C" SAL_DLLPUBLIC_EXPORT void SAL_CALL
component_getImplementationEnvironment(
    char const ** ppEnvTypeName, uno_Environment **)
{
    *ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
