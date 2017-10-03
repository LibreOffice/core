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

#include <inputsequencechecker.hxx>
#include <com/sun/star/i18n/InputSequenceCheckMode.hpp>
#include <com/sun/star/i18n/UnicodeType.hpp>
#include <cppuhelper/supportsservice.hxx>
#include <i18nutil/unicode.hxx>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::i18n;
using namespace ::com::sun::star::lang;

namespace i18npool {

InputSequenceCheckerImpl::InputSequenceCheckerImpl( const Reference < XComponentContext >& rxContext ) : m_xContext( rxContext )
{
    serviceName = "com.sun.star.i18n.InputSequenceChecker";
    cachedItem = nullptr;
}

InputSequenceCheckerImpl::InputSequenceCheckerImpl(const char *pServiceName)
    : serviceName(pServiceName)
    , cachedItem(nullptr)
{
}

InputSequenceCheckerImpl::~InputSequenceCheckerImpl()
{
    // Clear lookuptable
    for (lookupTableItem* p : lookupTable)
        delete p;

    lookupTable.clear();
}

sal_Bool SAL_CALL
InputSequenceCheckerImpl::checkInputSequence(const OUString& Text, sal_Int32 nStartPos,
        sal_Unicode inputChar, sal_Int16 inputCheckMode)
{
    if (inputCheckMode == InputSequenceCheckMode::PASSTHROUGH)
        return true;

    sal_Char* language = getLanguageByScripType(Text[nStartPos], inputChar);

    if (language)
        return getInputSequenceChecker(language)->checkInputSequence(Text, nStartPos, inputChar, inputCheckMode);
    else
        return true; // not a checkable languages.
}

sal_Int32 SAL_CALL
InputSequenceCheckerImpl::correctInputSequence(OUString& Text, sal_Int32 nStartPos,
        sal_Unicode inputChar, sal_Int16 inputCheckMode)
{
    if (inputCheckMode != InputSequenceCheckMode::PASSTHROUGH) {
        sal_Char* language = getLanguageByScripType(Text[nStartPos], inputChar);

        if (language)
            return getInputSequenceChecker(language)->correctInputSequence(Text, nStartPos, inputChar, inputCheckMode);
    }
    Text = Text.replaceAt(++nStartPos, 0, OUString(inputChar));
    return nStartPos;
}

static ScriptTypeList typeList[] = {
    //{ UnicodeScript_kHebrew,              UnicodeScript_kHebrew },        // 10,
    //{ UnicodeScript_kArabic,              UnicodeScript_kArabic },        // 11,
    { UnicodeScript_kDevanagari,  UnicodeScript_kDevanagari,    (sal_Int16)UnicodeScript_kDevanagari },    // 14,
    { UnicodeScript_kThai,        UnicodeScript_kThai,          (sal_Int16)UnicodeScript_kThai },          // 24,

    { UnicodeScript_kScriptCount, UnicodeScript_kScriptCount,   (sal_Int16)UnicodeScript_kScriptCount }    // 88
};

sal_Char* SAL_CALL
InputSequenceCheckerImpl::getLanguageByScripType(sal_Unicode cChar, sal_Unicode nChar)
{
    css::i18n::UnicodeScript type = (css::i18n::UnicodeScript)unicode::getUnicodeScriptType( cChar, typeList, (sal_Int16)UnicodeScript_kScriptCount );

    if (type != UnicodeScript_kScriptCount &&
            type == (css::i18n::UnicodeScript)unicode::getUnicodeScriptType( nChar, typeList, (sal_Int16)UnicodeScript_kScriptCount )) {
        switch(type) {
            case UnicodeScript_kThai:           return const_cast<sal_Char*>("th");
                                                //case UnicodeScript_kArabic:       return (sal_Char*)"ar";
                                                //case UnicodeScript_kHebrew:       return (sal_Char*)"he";
            case UnicodeScript_kDevanagari:   return const_cast<sal_Char*>("hi");
            default: break;
        }
    }
    return nullptr;
}

Reference< XExtendedInputSequenceChecker >& SAL_CALL
InputSequenceCheckerImpl::getInputSequenceChecker(sal_Char const * rLanguage)
{
    if (cachedItem && cachedItem->aLanguage == rLanguage) {
        return cachedItem->xISC;
    }
    else {
        for (lookupTableItem* l : lookupTable) {
            cachedItem = l;
            if (cachedItem->aLanguage == rLanguage)
                return cachedItem->xISC;
        }

        Reference < XInterface > xI = m_xContext->getServiceManager()->createInstanceWithContext(
                "com.sun.star.i18n.InputSequenceChecker_" +
                OUString::createFromAscii(rLanguage),
                m_xContext);

        if ( xI.is() ) {
            Reference< XExtendedInputSequenceChecker > xISC( xI, UNO_QUERY );
            if (xISC.is()) {
                lookupTable.push_back(cachedItem = new lookupTableItem(rLanguage, xISC));
                return cachedItem->xISC;
            }
        }
    }
    throw RuntimeException();
}

OUString SAL_CALL
InputSequenceCheckerImpl::getImplementationName()
{
    return OUString::createFromAscii(serviceName);
}

sal_Bool SAL_CALL
InputSequenceCheckerImpl::supportsService(const OUString& rServiceName)
{
    return cppu::supportsService(this, rServiceName);
}

Sequence< OUString > SAL_CALL
InputSequenceCheckerImpl::getSupportedServiceNames()
{
    Sequence< OUString > aRet { OUString::createFromAscii(serviceName) };
    return aRet;
}

}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface * SAL_CALL
com_sun_star_i18n_InputSequenceChecker_get_implementation(
    css::uno::XComponentContext *context,
    css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new i18npool::InputSequenceCheckerImpl(context));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
