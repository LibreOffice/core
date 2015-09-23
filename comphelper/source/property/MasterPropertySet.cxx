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


#include <comphelper/MasterPropertySet.hxx>
#include <comphelper/MasterPropertySetInfo.hxx>
#include <comphelper/ChainablePropertySet.hxx>
#include <comphelper/ChainablePropertySetInfo.hxx>
#include <comphelper/solarmutex.hxx>

#include <osl/diagnose.h>

#include <memory>

class AutoOGuardArray
{
    std::unique_ptr< osl::Guard< comphelper::SolarMutex > > *  mpGuardArray;

public:
    explicit AutoOGuardArray( sal_Int32 nNumElements );
    ~AutoOGuardArray();

    std::unique_ptr< osl::Guard< comphelper::SolarMutex > > &  operator[] ( sal_Int32 i ) { return mpGuardArray[i]; }
};

AutoOGuardArray::AutoOGuardArray( sal_Int32 nNumElements ) : mpGuardArray(new std::unique_ptr< osl::Guard< comphelper::SolarMutex > >[nNumElements])
{
}

AutoOGuardArray::~AutoOGuardArray()
{
    //!! release unique_ptr's and thus the mutexes locks
    delete [] mpGuardArray;

}



using namespace ::comphelper;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;


SlaveData::SlaveData ( ChainablePropertySet *pSlave)
: mpSlave ( pSlave )
, mxSlave ( pSlave )
, mbInit ( false )
{
}

MasterPropertySet::MasterPropertySet( comphelper::MasterPropertySetInfo* pInfo, comphelper::SolarMutex* pMutex )
    throw()
: mpInfo ( pInfo )
, mpMutex ( pMutex )
, mnLastId ( 0 )
, mxInfo ( pInfo )
{
}

MasterPropertySet::~MasterPropertySet()
    throw()
{
    for( auto& rSlave : maSlaveMap )
        delete rSlave.second;
}

// XPropertySet
Reference< XPropertySetInfo > SAL_CALL MasterPropertySet::getPropertySetInfo(  )
    throw(RuntimeException, std::exception)
{
    return mxInfo;
}

void MasterPropertySet::registerSlave ( ChainablePropertySet *pNewSet )
    throw()
{
    maSlaveMap [ ++mnLastId ] = new SlaveData ( pNewSet );
    mpInfo->add ( pNewSet->mpInfo->maMap, mnLastId );
}

void SAL_CALL MasterPropertySet::setPropertyValue( const OUString& rPropertyName, const Any& rValue )
    throw(UnknownPropertyException, PropertyVetoException, IllegalArgumentException, WrappedTargetException, RuntimeException, std::exception)
{
    // acquire mutex in c-tor and releases it in the d-tor (exception safe!).
    std::unique_ptr< osl::Guard< comphelper::SolarMutex > > xMutexGuard;
    if (mpMutex)
        xMutexGuard.reset( new osl::Guard< comphelper::SolarMutex >(mpMutex) );

    PropertyDataHash::const_iterator aIter = mpInfo->maMap.find ( rPropertyName );

    if( aIter == mpInfo->maMap.end())
        throw UnknownPropertyException( rPropertyName, static_cast< XPropertySet* >( this ) );

    if ( (*aIter).second->mnMapId == 0 ) // 0 means it's one of ours !
    {
        _preSetValues();
        _setSingleValue( *((*aIter).second->mpInfo), rValue );
        _postSetValues();
    }
    else
    {
        ChainablePropertySet * pSlave = maSlaveMap [ (*aIter).second->mnMapId ]->mpSlave;

        // acquire mutex in c-tor and releases it in the d-tor (exception safe!).
        std::unique_ptr< osl::Guard< comphelper::SolarMutex > > xMutexGuard2;
        if (pSlave->mpMutex)
            xMutexGuard2.reset( new osl::Guard< comphelper::SolarMutex >(pSlave->mpMutex) );

        pSlave->_preSetValues();
        pSlave->_setSingleValue( *((*aIter).second->mpInfo), rValue );
        pSlave->_postSetValues();
    }
}

