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

#include "vbaeventshelper.hxx"
#include <com/sun/star/script/ModuleType.hpp>
#include <com/sun/star/script/vba/VBAEventId.hpp>
#include <vbahelper/helperdecl.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::script::vba::VBAEventId;
using namespace ::ooo::vba;

// ============================================================================

SwVbaEventsHelper::SwVbaEventsHelper( uno::Sequence< css::uno::Any > const& aArgs, uno::Reference< uno::XComponentContext > const& xContext ) :
    VbaEventsHelperBase( aArgs, xContext )
{
    using namespace ::com::sun::star::script::ModuleType;
    registerEventHandler( DOCUMENT_NEW,     DOCUMENT,   "Document_New" );
    registerEventHandler( AUTO_NEW,         NORMAL,     "AutoNew" );
    registerEventHandler( DOCUMENT_OPEN,    DOCUMENT,   "Document_Open" );
    registerEventHandler( AUTO_OPEN,        NORMAL,     "AutoOpen" );
    registerEventHandler( DOCUMENT_CLOSE,   DOCUMENT,   "Document_Close" );
    registerEventHandler( AUTO_CLOSE,       NORMAL,     "AutoClose" );
}

SwVbaEventsHelper::~SwVbaEventsHelper()
{
}

bool SwVbaEventsHelper::implPrepareEvent( EventQueue& rEventQueue,
        const EventHandlerInfo& rInfo, const uno::Sequence< uno::Any >& /*rArgs*/ ) throw (uno::RuntimeException)
{
    switch( rInfo.mnEventId )
    {
        case DOCUMENT_NEW:
            rEventQueue.push_back( AUTO_NEW );
        break;
        case DOCUMENT_OPEN:
            rEventQueue.push_back( AUTO_OPEN );
        break;
        case DOCUMENT_CLOSE:
            rEventQueue.push_back( AUTO_CLOSE );
        break;
    }
    return true;
}

uno::Sequence< uno::Any > SwVbaEventsHelper::implBuildArgumentList( const EventHandlerInfo& /*rInfo*/,
        const uno::Sequence< uno::Any >& /*rArgs*/ ) throw (lang::IllegalArgumentException)
{
    // no event handler expects any arguments
    return uno::Sequence< uno::Any >();
}

void SwVbaEventsHelper::implPostProcessEvent( EventQueue& /*rEventQueue*/,
        const EventHandlerInfo& /*rInfo*/, bool /*bCancel*/ ) throw (uno::RuntimeException)
{
    // nothing to do after any event
}

::rtl::OUString SwVbaEventsHelper::implGetDocumentModuleName( const EventHandlerInfo& /*rInfo*/,
        const uno::Sequence< uno::Any >& /*rArgs*/ ) const throw (lang::IllegalArgumentException)
{
    // TODO: get actual codename from document
    return ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "ThisDocument" ) );
}

// ============================================================================

namespace vbaeventshelper
{
namespace sdecl = comphelper::service_decl;
sdecl::class_<SwVbaEventsHelper, sdecl::with_args<true> > serviceImpl;
extern sdecl::ServiceDecl const serviceDecl(
    serviceImpl,
    "SwVbaEventsHelper",
    "com.sun.star.document.vba.VBATextEventProcessor" );
}

// ============================================================================
