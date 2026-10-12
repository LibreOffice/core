/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <remotechecker.hxx>

// The Duden Korrekturserver REST API: a JSON POST to /check under the
// server's /api root, answered with "check-positions" and, when asked for,
// a "messages" map that explains the error codes.
class DudenChecker final : public lingucomponent::remote::CheckerBase
{
public:
    DudenChecker();

    // XServiceDisplayName
    OUString getServiceDisplayName(const css::lang::Locale& rLocale) override;

    // XServiceInfo
    OUString getImplementationName() override;

private:
    bool isEnabled() override;
    std::vector<OUString> configuredLocales() override;
    std::optional<lingucomponent::remote::Request>
    buildRequest(const OUString& rText, const css::lang::Locale& rLocale) override;
    std::vector<css::linguistic2::SingleProofreadingError>
    parseResponse(const std::string& rBody, const OUString& rText,
                  const css::lang::Locale& rLocale) override;
    cpo::uno::Sequence<css::beans::PropertyValue> probe() override;

    static lingucomponent::remote::Request makeRequest(std::u16string_view rPath);
    static std::vector<OUString> fetchNames(std::u16string_view rPath, const char* pNameKey);
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
