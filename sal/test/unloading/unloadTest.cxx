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

#include<osl/module.hxx>
#include <osl/time.h>
#include <rtl/ustring.hxx>
#include <stdio.h>
#include <cppuhelper/factory.hxx>
#include <cppuhelper/servicefactory.hxx>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/uno/XUnloadingPreference.hpp>
#include <com/sun/star/lang/XTypeProvider.hpp>
#include <com/sun/star/container/XContentEnumerationAccess.hpp>

using namespace ::rtl;
using namespace ::osl;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::cppu;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;

#define IMPLNAME1 "com.sun.star.comp.sal.UnloadingTest1"
#define SERVICENAME1 "com.sun.star.UnloadingTest1"
#define IMPLNAME2 "com.sun.star.comp.sal.UnloadingTest2"
#define SERVICENAME2 "com.sun.star.UnloadingTest2"
#define IMPLNAME3 "com.sun.star.comp.sal.UnloadingTest3"
#define SERVICENAME3 "com.sun.star.UnloadingTest3"
#define IMPLNAME4 "com.sun.star.comp.sal.OneInstanceTest"
#define SERVICENAME4 "com.sun.star.OneInstanceTest"

#define IMPLNAME21 "com.sun.star.comp.sal.UnloadingTest21"
#define SERVICENAME21 "com.sun.star.UnloadingTest21"
#define IMPLNAME22 "com.sun.star.comp.sal.UnloadingTest22"
#define SERVICENAME22 "com.sun.star.UnloadingTest22"
#define IMPLNAME23 "com.sun.star.comp.sal.UnloadingTest23"
#define SERVICENAME23 "com.sun.star.UnloadingTest23"

#ifdef UNX
#define LIBRARY1 "libsamplelib1.so"
#define LIBRARY2 "libsamplelib2.so"
#elif defined WNT
#define LIBRARY1 "samplelib1"
#define LIBRARY2 "samplelib2"
#endif
/*
Tested: rtl_registerModuleForUnloading, rtl_unregisterModuleForUnloading, rtl_unloadUnusedLibraries
        1 component.

next: Test with multiple components
        Listener mechanism.
*/

sal_Bool test1();
sal_Bool test2();
sal_Bool test3();
sal_Bool test4();
sal_Bool test5();
sal_Bool test6();
sal_Bool test7();
sal_Bool test8();
sal_Bool test9();
void SAL_CALL listenerCallback( void* id);

int main(int argc, char* argv[])
{
  // Test if the servicemanager can be created and if the sample libs
  // can be loaded
//  Reference<XMultiServiceFactory> serviceManager= createRegistryServiceFactory(
//     OUString("applicat.rdb"));
//    if( !serviceManager.is())
//    {
//      printf("\n ####################################################\n"
//         "Error: could not create service manager. \n"
//         "Is the executable in the office program directory?\n");
//      return -1;
//    }

//    Reference<XInterface> xint1=  serviceManager->createInstance( OUString(
//                  RTL_CONSTASCII_USTRINGPARAM(SERVICENAME1)));

//    if( !xint1.is())
//    {
//        printf("\n ###################################################\n"
//           "Error: could not create service from samplelib1\n"
//           "Is samplelib1 in the office program directory and is it "
//           "registered?\n");
//        return -1;
//    }
//    Reference<XInterface> xint2=  serviceManager->createInstance( OUString(
//                  RTL_CONSTASCII_USTRINGPARAM(SERVICENAME21)));
//    if( !xint2.is())
//      {
//        printf("\n ###################################################"
//           "Error: could not create service from samplelib2\n"
//           "Is samplelib2 in the office program directory and is it "
//           "registered?\n");
//        return -1;
//      }
//        //destroy servicemanager
//        Reference<XPropertySet> xSet( serviceManager, UNO_QUERY);
//        Any any_prop= xSet->getPropertyValue( OUString("DefaultContext"));
//        Reference<XComponentContext> xContext;
//        any_prop >>= xContext;
//        Reference<XComponent> xComponent( xContext, UNO_QUERY);
//        xComponent->dispose();

//        //unload samplelib1 and samplelib2. We need the handles, therefore load
//        // the libs
//        OUString libname1( RTL_CONSTASCII_USTRINGPARAM(LIBRARY1));
//        OUString libname2( RTL_CONSTASCII_USTRINGPARAM(LIBRARY2));
//        oslModule m1= osl_loadModule(libname1.pData, 0);
//        oslModule m2= osl_loadModule(libname2.pData, 0);
//        osl_unloadModule( m1);
//        osl_unloadModule( m1);
//        osl_unloadModule( m2);
//        osl_unloadModule( m2);


  sal_Bool ret1= test1();
  if( ret1) printf( "\n Test 1 successful \n");
    else printf("\n !!!!!! Test 1 failed\n");
    sal_Bool ret2= test2();
    if( ret2) printf( "\n Test 2 successful \n");
    else printf("\n !!!!!! Test 2 failed\n");
    sal_Bool ret3= test3();
    if( ret3) printf( "\n Test 3 successful \n");
    else printf("\n !!!!!! Test 3 failed\n");
    sal_Bool ret4= test4();
    if( ret4) printf( "\n Test 4 successful \n");
    else printf("\n !!!!!! Test 4 failed\n");
    sal_Bool ret5= test5();
    if( ret5) printf( "\n Test 5 successful \n");
    else printf("\n !!!!!! Test 5 failed\n");
    // takes some time (10s)
    sal_Bool ret6= test6();
    sal_Bool ret7= test7(); // prints message itself
    sal_Bool ret8= test8();
    if( ret8) printf( "\n Test 8 successful \n");
    else printf("\n !!!!!! Test 8 failed\n");
    sal_Bool ret9= test9();
    if( ret9) printf( "\n Test 9 successful: service manager is unloading listener\n");
    else printf("\n !!!!! Test 9 failed\n");

    return 0;
}

