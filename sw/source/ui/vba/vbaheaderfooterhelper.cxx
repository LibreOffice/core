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
#include "vbaheaderfooterhelper.hxx"
#include "wordvbahelper.hxx"
#include <comphelper/processfactory.hxx>
#include <com/sun/star/frame/XController.hpp>
#include <com/sun/star/text/XTextViewCursorSupplier.hpp>
#include <com/sun/star/text/XTextRangeCompare.hpp>
#include <com/sun/star/text/XTextRange.hpp>
#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>

using namespace ::com::sun::star;
using namespace ::ooo::vba;

#define FIRST_PAGE 1;

// Class HeaderFooterHelper
sal_Bool HeaderFooterHelper::isHeaderFooter( const uno::Reference< frame::XModel >& xModel ) throw (uno::RuntimeException)
{
    return isHeaderFooter( word::getCurrentXText( xModel ) );
}

sal_Bool HeaderFooterHelper::isHeaderFooter( const uno::Reference< text::XText >& xText ) throw (uno::RuntimeException)
{
    uno::Reference< lang::XServiceInfo > xServiceInfo( xText, uno::UNO_QUERY_THROW );
    rtl::OUString aImplName = xServiceInfo->getImplementationName();
    if( aImplName.equalsAscii("SwXHeadFootText") )
        return sal_True;
    return sal_False;
}

sal_Bool HeaderFooterHelper::isHeader( const uno::Reference< frame::XModel >& xModel ) throw (uno::RuntimeException)
{
    const uno::Reference< text::XText > xCurrentText = word::getCurrentXText( xModel );
    if( !isHeaderFooter( xCurrentText ) )
        return sal_False;

    rtl::OUString aPropIsShared = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("HeaderIsShared") );
    rtl::OUString aPropText = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("HeaderText") );
    uno::Reference< style::XStyle > xPageStyle = word::getCurrentPageStyle( xModel );
    uno::Reference< beans::XPropertySet > xPageProps( xPageStyle, uno::UNO_QUERY_THROW );
    sal_Bool isShared = sal_True;
    xPageProps->getPropertyValue( aPropIsShared ) >>= isShared;
    if( !isShared )
    {
        uno::Reference< text::XPageCursor > xPageCursor( word::getXTextViewCursor( xModel ), uno::UNO_QUERY_THROW );
        if( 0 == xPageCursor->getPage() % 2 )
            aPropText = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("HeaderTextLeft") );
        else
            aPropText = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("HeaderTextRight") );
    }

    uno::Reference< text::XText > xHeaderText( xPageProps->getPropertyValue( aPropText ), uno::UNO_QUERY_THROW );
    uno::Reference< text::XTextRangeCompare > xTRC( xHeaderText, uno::UNO_QUERY_THROW );
    uno::Reference< text::XTextRange > xTR1( xCurrentText, uno::UNO_QUERY_THROW );
    uno::Reference< text::XTextRange > xTR2( xHeaderText, uno::UNO_QUERY_THROW );
    try
    {
        if( xTRC->compareRegionStarts( xTR1, xTR2 ) == 0 )
            return sal_True;
    }
    catch( lang::IllegalArgumentException& )
    {
        return sal_False;
    }

    return sal_False;
}

sal_Bool HeaderFooterHelper::isFirstPageHeader( const uno::Reference< frame::XModel >& xModel ) throw (uno::RuntimeException)
{
    if( isHeader( xModel ) )
    {
        uno::Reference< text::XPageCursor > xPageCursor( word::getXTextViewCursor( xModel ), uno::UNO_QUERY_THROW );
        // FIXME: getPage allways returns 1
        sal_Int32 nPage = xPageCursor->getPage();
        return nPage == FIRST_PAGE;
    }
    return sal_False;
}

sal_Bool HeaderFooterHelper::isEvenPagesHeader( const uno::Reference< frame::XModel >& xModel ) throw (uno::RuntimeException)
{
    if( isHeader( xModel ) )
    {
        uno::Reference< beans::XPropertySet > xStyleProps( word::getCurrentPageStyle( xModel ), uno::UNO_QUERY_THROW );
        sal_Bool isShared = sal_False;
        xStyleProps->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("HeaderIsShared"))) >>= isShared;
        if( !isShared )
        {
            uno::Reference< text::XPageCursor > xPageCursor( word::getXTextViewCursor( xModel ), uno::UNO_QUERY_THROW );
            return ( 0 == xPageCursor->getPage() % 2 );
        }
    }
    return sal_False;
}

