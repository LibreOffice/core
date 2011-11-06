/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


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
