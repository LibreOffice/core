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
