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

namespace comphelper
{
class PropertyMapImpl
{
public:
    PropertyMapImpl() throw();
    virtual ~PropertyMapImpl() throw();

    void add(PropertyMapEntry const * pMap) throw();
    void remove( const OUString& aName ) throw();

    std::vector< Property > const & getProperties() throw();

    const PropertyMap& getPropertyMap() const throw() { return maPropertyMap;}

    Property getPropertyByName( const OUString& aName ) throw( UnknownPropertyException );
    bool hasPropertyByName( const OUString& aName ) throw();

private:
    PropertyMap maPropertyMap;
    std::vector< Property > maProperties;
};
}

PropertyMapImpl::PropertyMapImpl() throw()
{
}

PropertyMapImpl::~PropertyMapImpl() throw()
{
}

void PropertyMapImpl::add(PropertyMapEntry const * pMap) throw()
{
    while (!pMap->maName.isEmpty())
    {
        // check for duplicates
        assert(maPropertyMap.find(pMap->maName) == maPropertyMap.end());

        maPropertyMap[pMap->maName] = pMap;

        maProperties.clear();

        pMap = &pMap[1];
    }
}

void PropertyMapImpl::remove( const OUString& aName ) throw()
{
    maPropertyMap.erase( aName );

    maProperties.clear();
}

std::vector< Property > const & PropertyMapImpl::getProperties() throw()
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

    return maProperties;
}


Property PropertyMapImpl::getPropertyByName( const OUString& aName ) throw( UnknownPropertyException )
{
    PropertyMap::iterator aIter = maPropertyMap.find( aName );

    if( maPropertyMap.end() == aIter )
        throw UnknownPropertyException( aName );

    PropertyMapEntry const * pEntry = (*aIter).second;

    return Property( aName, pEntry->mnHandle, pEntry->maType, pEntry->mnAttributes );
}

bool PropertyMapImpl::hasPropertyByName( const OUString& aName ) throw()
{
    return maPropertyMap.find( aName ) != maPropertyMap.end();
}


PropertySetInfo::PropertySetInfo() throw()
    : mpImpl(new PropertyMapImpl)
{
}

PropertySetInfo::PropertySetInfo( PropertyMapEntry const * pMap ) throw()
    : mpImpl(new PropertyMapImpl)
{
    mpImpl->add( pMap );
}

PropertySetInfo::PropertySetInfo(uno::Sequence<beans::Property> const& rProps) throw()
    : mpImpl(new PropertyMapImpl)
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
    mpImpl->add(pEntries);
}

PropertySetInfo::~PropertySetInfo() throw()
{
}

void PropertySetInfo::add( PropertyMapEntry const * pMap ) throw()
{
    mpImpl->add( pMap );
}

void PropertySetInfo::remove( const OUString& aName ) throw()
{
    mpImpl->remove( aName );
}

Sequence< css::beans::Property > SAL_CALL PropertySetInfo::getProperties() throw(css::uno::RuntimeException, std::exception)
{
    return comphelper::containerToSequence(mpImpl->getProperties());
}

Property SAL_CALL PropertySetInfo::getPropertyByName( const OUString& aName ) throw(css::beans::UnknownPropertyException, css::uno::RuntimeException, std::exception)
{
    return mpImpl->getPropertyByName( aName );
}

sal_Bool SAL_CALL PropertySetInfo::hasPropertyByName( const OUString& Name ) throw(css::uno::RuntimeException, std::exception)
{
    return mpImpl->hasPropertyByName( Name );
}

const PropertyMap& PropertySetInfo::getPropertyMap() const throw()
{
    return mpImpl->getPropertyMap();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
