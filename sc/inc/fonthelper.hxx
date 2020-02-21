/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "scdllapi.h"
#include <optional>
#include <tools/fontenum.hxx>
#include <tools/color.hxx>
#include <vcl/fntstyle.hxx>
#include <i18nlangtag/lang.h>

class SvxFontItem;

struct SC_DLLPUBLIC ScDxfFont
{
    std::optional<const SvxFontItem*> pFontAttr;
    std::optional<sal_uInt32> nFontHeight;
    std::optional<FontWeight> eWeight;
    std::optional<FontItalic> eItalic;
    std::optional<FontLineStyle> eUnder;
    std::optional<FontLineStyle> eOver;
    std::optional<bool> bWordLine;
    std::optional<FontStrikeout> eStrike;
    std::optional<bool> bOutline;
    std::optional<bool> bShadow;
    std::optional<FontEmphasisMark> eEmphasis;
    std::optional<FontRelief> eRelief;
    std::optional<Color> aColor;
    std::optional<LanguageType> eLang;

    bool isEmpty() const
    {
        return !(pFontAttr || nFontHeight ||
            eWeight || eItalic || eUnder ||
            eOver || bWordLine || eStrike ||
            bOutline || bShadow || eEmphasis ||
            eRelief || aColor || eLang);
    }
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
