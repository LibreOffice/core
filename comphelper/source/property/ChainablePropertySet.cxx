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

#include <osl/diagnose.h>

#include <memory>

using namespace ::comphelper;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;

ChainablePropertySet::ChainablePropertySet( comphelper::ChainablePropertySetInfo* pInfo, comphelper::SolarMutex* pMutex )
    throw()
: mpInfo ( pInfo )
, mpMutex ( pMutex )
, mxInfo ( pInfo )
{
}

ChainablePropertySet::~ChainablePropertySet()
    throw()
{
}

// XPropertySet
Reference< XPropertySetInfo > SAL_CALL ChainablePropertySet::getPropertySetInfo(  )
    throw(RuntimeException, std::exception)
{
    return mxInfo;
}

void SAL_CALL ChainablePropertySet::setPropertyValue( const OUString& rPropertyName, const Any& rValue )
    throw(UnknownPropertyException, PropertyVetoException, IllegalArgumentException, WrappedTargetException, RuntimeException, std::exception)
{
    // acquire mutex in c-tor and releases it in the d-tor (exception safe!).
    std::unique_ptr< osl::Guard< comphelper::SolarMutex > > xMutexGuard;
    if (mpMutex)
        xMutexGuard.reset( new osl::Guard< comphelper::SolarMutex >(mpMutex) );

    PropertyInfoHash::const_iterator aIter = mpInfo->maMap.find ( rPropertyName );

    if( aIter == mpInfo->maMap.end())
        throw UnknownPropertyException( rPropertyName, static_cast< XPropertySet* >( this ) );

    _preSetValues();
    _setSingleValue( *((*aIter).second), rValue );
    _postSetValues();
}

Any SAL_CALL ChainablePropertySet::getPropertyValue( const OUString& rPropertyName )
    throw(UnknownPropertyException, WrappedTargetException, RuntimeException, std::exception)
{
    // acquire mutex in c-tor and releases it in the d-tor (exception safe!).
    std::unique_ptr< osl::Guard< comphelper::SolarMutex > > xMutexGuard;
    if (mpMutex)
        xMutexGuard.reset( new osl::Guard< comphelper::SolarMutex >(mpMutex) );

    PropertyInfoHash::const_iterator aIter = mpInfo->maMap.find ( rPropertyName );

    if( aIter == mpInfo->maMap.end())
        throw UnknownPropertyException( rPropertyName, static_cast< XPropertySet* >( this ) );

    Any aAny;
    _preGetValues ();
    _getSingleValue( *((*aIter).second), aAny );
    _postGetValues ();

    return aAny;
}

void SAL_CALL ChainablePropertySet::addPropertyChangeListener( const OUString&, const Reference< XPropertyChangeListener >& )
    throw(UnknownPropertyException, WrappedTargetException, RuntimeException, std::exception)
{
    // todo
}

void SAL_CALL ChainablePropertySet::removePropertyChangeListener( const OUString&, const Reference< XPropertyChangeListener >& )
    throw(UnknownPropertyException, WrappedTargetException, RuntimeException, std::exception)
{
    // todo
}

void SAL_CALL ChainablePropertySet::addVetoableChangeListener( const OUString&, const Reference< XVetoableChangeListener >& )
    throw(UnknownPropertyException, WrappedTargetException, RuntimeException, std::exception)
{
    // todo
}

void SAL_CALL ChainablePropertySet::removeVetoableChangeListener( const OUString&, const Reference< XVetoableChangeListener >& )
    throw(UnknownPropertyException, WrappedTargetException, RuntimeException, std::exception)
{
    // todo
}

// XMultiPropertySet
void SAL_CALL ChainablePropertySet::setPropertyValues(const Sequence< OUString >& rPropertyNames, const Sequence< Any >& rValues)
    throw (PropertyVetoException, IllegalArgumentException,
           WrappedTargetException, RuntimeException, std::exception)
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
        PropertyInfoHash::const_iterator aEnd = mpInfo->maMap.end(), aIter;

        for ( sal_Int32 i = 0; i < nCount; ++i, ++pString, ++pAny )
        {
            aIter = mpInfo->maMap.find ( *pString );
            if ( aIter == aEnd )
                throw RuntimeException( *pString, static_cast< XPropertySet* >( this ) );

            _setSingleValue ( *((*aIter).second), *pAny );
        }

        _postSetValues();
    }
}

Sequence< Any > SAL_CALL ChainablePropertySet::getPropertyValues(const Sequence< OUString >& rPropertyNames)
    throw (RuntimeException, std::exception)
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
        PropertyInfoHash::const_iterator aEnd = mpInfo->maMap.end(), aIter;

        for ( sal_Int32 i = 0; i < nCount; ++i, ++pString, ++pAny )
        {
            aIter = mpInfo->maMap.find ( *pString );
            if ( aIter == aEnd )
                throw RuntimeException( *pString, static_cast< XPropertySet* >( this ) );

            _getSingleValue ( *((*aIter).second), *pAny );
        }

        _postGetValues();
    }
    return aValues;
}

