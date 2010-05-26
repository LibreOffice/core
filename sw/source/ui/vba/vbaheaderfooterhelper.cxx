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
#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#include <com/sun/star/container/XNameAccess.hpp>

using namespace ::com::sun::star;
using namespace ::ooo::vba;

#define FIRST_PAGE 1;

// Class HeaderFooterHelper

sal_Bool HeaderFooterHelper::isHeader( const uno::Reference< frame::XModel >& xModel, const uno::Reference< text::XText >& xCurrentText ) throw (uno::RuntimeException)
{
    uno::Reference< text::XPageCursor > xPageCursor( word::getXTextViewCursor( xModel ), uno::UNO_QUERY_THROW );
    uno::Reference< beans::XPropertySet > xStyleProps( word::getCurrentPageStyle( xModel ), uno::UNO_QUERY_THROW );

    sal_Bool isOn = sal_False;
    xStyleProps->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("HeaderIsOn"))) >>= isOn;
    if( !isOn )
        return sal_False;

    sal_Bool isShared = sal_False;
    xStyleProps->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("HeaderIsShared"))) >>= isShared;

    rtl::OUString aPropText = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("HeaderText") );
    if( !isShared )
    {
        if( 0 == xPageCursor->getPage() % 2 )
        {
            aPropText = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("HeaderTextLeft") );
        }
        else
        {
            aPropText = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("HeaderTextRight") );
        }
    }

    uno::Reference< text::XText > xText( xStyleProps->getPropertyValue( aPropText ), uno::UNO_QUERY_THROW );
    //FIXME: can not compare in this way?
    return ( xText == xCurrentText );
}

sal_Bool HeaderFooterHelper::isFirstPageHeader( const uno::Reference< frame::XModel >& xModel, const uno::Reference< text::XText >& xCurrentText ) throw (uno::RuntimeException)
{
    if( isHeader( xModel, xCurrentText ) )
    {
        uno::Reference< text::XPageCursor > xPageCursor( word::getXTextViewCursor( xModel ), uno::UNO_QUERY_THROW );
        // FIXME: getPage allways returns 1
        sal_Int32 nPage = xPageCursor->getPage();
        return nPage == FIRST_PAGE;
    }
    return sal_False;
}

sal_Bool HeaderFooterHelper::isEvenPagesHeader( const uno::Reference< frame::XModel >& xModel, const uno::Reference< text::XText >& xCurrentText ) throw (uno::RuntimeException)
{
    if( isHeader( xModel, xCurrentText ) )
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

sal_Bool HeaderFooterHelper::isFooter( const uno::Reference< frame::XModel >& xModel, const uno::Reference< text::XText >& xCurrentText ) throw (uno::RuntimeException)
{
    uno::Reference< text::XPageCursor > xPageCursor( word::getXTextViewCursor( xModel ), uno::UNO_QUERY_THROW );
    uno::Reference< beans::XPropertySet > xStyleProps( word::getCurrentPageStyle( xModel ), uno::UNO_QUERY_THROW );

    sal_Bool isOn = sal_False;
    xStyleProps->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("FooterIsOn"))) >>= isOn;
    if( !isOn )
        return sal_False;

    sal_Bool isShared = sal_False;
    xStyleProps->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("FooterIsShared"))) >>= isShared;

    rtl::OUString aPropText = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("FooterText") );
    if( !isShared )
    {
        if( 0 == xPageCursor->getPage() % 2 )
        {
            aPropText = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("FooterTextLeft") );
        }
        else
        {
            aPropText = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("FooterTextRight") );
        }
    }

    uno::Reference< text::XText > xText( xStyleProps->getPropertyValue( aPropText ), uno::UNO_QUERY_THROW );

    return ( xText == xCurrentText );
}

sal_Bool HeaderFooterHelper::isFirstPageFooter( const uno::Reference< frame::XModel >& xModel, const uno::Reference< text::XText >& xCurrentText ) throw (uno::RuntimeException)
{
    if( isFooter( xModel, xCurrentText ) )
    {
        uno::Reference< text::XPageCursor > xPageCursor( word::getXTextViewCursor( xModel ), uno::UNO_QUERY_THROW );
        sal_Int32 nPage = xPageCursor->getPage();
        return nPage == FIRST_PAGE;
    }
    return sal_False;
}

sal_Bool HeaderFooterHelper::isEvenPagesFooter( const uno::Reference< frame::XModel >& xModel, const uno::Reference< text::XText >& xCurrentText ) throw (uno::RuntimeException)
{
    if( isFooter( xModel, xCurrentText ) )
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
    if( isHeader( xModel, xCurrentText ) )
    {
        return( !( isFirstPageHeader( xModel, xCurrentText ) && isEvenPagesHeader( xModel, xCurrentText ) ) );
    }
    return sal_False;
}

sal_Bool HeaderFooterHelper::isPrimaryFooter( const uno::Reference< frame::XModel >& xModel, const uno::Reference< text::XText >& xCurrentText ) throw (uno::RuntimeException)
{
    if( isHeader( xModel, xCurrentText ) )
    {
        return( !( isFirstPageFooter( xModel, xCurrentText ) && isEvenPagesFooter( xModel, xCurrentText ) ) );
    }
    return sal_False;
}
#endif
