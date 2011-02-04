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
#include "precompiled_extensions.hxx"

#include<osl/module.hxx>
#include <osl/time.h>
#include <rtl/ustring.hxx>
#include <cppuhelper/factory.hxx>
#include <cppuhelper/servicefactory.hxx>
#include <cppuhelper/bootstrap.hxx>
#include <com/sun/star/lang/XMultiComponentFactory.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/registry/XSimpleRegistry.hpp>

#include <stdio.h>
using namespace ::rtl;
using namespace ::osl;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::cppu;
using namespace ::com::sun::star::registry;


sal_Bool test1();
sal_Bool test2();
sal_Bool test3();
sal_Bool test4();

int main(int, char**)
{
       sal_Bool bTest1= test1();
       sal_Bool bTest2= test2();
    sal_Bool bTest3= test3();
    sal_Bool bTest4= test4();

    if( bTest1 && bTest2 && bTest3 && bTest4)
        printf("\n#########################\n Test was successful\n#######################\n");

    return 0;
}

sal_Bool test1()
{
    printf("\n Test1:  com.sun.star.bridge.oleautomation.BridgeSupplier\n");
    Reference<XSimpleRegistry> xreg= createSimpleRegistry();
    xreg->open( OUString( RTL_CONSTASCII_USTRINGPARAM("services.rdb")),
                               sal_False, sal_False );

    Reference< XComponentContext > context= bootstrap_InitialComponentContext(xreg);
    Reference<XMultiComponentFactory> fac= context->getServiceManager();
    OUString sService1( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.bridge.oleautomation.BridgeSupplier"));
    Reference<XInterface> xint1= fac->createInstanceWithContext( sService1, context);

    OUString sModule(
        RTL_CONSTASCII_USTRINGPARAM("oleautobridge.uno" SAL_DLLEXTENSION));
    oslModule hMod= osl_loadModule( sModule.pData, 0);
    osl_unloadModule( hMod);

    rtl_unloadUnusedModules( NULL);

    OUString sFactoryFunc( RTL_CONSTASCII_USTRINGPARAM("component_getFactory"));
    void* pSymbol= osl_getSymbol( hMod,sFactoryFunc.pData);
    // true, instance alive
    sal_Bool bTest1= pSymbol ? sal_True : sal_False;

    xint1=0;
    rtl_unloadUnusedModules( NULL);
    pSymbol= osl_getSymbol( hMod,sFactoryFunc.pData);
    sal_Bool bTest2= pSymbol ? sal_False : sal_True;

    Reference<XComponent> xcomp( context, UNO_QUERY);
    xcomp->dispose();

    return bTest2 && bTest1;
}

sal_Bool test2()
{
    printf("Test2:  com.sun.star.bridge.OleBridgeSupplierVar1\n");
    Reference<XSimpleRegistry> xreg= createSimpleRegistry();
    xreg->open( OUString( RTL_CONSTASCII_USTRINGPARAM("services.rdb")),
                               sal_False, sal_False );

    Reference< XComponentContext > context= bootstrap_InitialComponentContext(xreg);
    Reference<XMultiComponentFactory> fac= context->getServiceManager();
      OUString sService2( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.bridge.OleBridgeSupplierVar1"));
    Reference<XInterface> xint= fac->createInstanceWithContext( sService2, context);

    OUString sModule(
        RTL_CONSTASCII_USTRINGPARAM("oleautobridge.uno" SAL_DLLEXTENSION));
    oslModule hMod= osl_loadModule( sModule.pData, 0);
    osl_unloadModule( hMod);

    rtl_unloadUnusedModules( NULL);
    OUString sFactoryFunc( RTL_CONSTASCII_USTRINGPARAM("component_getFactory"));
    void* pSymbol= osl_getSymbol( hMod,sFactoryFunc.pData);
    // true, instance alive
    sal_Bool bTest1= pSymbol ? sal_True : sal_False;

    xint=0;
    rtl_unloadUnusedModules( NULL);
    pSymbol= osl_getSymbol( hMod,sFactoryFunc.pData);
    sal_Bool bTest2= pSymbol ? sal_False : sal_True;

    Reference<XComponent> xcomp( context, UNO_QUERY);
    xcomp->dispose();
    return bTest1 && bTest2;
}

sal_Bool test3()
{
    printf("Test3:  com.sun.star.bridge.oleautomation.Factory\n");
    Reference<XSimpleRegistry> xreg= createSimpleRegistry();
    xreg->open( OUString( RTL_CONSTASCII_USTRINGPARAM("services.rdb")),
                sal_False, sal_False );

    Reference< XComponentContext > context= bootstrap_InitialComponentContext(xreg);

    Reference<XMultiComponentFactory> fac= context->getServiceManager();
      OUString sService( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.bridge.oleautomation.Factory"));
    Reference<XInterface> xint= fac->createInstanceWithContext( sService, context);


    OUString sModule(
        RTL_CONSTASCII_USTRINGPARAM("oleautobridge.uno" SAL_DLLEXTENSION));
    oslModule hMod= osl_loadModule( sModule.pData, 0);
    osl_unloadModule( hMod);

    rtl_unloadUnusedModules( NULL);
    OUString sFactoryFunc( RTL_CONSTASCII_USTRINGPARAM("component_getFactory"));
    void* pSymbol= osl_getSymbol( hMod,sFactoryFunc.pData);
    // true, instance alive
    sal_Bool bTest1= pSymbol ? sal_True : sal_False;

    xint=0;
    rtl_unloadUnusedModules( NULL);
    pSymbol= osl_getSymbol( hMod,sFactoryFunc.pData);
    sal_Bool bTest2= pSymbol ? sal_False : sal_True;

    Reference<XComponent> xcomp( context, UNO_QUERY);
    xcomp->dispose();

    return bTest1 && bTest2;
}

sal_Bool test4()
{
    void* pSymbol= NULL;
    sal_Bool bTest1= sal_False;
    sal_Bool bTest2= sal_False;
    oslModule hMod= NULL;
    OUString sModule(
        RTL_CONSTASCII_USTRINGPARAM("oleautobridge.uno" SAL_DLLEXTENSION));
    OUString sFactoryFunc( RTL_CONSTASCII_USTRINGPARAM("component_getFactory"));
    {
    printf("Test4:  com.sun.star.bridge.oleautomation.ApplicationRegistration\n");
       Reference<XSimpleRegistry> xreg= createSimpleRegistry();
    xreg->open( OUString( RTL_CONSTASCII_USTRINGPARAM("services.rdb")),
                               sal_False, sal_False );

    Reference< XComponentContext > context= bootstrap_InitialComponentContext(xreg);
    Reference<XMultiComponentFactory> fac= context->getServiceManager();
    OUString sService4( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.bridge.oleautomation.ApplicationRegistration"));
    Reference<XInterface> xint= fac->createInstanceWithContext( sService4, context);

    hMod= osl_loadModule( sModule.pData, 0);
    osl_unloadModule( hMod);

    rtl_unloadUnusedModules( NULL);
    void* pSymbol= osl_getSymbol( hMod,sFactoryFunc.pData);
    // true, instance alive
    bTest1= pSymbol ? sal_True : sal_False;
    // ApplicationRegistration is a one-instance-service, therefore kill service manager first
    Reference<XComponent> xcomp( context, UNO_QUERY);
    xcomp->dispose();

    }
    rtl_unloadUnusedModules( NULL);
    pSymbol= osl_getSymbol( hMod,sFactoryFunc.pData);
    bTest2= pSymbol ? sal_False : sal_True;

    return bTest1 && bTest2;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
