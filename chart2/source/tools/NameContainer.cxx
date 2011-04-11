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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_chart2.hxx"

#include "NameContainer.hxx"

/*
//SvXMLUnitConverter
#include <xmloff/xmluconv.hxx>
*/
#include <com/sun/star/uno/Any.hxx>

using namespace ::com::sun::star;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::Any;
using ::rtl::OUString;


//.............................................................................
namespace chart
{
//.............................................................................

uno::Reference< container::XNameContainer > createNameContainer(
        const ::com::sun::star::uno::Type& rType, const rtl::OUString& rServicename, const rtl::OUString& rImplementationName )
{
    return new NameContainer( rType, rServicename, rImplementationName );
}

NameContainer::NameContainer( const ::com::sun::star::uno::Type& rType, const OUString& rServicename, const OUString& rImplementationName )
    : m_aType( rType )
    , m_aServicename( rServicename )
    , m_aImplementationName( rImplementationName )
    , m_aMap()
{
}

NameContainer::NameContainer(
    const NameContainer & rOther )
    : impl::NameContainer_Base()
    , m_aType( rOther.m_aType )
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
    for( tContentMap::iterator aIter = m_aMap.begin(); aIter != m_aMap.end(), nN < nCount; ++aIter, ++nN )
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
