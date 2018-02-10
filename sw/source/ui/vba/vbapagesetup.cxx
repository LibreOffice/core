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
#include <com/sun/star/frame/XModel.hpp>
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
                const uno::Reference< beans::XPropertySet >& xProps ):
           SwVbaPageSetup_BASE( xParent, xContext )
{
    mxModel.set( xModel, uno::UNO_QUERY_THROW );
    mxPageProps.set( xProps, uno::UNO_QUERY_THROW );
    mnOrientPortrait = word::WdOrientation::wdOrientPortrait;
    mnOrientLandscape = word::WdOrientation::wdOrientLandscape;
}

double SAL_CALL SwVbaPageSetup::getGutter()
{
    // not support in Writer
    return 0;
}

void SAL_CALL SwVbaPageSetup::setGutter( double _gutter )
{
    // default add gutter into left margin
    if( _gutter != 0 )
    {
        double margin = VbaPageSetupBase::getLeftMargin() + _gutter;
        VbaPageSetupBase::setLeftMargin( margin );
    }
}

double SAL_CALL SwVbaPageSetup::getHeaderDistance()
{
    bool isHeaderOn = false;
    mxPageProps->getPropertyValue("HeaderIsOn") >>= isHeaderOn;
    if( !isHeaderOn )
        mxPageProps->setPropertyValue("HeaderIsOn", uno::makeAny( true ) );
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
void SAL_CALL SwVbaPageSetup::setHeaderDistance( double _headerdistance )
{
    sal_Int32 newHeaderDistance = Millimeter::getInHundredthsOfOneMillimeter( _headerdistance );
    bool isHeaderOn = false;
    sal_Int32 currentTopMargin = 0;
    sal_Int32 currentSpacing = 0;
    sal_Int32 currentHeaderHeight = 0;

    mxPageProps->getPropertyValue("HeaderIsOn") >>= isHeaderOn;
    if( !isHeaderOn )
        mxPageProps->setPropertyValue("HeaderIsOn", uno::makeAny( true ) );

    mxPageProps->getPropertyValue("TopMargin") >>= currentTopMargin;
    mxPageProps->getPropertyValue("HeaderBodyDistance") >>= currentSpacing;
    mxPageProps->getPropertyValue("HeaderHeight") >>= currentHeaderHeight;

    sal_Int32 newSpacing = currentSpacing - ( newHeaderDistance - currentTopMargin );
    sal_Int32 height = currentHeaderHeight - currentSpacing;
    sal_Int32 newHeaderHeight = newSpacing + height;

    mxPageProps->setPropertyValue("TopMargin", uno::makeAny( newHeaderDistance ) );
    mxPageProps->setPropertyValue("HeaderBodyDistance", uno::makeAny( newSpacing ) );
    mxPageProps->setPropertyValue("HeaderHeight", uno::makeAny( newHeaderHeight ) );
}

double SAL_CALL SwVbaPageSetup::getFooterDistance()
{
    bool isFooterOn = false;
    mxPageProps->getPropertyValue("FooterIsOn") >>= isFooterOn;
    if( !isFooterOn )
        mxPageProps->setPropertyValue("FooterIsOn", uno::makeAny( true ) );
    return VbaPageSetupBase::getFooterMargin();
}

void SAL_CALL SwVbaPageSetup::setFooterDistance( double _footerdistance )
{
    sal_Int32 newFooterDistance = Millimeter::getInHundredthsOfOneMillimeter( _footerdistance );
    bool isFooterOn = false;
    sal_Int32 currentBottomMargin = 0;
    sal_Int32 currentSpacing = 0;
    sal_Int32 currentFooterHeight = 0;

    mxPageProps->getPropertyValue("FooterIsOn") >>= isFooterOn;
    if( !isFooterOn )
        mxPageProps->setPropertyValue("FooterIsOn", uno::makeAny( true ) );

    mxPageProps->getPropertyValue("BottomMargin") >>= currentBottomMargin;
    mxPageProps->getPropertyValue("FooterBodyDistance") >>= currentSpacing;
    mxPageProps->getPropertyValue("FooterHeight") >>= currentFooterHeight;

    sal_Int32 newSpacing = currentSpacing - ( newFooterDistance - currentBottomMargin );
    sal_Int32 height = currentFooterHeight - currentSpacing;
    sal_Int32 newFooterHeight = newSpacing + height;

    mxPageProps->setPropertyValue("BottomMargin", uno::makeAny( newFooterDistance ) );
    mxPageProps->setPropertyValue("FooterBodyDistance", uno::makeAny( newSpacing ) );
    mxPageProps->setPropertyValue("FooterHeight", uno::makeAny( newFooterHeight ) );
}

sal_Bool SAL_CALL SwVbaPageSetup::getDifferentFirstPageHeaderFooter()
{
    OUString pageStyle = getStyleOfFirstPage();
    if ( pageStyle == "First Page" )
        return true;

    return false;
}

void SAL_CALL SwVbaPageSetup::setDifferentFirstPageHeaderFooter( sal_Bool status )
{
    if( status == getDifferentFirstPageHeaderFooter() )
        return;

    OUString newStyle;
    if( status )
        newStyle = "First Page";
    else
        newStyle = "Standard";

    uno::Reference< beans::XPropertySet > xStyleProps( word::getCurrentPageStyle( mxModel ), uno::UNO_QUERY_THROW );
    sal_Int32 nTopMargin = 0;
    xStyleProps->getPropertyValue("TopMargin") >>= nTopMargin;
    sal_Int32 nBottomMargin = 0;
    xStyleProps->getPropertyValue("BottomMargin") >>= nBottomMargin;
    sal_Int32 nLeftMargin = 0;
    xStyleProps->getPropertyValue("LeftMargin") >>= nLeftMargin;
    sal_Int32 nRightMargin = 0;
    xStyleProps->getPropertyValue("RightMargin") >>= nRightMargin;
    sal_Int32 nHeaderHeight = 0;
    xStyleProps->getPropertyValue("HeaderHeight") >>= nHeaderHeight;
    sal_Int32 nFooterHeight = 0;
    xStyleProps->getPropertyValue("FooterHeight") >>= nFooterHeight;

    bool isHeaderOn = false;
    xStyleProps->getPropertyValue("HeaderIsOn") >>= isHeaderOn;
    if( isHeaderOn )
    {
        nTopMargin += nHeaderHeight;
        nBottomMargin += nFooterHeight;
        xStyleProps->setPropertyValue("HeaderIsOn", uno::makeAny( false ) );
        xStyleProps->setPropertyValue("FooterIsOn", uno::makeAny( false ) );
    }
    uno::Reference< text::XPageCursor > xPageCursor( word::getXTextViewCursor( mxModel ), uno::UNO_QUERY_THROW );
    if( xPageCursor->getPage() != 1 )
    {
        xPageCursor->jumpToFirstPage();
    }

    uno::Reference< beans::XPropertySet > xCursorProps( xPageCursor, uno::UNO_QUERY_THROW );
    uno::Reference< beans::XPropertySet > xTableProps( xCursorProps->getPropertyValue("TextTable"), uno::UNO_QUERY );
    if( xTableProps.is() )
    {
        xTableProps->setPropertyValue("PageDescName", uno::makeAny( newStyle ) );
    }
    else
    {
        xCursorProps->setPropertyValue("PageDescName", uno::makeAny( newStyle ) );
    }

    uno::Reference< beans::XPropertySet > xFirstPageProps( word::getCurrentPageStyle( mxModel ), uno::UNO_QUERY_THROW );
    xFirstPageProps->setPropertyValue("TopMargin", uno::makeAny( nTopMargin ) );
    xFirstPageProps->setPropertyValue("BottomMargin", uno::makeAny( nBottomMargin ) );
    xFirstPageProps->setPropertyValue("LeftMargin", uno::makeAny( nLeftMargin ) );
    xFirstPageProps->setPropertyValue("RightMargin", uno::makeAny( nRightMargin ) );
}

OUString SwVbaPageSetup::getStyleOfFirstPage()
{
    OUString styleFirstPage;
    uno::Reference< text::XPageCursor > xPageCursor( word::getXTextViewCursor( mxModel ), uno::UNO_QUERY_THROW );
    if( xPageCursor->getPage() != 1 )
    {
        xPageCursor->jumpToFirstPage();
    }

    uno::Reference< beans::XPropertySet > xCursorProps( xPageCursor, uno::UNO_QUERY_THROW );
    uno::Reference< beans::XPropertySet > xTableProps( xCursorProps->getPropertyValue("TextTable"), uno::UNO_QUERY );
    if( xTableProps.is() )
    {
        xTableProps->getPropertyValue("PageDescName") >>= styleFirstPage;
    }
    else
    {
        xCursorProps->getPropertyValue("PageDescName") >>= styleFirstPage;
    }
    return styleFirstPage;
}

::sal_Int32 SAL_CALL SwVbaPageSetup::getSectionStart()
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

void SAL_CALL SwVbaPageSetup::setSectionStart( ::sal_Int32 /*_sectionstart*/ )
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
        aServiceNames[ 0 ] = "ooo.vba.word.PageSetup";
    }
    return aServiceNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
