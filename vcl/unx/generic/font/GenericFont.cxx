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

#include <sal/config.h>

#include <unx/font/GenericFont.hxx>
#include <unx/font/GenericFontFace.hxx>
#include <unx/font/GenericFontList.hxx>
#include <unx/font/fontmanager.hxx>

#include <font/LogicalFontInstance.hxx>
#include <fontattributes.hxx>

#include <hb.h>

// GenericFont

GenericFont::GenericFont(const GenericFontFace& rFontFace, const vcl::font::FontSelectPattern& rFSD)
    : LogicalFontInstance(rFontFace, rFSD)
    , mnPrioAntiAlias(GetDefaultAntiAliasPrio())
{
}

bool GenericFont::TestFont() const
{
    return hb_face_get_glyph_count(GetFontFace()->GetHbFace()) > 0;
}

namespace
{
    std::unique_ptr<FontConfigFontOptions> GetFCFontOptions(const FontAttributes& rFontAttributes, int nSize)
    {
        return FontConfigManager::getFontOptions(rFontAttributes, nSize);
    }
}

const FontConfigFontOptions* GenericFont::GetFontOptions() const
{
    if (!mxFontOptions)
    {
        const GenericFontFace* pFontFace = GetFontFace();
        mxFontOptions = GetFCFontOptions(*pFontFace, GetFontSelectPattern().mnHeight);
        mxFontOptions->SyncPattern(pFontFace->GetFontFileName(), pFontFace->GetFontFaceIndex(),
                                   pFontFace->GetFontFaceVariation(), NeedsArtificialBold(),
                                   GetVariations());
    }
    return mxFontOptions.get();
}

bool GenericFont::GetAntialiasAdvice() const
{
    // TODO: also use GASP info
    return !GetFontSelectPattern().mbNonAntialiased && (mnPrioAntiAlias > 0);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
