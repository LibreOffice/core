/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: NameContainer.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-05-22 19:02:13 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_chart2.hxx"

#include "NameContainer.hxx"

/*
//SvXMLUnitConverter
#ifndef _XMLOFF_XMLUCONV_HXX
#include <xmloff/xmluconv.hxx>
#endif
*/

#ifndef _COM_SUN_STAR_UNO_ANY_HXX_
#include <com/sun/star/uno/Any.hxx>
#endif

using namespace ::com::sun::star;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::Any;
using ::rtl::OUString;


//.............................................................................
namespace chart
{
//.............................................................................

NameContainer::NameContainer( const ::com::sun::star::uno::Type& rType, const OUString& rServicename, const OUString& rImplementationName )
    : m_aType( rType )
    , m_aServicename( rServicename )
    , m_aImplementationName( rImplementationName )
    , m_aMap()
{
}

NameContainer::NameContainer(
    const NameContainer & rOther )
    : m_aType( rOther.m_aType )
    , m_aServicename( rOther.m_aServicename )
    , m_aImplementationName( rOther.m_aImplementationName )
    , m_aMap( rOther.m_aMap )
{
}

NameContainer::~NameContainer()
{
}

//XServiceInfo
OUString SAL_CALL NameContainer::getImplementationName()
    throw( ::com::sun::star::uno::RuntimeException )
{
    return m_aImplementationName;
}

sal_Bool SAL_CALL NameContainer::supportsService( const OUString& ServiceName )
    throw( ::com::sun::star::uno::RuntimeException )
{
    Sequence< OUString > aSNL = getSupportedServiceNames();
    const OUString* pArray = aSNL.getArray();
    for( sal_Int32 i = 0; i < aSNL.getLength(); i++ )
    {
        if( pArray[ i ] == ServiceName )
            return sal_True;
    }
    return sal_False;
}

Sequence< OUString > SAL_CALL NameContainer::getSupportedServiceNames()
    throw( ::com::sun::star::uno::RuntimeException )
{
    Sequence< OUString > aSNS( 1 );
    aSNS.getArray()[ 0 ] = m_aServicename;
    return aSNS;
}

//-----------------------------------------------------------------
//-----------------------------------------------------------------
//-----------------------------------------------------------------

// XNameContainer
void SAL_CALL NameContainer::insertByName( const OUString& rName, const Any& rElement )
    throw( lang::IllegalArgumentException, container::ElementExistException, lang::WrappedTargetException, uno::RuntimeException )
{
    if( m_aMap.find( rName ) != m_aMap.end() )
        throw container::ElementExistException();
    m_aMap.insert( tContentMap::value_type( rName, rElement ));
}



void SAL_CALL NameContainer::removeByName( const OUString& Name )
    throw( container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException)
{
    tContentMap::iterator aIt( m_aMap.find( Name ));
    if( aIt == m_aMap.end())
        throw container::NoSuchElementException();
    m_aMap.erase( aIt );
}

// XNameReplace
void SAL_CALL NameContainer::replaceByName( const OUString& rName, const Any& rElement )
    throw( lang::IllegalArgumentException, container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException )
{
    tContentMap::iterator aIt( m_aMap.find( rName ));
    if( aIt == m_aMap.end() )
        throw container::NoSuchElementException();
    aIt->second = rElement;
}

// XNameAccess
Any SAL_CALL NameContainer::getByName( const OUString& rName )
    throw( container::NoSuchElementException,  lang::WrappedTargetException, uno::RuntimeException)
{
    tContentMap::iterator aIter( m_aMap.find( rName ) );
    if( aIter == m_aMap.end() )
        throw container::NoSuchElementException();
    return aIter->second;
}

Sequence< OUString > SAL_CALL NameContainer::getElementNames()
    throw( uno::RuntimeException )
{
    sal_Int32 nCount = m_aMap.size();
    Sequence< OUString > aSeq(nCount);
    sal_Int32 nN = 0;
    for( tContentMap::iterator aIter = m_aMap.begin(); aIter != m_aMap.end(), nN < nCount; aIter++, nN++ )
        aSeq[nN]=aIter->first;
    return aSeq;
}

sal_Bool SAL_CALL NameContainer::hasByName( const OUString& rName )
    throw( uno::RuntimeException )
{
    return ( m_aMap.find( rName ) != m_aMap.end() );
}

// XElementAccess
sal_Bool SAL_CALL NameContainer::hasElements()
    throw( uno::RuntimeException )
{
    return ! m_aMap.empty();
}

uno::Type SAL_CALL NameContainer::getElementType()
    throw( uno::RuntimeException )
{
    return m_aType;
}

// XCloneable
uno::Reference< util::XCloneable > SAL_CALL NameContainer::createClone()
    throw ( uno::RuntimeException )
{
    return uno::Reference< util::XCloneable >( new NameContainer( *this ));
}

//.............................................................................
} //namespace chart
//.............................................................................
