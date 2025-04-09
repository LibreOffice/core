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
#ifndef INCLUDED_SW_SOURCE_UI_VBA_WORDVBAHELPER_HXX
#define INCLUDED_SW_SOURCE_UI_VBA_WORDVBAHELPER_HXX

#include <vbahelper/vbahelper.hxx>
#include <com/sun/star/text/XText.hpp>
#include <com/sun/star/text/XTextViewCursor.hpp>
#include <com/sun/star/style/XStyle.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <rtl/ref.hxx>

class SwView;
class SwXTextDocument;
class SwXBaseStyle;

namespace ooo::vba::word
    {
        SwView* getView( const rtl::Reference<SwXTextDocument>& xModel );
        /// @throws css::uno::RuntimeException
        css::uno::Reference< css::text::XTextViewCursor > getXTextViewCursor( const rtl::Reference< SwXTextDocument >& xModel );
        /// @throws css::uno::RuntimeException
        rtl::Reference< SwXBaseStyle > getCurrentPageStyle( const rtl::Reference< SwXTextDocument >& xModel );
        /// @throws css::uno::RuntimeException
        rtl::Reference< SwXBaseStyle > getCurrentPageStyle( const rtl::Reference< SwXTextDocument>& xModel, const css::uno::Reference< css::beans::XPropertySet >& xProps );
        /// @throws css::uno::RuntimeException
        sal_Int32 getPageCount( const rtl::Reference<SwXTextDocument>& xModel );
        /// @throws css::uno::RuntimeException
        rtl::Reference< SwXBaseStyle > getDefaultParagraphStyle( const rtl::Reference< SwXTextDocument >& xModel );
        /// @throws css::uno::RuntimeException
        css::uno::Reference< css::text::XTextRange > getFirstObjectPosition( const css::uno::Reference< css::text::XText >& xText );
        /// @throws css::uno::RuntimeException
        css::uno::Reference< css::text::XText > getCurrentXText( const rtl::Reference<SwXTextDocument>& xModel );
        /// @throws css::uno::RuntimeException
        bool gotoSelectedObjectAnchor( const rtl::Reference<SwXTextDocument>& xModel );
        /// @throws css::uno::RuntimeException
        rtl::Reference< SwXTextDocument > getCurrentWordDoc( const css::uno::Reference< css::uno::XComponentContext >& xContext );
        /// @throws css::uno::RuntimeException
        rtl::Reference< SwXTextDocument > getThisWordDoc( const css::uno::Reference< css::uno::XComponentContext >& xContext );

    enum E_DIRECTION
    {
        MOVE_LEFT = 1,
        MOVE_RIGHT,
        MOVE_UP,
        MOVE_DOWN
    };

} // ooo::vba::word
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
