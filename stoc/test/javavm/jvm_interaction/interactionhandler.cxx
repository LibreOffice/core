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
#include <cppuhelper/implbase1.hxx>
#include <uno/current_context.hxx>

using namespace std;
using namespace rtl;
using namespace cppu;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::lang;
using namespace com::sun::star::registry;
using namespace com::sun::star::java;
using namespace com::sun::star::task;

#define OUSTR( x ) OUString(RTL_CONSTASCII_USTRINGPARAM( x ))
#define INTERACTION_HANDLER_NAME "java-vm.interaction-handler"

class Context: public WeakImplHelper1<XCurrentContext>
{
    virtual Any SAL_CALL getValueByName( const OUString& Name ) throw (RuntimeException);
};

class InteractionHandler: public WeakImplHelper1<XInteractionHandler>
{
    virtual void SAL_CALL handle( const Reference< XInteractionRequest >& Request )
        throw (RuntimeException);
};

Any SAL_CALL Context::getValueByName( const OUString& Name) throw (RuntimeException)
{
    Any retVal;
    if( Name.equals( OUSTR(INTERACTION_HANDLER_NAME)))
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
        abort= Reference<XInteractionAbort>::query( seqCont[i]);
        if(abort.is())
            break;
    }
    for (sal_Int32 i= 0; i < seqCont.getLength(); i++)
    {
        retry= Reference<XInteractionRetry>::query( seqCont[i]);
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

      OUString sVMService( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.java.JavaVirtualMachine"));
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
    catch (JavaNotConfiguredException& e)
    {
        OString msg= OUStringToOString(e.Message, osl_getThreadTextEncoding());
        printf("JavaNotConfiguredException: %s\n", msg.getStr());
    }
    catch (JavaVMCreationFailureException& e)
    {
        OString msg= OUStringToOString(e.Message, osl_getThreadTextEncoding());
        printf("JavaVMCreationFailureException: %s\n", msg.getStr());
    }
    catch (MissingJavaRuntimeException& e)
    {
        OString msg= OUStringToOString(e.Message, osl_getThreadTextEncoding());
        printf("MissingJavaRuntimeException: %s\n", msg.getStr());
    }
    catch (JavaDisabledException& e)
    {
        OString msg= OUStringToOString(e.Message, osl_getThreadTextEncoding());
        printf("JavaDisabledException: %s\n", msg.getStr());
    }
    catch (RuntimeException & e)
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
    xreg->open( OUString( RTL_CONSTASCII_USTRINGPARAM("applicat.rdb")),
                               sal_False, sal_False );

    Reference< XComponentContext > context= bootstrap_InitialComponentContext(xreg);
    Reference<XMultiComponentFactory> fac= context->getServiceManager();
    Reference<XMultiServiceFactory> xMgr( fac, UNO_QUERY);

    sal_Bool bSucc = sal_False;
    bSucc= test1(xMgr);
    Reference< XComponent > xCompContext( context, UNO_QUERY );
    xCompContext->dispose();
    return (bSucc ? 0 : -1);
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
