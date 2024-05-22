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
#include <com/sun/star/text/XTextRangeCompare.hpp>
#include <com/sun/star/text/XTextRange.hpp>
#include <com/sun/star/text/XPageCursor.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>

using namespace ::com::sun::star;
using namespace ::ooo::vba;

#define FIRST_PAGE 1

// Class HeaderFooterHelper
bool HeaderFooterHelper::isHeaderFooter( const uno::Reference< frame::XModel >& xModel )
{
    return isHeaderFooter( word::getCurrentXText( xModel ) );
}

bool HeaderFooterHelper::isHeaderFooter( const uno::Reference< text::XText >& xText )
{
    uno::Reference< lang::XServiceInfo > xServiceInfo( xText, uno::UNO_QUERY_THROW );
    OUString aImplName = xServiceInfo->getImplementationName();
    return aImplName == "SwXHeadFootText";
}

bool HeaderFooterHelper::isHeader( const uno::Reference< frame::XModel >& xModel )
{
    const uno::Reference< text::XText > xCurrentText = word::getCurrentXText( xModel );
    if( !isHeaderFooter( xCurrentText ) )
        return false;

    OUString aPropText = u"HeaderText"_ustr;
    uno::Reference< style::XStyle > xPageStyle = word::getCurrentPageStyle( xModel );
    uno::Reference< beans::XPropertySet > xPageProps( xPageStyle, uno::UNO_QUERY_THROW );
    bool isShared = true;
    xPageProps->getPropertyValue( u"HeaderIsShared"_ustr ) >>= isShared;
    if( !isShared )
    {
        uno::Reference< text::XPageCursor > xPageCursor( word::getXTextViewCursor( xModel ), uno::UNO_QUERY_THROW );
        if( 0 == xPageCursor->getPage() % 2 )
            aPropText = "HeaderTextLeft";
        else
            aPropText = "HeaderTextRight";
    }

    uno::Reference< text::XText > xHeaderText( xPageProps->getPropertyValue( aPropText ), uno::UNO_QUERY_THROW );
    uno::Reference< text::XTextRangeCompare > xTRC( xHeaderText, uno::UNO_QUERY_THROW );
    uno::Reference< text::XTextRange > xTR1( xCurrentText, uno::UNO_QUERY_THROW );
    uno::Reference< text::XTextRange > xTR2( xHeaderText, uno::UNO_QUERY_THROW );
    try
    {
        if( xTRC->compareRegionStarts( xTR1, xTR2 ) == 0 )
            return true;
    }
    catch (const lang::IllegalArgumentException&)
    {
        return false;
    }

    return false;
}

bool HeaderFooterHelper::isFirstPageHeader( const uno::Reference< frame::XModel >& xModel )
{
    if( isHeader( xModel ) )
    {
        uno::Reference< text::XPageCursor > xPageCursor( word::getXTextViewCursor( xModel ), uno::UNO_QUERY_THROW );
        // FIXME: getPage always returns 1
        sal_Int32 nPage = xPageCursor->getPage();
        return nPage == FIRST_PAGE;
    }
    return false;
}

bool HeaderFooterHelper::isEvenPagesHeader( const uno::Reference< frame::XModel >& xModel )
{
    if( isHeader( xModel ) )
    {
        uno::Reference< beans::XPropertySet > xStyleProps( word::getCurrentPageStyle( xModel ), uno::UNO_QUERY_THROW );
        bool isShared = false;
        xStyleProps->getPropertyValue(u"HeaderIsShared"_ustr) >>= isShared;
        if( !isShared )
        {
            uno::Reference< text::XPageCursor > xPageCursor( word::getXTextViewCursor( xModel ), uno::UNO_QUERY_THROW );
            return ( 0 == xPageCursor->getPage() % 2 );
        }
    }
    return false;
}

bool HeaderFooterHelper::isFooter( const uno::Reference< frame::XModel >& xModel )
{
    const uno::Reference< text::XText > xCurrentText = word::getCurrentXText( xModel );
    if( !isHeaderFooter( xCurrentText ) )
        return false;

    OUString aPropText = u"FooterText"_ustr;
    uno::Reference< style::XStyle > xPageStyle = word::getCurrentPageStyle( xModel );
    uno::Reference< beans::XPropertySet > xPageProps( xPageStyle, uno::UNO_QUERY_THROW );
    bool isShared = true;
    xPageProps->getPropertyValue( u"FooterIsShared"_ustr ) >>= isShared;
    if( !isShared )
    {
        uno::Reference< text::XPageCursor > xPageCursor( word::getXTextViewCursor( xModel ), uno::UNO_QUERY_THROW );
        if( 0 == xPageCursor->getPage() % 2 )
            aPropText = "FooterTextLeft";
        else
            aPropText = "FooterTextRight";
    }

    uno::Reference< text::XText > xFooterText( xPageProps->getPropertyValue( aPropText ), uno::UNO_QUERY_THROW );
    uno::Reference< text::XTextRangeCompare > xTRC( xFooterText, uno::UNO_QUERY_THROW );
    uno::Reference< text::XTextRange > xTR1( xCurrentText, uno::UNO_QUERY_THROW );
    uno::Reference< text::XTextRange > xTR2( xFooterText, uno::UNO_QUERY_THROW );
    try
    {
        if( xTRC->compareRegionStarts( xTR1, xTR2 ) == 0 )
            return true;
    }
    catch (const lang::IllegalArgumentException&)
    {
        return false;
    }

    return false;
}

bool HeaderFooterHelper::isFirstPageFooter( const uno::Reference< frame::XModel >& xModel )
{
    if( isFooter( xModel ) )
    {
        uno::Reference< text::XPageCursor > xPageCursor( word::getXTextViewCursor( xModel ), uno::UNO_QUERY_THROW );
        sal_Int32 nPage = xPageCursor->getPage();
        return nPage == FIRST_PAGE;
    }
    return false;
}

bool HeaderFooterHelper::isEvenPagesFooter( const uno::Reference< frame::XModel >& xModel )
{
    if( isFooter( xModel ) )
    {
        uno::Reference< beans::XPropertySet > xStyleProps( word::getCurrentPageStyle( xModel ), uno::UNO_QUERY_THROW );
        bool isShared = false;
        xStyleProps->getPropertyValue(u"FooterIsShared"_ustr) >>= isShared;
        if( !isShared )
        {
            uno::Reference< text::XPageCursor > xPageCursor( word::getXTextViewCursor( xModel ), uno::UNO_QUERY_THROW );
            return ( 0 == xPageCursor->getPage() % 2 );
        }
    }
    return false;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