/* Create an instance of SERVICENAME1, call a function and unload module.
   This tests the loader and basic functionality.
   The library will be loaded once manually and the handle will be stored.
   Then the library will be unloaded. After rtl_unloadUnusedLibraries we try to
   get a symbol of the unloaded lib. If this fails then the test is successful.
*/
sal_Bool test1()
{
    printf("Test 1 ####################################################\n");
    oslModule handleMod=0;
    OUString lib1Name( RTL_CONSTASCII_USTRINGPARAM(LIBRARY1));
    {
    Reference<XMultiServiceFactory> serviceManager= createRegistryServiceFactory(
        OUString("applicat.rdb"));
    Reference<XInterface> xint= serviceManager->createInstance( OUString(
                RTL_CONSTASCII_USTRINGPARAM(SERVICENAME1)));

    // get the handle
    handleMod=  osl_loadModule( lib1Name.pData, 0);
    osl_unloadModule( handleMod);
    xint=0;

    Reference<XPropertySet> xSet( serviceManager, UNO_QUERY);
    Any any_prop=   xSet->getPropertyValue( OUString("DefaultContext"));
    Reference<XComponentContext> xContext;
    any_prop >>= xContext;
    Reference<XComponent> xComponent( xContext, UNO_QUERY);
    xComponent->dispose();
    }
    rtl_unloadUnusedModules( NULL);

    // Try to get a symbol, must fail
    OUString sSymbol( RTL_CONSTASCII_USTRINGPARAM("component_getFactory"));
    void* pSymbol= osl_getSymbol(  handleMod, sSymbol.pData);

    if( !pSymbol)
        return sal_True;
    return sal_False;
}

