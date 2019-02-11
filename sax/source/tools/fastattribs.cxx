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
#include <com/sun/star/xml/sax/XFastTokenHandler.hpp>
#include <sax/fastattribs.hxx>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml;
using namespace ::com::sun::star::xml::sax;
namespace sax_fastparser
{

// wastage to keep MSVC happy vs. an in-line {}
FastTokenHandlerBase::~FastTokenHandlerBase()
{
}

UnknownAttribute::UnknownAttribute( const OUString& rNamespaceURL, const OString& rName, const OString& value )
    : maNamespaceURL( rNamespaceURL ), maName( rName ), maValue( value )
{
}

UnknownAttribute::UnknownAttribute( const OString& rName, const OString& value )
    : maName( rName ), maValue( value )
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

FastAttributeList::FastAttributeList( const css::uno::Reference< css::xml::sax::XFastTokenHandler >& xTokenHandler,
                                      sax_fastparser::FastTokenHandlerBase *pTokenHandler)
: mxTokenHandler( xTokenHandler ),
  mpTokenHandler( pTokenHandler )
{
    // random initial size of buffer to store attribute values
    mnChunkLength = 58;
    mpChunk = static_cast<sal_Char *>(malloc( mnChunkLength ));
    maAttributeValues.push_back( 0 );
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

void FastAttributeList::add( sal_Int32 nToken, const sal_Char* pValue, size_t nValueLength )
{
    assert(nToken != -1);
    maAttributeTokens.push_back( nToken );
    sal_Int32 nWritePosition = maAttributeValues.back();
    maAttributeValues.push_back( maAttributeValues.back() + nValueLength + 1 );
    if (maAttributeValues.back() > mnChunkLength)
    {
        const sal_Int32 newLen = std::max(mnChunkLength * 2, maAttributeValues.back());
        if (auto p = static_cast<sal_Char*>(realloc(mpChunk, newLen)))
        {
            mnChunkLength = newLen;
            mpChunk = p;
        }
        else
            throw std::bad_alloc();
    }
    strncpy(mpChunk + nWritePosition, pValue, nValueLength);
    mpChunk[nWritePosition + nValueLength] = '\0';
}

void FastAttributeList::add( sal_Int32 nToken, const sal_Char* pValue )
{
    add( nToken, pValue, strlen( pValue ));
}

void FastAttributeList::add( sal_Int32 nToken, const OString& rValue )
{
    add( nToken, rValue.getStr(), rValue.getLength() );
}

void FastAttributeList::addNS( sal_Int32 nNamespaceToken, sal_Int32 nToken, const OString& rValue )
{
    sal_Int32 nCombinedToken = (nNamespaceToken << 16) | nToken;
    add( nCombinedToken, rValue );
}

void FastAttributeList::addUnknown( const OUString& rNamespaceURL, const OString& rName, const OString& value )
{
    maUnknownAttributes.emplace_back( rNamespaceURL, rName, value );
}

void FastAttributeList::addUnknown( const OString& rName, const OString& value )
{
    maUnknownAttributes.emplace_back( rName, value );
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
    for (size_t i = 0; i < maAttributeTokens.size(); ++i)
        if (maAttributeTokens[i] == Token)
            return FastTokenHandlerBase::getTokenFromChars(
                       mxTokenHandler, mpTokenHandler,
                       getFastAttributeValue(i),
                       AttributeValueLength( i ) );

    throw SAXException();
}

sal_Int32 FastAttributeList::getOptionalValueToken( ::sal_Int32 Token, ::sal_Int32 Default )
{
    for (size_t i = 0; i < maAttributeTokens.size(); ++i)
        if (maAttributeTokens[i] == Token)
            return FastTokenHandlerBase::getTokenFromChars(
                       mxTokenHandler, mpTokenHandler,
                       getFastAttributeValue(i),
                       AttributeValueLength( i ) );

    return Default;
}

// performance sensitive shortcuts to avoid allocation ...
bool FastAttributeList::getAsInteger( sal_Int32 nToken, sal_Int32 &rInt) const
{
    rInt = 0;
    for (size_t i = 0; i < maAttributeTokens.size(); ++i)
        if (maAttributeTokens[i] == nToken)
        {
            rInt = rtl_str_toInt32( getFastAttributeValue(i), 10 );
            return true;
        }
    return false;
}

sal_Int32 FastAttributeList::getAsIntegerByIndex( sal_Int32 nTokenIndex ) const
{
    return rtl_str_toInt32( getFastAttributeValue(nTokenIndex), 10 );
}

bool FastAttributeList::getAsDouble( sal_Int32 nToken, double &rDouble) const
{
    rDouble = 0.0;
    for (size_t i = 0; i < maAttributeTokens.size(); ++i)
        if (maAttributeTokens[i] == nToken)
        {
            rDouble = rtl_str_toDouble( getFastAttributeValue(i) );
            return true;
        }
    return false;
}

bool FastAttributeList::getAsChar( sal_Int32 nToken, const char*& rPos ) const
{
    for (size_t i = 0, n = maAttributeTokens.size(); i < n; ++i)
    {
        if (maAttributeTokens[i] != nToken)
            continue;

        sal_Int32 nOffset = maAttributeValues[i];
        rPos = mpChunk + nOffset;
        return true;
    }

    return false;
}

const char* FastAttributeList::getAsCharByIndex( sal_Int32 nTokenIndex ) const
{
    sal_Int32 nOffset = maAttributeValues[nTokenIndex];
    return mpChunk + nOffset;
}

OUString FastAttributeList::getValue( ::sal_Int32 Token )
{
    for (size_t i = 0; i < maAttributeTokens.size(); ++i)
        if (maAttributeTokens[i] == Token)
            return OUString( getFastAttributeValue(i), AttributeValueLength(i), RTL_TEXTENCODING_UTF8 );

    throw SAXException();
}

OUString FastAttributeList::getValueByIndex( ::sal_Int32 nTokenIndex ) const
{
    return OUString( getFastAttributeValue(nTokenIndex), AttributeValueLength(nTokenIndex), RTL_TEXTENCODING_UTF8 );
}

OUString FastAttributeList::getOptionalValue( ::sal_Int32 Token )
{
    for (size_t i = 0; i < maAttributeTokens.size(); ++i)
        if (maAttributeTokens[i] == Token)
            return OUString( getFastAttributeValue(i), AttributeValueLength(i), RTL_TEXTENCODING_UTF8 );

    return OUString();
}
Sequence< Attribute > FastAttributeList::getUnknownAttributes(  )
{
    Sequence< Attribute > aSeq( maUnknownAttributes.size() );
    Attribute* pAttr = aSeq.getArray();
    for( auto& rAttr : maUnknownAttributes )
        rAttr.FillAttribute( pAttr++ );
    return aSeq;
}
Sequence< FastAttribute > FastAttributeList::getFastAttributes(  )
{
    Sequence< FastAttribute > aSeq( maAttributeTokens.size() );
    FastAttribute* pAttr = aSeq.getArray();
    for (size_t i = 0; i < maAttributeTokens.size(); ++i)
    {
        pAttr->Token = maAttributeTokens[i];
        pAttr->Value = OUString( getFastAttributeValue(i), AttributeValueLength(i), RTL_TEXTENCODING_UTF8 );
        pAttr++;
    }
    return aSeq;
}

const FastAttributeList::FastAttributeIter FastAttributeList::find( sal_Int32 nToken ) const
{
    for (size_t i = 0; i < maAttributeTokens.size(); ++i)
        if( maAttributeTokens[i] == nToken )
            return FastAttributeIter(*this, i);
    return end();
}

sal_Int32 FastTokenHandlerBase::getTokenFromChars(
        const css::uno::Reference< css::xml::sax::XFastTokenHandler > &xTokenHandler,
        const FastTokenHandlerBase *pTokenHandler,
        const char *pToken, size_t nLen /* = 0 */ )
{
    sal_Int32 nRet;

    if( !nLen )
        nLen = strlen( pToken );

    if( pTokenHandler )
        nRet = pTokenHandler->getTokenDirect( pToken, static_cast<sal_Int32>(nLen) );
    else
    {
        // heap allocate, copy & then free
        Sequence< sal_Int8 > aSeq( reinterpret_cast<sal_Int8 const *>(pToken), nLen );
        nRet = xTokenHandler->getTokenFromUTF8( aSeq );
    }

    return nRet;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
