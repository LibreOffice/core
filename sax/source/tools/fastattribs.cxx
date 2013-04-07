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
    maLastIter = maAttributes.end();
}

FastAttributeList::~FastAttributeList()
{
}

void FastAttributeList::clear()
{
    maAttributes.clear();
    maUnknownAttributes.clear();
    maLastIter = maAttributes.end();
}

void FastAttributeList::add( sal_Int32 nToken, const OString& rValue )
{
    maAttributes[nToken] = rValue;
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
    maLastIter = maAttributes.find( Token );
    return ( maLastIter != maAttributes.end() ) ? sal_True : sal_False;
}

sal_Int32 FastAttributeList::getValueToken( ::sal_Int32 Token ) throw (SAXException, RuntimeException)
{
    if( ( maLastIter == maAttributes.end() ) || ( ( *maLastIter ).first != Token ) )
        maLastIter = maAttributes.find( Token );

    if( maLastIter == maAttributes.end() )
        throw SAXException();

    Sequence< sal_Int8 > aSeq( (sal_Int8*)(*maLastIter).second.getStr(), (*maLastIter).second.getLength() ) ;
    return mxTokenHandler->getTokenFromUTF8( aSeq );
}

sal_Int32 FastAttributeList::getOptionalValueToken( ::sal_Int32 Token, ::sal_Int32 Default ) throw (RuntimeException)
{
    if( ( maLastIter == maAttributes.end() ) || ( ( *maLastIter ).first != Token ) )
        maLastIter = maAttributes.find( Token );

    if( maLastIter == maAttributes.end() )
        return Default;

    Sequence< sal_Int8 > aSeq( (sal_Int8*)(*maLastIter).second.getStr(), (*maLastIter).second.getLength() ) ;
    return mxTokenHandler->getTokenFromUTF8( aSeq );
}

OUString FastAttributeList::getValue( ::sal_Int32 Token ) throw (SAXException, RuntimeException)
{
    if( ( maLastIter == maAttributes.end() ) || ( ( *maLastIter ).first != Token ) )
        maLastIter = maAttributes.find( Token );

    if( maLastIter == maAttributes.end() )
        throw SAXException();

    return OStringToOUString( (*maLastIter).second, RTL_TEXTENCODING_UTF8 );
}

OUString FastAttributeList::getOptionalValue( ::sal_Int32 Token ) throw (RuntimeException)
{
    if( ( maLastIter == maAttributes.end() ) || ( ( *maLastIter ).first != Token ) )
        maLastIter = maAttributes.find( Token );

    OUString aRet;
    if( maLastIter != maAttributes.end() )
        aRet = OStringToOUString( (*maLastIter).second, RTL_TEXTENCODING_UTF8 );

    return aRet;
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
    Sequence< FastAttribute > aSeq( maAttributes.size() );
    FastAttribute* pAttr = aSeq.getArray();
    FastAttributeMap::iterator fastAttrIter = maAttributes.begin();
    for(; fastAttrIter != maAttributes.end(); ++fastAttrIter )
    {
        pAttr->Token = fastAttrIter->first;
        pAttr->Value = OStringToOUString( fastAttrIter->second, RTL_TEXTENCODING_UTF8 );
        pAttr++;
    }
    return aSeq;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
