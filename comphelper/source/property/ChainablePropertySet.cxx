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

#include <comphelper/ChainablePropertySet.hxx>
#include <comphelper/ChainablePropertySetInfo.hxx>
#include <comphelper/solarmutex.hxx>


#include <memory>

using namespace ::comphelper;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;

ChainablePropertySet::ChainablePropertySet( comphelper::ChainablePropertySetInfo* pInfo, comphelper::SolarMutex* pMutex )
    throw()
: mpMutex ( pMutex )
, mxInfo ( pInfo )
{
}

ChainablePropertySet::~ChainablePropertySet()
    throw()
{
}

// XPropertySet
Reference< XPropertySetInfo > SAL_CALL ChainablePropertySet::getPropertySetInfo(  )
{
    return mxInfo.get();
}

void SAL_CALL ChainablePropertySet::setPropertyValue( const OUString& rPropertyName, const Any& rValue )
{
    // acquire mutex in c-tor and releases it in the d-tor (exception safe!).
    std::unique_ptr< osl::Guard< comphelper::SolarMutex > > xMutexGuard;
    if (mpMutex)
        xMutexGuard.reset( new osl::Guard< comphelper::SolarMutex >(mpMutex) );

    PropertyInfoHash::const_iterator aIter = mxInfo->maMap.find ( rPropertyName );

    if( aIter == mxInfo->maMap.end())
        throw UnknownPropertyException( rPropertyName, static_cast< XPropertySet* >( this ) );

    _preSetValues();
    _setSingleValue( *((*aIter).second), rValue );
    _postSetValues();
}

Any SAL_CALL ChainablePropertySet::getPropertyValue( const OUString& rPropertyName )
{
    // acquire mutex in c-tor and releases it in the d-tor (exception safe!).
    std::unique_ptr< osl::Guard< comphelper::SolarMutex > > xMutexGuard;
    if (mpMutex)
        xMutexGuard.reset( new osl::Guard< comphelper::SolarMutex >(mpMutex) );

    PropertyInfoHash::const_iterator aIter = mxInfo->maMap.find ( rPropertyName );

    if( aIter == mxInfo->maMap.end())
        throw UnknownPropertyException( rPropertyName, static_cast< XPropertySet* >( this ) );

    Any aAny;
    _preGetValues ();
    _getSingleValue( *((*aIter).second), aAny );
    _postGetValues ();

    return aAny;
}

void SAL_CALL ChainablePropertySet::addPropertyChangeListener( const OUString&, const Reference< XPropertyChangeListener >& )
{
    // todo
}

void SAL_CALL ChainablePropertySet::removePropertyChangeListener( const OUString&, const Reference< XPropertyChangeListener >& )
{
    // todo
}

void SAL_CALL ChainablePropertySet::addVetoableChangeListener( const OUString&, const Reference< XVetoableChangeListener >& )
{
    // todo
}

void SAL_CALL ChainablePropertySet::removeVetoableChangeListener( const OUString&, const Reference< XVetoableChangeListener >& )
{
    // todo
}

// XMultiPropertySet
void SAL_CALL ChainablePropertySet::setPropertyValues(const Sequence< OUString >& rPropertyNames, const Sequence< Any >& rValues)
{
    // acquire mutex in c-tor and releases it in the d-tor (exception safe!).
    std::unique_ptr< osl::Guard< comphelper::SolarMutex > > xMutexGuard;
    if (mpMutex)
        xMutexGuard.reset( new osl::Guard< comphelper::SolarMutex >(mpMutex) );

    const sal_Int32 nCount = rPropertyNames.getLength();

    if( nCount != rValues.getLength() )
        throw IllegalArgumentException();

    if( nCount )
    {
        _preSetValues();

        const Any * pAny = rValues.getConstArray();
        const OUString * pString = rPropertyNames.getConstArray();
        PropertyInfoHash::const_iterator aEnd = mxInfo->maMap.end(), aIter;

        for ( sal_Int32 i = 0; i < nCount; ++i, ++pString, ++pAny )
        {
            aIter = mxInfo->maMap.find ( *pString );
            if ( aIter == aEnd )
                throw RuntimeException( *pString, static_cast< XPropertySet* >( this ) );

            _setSingleValue ( *((*aIter).second), *pAny );
        }

        _postSetValues();
    }
}