Any SAL_CALL MasterPropertySet::getPropertyValue( const OUString& rPropertyName )
    throw(UnknownPropertyException, WrappedTargetException, RuntimeException, std::exception)
{
    // acquire mutex in c-tor and releases it in the d-tor (exception safe!).
    std::unique_ptr< osl::Guard< comphelper::SolarMutex > > xMutexGuard;
    if (mpMutex)
        xMutexGuard.reset( new osl::Guard< comphelper::SolarMutex >(mpMutex) );

    PropertyDataHash::const_iterator aIter = mpInfo->maMap.find ( rPropertyName );

    if( aIter == mpInfo->maMap.end())
        throw UnknownPropertyException( rPropertyName, static_cast< XPropertySet* >( this ) );

    Any aAny;
    if ( (*aIter).second->mnMapId == 0 ) // 0 means it's one of ours !
    {
        _preGetValues();
        _getSingleValue( *((*aIter).second->mpInfo), aAny );
        _postGetValues();
    }
    else
    {
        ChainablePropertySet * pSlave = maSlaveMap [ (*aIter).second->mnMapId ]->mpSlave;

        // acquire mutex in c-tor and releases it in the d-tor (exception safe!).
        std::unique_ptr< osl::Guard< comphelper::SolarMutex > > xMutexGuard2;
        if (pSlave->mpMutex)
            xMutexGuard2.reset( new osl::Guard< comphelper::SolarMutex >(pSlave->mpMutex) );

        pSlave->_preGetValues();
        pSlave->_getSingleValue( *((*aIter).second->mpInfo), aAny );
        pSlave->_postGetValues();
    }
    return aAny;
}

void SAL_CALL MasterPropertySet::addPropertyChangeListener( const OUString&, const Reference< XPropertyChangeListener >& )
    throw(UnknownPropertyException, WrappedTargetException, RuntimeException, std::exception)
{
    // todo
}

void SAL_CALL MasterPropertySet::removePropertyChangeListener( const OUString&, const Reference< XPropertyChangeListener >& )
    throw(UnknownPropertyException, WrappedTargetException, RuntimeException, std::exception)
{
    // todo
}

void SAL_CALL MasterPropertySet::addVetoableChangeListener( const OUString&, const Reference< XVetoableChangeListener >& )
    throw(UnknownPropertyException, WrappedTargetException, RuntimeException, std::exception)
{
    // todo
}

void SAL_CALL MasterPropertySet::removeVetoableChangeListener( const OUString&, const Reference< XVetoableChangeListener >& )
    throw(UnknownPropertyException, WrappedTargetException, RuntimeException, std::exception)
{
    // todo
}

// XMultiPropertySet
void SAL_CALL MasterPropertySet::setPropertyValues( const Sequence< OUString >& aPropertyNames, const Sequence< Any >& aValues )
    throw(PropertyVetoException, IllegalArgumentException, WrappedTargetException, RuntimeException, std::exception)
{
    // acquire mutex in c-tor and releases it in the d-tor (exception safe!).
    std::unique_ptr< osl::Guard< comphelper::SolarMutex > > xMutexGuard;
    if (mpMutex)
        xMutexGuard.reset( new osl::Guard< comphelper::SolarMutex >(mpMutex) );

    const sal_Int32 nCount = aPropertyNames.getLength();

    if( nCount != aValues.getLength() )
        throw IllegalArgumentException();

    if( nCount )
    {
        _preSetValues();

        const Any * pAny = aValues.getConstArray();
        const OUString * pString = aPropertyNames.getConstArray();
        PropertyDataHash::const_iterator aEnd = mpInfo->maMap.end(), aIter;

        //!! have a unique_ptr to an array of OGuards in order to have the
        //!! allocated memory properly freed (exception safe!).
        //!! Since the array itself has unique_ptrs as members we have to use a
        //!! helper class 'AutoOGuardArray' in order to have
        //!! the acquired locks properly released.
        AutoOGuardArray aOGuardArray( nCount );

        for ( sal_Int32 i = 0; i < nCount; ++i, ++pString, ++pAny )
        {
            aIter = mpInfo->maMap.find ( *pString );
            if ( aIter == aEnd )
                throw RuntimeException( *pString, static_cast< XPropertySet* >( this ) );

            if ( (*aIter).second->mnMapId == 0 ) // 0 means it's one of ours !
                _setSingleValue( *((*aIter).second->mpInfo), *pAny );
            else
            {
                SlaveData * pSlave = maSlaveMap [ (*aIter).second->mnMapId ];
                if (!pSlave->IsInit())
                {
                    // acquire mutex in c-tor and releases it in the d-tor (exception safe!).
                    if (pSlave->mpSlave->mpMutex)
                        aOGuardArray[i].reset( new osl::Guard< comphelper::SolarMutex >(pSlave->mpSlave->mpMutex) );

                    pSlave->mpSlave->_preSetValues();
                    pSlave->SetInit ( true );
                }
                pSlave->mpSlave->_setSingleValue( *((*aIter).second->mpInfo), *pAny );
            }
        }

        _postSetValues();
        for( const auto& rSlave : maSlaveMap )
        {
            if( rSlave.second->IsInit() )
            {
                rSlave.second->mpSlave->_postSetValues();
                rSlave.second->SetInit( false );
            }
        }
    }
}

