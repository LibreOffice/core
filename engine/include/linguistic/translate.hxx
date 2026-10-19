/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once
#include <linguistic/lngdllapi.h>
#include <rtl/string.hxx>
#include <rtl/ustring.hxx>

#include <chrono>
#include <memory>
#include <optional>
#include <string_view>
#include <vector>

namespace linguistic
{
/// Outcome of a translation request.
struct TranslateResult
{
    /// The translated text; empty when the request failed.
    OString aText;
    /// Describes the failure, e.g. the HTTP status and the message returned by
    /// the service; empty when the request succeeded.
    OString aError;
};

/// Something that turns an HTML fragment into the same fragment in another
/// language: a service the engine talks to over HTTP, like DeepL or a
/// LibreTranslate server. Each provider has a group of its own under
/// Linguistic/Translation in the configuration, which it reads on every call,
/// so a change in the configuration takes effect at once.
class LNG_DLLPUBLIC TranslationProvider
{
public:
    virtual ~TranslationProvider();

    /// The value of the Linguistic/Translation/Provider setting that picks
    /// this provider, and its name in the configuration-related JSON: "deepl"
    /// or "libretranslate".
    virtual OString getId() const = 0;

    /// The name shown to the user, e.g. "DeepL".
    virtual OUString getDisplayName() const = 0;

    /// Switched on in the configuration.
    virtual bool isEnabled() const = 0;

    /// Switched on and told what it needs: a server, and a key where the
    /// service wants one.
    virtual bool isConfigured() const = 0;

    /// Translates the HTML fragment rHtml to rTargetLang, a BCP 47 language
    /// tag like "de" or "en-GB", in whatever case; the provider spells it the
    /// way its service wants. A provider that waits for an answer while the
    /// main loop runs on watches rCancel, and returns an empty aText and an
    /// empty aError once it is set. The HTTP providers block the main loop
    /// for their request and cannot see it change.
    virtual TranslateResult translate(const OString& rHtml, const OString& rTargetLang,
                                      const bool& rCancel)
        = 0;

    /// The BCP 47 tags of the languages text can be translated into, e.g.
    /// "de" or "en-GB", as the service lists them. The service is asked once
    /// per configuration and the answer kept; after a failed request it is
    /// not asked again for a while, so a service that is down does not block
    /// every call. With bAskService false, only what is already known is
    /// returned.
    std::vector<OString> getTargetLanguages(bool bAskService = true);

protected:
    /// Asks the service for its target languages; empty when it cannot tell.
    virtual std::vector<OString> fetchTargetLanguages() = 0;

    /// What the language list depends on, e.g. the server and the account: a
    /// changed key means the service is asked anew.
    virtual OString getLanguageCacheKey() const = 0;

private:
    OString m_aCachedKey;
    std::vector<OString> m_aCachedLanguages;
    std::optional<std::chrono::steady_clock::time_point> m_oLastFailure;
};

/// Every provider there is, in the order the automatic choice tries them.
LNG_DLLPUBLIC const std::vector<std::shared_ptr<TranslationProvider>>& getTranslationProviders();

/// The provider with the id rId, or nothing.
LNG_DLLPUBLIC std::shared_ptr<TranslationProvider> getTranslationProvider(std::string_view rId);

/// The provider that translates: the one Linguistic/Translation/Provider
/// names when it is configured, and nothing when it is not (a provider that
/// is asked for is not silently swapped for another); with an empty setting,
/// the first configured one. Nothing when no provider is configured.
LNG_DLLPUBLIC std::shared_ptr<TranslationProvider> getActiveTranslationProvider();
} // namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
