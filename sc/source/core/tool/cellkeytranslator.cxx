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

#include "cellkeytranslator.hxx"
#include <comphelper/processfactory.hxx>
#include <i18nlangtag/mslangid.hxx>
#include <i18nlangtag/lang.h>
#include <rtl/ustring.hxx>
#include <unotools/syslocale.hxx>

#include <com/sun/star/i18n/TransliterationModules.hpp>

using ::com::sun::star::uno::Sequence;
using ::std::list;

using namespace ::com::sun::star;

enum LocaleMatch
{
    LOCALE_MATCH_NONE = 0,
    LOCALE_MATCH_LANG,
    LOCALE_MATCH_LANG_SCRIPT,
    LOCALE_MATCH_LANG_SCRIPT_COUNTRY,
    LOCALE_MATCH_ALL
};

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

ScCellKeyword::ScCellKeyword(const sal_Char* pName, OpCode eOpCode, const lang::Locale& rLocale) :
    mpName(pName),
    meOpCode(eOpCode),
    mrLocale(rLocale)
{
}

::std::unique_ptr<ScCellKeywordTranslator> ScCellKeywordTranslator::spInstance;

static void lclMatchKeyword(OUString& rName, const ScCellKeywordHashMap& aMap,
                            OpCode eOpCode = ocNone, const lang::Locale* pLocale = nullptr)
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
        rName = OUString::createFromAscii( itr->second.front().mpName );
        return;
    }

    LanguageTag aLanguageTag( pLocale ? *pLocale : lang::Locale("","",""));
    const sal_Char* aBestMatchName = itr->second.front().mpName;
    LocaleMatch eLocaleMatchLevel = LOCALE_MATCH_NONE;
    bool bOpCodeMatched = false;

    list<ScCellKeyword>::const_iterator itrListEnd = itr->second.end();
    list<ScCellKeyword>::const_iterator itrList = itr->second.begin();
    for ( ; itrList != itrListEnd; ++itrList )
    {
        if ( eOpCode != ocNone && pLocale )
        {
            if ( itrList->meOpCode == eOpCode )
            {
                LocaleMatch eLevel = lclLocaleCompare(itrList->mrLocale, aLanguageTag);
                if ( eLevel == LOCALE_MATCH_ALL )
                {
                    // Name with matching opcode and locale found.
                    rName = OUString::createFromAscii( itrList->mpName );
                    return;
                }
                else if ( eLevel > eLocaleMatchLevel )
                {
                    // Name with a better matching locale.
                    eLocaleMatchLevel = eLevel;
                    aBestMatchName = itrList->mpName;
                }
                else if ( !bOpCodeMatched )
                    // At least the opcode matches.
                    aBestMatchName = itrList->mpName;

                bOpCodeMatched = true;
            }
        }
        else if ( eOpCode != ocNone && !pLocale )
        {
            if ( itrList->meOpCode == eOpCode )
            {
                // Name with a matching opcode preferred.
                rName = OUString::createFromAscii( itrList->mpName );
                return;
            }
        }
        else if ( pLocale )
        {
            LocaleMatch eLevel = lclLocaleCompare(itrList->mrLocale, aLanguageTag);
            if ( eLevel == LOCALE_MATCH_ALL )
            {
                // Name with matching locale preferred.
                rName = OUString::createFromAscii( itrList->mpName );
                return;
            }
            else if ( eLevel > eLocaleMatchLevel )
            {
                // Name with a better matching locale.
                eLocaleMatchLevel = eLevel;
                aBestMatchName = itrList->mpName;
            }
        }
    }

    // No preferred strings found.  Return the best matching name.
    rName = OUString::createFromAscii(aBestMatchName);
}

void ScCellKeywordTranslator::transKeyword(OUString& rName, const lang::Locale* pLocale, OpCode eOpCode)
{
    if ( !spInstance.get() )
        spInstance.reset( new ScCellKeywordTranslator );

    LanguageType nLang = pLocale ?
        LanguageTag(*pLocale).makeFallback().getLanguageType() : ScGlobal::pSysLocale->GetLanguageTag().getLanguageType();
    Sequence<sal_Int32> aOffsets;
    rName = spInstance->maTransWrapper.transliterate(rName, nLang, 0, rName.getLength(), &aOffsets);
    lclMatchKeyword(rName, spInstance->maStringNameMap, eOpCode, pLocale);
}

ScCellKeywordTranslator::ScCellKeywordTranslator() :
    maTransWrapper( ::comphelper::getProcessComponentContext(),
                    i18n::TransliterationModules_LOWERCASE_UPPERCASE )
{
    init();
}

ScCellKeywordTranslator::~ScCellKeywordTranslator()
{
}

struct TransItem
{
    const sal_Unicode*  from;
    const sal_Char*     to;
    OpCode              func;
};

void ScCellKeywordTranslator::init()
{
    ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );

    // The file below has been autogenerated by sc/workben/celltrans/parse.py.
    // To add new locale keywords, edit sc/workben/celltrans/keywords_utf16.txt
    // and re-run the parse.py script.
    //
    // All keywords must be uppercase, and the mapping must be from the
    // localized keyword to the English keyword.
    //
    // Make sure that the original keyword file (keywords_utf16.txt) is
    // encoded in UCS-2/UTF-16!

    #include "cellkeywords.inl"
}

void ScCellKeywordTranslator::addToMap(const OUString& rKey, const sal_Char* pName, const lang::Locale& rLocale, OpCode eOpCode)
{
    ScCellKeyword aKeyItem( pName, eOpCode, rLocale );

    ScCellKeywordHashMap::iterator itrEnd = maStringNameMap.end();
    ScCellKeywordHashMap::iterator itr = maStringNameMap.find(rKey);

    if ( itr == itrEnd )
    {
        // New keyword.
        list<ScCellKeyword> aList;
        aList.push_back(aKeyItem);
        maStringNameMap.insert( ScCellKeywordHashMap::value_type(rKey, aList) );
    }
    else
        itr->second.push_back(aKeyItem);
}

void ScCellKeywordTranslator::addToMap(const TransItem* pItems, const lang::Locale& rLocale)
{
    for (sal_uInt16 i = 0; pItems[i].from != nullptr; ++i)
        addToMap(OUString(pItems[i].from), pItems[i].to, rLocale, pItems[i].func);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
