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
        bool gotoSelectedObjectAnchor( const css::uno::Reference< css::frame::XModel>& xModel ) throw (css::uno::RuntimeException);

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
