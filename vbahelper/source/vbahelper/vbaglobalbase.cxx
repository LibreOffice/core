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
#include "vbahelper/vbaglobalbase.hxx"
#include <sal/macros.h>

#include <cppuhelper/component_context.hxx>
#include <cppuhelper/exc_hlp.hxx>
#include <comphelper/processfactory.hxx>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/lang/WrappedTargetRuntimeException.hpp>

using namespace com::sun::star;
using namespace ooo::vba;

// special key to return the Application
const char sAppService[] = "ooo.vba.Application";

VbaGlobalsBase::VbaGlobalsBase(
const uno::Reference< ov::XHelperInterface >& xParent,
const uno::Reference< uno::XComponentContext >& xContext, const OUString& sDocCtxName )
    : Globals_BASE( xParent, xContext )
    , msDocCtxName( sDocCtxName )
    , msApplication( RTL_CONSTASCII_USTRINGPARAM("Application") )
{
    // overwrite context with custom one ( that contains the application )
    // wrap the service manager as we don't want the disposing context to tear down the 'normal' ServiceManager ( or at least thats what the code appears like it wants to do )
    uno::Reference< uno::XInterface > aSrvMgr;
    if ( xContext.is() && xContext->getServiceManager().is() )
    {
        aSrvMgr = xContext->getServiceManager()->createInstanceWithContext( "com.sun.star.comp.stoc.OServiceManagerWrapper" , xContext );
    }

    ::cppu::ContextEntry_Init aHandlerContextInfo[] =
    {
        ::cppu::ContextEntry_Init( msApplication, uno::Any() ),
        ::cppu::ContextEntry_Init( sDocCtxName, uno::Any() ),
        ::cppu::ContextEntry_Init( "/singletons/com.sun.star.lang.theServiceManager" , uno::makeAny( aSrvMgr ) )
    };
    // don't pass a delegate, this seems to introduce yet another cyclic dependency ( and
    // some strange behavior
    mxContext = ::cppu::createComponentContext(
                        aHandlerContextInfo,
                        ( sizeof ( aHandlerContextInfo ) / sizeof ( aHandlerContextInfo[0] ) ), NULL );
    if ( aSrvMgr.is() )
    {
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
}

VbaGlobalsBase::~VbaGlobalsBase()
{
    try
    {
        uno::Reference< container::XNameContainer > xNameContainer( mxContext, uno::UNO_QUERY );
        if ( xNameContainer.is() )
        {
            // release document reference ( we don't wan't the component context trying to dispose that )
            xNameContainer->removeByName( msDocCtxName );
            // release application reference, as it is holding onto the context
            xNameContainer->removeByName( msApplication );
        }
    }
    catch ( const uno::Exception& )
    {
    }
}

void
VbaGlobalsBase::init(  const uno::Sequence< beans::PropertyValue >& aInitArgs )
{
    sal_Int32 nLen = aInitArgs.getLength();
    for ( sal_Int32 nIndex = 0; nIndex < nLen; ++nIndex )
    {
        uno::Reference< container::XNameContainer > xNameContainer( mxContext, uno::UNO_QUERY_THROW );
        if ( aInitArgs[ nIndex ].Name.equals( msApplication ) )
        {
            xNameContainer->replaceByName( msApplication, aInitArgs[ nIndex ].Value );
            uno::Reference< XHelperInterface > xParent( aInitArgs[ nIndex ].Value, uno::UNO_QUERY );
            mxParent = xParent;
        }
        else
            xNameContainer->replaceByName( aInitArgs[ nIndex ].Name, aInitArgs[ nIndex ].Value );
    }
}

uno::Reference< uno::XInterface > SAL_CALL
VbaGlobalsBase::createInstance( const OUString& aServiceSpecifier ) throw (uno::Exception, uno::RuntimeException)
{
    uno::Reference< uno::XInterface > xReturn;
    if ( aServiceSpecifier == sAppService )
    {
        // try to extract the Application from the context
        uno::Reference< container::XNameContainer > xNameContainer( mxContext, uno::UNO_QUERY );
        xNameContainer->getByName( msApplication ) >>= xReturn;
    }
    else if ( hasServiceName( aServiceSpecifier ) )
        xReturn = mxContext->getServiceManager()->createInstanceWithContext( aServiceSpecifier, mxContext );
    return xReturn;
}

uno::Reference< uno::XInterface > SAL_CALL
VbaGlobalsBase::createInstanceWithArguments( const OUString& aServiceSpecifier, const uno::Sequence< uno::Any >& Arguments ) throw (uno::Exception, uno::RuntimeException)
{

    uno::Reference< uno::XInterface > xReturn;
    if ( aServiceSpecifier == sAppService )
    {
        // try to extract the Application from the context
        uno::Reference< container::XNameContainer > xNameContainer( mxContext, uno::UNO_QUERY );
        xNameContainer->getByName( msApplication ) >>= xReturn;
    }
    else if ( hasServiceName( aServiceSpecifier ) )
        xReturn = mxContext->getServiceManager()->createInstanceWithArgumentsAndContext( aServiceSpecifier, Arguments, mxContext );
    return xReturn;
}

uno::Sequence< OUString > SAL_CALL
VbaGlobalsBase::getAvailableServiceNames(  ) throw (uno::RuntimeException)
{
    static const OUString names[] = {
    // common
        "ooo.vba.msforms.UserForm" ,
      };
    static uno::Sequence< OUString > serviceNames( names, sizeof( names )/ sizeof( names[0] ) );
    return serviceNames;
}

bool
VbaGlobalsBase::hasServiceName( const OUString& serviceName )
{
    uno::Sequence< OUString > sServiceNames( getAvailableServiceNames() );
    sal_Int32 nLen = sServiceNames.getLength();
    for ( sal_Int32 index = 0; index < nLen; ++index )
    {
        if ( sServiceNames[ index ].equals( serviceName ) )
            return true;
    }
    return false;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
