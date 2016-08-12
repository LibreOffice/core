/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_XMLOFF_FASTTOKENHANDLER_HXX
#define INCLUDED_XMLOFF_FASTTOKENHANDLER_HXX

#include <com/sun/star/xml/sax/XFastTokenHandler.hpp>
#include <cppuhelper/implbase1.hxx>
#include <sax/fastattribs.hxx>
#include <xmloff/token/tokens.hxx>
#include <rtl/instance.hxx>
#include <xmloff/dllapi.h>

namespace xmloff {
namespace token {

class TokenMap
{
public:
    explicit TokenMap();
            ~TokenMap();

    /** Returns the token identifier for the passed Unicode token name. */
    sal_Int32 getTokenFromUnicode( const OUString& rUnicodeName ) const;

    /** Returns the UTF8 name of the passed token identifier as byte sequence. */
    css::uno::Sequence< sal_Int8 > getUtf8TokenName( sal_Int32 nToken ) const
    {
        SAL_WARN_IF(nToken < 0 || nToken >= XML_TOKEN_COUNT, "xmloff", "Wrong nToken parameter");
        if( 0 <= nToken && nToken < XML_TOKEN_COUNT )
            return maTokenNames[ nToken ];
        return css::uno::Sequence< sal_Int8 >();
    }

    /** Returns the token identifier for the passed UTF8 token name. */
    sal_Int32 getTokenFromUtf8( const css::uno::Sequence< sal_Int8 >& rUtf8Name ) const
    {
        return getTokenFromUTF8( reinterpret_cast< const char* >(
                    rUtf8Name.getConstArray() ), rUtf8Name.getLength() );
    }

    /** Returns the token identifier for a UTF8 string passed in pToken */
    sal_Int32 getTokenFromUTF8( const char *pToken, sal_Int32 nLength ) const
    {
        return getTokenPerfectHash( pToken, nLength );
    }

private:
    sal_Int32 getTokenPerfectHash( const char *pToken, sal_Int32 nLength ) const;

    std::vector< css::uno::Sequence< sal_Int8 > > maTokenNames;
};

struct StaticTokenMap : public rtl::Static< TokenMap, StaticTokenMap > {};

class XMLOFF_DLLPUBLIC FastTokenHandler : public cppu::WeakImplHelper1<
    css::xml::sax::XFastTokenHandler >,
    public sax_fastparser::FastTokenHandlerBase
{
public:
    explicit FastTokenHandler();
    virtual ~FastTokenHandler();

    // XFastTokenHandler
    virtual css::uno::Sequence< sal_Int8 > SAL_CALL getUTF8Identifier( sal_Int32 nToken )
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Int32 SAL_CALL getTokenFromUTF8( const css::uno::Sequence< sal_Int8 >& Identifier )
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // Much faster direct C++ shortcut to the method that matters
    virtual sal_Int32 getTokenDirect( const char *pToken, sal_Int32 nLength ) const SAL_OVERRIDE;

private:
    TokenMap& mrTokenMap;
};

} // namespace token
} // namespace xmloff

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
