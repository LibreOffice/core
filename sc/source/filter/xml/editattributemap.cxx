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
#include "xmloff/xmlnmspe.hxx"

ScXMLEditAttributeMap::Entry aEntries[] = {

    { XML_NAMESPACE_FO, "color", "CharColor", EE_CHAR_COLOR, 0 },
    { XML_NAMESPACE_STYLE, "font-charset", "CharFontCharSet", EE_CHAR_FONTINFO, MID_FONT_CHAR_SET },
    { XML_NAMESPACE_STYLE, "font-charset-asian", "CharFontCharSetAsian", EE_CHAR_FONTINFO_CJK, MID_FONT_CHAR_SET },
    { XML_NAMESPACE_STYLE, "font-charset-complex", "CharFontCharSetComplex", EE_CHAR_FONTINFO_CTL, MID_FONT_CHAR_SET },
    { XML_NAMESPACE_FO, "font-family", "CharFontName", EE_CHAR_FONTINFO, MID_FONT_FAMILY_NAME },
    { XML_NAMESPACE_STYLE, "font-family-asian", "CharFontNameAsian", EE_CHAR_FONTINFO_CJK, MID_FONT_FAMILY_NAME },
    { XML_NAMESPACE_STYLE, "font-family-complex", "CharFontNameAsian", EE_CHAR_FONTINFO_CTL, MID_FONT_FAMILY_NAME },
    { XML_NAMESPACE_STYLE, "font-family-generic", "CharFontFamily", EE_CHAR_FONTINFO, MID_FONT_FAMILY },
    { XML_NAMESPACE_STYLE, "font-family-generic-asian", "CharFontFamilyAsian", EE_CHAR_FONTINFO_CJK, MID_FONT_FAMILY },
    { XML_NAMESPACE_STYLE, "font-family-generic-complex", "CharFontFamilyComplex", EE_CHAR_FONTINFO_CTL, MID_FONT_FAMILY },
    { XML_NAMESPACE_STYLE, "font-pitch", "CharFontPitch", EE_CHAR_FONTINFO, MID_FONT_PITCH },
    { XML_NAMESPACE_STYLE, "font-pitch-asian", "CharFontPitchAsian", EE_CHAR_FONTINFO_CJK, MID_FONT_PITCH },
    { XML_NAMESPACE_STYLE, "font-pitch-complex", "CharFontPitchComplex", EE_CHAR_FONTINFO_CTL, MID_FONT_PITCH },
    { XML_NAMESPACE_FO, "font-size", "CharHeight", EE_CHAR_FONTHEIGHT, MID_FONTHEIGHT },
    { XML_NAMESPACE_STYLE, "font-size-asian", "CharHeightAsian", EE_CHAR_FONTHEIGHT_CJK, MID_FONTHEIGHT },
    { XML_NAMESPACE_STYLE, "font-size-complex", "CharHeightComplex", EE_CHAR_FONTHEIGHT_CTL, MID_FONTHEIGHT },
    { XML_NAMESPACE_FO, "font-style", "CharPosture", EE_CHAR_ITALIC, MID_POSTURE },
    { XML_NAMESPACE_STYLE, "font-style-asian", "CharPostureAsian", EE_CHAR_ITALIC_CJK, MID_POSTURE },
    { XML_NAMESPACE_STYLE, "font-style-complex", "CharPostureComplex", EE_CHAR_ITALIC_CTL, MID_POSTURE },
    { XML_NAMESPACE_STYLE, "font-style-name", "CharFontStyleName", EE_CHAR_FONTINFO, MID_FONT_STYLE_NAME },
    { XML_NAMESPACE_STYLE, "font-style-name-asian", "CharFontStyleNameAsian", EE_CHAR_FONTINFO_CJK, MID_FONT_STYLE_NAME },
    { XML_NAMESPACE_STYLE, "font-style-name-complex", "CharFontStyleNameComplex", EE_CHAR_FONTINFO_CTL, MID_FONT_STYLE_NAME },
    { XML_NAMESPACE_FO, "font-weight", "CharWeight", EE_CHAR_WEIGHT, MID_WEIGHT },
    { XML_NAMESPACE_STYLE, "font-weight-asian", "CharWeightAsian", EE_CHAR_WEIGHT_CJK, MID_WEIGHT },
    { XML_NAMESPACE_STYLE, "font-weight-complex", "CharWeightComplex", EE_CHAR_WEIGHT_CTL, MID_WEIGHT },
    { XML_NAMESPACE_STYLE, "text-overline-width", "CharOverline", EE_CHAR_OVERLINE, MID_TL_STYLE },
    { XML_NAMESPACE_STYLE, "text-overline-color", "CharOverlineColor", EE_CHAR_OVERLINE, MID_TL_COLOR },
    { XML_NAMESPACE_STYLE, "text-overline-color", "CharOverlineHasColor", EE_CHAR_OVERLINE, MID_TL_HASCOLOR },
    { XML_NAMESPACE_STYLE, "text-underline-width", "CharUnderline", EE_CHAR_UNDERLINE, MID_TL_STYLE },
    { XML_NAMESPACE_STYLE, "text-underline-color", "CharUnderlineColor", EE_CHAR_UNDERLINE, MID_TL_COLOR },
    { XML_NAMESPACE_STYLE, "text-underline-color", "CharUnderlineHasColor", EE_CHAR_UNDERLINE, MID_TL_HASCOLOR },
    { XML_NAMESPACE_STYLE, "text-line-through-mode", "CharWordMode", EE_CHAR_WLM, 0 },
    { XML_NAMESPACE_STYLE, "text-line-through-type", "CharStrikeout", EE_CHAR_STRIKEOUT, MID_CROSS_OUT },
    { XML_NAMESPACE_STYLE, "font-relief", "CharRelief", EE_CHAR_RELIEF, MID_RELIEF },
    { XML_NAMESPACE_STYLE, "text-outline", "CharContoured", EE_CHAR_OUTLINE, 0 },
    { XML_NAMESPACE_FO, "text-shadow", "CharShadowed", EE_CHAR_SHADOW, 0 },
    { XML_NAMESPACE_FO, "letter-spacing", "CharKerning", EE_CHAR_KERNING, 0 },
    { XML_NAMESPACE_STYLE, "letter-kerning", "CharAutoKerning", EE_CHAR_PAIRKERNING, 0 },
    { XML_NAMESPACE_STYLE, "text-scale", "CharScaleWidth", EE_CHAR_FONTWIDTH, 0 },
    { XML_NAMESPACE_STYLE, "text-position", "CharEscapement", EE_CHAR_ESCAPEMENT, MID_ESC },
    { XML_NAMESPACE_STYLE, "text-position", "CharEscapementHeight", EE_CHAR_ESCAPEMENT, MID_ESC_HEIGHT },
    { XML_NAMESPACE_STYLE, "text-emphasize", "CharEmphasis", EE_CHAR_EMPHASISMARK, MID_EMPHASIS },
    { XML_NAMESPACE_FO, "country", "CharLocale", EE_CHAR_LANGUAGE, MID_LANG_LOCALE },
    { XML_NAMESPACE_STYLE, "country-asian", "CharLocaleAsian", EE_CHAR_LANGUAGE_CJK, MID_LANG_LOCALE },
    { XML_NAMESPACE_STYLE, "country-complex", "CharLocaleComplex", EE_CHAR_LANGUAGE_CTL, MID_LANG_LOCALE },
};

ScXMLEditAttributeMap::ScXMLEditAttributeMap()
{
    size_t n = sizeof(aEntries) / sizeof(aEntries[0]);
    for (size_t i = 0; i < n; ++i)
    {
        maAPIEntries.insert(
            StrToEntriesType::value_type(
                OUString::createFromAscii(aEntries[i].mpAPIName), &aEntries[i]));

        maItemIDEntries.insert(
            IndexToEntriesType::value_type(aEntries[i].mnItemID, &aEntries[i]));
    }
}

const ScXMLEditAttributeMap::Entry* ScXMLEditAttributeMap::getEntryByAPIName(const OUString& rAPIName) const
{
    StrToEntriesType::const_iterator it = maAPIEntries.find(rAPIName);
    return it == maAPIEntries.end() ? NULL : it->second;
}

const ScXMLEditAttributeMap::Entry* ScXMLEditAttributeMap::getEntryByItemID(sal_uInt16 nItemID) const
{
    IndexToEntriesType::const_iterator it = maItemIDEntries.find(nItemID);
    return it == maItemIDEntries.end() ? NULL : it->second;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
