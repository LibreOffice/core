/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <config_features.h>

#include <linguistic/translate.hxx>
#include <translate/providers.hxx>

#include <o3tl/string_view.hxx>
#include <officecfg/Office/Linguistic.hxx>
#include <sal/log.hxx>

namespace linguistic
{
TranslationProvider::~TranslationProvider() = default;

std::vector<OString> TranslationProvider::getTargetLanguages(bool bAskService)
{
    // A request can block for seconds, so after a failed one the service is not asked
    // again for a while; it is asked again later, so a temporary failure does not hide
    // the list for good.
    static constexpr std::chrono::minutes RETRY_AFTER_FAILURE(5);

    const OString aKey = getLanguageCacheKey();
    const std::chrono::steady_clock::time_point aNow = std::chrono::steady_clock::now();
    if (aKey == m_aCachedKey
        && (!m_aCachedLanguages.empty()
            || (m_oLastFailure && aNow - *m_oLastFailure < RETRY_AFTER_FAILURE)))
        return m_aCachedLanguages;
    if (!bAskService)
        return aKey == m_aCachedKey ? m_aCachedLanguages : std::vector<OString>();

    m_aCachedLanguages = fetchTargetLanguages();
    m_aCachedKey = aKey;
    if (m_aCachedLanguages.empty())
        m_oLastFailure = aNow;
    else
        m_oLastFailure.reset();
    return m_aCachedLanguages;
}

const std::vector<std::shared_ptr<TranslationProvider>>& getTranslationProviders()
{
    static const std::vector<std::shared_ptr<TranslationProvider>> aProviders = [] {
        std::vector<std::shared_ptr<TranslationProvider>> aList;
#if HAVE_FEATURE_CURL
        aList.push_back(translate::createDeeplProvider());
#endif
        return aList;
    }();
    return aProviders;
}

std::shared_ptr<TranslationProvider> getTranslationProvider(std::string_view rId)
{
    for (const std::shared_ptr<TranslationProvider>& pProvider : getTranslationProviders())
    {
        if (pProvider->getId() == rId)
            return pProvider;
    }
    return nullptr;
}

std::shared_ptr<TranslationProvider> getActiveTranslationProvider()
{
    const OUString aWanted(o3tl::trim(officecfg::Office::Linguistic::Translation::Provider::get()));
    if (!aWanted.isEmpty())
    {
        const OString aId = OUStringToOString(aWanted, RTL_TEXTENCODING_UTF8).toAsciiLowerCase();
        std::shared_ptr<TranslationProvider> pProvider = getTranslationProvider(aId);
        if (!pProvider)
        {
            SAL_WARN("linguistic", "getActiveTranslationProvider: no provider called " << aId);
            return nullptr;
        }
        if (!pProvider->isConfigured())
        {
            SAL_INFO("linguistic", "getActiveTranslationProvider: " << aId << " is not set up");
            return nullptr;
        }
        return pProvider;
    }

    for (const std::shared_ptr<TranslationProvider>& pProvider : getTranslationProviders())
    {
        if (pProvider->isConfigured())
            return pProvider;
    }
    return nullptr;
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
