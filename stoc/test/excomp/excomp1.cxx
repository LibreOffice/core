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

#include <osl/diagnose.h>
#include <osl/mutex.hxx>
#include <rtl/alloc.h>
#include <cppuhelper/factory.hxx>
#include <cppuhelper/implbase2.hxx>

#include <example/XTest.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/registry/XRegistryKey.hpp>

using namespace example;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::registry;
using namespace cppu;
using namespace osl;

using ::rtl::OUString;

#define SERVICENAME1 "example.ExampleComponent1"
#define IMPLNAME1   "example.ExampleComponent1.Impl"

namespace excomp_impl {

//*************************************************************************
// ExampleComponent1Impl
//*************************************************************************
class ExampleComponent1Impl : public WeakImplHelper2< XTest, XServiceInfo >
{
public:
    ExampleComponent1Impl( const Reference<XMultiServiceFactory> & rXSMgr );

    ~ExampleComponent1Impl();

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName(  ) throw(RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw(RuntimeException);
    virtual Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) throw(RuntimeException);
    static Sequence< OUString > SAL_CALL getSupportedServiceNames_Static(  );

    // XSimpleRegistry
    virtual OUString SAL_CALL getMessage() throw(RuntimeException);

protected:
    Mutex       m_mutex;

    Reference<XMultiServiceFactory> m_xSMgr;
};

//*************************************************************************
ExampleComponent1Impl::ExampleComponent1Impl( const Reference<XMultiServiceFactory> & rXSMgr )
    : m_xSMgr(rXSMgr)
{
}

//*************************************************************************
ExampleComponent1Impl::~ExampleComponent1Impl()
{
}

//*************************************************************************
OUString SAL_CALL ExampleComponent1Impl::getImplementationName(  )
    throw(RuntimeException)
{
    Guard< Mutex > aGuard( m_mutex );
    return OUString( IMPLNAME1 );
}

//*************************************************************************
sal_Bool SAL_CALL ExampleComponent1Impl::supportsService( const OUString& ServiceName )
    throw(RuntimeException)
{
    Guard< Mutex > aGuard( m_mutex );
    Sequence< OUString > aSNL = getSupportedServiceNames();
    const OUString * pArray = aSNL.getArray();
    for( sal_Int32 i = 0; i < aSNL.getLength(); i++ )
        if( pArray[i] == ServiceName )
            return sal_True;
    return sal_False;
}

//*************************************************************************
Sequence<OUString> SAL_CALL ExampleComponent1Impl::getSupportedServiceNames(  )
    throw(RuntimeException)
{
    Guard< Mutex > aGuard( m_mutex );
    return getSupportedServiceNames_Static();
}

//*************************************************************************
Sequence<OUString> SAL_CALL ExampleComponent1Impl::getSupportedServiceNames_Static(  )
{
    OUString aName( SERVICENAME1 );
    return Sequence< OUString >( &aName, 1 );
}

//*************************************************************************
OUString SAL_CALL ExampleComponent1Impl::getMessage() throw(RuntimeException)
{
    Guard< Mutex > aGuard( m_mutex );
    return OUString("Lalelu nur der Mann im Mond schaut zu ...");
}


//*************************************************************************
Reference<XInterface> SAL_CALL ExampleComponent1_CreateInstance( const Reference<XMultiServiceFactory>& rSMgr )
{
    Reference<XInterface> xRet;

    XTest *pXTest = (XTest*) new ExampleComponent1Impl(rSMgr);

    if (pXTest)
    {
        xRet = Reference< XInterface >::query(pXTest);
    }

    return xRet;
}

} // excomp_impl


extern "C"
{
//==================================================================================================
sal_Bool SAL_CALL component_writeInfo(
    void * /* pServiceManager */ , void * pRegistryKey )
{
    if (pRegistryKey)
    {
        try
        {
            // ExampleComponent1
            Reference< XRegistryKey > xNewKey(
                reinterpret_cast< XRegistryKey * >( pRegistryKey )->createKey(
                    OUString( "/" IMPLNAME1 "/UNO/SERVICES" ) ) );

            const Sequence< OUString > & rSNL =
                ::excomp_impl::ExampleComponent1Impl::getSupportedServiceNames_Static();
            const OUString * pArray = rSNL.getConstArray();
            for ( sal_Int32 nPos = rSNL.getLength(); nPos--; )
                xNewKey->createKey( pArray[nPos] );

            return sal_True;
        }
        catch (InvalidRegistryException &)
        {
            OSL_FAIL( "### InvalidRegistryException!" );
        }
    }
    return sal_False;
}
//==================================================================================================
SAL_DLLPUBLIC_EXPORT void * SAL_CALL component_getFactory(
    const sal_Char * pImplName, void * pServiceManager, void * /* pRegistryKey */ )
{
    void * pRet = 0;

    if (rtl_str_compare( pImplName, IMPLNAME1 ) == 0)
    {
        Reference< XSingleServiceFactory > xFactory( createSingleFactory(
            reinterpret_cast< XMultiServiceFactory * >( pServiceManager ),
            OUString( IMPLNAME1 ),
            ::excomp_impl::ExampleComponent1_CreateInstance,
            ::excomp_impl::ExampleComponent1Impl::getSupportedServiceNames_Static() ) );

        if (xFactory.is())
        {
            xFactory->acquire();
            pRet = xFactory.get();
        }
    }

    return pRet;
}
}



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
