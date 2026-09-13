/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <com/sun/star/lang/Locale.hpp>
#include <com/sun/star/lang/XServiceDisplayName.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/linguistic2/ProofreadingResult.hpp>
#include <com/sun/star/linguistic2/XProofreader.hpp>
#include <com/sun/star/linguistic2/XSupportedLocales.hpp>
#include <cppuhelper/implbase.hxx>

#include <map>
#include <memory>
#include <set>
#include <vector>

#include <unicode/regex.h>

#include "lprfile.hxx"

namespace lightproof
{
// A rule with its ICU pattern built. Patterns are compiled on a rule's first
// use rather than at load, because most rules in a package are never reached
// by a given document.
struct CompiledRule
{
    std::unique_ptr<icu::RegexPattern> pPattern;
    std::unique_ptr<icu::RegexMatcher> pMatcher;
    // The pattern text, which is also the rule's identifier for ignoreRule().
    OUString aIdentifier;
    bool bCompiled = false;
};

// A loaded rule package, with the option values in force for it.
struct Package
{
    std::shared_ptr<const RuleFile> pFile;
    std::vector<bool> aOptions;
    std::vector<CompiledRule> aRules;
};

// XProofreader already derives from XSupportedLocales, so it must not be
// listed again here.
class Lightproof final
    : public cppu::WeakImplHelper<css::linguistic2::XProofreader, css::lang::XServiceDisplayName,
                                  css::lang::XServiceInfo>
{
public:
    Lightproof();
    ~Lightproof() override;

    Lightproof(const Lightproof&) = delete;
    Lightproof& operator=(const Lightproof&) = delete;

    // XSupportedLocales
    cpo::uno::Sequence<css::lang::Locale> getLocales() override;
    bool hasLocale(const css::lang::Locale& rLocale) override;

    // XProofreader
    bool isSpellChecker() override;
    css::linguistic2::ProofreadingResult
    doProofreading(const OUString& aDocumentIdentifier, const OUString& aText,
                   const css::lang::Locale& aLocale, sal_Int32 nStartOfSentencePosition,
                   sal_Int32 nSuggestedBehindEndOfSentencePosition,
                   const cpo::uno::Sequence<css::beans::PropertyValue>& aProperties) override;
    void ignoreRule(const OUString& aRuleIdentifier, const css::lang::Locale& aLocale) override;
    void resetIgnoreRules() override;

    // XServiceDisplayName
    OUString getServiceDisplayName(const css::lang::Locale& rLocale) override;

    // XServiceInfo
    OUString getImplementationName() override;
    bool supportsService(const OUString& rServiceName) override;
    cpo::uno::Sequence<OUString> getSupportedServiceNames() override;

private:
    // Reads the DICT_LIGHTPROOF entries out of the linguistic configuration.
    // The rule files themselves are opened on demand.
    void discoverPackages();
    // The package serving a locale, loading it if this is its first use.
    Package* getPackage(const css::lang::Locale& rLocale);

    bool m_bDiscovered = false;
    // BCP 47 tag to rule file URL, from the configuration.
    std::map<OUString, OUString> m_aLocationByLocale;
    cpo::uno::Sequence<css::lang::Locale> m_aSupportedLocales;
    // Rule file URL to loaded package, so two locales sharing a package share
    // its compiled patterns.
    std::map<OUString, std::unique_ptr<Package>> m_aPackages;
    std::set<OUString> m_aIgnoredRules;
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
