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
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::lang;
using namespace com::sun::star::registry;
using namespace com::sun::star::java;

using ::rtl::OUString;
using ::rtl::OUStringToOString;
using ::rtl::OString;


sal_Bool testJavaVM(const Reference< XMultiServiceFactory > & xMgr )
{

      OUString sVMService( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.java.JavaVirtualMachine"));
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
        OSL_ENSURE(0,"could not get Java VM");
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
        OSL_TRACE( "Can't find Prog class\n");
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
    xreg->open( OUString( RTL_CONSTASCII_USTRINGPARAM("applicat.rdb")),
                               sal_False, sal_False );

    Reference< XComponentContext > context= bootstrap_InitialComponentContext(xreg);
    Reference<XMultiComponentFactory> fac= context->getServiceManager();
    Reference<XMultiServiceFactory> xMgr( fac, UNO_QUERY);

    sal_Bool bSucc = sal_False;
    try
    {
        OUString sImplReg(RTL_CONSTASCII_USTRINGPARAM(
            "com.sun.star.registry.ImplementationRegistration"));
        Reference<com::sun::star::registry::XImplementationRegistration> xImplReg(
            xMgr->createInstance( sImplReg ), UNO_QUERY );
        OSL_ENSURE( xImplReg.is(), "### no impl reg!" );


        OUString sLibLoader( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.loader.SharedLibrary"));
        OUString sJenLib(
            RTL_CONSTASCII_USTRINGPARAM( "javavm.uno" SAL_DLLEXTENSION ) );
        xImplReg->registerImplementation(
            sLibLoader, sJenLib, Reference< XSimpleRegistry >() );

        bSucc = testJavaVM( xMgr );
    }
    catch (Exception & rExc)
    {
        OSL_ENSURE( sal_False, "### exception occurred!" );
        OString aMsg( OUStringToOString( rExc.Message, RTL_TEXTENCODING_ASCII_US ) );
        OSL_TRACE( "### exception occurred: " );
        OSL_TRACE( aMsg.getStr() );
        OSL_TRACE( "\n" );
    }

    Reference< XComponent > xCompContext( context, UNO_QUERY );
    xCompContext->dispose();
    printf("javavm %s", bSucc ? "succeeded" : "failed");
    return (bSucc ? 0 : -1);
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
