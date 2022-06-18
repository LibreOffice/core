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
#pragma once
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/XServiceDisplayName.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XServiceName.hpp>
#include <com/sun/star/linguistic2/XProofreader.hpp>
#include <com/sun/star/linguistic2/ProofreadingResult.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/PropertyValues.hpp>
#include <linguistic/misc.hxx>
#include <string_view>
#include <o3tl/lru_map.hxx>
#include <tools/long.hxx>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::linguistic2;

// Magical numbers
#define MAX_CACHE_SIZE 10
#define MAX_SUGGESTIONS_SIZE 10
#define PROOFREADING_ERROR 2
#define CURL_TIMEOUT 10L

enum class HTTP_METHOD
{
    HTTP_GET,
    HTTP_POST
};

class LanguageToolGrammarChecker
    : public cppu::WeakImplHelper<XProofreader, XInitialization, XServiceInfo, XServiceDisplayName>
{
    Sequence<Locale> m_aSuppLocales;
    o3tl::lru_map<OUString, Sequence<SingleProofreadingError>> mCachedResults;
    LanguageToolGrammarChecker(const LanguageToolGrammarChecker&) = delete;
    LanguageToolGrammarChecker& operator=(const LanguageToolGrammarChecker&) = delete;
    static void parseProofreadingJSONResponse(ProofreadingResult& rResult,
                                              std::string_view aJSONBody);
    static std::string makeHttpRequest(std::string_view aURL, HTTP_METHOD method,
                                       const OString& aPostData, tools::Long& nStatusCode);

public:
    LanguageToolGrammarChecker();
    virtual ~LanguageToolGrammarChecker() override;

    // XSupportedLocales
    virtual Sequence<Locale> SAL_CALL getLocales() override;
    virtual sal_Bool SAL_CALL hasLocale(const Locale& rLocale) override;

    // XProofReader
    virtual sal_Bool SAL_CALL isSpellChecker() override;
    virtual ProofreadingResult SAL_CALL doProofreading(
        const OUString& aDocumentIdentifier, const OUString& aText, const Locale& aLocale,
        sal_Int32 nStartOfSentencePosition, sal_Int32 nSuggestedBehindEndOfSentencePosition,
        const Sequence<PropertyValue>& aProperties) override;

    virtual void SAL_CALL ignoreRule(const OUString& aRuleIdentifier,
                                     const Locale& aLocale) override;
    virtual void SAL_CALL resetIgnoreRules() override;

    // XServiceDisplayName
    virtual OUString SAL_CALL getServiceDisplayName(const Locale& rLocale) override;

    // XInitialization
    virtual void SAL_CALL initialize(const Sequence<Any>& rArguments) override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService(const OUString& rServiceName) override;
    virtual Sequence<OUString> SAL_CALL getSupportedServiceNames() override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
