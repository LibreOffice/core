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


// generated list of languages
#include "lrl_include.hxx"

#include <rtl/ustrbuf.hxx>
#include <i18nlangtag/languagetag.hxx>
#include <i18nlangtag/languagetagicu.hxx>
#include <collator_unicode.hxx>
#include <localedata.hxx>
#include <com/sun/star/i18n/CollatorOptions.hpp>

using namespace ::com::sun::star;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;
using namespace ::rtl;

namespace com { namespace sun { namespace star { namespace i18n {

Collator_Unicode::Collator_Unicode()
{
    implementationName = "com.sun.star.i18n.Collator_Unicode";
    collator = NULL;
    uca_base = NULL;
#ifndef DISABLE_DYNLOADING
    hModule = NULL;
#endif
}

Collator_Unicode::~Collator_Unicode()
{
    if (collator) delete collator;
    if (uca_base) delete uca_base;
#ifndef DISABLE_DYNLOADING
    if (hModule) osl_unloadModule(hModule);
#endif
}

#ifdef DISABLE_DYNLOADING

extern "C" {

// For DISABLE_DYNLOADING the generated functions have names that
// start with get_collator_data_ to avoid clashing with a few
// functions in the generated libindex_data that are called just
// get_zh_pinyin for instance.

const sal_uInt8* get_collator_data_ca_charset();
const sal_uInt8* get_collator_data_dz_charset();
const sal_uInt8* get_collator_data_hu_charset();
const sal_uInt8* get_collator_data_ja_charset();
const sal_uInt8* get_collator_data_ja_phonetic_alphanumeric_first();
const sal_uInt8* get_collator_data_ja_phonetic_alphanumeric_last();
const sal_uInt8* get_collator_data_ko_charset();
const sal_uInt8* get_collator_data_ku_alphanumeric();
const sal_uInt8* get_collator_data_ln_charset();
const sal_uInt8* get_collator_data_my_dictionary();
const sal_uInt8* get_collator_data_ne_charset();
const sal_uInt8* get_collator_data_sid_charset();
const sal_uInt8* get_collator_data_zh_TW_charset();
const sal_uInt8* get_collator_data_zh_TW_radical();
const sal_uInt8* get_collator_data_zh_TW_stroke();
const sal_uInt8* get_collator_data_zh_charset();
const sal_uInt8* get_collator_data_zh_pinyin();
const sal_uInt8* get_collator_data_zh_radical();
const sal_uInt8* get_collator_data_zh_stroke();
const sal_uInt8* get_collator_data_zh_zhuyin();

}

#endif

sal_Int32 SAL_CALL
Collator_Unicode::compareSubstring( const OUString& str1, sal_Int32 off1, sal_Int32 len1,
    const OUString& str2, sal_Int32 off2, sal_Int32 len2) throw(RuntimeException)
{
    return collator->compare(reinterpret_cast<const UChar *>(str1.getStr()) + off1, len1, reinterpret_cast<const UChar *>(str2.getStr()) + off2, len2); // UChar != sal_Unicode in MinGW
}

sal_Int32 SAL_CALL
Collator_Unicode::compareString( const OUString& str1, const OUString& str2) throw(RuntimeException)
{
    return collator->compare(reinterpret_cast<const UChar *>(str1.getStr()), reinterpret_cast<const UChar *>(str2.getStr()));   // UChar != sal_Unicode in MinGW
}

#ifndef DISABLE_DYNLOADING

extern "C" { static void SAL_CALL thisModule() {} }

#endif

sal_Int32 SAL_CALL
Collator_Unicode::loadCollatorAlgorithm(const OUString& rAlgorithm, const lang::Locale& rLocale, sal_Int32 options)
    throw(RuntimeException)
{
    if (!collator) {
        UErrorCode status = U_ZERO_ERROR;
        OUString rule = LocaleDataImpl().getCollatorRuleByAlgorithm(rLocale, rAlgorithm);
        if (!rule.isEmpty()) {
            collator = new RuleBasedCollator(reinterpret_cast<const UChar *>(rule.getStr()), status);   // UChar != sal_Unicode in MinGW
            if (! U_SUCCESS(status)) throw RuntimeException();
        }
        if (!collator && OUString::createFromAscii(LOCAL_RULE_LANGS).indexOf(rLocale.Language) >= 0) {
            const sal_uInt8* (*func)() = NULL;

#ifndef DISABLE_DYNLOADING
            OUStringBuffer aBuf;
#ifdef SAL_DLLPREFIX
            aBuf.appendAscii(SAL_DLLPREFIX);
#endif
            aBuf.appendAscii( "collator_data" ).appendAscii( SAL_DLLEXTENSION );
            hModule = osl_loadModuleRelative( &thisModule, aBuf.makeStringAndClear().pData, SAL_LOADMODULE_DEFAULT );
            if (hModule) {
                aBuf.appendAscii("get_").append(rLocale.Language).appendAscii("_");
                if ( rLocale.Language == "zh" ) {
                    OUString func_base = aBuf.makeStringAndClear();
                    if (OUString("TW HK MO").indexOf(rLocale.Country) >= 0)
                        func=(const sal_uInt8* (*)()) osl_getFunctionSymbol(hModule,
                                    OUString(func_base + "TW_" + rAlgorithm).pData);
                    if (!func)
                        func=(const sal_uInt8* (*)()) osl_getFunctionSymbol(hModule, OUString(func_base + rAlgorithm).pData);
                } else {
                    if ( rLocale.Language == "ja" ) {
                        // replace algorithm name to implementation name.
                        if (rAlgorithm.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("phonetic (alphanumeric first)")) )
                            aBuf.appendAscii("phonetic_alphanumeric_first");
                        else if (rAlgorithm.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("phonetic (alphanumeric last)")))
                            aBuf.appendAscii("phonetic_alphanumeric_last");
                        else
                            aBuf.append(rAlgorithm);
                    } else {
                        aBuf.append(rAlgorithm);
                    }
                    func=(const sal_uInt8* (*)()) osl_getFunctionSymbol(hModule, aBuf.makeStringAndClear().pData);
                }
            }
#else
            if ( rLocale.Language == "ca" ) {
                if ( rAlgorithm == "charset" )
                    func = get_collator_data_ca_charset;
            } else if ( rLocale.Language == "dz" || rLocale.Language == "bo" ) {
                // 'bo' Tibetan uses the same collation rules as 'dz' Dzongkha
                if ( rAlgorithm == "charset" )
                    func = get_collator_data_dz_charset;
            } else if ( rLocale.Language == "hu" ) {
                if ( rAlgorithm == "charset" )
                    func = get_collator_data_hu_charset;
            } else if ( rLocale.Language == "ja" ) {
                if ( rAlgorithm == "charset" )
                    func = get_collator_data_ja_charset;
                else if ( rAlgorithm == "phonetic (alphanumeric first)" )
                    func = get_collator_data_ja_phonetic_alphanumeric_first;
                else if ( rAlgorithm == "phonetic (alphanumeric last)" )
                    func = get_collator_data_ja_phonetic_alphanumeric_last;
            } else if ( rLocale.Language == "ko" ) {
                if ( rAlgorithm == "charset" )
                    func = get_collator_data_ko_charset;
            } else if ( rLocale.Language == "ku" ) {
                if ( rAlgorithm == "alphanumeric" )
                    func = get_collator_data_ku_alphanumeric;
            } else if ( rLocale.Language == "ln" ) {
                if ( rAlgorithm == "charset" )
                    func = get_collator_data_ln_charset;
            } else if ( rLocale.Language == "my" ) {
                if ( rAlgorithm == "dictionary" )
                    func = get_collator_data_my_dictionary;
            } else if ( rLocale.Language == "ne" ) {
                if ( rAlgorithm == "charset" )
                    func = get_collator_data_ne_charset;
            } else if ( rLocale.Language == "sid" ) {
                if ( rAlgorithm == "charset" )
                    func = get_collator_data_sid_charset;
            } else if ( rLocale.Language == "zh" && (rLocale.Country == "TW" || rLocale.Country == "HK" || rLocale.Country == "MO") ) {
                if ( rAlgorithm == "charset" )
                    func = get_collator_data_zh_TW_charset;
                else if ( rAlgorithm == "radical" )
                    func = get_collator_data_zh_TW_radical;
                else if ( rAlgorithm == "stroke" )
                    func = get_collator_data_zh_TW_stroke;
            } else if ( rLocale.Language == "zh" ) {
                if ( rAlgorithm == "charset" )
                    func = get_collator_data_zh_charset;
                else if ( rAlgorithm == "pinyin" )
                    func = get_collator_data_zh_pinyin;
                else if ( rAlgorithm == "radical" )
                    func = get_collator_data_zh_radical;
                else if ( rAlgorithm == "stroke" )
                    func = get_collator_data_zh_stroke;
                else if ( rAlgorithm == "zhuyin" )
                    func = get_collator_data_zh_zhuyin;
            }
#endif
            if (func) {
                const sal_uInt8* ruleImage=func();
                uca_base = new RuleBasedCollator(static_cast<UChar*>(NULL), status);
                if (! U_SUCCESS(status)) throw RuntimeException();
                collator = new RuleBasedCollator(reinterpret_cast<const uint8_t*>(ruleImage), -1, uca_base, status);
                if (! U_SUCCESS(status)) throw RuntimeException();
            }
        }
        if (!collator) {
            /** ICU collators are loaded using a locale only.
                ICU uses Variant as collation algorithm name (like de__PHONEBOOK
                locale), note the empty territory (Country) designator in this special
                case here. The icu::Locale constructor changes the algorithm name to
                uppercase itself, so we don't have to bother with that.
            */
            icu::Locale icuLocale( LanguageTagIcu::getIcuLocale( LanguageTag( rLocale), rAlgorithm));
            // load ICU collator
            collator = (RuleBasedCollator*) icu::Collator::createInstance(icuLocale, status);
            if (! U_SUCCESS(status)) throw RuntimeException();
        }
    }

    if (options & CollatorOptions::CollatorOptions_IGNORE_CASE_ACCENT)
        collator->setStrength(Collator::PRIMARY);
    else if (options & CollatorOptions::CollatorOptions_IGNORE_CASE)
        collator->setStrength(Collator::SECONDARY);
    else
        collator->setStrength(Collator::TERTIARY);

    return(0);
}


OUString SAL_CALL
Collator_Unicode::getImplementationName() throw( RuntimeException )
{
    return OUString::createFromAscii(implementationName);
}

sal_Bool SAL_CALL
Collator_Unicode::supportsService(const OUString& rServiceName) throw( RuntimeException )
{
    return !rServiceName.compareToAscii(implementationName);
}

Sequence< OUString > SAL_CALL
Collator_Unicode::getSupportedServiceNames() throw( RuntimeException )
{
    Sequence< OUString > aRet(1);
    aRet[0] = OUString::createFromAscii(implementationName);
    return aRet;
}

} } } }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
