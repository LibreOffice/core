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
    if ( aImplName == "SwXHeadFootText" )
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
    catch (const lang::IllegalArgumentException&)
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
        // FIXME: getPage always returns 1
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
    catch (const lang::IllegalArgumentException&)
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
