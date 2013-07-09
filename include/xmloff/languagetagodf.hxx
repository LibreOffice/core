/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>
#include <rtl/ustring.hxx>
#include <i18nlangtag/languagetag.hxx>

/** Helper to gather the single language tag relevant attributes during reading
    ODF and form a resulting LanguageTag when done.

    For example fo:language, fo:script, fo:country, style:rfc-language-tag
 */
struct LanguageTagODF
{
    OUString    maRfcLanguageTag;
    OUString    maLanguage;
    OUString    maScript;
    OUString    maCountry;

    bool isEmpty() const { return maRfcLanguageTag.isEmpty() &&
        maLanguage.isEmpty() && maScript.isEmpty() && maCountry.isEmpty(); }

    /** Best call this only once per instance, it recreates a LanguageTag
        instance on every call.
    */
    LanguageTag getLanguageTag() const { return LanguageTag( maRfcLanguageTag, maLanguage, maScript, maCountry); }
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
