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
#include <vbahelper/vbaglobalbase.hxx>
#include <sal/macros.h>

#include <comphelper/sequence.hxx>
#include <cppuhelper/component_context.hxx>
#include <cppuhelper/exc_hlp.hxx>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/lang/WrappedTargetRuntimeException.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

using namespace com::sun::star;
using namespace ooo::vba;

// special key to return the Application
const char sAppService[] = "ooo.vba.Application";
const OUStringLiteral gsApplication( u"Application" );

VbaGlobalsBase::VbaGlobalsBase(
const uno::Reference< ov::XHelperInterface >& xParent,
const uno::Reference< uno::XComponentContext >& xContext, const OUString& sDocCtxName )
    : Globals_BASE( xParent, xContext )
    , msDocCtxName( sDocCtxName )
{
    // overwrite context with custom one ( that contains the application )
    // wrap the service manager as we don't want the disposing context to tear down the 'normal' ServiceManager ( or at least that's what the code appears like it wants to do )
    uno::Reference< uno::XInterface > aSrvMgr;
    if ( xContext.is() && xContext->getServiceManager().is() )
    {
        aSrvMgr = xContext->getServiceManager()->createInstanceWithContext( "com.sun.star.comp.stoc.OServiceManagerWrapper" , xContext );
    }

    ::cppu::ContextEntry_Init aHandlerContextInfo[] =
    {
        ::cppu::ContextEntry_Init( gsApplication, uno::Any() ),
        ::cppu::ContextEntry_Init( sDocCtxName, uno::Any() ),
        ::cppu::ContextEntry_Init( "/singletons/com.sun.star.lang.theServiceManager" , uno::makeAny( aSrvMgr ) )
    };
    // don't pass a delegate, this seems to introduce yet another cyclic dependency ( and
    // some strange behavior
    mxContext = ::cppu::createComponentContext(
                        aHandlerContextInfo,
                        SAL_N_ELEMENTS( aHandlerContextInfo ), nullptr );
    if ( !aSrvMgr.is() )
        return;

    try
    {
        uno::Reference< beans::XPropertySet >(
            aSrvMgr, uno::UNO_QUERY_THROW )->
            setPropertyValue( "DefaultContext", uno::makeAny( mxContext ) );
    }
    catch ( uno::RuntimeException & )
    {
        throw;
    }
    catch ( uno::Exception & )
    {
        uno::Any e(cppu::getCaughtException());
        throw lang::WrappedTargetRuntimeException(
            ("VbaGlobalsBase ctor, setting OServiceManagerWrapper"
             " DefaultContext failed"),
            uno::Reference< uno::XInterface >(), e);
    }
}

VbaGlobalsBase::~VbaGlobalsBase()
{
    try
    {
        uno::Reference< container::XNameContainer > xNameContainer( mxContext, uno::UNO_QUERY );
        if ( xNameContainer.is() )
        {
            // release document reference (we don't want the component context trying to dispose that)
            xNameContainer->removeByName( msDocCtxName );
            // release application reference, as it is holding onto the context
            xNameContainer->removeByName( gsApplication );
        }
    }
    catch ( const uno::Exception& )
    {
    }
}

void
VbaGlobalsBase::init(  const uno::Sequence< beans::PropertyValue >& aInitArgs )
{
    for ( const auto& rInitArg : aInitArgs )
    {
        uno::Reference< container::XNameContainer > xNameContainer( mxContext, uno::UNO_QUERY_THROW );
        if ( rInitArg.Name == gsApplication )
        {
            xNameContainer->replaceByName( gsApplication, rInitArg.Value );
            uno::Reference< XHelperInterface > xParent( rInitArg.Value, uno::UNO_QUERY );
            mxParent = xParent;
        }
        else
            xNameContainer->replaceByName( rInitArg.Name, rInitArg.Value );
    }
}

uno::Reference< uno::XInterface > SAL_CALL
VbaGlobalsBase::createInstance( const OUString& aServiceSpecifier )
{
    uno::Reference< uno::XInterface > xReturn;
    if ( aServiceSpecifier == sAppService )
    {
        // try to extract the Application from the context
        uno::Reference< container::XNameContainer > xNameContainer( mxContext, uno::UNO_QUERY );
        xNameContainer->getByName( gsApplication ) >>= xReturn;
    }
    else if ( hasServiceName( aServiceSpecifier ) )
        xReturn = mxContext->getServiceManager()->createInstanceWithContext( aServiceSpecifier, mxContext );
    return xReturn;
}

uno::Reference< uno::XInterface > SAL_CALL
VbaGlobalsBase::createInstanceWithArguments( const OUString& aServiceSpecifier, const uno::Sequence< uno::Any >& Arguments )
{

    uno::Reference< uno::XInterface > xReturn;
    if ( aServiceSpecifier == sAppService )
    {
        // try to extract the Application from the context
        uno::Reference< container::XNameContainer > xNameContainer( mxContext, uno::UNO_QUERY );
        xNameContainer->getByName( gsApplication ) >>= xReturn;
    }
    else if ( hasServiceName( aServiceSpecifier ) )
        xReturn = mxContext->getServiceManager()->createInstanceWithArgumentsAndContext( aServiceSpecifier, Arguments, mxContext );
    return xReturn;
}

uno::Sequence< OUString > SAL_CALL
VbaGlobalsBase::getAvailableServiceNames(  )
{
    uno::Sequence< OUString > serviceNames { "ooo.vba.msforms.UserForm" };
    return serviceNames;
}

bool
VbaGlobalsBase::hasServiceName( const OUString& serviceName )
{
    uno::Sequence< OUString > sServiceNames( getAvailableServiceNames() );
    return comphelper::findValue(sServiceNames, serviceName) != -1;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
