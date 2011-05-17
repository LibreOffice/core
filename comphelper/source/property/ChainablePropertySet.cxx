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
#include "precompiled_comphelper.hxx"
#include <comphelper/ChainablePropertySet.hxx>
#include <comphelper/ChainablePropertySetInfo.hxx>
#include <osl/mutex.hxx>

#include <memory>       // STL auto_ptr


using namespace ::rtl;
using namespace ::comphelper;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;

ChainablePropertySet::ChainablePropertySet( comphelper::ChainablePropertySetInfo* pInfo, osl::SolarMutex* pMutex )
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
    throw(RuntimeException)
{
    return mxInfo;
}

void ChainablePropertySet::lockMutex()
{
    if (mpMutex)
        mpMutex->acquire();
}

void ChainablePropertySet::unlockMutex()
{
    if (mpMutex)
        mpMutex->release();
}

void SAL_CALL ChainablePropertySet::setPropertyValue( const ::rtl::OUString& rPropertyName, const Any& rValue )
    throw(UnknownPropertyException, PropertyVetoException, IllegalArgumentException, WrappedTargetException, RuntimeException)
{
    // acquire mutex in c-tor and releases it in the d-tor (exception safe!).
    std::auto_ptr< osl::SolarGuard > pMutexGuard;
    if (mpMutex)
        pMutexGuard.reset( new osl::SolarGuard(mpMutex) );

    PropertyInfoHash::const_iterator aIter = mpInfo->maMap.find ( rPropertyName );

    if( aIter == mpInfo->maMap.end())
        throw UnknownPropertyException( rPropertyName, static_cast< XPropertySet* >( this ) );

    _preSetValues();
    _setSingleValue( *((*aIter).second), rValue );
    _postSetValues();
}

Any SAL_CALL ChainablePropertySet::getPropertyValue( const ::rtl::OUString& rPropertyName )
    throw(UnknownPropertyException, WrappedTargetException, RuntimeException)
{
    // acquire mutex in c-tor and releases it in the d-tor (exception safe!).
    std::auto_ptr< osl::SolarGuard > pMutexGuard;
    if (mpMutex)
        pMutexGuard.reset( new osl::SolarGuard(mpMutex) );

    PropertyInfoHash::const_iterator aIter = mpInfo->maMap.find ( rPropertyName );

    if( aIter == mpInfo->maMap.end())
        throw UnknownPropertyException( rPropertyName, static_cast< XPropertySet* >( this ) );

    Any aAny;
    _preGetValues ();
    _getSingleValue( *((*aIter).second), aAny );
    _postGetValues ();

    return aAny;
}

void SAL_CALL ChainablePropertySet::addPropertyChangeListener( const ::rtl::OUString&, const Reference< XPropertyChangeListener >& )
    throw(UnknownPropertyException, WrappedTargetException, RuntimeException)
{
    // todo
}

void SAL_CALL ChainablePropertySet::removePropertyChangeListener( const ::rtl::OUString&, const Reference< XPropertyChangeListener >& )
    throw(UnknownPropertyException, WrappedTargetException, RuntimeException)
{
    // todo
}

void SAL_CALL ChainablePropertySet::addVetoableChangeListener( const ::rtl::OUString&, const Reference< XVetoableChangeListener >& )
    throw(UnknownPropertyException, WrappedTargetException, RuntimeException)
{
    // todo
}

void SAL_CALL ChainablePropertySet::removeVetoableChangeListener( const ::rtl::OUString&, const Reference< XVetoableChangeListener >& )
    throw(UnknownPropertyException, WrappedTargetException, RuntimeException)
{
    // todo
}

// XMultiPropertySet
void SAL_CALL ChainablePropertySet::setPropertyValues( const Sequence< ::rtl::OUString >& aPropertyNames, const Sequence< Any >& aValues )
    throw(PropertyVetoException, IllegalArgumentException, WrappedTargetException, RuntimeException)
{
    // acquire mutex in c-tor and releases it in the d-tor (exception safe!).
    std::auto_ptr< osl::SolarGuard > pMutexGuard;
    if (mpMutex)
        pMutexGuard.reset( new osl::SolarGuard(mpMutex) );

    const sal_Int32 nCount = aPropertyNames.getLength();

    if( nCount != aValues.getLength() )
        throw IllegalArgumentException();

    if( nCount )
    {
        _preSetValues();

        const Any * pAny = aValues.getConstArray();
        const OUString * pString = aPropertyNames.getConstArray();
        PropertyInfoHash::const_iterator aEnd = mpInfo->maMap.end(), aIter;

        for ( sal_Int32 i = 0; i < nCount; ++i, ++pString, ++pAny )
        {
            aIter = mpInfo->maMap.find ( *pString );
            if ( aIter == aEnd )
                throw UnknownPropertyException( *pString, static_cast< XPropertySet* >( this ) );

            _setSingleValue ( *((*aIter).second), *pAny );
        }

        _postSetValues();
    }
}

