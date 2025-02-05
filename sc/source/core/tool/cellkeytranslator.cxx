/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <sal/config.h>

#include <vector>
#include <unordered_map>

#include <global.hxx>
#include <cellkeytranslator.hxx>
#include <comphelper/processfactory.hxx>
#include <i18nlangtag/lang.h>
#include <i18nutil/transliteration.hxx>
#include <unotools/syslocale.hxx>
#include <unotools/transliterationwrapper.hxx>

using namespace ::com::sun::star;

namespace {

enum LocaleMatch
{
    LOCALE_MATCH_NONE = 0,
    LOCALE_MATCH_LANG,
    LOCALE_MATCH_LANG_SCRIPT,
    LOCALE_MATCH_LANG_SCRIPT_COUNTRY,
    LOCALE_MATCH_ALL
};

LocaleMatch lclLocaleCompare(const lang::Locale& rLocale1, const LanguageTag& rLanguageTag2)
{
    LanguageTag aLanguageTag1( rLocale1);

    if (aLanguageTag1.getLanguage() != rLanguageTag2.getLanguage())
        return LOCALE_MATCH_NONE;

    if (aLanguageTag1.getScript() != rLanguageTag2.getScript())
        return LOCALE_MATCH_LANG;

    if (aLanguageTag1.getCountry() != rLanguageTag2.getCountry())
        return LOCALE_MATCH_LANG_SCRIPT;

    if (aLanguageTag1 != rLanguageTag2)
        return LOCALE_MATCH_LANG_SCRIPT_COUNTRY;

    return LOCALE_MATCH_ALL;
}

struct ScCellKeyword
{
    OUString msName;
    OpCode meOpCode;
    const css::lang::Locale& mrLocale;

