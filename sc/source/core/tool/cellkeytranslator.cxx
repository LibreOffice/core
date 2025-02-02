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

#include <memory>
#include <global.hxx>
#include <cellkeytranslator.hxx>
#include <comphelper/processfactory.hxx>
#include <i18nlangtag/lang.h>
#include <i18nutil/transliteration.hxx>
#include <rtl/ustring.hxx>
#include <unotools/syslocale.hxx>
#include <com/sun/star/uno/Sequence.hxx>

using ::com::sun::star::uno::Sequence;

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

}

static LocaleMatch lclLocaleCompare(const lang::Locale& rLocale1, const LanguageTag& rLanguageTag2)
{
    LocaleMatch eMatchLevel = LOCALE_MATCH_NONE;
    LanguageTag aLanguageTag1( rLocale1);

    if ( aLanguageTag1.getLanguage() == rLanguageTag2.getLanguage() )
        eMatchLevel = LOCALE_MATCH_LANG;
    else
        return eMatchLevel;

    if ( aLanguageTag1.getScript() == rLanguageTag2.getScript() )
        eMatchLevel = LOCALE_MATCH_LANG_SCRIPT;
    else
        return eMatchLevel;

    if ( aLanguageTag1.getCountry() == rLanguageTag2.getCountry() )
        eMatchLevel = LOCALE_MATCH_LANG_SCRIPT_COUNTRY;
    else
        return eMatchLevel;

    if (aLanguageTag1 == rLanguageTag2)
        return LOCALE_MATCH_ALL;

    return eMatchLevel;
}

ScCellKeyword::ScCellKeyword(const OUString& sName, OpCode eOpCode, const lang::Locale& rLocale) :
    msName(sName),
    meOpCode(eOpCode),
    mrLocale(rLocale)
{
}

::std::unique_ptr<ScCellKeywordTranslator> ScCellKeywordTranslator::spInstance;

