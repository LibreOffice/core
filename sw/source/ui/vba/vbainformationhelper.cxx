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
#include "vbainformationhelper.hxx"
#include <com/sun/star/text/XPageCursor.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <tools/UnitConversion.hxx>
#include "wordvbahelper.hxx"
#include <docsh.hxx>
#include <doc.hxx>
#include <vbahelper/vbahelper.hxx>
#include <viewsh.hxx>
#include <IDocumentLayoutAccess.hxx>

using namespace ::ooo::vba;
using namespace ::com::sun::star;

const sal_Int32 DEFAULT_PAGE_DISTANCE = 500;

sal_Int32 SwVbaInformationHelper::handleWdActiveEndPageNumber( const css::uno::Reference< css::text::XTextViewCursor >& xTVCursor )
{
    uno::Reference< text::XPageCursor > xPageCursor( xTVCursor, uno::UNO_QUERY_THROW );
    return xPageCursor->getPage();
}

sal_Int32 SwVbaInformationHelper::handleWdNumberOfPagesInDocument( const css::uno::Reference< css::frame::XModel >& xModel )
{
    return word::getPageCount( xModel );
}

double SwVbaInformationHelper::handleWdVerticalPositionRelativeToPage( const css::uno::Reference< css::frame::XModel >& xModel, const css::uno::Reference< css::text::XTextViewCursor >& xTVCursor )
{
    xTVCursor->collapseToStart();
    uno::Reference< beans::XPropertySet > xStyleProps( word::getCurrentPageStyle( xModel ), uno::UNO_QUERY_THROW );
    sal_Int32 nTopMargin = 0;
    xStyleProps->getPropertyValue( u"TopMargin"_ustr ) >>= nTopMargin;
    sal_Int32 nCurrentPos = xTVCursor->getPosition().Y;

    sal_Int32 nCurrentPage = handleWdActiveEndPageNumber( xTVCursor );
    SwDoc* pDoc = word::getDocShell( xModel )->GetDoc();
    SwViewShell* pViewSh = pDoc->getIDocumentLayoutAccess().GetCurrentViewShell();
    sal_Int32 nPageHeight = pViewSh ? pViewSh->GetPageSize( nCurrentPage, false ).Height() : 0;
    // FIXME: handle multiple page style
    // it is very strange that the cursor position is incorrect when open Word file.
    // e.g. if current cursor in the top left of the text body of the first page without header,
    // the top value of current position should be 0, but is 201 when open a Word file.
    nCurrentPos = nCurrentPos + nTopMargin - ( DEFAULT_PAGE_DISTANCE + convertTwipToMm100( nPageHeight ) ) * (  nCurrentPage - 1  );
    return Millimeter::getInPoints( nCurrentPos );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
