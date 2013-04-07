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

#include <comphelper/MasterPropertySetInfo.hxx>
#include <comphelper/TypeGeneration.hxx>

using ::comphelper::PropertyInfo;
using ::comphelper::GenerateCppuType;
using ::comphelper::MasterPropertySetInfo;
using ::com::sun::star::uno::Any;
using ::com::sun::star::uno::Type;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::XInterface;
using ::com::sun::star::uno::RuntimeException;
using ::com::sun::star::beans::Property;
using ::com::sun::star::beans::XPropertySetInfo;
using ::com::sun::star::beans::UnknownPropertyException;

MasterPropertySetInfo::MasterPropertySetInfo( PropertyInfo* pMap )
    throw()
{
    add ( pMap );
}

MasterPropertySetInfo::~MasterPropertySetInfo()
    throw()
{
    PropertyDataHash::iterator aEnd = maMap.end(), aIter = maMap.begin();
    while (aIter != aEnd )
    {
        delete (*aIter).second;
        ++aIter;
    }
}

void MasterPropertySetInfo::add( PropertyInfo* pMap, sal_Int32 nCount, sal_uInt8 nMapId )
    throw()
{
    // nCount < 0   => add all
    // nCount == 0  => add nothing
    // nCount > 0   => add at most nCount entries
    if( maProperties.getLength() )
        maProperties.realloc( 0 );

    for ( ; pMap->mpName && ( ( nCount < 0 ) || ( nCount > 0 ) ); --nCount, ++pMap )
    {
        OUString aName( pMap->mpName, pMap->mnNameLen, RTL_TEXTENCODING_ASCII_US );

#ifdef DBG_UTIL
        PropertyDataHash::iterator aIter = maMap.find( aName );
        if( aIter != maMap.end() )
            OSL_FAIL( "Warning: PropertyInfo added twice, possible error!");
#endif
        maMap[aName] = new PropertyData ( nMapId, pMap );
    }
}

void MasterPropertySetInfo::add( PropertyInfoHash &rHash, sal_uInt8 nMapId )
    throw()
{
    if( maProperties.getLength() )
        maProperties.realloc( 0 );
    PropertyInfoHash::iterator aIter = rHash.begin(), aEnd = rHash.end();

    while ( aIter != aEnd )
    {
#ifdef DBG_UTIL
        PropertyDataHash::iterator aDebugIter = maMap.find( (*aIter).first );
        if( aDebugIter != maMap.end() )
            OSL_FAIL( "Warning: PropertyInfo added twice, possible error!");
#endif
        maMap[(*aIter).first] = new PropertyData ( nMapId, (*aIter).second );
        ++aIter;
    }
}

Sequence< ::Property > SAL_CALL MasterPropertySetInfo::getProperties()
    throw(::com::sun::star::uno::RuntimeException)
{
    sal_Int32 nSize = maMap.size();
    if( maProperties.getLength() != nSize )
    {
        maProperties.realloc ( nSize );
        Property* pProperties = maProperties.getArray();

        for (PropertyDataHash::const_iterator aIter(maMap.begin()), aEnd(maMap.end()) ; aIter != aEnd; ++aIter, ++pProperties)
        {
            PropertyInfo* pInfo = (*aIter).second->mpInfo;

            pProperties->Name = OUString( pInfo->mpName, pInfo->mnNameLen, RTL_TEXTENCODING_ASCII_US );
            pProperties->Handle = pInfo->mnHandle;
            const Type* pType;
            GenerateCppuType ( pInfo->meCppuType, pType);
            pProperties->Type = *pType;
            pProperties->Attributes = pInfo->mnAttributes;
        }
    }
    return maProperties;
}

Property SAL_CALL MasterPropertySetInfo::getPropertyByName( const OUString& rName )
    throw(::UnknownPropertyException, ::com::sun::star::uno::RuntimeException)
{
    PropertyDataHash::iterator aIter = maMap.find( rName );

    if ( maMap.end() == aIter )
        throw UnknownPropertyException( rName, *this );

    PropertyInfo *pInfo = (*aIter).second->mpInfo;
    Property aProperty;
    aProperty.Name   = OUString( pInfo->mpName, pInfo->mnNameLen, RTL_TEXTENCODING_ASCII_US );
    aProperty.Handle = pInfo->mnHandle;
    const Type* pType;
    GenerateCppuType ( pInfo->meCppuType, pType );
    aProperty.Type = *pType;

    aProperty.Attributes = pInfo->mnAttributes;
    return aProperty;
}

sal_Bool SAL_CALL MasterPropertySetInfo::hasPropertyByName( const OUString& rName )
    throw(::com::sun::star::uno::RuntimeException)
{
    return static_cast < sal_Bool > ( maMap.find ( rName ) != maMap.end() );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
