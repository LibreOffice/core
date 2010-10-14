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
#include "vbapagesetup.hxx"
#include <com/sun/star/text/XText.hpp>
#include <com/sun/star/text/XPageCursor.hpp>
#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <ooo/vba/word/WdSectionStart.hpp>
#include <ooo/vba/word/WdOrientation.hpp>
#include "wordvbahelper.hxx"

using namespace ::com::sun::star;
using namespace ::ooo::vba;

SwVbaPageSetup::SwVbaPageSetup(const uno::Reference< XHelperInterface >& xParent,
                const uno::Reference< uno::XComponentContext >& xContext,
                const uno::Reference< frame::XModel >& xModel,
                const uno::Reference< beans::XPropertySet >& xProps ) throw (uno::RuntimeException):
           SwVbaPageSetup_BASE( xParent, xContext )
{
    mxModel.set( xModel, uno::UNO_QUERY_THROW );
    mxPageProps.set( xProps, uno::UNO_QUERY_THROW );
    mnOrientPortrait = word::WdOrientation::wdOrientPortrait;
    mnOrientLandscape = word::WdOrientation::wdOrientLandscape;
}

double SAL_CALL SwVbaPageSetup::getGutter() throw (uno::RuntimeException)
{
    // not support in Writer
    return 0;
}

void SAL_CALL SwVbaPageSetup::setGutter( double _gutter ) throw (uno::RuntimeException)
{
    // default add gutter into left margin
    if( _gutter != 0 )
    {
        double margin = VbaPageSetupBase::getLeftMargin() + _gutter;
        VbaPageSetupBase::setLeftMargin( margin );
    }
}

double SAL_CALL SwVbaPageSetup::getHeaderDistance() throw (uno::RuntimeException)
{
    sal_Bool isHeaderOn = sal_False;
    mxPageProps->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("HeaderIsOn"))) >>= isHeaderOn;
    if( !isHeaderOn )
        mxPageProps->setPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("HeaderIsOn")), uno::makeAny( sal_True ) );
    return VbaPageSetupBase::getHeaderMargin();
}

    /**
     * changes the value of TopMargin to the value of new MS-Word-HeaderDistance. Subtracts the difference
     * between old TopMargin and the new headerDistance from the value of HeaderSpacing (which defines the
     * space between the header and the body of the text). calculates the new HeaderHeight (= height of the
     * header + headerBodyDistance).
     *
     * @param: headerDistance is the value that is set in MS Word for the distance from the top of the page
     *          to the header
     */
void SAL_CALL SwVbaPageSetup::setHeaderDistance( double _headerdistance ) throw (uno::RuntimeException)
{
    sal_Int32 newHeaderDistance = Millimeter::getInHundredthsOfOneMillimeter( _headerdistance );
    sal_Bool isHeaderOn = sal_False;
    sal_Int32 aktTopMargin = 0;
    sal_Int32 aktSpacing = 0;
    sal_Int32 aktHeaderHeight = 0;

    mxPageProps->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("HeaderIsOn"))) >>= isHeaderOn;
    if( !isHeaderOn )
        mxPageProps->setPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("HeaderIsOn")), uno::makeAny( sal_True ) );

    mxPageProps->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("TopMargin"))) >>= aktTopMargin;
    mxPageProps->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("HeaderBodyDistance"))) >>= aktSpacing;
    mxPageProps->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("HeaderHeight"))) >>= aktHeaderHeight;

    sal_Int32 newSpacing = aktSpacing - ( newHeaderDistance - aktTopMargin );
    sal_Int32 height = aktHeaderHeight - aktSpacing;
    sal_Int32 newHeaderHeight = newSpacing + height;

    mxPageProps->setPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("TopMargin")), uno::makeAny( newHeaderDistance ) );
    mxPageProps->setPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("HeaderBodyDistance")), uno::makeAny( newSpacing ) );
    mxPageProps->setPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("HeaderHeight")), uno::makeAny( newHeaderHeight ) );
}