/* Multipe loadModule + rtl_registerModuleForUnloading.
The module will be registered as often as it has been loaded.
*/
sal_Bool test2()
{
    printf("Test 2 ####################################################\n");
    oslModule handleMod=0;
    OUString lib1Name( RTL_CONSTASCII_USTRINGPARAM(LIBRARY1));
    {
    Reference<XMultiServiceFactory> serviceManager= createRegistryServiceFactory(
         OUString("applicat.rdb"));

    Reference<XInterface> xint= serviceManager->createInstance( OUString(
                RTL_CONSTASCII_USTRINGPARAM(SERVICENAME1)));

    handleMod=  osl_loadModule( lib1Name.pData, 0);
    osl_unloadModule( handleMod);
    //-----------------------------------------------------------
    oslModule mod1= osl_loadModule( lib1Name.pData, 0);
    oslModule mod2= osl_loadModule( lib1Name.pData, 0);
    oslModule mod3= osl_loadModule( lib1Name.pData, 0);

    rtl_registerModuleForUnloading(mod1);
    rtl_registerModuleForUnloading(mod2);
    rtl_registerModuleForUnloading(mod3);
    // ----------------------------------------------------------
    Reference<XPropertySet> xSet( serviceManager, UNO_QUERY);
    Any any_prop=   xSet->getPropertyValue( OUString("DefaultContext"));
    Reference<XComponentContext> xContext;
    any_prop >>= xContext;
    Reference<XComponent> xComponent( xContext, UNO_QUERY);
    xComponent->dispose();
    }
    rtl_unloadUnusedModules( NULL);

    // Try to get a symbol, must fail
    OUString sSymbol( RTL_CONSTASCII_USTRINGPARAM("component_getFactory"));
    void* pSymbol= osl_getSymbol(  handleMod, sSymbol.pData);

    if( !pSymbol)
        return sal_True;
    return sal_False;
}

/* Multipe loadModule + rtl_registerModuleForUnloading.
The module will be registered one time less as it has been loaded.
*/
sal_Bool test3()
{
    printf("Test 3 ####################################################\n");
    oslModule handleMod=0;
    sal_Bool retval=sal_False;
    OUString lib1Name( RTL_CONSTASCII_USTRINGPARAM(LIBRARY1));
    {
    Reference<XMultiServiceFactory> serviceManager= createRegistryServiceFactory(
         OUString("applicat.rdb"));

    Reference<XInterface> xint= serviceManager->createInstance( OUString(
                RTL_CONSTASCII_USTRINGPARAM(SERVICENAME1)));

    handleMod=  osl_loadModule( lib1Name.pData, 0);
    osl_unloadModule( handleMod);
    //-----------------------------------------------------------
    oslModule mod1= osl_loadModule( lib1Name.pData, 0);
    oslModule mod2= osl_loadModule( lib1Name.pData, 0);
    oslModule mod3= osl_loadModule( lib1Name.pData, 0);

    rtl_registerModuleForUnloading(mod1);
    rtl_registerModuleForUnloading(mod2);
    // ----------------------------------------------------------
    Reference<XPropertySet> xSet( serviceManager, UNO_QUERY);
    Any any_prop=   xSet->getPropertyValue( OUString("DefaultContext"));
    Reference<XComponentContext> xContext;
    any_prop >>= xContext;
    Reference<XComponent> xComponent( xContext, UNO_QUERY);
    xComponent->dispose();
    }
    rtl_unloadUnusedModules( NULL);

    // Try to get a symbol, must succeed
    OUString sSymbol( RTL_CONSTASCII_USTRINGPARAM("component_getFactory"));
    void* pSymbol= osl_getSymbol(  handleMod, sSymbol.pData);

    if( pSymbol)
    {
        retval= sal_True;
        osl_unloadModule( handleMod);
        pSymbol= osl_getSymbol( handleMod, sSymbol.pData);
    }
    return retval;
}
/* 2 Modules

*/
sal_Bool test4()
{
    printf("Test 4 ####################################################\n");
    oslModule handleMod1=0;
    oslModule handleMod2=0;
    OUString lib1Name( RTL_CONSTASCII_USTRINGPARAM(LIBRARY1));
    OUString lib2Name( RTL_CONSTASCII_USTRINGPARAM(LIBRARY2));
    {
    Reference<XMultiServiceFactory> serviceManager= createRegistryServiceFactory(
         OUString("applicat.rdb"));

    Reference<XInterface> xint= serviceManager->createInstance( OUString(
                RTL_CONSTASCII_USTRINGPARAM(SERVICENAME1)));

    handleMod1= osl_loadModule( lib1Name.pData, 0);
    osl_unloadModule( handleMod1);
    Reference<XInterface> xint2=    serviceManager->createInstance( OUString(
                RTL_CONSTASCII_USTRINGPARAM(SERVICENAME21)));

    handleMod2= osl_loadModule( lib2Name.pData, 0);
    osl_unloadModule( handleMod2);

    //-----------------------------------------------------------
    // ----------------------------------------------------------
    Reference<XPropertySet> xSet( serviceManager, UNO_QUERY);
    Any any_prop=   xSet->getPropertyValue( OUString("DefaultContext"));
    Reference<XComponentContext> xContext;
    any_prop >>= xContext;
    Reference<XComponent> xComponent( xContext, UNO_QUERY);
    xComponent->dispose();
    }
    rtl_unloadUnusedModules( NULL);

    // Try to get a symbol, must fail
    OUString sSymbol( RTL_CONSTASCII_USTRINGPARAM("component_getFactory"));
    void* pSymbol= osl_getSymbol(  handleMod1, sSymbol.pData);

    void* pSymbol2= osl_getSymbol(  handleMod2, sSymbol.pData);
    if( ! pSymbol && !pSymbol2)
        return sal_True;
    return sal_False;
}