Sequence< Any > SAL_CALL MasterPropertySet::getPropertyValues( const Sequence< OUString >& aPropertyNames )
    throw(RuntimeException, std::exception)
{
    // acquire mutex in c-tor and releases it in the d-tor (exception safe!).
    std::unique_ptr< osl::Guard< comphelper::SolarMutex > > xMutexGuard;
    if (mpMutex)
        xMutexGuard.reset( new osl::Guard< comphelper::SolarMutex >(mpMutex) );

    const sal_Int32 nCount = aPropertyNames.getLength();

    Sequence < Any > aValues ( nCount );

    if( nCount )
    {
        _preGetValues();

        Any * pAny = aValues.getArray();
        const OUString * pString = aPropertyNames.getConstArray();
        PropertyDataHash::const_iterator aEnd = mpInfo->maMap.end(), aIter;

        //!! have an unique_ptr to an array of OGuards in order to have the
        //!! allocated memory properly freed (exception safe!).
        //!! Since the array itself has unique_ptrs as members we have to use a
        //!! helper class 'AutoOGuardArray' in order to have
        //!! the acquired locks properly released.
        AutoOGuardArray aOGuardArray( nCount );

        for ( sal_Int32 i = 0; i < nCount; ++i, ++pString, ++pAny )
        {
            aIter = mpInfo->maMap.find ( *pString );
            if ( aIter == aEnd )
                throw RuntimeException( *pString, static_cast< XPropertySet* >( this ) );

            if ( (*aIter).second->mnMapId == 0 ) // 0 means it's one of ours !
                _getSingleValue( *((*aIter).second->mpInfo), *pAny );
            else
            {
                SlaveData * pSlave = maSlaveMap [ (*aIter).second->mnMapId ];
                if (!pSlave->IsInit())
                {
                    // acquire mutex in c-tor and releases it in the d-tor (exception safe!).
                    if (pSlave->mpSlave->mpMutex)
                        aOGuardArray[i].reset( new osl::Guard< comphelper::SolarMutex >(pSlave->mpSlave->mpMutex) );

                    pSlave->mpSlave->_preGetValues();
                    pSlave->SetInit ( true );
                }
                pSlave->mpSlave->_getSingleValue( *((*aIter).second->mpInfo), *pAny );
            }
        }

        _postSetValues();
        for( const auto& rSlave : maSlaveMap )
        {
            if( rSlave.second->IsInit() )
            {
                rSlave.second->mpSlave->_postSetValues();
                rSlave.second->SetInit( false );
            }
        }
    }
    return aValues;
}

void SAL_CALL MasterPropertySet::addPropertiesChangeListener( const Sequence< OUString >&, const Reference< XPropertiesChangeListener >& )
    throw(RuntimeException, std::exception)
{
    // todo
}

void SAL_CALL MasterPropertySet::removePropertiesChangeListener( const Reference< XPropertiesChangeListener >& )
    throw(RuntimeException, std::exception)
{
    // todo
}

void SAL_CALL MasterPropertySet::firePropertiesChangeEvent( const Sequence< OUString >&, const Reference< XPropertiesChangeListener >& )
    throw(RuntimeException, std::exception)
{
    // todo
}

// XPropertyState
PropertyState SAL_CALL MasterPropertySet::getPropertyState( const OUString& PropertyName )
    throw(UnknownPropertyException, RuntimeException, std::exception)
{
    PropertyDataHash::const_iterator aIter =  mpInfo->maMap.find( PropertyName );
    if( aIter == mpInfo->maMap.end())
        throw UnknownPropertyException( PropertyName, static_cast< XPropertySet* >( this ) );

    PropertyState aState(PropertyState_AMBIGUOUS_VALUE);

    if ( (*aIter).second->mnMapId == 0 ) // 0 means it's one of ours !
    {
        _preGetPropertyState();
        _getPropertyState( *((*aIter).second->mpInfo), aState );
        _postGetPropertyState();
    }
    else
    {
        ChainablePropertySet * pSlave = maSlaveMap [ (*aIter).second->mnMapId ]->mpSlave;

        // acquire mutex in c-tor and releases it in the d-tor (exception safe!).
        std::unique_ptr< osl::Guard< comphelper::SolarMutex > > xMutexGuard;
        if (pSlave->mpMutex)
            xMutexGuard.reset( new osl::Guard< comphelper::SolarMutex >(pSlave->mpMutex) );

        // FIXME: Each of these three methods does OSL_FAIL( "you have
        // to implement this yourself!") and nothing else, so this
        // can't make much sense. Is this whole else branch in fact
        // dead code?
        ChainablePropertySet::_preGetPropertyState();
        ChainablePropertySet::_getPropertyState( *((*aIter).second->mpInfo), aState );
        ChainablePropertySet::_postGetPropertyState();
    }

    return aState;
}

