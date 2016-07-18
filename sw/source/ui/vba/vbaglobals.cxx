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
#include <vbahelper/helperdecl.hxx>
#include "vbaglobals.hxx"
#include <osl/diagnose.h>
#include <sal/macros.h>
#include <comphelper/unwrapargs.hxx>

#include <com/sun/star/lang/XMultiComponentFactory.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <cppuhelper/bootstrap.hxx>
#include "vbaapplication.hxx"
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::ooo::vba;

SwVbaGlobals::SwVbaGlobals(  uno::Sequence< uno::Any > const& aArgs, uno::Reference< uno::XComponentContext >const& rxContext ) : SwVbaGlobals_BASE( uno::Reference< XHelperInterface >(), rxContext, "WordDocumentContext" )
{
    OSL_TRACE("SwVbaGlobals::SwVbaGlobals()");
        uno::Sequence< beans::PropertyValue > aInitArgs( 2 );
        aInitArgs[ 0 ].Name = "Application";
        aInitArgs[ 0 ].Value = uno::makeAny( getApplication() );
        aInitArgs[ 1 ].Name = "WordDocumentContext";
        aInitArgs[ 1 ].Value = uno::makeAny( getXSomethingFromArgs< frame::XModel >( aArgs, 0 ) );

        init( aInitArgs );
}

SwVbaGlobals::~SwVbaGlobals()
{
    OSL_TRACE("SwVbaGlobals::~SwVbaGlobals");
}

// XGlobals

uno::Reference<word::XApplication > const &
SwVbaGlobals::getApplication() throw (uno::RuntimeException)
{
    OSL_TRACE("In SwVbaGlobals::getApplication");
    if ( !mxApplication.is() )
         mxApplication.set( new SwVbaApplication( mxContext) );

       return mxApplication;
}

uno::Reference<word::XSystem > SAL_CALL
SwVbaGlobals::getSystem() throw (uno::RuntimeException, std::exception)
{
    return getApplication()->getSystem();
}

uno::Reference< word::XDocument > SAL_CALL
SwVbaGlobals::getActiveDocument() throw (uno::RuntimeException, std::exception)
{
    return getApplication()->getActiveDocument();
}

uno::Reference< word::XWindow > SAL_CALL
SwVbaGlobals::getActiveWindow() throw (uno::RuntimeException, std::exception)
{
    return getApplication()->getActiveWindow();
}

OUString SAL_CALL
SwVbaGlobals::getName() throw (uno::RuntimeException, std::exception)
{
    return getApplication()->getName();
}

uno::Reference<word::XOptions > SAL_CALL
SwVbaGlobals::getOptions() throw (uno::RuntimeException, std::exception)
{
    return getApplication()->getOptions();
}

uno::Any SAL_CALL
SwVbaGlobals::CommandBars( const uno::Any& aIndex ) throw (uno::RuntimeException, std::exception)
{
    return getApplication()->CommandBars( aIndex );
}

uno::Any SAL_CALL
SwVbaGlobals::Documents( const uno::Any& index ) throw (uno::RuntimeException, std::exception)
{
    return getApplication()->Documents( index );
}

uno::Any SAL_CALL
SwVbaGlobals::Addins( const uno::Any& index ) throw (uno::RuntimeException, std::exception)
{
    return getApplication()->Addins( index );
}

uno::Any SAL_CALL
SwVbaGlobals::Dialogs( const uno::Any& index ) throw (uno::RuntimeException, std::exception)
{
    return getApplication()->Dialogs( index );
}

uno::Any SAL_CALL
SwVbaGlobals::ListGalleries( const uno::Any& index ) throw (uno::RuntimeException, std::exception)
{
    return getApplication()->ListGalleries( index );
}

uno::Reference<word::XSelection > SAL_CALL
SwVbaGlobals::getSelection() throw (uno::RuntimeException, std::exception)
{
    return getApplication()->getSelection();
}

float SAL_CALL SwVbaGlobals::CentimetersToPoints( float Centimeters ) throw (uno::RuntimeException, std::exception)
{
    return getApplication()->CentimetersToPoints( Centimeters );
}

OUString
SwVbaGlobals::getServiceImplName()
{
    return OUString("SwVbaGlobals");
}

uno::Sequence< OUString >
SwVbaGlobals::getServiceNames()
{
        static uno::Sequence< OUString > aServiceNames;
        if ( aServiceNames.getLength() == 0 )
        {
                aServiceNames.realloc( 1 );
                aServiceNames[ 0 ] = "ooo.vba.word.Globals";
        }
        return aServiceNames;
}

uno::Sequence< OUString >
SwVbaGlobals::getAvailableServiceNames(  ) throw (uno::RuntimeException, std::exception)
{
    static bool bInit = false;
    static uno::Sequence< OUString > serviceNames( SwVbaGlobals_BASE::getAvailableServiceNames() );
    if ( !bInit )
    {
         OUString names[] = {
            OUString( "ooo.vba.word.Document" ),
//            #FIXME #TODO make Application a proper service
//            OUString( "ooo.vba.word.Application" ),
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
