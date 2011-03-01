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
#include "precompiled_unotools.hxx"
#include <tools/debug.hxx>

#include "unotools/propertysetinfo.hxx"

using namespace ::utl;
using namespace ::rtl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::lang;

namespace utl
{
class PropertyMapImpl
{
public:
    PropertyMapImpl() throw();
    virtual ~PropertyMapImpl() throw();

    void add( PropertyMapEntry* pMap ) throw();
    void remove( const OUString& aName ) throw();

    Sequence< Property > getProperties() throw();

    const PropertyMap* getPropertyMap() const throw();

    Property getPropertyByName( const OUString& aName ) throw( UnknownPropertyException );
    sal_Bool hasPropertyByName( const OUString& aName ) throw();

private:
    PropertyMap maPropertyMap;
    Sequence< Property > maProperties;
};
}

PropertyMapImpl::PropertyMapImpl() throw()
{
}

PropertyMapImpl::~PropertyMapImpl() throw()
{
}

void PropertyMapImpl::add( PropertyMapEntry* pMap ) throw()
{
    while( pMap->mpName )
    {
        OUString aName( pMap->mpName, pMap->mnNameLen, RTL_TEXTENCODING_ASCII_US );

#ifdef DBG_UTIL
        PropertyMap::iterator aIter = maPropertyMap.find( aName );
        if( aIter != maPropertyMap.end() )
        {
            OSL_FAIL( "Warning: PropertyMapEntry added twice, possible error!" );
        }
#endif
        if( NULL == pMap->mpType )
        {
            OSL_FAIL( "No type in PropertyMapEntry!" );
            pMap->mpType = &::getCppuType((const sal_Int32*)0);
        }

        maPropertyMap[aName] = pMap;

        if( maProperties.getLength() )
            maProperties.realloc( 0 );

        pMap = &pMap[1];
    }
}

void PropertyMapImpl::remove( const OUString& aName ) throw()
{
    maPropertyMap.erase( aName );

    if( maProperties.getLength() )
        maProperties.realloc( 0 );
}

Sequence< Property > PropertyMapImpl::getProperties() throw()
{
    // maybe we have to generate the properties after
    // a change in the property map or at first call
    // to getProperties
    if( maProperties.getLength() != (sal_Int32)maPropertyMap.size() )
    {
        maProperties = Sequence< Property >( maPropertyMap.size() );
        Property* pProperties = maProperties.getArray();

        PropertyMap::iterator aIter = maPropertyMap.begin();
        const PropertyMap::iterator aEnd = maPropertyMap.end();
        while( aIter != aEnd )
        {
            PropertyMapEntry* pEntry = (*aIter).second;

            pProperties->Name = OUString( pEntry->mpName, pEntry->mnNameLen, RTL_TEXTENCODING_ASCII_US );
            pProperties->Handle = pEntry->mnWhich;
            pProperties->Type = *pEntry->mpType;
            pProperties->Attributes = pEntry->mnFlags;
            ++pProperties;
            ++aIter;
        }
    }

    return maProperties;
}

const PropertyMap* PropertyMapImpl::getPropertyMap() const throw()
{
    return &maPropertyMap;
}

Property PropertyMapImpl::getPropertyByName( const OUString& aName ) throw( UnknownPropertyException )
{
    PropertyMap::iterator aIter = maPropertyMap.find( aName );

    if( maPropertyMap.end() == aIter )
        throw UnknownPropertyException();

    PropertyMapEntry* pEntry = (*aIter).second;

    return Property( aName, pEntry->mnWhich, *pEntry->mpType, pEntry->mnFlags );
}

sal_Bool PropertyMapImpl::hasPropertyByName( const OUString& aName ) throw()
{
    return maPropertyMap.find( aName ) != maPropertyMap.end();
}

///////////////////////////////////////////////////////////////////////

PropertySetInfo::PropertySetInfo() throw()
{
    mpMap = new PropertyMapImpl();
}

PropertySetInfo::~PropertySetInfo() throw()
{
    delete mpMap;
}

void PropertySetInfo::add( PropertyMapEntry* pMap ) throw()
{
    mpMap->add( pMap );
}

void PropertySetInfo::remove( const rtl::OUString& aName ) throw()
{
    mpMap->remove( aName );
}

Sequence< ::com::sun::star::beans::Property > SAL_CALL PropertySetInfo::getProperties() throw(::com::sun::star::uno::RuntimeException)
{
    return mpMap->getProperties();
}

Property SAL_CALL PropertySetInfo::getPropertyByName( const ::rtl::OUString& aName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException)
{
    return mpMap->getPropertyByName( aName );
}

sal_Bool SAL_CALL PropertySetInfo::hasPropertyByName( const ::rtl::OUString& Name ) throw(::com::sun::star::uno::RuntimeException)
{
    return mpMap->hasPropertyByName( Name );
}

const PropertyMap* PropertySetInfo::getPropertyMap() const throw()
{
    return mpMap->getPropertyMap();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
