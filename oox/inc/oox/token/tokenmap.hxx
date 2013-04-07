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

#ifndef OOX_TOKEN_TOKENMAP_HXX
#define OOX_TOKEN_TOKENMAP_HXX

#include <vector>
#include <rtl/instance.hxx>
#include <rtl/ustring.hxx>
#include <com/sun/star/uno/Sequence.hxx>

namespace oox {

// ============================================================================

class TokenMap
{
public:
    explicit            TokenMap();
                        ~TokenMap();

    /** Returns the Unicode name of the passed token identifier. */
    OUString     getUnicodeTokenName( sal_Int32 nToken ) const;

    /** Returns the token identifier for the passed Unicode token name. */
    sal_Int32           getTokenFromUnicode( const OUString& rUnicodeName ) const;

    /** Returns the UTF8 name of the passed token identifier as byte sequence. */
    ::com::sun::star::uno::Sequence< sal_Int8 >
                        getUtf8TokenName( sal_Int32 nToken ) const;

    /** Returns the token identifier for the passed UTF8 token name. */
    sal_Int32           getTokenFromUtf8(
                            const ::com::sun::star::uno::Sequence< sal_Int8 >& rUtf8Name ) const;

private:
    struct TokenName
    {
        OUString maUniName;
        ::com::sun::star::uno::Sequence< sal_Int8 > maUtf8Name;
    };
    typedef ::std::vector< TokenName > TokenNameVector;

    TokenNameVector     maTokenNames;
};

// ============================================================================

struct StaticTokenMap : public ::rtl::Static< TokenMap, StaticTokenMap > {};

// ============================================================================

} // namespace oox

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
