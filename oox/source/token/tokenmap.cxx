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

#include <sal/config.h>

#include <array>

#include <oox/token/tokenmap.hxx>

#include <string.h>
#include <rtl/string.hxx>
#include <oox/token/tokens.hxx>

namespace oox {

namespace {
// include auto-generated Perfect_Hash
#if defined __clang__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wimplicit-fallthrough"
#if __has_warning("-Wdeprecated-register")
#pragma GCC diagnostic ignored "-Wdeprecated-register"
#endif
#endif
#include <tokenhash.inc>
#if defined __clang__
#pragma GCC diagnostic pop
#endif
} // namespace

static sal_Int32 getTokenPerfectHash(const char* pStr, sal_Int32 nLength)
{
    const struct xmltoken* pToken = Perfect_Hash::in_word_set( pStr, nLength );
    return pToken ? pToken->nToken : XML_TOKEN_INVALID;
}

css::uno::Sequence<sal_Int8> const& TokenMap::getUtf8TokenName(sal_Int32 nToken)
{
    static const auto saTokenNames = []()
    {
        static constexpr std::string_view sppcTokenNames[] = {
// include auto-generated C array with token names as C strings
#include <tokennames.inc>
        };
        static_assert(std::size(sppcTokenNames) == XML_TOKEN_COUNT);

        std::vector<css::uno::Sequence<sal_Int8>> aTokenNames;
        aTokenNames.reserve(std::size(sppcTokenNames));
        std::transform(
            std::begin(sppcTokenNames), std::end(sppcTokenNames), std::back_inserter(aTokenNames),
            [](auto aUtf8Token)
            {
                return css::uno::Sequence<sal_Int8>(
                    reinterpret_cast<const sal_Int8*>(aUtf8Token.data()), aUtf8Token.size());
            });
        return aTokenNames;
    }();

    SAL_WARN_IF(nToken < 0 || nToken >= XML_TOKEN_COUNT, "oox", "Wrong nToken parameter");
    if (0 <= nToken && nToken < XML_TOKEN_COUNT)
        return saTokenNames[nToken];
    static const css::uno::Sequence<sal_Int8> EMPTY_BYTE_SEQ;
    return EMPTY_BYTE_SEQ;
}


/** Returns the token identifier for a UTF8 string passed in pToken */
sal_Int32 TokenMap::getTokenFromUtf8(std::string_view token)
{
    static const auto snAlphaTokens = []()
    {
        std::array<sal_Int32, 26> nAlphaTokens{};
        for (char c = 'a'; c <= 'z'; c++)
            nAlphaTokens[c - 'a'] = getTokenPerfectHash(&c, 1);
        return nAlphaTokens;
    }();

    // 50% of OOXML tokens are primarily 1 lower-case character, a-z
    if (token.size() == 1)
    {
        char c = token[0];
        if (c >= 'a' && c <= 'z')
            return snAlphaTokens[c - 'a'];
    }
    return getTokenPerfectHash(token.data(), token.size());
}

} // namespace oox

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