double SAL_CALL SwVbaPageSetup::getFooterDistance() throw (uno::RuntimeException)
{
    sal_Bool isFooterOn = sal_False;
    mxPageProps->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("FooterIsOn"))) >>= isFooterOn;
    if( !isFooterOn )
        mxPageProps->setPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("FooterIsOn")), uno::makeAny( sal_True ) );
    return VbaPageSetupBase::getFooterMargin();
}

void SAL_CALL SwVbaPageSetup::setFooterDistance( double _footerdistance ) throw (uno::RuntimeException)
{
    sal_Int32 newFooterDistance = Millimeter::getInHundredthsOfOneMillimeter( _footerdistance );
    sal_Bool isFooterOn = sal_False;
    sal_Int32 aktBottomMargin = 0;
    sal_Int32 aktSpacing = 0;
    sal_Int32 aktFooterHeight = 0;

    mxPageProps->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("FooterIsOn"))) >>= isFooterOn;
    if( !isFooterOn )
        mxPageProps->setPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("FooterIsOn")), uno::makeAny( sal_True ) );

    mxPageProps->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("BottomMargin"))) >>= aktBottomMargin;
    mxPageProps->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("FooterBodyDistance"))) >>= aktSpacing;
    mxPageProps->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("FooterHeight"))) >>= aktFooterHeight;

    sal_Int32 newSpacing = aktSpacing - ( newFooterDistance - aktBottomMargin );
    sal_Int32 height = aktFooterHeight - aktSpacing;
    sal_Int32 newFooterHeight = newSpacing + height;

    mxPageProps->setPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("BottomMargin")), uno::makeAny( newFooterDistance ) );
    mxPageProps->setPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("FooterBodyDistance")), uno::makeAny( newSpacing ) );
    mxPageProps->setPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("FooterHeight")), uno::makeAny( newFooterHeight ) );
}

sal_Bool SAL_CALL SwVbaPageSetup::getDifferentFirstPageHeaderFooter() throw (uno::RuntimeException)
{
    rtl::OUString pageStyle = getStyleOfFirstPage();
    if( pageStyle.equalsAscii( "First Page" ) )
        return sal_True;

    return sal_False;
}

void SAL_CALL SwVbaPageSetup::setDifferentFirstPageHeaderFooter( sal_Bool status ) throw (uno::RuntimeException)
{
    if( status == getDifferentFirstPageHeaderFooter() )
        return;

    rtl::OUString newStyle;
    if( status )
        newStyle = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("First Page") );
    else
        newStyle = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("Standard") );

    uno::Reference< beans::XPropertySet > xStyleProps( word::getCurrentPageStyle( mxModel ), uno::UNO_QUERY_THROW );
    sal_Int32 nTopMargin = 0;
    xStyleProps->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("TopMargin"))) >>= nTopMargin;
    sal_Int32 nBottomMargin = 0;
    xStyleProps->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("BottomMargin"))) >>= nBottomMargin;
    sal_Int32 nLeftMargin = 0;
    xStyleProps->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("LeftMargin"))) >>= nLeftMargin;
    sal_Int32 nRightMargin = 0;
    xStyleProps->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("RightMargin"))) >>= nRightMargin;
    sal_Int32 nHeaderHeight = 0;
    xStyleProps->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("HeaderHeight"))) >>= nHeaderHeight;
    sal_Int32 nFooterHeight = 0;
    xStyleProps->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("FooterHeight"))) >>= nFooterHeight;

    sal_Bool isHeaderOn = sal_False;
    xStyleProps->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("HeaderIsOn"))) >>= isHeaderOn;
    if( isHeaderOn )
    {
        nTopMargin += nHeaderHeight;
        nBottomMargin += nFooterHeight;
        xStyleProps->setPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("HeaderIsOn")), uno::makeAny( sal_False ) );
        xStyleProps->setPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("FooterIsOn")), uno::makeAny( sal_False ) );
    }
    uno::Reference< text::XPageCursor > xPageCursor( word::getXTextViewCursor( mxModel ), uno::UNO_QUERY_THROW );
    if( xPageCursor->getPage() != 1 )
    {
        xPageCursor->jumpToFirstPage();
    }

    uno::Reference< beans::XPropertySet > xCursorProps( xPageCursor, uno::UNO_QUERY_THROW );
    uno::Reference< beans::XPropertySet > xTableProps( xCursorProps->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("TextTable") ) ), uno::UNO_QUERY );
    if( xTableProps.is() )
    {
        xTableProps->setPropertyValue(  rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("PageDescName") ), uno::makeAny( newStyle ) );
    }
    else
    {
        xCursorProps->setPropertyValue(  rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("PageDescName") ), uno::makeAny( newStyle ) );
    }

    uno::Reference< beans::XPropertySet > xFirstPageProps( word::getCurrentPageStyle( mxModel ), uno::UNO_QUERY_THROW );
    xFirstPageProps->setPropertyValue(  rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("TopMargin") ), uno::makeAny( nTopMargin ) );
    xFirstPageProps->setPropertyValue(  rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("BottomMargin") ), uno::makeAny( nBottomMargin ) );
    xFirstPageProps->setPropertyValue(  rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("LeftMargin") ), uno::makeAny( nLeftMargin ) );
    xFirstPageProps->setPropertyValue(  rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("RightMargin") ), uno::makeAny( nRightMargin ) );
}

