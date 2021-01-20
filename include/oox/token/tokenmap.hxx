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
#include <rtl/instance.hxx>
#include <rtl/ustring.hxx>
#include <sal/log.hxx>
#include <sal/types.h>

namespace oox {


class TokenMap
{
public:
    explicit            TokenMap();
                        ~TokenMap();

    /** Returns the token identifier for the passed Unicode token name. */
    static sal_Int32    getTokenFromUnicode( std::u16string_view rUnicodeName );

    /** Returns the UTF8 name of the passed token identifier as byte sequence. */
    css::uno::Sequence< sal_Int8 > const &
                        getUtf8TokenName( sal_Int32 nToken ) const
    {
        SAL_WARN_IF(nToken < 0 || nToken >= XML_TOKEN_COUNT, "oox", "Wrong nToken parameter");
        if (0 <= nToken && nToken < XML_TOKEN_COUNT)
            return maTokenNames[ nToken ];
        return EMPTY_BYTE_SEQ;
    }

    /** Returns the token identifier for the passed UTF8 token name. */
    sal_Int32           getTokenFromUtf8(
                            const css::uno::Sequence< sal_Int8 >& rUtf8Name ) const
    {
        return getTokenFromUTF8( reinterpret_cast< const char * >(
                                rUtf8Name.getConstArray() ),
                             rUtf8Name.getLength() );
    }

    /** Returns the token identifier for a UTF8 string passed in pToken */
    sal_Int32 getTokenFromUTF8( const char *pToken, sal_Int32 nLength ) const
    {
        // 50% of OOXML tokens are primarily 1 lower-case character, a-z
        if( nLength == 1)
        {
            char c = pToken[0];
            if (c >= 'a' && c <= 'z')
                return mnAlphaTokens[ c - 'a' ];
        }
        return getTokenPerfectHash( pToken, nLength );
    }

private:
    static sal_Int32 getTokenPerfectHash( const char *pToken, sal_Int32 nLength );
    static const css::uno::Sequence< sal_Int8 > EMPTY_BYTE_SEQ;

    std::vector< css::uno::Sequence< sal_Int8 > >
                        maTokenNames;
    sal_Int32           mnAlphaTokens[26];
};


struct StaticTokenMap : public ::rtl::Static< TokenMap, StaticTokenMap > {};


} // namespace oox

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
