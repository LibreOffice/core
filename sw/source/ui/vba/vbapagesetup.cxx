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
#include <com/sun/star/text/XPageCursor.hpp>
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
    mxModel.set( xModel, uno::UNO_SET_THROW );
    mxPageProps.set( xProps, uno::UNO_SET_THROW );
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
    mxPageProps->getPropertyValue(u"HeaderIsOn"_ustr) >>= isHeaderOn;
    if( !isHeaderOn )
        mxPageProps->setPropertyValue(u"HeaderIsOn"_ustr, uno::Any( true ) );
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

    mxPageProps->getPropertyValue(u"HeaderIsOn"_ustr) >>= isHeaderOn;
    if( !isHeaderOn )
        mxPageProps->setPropertyValue(u"HeaderIsOn"_ustr, uno::Any( true ) );

    mxPageProps->getPropertyValue(u"TopMargin"_ustr) >>= currentTopMargin;
    mxPageProps->getPropertyValue(u"HeaderBodyDistance"_ustr) >>= currentSpacing;
    mxPageProps->getPropertyValue(u"HeaderHeight"_ustr) >>= currentHeaderHeight;

    sal_Int32 newSpacing = currentSpacing - ( newHeaderDistance - currentTopMargin );
    sal_Int32 height = currentHeaderHeight - currentSpacing;
    sal_Int32 newHeaderHeight = newSpacing + height;

    mxPageProps->setPropertyValue(u"TopMargin"_ustr, uno::Any( newHeaderDistance ) );
    mxPageProps->setPropertyValue(u"HeaderBodyDistance"_ustr, uno::Any( newSpacing ) );
    mxPageProps->setPropertyValue(u"HeaderHeight"_ustr, uno::Any( newHeaderHeight ) );
}

double SAL_CALL SwVbaPageSetup::getFooterDistance()
{
    bool isFooterOn = false;
    mxPageProps->getPropertyValue(u"FooterIsOn"_ustr) >>= isFooterOn;
    if( !isFooterOn )
        mxPageProps->setPropertyValue(u"FooterIsOn"_ustr, uno::Any( true ) );
    return VbaPageSetupBase::getFooterMargin();
}

