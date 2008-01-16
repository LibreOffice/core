/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: collator_unicode.cxx,v $
 *
 *  $Revision: 1.18 $
 *
 *  last change: $Author: ihi $ $Date: 2008-01-16 14:32:11 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_i18npool.hxx"

// generated list of languages
#include "lrl_include.hxx"

#include <rtl/ustrbuf.hxx>
#include <collator_unicode.hxx>
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
    hModule = NULL;
}

Collator_Unicode::~Collator_Unicode()
{
    if (collator) delete collator;
    if (hModule) osl_unloadModule(hModule);
}

sal_Int32 SAL_CALL
Collator_Unicode::compareSubstring( const OUString& str1, sal_Int32 off1, sal_Int32 len1,
    const OUString& str2, sal_Int32 off2, sal_Int32 len2) throw(RuntimeException)
{
    return collator->compare(str1.getStr() + off1, len1, str2.getStr() + off2, len2);
}

sal_Int32 SAL_CALL
Collator_Unicode::compareString( const OUString& str1, const OUString& str2) throw(RuntimeException)
{
    return collator->compare(str1.getStr(), str2.getStr());
}

extern "C" { static void SAL_CALL thisModule() {} }

sal_Int32 SAL_CALL
Collator_Unicode::loadCollatorAlgorithm(const OUString& rAlgorithm, const lang::Locale& rLocale, sal_Int32 options)
    throw(RuntimeException)
{
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
        UErrorCode status = U_ZERO_ERROR;
        collator = (RuleBasedCollator*) icu::Collator::createInstance(icuLocale, status);
        if (! U_SUCCESS(status))
            throw RuntimeException();
        if (OUString::createFromAscii(LOCAL_RULE_LANGS).indexOf(rLocale.Language) >= 0) {
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
                    if (OUString::createFromAscii("TW HK MO").indexOf(rLocale.Country) >= 0)
                        func=(const sal_uInt8* (*)()) osl_getFunctionSymbol(hModule,
                                    (func_base + OUString::createFromAscii("TW_") + rAlgorithm).pData);
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
                    collator = new RuleBasedCollator(reinterpret_cast<const uint8_t*>(ruleImage), -1, collator, status);
                    if (! U_SUCCESS(status))
                        throw RuntimeException();
                }
            }
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

