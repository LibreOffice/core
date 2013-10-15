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

#include <sax/fastattribs.hxx>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml;
using namespace ::com::sun::star::xml::sax;
namespace sax_fastparser
{

UnknownAttribute::UnknownAttribute( const OUString& rNamespaceURL, const OString& rName, const OString& rValue )
    : maNamespaceURL( rNamespaceURL ), maName( rName ), maValue( rValue )
{
}

UnknownAttribute::UnknownAttribute( const OString& rName, const OString& rValue )
    : maName( rName ), maValue( rValue )
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

FastAttributeList::FastAttributeList( const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastTokenHandler >& xTokenHandler )
: mxTokenHandler( xTokenHandler )
{
}

FastAttributeList::~FastAttributeList()
{
}

void FastAttributeList::clear()
{
    maAttributeTokens.clear();
    maAttributeValues.clear();
    maUnknownAttributes.clear();
}

void FastAttributeList::add( sal_Int32 nToken, const OString& rValue )
{
    maAttributeTokens.push_back( nToken );
    maAttributeValues.push_back( rValue );
}

void FastAttributeList::addUnknown( const OUString& rNamespaceURL, const OString& rName, const OString& rValue )
{
    maUnknownAttributes.push_back( UnknownAttribute( rNamespaceURL, rName, rValue ) );
}

void FastAttributeList::addUnknown( const OString& rName, const OString& rValue )
{
    maUnknownAttributes.push_back( UnknownAttribute( rName, rValue ) );
}

// XFastAttributeList
sal_Bool FastAttributeList::hasAttribute( ::sal_Int32 Token ) throw (RuntimeException)
{
    for (size_t i = 0; i < maAttributeTokens.size(); ++i)
        if (maAttributeTokens[i] == Token)
            return sal_True;

    return sal_False;
}

sal_Int32 FastAttributeList::getValueToken( ::sal_Int32 Token ) throw (SAXException, RuntimeException)
{
    for (size_t i = 0; i < maAttributeTokens.size(); ++i)
        if (maAttributeTokens[i] == Token)
        {
            Sequence< sal_Int8 > aSeq( (sal_Int8*) maAttributeValues[i].getStr(), maAttributeValues[i].getLength() );
            return mxTokenHandler->getTokenFromUTF8( aSeq );
        }

    throw SAXException();
}

sal_Int32 FastAttributeList::getOptionalValueToken( ::sal_Int32 Token, ::sal_Int32 Default ) throw (RuntimeException)
{
    for (size_t i = 0; i < maAttributeTokens.size(); ++i)
        if (maAttributeTokens[i] == Token)
        {
            Sequence< sal_Int8 > aSeq( (sal_Int8*) maAttributeValues[i].getStr(), maAttributeValues[i].getLength() );
            return mxTokenHandler->getTokenFromUTF8( aSeq );
        }

    return Default;
}

OUString FastAttributeList::getValue( ::sal_Int32 Token ) throw (SAXException, RuntimeException)
{
    for (size_t i = 0; i < maAttributeTokens.size(); ++i)
        if (maAttributeTokens[i] == Token)
            return OStringToOUString( maAttributeValues[i], RTL_TEXTENCODING_UTF8 );

    throw SAXException();
}

OUString FastAttributeList::getOptionalValue( ::sal_Int32 Token ) throw (RuntimeException)
{
    for (size_t i = 0; i < maAttributeTokens.size(); ++i)
        if (maAttributeTokens[i] == Token)
            return OStringToOUString( maAttributeValues[i], RTL_TEXTENCODING_UTF8 );

    return OUString();
}
Sequence< Attribute > FastAttributeList::getUnknownAttributes(  ) throw (RuntimeException)
{
    Sequence< Attribute > aSeq( maUnknownAttributes.size() );
    Attribute* pAttr = aSeq.getArray();
    for( UnknownAttributeList::iterator attrIter = maUnknownAttributes.begin(); attrIter != maUnknownAttributes.end(); ++attrIter )
        (*attrIter).FillAttribute( pAttr++ );
    return aSeq;
}
Sequence< FastAttribute > FastAttributeList::getFastAttributes(  ) throw (RuntimeException)
{
    Sequence< FastAttribute > aSeq( maAttributeTokens.size() );
    FastAttribute* pAttr = aSeq.getArray();
    for (size_t i = 0; i < maAttributeTokens.size(); ++i)
    {
        pAttr->Token = maAttributeTokens[i];
        pAttr->Value = OStringToOUString( maAttributeValues[i], RTL_TEXTENCODING_UTF8 );
        pAttr++;
    }
    return aSeq;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
