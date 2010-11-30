/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_i18npool.hxx"

// generated list of languages
#include "lrl_include.hxx"

#include <rtl/ustrbuf.hxx>
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
    hModule = NULL;
}

Collator_Unicode::~Collator_Unicode()
{
    if (collator) delete collator;
    if (uca_base) delete uca_base;
    if (hModule) osl_unloadModule(hModule);
}

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

extern "C" { static void SAL_CALL thisModule() {} }

sal_Int32 SAL_CALL
Collator_Unicode::loadCollatorAlgorithm(const OUString& rAlgorithm, const lang::Locale& rLocale, sal_Int32 options)
    throw(RuntimeException)
{
    if (!collator) {
        UErrorCode status = U_ZERO_ERROR;
        OUString rule = LocaleData().getCollatorRuleByAlgorithm(rLocale, rAlgorithm);
        if (rule.getLength() > 0) {
            collator = new RuleBasedCollator(reinterpret_cast<const UChar *>(rule.getStr()), status);   // UChar != sal_Unicode in MinGW
            if (! U_SUCCESS(status)) throw RuntimeException();
        }
        if (!collator && OUString::createFromAscii(LOCAL_RULE_LANGS).indexOf(rLocale.Language) >= 0) {
            OUStringBuffer aBuf;
#ifdef SAL_DLLPREFIX
            aBuf.appendAscii(SAL_DLLPREFIX);
#endif
            aBuf.appendAscii( "collator_data" ).appendAscii( SAL_DLLEXTENSION );
            hModule = osl_loadModuleRelative( &thisModule, aBuf.makeStringAndClear().pData, SAL_LOADMODULE_DEFAULT );
            if (hModule) {
                const sal_uInt8* (*func)() = NULL;
                aBuf.appendAscii("get_").append(rLocale.Language).appendAscii("_");
                if (rLocale.Language.equalsAscii("zh")) {
                    OUString func_base = aBuf.makeStringAndClear();
                    if (OUString(RTL_CONSTASCII_USTRINGPARAM("TW HK MO")).indexOf(rLocale.Country) >= 0)
                        func=(const sal_uInt8* (*)()) osl_getFunctionSymbol(hModule,
                                    (func_base + OUString(RTL_CONSTASCII_USTRINGPARAM("TW_")) + rAlgorithm).pData);
                    if (!func)
                        func=(const sal_uInt8* (*)()) osl_getFunctionSymbol(hModule, (func_base + rAlgorithm).pData);
                } else {
                    if (rLocale.Language.equalsAscii("ja")) {
                        // replace algrithm name to implementation name.
                        if (rAlgorithm.equalsAscii("phonetic (alphanumeric first)") )
                            aBuf.appendAscii("phonetic_alphanumeric_first");
                        else if (rAlgorithm.equalsAscii("phonetic (alphanumeric last)"))
                            aBuf.appendAscii("phonetic_alphanumeric_last");
                        else
                            aBuf.append(rAlgorithm);
                    } else {
                        aBuf.append(rAlgorithm);
                    }
                    func=(const sal_uInt8* (*)()) osl_getFunctionSymbol(hModule, aBuf.makeStringAndClear().pData);
                }
                if (func) {
                    const sal_uInt8* ruleImage=func();
                    uca_base = new RuleBasedCollator(static_cast<UChar*>(NULL), status);
                    if (! U_SUCCESS(status)) throw RuntimeException();
                    collator = new RuleBasedCollator(reinterpret_cast<const uint8_t*>(ruleImage), -1, uca_base, status);
                    if (! U_SUCCESS(status)) throw RuntimeException();
                }
            }
        }
        if (!collator) {
            /** ICU collators are loaded using a locale only.
                ICU uses Variant as collation algorithm name (like de__PHONEBOOK
                locale), note the empty territory (Country) designator in this special
                case here. The icu::Locale contructor changes the algorithm name to
                uppercase itself, so we don't have to bother with that.
            */
            icu::Locale icuLocale(
                   OUStringToOString(rLocale.Language, RTL_TEXTENCODING_ASCII_US).getStr(),
                   OUStringToOString(rLocale.Country, RTL_TEXTENCODING_ASCII_US).getStr(),
                   OUStringToOString(rAlgorithm, RTL_TEXTENCODING_ASCII_US).getStr());
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
Collator_Unicode::supportsService(const rtl::OUString& rServiceName) throw( RuntimeException )
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