Sequence< Any > SAL_CALL ChainablePropertySet::getPropertyValues( const Sequence< ::rtl::OUString >& aPropertyNames )
    throw(RuntimeException)
{
    // acquire mutex in c-tor and releases it in the d-tor (exception safe!).
    std::auto_ptr< osl::SolarGuard > pMutexGuard;
    if (mpMutex)
        pMutexGuard.reset( new osl::SolarGuard(mpMutex) );

    const sal_Int32 nCount = aPropertyNames.getLength();

    Sequence < Any > aValues ( nCount );

    if( nCount )
    {
        _preGetValues();

        Any * pAny = aValues.getArray();
        const OUString * pString = aPropertyNames.getConstArray();
        PropertyInfoHash::const_iterator aEnd = mpInfo->maMap.end(), aIter;

        for ( sal_Int32 i = 0; i < nCount; ++i, ++pString, ++pAny )
        {
            aIter = mpInfo->maMap.find ( *pString );
            if ( aIter == aEnd )
                throw UnknownPropertyException( *pString, static_cast< XPropertySet* >( this ) );

            _getSingleValue ( *((*aIter).second), *pAny );
        }

        _postGetValues();
    }
    return aValues;
}

void SAL_CALL ChainablePropertySet::addPropertiesChangeListener( const Sequence< ::rtl::OUString >&, const Reference< XPropertiesChangeListener >& )
    throw(RuntimeException)
{
    // todo
}

void SAL_CALL ChainablePropertySet::removePropertiesChangeListener( const Reference< XPropertiesChangeListener >& )
    throw(RuntimeException)
{
    // todo
}

void SAL_CALL ChainablePropertySet::firePropertiesChangeEvent( const Sequence< ::rtl::OUString >&, const Reference< XPropertiesChangeListener >& )
    throw(RuntimeException)
{
    // todo
}

// XPropertyState
PropertyState SAL_CALL ChainablePropertySet::getPropertyState( const ::rtl::OUString& PropertyName )
    throw(UnknownPropertyException, RuntimeException)
{
    PropertyInfoHash::const_iterator aIter =  mpInfo->maMap.find( PropertyName );
    if( aIter == mpInfo->maMap.end())
        throw UnknownPropertyException( PropertyName, static_cast< XPropertySet* >( this ) );

    PropertyState aState;

    _preGetPropertyState();
    _getPropertyState( *((*aIter).second), aState );
    _postGetPropertyState();

    return aState;
}

Sequence< PropertyState > SAL_CALL ChainablePropertySet::getPropertyStates( const Sequence< ::rtl::OUString >& rPropertyNames )
    throw(UnknownPropertyException, RuntimeException)
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

void SAL_CALL ChainablePropertySet::setPropertyToDefault( const ::rtl::OUString& rPropertyName )
    throw(UnknownPropertyException, RuntimeException)
{
    PropertyInfoHash::const_iterator aIter = mpInfo->maMap.find ( rPropertyName );

    if( aIter == mpInfo->maMap.end())
        throw UnknownPropertyException( rPropertyName, static_cast< XPropertySet* >( this ) );
    _setPropertyToDefault( *((*aIter).second) );
}

Any SAL_CALL ChainablePropertySet::getPropertyDefault( const ::rtl::OUString& rPropertyName )
    throw(UnknownPropertyException, WrappedTargetException, RuntimeException)
{
    PropertyInfoHash::const_iterator aIter = mpInfo->maMap.find ( rPropertyName );

    if( aIter == mpInfo->maMap.end())
        throw UnknownPropertyException( rPropertyName, static_cast< XPropertySet* >( this ) );
    return _getPropertyDefault( *((*aIter).second) );
}

void ChainablePropertySet::_preGetPropertyState ()
    throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException )
{
    OSL_FAIL( "you have to implement this yourself!");
}

void ChainablePropertySet::_getPropertyState( const comphelper::PropertyInfo&, PropertyState& )
    throw(UnknownPropertyException )
{
    OSL_FAIL( "you have to implement this yourself!");
}

void ChainablePropertySet::_postGetPropertyState ()
    throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException )
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