/* 2 Modules and 6 services

*/
sal_Bool test5()
{
    printf("test5 ####################################################\n");
    oslModule handleMod1=0;
    oslModule handleMod2=0;
    sal_Bool btest1= sal_False;
    OUString lib1Name( RTL_CONSTASCII_USTRINGPARAM(LIBRARY1));
    OUString lib2Name( RTL_CONSTASCII_USTRINGPARAM(LIBRARY2));
    OUString sSymbol( RTL_CONSTASCII_USTRINGPARAM("component_getFactory"));
    {
    Reference<XMultiServiceFactory> serviceManager= createRegistryServiceFactory(
         OUString("applicat.rdb"));

    //-----------------------------------------------------------
    Reference<XInterface> xint= serviceManager->createInstance( OUString(
                RTL_CONSTASCII_USTRINGPARAM(SERVICENAME1)));
    Reference<XInterface> xint2=    serviceManager->createInstance( OUString(
                RTL_CONSTASCII_USTRINGPARAM(SERVICENAME2)));
    Reference<XInterface> xint3=    serviceManager->createInstance( OUString(
                RTL_CONSTASCII_USTRINGPARAM(SERVICENAME3)));
    Reference<XInterface> xint4=    serviceManager->createInstance( OUString(
                RTL_CONSTASCII_USTRINGPARAM(SERVICENAME21)));
    Reference<XInterface> xint5=    serviceManager->createInstance( OUString(
                RTL_CONSTASCII_USTRINGPARAM(SERVICENAME22)));
    Reference<XInterface> xint6=    serviceManager->createInstance( OUString(
                RTL_CONSTASCII_USTRINGPARAM(SERVICENAME23)));

    // ----------------------------------------------------------
    handleMod1= osl_loadModule( lib1Name.pData, 0);
    osl_unloadModule( handleMod1);
    handleMod2= osl_loadModule( lib2Name.pData, 0);
    osl_unloadModule( handleMod2);

    // get rid of the service manager
    Reference<XPropertySet> xSet( serviceManager, UNO_QUERY);
    Any any_prop=   xSet->getPropertyValue( OUString("DefaultContext"));
    Reference<XComponentContext> xContext;
    any_prop >>= xContext;
    Reference<XComponent> xComponent( xContext, UNO_QUERY);
    xComponent->dispose();

    // try unloading, must fail
    rtl_unloadUnusedModules( NULL);
    void* pSymbol= osl_getSymbol(  handleMod1, sSymbol.pData);

    void* pSymbol2= osl_getSymbol(  handleMod2, sSymbol.pData);
    if(  pSymbol && pSymbol2)
        btest1= sal_True;

    }

    // Try to get a symbol, must succeed
    rtl_unloadUnusedModules( NULL);

    void* pSymbol= osl_getSymbol(  handleMod1, sSymbol.pData);
    void* pSymbol2= osl_getSymbol(  handleMod2, sSymbol.pData);
    if( ! pSymbol && !pSymbol2 && btest1)
        return sal_True;
    return sal_False;
}

