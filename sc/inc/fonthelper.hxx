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
#include <boost/optional.hpp>
#include <tools/fontenum.hxx>
#include <tools/color.hxx>
#include <vcl/fntstyle.hxx>
#include <i18nlangtag/lang.h>

class SvxFontItem;

struct ScDxfFont
{
    boost::optional<const SvxFontItem*> pFontAttr;
    boost::optional<sal_uInt32> nFontHeight;
    boost::optional<FontWeight> eWeight;
    boost::optional<FontItalic> eItalic;
    boost::optional<FontLineStyle> eUnder;
    boost::optional<FontLineStyle> eOver;
    boost::optional<bool> bWordLine;
    boost::optional<FontStrikeout> eStrike;
    boost::optional<bool> bOutline;
    boost::optional<bool> bShadow;
    boost::optional<FontEmphasisMark> eEmphasis;
    boost::optional<FontRelief> eRelief;
    boost::optional<Color> aColor;
    boost::optional<LanguageType> eLang;

    bool isEmpty()
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
