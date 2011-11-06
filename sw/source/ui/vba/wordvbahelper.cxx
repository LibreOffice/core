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


#include <docsh.hxx>
#include "wordvbahelper.hxx"
#include <comphelper/processfactory.hxx>
#include <com/sun/star/frame/XController.hpp>
#include <com/sun/star/text/XTextViewCursorSupplier.hpp>
#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <unotxdoc.hxx>
#include <doc.hxx>
#include <viewsh.hxx>

using namespace ::com::sun::star;
using namespace ::ooo::vba;

#define FIRST_PAGE 1;

namespace ooo
{
namespace vba
{
namespace word
{

SwDocShell* getDocShell( const uno::Reference< frame::XModel>& xModel )
{
    uno::Reference< lang::XUnoTunnel > xTunnel( xModel, uno::UNO_QUERY_THROW );
    SwXTextDocument* pXDoc = reinterpret_cast< SwXTextDocument * >( sal::static_int_cast< sal_IntPtr >(xTunnel->getSomething(SwXTextDocument::getUnoTunnelId())));
    return pXDoc ? pXDoc->GetDocShell() : 0;
}

SwView* getView( const uno::Reference< frame::XModel>& xModel )
{
    SwDocShell* pDocShell = getDocShell( xModel );
    return pDocShell? pDocShell->GetView() : 0;
}

uno::Reference< text::XTextViewCursor > getXTextViewCursor( const uno::Reference< frame::XModel >& xModel ) throw (uno::RuntimeException)
{
    uno::Reference< frame::XController > xController = xModel->getCurrentController();
    uno::Reference< text::XTextViewCursorSupplier > xTextViewCursorSupp( xController, uno::UNO_QUERY_THROW );
    uno::Reference< text::XTextViewCursor > xTextViewCursor = xTextViewCursorSupp->getViewCursor();
    return xTextViewCursor;
}

uno::Reference< style::XStyle > getCurrentPageStyle( const uno::Reference< frame::XModel >& xModel ) throw (uno::RuntimeException)
{
    uno::Reference< beans::XPropertySet > xCursorProps( getXTextViewCursor( xModel ), uno::UNO_QUERY_THROW );
    rtl::OUString aPageStyleName;
    xCursorProps->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("PageStyleName"))) >>= aPageStyleName;
    uno::Reference< style::XStyleFamiliesSupplier > xSytleFamSupp( xModel, uno::UNO_QUERY_THROW );
    uno::Reference< container::XNameAccess > xSytleFamNames( xSytleFamSupp->getStyleFamilies(), uno::UNO_QUERY_THROW );
    uno::Reference< container::XNameAccess > xPageStyles( xSytleFamNames->getByName( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("PageStyles") ) ), uno::UNO_QUERY_THROW );
    uno::Reference< style::XStyle > xStyle( xPageStyles->getByName( aPageStyleName ), uno::UNO_QUERY_THROW );

    return xStyle;
}

sal_Int32 getPageCount( const uno::Reference< frame::XModel>& xModel ) throw (uno::RuntimeException)
{
    SwDocShell* pDocShell = getDocShell( xModel );
    ViewShell* pViewSh = pDocShell ? pDocShell->GetDoc()->GetCurrentViewShell() : 0;
    return pViewSh ? pViewSh->GetPageCount() : 0;
}

} // word
} //
} //
