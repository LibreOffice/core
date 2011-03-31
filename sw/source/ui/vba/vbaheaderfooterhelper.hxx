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
#ifndef SW_WORD_VBA_HEADERFOOTERHELPER_HXX
#define SW_WORD_VBA_HEADERFOOTERHELPER_HXX

#include <vbahelper/vbahelper.hxx>
#include <com/sun/star/text/XText.hpp>
#include <com/sun/star/text/XTextViewCursor.hpp>
#include <com/sun/star/text/XPageCursor.hpp>
#include <com/sun/star/style/XStyle.hpp>

class HeaderFooterHelper
{
public:
    static sal_Bool isHeaderFooter( const css::uno::Reference< css::frame::XModel >& xModel ) throw (css::uno::RuntimeException);
    static sal_Bool isHeaderFooter( const css::uno::Reference< css::text::XText >& xText ) throw (css::uno::RuntimeException);
    static sal_Bool isHeader( const css::uno::Reference< css::frame::XModel >& xModel ) throw (css::uno::RuntimeException);
    static sal_Bool isFirstPageHeader( const css::uno::Reference< css::frame::XModel >& xModel ) throw (css::uno::RuntimeException);
    static sal_Bool isEvenPagesHeader( const css::uno::Reference< css::frame::XModel >& xModel ) throw (css::uno::RuntimeException);
    static sal_Bool isFooter( const css::uno::Reference< css::frame::XModel >& xModel ) throw (css::uno::RuntimeException);
    static sal_Bool isFirstPageFooter( const css::uno::Reference< css::frame::XModel >& xModel ) throw (css::uno::RuntimeException);
    static sal_Bool isEvenPagesFooter( const css::uno::Reference< css::frame::XModel >& xModel ) throw (css::uno::RuntimeException);
    static sal_Bool isPrimaryHeader( const css::uno::Reference< css::frame::XModel >& xModel ) throw (css::uno::RuntimeException);
    static sal_Bool isPrimaryFooter( const css::uno::Reference< css::frame::XModel >& xModel ) throw (css::uno::RuntimeException);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
