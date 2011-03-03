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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_stoc.hxx"
#include <rtl/alloc.h>
#include <osl/security.h>
#include <osl/thread.h>
#include <osl/mutex.hxx>
#include <cppuhelper/queryinterface.hxx>
#include <cppuhelper/weak.hxx>
#include <uno/mapping.hxx>


#include <cppuhelper/factory.hxx>
#include <cppuhelper/servicefactory.hxx>
#include <cppuhelper/implbase1.hxx>
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



#if OSL_DEBUG_LEVEL > 0
#define TEST_ENSHURE(c, m)   OSL_ENSURE(c, m)
#else
#define TEST_ENSHURE(c, m)   OSL_VERIFY(c)
#endif


#define IMPLEMENTATION_NAME "com.sun.star.DummyService.V10"
#define SERVICE_NAME "com.sun.star.ts.TestManagerImpl"


using namespace com::sun::star::uno;
using namespace com::sun::star::registry;
using namespace com::sun::star::lang;
using namespace com::sun::star::container;
using namespace osl;
using namespace cppu;

using ::rtl::OUString;
using ::rtl::OUStringToOString;
using ::rtl::OString;


Reference<XMultiServiceFactory> getProcessServiceManager()
{
    Reference<XMultiServiceFactory > s_x;
    if (! s_x.is())
    {
        MutexGuard aGuard( Mutex::getGlobalMutex() );
        if (! s_x.is())
            s_x = createRegistryServiceFactory( OUString(RTL_CONSTASCII_USTRINGPARAM("stoctest.rdb")), sal_False );
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
class Test_Manager_Impl : public WeakImplHelper1< XServiceInfo >
{
public:
    Test_Manager_Impl(){ nInstanceCount++;}
    ~Test_Manager_Impl();

    // XServiceInfo
    OUString                    SAL_CALL getImplementationName() throw();
    sal_Bool                    SAL_CALL supportsService(const OUString& ServiceName) throw();
    Sequence< OUString >        SAL_CALL getSupportedServiceNames(void) throw();
    static Sequence< OUString > SAL_CALL getSupportedServiceNames_Static(void) throw();

private:
//  static XIdlClassRef     getStaticIdlClass();
};

Test_Manager_Impl::~Test_Manager_Impl()
{
    nInstanceCount--;
}


// alt, wird von der neuen Mimic nicht mehr gebraucht
Reference< XInterface > SAL_CALL Test_Manager_Impl_CreateInstance_Impl()
{
    return (OWeakObject *)new Test_Manager_Impl();
}


//*************************************************************************
// Test_Manager_Impl_CreateInstance()
//
Reference < XInterface > SAL_CALL Test_Manager_Impl_CreateInstance(
    const Reference< XMultiServiceFactory > & /*rSMgr*/ ) throw (Exception)
{
    Reference < XInterface >  xService = (XWeak *)(OWeakObject *)new Test_Manager_Impl( );

    return xService;
}


//*************************************************************************
// Test_Manager_Impl::getImplementationName
//
OUString Test_Manager_Impl::getImplementationName() throw()
{
    return OUString(RTL_CONSTASCII_USTRINGPARAM(IMPLEMENTATION_NAME));
}

//*************************************************************************
// Test_Manager_Impl::supportsService
//
sal_Bool Test_Manager_Impl::supportsService( const OUString& ServiceName ) throw()
{
    Sequence< OUString > aSNL = getSupportedServiceNames();
    const OUString * pArray = aSNL.getConstArray();
    for( sal_Int32 i = 0; i < aSNL.getLength(); i++ )
        if( pArray[i] == ServiceName )
            return sal_True;
    return sal_False;
}

//*************************************************************************
// Test_Manager_Impl::getSupportedServiceNames
//
Sequence< OUString > Test_Manager_Impl::getSupportedServiceNames(void) throw ()
{
    return getSupportedServiceNames_Static();
}

//*************************************************************************
// Test_Manager_Impl::getSupportedServiceNames_Static
//
Sequence< OUString > Test_Manager_Impl::getSupportedServiceNames_Static(void) throw ()
{
    Sequence< OUString > aSNS( 2 );
    aSNS.getArray()[0] = OUString(RTL_CONSTASCII_USTRINGPARAM(SERVICE_NAME));
    aSNS.getArray()[1] = OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.bridge.Bridge"));
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
     OUString atUModule2 = OUString(
        RTL_CONSTASCII_USTRINGPARAM(
            SAL_DLLPREFIX "testsmgr_component" SAL_DLLEXTENSION ) );

    // expand shared library name
    OString  atModule2( OUStringToOString(atUModule2, RTL_TEXTENCODING_ASCII_US) );

    // get the process servicemanager
    Reference <XMultiServiceFactory>  xSMgr = getProcessServiceManager();

    TEST_ENSHURE( xSMgr.is() , "query on XServiceManager failed" );

    Reference<XContentEnumerationAccess> xContEnum(xSMgr, UNO_QUERY);
    TEST_ENSHURE( xContEnum.is() , "query on XContentEnumerationAccess failed" );
    Reference<XEnumeration > xEnum(xContEnum->createContentEnumeration(OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.registry.SimpleRegistry"))));
    TEST_ENSHURE( xEnum.is() , "createContentEnumeration failed" );
    sal_Int32 nLen = 0;
    while( xEnum->hasMoreElements() )
    {
        nLen++;
        xEnum->nextElement();
    }
    TEST_ENSHURE( nLen == 1, "more than one implementation for SimpleRegistry" );

    Reference<XEnumerationAccess> xImplEnum(xSMgr, UNO_QUERY);
    TEST_ENSHURE( xImplEnum.is() , "query on XEnumeration failed" );
    xEnum = Reference<XEnumeration >(xImplEnum->createEnumeration());
    TEST_ENSHURE( xEnum.is() , "createEnumeration failed" );
    nLen = 0;
    while( xEnum->hasMoreElements() )
    {
        nLen++;
        Reference< XServiceInfo > sf( xEnum->nextElement(), UNO_QUERY );
        OString str( OUStringToOString( sf->getImplementationName(), RTL_TEXTENCODING_ASCII_US ) );
        ::fprintf( stderr, "> implementation name: %s\n", str.getStr() );
    }
    TEST_ENSHURE( nLen == 8, "more than 6 factories" );

    // try to get an instance for a unknown service
    TEST_ENSHURE( !xSMgr->createInstance(OUString(RTL_CONSTASCII_USTRINGPARAM("bla.blup.Q"))).is(), "unknown service provider found" );

    //
    // First test : register service via the internal function of the component itself
    //
    {
        Reference< XImplementationRegistration >
            xInst( xSMgr->createInstance(OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.registry.ImplementationRegistration"))), UNO_QUERY );
        TEST_ENSHURE( xInst.is(), "no ImplementationRegistration" );

        try {
            // register the services via writeComponentRegInfo (see at end of this file)
            xInst->registerImplementation(OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.loader.SharedLibrary")), atUModule2, Reference< XSimpleRegistry >() );
        }
        catch( CannotRegisterImplementationException e ) {
            TEST_ENSHURE( 0, "register implementation failed" );
        }

        // getImplementations() check
         Sequence<OUString> seqImpl = xInst->getImplementations(OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.loader.SharedLibrary")), atUModule2);
        TEST_ENSHURE( seqImpl.getLength() == 1, "count of implementantions is wrong" );
        TEST_ENSHURE( seqImpl.getConstArray()[0] == OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.DummyService.V10")), "implementation name is not equal" );


        // tests, if a service provider can be instantiated.

        Reference< XInterface > xIFace(xSMgr->createInstance(OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.ts.TestManagerImpl"))));
        TEST_ENSHURE( xIFace.is(), "loadable service not found" );

        // remove the service
        TEST_ENSHURE(   xInst->revokeImplementation(atUModule2, Reference< XSimpleRegistry > ()),
                        "revoke implementation failed" );
    }

    Reference<XComponent> xComp(xSMgr, UNO_QUERY);
    xComp->dispose();

    xComp.clear();
    xSMgr.clear();
}



extern "C"
{
//==================================================================================================
void SAL_CALL component_getImplementationEnvironment(
    const sal_Char ** ppEnvTypeName, uno_Environment ** /*ppEnv*/ )
{
    *ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
}
//==================================================================================================
sal_Bool SAL_CALL component_writeInfo(
    void * /*pServiceManager*/, void * pRegistryKey )
{
    if (pRegistryKey)
    {
        try
        {
            Reference< XRegistryKey > xNewKey(
                reinterpret_cast< XRegistryKey * >( pRegistryKey )->createKey(
                    OUString( RTL_CONSTASCII_USTRINGPARAM("/" IMPLEMENTATION_NAME "/UNO/SERVICES") ) ) );

            const Sequence< OUString > & rSNL =
                Test_Manager_Impl::getSupportedServiceNames_Static();
            const OUString * pArray = rSNL.getConstArray();
            for ( sal_Int32 nPos = rSNL.getLength(); nPos--; )
                xNewKey->createKey( pArray[nPos] );

            return sal_True;
        }
        catch (InvalidRegistryException &)
        {
            OSL_ENSURE( sal_False, "### InvalidRegistryException!" );
        }
    }
    return sal_False;
}
//==================================================================================================
void * SAL_CALL component_getFactory(
    const sal_Char * pImplName, void * pServiceManager, void * /*pRegistryKey*/ )
{
    void * pRet = 0;

    if (rtl_str_compare( pImplName, IMPLEMENTATION_NAME ) == 0)
    {
        Reference< XSingleServiceFactory > xFactory( createSingleFactory(
            reinterpret_cast< XMultiServiceFactory * >( pServiceManager ),
            OUString( RTL_CONSTASCII_USTRINGPARAM(IMPLEMENTATION_NAME) ),
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
