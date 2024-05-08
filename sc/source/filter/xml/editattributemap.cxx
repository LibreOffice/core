/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "editattributemap.hxx"

#include <editeng/eeitem.hxx>
#include <editeng/memberids.h>
#include <xmloff/xmlnamespace.hxx>

constexpr ScXMLEditAttributeMap::Entry aEntries[] {

    { XML_NAMESPACE_FO, u"color"_ustr, u"CharColor"_ustr, EE_CHAR_COLOR, 0 },
    { XML_NAMESPACE_STYLE, u"font-charset"_ustr, u"CharFontCharSet"_ustr, EE_CHAR_FONTINFO, MID_FONT_CHAR_SET },
    { XML_NAMESPACE_STYLE, u"font-charset-asian"_ustr, u"CharFontCharSetAsian"_ustr, EE_CHAR_FONTINFO_CJK, MID_FONT_CHAR_SET },
    { XML_NAMESPACE_STYLE, u"font-charset-complex"_ustr, u"CharFontCharSetComplex"_ustr, EE_CHAR_FONTINFO_CTL, MID_FONT_CHAR_SET },
    { XML_NAMESPACE_FO, u"font-family"_ustr, u"CharFontName"_ustr, EE_CHAR_FONTINFO, MID_FONT_FAMILY_NAME },
    { XML_NAMESPACE_STYLE, u"font-family-asian"_ustr, u"CharFontNameAsian"_ustr, EE_CHAR_FONTINFO_CJK, MID_FONT_FAMILY_NAME },
    { XML_NAMESPACE_STYLE, u"font-family-complex"_ustr, u"CharFontNameComplex"_ustr, EE_CHAR_FONTINFO_CTL, MID_FONT_FAMILY_NAME },
    { XML_NAMESPACE_STYLE, u"font-family-generic"_ustr, u"CharFontFamily"_ustr, EE_CHAR_FONTINFO, MID_FONT_FAMILY },
    { XML_NAMESPACE_STYLE, u"font-family-generic-asian"_ustr, u"CharFontFamilyAsian"_ustr, EE_CHAR_FONTINFO_CJK, MID_FONT_FAMILY },
    { XML_NAMESPACE_STYLE, u"font-family-generic-complex"_ustr, u"CharFontFamilyComplex"_ustr, EE_CHAR_FONTINFO_CTL, MID_FONT_FAMILY },
    { XML_NAMESPACE_STYLE, u"font-pitch"_ustr, u"CharFontPitch"_ustr, EE_CHAR_FONTINFO, MID_FONT_PITCH },
    { XML_NAMESPACE_STYLE, u"font-pitch-asian"_ustr, u"CharFontPitchAsian"_ustr, EE_CHAR_FONTINFO_CJK, MID_FONT_PITCH },
    { XML_NAMESPACE_STYLE, u"font-pitch-complex"_ustr, u"CharFontPitchComplex"_ustr, EE_CHAR_FONTINFO_CTL, MID_FONT_PITCH },
    { XML_NAMESPACE_FO, u"font-size"_ustr, u"CharHeight"_ustr, EE_CHAR_FONTHEIGHT, MID_FONTHEIGHT },
    { XML_NAMESPACE_STYLE, u"font-size-asian"_ustr, u"CharHeightAsian"_ustr, EE_CHAR_FONTHEIGHT_CJK, MID_FONTHEIGHT },
    { XML_NAMESPACE_STYLE, u"font-size-complex"_ustr, u"CharHeightComplex"_ustr, EE_CHAR_FONTHEIGHT_CTL, MID_FONTHEIGHT },
    { XML_NAMESPACE_FO, u"font-style"_ustr, u"CharPosture"_ustr, EE_CHAR_ITALIC, MID_POSTURE },
    { XML_NAMESPACE_STYLE, u"font-style-asian"_ustr, u"CharPostureAsian"_ustr, EE_CHAR_ITALIC_CJK, MID_POSTURE },
    { XML_NAMESPACE_STYLE, u"font-style-complex"_ustr, u"CharPostureComplex"_ustr, EE_CHAR_ITALIC_CTL, MID_POSTURE },
    { XML_NAMESPACE_STYLE, u"font-style-name"_ustr, u"CharFontStyleName"_ustr, EE_CHAR_FONTINFO, MID_FONT_STYLE_NAME },
    { XML_NAMESPACE_STYLE, u"font-style-name-asian"_ustr, u"CharFontStyleNameAsian"_ustr, EE_CHAR_FONTINFO_CJK, MID_FONT_STYLE_NAME },
    { XML_NAMESPACE_STYLE, u"font-style-name-complex"_ustr, u"CharFontStyleNameComplex"_ustr, EE_CHAR_FONTINFO_CTL, MID_FONT_STYLE_NAME },
    { XML_NAMESPACE_FO, u"font-weight"_ustr, u"CharWeight"_ustr, EE_CHAR_WEIGHT, MID_WEIGHT },
    { XML_NAMESPACE_STYLE, u"font-weight-asian"_ustr, u"CharWeightAsian"_ustr, EE_CHAR_WEIGHT_CJK, MID_WEIGHT },
    { XML_NAMESPACE_STYLE, u"font-weight-complex"_ustr, u"CharWeightComplex"_ustr, EE_CHAR_WEIGHT_CTL, MID_WEIGHT },
    { XML_NAMESPACE_STYLE, u"text-overline-width"_ustr, u"CharOverline"_ustr, EE_CHAR_OVERLINE, MID_TL_STYLE },
    { XML_NAMESPACE_STYLE, u"text-overline-color"_ustr, u"CharOverlineColor"_ustr, EE_CHAR_OVERLINE, MID_TL_COLOR },
    { XML_NAMESPACE_STYLE, u"text-overline-color"_ustr, u"CharOverlineHasColor"_ustr, EE_CHAR_OVERLINE, MID_TL_HASCOLOR },
    { XML_NAMESPACE_STYLE, u"text-underline-width"_ustr, u"CharUnderline"_ustr, EE_CHAR_UNDERLINE, MID_TL_STYLE },
    { XML_NAMESPACE_STYLE, u"text-underline-color"_ustr, u"CharUnderlineColor"_ustr, EE_CHAR_UNDERLINE, MID_TL_COLOR },
    { XML_NAMESPACE_STYLE, u"text-underline-color"_ustr, u"CharUnderlineHasColor"_ustr, EE_CHAR_UNDERLINE, MID_TL_HASCOLOR },
    { XML_NAMESPACE_STYLE, u"text-line-through-mode"_ustr, u"CharWordMode"_ustr, EE_CHAR_WLM, 0 },
    { XML_NAMESPACE_STYLE, u"text-line-through-type"_ustr, u"CharStrikeout"_ustr, EE_CHAR_STRIKEOUT, MID_CROSS_OUT },
    { XML_NAMESPACE_STYLE, u"font-relief"_ustr, u"CharRelief"_ustr, EE_CHAR_RELIEF, MID_RELIEF },
    { XML_NAMESPACE_STYLE, u"text-outline"_ustr, u"CharContoured"_ustr, EE_CHAR_OUTLINE, 0 },
    { XML_NAMESPACE_FO, u"text-shadow"_ustr, u"CharShadowed"_ustr, EE_CHAR_SHADOW, 0 },
    { XML_NAMESPACE_FO, u"letter-spacing"_ustr, u"CharKerning"_ustr, EE_CHAR_KERNING, 0 },
    { XML_NAMESPACE_STYLE, u"letter-kerning"_ustr, u"CharAutoKerning"_ustr, EE_CHAR_PAIRKERNING, 0 },
    { XML_NAMESPACE_STYLE, u"text-scale"_ustr, u"CharScaleWidth"_ustr, EE_CHAR_FONTWIDTH, 0 },
    { XML_NAMESPACE_STYLE, u"text-position"_ustr, u"CharEscapement"_ustr, EE_CHAR_ESCAPEMENT, MID_ESC },
    { XML_NAMESPACE_STYLE, u"text-position"_ustr, u"CharEscapementHeight"_ustr, EE_CHAR_ESCAPEMENT, MID_ESC_HEIGHT },
    { XML_NAMESPACE_STYLE, u"text-emphasize"_ustr, u"CharEmphasis"_ustr, EE_CHAR_EMPHASISMARK, MID_EMPHASIS },
    // The following 3 "country" entries are just placeholders for language,
    // country, script and rfc-language-tag, which all map to CharLocale,
    // EE_CHAR_LANGUAGE and MID_LANG_LOCALE and are handled individually.
    { XML_NAMESPACE_FO, u"country"_ustr, u"CharLocale"_ustr, EE_CHAR_LANGUAGE, MID_LANG_LOCALE },
    { XML_NAMESPACE_STYLE, u"country-asian"_ustr, u"CharLocaleAsian"_ustr, EE_CHAR_LANGUAGE_CJK, MID_LANG_LOCALE },
    { XML_NAMESPACE_STYLE, u"country-complex"_ustr, u"CharLocaleComplex"_ustr, EE_CHAR_LANGUAGE_CTL, MID_LANG_LOCALE },
};

ScXMLEditAttributeMap::ScXMLEditAttributeMap()
{
    for (size_t i = 0; i < SAL_N_ELEMENTS(aEntries); ++i)
    {
        maAPIEntries.emplace(aEntries[i].maAPIName, &aEntries[i]);
        maItemIDEntries.emplace(aEntries[i].mnItemID, &aEntries[i]);
    }
}

const ScXMLEditAttributeMap::Entry* ScXMLEditAttributeMap::getEntryByAPIName(const OUString& rAPIName) const
{
    StrToEntriesType::const_iterator it = maAPIEntries.find(rAPIName);
    return it == maAPIEntries.end() ? nullptr : it->second;
}

const ScXMLEditAttributeMap::Entry* ScXMLEditAttributeMap::getEntryByItemID(sal_uInt16 nItemID) const
{
    IndexToEntriesType::const_iterator it = maItemIDEntries.find(nItemID);
    return it == maItemIDEntries.end() ? nullptr : it->second;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
