/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "i18nlangtag/languagetagicu.hxx"
#include "i18nlangtag/languagetag.hxx"


// static
icu::Locale LanguageTagIcu::getIcuLocale( const LanguageTag & rLanguageTag )
{
    if (rLanguageTag.isIsoLocale())
    {
        // The simple case.
        const css::lang::Locale& rLocale = rLanguageTag.getLocale();
        if (rLocale.Country.isEmpty())
            return icu::Locale( OUStringToOString( rLocale.Language, RTL_TEXTENCODING_ASCII_US).getStr());
        return icu::Locale(
                OUStringToOString( rLocale.Language, RTL_TEXTENCODING_ASCII_US).getStr(),
                OUStringToOString( rLocale.Country, RTL_TEXTENCODING_ASCII_US).getStr());
    }

    /* TODO: could we optimize this for the isIsoODF() case where only a script
     * is added? */

    // Let ICU decide how it wants a BCP47 string stuffed into its Locale.
    return icu::Locale::createFromName(
            OUStringToOString( rLanguageTag.getBcp47(), RTL_TEXTENCODING_ASCII_US).getStr());
}


// static
icu::Locale LanguageTagIcu::getIcuLocale( const LanguageTag & rLanguageTag, const OUString & rVariant )
{
    /* FIXME: how should this work with any BCP47? */
    return icu::Locale(
            OUStringToOString( rLanguageTag.getLanguage(), RTL_TEXTENCODING_ASCII_US).getStr(),
            OUStringToOString( rLanguageTag.getCountry(), RTL_TEXTENCODING_ASCII_US).getStr(),
            OUStringToOString( rVariant, RTL_TEXTENCODING_ASCII_US).getStr());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
