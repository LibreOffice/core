/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

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
    ::rtl::OUString     getUnicodeTokenName( sal_Int32 nToken ) const;

    /** Returns the token identifier for the passed Unicode token name. */
    sal_Int32           getTokenFromUnicode( const ::rtl::OUString& rUnicodeName ) const;

    /** Returns the UTF8 name of the passed token identifier as byte sequence. */
    ::com::sun::star::uno::Sequence< sal_Int8 >
                        getUtf8TokenName( sal_Int32 nToken ) const;

    /** Returns the token identifier for the passed UTF8 token name. */
    sal_Int32           getTokenFromUtf8(
                            const ::com::sun::star::uno::Sequence< sal_Int8 >& rUtf8Name ) const;

private:
    struct TokenName
    {
        ::rtl::OUString maUniName;
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
