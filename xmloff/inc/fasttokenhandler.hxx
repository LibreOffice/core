/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <com/sun/star/xml/sax/XFastTokenHandler.hpp>
#include <cppuhelper/implbase.hxx>
#include <sax/fastattribs.hxx>
#include <xmloff/token/tokens.hxx>
#include <sal/log.hxx>
#include <xmloff/dllapi.h>

namespace xmloff::token {

namespace TokenMap
{
/** Returns the UTF-8 name of the passed token identifier as byte sequence. */
css::uno::Sequence<sal_Int8> const& getUtf8TokenName(sal_Int32 nToken);

/** Returns the token identifier for a UTF-8 string */
sal_Int32 getTokenFromUtf8(std::string_view token);
};

class FastTokenHandler final :
    public sax_fastparser::FastTokenHandlerBase
{
public:
    explicit FastTokenHandler() = default;

    // XFastTokenHandler
    virtual css::uno::Sequence< sal_Int8 > SAL_CALL getUTF8Identifier( sal_Int32 nToken ) override;
    virtual sal_Int32 SAL_CALL getTokenFromUTF8( const css::uno::Sequence< sal_Int8 >& Identifier ) override;

    static const OUString& getIdentifier(sal_Int32 nToken);

    // Much faster direct C++ shortcut to the method that matters
    virtual sal_Int32 getTokenDirect(std::string_view token) const override;
};

} // namespace xmloff::token

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
