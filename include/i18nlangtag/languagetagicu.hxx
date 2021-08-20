/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_I18NLANGTAG_LANGUAGETAGICU_HXX
#define INCLUDED_I18NLANGTAG_LANGUAGETAGICU_HXX

#include <sal/config.h>

#include <string_view>

#include <i18nlangtag/i18nlangtagdllapi.h>
#include <rtl/ustring.hxx>
#include <unicode/locid.h>

class LanguageTag;


/** Interface LanguageTag to ICU's icu::Locale

    Separated from LanguageTag to not pollute the entire code base (and thus
    makefiles) with ICU header file inclusion, only the few code actually using
    this needs to know about ICU, which it did anyway.
*/
class SAL_WARN_UNUSED I18NLANGTAG_DLLPUBLIC LanguageTagIcu
{
public:

    /** Obtain language tag as ICU icu::Locale.

        If the language tag is a "pure" ISO locale (see
        LanguageTag::getLocale()) that is directly constructed, otherwise it is
        converted using the available ICU mechanisms.

        Always resolves an empty tag to the system locale.
     */
    static  icu::Locale     getIcuLocale( const LanguageTag & rLanguageTag );

    /** Obtain language tag as ICU icu::Locale, adding variant data.

        From the LanguageTag only language and country are used to construct
        the icu:Locale, the variant field is copied from rVariant.
        The 4th arg of icu::Locale "keywords" (eg: for collation)

        Always resolves an empty tag to the system locale.
     */
    static  icu::Locale     getIcuLocale( const LanguageTag & rLanguageTag, std::u16string_view rVariant, std::u16string_view rKeywords);

    /** Obtain the display name for a language tag.

        Gets a localized name from icu::Locale::getDisplayName(). Meant to be
        used only with language tags that do not have a language list entry in
        svtools/inc/langtab.hrc STR_ARR_SVT_LANGUAGE_TABLE, thus used by
        SvtLanguageTable::GetLanguageString()

        @param  rLanguageTag
                The language tag for which the name is to be displayed.

        @param  rDisplayLanguage
                The language in which the name is to be displayed.
                Usually Application::GetSettings().GetUILanguageTag().
     */
    static  OUString        getDisplayName( const LanguageTag & rLanguageTag, const LanguageTag & rDisplayLanguage );
};

#endif  // INCLUDED_I18NLANGTAG_LANGUAGETAGICU_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
