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
using namespace ::com::sun::star::lang;

namespace com { namespace sun { namespace star { namespace i18n {

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
    for (size_t l = 0; l < lookupTable.size(); l++)
        delete lookupTable[l];

    lookupTable.clear();
}

sal_Bool SAL_CALL
InputSequenceCheckerImpl::checkInputSequence(const OUString& Text, sal_Int32 nStartPos,
        sal_Unicode inputChar, sal_Int16 inputCheckMode) throw(RuntimeException, std::exception)
{
    if (inputCheckMode == InputSequenceCheckMode::PASSTHROUGH)
        return sal_True;

    sal_Char* language = getLanguageByScripType(Text[nStartPos], inputChar);

    if (language)
        return getInputSequenceChecker(language)->checkInputSequence(Text, nStartPos, inputChar, inputCheckMode);
    else
        return sal_True; // not a checkable languages.
}

sal_Int32 SAL_CALL
InputSequenceCheckerImpl::correctInputSequence(OUString& Text, sal_Int32 nStartPos,
        sal_Unicode inputChar, sal_Int16 inputCheckMode) throw(RuntimeException, std::exception)
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
    { UnicodeScript_kDevanagari,UnicodeScript_kDevanagari,          UnicodeScript_kDevanagari },    // 14,
    { UnicodeScript_kThai,  UnicodeScript_kThai,                  UnicodeScript_kThai },          // 24,

    { UnicodeScript_kScriptCount,   UnicodeScript_kScriptCount,           UnicodeScript_kScriptCount }    // 88
};

sal_Char* SAL_CALL
InputSequenceCheckerImpl::getLanguageByScripType(sal_Unicode cChar, sal_Unicode nChar)
{
    sal_Int16 type = unicode::getUnicodeScriptType( cChar, typeList, UnicodeScript_kScriptCount );

    if (type != UnicodeScript_kScriptCount &&
            type == unicode::getUnicodeScriptType( nChar, typeList, UnicodeScript_kScriptCount )) {
        switch(type) {
            case UnicodeScript_kThai:           return const_cast<sal_Char*>("th");
                                                //case UnicodeScript_kArabic:       return (sal_Char*)"ar";
                                                //case UnicodeScript_kHebrew:       return (sal_Char*)"he";
            case UnicodeScript_kDevanagari:   return const_cast<sal_Char*>("hi");
        }
    }
    return nullptr;
}

Reference< XExtendedInputSequenceChecker >& SAL_CALL
InputSequenceCheckerImpl::getInputSequenceChecker(sal_Char* rLanguage) throw (RuntimeException)
{
    if (cachedItem && cachedItem->aLanguage == rLanguage) {
        return cachedItem->xISC;
    }
    else {
        for (size_t l = 0; l < lookupTable.size(); l++) {
            cachedItem = lookupTable[l];
            if (cachedItem->aLanguage == rLanguage)
                return cachedItem->xISC;
        }

        Reference < uno::XInterface > xI = m_xContext->getServiceManager()->createInstanceWithContext(
                "com.sun.star.i18n.InputSequenceChecker_" +
                OUString::createFromAscii(rLanguage),
                m_xContext);

        if ( xI.is() ) {
            Reference< XExtendedInputSequenceChecker > xISC( xI, uno::UNO_QUERY );
            if (xISC.is()) {
                lookupTable.push_back(cachedItem = new lookupTableItem(rLanguage, xISC));
                return cachedItem->xISC;
            }
        }
    }
    throw RuntimeException();
}

OUString SAL_CALL
InputSequenceCheckerImpl::getImplementationName() throw( RuntimeException, std::exception )
{
    return OUString::createFromAscii(serviceName);
}

sal_Bool SAL_CALL
InputSequenceCheckerImpl::supportsService(const OUString& rServiceName) throw( RuntimeException, std::exception )
{
    return cppu::supportsService(this, rServiceName);
}

Sequence< OUString > SAL_CALL
InputSequenceCheckerImpl::getSupportedServiceNames() throw( RuntimeException, std::exception )
{
    Sequence< OUString > aRet { OUString::createFromAscii(serviceName) };
    return aRet;
}

} } } }

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface * SAL_CALL
com_sun_star_i18n_InputSequenceChecker_get_implementation(
    css::uno::XComponentContext *context,
    css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new css::i18n::InputSequenceCheckerImpl(context));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
