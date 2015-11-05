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

#include <rtl/alloc.h>
#include <osl/security.h>
#include <osl/thread.h>
#include <osl/mutex.hxx>
#include <cppuhelper/queryinterface.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <cppuhelper/weak.hxx>
#include <uno/mapping.hxx>


#include <cppuhelper/factory.hxx>
#include <cppuhelper/servicefactory.hxx>
#include <cppuhelper/implbase.hxx>
#include <registry/registry.hxx>

#include <com/sun/star/registry/XSimpleRegistry.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/registry/XImplementationRegistration.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/container/XContentEnumerationAccess.hpp>
#include <com/sun/star/container/XEnumerationAccess.hpp>
#include <com/sun/star/container/XEnumeration.hpp>

#if defined ( UNX )
#include <limits.h>
#define _MAX_PATH PATH_MAX
#endif


#define IMPLEMENTATION_NAME "com.sun.star.DummyService.V10"
#define SERVICE_NAME "com.sun.star.ts.TestManagerImpl"


using namespace css::uno;
using namespace css::registry;
using namespace css::lang;
using namespace css::container;
using namespace osl;
using namespace cppu;



Reference<XMultiServiceFactory> getProcessServiceManager()
{
    Reference<XMultiServiceFactory > s_x;
    if (! s_x.is())
    {
        MutexGuard aGuard( Mutex::getGlobalMutex() );
        if (! s_x.is())
            s_x = createRegistryServiceFactory( OUString("stoctest.rdb"), sal_False );
    }
    return s_x;
}

Reference< XMultiServiceFactory > createRegistryServiceManager( const OUString& registryName )
{
    return createRegistryServiceFactory( registryName );
}


/**********************************
* The service, that is used to test the Service manager
*
*
*
*************************************/
static sal_uInt32 nInstanceCount = 0;
class Test_Manager_Impl : public WeakImplHelper< XServiceInfo >
{
public:
    Test_Manager_Impl(){ nInstanceCount++;}
    ~Test_Manager_Impl();

    // XServiceInfo
    OUString                    SAL_CALL getImplementationName() throw();
    sal_Bool                    SAL_CALL supportsService(const OUString& ServiceName) throw();
    Sequence< OUString >        SAL_CALL getSupportedServiceNames() throw();
    static Sequence< OUString > SAL_CALL getSupportedServiceNames_Static() throw();

private:
//  static XIdlClassRef     getStaticIdlClass();
};

Test_Manager_Impl::~Test_Manager_Impl()
{
    nInstanceCount--;
}


// old, is no longer needed by the new Mimic
Reference< XInterface > SAL_CALL Test_Manager_Impl_CreateInstance_Impl()
{
    return (OWeakObject *)new Test_Manager_Impl();
}



// Test_Manager_Impl_CreateInstance()

Reference < XInterface > SAL_CALL Test_Manager_Impl_CreateInstance(
    const Reference< XMultiServiceFactory > & /*rSMgr*/ ) throw (Exception)
{
    Reference < XInterface >  xService = (XWeak *)(OWeakObject *)new Test_Manager_Impl( );

    return xService;
}



// Test_Manager_Impl::getImplementationName

OUString Test_Manager_Impl::getImplementationName() throw()
{
    return OUString(IMPLEMENTATION_NAME);
}

// Test_Manager_Impl::supportsService
sal_Bool Test_Manager_Impl::supportsService( const OUString& ServiceName ) throw()
{
    return cppu::supportsService(this, ServiceName);
}


// Test_Manager_Impl::getSupportedServiceNames

Sequence< OUString > Test_Manager_Impl::getSupportedServiceNames() throw ()
{
    return getSupportedServiceNames_Static();
}


// Test_Manager_Impl::getSupportedServiceNames_Static

Sequence< OUString > Test_Manager_Impl::getSupportedServiceNames_Static() throw ()
{
    Sequence< OUString > aSNS( 2 );
    aSNS.getArray()[0] = SERVICE_NAME;
    aSNS.getArray()[1] = "com.sun.star.bridge.Bridge";
    return aSNS;
}




/****
*
*
*  This routine performs the test of the process service manager ( getProcessServiceManager is called )
*
*
*
****/

#include <stdio.h>

