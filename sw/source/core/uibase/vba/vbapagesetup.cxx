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

double SAL_CALL SwVbaPageSetup::getGutter() throw (uno::RuntimeException, std::exception)
{
    // not support in Writer
    return 0;
}

void SAL_CALL SwVbaPageSetup::setGutter( double _gutter ) throw (uno::RuntimeException, std::exception)
{
    // default add gutter into left margin
    if( _gutter != 0 )
    {
        double margin = VbaPageSetupBase::getLeftMargin() + _gutter;
        VbaPageSetupBase::setLeftMargin( margin );
    }
}

double SAL_CALL SwVbaPageSetup::getHeaderDistance() throw (uno::RuntimeException, std::exception)
{
    sal_Bool isHeaderOn = sal_False;
    mxPageProps->getPropertyValue("HeaderIsOn") >>= isHeaderOn;
    if( !isHeaderOn )
        mxPageProps->setPropertyValue("HeaderIsOn", uno::makeAny( sal_True ) );
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
void SAL_CALL SwVbaPageSetup::setHeaderDistance( double _headerdistance ) throw (uno::RuntimeException, std::exception)
{
    sal_Int32 newHeaderDistance = Millimeter::getInHundredthsOfOneMillimeter( _headerdistance );
    sal_Bool isHeaderOn = sal_False;
    sal_Int32 aktTopMargin = 0;
    sal_Int32 aktSpacing = 0;
    sal_Int32 aktHeaderHeight = 0;

    mxPageProps->getPropertyValue("HeaderIsOn") >>= isHeaderOn;
    if( !isHeaderOn )
        mxPageProps->setPropertyValue("HeaderIsOn", uno::makeAny( sal_True ) );

    mxPageProps->getPropertyValue("TopMargin") >>= aktTopMargin;
    mxPageProps->getPropertyValue("HeaderBodyDistance") >>= aktSpacing;
    mxPageProps->getPropertyValue("HeaderHeight") >>= aktHeaderHeight;

    sal_Int32 newSpacing = aktSpacing - ( newHeaderDistance - aktTopMargin );
    sal_Int32 height = aktHeaderHeight - aktSpacing;
    sal_Int32 newHeaderHeight = newSpacing + height;

    mxPageProps->setPropertyValue("TopMargin", uno::makeAny( newHeaderDistance ) );
    mxPageProps->setPropertyValue("HeaderBodyDistance", uno::makeAny( newSpacing ) );
    mxPageProps->setPropertyValue("HeaderHeight", uno::makeAny( newHeaderHeight ) );
}

double SAL_CALL SwVbaPageSetup::getFooterDistance() throw (uno::RuntimeException, std::exception)
{
    sal_Bool isFooterOn = sal_False;
    mxPageProps->getPropertyValue("FooterIsOn") >>= isFooterOn;
    if( !isFooterOn )
        mxPageProps->setPropertyValue("FooterIsOn", uno::makeAny( sal_True ) );
    return VbaPageSetupBase::getFooterMargin();
}

void SAL_CALL SwVbaPageSetup::setFooterDistance( double _footerdistance ) throw (uno::RuntimeException, std::exception)
{
    sal_Int32 newFooterDistance = Millimeter::getInHundredthsOfOneMillimeter( _footerdistance );
    sal_Bool isFooterOn = sal_False;
    sal_Int32 aktBottomMargin = 0;
    sal_Int32 aktSpacing = 0;
    sal_Int32 aktFooterHeight = 0;

    mxPageProps->getPropertyValue("FooterIsOn") >>= isFooterOn;
    if( !isFooterOn )
        mxPageProps->setPropertyValue("FooterIsOn", uno::makeAny( sal_True ) );

    mxPageProps->getPropertyValue("BottomMargin") >>= aktBottomMargin;
    mxPageProps->getPropertyValue("FooterBodyDistance") >>= aktSpacing;
    mxPageProps->getPropertyValue("FooterHeight") >>= aktFooterHeight;

    sal_Int32 newSpacing = aktSpacing - ( newFooterDistance - aktBottomMargin );
    sal_Int32 height = aktFooterHeight - aktSpacing;
    sal_Int32 newFooterHeight = newSpacing + height;

    mxPageProps->setPropertyValue("BottomMargin", uno::makeAny( newFooterDistance ) );
    mxPageProps->setPropertyValue("FooterBodyDistance", uno::makeAny( newSpacing ) );
    mxPageProps->setPropertyValue("FooterHeight", uno::makeAny( newFooterHeight ) );
}

sal_Bool SAL_CALL SwVbaPageSetup::getDifferentFirstPageHeaderFooter() throw (uno::RuntimeException, std::exception)
{
    OUString pageStyle = getStyleOfFirstPage();
    if ( pageStyle == "First Page" )
        return sal_True;

    return sal_False;
}

void SAL_CALL SwVbaPageSetup::setDifferentFirstPageHeaderFooter( sal_Bool status ) throw (uno::RuntimeException, std::exception)
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

    sal_Bool isHeaderOn = sal_False;
    xStyleProps->getPropertyValue("HeaderIsOn") >>= isHeaderOn;
    if( isHeaderOn )
    {
        nTopMargin += nHeaderHeight;
        nBottomMargin += nFooterHeight;
        xStyleProps->setPropertyValue("HeaderIsOn", uno::makeAny( sal_False ) );
        xStyleProps->setPropertyValue("FooterIsOn", uno::makeAny( sal_False ) );
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

OUString SwVbaPageSetup::getStyleOfFirstPage() throw (uno::RuntimeException)
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

::sal_Int32 SAL_CALL SwVbaPageSetup::getSectionStart() throw (uno::RuntimeException, std::exception)
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

void SAL_CALL SwVbaPageSetup::setSectionStart( ::sal_Int32 /*_sectionstart*/ ) throw (uno::RuntimeException, std::exception)
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
