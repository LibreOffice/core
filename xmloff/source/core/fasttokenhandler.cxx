/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <fasttokenhandler.hxx>

#include <xmloff/token/tokens.hxx>

namespace xmloff {

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

namespace token {

using namespace css;

static sal_Int32 getTokenPerfectHash(const char* pStr, sal_Int32 nLength)
{
    const struct xmltoken* pToken = Perfect_Hash::in_word_set(pStr, nLength);
    return pToken ? pToken->nToken : xmloff::XML_TOKEN_INVALID;
}

static const std::pair<css::uno::Sequence<sal_Int8>, OUString>& getNames(sal_Int32 nToken)
{
    static const auto saTokenNames = []()
    {
        static constexpr std::string_view sppcTokenNames[] = {
#include <tokennames.inc>
        };
        static_assert(std::size(sppcTokenNames) == XML_TOKEN_COUNT);

        std::vector<std::pair<css::uno::Sequence<sal_Int8>, OUString>> names;
        names.reserve(std::size(sppcTokenNames));
        std::transform(std::begin(sppcTokenNames), std::end(sppcTokenNames),
                       std::back_inserter(names),
                       [](auto token)
                       {
                           return std::make_pair(
                               css::uno::Sequence<sal_Int8>(
                                   reinterpret_cast<const sal_Int8*>(token.data()), token.size()),
                               OStringToOUString(token, RTL_TEXTENCODING_UTF8));
                       });
        return names;
    }();

    SAL_WARN_IF(nToken < 0 || nToken >= XML_TOKEN_COUNT, "xmloff", "Wrong nToken parameter");
    if (0 <= nToken && nToken < XML_TOKEN_COUNT)
        return saTokenNames[nToken];
    static const std::pair<css::uno::Sequence<sal_Int8>, OUString> EMPTY;
    return EMPTY;
}

css::uno::Sequence<sal_Int8> const& TokenMap::getUtf8TokenName(sal_Int32 nToken)
{
    return getNames(nToken).first;
}

sal_Int32 TokenMap::getTokenFromUtf8(std::string_view token)
{
    return getTokenPerfectHash(token.data(), token.size());
}

// XFastTokenHandler
uno::Sequence< sal_Int8 > FastTokenHandler::getUTF8Identifier( sal_Int32 nToken )
{
    return TokenMap::getUtf8TokenName( nToken );
}

const OUString& FastTokenHandler::getIdentifier(sal_Int32 nToken)
{
    return getNames(nToken).second;
}

sal_Int32 FastTokenHandler::getTokenFromUTF8( const uno::Sequence< sal_Int8 >& rIdentifier )
{
    return TokenMap::getTokenFromUtf8(std::string_view(
        reinterpret_cast<const char*>(rIdentifier.getConstArray()), rIdentifier.getLength()));
}

// Much faster direct C++ shortcut
sal_Int32 FastTokenHandler::getTokenDirect(std::string_view token) const
{
    return TokenMap::getTokenFromUtf8(token);
}

} // namespace token
} // namespace xmloff

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