rtl::OUString SwVbaPageSetup::getStyleOfFirstPage() throw (uno::RuntimeException)
{
    rtl::OUString styleFirstPage;
    uno::Reference< text::XPageCursor > xPageCursor( word::getXTextViewCursor( mxModel ), uno::UNO_QUERY_THROW );
    if( xPageCursor->getPage() != 1 )
    {
        xPageCursor->jumpToFirstPage();
    }

    uno::Reference< beans::XPropertySet > xCursorProps( xPageCursor, uno::UNO_QUERY_THROW );
    uno::Reference< beans::XPropertySet > xTableProps( xCursorProps->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("TextTable") ) ), uno::UNO_QUERY );
    if( xTableProps.is() )
    {
        xTableProps->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("PageDescName") ) ) >>= styleFirstPage;
    }
    else
    {
        xCursorProps->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("PageDescName") ) ) >>= styleFirstPage;
    }
    return styleFirstPage;
}

::sal_Int32 SAL_CALL SwVbaPageSetup::getSectionStart() throw (uno::RuntimeException)
{
    // FIXME:
    sal_Int32 wdSectionStart = word::WdSectionStart::wdSectionNewPage;
    uno::Reference< container::XNamed > xNamed( mxPageProps, uno::UNO_QUERY_THROW );
    rtl::OUString sStyleName = xNamed->getName();
    //mxPageProps->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("Name") ) ) >>= sStyleName;
    if( sStyleName.equalsAscii("Left Page") )
        wdSectionStart = word::WdSectionStart::wdSectionEvenPage;
    else if( sStyleName.equalsAscii("Right Page") )
        wdSectionStart = word::WdSectionStart::wdSectionOddPage;
    else
        wdSectionStart = word::WdSectionStart::wdSectionNewPage;
    return wdSectionStart;
}

void SAL_CALL SwVbaPageSetup::setSectionStart( ::sal_Int32 /*_sectionstart*/ ) throw (uno::RuntimeException)
{
    // fail to find corresponding feature in Writer
    // #FIXME:
}

rtl::OUString&
SwVbaPageSetup::getServiceImplName()
{
    static rtl::OUString sImplName( RTL_CONSTASCII_USTRINGPARAM("SwVbaPageSetup") );
    return sImplName;
}

uno::Sequence< rtl::OUString >
SwVbaPageSetup::getServiceNames()
{
    static uno::Sequence< rtl::OUString > aServiceNames;
    if ( aServiceNames.getLength() == 0 )
    {
        aServiceNames.realloc( 1 );
        aServiceNames[ 0 ] = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("ooo.vba.word.PageSetup" ) );
    }
    return aServiceNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
