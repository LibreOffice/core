/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
#ifndef SW_WORD_VBA_HELPER_HXX
#define SW_WORD_VBA_HELPER_HXX

#include <vbahelper/vbahelper.hxx>
#include <com/sun/star/text/XText.hpp>
#include <com/sun/star/text/XTextViewCursor.hpp>
#include <com/sun/star/text/XPageCursor.hpp>
#include <com/sun/star/style/XStyle.hpp>
#include <com/sun/star/text/XTextTable.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>

class SwDocShell;
class SwView;
namespace ooo
{
    namespace vba
    {
    namespace word
    {
        //css::uno::Reference< css::frame::XModel > getCurrentDocument() throw (css::uno::RuntimeException);
        SwDocShell* getDocShell( const css::uno::Reference< css::frame::XModel>& xModel );
        SwView* getView( const css::uno::Reference< css::frame::XModel>& xModel );
        css::uno::Reference< css::text::XTextViewCursor > getXTextViewCursor( const css::uno::Reference< css::frame::XModel >& xModel ) throw (css::uno::RuntimeException);
        css::uno::Reference< css::style::XStyle > getCurrentPageStyle( const css::uno::Reference< css::frame::XModel >& xModel ) throw (css::uno::RuntimeException);
        css::uno::Reference< css::style::XStyle > getCurrentPageStyle( const css::uno::Reference< css::frame::XModel>& xModel, const css::uno::Reference< css::beans::XPropertySet >& xProps ) throw (css::uno::RuntimeException);
        sal_Int32 getPageCount( const css::uno::Reference< css::frame::XModel>& xModel ) throw (css::uno::RuntimeException);
        css::uno::Reference< css::style::XStyle > getDefaultParagraphStyle( const css::uno::Reference< css::frame::XModel >& xModel ) throw (css::uno::RuntimeException);
        css::uno::Reference< css::text::XTextRange > getFirstObjectPosition( const css::uno::Reference< css::text::XText >& xText ) throw (css::uno::RuntimeException);
        css::uno::Reference< css::text::XText > getCurrentXText( const css::uno::Reference< css::frame::XModel>& xModel ) throw (css::uno::RuntimeException);
        sal_Bool gotoSelectedObjectAnchor( const css::uno::Reference< css::frame::XModel>& xModel ) throw (css::uno::RuntimeException);

    enum E_DIRECTION
    {
        MOVE_LEFT = 1,
        MOVE_RIGHT,
        MOVE_UP,
        MOVE_DOWN
    };

} // word
} // vba
} // ooo
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