extern "C" void SAL_CALL test_ServiceManager()
{
#if ! defined SAL_DLLPREFIX
#define SAL_DLLPREFIX ""
#endif
     OUString atUModule2 = SAL_DLLPREFIX "testsmgr_component" SAL_DLLEXTENSION ;

    // expand shared library name
    OString  atModule2( OUStringToOString(atUModule2, RTL_TEXTENCODING_ASCII_US) );

    // get the process servicemanager
    Reference <XMultiServiceFactory>  xSMgr = getProcessServiceManager();

    OSL_ENSURE( xSMgr.is() , "query on XServiceManager failed" );

    Reference<XContentEnumerationAccess> xContEnum(xSMgr, UNO_QUERY);
    OSL_ENSURE( xContEnum.is() , "query on XContentEnumerationAccess failed" );
    Reference<XEnumeration > xEnum(xContEnum->createContentEnumeration(OUString("com.sun.star.registry.SimpleRegistry")));
    OSL_ENSURE( xEnum.is() , "createContentEnumeration failed" );
    sal_Int32 nLen = 0;
    while( xEnum->hasMoreElements() )
    {
        nLen++;
        xEnum->nextElement();
    }
    OSL_ENSURE( nLen == 1, "more than one implementation for SimpleRegistry" );

    Reference<XEnumerationAccess> xImplEnum(xSMgr, UNO_QUERY);
    OSL_ENSURE( xImplEnum.is() , "query on XEnumeration failed" );
    xEnum.set(xImplEnum->createEnumeration());
    OSL_ENSURE( xEnum.is() , "createEnumeration failed" );
    nLen = 0;
    while( xEnum->hasMoreElements() )
    {
        nLen++;
        Reference< XServiceInfo > sf( xEnum->nextElement(), UNO_QUERY );
        OString str( OUStringToOString( sf->getImplementationName(), RTL_TEXTENCODING_ASCII_US ) );
        ::fprintf( stderr, "> implementation name: %s\n", str.getStr() );
    }
    OSL_ENSURE( nLen == 8, "more than 6 factories" );

    // try to get an instance for a unknown service
    OSL_VERIFY( !xSMgr->createInstance("bla.blup.Q").is() );


    // First test : register service via the internal function of the component itself

    {
        Reference< XImplementationRegistration >
            xInst( xSMgr->createInstance("com.sun.star.registry.ImplementationRegistration"), UNO_QUERY );
        OSL_ENSURE( xInst.is(), "no ImplementationRegistration" );

        try {
            // register the services via writeComponentRegInfo (see at end of this file)
            xInst->registerImplementation(OUString("com.sun.star.loader.SharedLibrary"), atUModule2, Reference< XSimpleRegistry >() );
        }
        catch(const CannotRegisterImplementationException &) {
            OSL_ENSURE( 0, "register implementation failed" );
        }

        // getImplementations() check
         Sequence<OUString> seqImpl = xInst->getImplementations(OUString("com.sun.star.loader.SharedLibrary"), atUModule2);
        OSL_ENSURE( seqImpl.getLength() == 1, "count of implementantions is wrong" );
        OSL_ENSURE( seqImpl.getConstArray()[0] == "com.sun.star.DummyService.V10", "implementation name is not equal" );


        // tests, if a service provider can be instantiated.

        Reference< XInterface > xIFace(xSMgr->createInstance("com.sun.star.ts.TestManagerImpl"));
        OSL_ENSURE( xIFace.is(), "loadable service not found" );

        // remove the service
        OSL_VERIFY( xInst->revokeImplementation(atUModule2, Reference< XSimpleRegistry > ()) );
    }

    Reference<XComponent> xComp(xSMgr, UNO_QUERY);
    xComp->dispose();

    xComp.clear();
    xSMgr.clear();
}



extern "C"
{

sal_Bool SAL_CALL component_writeInfo(
    void * /*pServiceManager*/, void * pRegistryKey )
{
    if (pRegistryKey)
    {
        try
        {
            Reference< XRegistryKey > xNewKey(
                reinterpret_cast< XRegistryKey * >( pRegistryKey )->createKey(
                    OUString( "/" IMPLEMENTATION_NAME "/UNO/SERVICES" ) ) );

            const Sequence< OUString > & rSNL =
                Test_Manager_Impl::getSupportedServiceNames_Static();
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

SAL_DLLPUBLIC_EXPORT void * SAL_CALL component_getFactory(
    const sal_Char * pImplName, void * pServiceManager, void * /*pRegistryKey*/ )
{
    void * pRet = 0;

    if (rtl_str_compare( pImplName, IMPLEMENTATION_NAME ) == 0)
    {
        Reference< XSingleServiceFactory > xFactory( createSingleFactory(
            reinterpret_cast< XMultiServiceFactory * >( pServiceManager ),
            OUString( IMPLEMENTATION_NAME ),
            Test_Manager_Impl_CreateInstance,
            Test_Manager_Impl::getSupportedServiceNames_Static() ) );

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
