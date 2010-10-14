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
#include "vbaheaderfooter.hxx"
#include <vbahelper/vbahelper.hxx>
#include <tools/diagnose_ex.h>
#include <ooo/vba/word/WdHeaderFooterIndex.hpp>
#include <com/sun/star/text/XText.hpp>
#include <com/sun/star/text/XTextDocument.hpp>
#include <com/sun/star/drawing/XDrawPageSupplier.hpp>
#include "vbarange.hxx"
#include <vbahelper/vbashapes.hxx>

using namespace ::ooo::vba;
using namespace ::com::sun::star;

SwVbaHeaderFooter::SwVbaHeaderFooter( const uno::Reference< ooo::vba::XHelperInterface >& rParent, const uno::Reference< uno::XComponentContext >& rContext, const uno::Reference< frame::XModel >& xModel, const uno::Reference< beans::XPropertySet >& rProps, sal_Bool isHeader, sal_Int32 index ) throw ( uno::RuntimeException ) : SwVbaHeaderFooter_BASE( rParent, rContext ), mxModel( xModel ), mxPageStyleProps( rProps ), mbHeader( isHeader ), mnIndex( index )
{
}

sal_Bool SAL_CALL SwVbaHeaderFooter::getIsHeader() throw (uno::RuntimeException)
{
    return mbHeader;
}

sal_Bool SAL_CALL SwVbaHeaderFooter::getLinkToPrevious() throw (uno::RuntimeException)
{
    // seems always false
    return sal_False;
}

void SAL_CALL SwVbaHeaderFooter::setLinkToPrevious( ::sal_Bool /*_linktoprevious*/ ) throw (uno::RuntimeException)
{
    // not support in Writer
}

uno::Reference< word::XRange > SAL_CALL SwVbaHeaderFooter::getRange() throw (uno::RuntimeException)
{
    rtl::OUString sPropsNameText;
    if( mbHeader )
    {
        sPropsNameText = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("HeaderText") );
    }
    else
    {
        sPropsNameText = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("FooterText") );
    }
    if( mnIndex == word::WdHeaderFooterIndex::wdHeaderFooterEvenPages )
    {
        sPropsNameText.concat( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("Left") ) );
    }

    uno::Reference< text::XText > xText( mxPageStyleProps->getPropertyValue( sPropsNameText ), uno::UNO_QUERY_THROW );
    uno::Reference< text::XTextDocument > xDocument( mxModel, uno::UNO_QUERY_THROW );
    return uno::Reference< word::XRange >( new SwVbaRange( this, mxContext, xDocument, xText->getStart(), xText->getEnd(), xText ) );
}

uno::Any SAL_CALL
SwVbaHeaderFooter::Shapes( const uno::Any& index ) throw (uno::RuntimeException)
{
    // #FIXME: only get the shapes in the current header/footer
    uno::Reference< drawing::XDrawPageSupplier > xDrawPageSupplier( mxModel, uno::UNO_QUERY_THROW );
    //uno::Reference< drawing::XShapes > xShapes( xDrawPageSupplier->getDrawPage(), uno::UNO_QUERY_THROW );
    uno::Reference< container::XIndexAccess > xIndexAccess( xDrawPageSupplier->getDrawPage(), uno::UNO_QUERY_THROW );
    uno::Reference< XCollection > xCol( new ScVbaShapes( this, mxContext, xIndexAccess, mxModel ) );
    if ( index.hasValue() )
        return xCol->Item( index, uno::Any() );
    return uno::makeAny( xCol );
}

rtl::OUString&
SwVbaHeaderFooter::getServiceImplName()
{
    static rtl::OUString sImplName( RTL_CONSTASCII_USTRINGPARAM("SwVbaHeaderFooter") );
    return sImplName;
}

uno::Sequence< rtl::OUString >
SwVbaHeaderFooter::getServiceNames()
{
    static uno::Sequence< rtl::OUString > aServiceNames;
    if ( aServiceNames.getLength() == 0 )
    {
        aServiceNames.realloc( 1 );
        aServiceNames[ 0 ] = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("ooo.vba.word.Pane" ) );
    }
    return aServiceNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
