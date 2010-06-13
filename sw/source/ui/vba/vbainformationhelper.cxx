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
#include "vbainformationhelper.hxx"
#include <com/sun/star/text/XPageCursor.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include "wordvbahelper.hxx"
#include <docsh.hxx>
#include <doc.hxx>
#include <vbahelper/vbahelper.hxx>
#include <swtypes.hxx>
#include <viewsh.hxx>

using namespace ::ooo::vba;
using namespace ::com::sun::star;

static const sal_Int32 DEFAULT_PAGE_DISTANCE = 500;

sal_Int32 SwVbaInformationHelper::handleWdActiveEndPageNumber( const css::uno::Reference< css::text::XTextViewCursor >& xTVCursor ) throw( css::uno::RuntimeException )
{
    uno::Reference< text::XPageCursor > xPageCursor( xTVCursor, uno::UNO_QUERY_THROW );
    return xPageCursor->getPage();
}

sal_Int32 SwVbaInformationHelper::handleWdNumberOfPagesInDocument( const css::uno::Reference< css::frame::XModel >& xModel ) throw( css::uno::RuntimeException )
{
    return word::getPageCount( xModel );
}

double SwVbaInformationHelper::handleWdVerticalPositionRelativeToPage( const css::uno::Reference< css::frame::XModel >& xModel, const css::uno::Reference< css::text::XTextViewCursor >& xTVCursor ) throw( css::uno::RuntimeException )
{
    xTVCursor->collapseToStart();
    uno::Reference< beans::XPropertySet > xStyleProps( word::getCurrentPageStyle( xModel ), uno::UNO_QUERY_THROW );
    sal_Int32 nTopMargin = 0;
    xStyleProps->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("TopMargin") ) ) >>= nTopMargin;
    sal_Int32 nCurrentPos = xTVCursor->getPosition().Y;

    sal_Int32 nCurrentPage = handleWdActiveEndPageNumber( xTVCursor );
    SwDoc* pDoc = word::getDocShell( xModel )->GetDoc();
    ViewShell* pViewSh = pDoc->GetCurrentViewShell();
    sal_Int32 nPageHeight = pViewSh ? pViewSh->GetPageSize( nCurrentPage, sal_False ).Height() : 0;
    // FIXME: handle multipul page style
    // it is very strange that the curros position is incorrect when open Word file.
    // e.g. if current cursor in the top left of the text body of the first page without header,
    // the top value of current position should be 0, but is 201 when open a Word file.
    nCurrentPos = nCurrentPos + nTopMargin - ( DEFAULT_PAGE_DISTANCE + TWIP_TO_MM100( nPageHeight ) ) * (  nCurrentPage - 1  );
    return Millimeter::getInPoints( nCurrentPos );
}