Sequence< Any > SAL_CALL ChainablePropertySet::getPropertyValues(const Sequence< OUString >& rPropertyNames)
{
    // acquire mutex in c-tor and releases it in the d-tor (exception safe!).
    std::unique_ptr< osl::Guard< comphelper::SolarMutex > > xMutexGuard;
    if (mpMutex)
        xMutexGuard.reset( new osl::Guard< comphelper::SolarMutex >(mpMutex) );

    const sal_Int32 nCount = rPropertyNames.getLength();

    Sequence < Any > aValues ( nCount );

    if( nCount )
    {
        _preGetValues();

        Any * pAny = aValues.getArray();
        const OUString * pString = rPropertyNames.getConstArray();
        PropertyInfoHash::const_iterator aEnd = mxInfo->maMap.end(), aIter;

        for ( sal_Int32 i = 0; i < nCount; ++i, ++pString, ++pAny )
        {
            aIter = mxInfo->maMap.find ( *pString );
            if ( aIter == aEnd )
                throw RuntimeException( *pString, static_cast< XPropertySet* >( this ) );

            _getSingleValue ( *((*aIter).second), *pAny );
        }

        _postGetValues();
    }
    return aValues;
}

void SAL_CALL ChainablePropertySet::addPropertiesChangeListener( const Sequence< OUString >&, const Reference< XPropertiesChangeListener >& )
{
    // todo
}

void SAL_CALL ChainablePropertySet::removePropertiesChangeListener( const Reference< XPropertiesChangeListener >& )
{
    // todo
}

void SAL_CALL ChainablePropertySet::firePropertiesChangeEvent( const Sequence< OUString >&, const Reference< XPropertiesChangeListener >& )
{
    // todo
}

// XPropertyState
PropertyState SAL_CALL ChainablePropertySet::getPropertyState( const OUString& PropertyName )
{
    PropertyInfoHash::const_iterator aIter =  mxInfo->maMap.find( PropertyName );
    if( aIter == mxInfo->maMap.end())
        throw UnknownPropertyException( PropertyName, static_cast< XPropertySet* >( this ) );

    return PropertyState_AMBIGUOUS_VALUE;
}

Sequence< PropertyState > SAL_CALL ChainablePropertySet::getPropertyStates( const Sequence< OUString >& rPropertyNames )
{
    const sal_Int32 nCount = rPropertyNames.getLength();

    Sequence< PropertyState > aStates( nCount );
    if( nCount )
    {
        PropertyState * pState = aStates.getArray();
        const OUString * pString = rPropertyNames.getConstArray();
        PropertyInfoHash::const_iterator aEnd = mxInfo->maMap.end(), aIter;

        for ( sal_Int32 i = 0; i < nCount; ++i, ++pString, ++pState )
        {
            aIter = mxInfo->maMap.find ( *pString );
            if ( aIter == aEnd )
                throw UnknownPropertyException( *pString, static_cast< XPropertySet* >( this ) );
        }
    }
    return aStates;
}

void SAL_CALL ChainablePropertySet::setPropertyToDefault( const OUString& rPropertyName )
{
    PropertyInfoHash::const_iterator aIter = mxInfo->maMap.find ( rPropertyName );

    if( aIter == mxInfo->maMap.end())
        throw UnknownPropertyException( rPropertyName, static_cast< XPropertySet* >( this ) );
}

Any SAL_CALL ChainablePropertySet::getPropertyDefault( const OUString& rPropertyName )
{
    PropertyInfoHash::const_iterator aIter = mxInfo->maMap.find ( rPropertyName );

    if( aIter == mxInfo->maMap.end())
        throw UnknownPropertyException( rPropertyName, static_cast< XPropertySet* >( this ) );
    return Any();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