sal_Bool HeaderFooterHelper::isFooter( const uno::Reference< frame::XModel >& xModel ) throw (uno::RuntimeException)
{
    const uno::Reference< text::XText > xCurrentText = word::getCurrentXText( xModel );
    if( !isHeaderFooter( xCurrentText ) )
        return sal_False;

    rtl::OUString aPropIsShared = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("FooterIsShared") );
    rtl::OUString aPropText = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("FooterText") );
    uno::Reference< style::XStyle > xPageStyle = word::getCurrentPageStyle( xModel );
    uno::Reference< beans::XPropertySet > xPageProps( xPageStyle, uno::UNO_QUERY_THROW );
    sal_Bool isShared = sal_True;
    xPageProps->getPropertyValue( aPropIsShared ) >>= isShared;
    if( !isShared )
    {
        uno::Reference< text::XPageCursor > xPageCursor( word::getXTextViewCursor( xModel ), uno::UNO_QUERY_THROW );
        if( 0 == xPageCursor->getPage() % 2 )
            aPropText = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("FooterTextLeft") );
        else
            aPropText = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("FooterTextRight") );
    }

    uno::Reference< text::XText > xFooterText( xPageProps->getPropertyValue( aPropText ), uno::UNO_QUERY_THROW );
    uno::Reference< text::XTextRangeCompare > xTRC( xFooterText, uno::UNO_QUERY_THROW );
    uno::Reference< text::XTextRange > xTR1( xCurrentText, uno::UNO_QUERY_THROW );
    uno::Reference< text::XTextRange > xTR2( xFooterText, uno::UNO_QUERY_THROW );
    try
    {
        if( xTRC->compareRegionStarts( xTR1, xTR2 ) == 0 )
            return sal_True;
    }
    catch( lang::IllegalArgumentException& )
    {
        return sal_False;
    }

    return sal_False;
}

sal_Bool HeaderFooterHelper::isFirstPageFooter( const uno::Reference< frame::XModel >& xModel ) throw (uno::RuntimeException)
{
    if( isFooter( xModel ) )
    {
        uno::Reference< text::XPageCursor > xPageCursor( word::getXTextViewCursor( xModel ), uno::UNO_QUERY_THROW );
        sal_Int32 nPage = xPageCursor->getPage();
        return nPage == FIRST_PAGE;
    }
    return sal_False;
}

sal_Bool HeaderFooterHelper::isEvenPagesFooter( const uno::Reference< frame::XModel >& xModel ) throw (uno::RuntimeException)
{
    if( isFooter( xModel ) )
    {
        uno::Reference< beans::XPropertySet > xStyleProps( word::getCurrentPageStyle( xModel ), uno::UNO_QUERY_THROW );
        sal_Bool isShared = sal_False;
        xStyleProps->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("FooterIsShared"))) >>= isShared;
        if( !isShared )
        {
            uno::Reference< text::XPageCursor > xPageCursor( word::getXTextViewCursor( xModel ), uno::UNO_QUERY_THROW );
            return ( 0 == xPageCursor->getPage() % 2 );
        }
    }
    return sal_False;
}
#ifdef TOMORROW
sal_Bool HeaderFooterHelper::isPrimaryHeader( const uno::Reference< frame::XModel >& xModel, const uno::Reference< text::XText >& xCurrentText ) throw (uno::RuntimeException)
{
    if( isHeader( xModel ) )
    {
        return( !( isFirstPageHeader( xModel ) && isEvenPagesHeader( xModel ) ) );
    }
    return sal_False;
}

sal_Bool HeaderFooterHelper::isPrimaryFooter( const uno::Reference< frame::XModel >& xModel ) throw (uno::RuntimeException)
{
    if( isHeader( xModel ) )
    {
        return( !( isFirstPageFooter( xModel ) && isEvenPagesFooter( xModel ) ) );
    }
    return sal_False;
}
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