static void lclMatchKeyword(OUString& rName, const ScCellKeywordHashMap& aMap,
                            OpCode eOpCode, const lang::Locale* pLocale)
{
    ScCellKeywordHashMap::const_iterator itrEnd = aMap.end();
    ScCellKeywordHashMap::const_iterator itr = aMap.find(rName);

    if ( itr == itrEnd || itr->second.empty() )
        // No candidate strings exist.  Bail out.
        return;

    if ( eOpCode == ocNone && !pLocale )
    {
        // Since no locale nor opcode matching is needed, simply return
        // the first item on the list.
        rName = itr->second.front().msName;
        return;
    }

    LanguageTag aLanguageTag( pLocale ? *pLocale : lang::Locale(u""_ustr,u""_ustr,u""_ustr));
    const OUString* aBestMatchName = nullptr;
    LocaleMatch eLocaleMatchLevel = LOCALE_MATCH_NONE;
    bool bOpCodeMatched = false;

    for (auto const& elem : itr->second)
    {
        if ( eOpCode != ocNone && pLocale )
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
                else if ( !bOpCodeMatched )
                    // At least the opcode matches.
                    aBestMatchName = &elem.msName;

                bOpCodeMatched = true;
            }
        }
        else if ( eOpCode != ocNone && !pLocale )
        {
            if ( elem.meOpCode == eOpCode )
            {
                // Name with a matching opcode preferred.
                rName = elem.msName;
                return;
            }
        }
        else if ( pLocale )
        {
            LocaleMatch eLevel = lclLocaleCompare(elem.mrLocale, aLanguageTag);
            if ( eLevel == LOCALE_MATCH_ALL )
            {
                // Name with matching locale preferred.
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

void ScCellKeywordTranslator::transKeyword(OUString& rName, const lang::Locale* pLocale, OpCode eOpCode)
{
    if (!spInstance)
        spInstance.reset( new ScCellKeywordTranslator );

    LanguageType nLang = pLocale ?
        LanguageTag(*pLocale).makeFallback().getLanguageType() : ScGlobal::oSysLocale->GetLanguageTag().getLanguageType();
    Sequence<sal_Int32> aOffsets;
    rName = spInstance->maTransWrapper.transliterate(rName, nLang, 0, rName.getLength(), &aOffsets);
    lclMatchKeyword(rName, spInstance->maStringNameMap, eOpCode, pLocale);
}

struct TransItem
{
    OUString            from;
    OUString            to;
    OpCode              func;
};

ScCellKeywordTranslator::ScCellKeywordTranslator() :
    maTransWrapper( ::comphelper::getProcessComponentContext(),
                    TransliterationFlags::LOWERCASE_UPPERCASE )
{
    // All keywords must be uppercase, and the mapping must be from the
    // localized keyword to the English keyword.

    // French language locale

    static const lang::Locale aFr(u"fr"_ustr, u""_ustr, u""_ustr);

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
        addToMap(element.from, element.to, aFr, element.func);

    // Hungarian language locale

    static const lang::Locale aHu(u"hu"_ustr, u""_ustr, u""_ustr);

    static constexpr TransItem pHu[] = {
        { u"CÍM"_ustr, u"ADDRESS"_ustr, ocCell },
        { u"OSZLOP"_ustr, u"COL"_ustr, ocCell },
        { u"SZÍN"_ustr, u"COLOR"_ustr, ocCell },
        { u"TARTALOM"_ustr, u"CONTENTS"_ustr, ocCell },
        { u"SZÉLES"_ustr, u"WIDTH"_ustr, ocCell },
        { u"SOR"_ustr, u"ROW"_ustr, ocCell },
        { u"FILENÉV"_ustr, u"FILENAME"_ustr, ocCell },
        { u"PREFIX"_ustr, u"PREFIX"_ustr, ocCell },
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
        addToMap(element.from, element.to, aHu, element.func);

    // German language locale

    static const lang::Locale aDe(u"de"_ustr, u""_ustr, u""_ustr);

    static constexpr TransItem pDe[] = {
        { u"ZEILE"_ustr, u"ROW"_ustr, ocCell },
        { u"SPALTE"_ustr, u"COL"_ustr, ocCell },
        { u"BREITE"_ustr, u"WIDTH"_ustr, ocCell },
        { u"ADRESSE"_ustr, u"ADDRESS"_ustr, ocCell },
        { u"DATEINAME"_ustr, u"FILENAME"_ustr, ocCell },
        { u"FARBE"_ustr, u"COLOR"_ustr, ocCell },
        { u"FORMAT"_ustr, u"FORMAT"_ustr, ocCell },
        { u"INHALT"_ustr, u"CONTENTS"_ustr, ocCell },
        { u"KLAMMERN"_ustr, u"PARENTHESES"_ustr, ocCell },
        { u"SCHUTZ"_ustr, u"PROTECT"_ustr, ocCell },
        { u"TYP"_ustr, u"TYPE"_ustr, ocCell },
        { u"PRÄFIX"_ustr, u"PREFIX"_ustr, ocCell },
        { u"BLATT"_ustr, u"SHEET"_ustr, ocCell },
        { u"KOORD"_ustr, u"COORD"_ustr, ocCell },
    };

    for (const auto& element : pDe)
        addToMap(element.from, element.to, aDe, element.func);
}

ScCellKeywordTranslator::~ScCellKeywordTranslator()
{
}

void ScCellKeywordTranslator::addToMap(const OUString& rKey, const OUString& pName, const lang::Locale& rLocale, OpCode eOpCode)
{
    ScCellKeyword aKeyItem( pName, eOpCode, rLocale );

    ScCellKeywordHashMap::iterator itrEnd = maStringNameMap.end();
    ScCellKeywordHashMap::iterator itr = maStringNameMap.find(rKey);

    if ( itr == itrEnd )
    {
        // New keyword.
        std::vector<ScCellKeyword> aVector { aKeyItem };
        maStringNameMap.emplace(rKey, aVector);
    }
    else
        itr->second.push_back(aKeyItem);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
