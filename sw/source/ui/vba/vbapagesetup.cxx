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
    mxPageProps->getPropertyValue( OUString("HeaderIsOn")) >>= isHeaderOn;
    if( !isHeaderOn )
        mxPageProps->setPropertyValue( OUString("HeaderIsOn"), uno::makeAny( sal_True ) );
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

    mxPageProps->getPropertyValue( OUString("HeaderIsOn")) >>= isHeaderOn;
    if( !isHeaderOn )
        mxPageProps->setPropertyValue( OUString("HeaderIsOn"), uno::makeAny( sal_True ) );

    mxPageProps->getPropertyValue( OUString("TopMargin")) >>= aktTopMargin;
    mxPageProps->getPropertyValue( OUString("HeaderBodyDistance")) >>= aktSpacing;
    mxPageProps->getPropertyValue( OUString("HeaderHeight")) >>= aktHeaderHeight;

    sal_Int32 newSpacing = aktSpacing - ( newHeaderDistance - aktTopMargin );
    sal_Int32 height = aktHeaderHeight - aktSpacing;
    sal_Int32 newHeaderHeight = newSpacing + height;

    mxPageProps->setPropertyValue( OUString("TopMargin"), uno::makeAny( newHeaderDistance ) );
    mxPageProps->setPropertyValue( OUString("HeaderBodyDistance"), uno::makeAny( newSpacing ) );
    mxPageProps->setPropertyValue( OUString("HeaderHeight"), uno::makeAny( newHeaderHeight ) );
}

double SAL_CALL SwVbaPageSetup::getFooterDistance() throw (uno::RuntimeException)
{
    sal_Bool isFooterOn = sal_False;
    mxPageProps->getPropertyValue( OUString("FooterIsOn")) >>= isFooterOn;
    if( !isFooterOn )
        mxPageProps->setPropertyValue( OUString("FooterIsOn"), uno::makeAny( sal_True ) );
    return VbaPageSetupBase::getFooterMargin();
}

void SAL_CALL SwVbaPageSetup::setFooterDistance( double _footerdistance ) throw (uno::RuntimeException)
{
    sal_Int32 newFooterDistance = Millimeter::getInHundredthsOfOneMillimeter( _footerdistance );
    sal_Bool isFooterOn = sal_False;
    sal_Int32 aktBottomMargin = 0;
    sal_Int32 aktSpacing = 0;
    sal_Int32 aktFooterHeight = 0;

    mxPageProps->getPropertyValue( OUString("FooterIsOn")) >>= isFooterOn;
    if( !isFooterOn )
        mxPageProps->setPropertyValue( OUString("FooterIsOn"), uno::makeAny( sal_True ) );

    mxPageProps->getPropertyValue( OUString("BottomMargin")) >>= aktBottomMargin;
    mxPageProps->getPropertyValue( OUString("FooterBodyDistance")) >>= aktSpacing;
    mxPageProps->getPropertyValue( OUString("FooterHeight")) >>= aktFooterHeight;

    sal_Int32 newSpacing = aktSpacing - ( newFooterDistance - aktBottomMargin );
    sal_Int32 height = aktFooterHeight - aktSpacing;
    sal_Int32 newFooterHeight = newSpacing + height;

    mxPageProps->setPropertyValue( OUString("BottomMargin"), uno::makeAny( newFooterDistance ) );
    mxPageProps->setPropertyValue( OUString("FooterBodyDistance"), uno::makeAny( newSpacing ) );
    mxPageProps->setPropertyValue( OUString("FooterHeight"), uno::makeAny( newFooterHeight ) );
}

sal_Bool SAL_CALL SwVbaPageSetup::getDifferentFirstPageHeaderFooter() throw (uno::RuntimeException)
{
    OUString pageStyle = getStyleOfFirstPage();
    if ( pageStyle == "First Page" )
        return sal_True;

    return sal_False;
}

