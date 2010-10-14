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
#include "vbahelper/vbaglobalbase.hxx"
#include <sal/macros.h>

#include <cppuhelper/component_context.hxx>
#include <comphelper/processfactory.hxx>
#include <com/sun/star/container/XNameContainer.hpp>

using namespace com::sun::star;
using namespace ooo::vba;

rtl::OUString sApplication( RTL_CONSTASCII_USTRINGPARAM("Application") );

// special key to return the Application
rtl::OUString sAppService( RTL_CONSTASCII_USTRINGPARAM("ooo.vba.Application") );

VbaGlobalsBase::VbaGlobalsBase(
const uno::Reference< ov::XHelperInterface >& xParent,
const uno::Reference< uno::XComponentContext >& xContext, const rtl::OUString& sDocCtxName )
:  Globals_BASE( xParent, xContext ), msDocCtxName( sDocCtxName )
{
    // overwrite context with custom one ( that contains the application )
    // wrap the service manager as we don't want the disposing context to tear down the 'normal' ServiceManager ( or at least thats what the code appears like it wants to do )
    uno::Any aSrvMgr;
    if ( xContext.is() && xContext->getServiceManager().is() )
    {
        aSrvMgr = uno::makeAny( xContext->getServiceManager()->createInstanceWithContext( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.comp.stoc.OServiceManagerWrapper") ), xContext ) );
    }

    ::cppu::ContextEntry_Init aHandlerContextInfo[] =
    {
        ::cppu::ContextEntry_Init( sApplication, uno::Any() ),
        ::cppu::ContextEntry_Init( sDocCtxName, uno::Any() ),
        ::cppu::ContextEntry_Init( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("/singletons/com.sun.star.lang.theServiceManager" ) ), aSrvMgr )
    };
    // don't pass a delegate, this seems to introduce yet another cyclic dependency ( and
    // some strange behavior
    mxContext = ::cppu::createComponentContext( aHandlerContextInfo, SAL_N_ELEMENTS( aHandlerContextInfo ), NULL );
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
            xNameContainer->removeByName( sApplication );
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
        if ( aInitArgs[ nIndex ].Name.equals( sApplication ) )
        {
            xNameContainer->replaceByName( sApplication, aInitArgs[ nIndex ].Value );
            uno::Reference< XHelperInterface > xParent( aInitArgs[ nIndex ].Value, uno::UNO_QUERY );
            mxParent = xParent;
        }
        else
            xNameContainer->replaceByName( aInitArgs[ nIndex ].Name, aInitArgs[ nIndex ].Value );
    }
}

uno::Reference< uno::XInterface > SAL_CALL
VbaGlobalsBase::createInstance( const ::rtl::OUString& aServiceSpecifier ) throw (uno::Exception, uno::RuntimeException)
{
    uno::Reference< uno::XInterface > xReturn;
    if ( aServiceSpecifier.equals( sAppService ) )
    {
        // try to extract the Application from the context
        uno::Reference< container::XNameContainer > xNameContainer( mxContext, uno::UNO_QUERY );
        xNameContainer->getByName( sApplication ) >>= xReturn;
    }
    else if ( hasServiceName( aServiceSpecifier ) )
        xReturn = mxContext->getServiceManager()->createInstanceWithContext( aServiceSpecifier, mxContext );
    return xReturn;
}

uno::Reference< uno::XInterface > SAL_CALL
VbaGlobalsBase::createInstanceWithArguments( const ::rtl::OUString& aServiceSpecifier, const uno::Sequence< uno::Any >& Arguments ) throw (uno::Exception, uno::RuntimeException)
{

    uno::Reference< uno::XInterface > xReturn;
    if ( aServiceSpecifier.equals( sAppService ) )
    {
        // try to extract the Application from the context
        uno::Reference< container::XNameContainer > xNameContainer( mxContext, uno::UNO_QUERY );
        xNameContainer->getByName( sApplication ) >>= xReturn;
    }
    else if ( hasServiceName( aServiceSpecifier ) )
        xReturn = mxContext->getServiceManager()->createInstanceWithArgumentsAndContext( aServiceSpecifier, Arguments, mxContext );
    return xReturn;
}

uno::Sequence< ::rtl::OUString > SAL_CALL
VbaGlobalsBase::getAvailableServiceNames(  ) throw (uno::RuntimeException)
{
    static const rtl::OUString names[] = {
    // common
        ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM ( "ooo.vba.msforms.UserForm" ) ),
      };
    static uno::Sequence< rtl::OUString > serviceNames( names, SAL_N_ELEMENTS( names ) );
    return serviceNames;
}

bool
VbaGlobalsBase::hasServiceName( const rtl::OUString& serviceName )
{
    uno::Sequence< rtl::OUString > sServiceNames( getAvailableServiceNames() );
    sal_Int32 nLen = sServiceNames.getLength();
    for ( sal_Int32 index = 0; index < nLen; ++index )
    {
        if ( sServiceNames[ index ].equals( serviceName ) )
            return true;
    }
    return false;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
