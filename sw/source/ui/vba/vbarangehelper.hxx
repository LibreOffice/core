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
#ifndef SW_VBA_RANGEHELPER_HXX
#define SW_VBA_RANGEHELPER_HXX

#include <vbahelper/vbahelperinterface.hxx>
#include <com/sun/star/text/XTextViewCursor.hpp>
#include <com/sun/star/text/XText.hpp>
#include <com/sun/star/text/XTextRange.hpp>
#include <com/sun/star/text/XTextDocument.hpp>

class SwVbaRangeHelper
{
public:
    static css::uno::Reference< css::text::XTextRange > getRangeByPosition( const css::uno::Reference< css::text::XText >& rText, sal_Int32 _position )throw ( css::uno::RuntimeException );
    static void insertString( css::uno::Reference< css::text::XTextRange >& rTextRange, css::uno::Reference< css::text::XText >& rText, const OUString& rStr, sal_Bool _bAbsorb ) throw ( css::uno::RuntimeException );
    static css::uno::Reference< css::text::XTextCursor > initCursor( const css::uno::Reference< css::text::XTextRange >& rTextRange, const css::uno::Reference< css::text::XText >& rText ) throw ( css::uno::RuntimeException );
    static sal_Int32 getPosition( const css::uno::Reference< css::text::XText >& rText, const css::uno::Reference< css::text::XTextRange >& rTextRange ) throw ( css::uno::RuntimeException );
    static css::uno::Reference< css::text::XTextContent > findBookmarkByPosition( const css::uno::Reference< css::text::XTextDocument >& xTextDoc, const css::uno::Reference< css::text::XTextRange >& xTextRange ) throw ( css::uno::RuntimeException );

};
#endif /* SW_VBA_RANGEHELPER_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
