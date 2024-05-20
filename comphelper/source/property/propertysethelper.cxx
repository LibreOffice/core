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
#include <comphelper/propertysethelper.hxx>
#include <osl/diagnose.h>
#include <rtl/ref.hxx>

#include <memory>
#include <utility>

using namespace ::comphelper;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::lang;

static PropertyMapEntry const * find( const rtl::Reference<PropertySetInfo>& mxInfo, const OUString& aName ) noexcept
{
    PropertyMap::const_iterator aIter = mxInfo->getPropertyMap().find( aName );

    if( mxInfo->getPropertyMap().end() != aIter )
        return (*aIter).second;
    else
        return nullptr;
}


PropertySetHelper::PropertySetHelper( rtl::Reference<comphelper::PropertySetInfo> xInfo ) noexcept
    : mxInfo(std::move(xInfo))
{
}

PropertySetHelper::~PropertySetHelper() noexcept
{
}

// XPropertySet
Reference< XPropertySetInfo > SAL_CALL PropertySetHelper::getPropertySetInfo(  )
{
    return mxInfo;
}

void SAL_CALL PropertySetHelper::setPropertyValue( const OUString& aPropertyName, const Any& aValue )
{
    PropertyMapEntry const * aEntries[2];
    aEntries[0] = find( mxInfo, aPropertyName );

    if( nullptr == aEntries[0] )
        throw UnknownPropertyException( aPropertyName, static_cast< XPropertySet* >( this ) );

    aEntries[1] = nullptr;

    _setPropertyValues( aEntries, &aValue );
}

Any SAL_CALL PropertySetHelper::getPropertyValue( const OUString& PropertyName )
{
    PropertyMapEntry const * aEntries[2];
    aEntries[0] = find( mxInfo, PropertyName );

    if( nullptr == aEntries[0] )
        throw UnknownPropertyException( PropertyName, static_cast< XPropertySet* >( this ) );

    aEntries[1] = nullptr;

    Any aAny;
    _getPropertyValues( aEntries, &aAny );

    return aAny;
}

void SAL_CALL PropertySetHelper::addPropertyChangeListener( const OUString&, const Reference< XPropertyChangeListener >& )
{
    // todo
}

void SAL_CALL PropertySetHelper::removePropertyChangeListener( const OUString&, const Reference< XPropertyChangeListener >& )
{
    // todo
}

void SAL_CALL PropertySetHelper::addVetoableChangeListener( const OUString&, const Reference< XVetoableChangeListener >& )
{
    // todo
}

void SAL_CALL PropertySetHelper::removeVetoableChangeListener( const OUString&, const Reference< XVetoableChangeListener >& )
{
    // todo
}

// XMultiPropertySet
void SAL_CALL PropertySetHelper::setPropertyValues( const Sequence< OUString >& rPropertyNames, const Sequence< Any >& rValues )
{
    const sal_Int32 nCount = rPropertyNames.getLength();

    if( nCount != rValues.getLength() )
        throw IllegalArgumentException(u"lengths do not match"_ustr, static_cast<XPropertySet*>(this), -1);

    if( !nCount )
        return;

    std::unique_ptr<PropertyMapEntry const *[]> pEntries(new PropertyMapEntry const *[nCount+1]);
    pEntries[nCount] = nullptr;

    for (sal_Int32 n = 0; n < nCount; n++)
    {
        pEntries[n] = find(mxInfo, rPropertyNames[n]);
        if (!pEntries[n])
            throw UnknownPropertyException(rPropertyNames[n], static_cast<XPropertySet*>(this));
    }

    _setPropertyValues(pEntries.get(), rValues.getConstArray());
}

Sequence< Any > SAL_CALL PropertySetHelper::getPropertyValues(const Sequence< OUString >& rPropertyNames)
{
    const sal_Int32 nCount = rPropertyNames.getLength();

    if( !nCount )
        return Sequence< Any >();

    std::unique_ptr<PropertyMapEntry const *[]> pEntries(new PropertyMapEntry const *[nCount+1]);

    for (sal_Int32 n = 0; n < nCount; n++)
    {
        pEntries[n] = find(mxInfo, rPropertyNames[n]);
        if (!pEntries[n])
            throw UnknownPropertyException(rPropertyNames[n], static_cast<XPropertySet*>(this));
    }

    pEntries[nCount] = nullptr;
    Sequence< Any > aValues(nCount);
    _getPropertyValues( pEntries.get(), aValues.getArray() );
    return aValues;

}

void SAL_CALL PropertySetHelper::addPropertiesChangeListener( const Sequence< OUString >&, const Reference< XPropertiesChangeListener >& )
{
    // todo
}

void SAL_CALL PropertySetHelper::removePropertiesChangeListener( const Reference< XPropertiesChangeListener >& )
{
    // todo
}

void SAL_CALL PropertySetHelper::firePropertiesChangeEvent( const Sequence< OUString >&, const Reference< XPropertiesChangeListener >& )
{
    // todo
}

// XPropertyState
PropertyState SAL_CALL PropertySetHelper::getPropertyState( const OUString& PropertyName )
{
    PropertyMapEntry const * aEntries[2];

    aEntries[0] = find( mxInfo, PropertyName );
    if( aEntries[0] == nullptr )
        throw UnknownPropertyException( PropertyName, static_cast< XPropertySet* >( this ) );

    aEntries[1] = nullptr;

    PropertyState aState(PropertyState_AMBIGUOUS_VALUE);
    _getPropertyStates( aEntries, &aState );

    return aState;
}

Sequence< PropertyState > SAL_CALL PropertySetHelper::getPropertyStates( const Sequence< OUString >& aPropertyName )
{
    const sal_Int32 nCount = aPropertyName.getLength();

    Sequence< PropertyState > aStates( nCount );

    if( nCount )
    {
        std::unique_ptr<PropertyMapEntry const *[]> pEntries(new PropertyMapEntry const *[nCount+1]);

        sal_Int32 n;
        for (n = 0; n < nCount; n++)
        {
            pEntries[n] = find(mxInfo, aPropertyName[n]);
            if (!pEntries[n])
                throw UnknownPropertyException(aPropertyName[n], static_cast<XPropertySet*>(this));
        }

        pEntries[nCount] = nullptr;

        _getPropertyStates(pEntries.get(), aStates.getArray());
    }

    return aStates;
}

void SAL_CALL PropertySetHelper::setPropertyToDefault( const OUString& PropertyName )
{
    PropertyMapEntry const *pEntry  = find(mxInfo, PropertyName );
    if( nullptr == pEntry )
        throw UnknownPropertyException( PropertyName, static_cast< XPropertySet* >( this ) );

    _setPropertyToDefault( pEntry );
}

Any SAL_CALL PropertySetHelper::getPropertyDefault( const OUString& aPropertyName )
{
    PropertyMapEntry const * pEntry = find(mxInfo, aPropertyName );
    if( nullptr == pEntry )
        throw UnknownPropertyException( aPropertyName, static_cast< XPropertySet* >( this ) );

    return _getPropertyDefault( pEntry );
}

void PropertySetHelper::_getPropertyStates(
        const comphelper::PropertyMapEntry**, PropertyState*)
{
    OSL_FAIL( "you have to implement this yourself!");
}

void
PropertySetHelper::_setPropertyToDefault(const comphelper::PropertyMapEntry*)
{
    OSL_FAIL( "you have to implement this yourself!");
}

Any PropertySetHelper::_getPropertyDefault(const comphelper::PropertyMapEntry*)
{
    OSL_FAIL( "you have to implement this yourself!");

    Any aAny;
    return aAny;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
