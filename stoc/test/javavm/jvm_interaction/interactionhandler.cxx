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
#include <osl/thread.h>

#include <com/sun/star/registry/XSimpleRegistry.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/XMultiComponentFactory.hpp>
#include <com/sun/star/java/XJavaVM.hpp>
#include <com/sun/star/registry/XImplementationRegistration.hpp>
#include <com/sun/star/java/XJavaThreadRegister_11.hpp>

#include <com/sun/star/uno/XCurrentContext.hpp>
#include <com/sun/star/task/XInteractionHandler.hpp>
#include <com/sun/star/task/XInteractionRequest.hpp>
#include <com/sun/star/task/XInteractionContinuation.hpp>
#include <com/sun/star/task/XInteractionAbort.hpp>
#include <com/sun/star/task/XInteractionRetry.hpp>
#include <com/sun/star/java/JavaNotConfiguredException.hpp>
#include <com/sun/star/java/MissingJavaRuntimeException.hpp>
#include <com/sun/star/java/JavaDisabledException.hpp>
#include <com/sun/star/java/JavaVMCreationFailureException.hpp>
#include <cppuhelper/implbase.hxx>
#include <uno/current_context.hxx>

using namespace std;
using namespace cppu;
using namespace css::uno;
using namespace css::lang;
using namespace css::registry;
using namespace css::java;
using namespace css::task;


#define INTERACTION_HANDLER_NAME "java-vm.interaction-handler"

class Context: public WeakImplHelper<XCurrentContext>
{
    virtual Any SAL_CALL getValueByName( const OUString& Name ) throw (RuntimeException);
};

class InteractionHandler: public WeakImplHelper<XInteractionHandler>
{
    virtual void SAL_CALL handle( const Reference< XInteractionRequest >& Request )
        throw (RuntimeException);
};

Any SAL_CALL Context::getValueByName( const OUString& Name) throw (RuntimeException)
{
    Any retVal;
    if( Name.equals( INTERACTION_HANDLER_NAME))
    {
        Reference<XInteractionHandler> handler( static_cast<XWeak*>(new InteractionHandler()),
                                                UNO_QUERY);
        retVal <<= handler;
    }
    return retVal;
}

void SAL_CALL InteractionHandler::handle( const Reference< XInteractionRequest >& Request )
        throw (RuntimeException)
{
    Any anyExc= Request->getRequest();
    Sequence<Reference< XInteractionContinuation> >seqCont= Request->getContinuations();

    Reference<XInteractionAbort> abort;
    Reference<XInteractionRetry> retry;

    for (sal_Int32 i= 0; i < seqCont.getLength(); i++)
    {
        abort.set( seqCont[i], UNO_QUERY );
        if(abort.is())
            break;
    }
    for (sal_Int32 i= 0; i < seqCont.getLength(); i++)
    {
        retry.set( seqCont[i], UNO_QUERY );
        if(retry.is())
            break;
    }

    static int cRetry= 0;

    if( cRetry++ == 5)
    {
        if( abort.is())
            abort->select();
        return;
    }
    if( retry.is())
         retry->select();
}

sal_Bool test1(const Reference< XMultiServiceFactory > & xMgr )
{
    sal_Bool retVal= sal_True;
    setCurrentContext( Reference<XCurrentContext>( static_cast<XWeak*>(new Context()), UNO_QUERY));

      OUString sVMService("com.sun.star.java.JavaVirtualMachine");
    Reference<XInterface> xXInt= xMgr->createInstance(sVMService);
    if( ! xXInt.is())
        return sal_False;
    Reference<XJavaVM> xVM( xXInt, UNO_QUERY);
    if( ! xVM.is())
        return sal_False;


    sal_Int8 arId[16];
    rtl_getGlobalProcessId((sal_uInt8*) arId);

    Any anyVM;
    try
    {
        anyVM = xVM->getJavaVM( Sequence<sal_Int8>(arId, 16));
    }
    catch (const JavaNotConfiguredException& e)
    {
        OString msg= OUStringToOString(e.Message, osl_getThreadTextEncoding());
        printf("JavaNotConfiguredException: %s\n", msg.getStr());
    }
    catch (const JavaVMCreationFailureException& e)
    {
        OString msg= OUStringToOString(e.Message, osl_getThreadTextEncoding());
        printf("JavaVMCreationFailureException: %s\n", msg.getStr());
    }
    catch (const MissingJavaRuntimeException& e)
    {
        OString msg= OUStringToOString(e.Message, osl_getThreadTextEncoding());
        printf("MissingJavaRuntimeException: %s\n", msg.getStr());
    }
    catch (const JavaDisabledException& e)
    {
        OString msg= OUStringToOString(e.Message, osl_getThreadTextEncoding());
        printf("JavaDisabledException: %s\n", msg.getStr());
    }
    catch (const RuntimeException & e)
    {
        OString msg= OUStringToOString(e.Message, osl_getThreadTextEncoding());
        printf("###RuntimeException: %s\n", msg.getStr());
        retVal= sal_False;
    }
    return retVal;
}

SAL_IMPLEMENT_MAIN()
{
    Reference<XSimpleRegistry> xreg= createSimpleRegistry();
    xreg->open( OUString("applicat.rdb"),
                               sal_False, sal_False );

    Reference< XComponentContext > context= bootstrap_InitialComponentContext(xreg);
    Reference<XMultiComponentFactory> fac= context->getServiceManager();
    Reference<XMultiServiceFactory> xMgr( fac, UNO_QUERY);

    sal_Bool bSucc = test1(xMgr);
    Reference< XComponent > xCompContext( context, UNO_QUERY );
    xCompContext->dispose();
    return (bSucc ? 0 : -1);
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
