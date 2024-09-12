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

#include <algorithm>

#include <com/sun/star/xml/sax/SAXException.hpp>
#include <sax/fastattribs.hxx>
#include <utility>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml;
using namespace ::com::sun::star::xml::sax;
namespace sax_fastparser
{

// wastage to keep MSVC happy vs. an in-line {}
FastTokenHandlerBase::~FastTokenHandlerBase()
{
}

UnknownAttribute::UnknownAttribute( OUString aNamespaceURL, OString aName, OString value )
    : maNamespaceURL(std::move( aNamespaceURL )), maName(std::move( aName )), maValue(std::move( value ))
{
}

UnknownAttribute::UnknownAttribute( OString aName, OString value )
    : maName(std::move( aName )), maValue(std::move( value ))
{
}

void UnknownAttribute::FillAttribute( Attribute* pAttrib ) const
{
    if( pAttrib )
    {
        pAttrib->Name = OStringToOUString( maName, RTL_TEXTENCODING_UTF8 );
        pAttrib->NamespaceURL = maNamespaceURL;
        pAttrib->Value = OStringToOUString( maValue, RTL_TEXTENCODING_UTF8 );
    }
}

FastAttributeList::FastAttributeList( sax_fastparser::FastTokenHandlerBase *pTokenHandler)
: mpTokenHandler( pTokenHandler )
{
    // random initial size of buffer to store attribute values
    mnChunkLength = 58;
    mpChunk = static_cast<char *>(malloc( mnChunkLength ));
    maAttributeValues.push_back( 0 );
}

FastAttributeList::FastAttributeList( const css::uno::Reference< css::xml::sax::XFastAttributeList > & xAttrList )
{
    const auto& rOther = castToFastAttributeList(xAttrList);
    mpTokenHandler = rOther.mpTokenHandler;
    mpChunk = static_cast<char *>(malloc( rOther.mnChunkLength ));
    mnChunkLength = rOther.mnChunkLength;
    memcpy(mpChunk, rOther.mpChunk, rOther.mnChunkLength);
    maAttributeValues = rOther.maAttributeValues;
    maAttributeTokens = rOther.maAttributeTokens;
    maUnknownAttributes = rOther.maUnknownAttributes;
}

css::uno::Reference< ::css::util::XCloneable > FastAttributeList::createClone()
{
    return new FastAttributeList(this);
}

FastAttributeList::~FastAttributeList()
{
    free( mpChunk );
}

void FastAttributeList::clear()
{
    maAttributeTokens.clear();
    maAttributeValues.resize(1);
    assert(maAttributeValues[0] == 0);
    maUnknownAttributes.clear();
}

void FastAttributeList::add( sal_Int32 nToken, std::string_view value )
{
    assert(nToken != -1);
    assert(nToken != 0);
    assert(value.length() < SAL_MAX_INT32); // protect against absurd values
    maAttributeTokens.push_back( nToken );
    sal_Int32 nWritePosition = maAttributeValues.back();
    maAttributeValues.push_back( maAttributeValues.back() + value.length() + 1 );
    if (maAttributeValues.back() > mnChunkLength)
    {
        const sal_Int32 newLen = std::max(mnChunkLength * 2, maAttributeValues.back());
        auto p = static_cast<char*>(realloc(mpChunk, newLen));
        if (!p)
            throw std::bad_alloc();

        mnChunkLength = newLen;
        mpChunk = p;

    }
    memcpy(mpChunk + nWritePosition, value.data(), value.length());
    mpChunk[nWritePosition + value.length()] = '\0';
}

void FastAttributeList::add(sal_Int32 nToken, std::u16string_view sValue)
{
    add(nToken, OUStringToOString(sValue, RTL_TEXTENCODING_UTF8));
}

void FastAttributeList::addNS( sal_Int32 nNamespaceToken, sal_Int32 nToken, std::string_view rValue )
{
    sal_Int32 nCombinedToken = (nNamespaceToken << 16) | nToken;
    add( nCombinedToken, rValue );
}

void FastAttributeList::addNS(sal_Int32 nNamespaceToken, sal_Int32 nToken,
                                     std::u16string_view sValue)
{
    sal_Int32 nCombinedToken = (nNamespaceToken << 16) | nToken;
    add(nCombinedToken, sValue);
}

void FastAttributeList::addUnknown( const OUString& rNamespaceURL, const OString& rName, const OString& value )
{
    maUnknownAttributes.emplace_back( rNamespaceURL, rName, value );
}

void FastAttributeList::addUnknown( const OString& rName, const OString& value )
{
    maUnknownAttributes.emplace_back( rName, value );
}

void FastAttributeList::add( const css::uno::Reference<css::xml::sax::XFastAttributeList>& xAttrList )
{
    const auto& rOther = castToFastAttributeList(xAttrList);
    add(rOther);
}

void FastAttributeList::add( const FastAttributeList& rOther )
{
    for (size_t i=0; i < rOther.maAttributeTokens.size(); ++i)
        add(rOther.maAttributeTokens[i], rOther.getAsViewByIndex(i));
    for (const auto & i : rOther.maUnknownAttributes)
        addUnknown(i.maNamespaceURL, i.maName, i.maValue);
}

// XFastAttributeList
sal_Bool FastAttributeList::hasAttribute( ::sal_Int32 Token )
{
    for (sal_Int32 i : maAttributeTokens)
        if (i == Token)
            return true;

    return false;
}

sal_Int32 FastAttributeList::getValueToken( ::sal_Int32 Token )
{
    for (size_t i = 0, n = maAttributeTokens.size(); i < n; ++i)
        if (maAttributeTokens[i] == Token)
            return getValueTokenByIndex(i);

    throw SAXException("FastAttributeList::getValueToken: unknown token " + OUString::number(Token), nullptr, Any());
}

sal_Int32 FastAttributeList::getOptionalValueToken( ::sal_Int32 Token, ::sal_Int32 Default )
{
    for (size_t i = 0, n = maAttributeTokens.size(); i < n; ++i)
        if (maAttributeTokens[i] == Token)
            return getValueTokenByIndex(i);

    return Default;
}

// performance sensitive shortcuts to avoid allocation ...
bool FastAttributeList::getAsInteger( sal_Int32 nToken, sal_Int32 &rInt) const
{
    rInt = 0;
    for (size_t i = 0, n = maAttributeTokens.size(); i < n; ++i)
        if (maAttributeTokens[i] == nToken)
        {
            rInt = getAsIntegerByIndex(i);
            return true;
        }
    return false;
}

bool FastAttributeList::getAsDouble( sal_Int32 nToken, double &rDouble) const
{
    rDouble = 0.0;
    for (size_t i = 0, n = maAttributeTokens.size(); i < n; ++i)
        if (maAttributeTokens[i] == nToken)
        {
            rDouble = o3tl::toDouble(getAsViewByIndex(i));
            return true;
        }
    return false;
}

bool FastAttributeList::getAsView( sal_Int32 nToken, std::string_view& rPos ) const
{
    for (size_t i = 0, n = maAttributeTokens.size(); i < n; ++i)
    {
        if (maAttributeTokens[i] != nToken)
            continue;

        rPos = getAsViewByIndex(i);
        return true;
    }

    return false;
}

OUString FastAttributeList::getValue( ::sal_Int32 Token )
{
    for (size_t i = 0, n = maAttributeTokens.size(); i < n; ++i)
        if (maAttributeTokens[i] == Token)
            return getValueByIndex(i);

    throw SAXException("FastAttributeList::getValue: unknown token " + OUString::number(Token), nullptr, Any());
}

OUString FastAttributeList::getOptionalValue( ::sal_Int32 Token )
{
    for (size_t i = 0, n = maAttributeTokens.size(); i < n; ++i)
        if (maAttributeTokens[i] == Token)
            return getValueByIndex(i);

    return OUString();
}

sal_Int32 FastAttributeList::getValueTokenByIndex(sal_Int32 nTokenIndex) const
{
    return FastTokenHandlerBase::getTokenFromChars(mpTokenHandler, getAsViewByIndex(nTokenIndex));
}

Sequence< Attribute > FastAttributeList::getUnknownAttributes(  )
{
    auto nSize = maUnknownAttributes.size();
    if (nSize == 0)
        return {};
    Sequence< Attribute > aSeq( nSize );
    Attribute* pAttr = aSeq.getArray();
    for( const auto& rAttr : maUnknownAttributes )
        rAttr.FillAttribute( pAttr++ );
    return aSeq;
}
Sequence< FastAttribute > FastAttributeList::getFastAttributes(  )
{
    Sequence< FastAttribute > aSeq( maAttributeTokens.size() );
    FastAttribute* pAttr = aSeq.getArray();
    for (size_t i = 0, n = maAttributeTokens.size(); i < n; ++i)
    {
        pAttr->Token = maAttributeTokens[i];
        pAttr->Value = getValueByIndex(i);
        pAttr++;
    }
    return aSeq;
}

FastAttributeList::FastAttributeIter FastAttributeList::find( sal_Int32 nToken ) const
{
    for (size_t i = 0, n = maAttributeTokens.size(); i < n; ++i)
        if( maAttributeTokens[i] == nToken )
            return FastAttributeIter(*this, i);
    return end();
}

sal_Int32 FastTokenHandlerBase::getTokenFromChars(
        const FastTokenHandlerBase *pTokenHandler,
        std::string_view token )
{
    return pTokenHandler->getTokenDirect(token);
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