    ScCellKeyword(const OUString& sName, OpCode eOpCode, const css::lang::Locale& rLocale)
        : msName(sName)
        , meOpCode(eOpCode)
        , mrLocale(rLocale)
    {
    }
};

typedef std::unordered_map<OUString, std::vector<ScCellKeyword>> ScCellKeywordHashMap;

void lclMatchKeyword(OUString& rName, const ScCellKeywordHashMap& aMap, OpCode eOpCode,
                     const lang::Locale& rLocale)
{
    assert(eOpCode != ocNone);
    ScCellKeywordHashMap::const_iterator itr = aMap.find(rName);

    if (itr == aMap.end() || itr->second.empty())
        // No candidate strings exist.  Bail out.
        return;

    LanguageTag aLanguageTag(rLocale);
    const OUString* aBestMatchName = nullptr;
    LocaleMatch eLocaleMatchLevel = LOCALE_MATCH_NONE;

    for (auto const& elem : itr->second)
    {
        if (elem.meOpCode == eOpCode)
        {
            LocaleMatch eLevel = lclLocaleCompare(elem.mrLocale, aLanguageTag);
            if ( eLevel == LOCALE_MATCH_ALL )
            {
                // Name with matching opcode and locale found.
                rName = elem.msName;
                return;
            }
            else if ( eLevel > eLocaleMatchLevel )
            {
                // Name with a better matching locale.
                eLocaleMatchLevel = eLevel;
                aBestMatchName = &elem.msName;
            }
        }
    }

    // No preferred strings found.  Return the best matching name.
    if (aBestMatchName)
        rName = *aBestMatchName;
}

ScCellKeywordHashMap MakeMap()
{
    ScCellKeywordHashMap map;

    // All keywords must be uppercase, and the mapping must be from the
    // localized keyword to the English keyword.
    struct TransItem
    {
        OUString from;
        OUString to;
        OpCode func;
    };

    // French language locale

    static const lang::Locale aFr(u"fr"_ustr, {}, {});

    static constexpr TransItem pFr[] = {
        { u"ADRESSE"_ustr, u"ADDRESS"_ustr, ocCell },
        { u"COLONNE"_ustr, u"COL"_ustr, ocCell },
        { u"CONTENU"_ustr, u"CONTENTS"_ustr, ocCell },
        { u"COULEUR"_ustr, u"COLOR"_ustr, ocCell },
        { u"LARGEUR"_ustr, u"WIDTH"_ustr, ocCell },
        { u"LIGNE"_ustr, u"ROW"_ustr, ocCell },
        { u"NOMFICHIER"_ustr, u"FILENAME"_ustr, ocCell },
        { u"PREFIXE"_ustr, u"PREFIX"_ustr, ocCell },
        { u"PROTEGE"_ustr, u"PROTECT"_ustr, ocCell },
        { u"NBFICH"_ustr, u"NUMFILE"_ustr, ocInfo },
        { u"RECALCUL"_ustr, u"RECALC"_ustr, ocInfo },
        { u"SYSTEXPL"_ustr, u"SYSTEM"_ustr, ocInfo },
        { u"VERSION"_ustr, u"RELEASE"_ustr, ocInfo },
        { u"VERSIONSE"_ustr, u"OSVERSION"_ustr, ocInfo },
    };

    for (const auto& element : pFr)
        map[element.from].emplace_back(element.to, element.func, aFr);

    // Hungarian language locale

    static const lang::Locale aHu(u"hu"_ustr, {}, {});

    static constexpr TransItem pHu[] = {
        { u"CÍM"_ustr, u"ADDRESS"_ustr, ocCell },
        { u"OSZLOP"_ustr, u"COL"_ustr, ocCell },
        { u"SZÍN"_ustr, u"COLOR"_ustr, ocCell },
        { u"TARTALOM"_ustr, u"CONTENTS"_ustr, ocCell },
        { u"SZÉLES"_ustr, u"WIDTH"_ustr, ocCell },
        { u"SOR"_ustr, u"ROW"_ustr, ocCell },
        { u"FILENÉV"_ustr, u"FILENAME"_ustr, ocCell },
        { u"VÉDETT"_ustr, u"PROTECT"_ustr, ocCell },
        { u"KOORD"_ustr, u"COORD"_ustr, ocCell },
        { u"FORMA"_ustr, u"FORMAT"_ustr, ocCell },
        { u"ZÁRÓJELEK"_ustr, u"PARENTHESES"_ustr, ocCell },
        { u"LAP"_ustr, u"SHEET"_ustr, ocCell },
        { u"TÍPUS"_ustr, u"TYPE"_ustr, ocCell },
        { u"FILESZÁM"_ustr, u"NUMFILE"_ustr, ocInfo },
        { u"SZÁMOLÁS"_ustr, u"RECALC"_ustr, ocInfo },
        { u"RENDSZER"_ustr, u"SYSTEM"_ustr, ocInfo },
        { u"VERZIÓ"_ustr, u"RELEASE"_ustr, ocInfo },
        { u"OPRENDSZER"_ustr, u"OSVERSION"_ustr, ocInfo },
    };

    for (const auto& element : pHu)
        map[element.from].emplace_back(element.to, element.func, aHu);

    // German language locale

    static const lang::Locale aDe(u"de"_ustr, {}, {});

    static constexpr TransItem pDe[] = {
        { u"ZEILE"_ustr, u"ROW"_ustr, ocCell },
        { u"SPALTE"_ustr, u"COL"_ustr, ocCell },
        { u"BREITE"_ustr, u"WIDTH"_ustr, ocCell },
        { u"ADRESSE"_ustr, u"ADDRESS"_ustr, ocCell },
        { u"DATEINAME"_ustr, u"FILENAME"_ustr, ocCell },
        { u"FARBE"_ustr, u"COLOR"_ustr, ocCell },
        { u"INHALT"_ustr, u"CONTENTS"_ustr, ocCell },
        { u"KLAMMERN"_ustr, u"PARENTHESES"_ustr, ocCell },
        { u"SCHUTZ"_ustr, u"PROTECT"_ustr, ocCell },
        { u"TYP"_ustr, u"TYPE"_ustr, ocCell },
        { u"PRÄFIX"_ustr, u"PREFIX"_ustr, ocCell },
        { u"BLATT"_ustr, u"SHEET"_ustr, ocCell },
        { u"KOORD"_ustr, u"COORD"_ustr, ocCell },
    };

    for (const auto& element : pDe)
        map[element.from].emplace_back(element.to, element.func, aDe);

    return map;
}

} // namespace

void ScCellKeywordTranslator::transKeyword(OUString& rName, const css::lang::Locale& rLocale,
                                           OpCode eOpCode)
{
    static const ScCellKeywordHashMap saStringNameMap(MakeMap());
    static utl::TransliterationWrapper saTransWrapper(comphelper::getProcessComponentContext(),
                                                      TransliterationFlags::LOWERCASE_UPPERCASE);

    const LanguageType nLang = LanguageTag(rLocale).makeFallback().getLanguageType();
    rName = saTransWrapper.transliterate(rName, nLang, 0, rName.getLength(), nullptr);
    lclMatchKeyword(rName, saStringNameMap, eOpCode, rLocale);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
