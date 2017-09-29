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


#include <jni.h>

#include <stdio.h>
#include <sal/main.h>
#include <rtl/process.h>

#include <cppuhelper/servicefactory.hxx>
#include <cppuhelper/weak.hxx>
#include <cppuhelper/bootstrap.hxx>

#include <com/sun/star/registry/XSimpleRegistry.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/XMultiComponentFactory.hpp>
#include <com/sun/star/java/XJavaVM.hpp>
#include <com/sun/star/registry/XImplementationRegistration.hpp>
#include <com/sun/star/java/XJavaThreadRegister_11.hpp>

using namespace std;
using namespace cppu;
using namespace css::uno;
using namespace css::lang;
using namespace css::registry;
using namespace css::java;


sal_Bool testJavaVM(const Reference< XMultiServiceFactory > & xMgr )
{

      OUString sVMService("com.sun.star.java.JavaVirtualMachine");
    Reference<XInterface> xXInt= xMgr->createInstance(sVMService);
    if( ! xXInt.is())
        return sal_False;
    Reference<XJavaVM> xVM( xXInt, UNO_QUERY);
    if( ! xVM.is())
        return sal_False;
    Reference<XJavaThreadRegister_11> xreg11(xVM, UNO_QUERY);
    if( ! xreg11.is())
        return sal_False;


    sal_Int8 arId[16];
    rtl_getGlobalProcessId((sal_uInt8*) arId);
    Any anyVM = xVM->getJavaVM( Sequence<sal_Int8>(arId, 16));
    if ( ! anyVM.hasValue())
    {
        OSL_FAIL("could not get Java VM");
        return sal_False;
    }

    sal_Bool b= xreg11->isThreadAttached();
    xreg11->registerThread();
    b= xreg11->isThreadAttached();
    xreg11->revokeThread();
    b= xreg11->isThreadAttached();


    b= xVM->isVMEnabled();
    b= xVM->isVMStarted();


    b= xVM->isVMEnabled();
    b= xVM->isVMStarted();


    JavaVM* _jvm= *(JavaVM**) anyVM.getValue();
    JNIEnv *p_env;
    if( _jvm->AttachCurrentThread((void**) &p_env, 0))
        return sal_False;

    jclass cls = p_env->FindClass( "TestJavaVM");
    if (cls == 0) {
        SAL_WARN("stoc", "Can't find Prog class");
        exit(1);
    }

    jmethodID id = p_env->GetStaticMethodID( cls, "getInt", "()I");
    if( id)
    {
        p_env->CallStaticIntMethod(cls, id);
    }

    if( p_env->ExceptionOccurred()){
        p_env->ExceptionDescribe();
        p_env->ExceptionClear();
    }


    _jvm->DetachCurrentThread();
    return sal_True;
}

SAL_IMPLEMENT_MAIN()
{
    Reference<XSimpleRegistry> xreg= createSimpleRegistry();
    xreg->open( OUString("applicat.rdb"),
                               sal_False, sal_False );

    Reference< XComponentContext > context= bootstrap_InitialComponentContext(xreg);
    Reference<XMultiComponentFactory> fac= context->getServiceManager();
    Reference<XMultiServiceFactory> xMgr( fac, UNO_QUERY);

    sal_Bool bSucc = sal_False;
    try
    {
        OUString sImplReg(
            "com.sun.star.registry.ImplementationRegistration");
        Reference<css::registry::XImplementationRegistration> xImplReg(
            xMgr->createInstance( sImplReg ), UNO_QUERY );
        OSL_ENSURE( xImplReg.is(), "### no impl reg!" );


        OUString sLibLoader("com.sun.star.loader.SharedLibrary");
        OUString sJenLib( "javavm.uno" SAL_DLLEXTENSION );
        xImplReg->registerImplementation(
            sLibLoader, sJenLib, Reference< XSimpleRegistry >() );

        bSucc = testJavaVM( xMgr );
    }
    catch (const Exception & rExc)
    {
        SAL_WARN("stoc", "### exception occurred: " << rExc );
    }

    Reference< XComponent > xCompContext( context, UNO_QUERY );
    xCompContext->dispose();
    printf("javavm %s", bSucc ? "succeeded" : "failed");
    return (bSucc ? 0 : -1);
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
