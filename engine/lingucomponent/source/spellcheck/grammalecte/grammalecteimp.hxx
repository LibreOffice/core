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

// The Grammalecte server, a French grammar checker reached over HTTP: a
// form-encoded POST to /gc_text/fr with the text and the options, answered
// with one entry per line of the text that has something to say.
class GrammalecteChecker final : public lingucomponent::remote::CheckerBase
{
public:
    GrammalecteChecker();

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
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