/*
TimeValue Test
rtl_unloadUnusedModules takes a TimeValue which determines a timespan
a module must have been constantly unused in order to be unloaded.
This is only a rough test. To make accurate tests, one should directly
write code in the unload.cxx file.
The function will not return (loop) when the test fails or the result value
is far off the 10 seconds value.
*/
sal_Bool test6()
{
    printf("test6 ####################################################\n");
    oslModule handleMod1=0;
    oslModule handleMod2=0;
    OUString lib1Name( RTL_CONSTASCII_USTRINGPARAM(LIBRARY1));
    OUString lib2Name( RTL_CONSTASCII_USTRINGPARAM(LIBRARY2));
    OUString sSymbol( RTL_CONSTASCII_USTRINGPARAM("component_getFactory"));
    {
    Reference<XMultiServiceFactory> serviceManager= createRegistryServiceFactory(
         OUString("applicat.rdb"));
    Reference<XInterface> xint= serviceManager->createInstance( OUString(
                RTL_CONSTASCII_USTRINGPARAM(SERVICENAME1)));

    // ----------------------------------------------------------
    handleMod1= osl_loadModule( lib1Name.pData, 0);
    osl_unloadModule( handleMod1);

    // get rid of the service manager
    Reference<XPropertySet> xSet( serviceManager, UNO_QUERY);
    Any any_prop=   xSet->getPropertyValue( OUString("DefaultContext"));
    Reference<XComponentContext> xContext;
    any_prop >>= xContext;
    Reference<XComponent> xComponent( xContext, UNO_QUERY);
    xComponent->dispose();
    }

    // Enter a loop and try unloading. At least after 10 seconds
    // this should be successful.
    TimeValue time={10,0};

    TimeValue beforeTime={0,0};
    printf("\n unloading should take about 10 seconds\n");
    osl_getSystemTime( &beforeTime);
    for(;;)
    {

        rtl_unloadUnusedModules( &time);
        void* pSymbol= osl_getSymbol(  handleMod1, sSymbol.pData);
        if( ! pSymbol)
            break;
    }
    TimeValue afterTime={0,0};
    osl_getSystemTime( &afterTime);

    printf("\n it took about %i seconds \n Check the value!!!", afterTime.Seconds - beforeTime.Seconds);
    printf(" hit return to continue\n");
    getchar();

    return sal_True;
}

/*
*/
sal_Bool test7()
{
    printf("Test 7 ####################################################"
            "\nThe callback function should be called 3 times\n");
    sal_Int32 id1=1;
    sal_Int32 id2=2;
    sal_Int32 id3=3;
    sal_Int32 cookie1= rtl_addUnloadingListener( listenerCallback, &id1);
    sal_Int32 cookie2= rtl_addUnloadingListener( listenerCallback, &id2);
    sal_Int32 cookie3= rtl_addUnloadingListener( listenerCallback, &id3);

    printf("\nTest 7 \nThe listener should be called 3 times\n");
    rtl_unloadUnusedModules( NULL);

    rtl_removeUnloadingListener( cookie1);
    rtl_removeUnloadingListener( cookie2);
    rtl_removeUnloadingListener( cookie3);

    sal_Int32 cookie4= rtl_addUnloadingListener( listenerCallback, &id1);
    sal_Int32 cookie5= rtl_addUnloadingListener( listenerCallback, &id2);
    sal_Int32 cookie6= rtl_addUnloadingListener( listenerCallback, &id3);

    if( cookie1 == cookie4 &&
        cookie2 == cookie5 )
    {
        printf("\n###cookie recycling works\n");
        printf("hit return to continue\n");
        getchar();
    }
    else
    {
        printf("\n###cookie recycling failed!!!\n");
        printf("hit return to continue\n");
        getchar();
    }

    rtl_removeUnloadingListener( cookie1);
    rtl_removeUnloadingListener( cookie2);
    rtl_removeUnloadingListener( cookie3);
    return sal_True;
}

