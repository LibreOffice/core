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

#ifdef _WIN32
#include "win/winlayout.hxx"

#elif defined(MACOSX) || defined(IOS)
#include <com/sun/star/i18n/XBreakIterator.hpp>
#include "quartz/ctfonts.hxx"
#include <hb-coretext.h>

#else
#include "unx/freetype_glyphcache.hxx"
#endif

#include <sallayout.hxx>
#include <hb-icu.h>
#include <hb-ot.h>

class CommonSalLayout : public GenericSalLayout
{
    hb_font_t* mpHbFont;
    FontSelectPattern maFontSelData;
    css::uno::Reference<css::i18n::XBreakIterator> mxBreak;
#ifdef _WIN32
    HDC   mhDC;
    HFONT mhFont;
    D2DWriteTextOutRenderer* mpD2DRenderer;
#elif defined(MACOSX) || defined(IOS)
    const CoreTextStyle& mrCoreTextStyle;
#else
    ServerFont& mrServerFont;
#endif

public:
#if defined(_WIN32)
    explicit                CommonSalLayout(WinSalGraphics*, WinFontInstance&, const WinFontFace&);
    virtual void            InitFont() const override;
#elif defined(MACOSX) || defined(IOS)
    explicit                CommonSalLayout(const CoreTextStyle&);
    const CoreTextStyle&    getFontData() const { return mrCoreTextStyle; };
#else
    explicit                CommonSalLayout(ServerFont&);
    const ServerFont&       getFontData() const { return mrServerFont; };
#endif

    void                    SetNeedFallback(ImplLayoutArgs&, sal_Int32, bool);
    void                    AdjustLayout(ImplLayoutArgs&) override;
    virtual bool            LayoutText(ImplLayoutArgs&) override;
    virtual void            DrawText( SalGraphics& ) const override;
    std::shared_ptr<vcl::TextLayoutCache> CreateTextLayoutCache(OUString const&) const override;
};
