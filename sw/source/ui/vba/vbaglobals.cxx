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
#include <vbahelper/helperdecl.hxx>
#include "vbaglobals.hxx"
#include <sal/macros.h>
#include <comphelper/unwrapargs.hxx>

#include <com/sun/star/lang/XMultiComponentFactory.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <cppuhelper/bootstrap.hxx>
#include "vbaapplication.hxx"
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::ooo::vba;


rtl::OUString sDocCtxName( RTL_CONSTASCII_USTRINGPARAM("WordDocumentContext") );

// =============================================================================
// SwVbaGlobals
// =============================================================================

SwVbaGlobals::SwVbaGlobals(  uno::Sequence< uno::Any > const& aArgs, uno::Reference< uno::XComponentContext >const& rxContext ) : SwVbaGlobals_BASE( uno::Reference< XHelperInterface >(), rxContext, sDocCtxName )
{
    OSL_TRACE("SwVbaGlobals::SwVbaGlobals()");
        uno::Sequence< beans::PropertyValue > aInitArgs( 2 );
        aInitArgs[ 0 ].Name = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Application"));
        aInitArgs[ 0 ].Value = uno::makeAny( getApplication() );
        aInitArgs[ 1 ].Name = sDocCtxName;
        aInitArgs[ 1 ].Value = uno::makeAny( getXSomethingFromArgs< frame::XModel >( aArgs, 0 ) );

        init( aInitArgs );
}

SwVbaGlobals::~SwVbaGlobals()
{
    OSL_TRACE("SwVbaGlobals::~SwVbaGlobals");
}

// =============================================================================
// XGlobals
// =============================================================================
uno::Reference<word::XApplication >
SwVbaGlobals::getApplication() throw (uno::RuntimeException)
{
    OSL_TRACE("In SwVbaGlobals::getApplication");
    if ( !mxApplication.is() )
         mxApplication.set( new SwVbaApplication( mxContext) );

       return mxApplication;
}

uno::Reference<word::XSystem > SAL_CALL
SwVbaGlobals::getSystem() throw (uno::RuntimeException)
{
    return getApplication()->getSystem();
}

uno::Reference< word::XDocument > SAL_CALL
SwVbaGlobals::getActiveDocument() throw (uno::RuntimeException)
{
    return getApplication()->getActiveDocument();
}

uno::Reference< word::XWindow > SAL_CALL
SwVbaGlobals::getActiveWindow() throw (uno::RuntimeException)
{
    return getApplication()->getActiveWindow();
}

rtl::OUString SAL_CALL
SwVbaGlobals::getName() throw (uno::RuntimeException)
{
    return getApplication()->getName();
}

uno::Reference<word::XOptions > SAL_CALL
SwVbaGlobals::getOptions() throw (uno::RuntimeException)
{
    return getApplication()->getOptions();
}

uno::Any SAL_CALL
SwVbaGlobals::CommandBars( const uno::Any& aIndex ) throw (uno::RuntimeException)
{
    return getApplication()->CommandBars( aIndex );
}

uno::Any SAL_CALL
SwVbaGlobals::Documents( const uno::Any& index ) throw (uno::RuntimeException)
{
    return getApplication()->Documents( index );
}

uno::Any SAL_CALL
SwVbaGlobals::Addins( const uno::Any& index ) throw (uno::RuntimeException)
{
    return getApplication()->Addins( index );
}

uno::Any SAL_CALL
SwVbaGlobals::Dialogs( const uno::Any& index ) throw (uno::RuntimeException)
{
    return getApplication()->Dialogs( index );
}

uno::Any SAL_CALL
SwVbaGlobals::ListGalleries( const uno::Any& index ) throw (uno::RuntimeException)
{
    return getApplication()->ListGalleries( index );
}

uno::Reference<word::XSelection > SAL_CALL
SwVbaGlobals::getSelection() throw (uno::RuntimeException)
{
    return getApplication()->getSelection();
}

float SAL_CALL SwVbaGlobals::CentimetersToPoints( float _Centimeters ) throw (uno::RuntimeException)
{
    return getApplication()->CentimetersToPoints( _Centimeters );
}

rtl::OUString&
SwVbaGlobals::getServiceImplName()
{
        static rtl::OUString sImplName( RTL_CONSTASCII_USTRINGPARAM("SwVbaGlobals") );
        return sImplName;
}

uno::Sequence< rtl::OUString >
SwVbaGlobals::getServiceNames()
{
        static uno::Sequence< rtl::OUString > aServiceNames;
        if ( aServiceNames.getLength() == 0 )
        {
                aServiceNames.realloc( 1 );
                aServiceNames[ 0 ] = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("ooo.vba.word.Globals" ) );
        }
        return aServiceNames;
}

uno::Sequence< rtl::OUString >
SwVbaGlobals::getAvailableServiceNames(  ) throw (uno::RuntimeException)
{
    static bool bInit = false;
    static uno::Sequence< rtl::OUString > serviceNames( SwVbaGlobals_BASE::getAvailableServiceNames() );
    if ( !bInit )
    {
         rtl::OUString names[] = {
            ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM ( "ooo.vba.word.Document" ) ),
//            #FIXME #TODO make Application a proper service
//            ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM ( "ooo.vba.word.Application" ) ),
        };
        sal_Int32 nWordServices = SAL_N_ELEMENTS( names );
        sal_Int32 startIndex = serviceNames.getLength();
        serviceNames.realloc( serviceNames.getLength() + nWordServices );
        for ( sal_Int32 index = 0; index < nWordServices; ++index )
             serviceNames[ startIndex + index ] = names[ index ];
        bInit = true;
    }
    return serviceNames;
}

namespace globals
{
namespace sdecl = comphelper::service_decl;
sdecl::vba_service_class_<SwVbaGlobals, sdecl::with_args<true> > serviceImpl;
extern sdecl::ServiceDecl const serviceDecl(
    serviceImpl,
    "SwVbaGlobals",
    "ooo.vba.word.Globals" );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