/* Test one-instance-service default factory (XUnloadingPreference)
    cppuhelper/source/factory.cxx
*/
sal_Bool test8()
{
    printf("Test 8 ####################################################\n");
    oslModule handleMod1=0;
    OUString lib1Name( RTL_CONSTASCII_USTRINGPARAM(LIBRARY1));
    OUString sSymbol( RTL_CONSTASCII_USTRINGPARAM("component_getFactory"));

    sal_Bool b_ifaceSupported=sal_False;
    sal_Bool b_instances_identical= sal_False;
    sal_Bool b_releaseBeforeLoading= sal_False;
    sal_Bool b_releaseAfterLoading= sal_False;
    sal_Bool b_unloaded= sal_False;

    {
    Reference<XMultiServiceFactory> serviceManager= createRegistryServiceFactory(
         OUString("applicat.rdb"));
    Reference<XContentEnumerationAccess> xContent( serviceManager, UNO_QUERY);
    Reference<XEnumeration> xenum=  xContent->createContentEnumeration(
        OUString( RTL_CONSTASCII_USTRINGPARAM( SERVICENAME4)));

    Any any_elem;
    if( xenum->hasMoreElements())
        any_elem= xenum->nextElement();
    Reference<XInterface> xinterfaceFact;
    any_elem>>=xinterfaceFact;
    Reference<XTypeProvider> xprov( xinterfaceFact, UNO_QUERY);

    Sequence<Type> seqTypes= xprov->getTypes();

    //  XTypeProvider test
    for( sal_Int32 i=0; i<seqTypes.getLength(); i++)
    {
        OUString name= seqTypes[i].getTypeName();
        if ( name == "com.sun.star.uno.XUnloadingPreference" )
            b_ifaceSupported= sal_True;
    }

    // XUnloadingPreference::releaseOnNotification should return true now because we haven't
    // created an instance yet
    Reference<XUnloadingPreference> xreject( xinterfaceFact, UNO_QUERY);
    b_releaseBeforeLoading= xreject->releaseOnNotification();

    // Create instance. Afterwards releaseOnNotification should return false.
    Reference<XInterface> xint= serviceManager->createInstance( OUString(
                RTL_CONSTASCII_USTRINGPARAM(SERVICENAME4)));
    b_releaseAfterLoading= xreject->releaseOnNotification();
    b_releaseAfterLoading= b_releaseAfterLoading? sal_False : sal_True;

    // safe the handle of the module
    handleMod1= osl_loadModule( lib1Name.pData, 0);
    osl_unloadModule( handleMod1);

    // ----------------------------------------------------------
    // for debugging
    Reference<XServiceInfo> info( xint, UNO_QUERY);
    OUString s= info->getImplementationName();

    // get another instance which must be the same
    Reference<XInterface> xint2=    serviceManager->createInstance( OUString(
                RTL_CONSTASCII_USTRINGPARAM(SERVICENAME4)));

    b_instances_identical= xint == xint2;

    // get rid of the service manager
    Reference<XPropertySet> xSet( serviceManager, UNO_QUERY);
    Any any_prop=   xSet->getPropertyValue( OUString("DefaultContext"));
    Reference<XComponentContext> xContext;
    any_prop >>= xContext;
    Reference<XComponent> xComponent( xContext, UNO_QUERY);
    xComponent->dispose();
    }

    rtl_unloadUnusedModules( NULL);

    // The library must be unloaded now
    void* pSymbol= osl_getSymbol(  handleMod1, sSymbol.pData);
    if( ! pSymbol )
        b_unloaded= sal_True;

    if( b_ifaceSupported && b_instances_identical && b_releaseBeforeLoading &&
        b_releaseAfterLoading && b_unloaded)
        return sal_True;
    return sal_False;
}

void SAL_CALL listenerCallback( void* id)
{
    printf(" listener called with id= %i\n", *(sal_Int32*)id);
}

/*

  */
sal_Bool test9()
{
    printf("Test 9 ####################################################\n");
    oslModule handleMod=0;
    sal_Bool retval=sal_False;
    OUString lib1Name( RTL_CONSTASCII_USTRINGPARAM(LIBRARY1));

    Reference<XMultiServiceFactory> serviceManager= createRegistryServiceFactory(
         OUString("applicat.rdb"));

    Reference<XInterface> xint= serviceManager->createInstance( OUString(
                RTL_CONSTASCII_USTRINGPARAM(SERVICENAME1)));
    // Release the service. The library refcount should be 1
    xint=0;

    handleMod=  osl_loadModule( lib1Name.pData, 0);
    osl_unloadModule( handleMod);
    //-----------------------------------------------------------

    // the service manager is still alive
    rtl_unloadUnusedModules( NULL);
    // Try to get a symbol, must fail
    OUString sSymbol( RTL_CONSTASCII_USTRINGPARAM("component_getFactory"));
    void* pSymbol= osl_getSymbol(  handleMod, sSymbol.pData);

    if( pSymbol)
    {
        retval= sal_False;
    }
    else
        retval= sal_True;
    return retval;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