void SAL_CALL SwVbaPageSetup::setDifferentFirstPageHeaderFooter( sal_Bool status ) throw (uno::RuntimeException)
{
    if( status == getDifferentFirstPageHeaderFooter() )
        return;

    OUString newStyle;
    if( status )
        newStyle = OUString("First Page");
    else
        newStyle = OUString("Standard");

    uno::Reference< beans::XPropertySet > xStyleProps( word::getCurrentPageStyle( mxModel ), uno::UNO_QUERY_THROW );
    sal_Int32 nTopMargin = 0;
    xStyleProps->getPropertyValue( OUString("TopMargin")) >>= nTopMargin;
    sal_Int32 nBottomMargin = 0;
    xStyleProps->getPropertyValue( OUString("BottomMargin")) >>= nBottomMargin;
    sal_Int32 nLeftMargin = 0;
    xStyleProps->getPropertyValue( OUString("LeftMargin")) >>= nLeftMargin;
    sal_Int32 nRightMargin = 0;
    xStyleProps->getPropertyValue( OUString("RightMargin")) >>= nRightMargin;
    sal_Int32 nHeaderHeight = 0;
    xStyleProps->getPropertyValue( OUString("HeaderHeight")) >>= nHeaderHeight;
    sal_Int32 nFooterHeight = 0;
    xStyleProps->getPropertyValue( OUString("FooterHeight")) >>= nFooterHeight;

    sal_Bool isHeaderOn = sal_False;
    xStyleProps->getPropertyValue( OUString("HeaderIsOn")) >>= isHeaderOn;
    if( isHeaderOn )
    {
        nTopMargin += nHeaderHeight;
        nBottomMargin += nFooterHeight;
        xStyleProps->setPropertyValue( OUString("HeaderIsOn"), uno::makeAny( sal_False ) );
        xStyleProps->setPropertyValue( OUString("FooterIsOn"), uno::makeAny( sal_False ) );
    }
    uno::Reference< text::XPageCursor > xPageCursor( word::getXTextViewCursor( mxModel ), uno::UNO_QUERY_THROW );
    if( xPageCursor->getPage() != 1 )
    {
        xPageCursor->jumpToFirstPage();
    }

    uno::Reference< beans::XPropertySet > xCursorProps( xPageCursor, uno::UNO_QUERY_THROW );
    uno::Reference< beans::XPropertySet > xTableProps( xCursorProps->getPropertyValue( OUString("TextTable") ), uno::UNO_QUERY );
    if( xTableProps.is() )
    {
        xTableProps->setPropertyValue(  OUString("PageDescName"), uno::makeAny( newStyle ) );
    }
    else
    {
        xCursorProps->setPropertyValue(  OUString("PageDescName"), uno::makeAny( newStyle ) );
    }

    uno::Reference< beans::XPropertySet > xFirstPageProps( word::getCurrentPageStyle( mxModel ), uno::UNO_QUERY_THROW );
    xFirstPageProps->setPropertyValue(  OUString("TopMargin"), uno::makeAny( nTopMargin ) );
    xFirstPageProps->setPropertyValue(  OUString("BottomMargin"), uno::makeAny( nBottomMargin ) );
    xFirstPageProps->setPropertyValue(  OUString("LeftMargin"), uno::makeAny( nLeftMargin ) );
    xFirstPageProps->setPropertyValue(  OUString("RightMargin"), uno::makeAny( nRightMargin ) );
}

OUString SwVbaPageSetup::getStyleOfFirstPage() throw (uno::RuntimeException)
{
    OUString styleFirstPage;
    uno::Reference< text::XPageCursor > xPageCursor( word::getXTextViewCursor( mxModel ), uno::UNO_QUERY_THROW );
    if( xPageCursor->getPage() != 1 )
    {
        xPageCursor->jumpToFirstPage();
    }

    uno::Reference< beans::XPropertySet > xCursorProps( xPageCursor, uno::UNO_QUERY_THROW );
    uno::Reference< beans::XPropertySet > xTableProps( xCursorProps->getPropertyValue( OUString("TextTable") ), uno::UNO_QUERY );
    if( xTableProps.is() )
    {
        xTableProps->getPropertyValue( OUString("PageDescName") ) >>= styleFirstPage;
    }
    else
    {
        xCursorProps->getPropertyValue( OUString("PageDescName") ) >>= styleFirstPage;
    }
    return styleFirstPage;
}

::sal_Int32 SAL_CALL SwVbaPageSetup::getSectionStart() throw (uno::RuntimeException)
{
    // FIXME:
    sal_Int32 wdSectionStart = word::WdSectionStart::wdSectionNewPage;
    uno::Reference< container::XNamed > xNamed( mxPageProps, uno::UNO_QUERY_THROW );
    OUString sStyleName = xNamed->getName();
    if ( sStyleName == "Left Page" )
        wdSectionStart = word::WdSectionStart::wdSectionEvenPage;
    else if ( sStyleName == "Right Page" )
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

OUString
SwVbaPageSetup::getServiceImplName()
{
    return OUString("SwVbaPageSetup");
}

uno::Sequence< OUString >
SwVbaPageSetup::getServiceNames()
{
    static uno::Sequence< OUString > aServiceNames;
    if ( aServiceNames.getLength() == 0 )
    {
        aServiceNames.realloc( 1 );
        aServiceNames[ 0 ] = OUString("ooo.vba.word.PageSetup" );
    }
    return aServiceNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
