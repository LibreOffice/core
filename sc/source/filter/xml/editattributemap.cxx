/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "editattributemap.hxx"

#include "editeng/eeitem.hxx"
#include "editeng/memberids.hrc"

namespace {

struct {
    const char* mpXMLName;
    const char* mpAPIName;
    sal_uInt16 mnItemID;
    sal_uInt8 mnFlag;

} aEntries[] = {

    { "color", "CharColor", EE_CHAR_COLOR, 0 },
    { "font-charset", "CharFontCharSet", EE_CHAR_FONTINFO, MID_FONT_CHAR_SET },
    { "font-charset-asian", "CharFontCharSetAsian", EE_CHAR_FONTINFO_CJK, MID_FONT_CHAR_SET },
    { "font-charset-complex", "CharFontCharSetComplex", EE_CHAR_FONTINFO_CTL, MID_FONT_CHAR_SET },
    { "font-family", "CharFontName", EE_CHAR_FONTINFO, MID_FONT_FAMILY_NAME },
    { "font-family-asian", "CharFontNameAsian", EE_CHAR_FONTINFO_CJK, MID_FONT_FAMILY_NAME },
    { "font-family-complex", "CharFontNameAsian", EE_CHAR_FONTINFO_CTL, MID_FONT_FAMILY_NAME },
    { "font-family-generic", "CharFontFamily", EE_CHAR_FONTINFO, MID_FONT_FAMILY },
    { "font-family-generic-asian", "CharFontFamilyAsian", EE_CHAR_FONTINFO_CJK, MID_FONT_FAMILY },
    { "font-family-generic-complex", "CharFontFamilyComplex", EE_CHAR_FONTINFO_CTL, MID_FONT_FAMILY },
    { "font-pitch", "CharFontPitch", EE_CHAR_FONTINFO, MID_FONT_PITCH },
    { "font-pitch-asian", "CharFontPitchAsian", EE_CHAR_FONTINFO_CJK, MID_FONT_PITCH },
    { "font-pitch-complex", "CharFontPitchComplex", EE_CHAR_FONTINFO_CTL, MID_FONT_PITCH },
    { "font-size", "CharHeight", EE_CHAR_FONTHEIGHT, MID_FONTHEIGHT },
    { "font-size-asian", "CharHeightAsian", EE_CHAR_FONTHEIGHT_CJK, MID_FONTHEIGHT },
    { "font-size-complex", "CharHeightComplex", EE_CHAR_FONTHEIGHT_CTL, MID_FONTHEIGHT },
    { "font-style", "CharPosture", EE_CHAR_ITALIC, MID_POSTURE },
    { "font-style-asian", "CharPostureAsian", EE_CHAR_ITALIC_CJK, MID_POSTURE },
    { "font-style-complex", "CharPostureComplex", EE_CHAR_ITALIC_CTL, MID_POSTURE },
    { "font-style-name", "CharFontStyleName", EE_CHAR_FONTINFO, MID_FONT_STYLE_NAME },
    { "font-style-name-asian", "CharFontStyleNameAsian", EE_CHAR_FONTINFO_CJK, MID_FONT_STYLE_NAME },
    { "font-style-name-complex", "CharFontStyleNameComplex", EE_CHAR_FONTINFO_CTL, MID_FONT_STYLE_NAME },
    { "font-weight", "CharWeight", EE_CHAR_WEIGHT, MID_WEIGHT },
    { "font-weight-asian", "CharWeightAsian", EE_CHAR_WEIGHT_CJK, MID_WEIGHT },
    { "font-weight-complex", "CharWeightComplex", EE_CHAR_WEIGHT_CTL, MID_WEIGHT },
    { "text-overline-width", "CharOverline", EE_CHAR_OVERLINE, MID_TL_STYLE },
    { "text-overline-color", "CharOverlineColor", EE_CHAR_OVERLINE, MID_TL_COLOR },
    { "text-overline-color", "CharOverlineHasColor", EE_CHAR_OVERLINE, MID_TL_HASCOLOR },
    { "text-underline-width", "CharUnderline", EE_CHAR_UNDERLINE, MID_TL_STYLE },
    { "text-underline-color", "CharUnderlineColor", EE_CHAR_UNDERLINE, MID_TL_COLOR },
    { "text-underline-color", "CharUnderlineHasColor", EE_CHAR_UNDERLINE, MID_TL_HASCOLOR },
    { "text-line-through-mode", "CharWordMode", EE_CHAR_WLM, 0 },
    { "text-line-through-type", "CharStrikeout", EE_CHAR_STRIKEOUT, MID_CROSS_OUT },
    { "font-relief", "CharRelief", EE_CHAR_RELIEF, MID_RELIEF },
    { "text-outline", "CharContoured", EE_CHAR_OUTLINE, 0 },
    { "text-shadow", "CharShadowed", EE_CHAR_SHADOW, 0 },
    { "letter-spacing", "CharKerning", EE_CHAR_KERNING, 0 },
    { "letter-kerning", "CharAutoKerning", EE_CHAR_PAIRKERNING, 0 },
    { "text-scale", "CharScaleWidth", EE_CHAR_FONTWIDTH, 0 },
    { "text-position", "CharEscapement", EE_CHAR_ESCAPEMENT, MID_ESC },
    { "text-position", "CharEscapementHeight", EE_CHAR_ESCAPEMENT, MID_ESC_HEIGHT },
    { "text-emphasize", "CharEmphasis", EE_CHAR_EMPHASISMARK, MID_EMPHASIS },
    { "country", "CharLocale", EE_CHAR_LANGUAGE, MID_LANG_LOCALE },
    { "country-asian", "CharLocaleAsian", EE_CHAR_LANGUAGE_CJK, MID_LANG_LOCALE },
    { "country-complex", "CharLocaleComplex", EE_CHAR_LANGUAGE_CTL, MID_LANG_LOCALE },
};

}

ScXMLEditAttributeMap::Entry::Entry(sal_uInt16 nItemID, sal_uInt8 nFlag) :
    mnItemID(nItemID), mnFlag(nFlag) {}

ScXMLEditAttributeMap::ScXMLEditAttributeMap()
{
    size_t n = sizeof(aEntries) / sizeof(aEntries[0]);
    for (size_t i = 0; i < n; ++i)
    {
        maEntries.insert(
            EntriesType::value_type(
                OUString::createFromAscii(aEntries[i].mpAPIName),
                Entry(aEntries[i].mnItemID, aEntries[i].mnFlag)));
    }
}

const ScXMLEditAttributeMap::Entry* ScXMLEditAttributeMap::getEntry(const OUString& rXMLName) const
{
    EntriesType::const_iterator it = maEntries.find(rXMLName);
    return it == maEntries.end() ? NULL : &it->second;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
