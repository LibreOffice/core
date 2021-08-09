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


#include <comphelper/propertysetinfo.hxx>
#include <comphelper/sequence.hxx>
#include <vector>


using namespace ::comphelper;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::lang;

void PropertySetInfo::addImpl(PropertyMapEntry const * pMap) noexcept
{
    while (!pMap->maName.isEmpty())
    {
        // check for duplicates
        assert(maPropertyMap.find(pMap->maName) == maPropertyMap.end());

        maPropertyMap[pMap->maName] = pMap;

        maProperties.clear();

        ++pMap;
    }
}

PropertySetInfo::PropertySetInfo() noexcept
{
}

PropertySetInfo::PropertySetInfo( PropertyMapEntry const * pMap ) noexcept
{
    while (!pMap->maName.isEmpty())
    {
        // check for duplicates
        assert(maPropertyMap.find(pMap->maName) == maPropertyMap.end());

        maPropertyMap[pMap->maName] = pMap;

        ++pMap;
    }
}

PropertySetInfo::PropertySetInfo(uno::Sequence<beans::Property> const& rProps) noexcept
{
    PropertyMapEntry * pEntries(new PropertyMapEntry[rProps.getLength() + 1]);
    PropertyMapEntry * pEntry(&pEntries[0]);
    for (auto const& it : rProps)
    {
        pEntry->maName = it.Name;
        pEntry->mnHandle = it.Handle;
        pEntry->maType = it.Type;
        pEntry->mnAttributes = it.Attributes;
        pEntry->mnMemberId = 0;
        ++pEntry;
    }
    pEntry->maName = OUString();

    addImpl(pEntries);
}

PropertySetInfo::~PropertySetInfo() noexcept
{
}

void PropertySetInfo::add( PropertyMapEntry const * pMap ) noexcept
{
    addImpl( pMap );
}

void PropertySetInfo::remove( const OUString& aName ) noexcept
{
    maPropertyMap.erase( aName );
    maProperties.clear();
}

Sequence< css::beans::Property > SAL_CALL PropertySetInfo::getProperties()
{
    // maybe we have to generate the properties after
    // a change in the property map or at first call
    // to getProperties
    if( maProperties.size() != maPropertyMap.size() )
    {
        maProperties.resize( maPropertyMap.size() );
        auto propIter = maProperties.begin();

        for( const auto& rProperty : maPropertyMap )
        {
            PropertyMapEntry const * pEntry = rProperty.second;

            propIter->Name = pEntry->maName;
            propIter->Handle = pEntry->mnHandle;
            propIter->Type = pEntry->maType;
            propIter->Attributes = pEntry->mnAttributes;

            ++propIter;
        }
    }
    return comphelper::containerToSequence(maProperties);
}

Property SAL_CALL PropertySetInfo::getPropertyByName( const OUString& aName )
{
    PropertyMap::iterator aIter = maPropertyMap.find( aName );

    if( maPropertyMap.end() == aIter )
        throw UnknownPropertyException( aName );

    PropertyMapEntry const * pEntry = (*aIter).second;

    return Property( aName, pEntry->mnHandle, pEntry->maType, pEntry->mnAttributes );
}

sal_Bool SAL_CALL PropertySetInfo::hasPropertyByName( const OUString& aName )
{
    return maPropertyMap.find( aName ) != maPropertyMap.end();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
