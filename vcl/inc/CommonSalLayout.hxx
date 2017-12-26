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

#ifndef INCLUDED_VCL_INC_COMMONSALLAYOUT_HXX
#define INCLUDED_VCL_INC_COMMONSALLAYOUT_HXX

#include <com/sun/star/i18n/XBreakIterator.hpp>

#include "sallayout.hxx"
#include "fontinstance.hxx"

#include <hb-icu.h>

class VCL_DLLPUBLIC CommonSalLayout : public GenericSalLayout
{
    LogicalFontInstance* const mpFont;
    css::uno::Reference<css::i18n::XBreakIterator> mxBreak;

    void                    ParseFeatures(const OUString& name);
    OString                 msLanguage;
    std::vector<hb_feature_t> maFeatures;

    hb_set_t*               mpVertGlyphs;
    const bool              mbFuzzing;
    bool                    HasVerticalAlternate(sal_UCS4 aChar, sal_UCS4 aNextChar);

    void                    SetNeedFallback(ImplLayoutArgs&, sal_Int32, bool);

public:
                            CommonSalLayout(LogicalFontInstance&);
                            ~CommonSalLayout() override;
    LogicalFontInstance&    getFont() const { return *mpFont; }

    virtual void            InitFont() const override;
    void                    AdjustLayout(ImplLayoutArgs&) final override;
    bool                    LayoutText(ImplLayoutArgs&) final override;
    void                    DrawText(SalGraphics&) const final override;
    std::shared_ptr<vcl::TextLayoutCache> CreateTextLayoutCache(OUString const&) const final override;

    bool                    GetCharWidths(DeviceCoordinate* pCharWidths) const final override;
    void                    ApplyDXArray(ImplLayoutArgs&) final override;

    bool                    IsKashidaPosValid(int nCharPos) const final override;
};

#endif // INCLUDED_VCL_INC_COMMONSALLAYOUT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
