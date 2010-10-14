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
#include <com/sun/star/drawing/TextFitToSizeType.hpp>
#include <com/sun/star/text/XText.hpp>
#include "vbatextframe.hxx"
#include "vbacharacters.hxx"

using namespace ::ooo::vba;
using namespace ::com::sun::star;

ScVbaTextFrame::ScVbaTextFrame( uno::Sequence< uno::Any> const & args, uno::Reference< uno::XComponentContext> const & xContext ) throw ( lang::IllegalArgumentException ) :  ScVbaTextFrame_BASE( getXSomethingFromArgs< XHelperInterface >( args, 0 ), xContext, getXSomethingFromArgs< drawing::XShape >( args, 1, false ) )
{
}

// Methods
uno::Any SAL_CALL
ScVbaTextFrame::Characters() throw (uno::RuntimeException)
{
    uno::Reference< text::XSimpleText > xSimpleText( m_xShape, uno::UNO_QUERY_THROW );
    ScVbaPalette aPalette( SfxObjectShell::Current() );
    uno::Any aStart( sal_Int32( 1 ) );
    uno::Any aLength(sal_Int32( -1 ) );
    return uno::makeAny( uno::Reference< ov::excel::XCharacters >( new ScVbaCharacters( this, mxContext, aPalette, xSimpleText, aStart, aLength, sal_True ) ) );
}

rtl::OUString&
ScVbaTextFrame::getServiceImplName()
{
    static rtl::OUString sImplName( RTL_CONSTASCII_USTRINGPARAM("ScVbaTextFrame") );
    return sImplName;
}

uno::Sequence< rtl::OUString >
ScVbaTextFrame::getServiceNames()
{
    static uno::Sequence< rtl::OUString > aServiceNames;
    if ( aServiceNames.getLength() == 0 )
    {
        aServiceNames.realloc( 1 );
        aServiceNames[ 0 ] = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("ooo.vba.excel.TextFrame" ) );
    }
    return aServiceNames;
}

namespace textframe
{
namespace sdecl = comphelper::service_decl;
sdecl::vba_service_class_<ScVbaTextFrame, sdecl::with_args<true> > serviceImpl;
extern sdecl::ServiceDecl const serviceDecl(
    serviceImpl,
    "ScVbaTextFrame",
    "ooo.vba.excel.TextFrame" );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