void SAL_CALL SwVbaPageSetup::setFooterDistance( double _footerdistance )
{
    sal_Int32 newFooterDistance = Millimeter::getInHundredthsOfOneMillimeter( _footerdistance );
    bool isFooterOn = false;
    sal_Int32 currentBottomMargin = 0;
    sal_Int32 currentSpacing = 0;
    sal_Int32 currentFooterHeight = 0;

    mxPageProps->getPropertyValue(u"FooterIsOn"_ustr) >>= isFooterOn;
    if( !isFooterOn )
        mxPageProps->setPropertyValue(u"FooterIsOn"_ustr, uno::Any( true ) );

    mxPageProps->getPropertyValue(u"BottomMargin"_ustr) >>= currentBottomMargin;
    mxPageProps->getPropertyValue(u"FooterBodyDistance"_ustr) >>= currentSpacing;
    mxPageProps->getPropertyValue(u"FooterHeight"_ustr) >>= currentFooterHeight;

    sal_Int32 newSpacing = currentSpacing - ( newFooterDistance - currentBottomMargin );
    sal_Int32 height = currentFooterHeight - currentSpacing;
    sal_Int32 newFooterHeight = newSpacing + height;

    mxPageProps->setPropertyValue(u"BottomMargin"_ustr, uno::Any( newFooterDistance ) );
    mxPageProps->setPropertyValue(u"FooterBodyDistance"_ustr, uno::Any( newSpacing ) );
    mxPageProps->setPropertyValue(u"FooterHeight"_ustr, uno::Any( newFooterHeight ) );
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
    xStyleProps->getPropertyValue(u"TopMargin"_ustr) >>= nTopMargin;
    sal_Int32 nBottomMargin = 0;
    xStyleProps->getPropertyValue(u"BottomMargin"_ustr) >>= nBottomMargin;
    sal_Int32 nLeftMargin = 0;
    xStyleProps->getPropertyValue(u"LeftMargin"_ustr) >>= nLeftMargin;
    sal_Int32 nRightMargin = 0;
    xStyleProps->getPropertyValue(u"RightMargin"_ustr) >>= nRightMargin;
    sal_Int32 nHeaderHeight = 0;
    xStyleProps->getPropertyValue(u"HeaderHeight"_ustr) >>= nHeaderHeight;
    sal_Int32 nFooterHeight = 0;
    xStyleProps->getPropertyValue(u"FooterHeight"_ustr) >>= nFooterHeight;

    bool isHeaderOn = false;
    xStyleProps->getPropertyValue(u"HeaderIsOn"_ustr) >>= isHeaderOn;
    if( isHeaderOn )
    {
        nTopMargin += nHeaderHeight;
        nBottomMargin += nFooterHeight;
        xStyleProps->setPropertyValue(u"HeaderIsOn"_ustr, uno::Any( false ) );
        xStyleProps->setPropertyValue(u"FooterIsOn"_ustr, uno::Any( false ) );
    }
    uno::Reference< text::XPageCursor > xPageCursor( word::getXTextViewCursor( mxModel ), uno::UNO_QUERY_THROW );
    if( xPageCursor->getPage() != 1 )
    {
        xPageCursor->jumpToFirstPage();
    }

    uno::Reference< beans::XPropertySet > xCursorProps( xPageCursor, uno::UNO_QUERY_THROW );
    uno::Reference< beans::XPropertySet > xTableProps( xCursorProps->getPropertyValue(u"TextTable"_ustr), uno::UNO_QUERY );
    if( xTableProps.is() )
    {
        xTableProps->setPropertyValue(u"PageDescName"_ustr, uno::Any( newStyle ) );
    }
    else
    {
        xCursorProps->setPropertyValue(u"PageDescName"_ustr, uno::Any( newStyle ) );
    }

    uno::Reference< beans::XPropertySet > xFirstPageProps( word::getCurrentPageStyle( mxModel ), uno::UNO_QUERY_THROW );
    xFirstPageProps->setPropertyValue(u"TopMargin"_ustr, uno::Any( nTopMargin ) );
    xFirstPageProps->setPropertyValue(u"BottomMargin"_ustr, uno::Any( nBottomMargin ) );
    xFirstPageProps->setPropertyValue(u"LeftMargin"_ustr, uno::Any( nLeftMargin ) );
    xFirstPageProps->setPropertyValue(u"RightMargin"_ustr, uno::Any( nRightMargin ) );
}

OUString SwVbaPageSetup::getStyleOfFirstPage() const
{
    OUString styleFirstPage;
    uno::Reference< text::XPageCursor > xPageCursor( word::getXTextViewCursor( mxModel ), uno::UNO_QUERY_THROW );
    if( xPageCursor->getPage() != 1 )
    {
        xPageCursor->jumpToFirstPage();
    }

    uno::Reference< beans::XPropertySet > xCursorProps( xPageCursor, uno::UNO_QUERY_THROW );
    uno::Reference< beans::XPropertySet > xTableProps( xCursorProps->getPropertyValue(u"TextTable"_ustr), uno::UNO_QUERY );
    if( xTableProps.is() )
    {
        xTableProps->getPropertyValue(u"PageDescName"_ustr) >>= styleFirstPage;
    }
    else
    {
        xCursorProps->getPropertyValue(u"PageDescName"_ustr) >>= styleFirstPage;
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
    return u"SwVbaPageSetup"_ustr;
}

uno::Sequence< OUString >
SwVbaPageSetup::getServiceNames()
{
    static uno::Sequence< OUString > const aServiceNames
    {
        u"ooo.vba.word.PageSetup"_ustr
    };
    return aServiceNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
