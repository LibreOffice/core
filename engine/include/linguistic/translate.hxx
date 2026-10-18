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

#include <string_view>
#include <vector>

namespace linguistic
{
/// Outcome of a DeepL translation request.
struct TranslateResult
{
    /// The translated text; empty when the request failed.
    OString aText;
    /// Describes the failure, e.g. the HTTP status and the message returned by
    /// the service; empty when the request succeeded.
    OString aError;
};

/// Translates the HTML fragment rData to rTargetLang with the DeepL API at
/// rAPIUrl, authenticating with rAuthKey.
LNG_DLLPUBLIC TranslateResult Translate(const OString& rTargetLang, const OString& rAPIUrl,
                                        std::string_view rAuthKey, const OString& rData);

/// Asks the DeepL API which languages text can be translated into. rAPIUrl is the
/// translate endpoint passed to Translate(), ending in /v2/translate; the list is
/// requested from the /v3/languages endpoint of the same host. Returns the BCP 47
/// codes, e.g. "de" or "en-GB", or nothing when rAPIUrl has a different form or the
/// request failed.
LNG_DLLPUBLIC std::vector<OString> GetTargetLanguages(const OString& rAPIUrl,
                                                      std::string_view rAuthKey);
} // namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
