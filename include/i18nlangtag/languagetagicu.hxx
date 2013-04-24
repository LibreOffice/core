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
#include <i18nlangtag/i18nlangtagdllapi.h>
#include <unicode/locid.h>

class LanguageTag;


/** Interface LanguageTag to ICU's icu::Locale

    Separated from LanguageTag into its own library to not pollute the entire
    code base with ICU header file inclusion and linkage, only the few code
    actually using this needs to link against ICU libraries, which it did
    anyway.
*/
class I18NLANGTAG_DLLPUBLIC LanguageTagIcu
{
public:

    /** Obtain language tag as ICU icu::Locale.

        If the language tag is a "pure" ISO locale (see
        LanguageTag::getLocale()) that is directly constructed, otherwise it is
        converted using the available ICU mechanisms.

        Always resolves an empty tag to the system locale.
     */
    static  icu::Locale     getIcuLocale( const LanguageTag & rLanguageTag );
};

#endif  // INCLUDED_I18NLANGTAG_LANGUAGETAGICU_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
