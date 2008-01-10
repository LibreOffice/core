/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: fastattribs.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2008-01-10 12:52:07 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#include <algorithm>
#include <boost/bind.hpp>

#include "fastattribs.hxx"

using ::rtl::OUString;
using ::rtl::OString;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml;
using namespace ::com::sun::star::xml::sax;
namespace sax_fastparser
{

struct UnknownAttribute
{
    OUString maNamespaceURL;
    OString maName;
    OString maValue;

    UnknownAttribute( const OUString& rNamespaceURL, const OString& rName, const OString& rValue )
        : maNamespaceURL( rNamespaceURL ), maName( rName ), maValue( rValue ) {}

    UnknownAttribute( const OString& rName, const OString& rValue )
        : maName( rName ), maValue( rValue ) {}

/*
    UnknownAttribute( const UnknownAttribute& r )
        : maNamespaceURL( r.maNamespaceURL ), maName( r.maName ), maValue( r.maValue ) {}

    const UnknownAttribute& operator=( const UnknownAttribute& r )
    {
        maNamespaceURL = r.maNamespaceURL;
        maName = r.maName;
        maValue = r.maValue;
        return *this;
    }
*/
    void FillAttribute( Attribute* pAttrib ) const
    {
        if( pAttrib )
        {
            pAttrib->Name = OStringToOUString( maName, RTL_TEXTENCODING_UTF8 );
            pAttrib->NamespaceURL = maNamespaceURL;
            pAttrib->Value = OStringToOUString( maValue, RTL_TEXTENCODING_UTF8 );
        }
    }
};

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
    std::for_each( maUnknownAttributes.begin(), maUnknownAttributes.end(), bind(&UnknownAttribute::FillAttribute, _1, pAttr++ ) );
    return aSeq;
}

}
