/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SC_INC_FONTHELPER_HXX
#define INCLUDED_SC_INC_FONTHELPER_HXX

#include "scdllapi.h"
#include <o3tl/optional.hxx>
#include <tools/fontenum.hxx>
#include <tools/color.hxx>
#include <vcl/fntstyle.hxx>
#include <i18nlangtag/lang.h>

class SvxFontItem;

struct SC_DLLPUBLIC ScDxfFont
{
    o3tl::optional<const SvxFontItem*> pFontAttr;
    o3tl::optional<sal_uInt32> nFontHeight;
    o3tl::optional<FontWeight> eWeight;
    o3tl::optional<FontItalic> eItalic;
    o3tl::optional<FontLineStyle> eUnder;
    o3tl::optional<FontLineStyle> eOver;
    o3tl::optional<bool> bWordLine;
    o3tl::optional<FontStrikeout> eStrike;
    o3tl::optional<bool> bOutline;
    o3tl::optional<bool> bShadow;
    o3tl::optional<FontEmphasisMark> eEmphasis;
    o3tl::optional<FontRelief> eRelief;
    o3tl::optional<Color> aColor;
    o3tl::optional<LanguageType> eLang;

    bool isEmpty() const
    {
        return !(pFontAttr || nFontHeight ||
            eWeight || eItalic || eUnder ||
            eOver || bWordLine || eStrike ||
            bOutline || bShadow || eEmphasis ||
            eRelief || aColor || eLang);
    }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
