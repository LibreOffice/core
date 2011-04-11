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

#include <algorithm>
#include <boost/bind.hpp>

#include <sax/fastattribs.hxx>

using ::rtl::OUString;
using ::rtl::OString;
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