void SAL_CALL ChainablePropertySet::addPropertiesChangeListener( const Sequence< OUString >&, const Reference< XPropertiesChangeListener >& )
    throw(RuntimeException, std::exception)
{
    // todo
}

void SAL_CALL ChainablePropertySet::removePropertiesChangeListener( const Reference< XPropertiesChangeListener >& )
    throw(RuntimeException, std::exception)
{
    // todo
}

void SAL_CALL ChainablePropertySet::firePropertiesChangeEvent( const Sequence< OUString >&, const Reference< XPropertiesChangeListener >& )
    throw(RuntimeException, std::exception)
{
    // todo
}

// XPropertyState
PropertyState SAL_CALL ChainablePropertySet::getPropertyState( const OUString& PropertyName )
    throw(UnknownPropertyException, RuntimeException, std::exception)
{
    PropertyInfoHash::const_iterator aIter =  mpInfo->maMap.find( PropertyName );
    if( aIter == mpInfo->maMap.end())
        throw UnknownPropertyException( PropertyName, static_cast< XPropertySet* >( this ) );

    PropertyState aState(PropertyState_AMBIGUOUS_VALUE);

    _preGetPropertyState();
    _getPropertyState( *((*aIter).second), aState );
    _postGetPropertyState();

    return aState;
}

Sequence< PropertyState > SAL_CALL ChainablePropertySet::getPropertyStates( const Sequence< OUString >& rPropertyNames )
    throw(UnknownPropertyException, RuntimeException, std::exception)
{
    const sal_Int32 nCount = rPropertyNames.getLength();

    Sequence< PropertyState > aStates( nCount );
    if( nCount )
    {
        PropertyState * pState = aStates.getArray();
        const OUString * pString = rPropertyNames.getConstArray();
        PropertyInfoHash::const_iterator aEnd = mpInfo->maMap.end(), aIter;
        _preGetPropertyState();

        for ( sal_Int32 i = 0; i < nCount; ++i, ++pString, ++pState )
        {
            aIter = mpInfo->maMap.find ( *pString );
            if ( aIter == aEnd )
                throw UnknownPropertyException( *pString, static_cast< XPropertySet* >( this ) );

            _getPropertyState ( *((*aIter).second), *pState );
        }
        _postGetPropertyState();
    }
    return aStates;
}

void SAL_CALL ChainablePropertySet::setPropertyToDefault( const OUString& rPropertyName )
    throw(UnknownPropertyException, RuntimeException, std::exception)
{
    PropertyInfoHash::const_iterator aIter = mpInfo->maMap.find ( rPropertyName );

    if( aIter == mpInfo->maMap.end())
        throw UnknownPropertyException( rPropertyName, static_cast< XPropertySet* >( this ) );
    _setPropertyToDefault( *((*aIter).second) );
}

Any SAL_CALL ChainablePropertySet::getPropertyDefault( const OUString& rPropertyName )
    throw(UnknownPropertyException, WrappedTargetException, RuntimeException, std::exception)
{
    PropertyInfoHash::const_iterator aIter = mpInfo->maMap.find ( rPropertyName );

    if( aIter == mpInfo->maMap.end())
        throw UnknownPropertyException( rPropertyName, static_cast< XPropertySet* >( this ) );
    return _getPropertyDefault( *((*aIter).second) );
}

void ChainablePropertySet::_preGetPropertyState ()
    throw(css::beans::UnknownPropertyException, css::beans::PropertyVetoException, css::lang::IllegalArgumentException, css::lang::WrappedTargetException )
{
    OSL_FAIL( "you have to implement this yourself!");
}

void ChainablePropertySet::_getPropertyState( const comphelper::PropertyInfo&, PropertyState& )
    throw(UnknownPropertyException )
{
    OSL_FAIL( "you have to implement this yourself!");
}

void ChainablePropertySet::_postGetPropertyState ()
    throw(css::beans::UnknownPropertyException, css::beans::PropertyVetoException, css::lang::IllegalArgumentException, css::lang::WrappedTargetException )
{
    OSL_FAIL( "you have to implement this yourself!");
}

void ChainablePropertySet::_setPropertyToDefault( const comphelper::PropertyInfo& )
    throw(UnknownPropertyException )
{
    OSL_FAIL( "you have to implement this yourself!");
}

Any ChainablePropertySet::_getPropertyDefault( const comphelper::PropertyInfo& )
    throw(UnknownPropertyException, WrappedTargetException )
{
    OSL_FAIL( "you have to implement this yourself!");

    Any aAny;
    return aAny;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
