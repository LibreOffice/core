/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <config_features.h>

#include <linguistic/translate.hxx>

#include <memory>

namespace linguistic::http
{
struct Response;
}

namespace linguistic::translate
{
#if HAVE_FEATURE_CURL
/// What went wrong with a failed request, for the user: the transfer's own
/// words or the HTTP status, and the service's message when its error body
/// has one under pMessageKey, e.g. "HTTP 403: Authorization failure".
OString describeFailure(const http::Response& rResponse, const char* pMessageKey);

/// The DeepL API, configured under Linguistic/Translation/Deepl.
std::shared_ptr<TranslationProvider> createDeeplProvider();
/// A LibreTranslate server, configured under Linguistic/Translation/LibreTranslate.
std::shared_ptr<TranslationProvider> createLibreTranslateProvider();
#endif
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
