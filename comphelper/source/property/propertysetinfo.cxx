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


using namespace ::comphelper;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;

PropertySetInfo::PropertySetInfo() noexcept
{
}

PropertySetInfo::PropertySetInfo( std::span<const PropertyMapEntry> pMap ) noexcept
{
    maPropertyMap.reserve(pMap.size());
    for (const auto & rEntry : pMap)
    {
        // check for duplicates
        assert(maPropertyMap.find(rEntry.maName) == maPropertyMap.end());
        // Make sure there are no accidental empty entries left at the end of the array from
        // when this method used to take a empty-terminated array.
        assert(!rEntry.maName.isEmpty());

        maPropertyMap.emplace(rEntry.maName, &rEntry);
    }
}

PropertySetInfo::~PropertySetInfo() noexcept
{
}

void PropertySetInfo::add( std::span<PropertyMapEntry const> pMap ) noexcept
{
    maPropertyMap.reserve(maPropertyMap.size() + pMap.size());
    for (const auto & rEntry : pMap)
    {
        // check for duplicates
        assert(maPropertyMap.find(rEntry.maName) == maPropertyMap.end());
        // Make sure there are no accidental empty entries left at the end of the array from
        // when this method used to take a empty-terminated array.
        assert(!rEntry.maName.isEmpty());

        maPropertyMap.emplace(rEntry.maName, &rEntry);
    }

    // clear cache
    maProperties.realloc(0);
}

void PropertySetInfo::remove( const OUString& aName ) noexcept
{
    maPropertyMap.erase( aName );
    maProperties.realloc(0);
}

Sequence< css::beans::Property > SAL_CALL PropertySetInfo::getProperties()
{
    // maybe we have to generate the properties after
    // a change in the property map or at first call
    // to getProperties
    if( maProperties.size() != maPropertyMap.size() )
    {
        maProperties.realloc( maPropertyMap.size() );
        auto propIter = maProperties.getArray();

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
    return maProperties;
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
    return maPropertyMap.contains( aName );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
