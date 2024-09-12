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

#ifndef INCLUDED_OOX_TOKEN_TOKENMAP_HXX
#define INCLUDED_OOX_TOKEN_TOKENMAP_HXX

#include <sal/config.h>

#include <string_view>
#include <vector>

#include <com/sun/star/uno/Sequence.hxx>
#include <oox/token/tokens.hxx>
#include <oox/dllapi.h>
#include <rtl/ustring.hxx>
#include <sal/log.hxx>
#include <sal/types.h>

namespace oox {


namespace TokenMap
{
/** Returns the token identifier for a UTF8 string passed in pToken */
sal_Int32 getTokenFromUtf8(std::string_view token);

/** Returns the token identifier for the passed Unicode token name. */
inline sal_Int32 getTokenFromUnicode(std::u16string_view rUnicodeName)
{
    return getTokenFromUtf8(OUStringToOString(rUnicodeName, RTL_TEXTENCODING_UTF8));
}

/** Returns the UTF8 name of the passed token identifier as byte sequence. */
css::uno::Sequence<sal_Int8> const& getUtf8TokenName(sal_Int32 nToken);

/** Returns the name of the passed token identifier as OUString. */
inline OUString getUnicodeTokenName(sal_Int32 nToken)
{
    auto name = getUtf8TokenName(nToken);
    return OUString(reinterpret_cast<const char*>(name.getConstArray()), name.getLength(),
                    RTL_TEXTENCODING_UTF8);
}
};

} // namespace oox

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