Sequence< PropertyState > SAL_CALL MasterPropertySet::getPropertyStates( const Sequence< OUString >& rPropertyNames )
    throw(UnknownPropertyException, RuntimeException, std::exception)
{
    const sal_Int32 nCount = rPropertyNames.getLength();

    Sequence< PropertyState > aStates( nCount );
    if( nCount )
    {
        PropertyState * pState = aStates.getArray();
        const OUString * pString = rPropertyNames.getConstArray();
        PropertyDataHash::const_iterator aEnd = mpInfo->maMap.end(), aIter;
        _preGetPropertyState();

        for ( sal_Int32 i = 0; i < nCount; ++i, ++pString, ++pState )
        {
            aIter = mpInfo->maMap.find ( *pString );
            if ( aIter == aEnd )
                throw UnknownPropertyException( *pString, static_cast< XPropertySet* >( this ) );

            if ( (*aIter).second->mnMapId == 0 ) // 0 means it's one of ours !
                _getPropertyState( *((*aIter).second->mpInfo), *pState );
            else
            {
                SlaveData * pSlave = maSlaveMap [ (*aIter).second->mnMapId ];
                if (!pSlave->IsInit())
                {
                    comphelper::ChainablePropertySet::_preGetPropertyState();
                    pSlave->SetInit ( true );
                }
                comphelper::ChainablePropertySet::_getPropertyState( *((*aIter).second->mpInfo), *pState );
            }
        }
        _postGetPropertyState();
        for( const auto& rSlave : maSlaveMap )
        {
            if( rSlave.second->IsInit() )
            {
                comphelper::ChainablePropertySet::_postGetPropertyState();
                rSlave.second->SetInit( false );
            }
        }
    }
    return aStates;
}

void SAL_CALL MasterPropertySet::setPropertyToDefault( const OUString& rPropertyName )
    throw(UnknownPropertyException, RuntimeException, std::exception)
{
    PropertyDataHash::const_iterator aIter = mpInfo->maMap.find ( rPropertyName );

    if( aIter == mpInfo->maMap.end())
        throw UnknownPropertyException( rPropertyName, static_cast< XPropertySet* >( this ) );
    _setPropertyToDefault( *((*aIter).second->mpInfo) );
}

Any SAL_CALL MasterPropertySet::getPropertyDefault( const OUString& rPropertyName )
    throw(UnknownPropertyException, WrappedTargetException, RuntimeException, std::exception)
{
    PropertyDataHash::const_iterator aIter = mpInfo->maMap.find ( rPropertyName );

    if( aIter == mpInfo->maMap.end())
        throw UnknownPropertyException( rPropertyName, static_cast< XPropertySet* >( this ) );
    return _getPropertyDefault( *((*aIter).second->mpInfo) );
}

void MasterPropertySet::_preGetPropertyState ()
    throw(css::beans::UnknownPropertyException, css::beans::PropertyVetoException, css::lang::IllegalArgumentException, css::lang::WrappedTargetException )
{
    OSL_FAIL( "you have to implement this yourself!");
}

void MasterPropertySet::_getPropertyState( const comphelper::PropertyInfo&, PropertyState& )
    throw(UnknownPropertyException )
{
    OSL_FAIL( "you have to implement this yourself!");
}

void MasterPropertySet::_postGetPropertyState ()
    throw(css::beans::UnknownPropertyException, css::beans::PropertyVetoException, css::lang::IllegalArgumentException, css::lang::WrappedTargetException )
{
    OSL_FAIL( "you have to implement this yourself!");
}

void MasterPropertySet::_setPropertyToDefault( const comphelper::PropertyInfo& )
    throw(UnknownPropertyException )
{
    OSL_FAIL( "you have to implement this yourself!");
}

Any MasterPropertySet::_getPropertyDefault( const comphelper::PropertyInfo& )
    throw(UnknownPropertyException, WrappedTargetException )
{
    OSL_FAIL( "you have to implement this yourself!");
    Any aAny;
    return aAny;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
