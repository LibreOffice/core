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

#ifndef INCLUDED_SDEXT_SOURCE_PDFIMPORT_INC_WRAPPER_HXX
#define INCLUDED_SDEXT_SOURCE_PDFIMPORT_INC_WRAPPER_HXX

#include "contentsink.hxx"
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/task/XInteractionHandler.hpp>

namespace com::sun::star {
    namespace uno {
        class XComponentContext;
    }
    namespace io {
        class XInputStream;
    }
}

namespace pdfi
{
    bool xpdf_ImportFromFile( const OUString&                               rURL,
                              const ContentSinkSharedPtr&                        rSink,
                              const css::uno::Reference<
                                    css::task::XInteractionHandler >& xIHdl,
                              const OUString&                               rPwd,
                              const css::uno::Reference<
                                    css::uno::XComponentContext >&    xContext,
                              const OUString&        rFilterOptions);
    bool xpdf_ImportFromStream( const css::uno::Reference<
                                      css::io::XInputStream >&          xInput,
                                const ContentSinkSharedPtr&                        rSink,
                                const css::uno::Reference<
                                      css::task::XInteractionHandler >& xIHdl,
                                const OUString&                               rPwd,
                                const css::uno::Reference<
                                      css::uno::XComponentContext >&    xContext,
                                const OUString&        rFilterOptions );

    constexpr OUString fontAttributesSuffixes[] = {
        // Note: for performance consideration, each one listed here is evaluated once,
        // and they are checked from the suffix, thus the order matters.
        // e.g. for "TimesNewRomanPS-BoldItalic", to get "TimesNewRoman", you should
        //      first have "Italic", and then "Bold", then "-", and then "PS".
        u"-VKana"_ustr,
        u"MT"_ustr,
        u"PS"_ustr,
        u"PSMT"_ustr,
        u"Regular"_ustr,
        u"Normal"_ustr,
        u"Book"_ustr,
        u"Medium"_ustr,
        u"ExtraBold"_ustr,
        u"UltraBold"_ustr,
        u"ExtraLight"_ustr,
        u"UltraLight"_ustr,
        u"Bold"_ustr,
        u"Heavy"_ustr,
        u"Black"_ustr,
        u"Italic"_ustr,
        u"Oblique"_ustr,
        u"Bold"_ustr, //BoldItalic, BoldOblique
        u"Light"_ustr,
        u"Thin"_ustr,
        u"Semibold"_ustr,
        u"-Roman"_ustr,
        u"Reg"_ustr,
        u"VKana"_ustr,
        u"-"_ustr,
        u","_ustr,
        u";"_ustr,
        u"PS"_ustr, // e.g. TimesNewRomanPS-BoldMT
    };
}

#endif // INCLUDED_SDEXT_SOURCE_PDFIMPORT_INC_WRAPPER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
