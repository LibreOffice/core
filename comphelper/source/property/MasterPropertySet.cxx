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
#include <osl/mutex.hxx>

#include <boost/scoped_ptr.hpp>

//////////////////////////////////////////////////////////////////////

class AutoOGuardArray
{
    boost::scoped_ptr< osl::SolarGuard > *  mpGuardArray;

public:
    AutoOGuardArray( sal_Int32 nNumElements );
    ~AutoOGuardArray();

    boost::scoped_ptr< osl::SolarGuard > &  operator[] ( sal_Int32 i ) { return mpGuardArray[i]; }
};

AutoOGuardArray::AutoOGuardArray( sal_Int32 nNumElements ) : mpGuardArray(new boost::scoped_ptr< osl::SolarGuard >[nNumElements])
{
}

AutoOGuardArray::~AutoOGuardArray()
{
    //!! release auto_ptr's and thus the mutexes locks
    delete [] mpGuardArray;

}

//////////////////////////////////////////////////////////////////////

using namespace ::rtl;
using namespace ::comphelper;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;


SlaveData::SlaveData ( ChainablePropertySet *pSlave)
: mpSlave ( pSlave )
, mxSlave ( pSlave )
, mbInit ( sal_False )
{
}

MasterPropertySet::MasterPropertySet( comphelper::MasterPropertySetInfo* pInfo, osl::SolarMutex* pMutex )
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
    SlaveMap::iterator aEnd = maSlaveMap.end(), aIter = maSlaveMap.begin();
    while (aIter != aEnd )
    {
        delete (*aIter).second;
        ++aIter;
    }
}

// XPropertySet
Reference< XPropertySetInfo > SAL_CALL MasterPropertySet::getPropertySetInfo(  )
    throw(RuntimeException)
{
    return mxInfo;
}

void MasterPropertySet::registerSlave ( ChainablePropertySet *pNewSet )
    throw()
{
    maSlaveMap [ ++mnLastId ] = new SlaveData ( pNewSet );
    mpInfo->add ( pNewSet->mpInfo->maMap, mnLastId );
}

void SAL_CALL MasterPropertySet::setPropertyValue( const ::rtl::OUString& rPropertyName, const Any& rValue )
    throw(UnknownPropertyException, PropertyVetoException, IllegalArgumentException, WrappedTargetException, RuntimeException)
{
    // acquire mutex in c-tor and releases it in the d-tor (exception safe!).
    boost::scoped_ptr< osl::SolarGuard > pMutexGuard;
    if (mpMutex)
        pMutexGuard.reset( new osl::SolarGuard(mpMutex) );

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
        boost::scoped_ptr< osl::SolarGuard > pMutexGuard2;
        if (pSlave->mpMutex)
            pMutexGuard2.reset( new osl::SolarGuard(pSlave->mpMutex) );

        pSlave->_preSetValues();
        pSlave->_setSingleValue( *((*aIter).second->mpInfo), rValue );
        pSlave->_postSetValues();
    }
}

Any SAL_CALL MasterPropertySet::getPropertyValue( const ::rtl::OUString& rPropertyName )
    throw(UnknownPropertyException, WrappedTargetException, RuntimeException)
{
    // acquire mutex in c-tor and releases it in the d-tor (exception safe!).
    boost::scoped_ptr< osl::SolarGuard > pMutexGuard;
    if (mpMutex)
        pMutexGuard.reset( new osl::SolarGuard(mpMutex) );

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
        boost::scoped_ptr< osl::SolarGuard > pMutexGuard2;
        if (pSlave->mpMutex)
            pMutexGuard2.reset( new osl::SolarGuard(pSlave->mpMutex) );

        pSlave->_preGetValues();
        pSlave->_getSingleValue( *((*aIter).second->mpInfo), aAny );
        pSlave->_postGetValues();
    }
    return aAny;
}

void SAL_CALL MasterPropertySet::addPropertyChangeListener( const ::rtl::OUString&, const Reference< XPropertyChangeListener >& )
    throw(UnknownPropertyException, WrappedTargetException, RuntimeException)
{
    // todo
}

void SAL_CALL MasterPropertySet::removePropertyChangeListener( const ::rtl::OUString&, const Reference< XPropertyChangeListener >& )
    throw(UnknownPropertyException, WrappedTargetException, RuntimeException)
{
    // todo
}

void SAL_CALL MasterPropertySet::addVetoableChangeListener( const ::rtl::OUString&, const Reference< XVetoableChangeListener >& )
    throw(UnknownPropertyException, WrappedTargetException, RuntimeException)
{
    // todo
}

void SAL_CALL MasterPropertySet::removeVetoableChangeListener( const ::rtl::OUString&, const Reference< XVetoableChangeListener >& )
    throw(UnknownPropertyException, WrappedTargetException, RuntimeException)
{
    // todo
}

// XMultiPropertySet
void SAL_CALL MasterPropertySet::setPropertyValues( const Sequence< ::rtl::OUString >& aPropertyNames, const Sequence< Any >& aValues )
    throw(PropertyVetoException, IllegalArgumentException, WrappedTargetException, RuntimeException)
{
    // acquire mutex in c-tor and releases it in the d-tor (exception safe!).
    boost::scoped_ptr< osl::SolarGuard > pMutexGuard;
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
        PropertyDataHash::const_iterator aEnd = mpInfo->maMap.end(), aIter;

        //!! have an auto_ptr to an array of OGuards in order to have the
        //!! allocated memory properly freed (exception safe!).
        //!! Since the array itself has auto_ptrs as members we have to use a
        //!! helper class 'AutoOGuardArray' in order to have
        //!! the acquired locks properly released.
        AutoOGuardArray aOGuardArray( nCount );

        for ( sal_Int32 i = 0; i < nCount; ++i, ++pString, ++pAny )
        {
            aIter = mpInfo->maMap.find ( *pString );
            if ( aIter == aEnd )
                throw UnknownPropertyException( *pString, static_cast< XPropertySet* >( this ) );

            if ( (*aIter).second->mnMapId == 0 ) // 0 means it's one of ours !
                _setSingleValue( *((*aIter).second->mpInfo), *pAny );
            else
            {
                SlaveData * pSlave = maSlaveMap [ (*aIter).second->mnMapId ];
                if (!pSlave->IsInit())
                {
                    // acquire mutex in c-tor and releases it in the d-tor (exception safe!).
                    if (pSlave->mpSlave->mpMutex)
                        aOGuardArray[i].reset( new osl::SolarGuard(pSlave->mpSlave->mpMutex) );

                    pSlave->mpSlave->_preSetValues();
                    pSlave->SetInit ( sal_True );
                }
                pSlave->mpSlave->_setSingleValue( *((*aIter).second->mpInfo), *pAny );
            }
        }

        _postSetValues();
        SlaveMap::const_iterator aSlaveIter = maSlaveMap.begin(), aSlaveEnd = maSlaveMap.end();
        while (aSlaveIter != aSlaveEnd)
        {
            if ( (*aSlaveIter).second->IsInit())
            {
                (*aSlaveIter).second->mpSlave->_postSetValues();
                (*aSlaveIter).second->SetInit ( sal_False );
            }
            ++aSlaveIter;
        }
    }
}

Sequence< Any > SAL_CALL MasterPropertySet::getPropertyValues( const Sequence< ::rtl::OUString >& aPropertyNames )
    throw(RuntimeException)
{
    // acquire mutex in c-tor and releases it in the d-tor (exception safe!).
    boost::scoped_ptr< osl::SolarGuard > pMutexGuard;
    if (mpMutex)
        pMutexGuard.reset( new osl::SolarGuard(mpMutex) );

    const sal_Int32 nCount = aPropertyNames.getLength();

    Sequence < Any > aValues ( nCount );

    if( nCount )
    {
        _preGetValues();

        Any * pAny = aValues.getArray();
        const OUString * pString = aPropertyNames.getConstArray();
        PropertyDataHash::const_iterator aEnd = mpInfo->maMap.end(), aIter;

        //!! have an auto_ptr to an array of OGuards in order to have the
        //!! allocated memory properly freed (exception safe!).
        //!! Since the array itself has auto_ptrs as members we have to use a
        //!! helper class 'AutoOGuardArray' in order to have
        //!! the acquired locks properly released.
        AutoOGuardArray aOGuardArray( nCount );

        for ( sal_Int32 i = 0; i < nCount; ++i, ++pString, ++pAny )
        {
            aIter = mpInfo->maMap.find ( *pString );
            if ( aIter == aEnd )
                throw UnknownPropertyException( *pString, static_cast< XPropertySet* >( this ) );

            if ( (*aIter).second->mnMapId == 0 ) // 0 means it's one of ours !
                _getSingleValue( *((*aIter).second->mpInfo), *pAny );
            else
            {
                SlaveData * pSlave = maSlaveMap [ (*aIter).second->mnMapId ];
                if (!pSlave->IsInit())
                {
                    // acquire mutex in c-tor and releases it in the d-tor (exception safe!).
                    if (pSlave->mpSlave->mpMutex)
                        aOGuardArray[i].reset( new osl::SolarGuard(pSlave->mpSlave->mpMutex) );

                    pSlave->mpSlave->_preGetValues();
                    pSlave->SetInit ( sal_True );
                }
                pSlave->mpSlave->_getSingleValue( *((*aIter).second->mpInfo), *pAny );
            }
        }

        _postSetValues();
        SlaveMap::const_iterator aSlaveIter = maSlaveMap.begin(), aSlaveEnd = maSlaveMap.end();
        while (aSlaveIter != aSlaveEnd)
        {
            if ( (*aSlaveIter).second->IsInit())
            {
                (*aSlaveIter).second->mpSlave->_postSetValues();
                (*aSlaveIter).second->SetInit ( sal_False );
            }
            ++aSlaveIter;
        }
    }
    return aValues;
}

void SAL_CALL MasterPropertySet::addPropertiesChangeListener( const Sequence< ::rtl::OUString >&, const Reference< XPropertiesChangeListener >& )
    throw(RuntimeException)
{
    // todo
}

void SAL_CALL MasterPropertySet::removePropertiesChangeListener( const Reference< XPropertiesChangeListener >& )
    throw(RuntimeException)
{
    // todo
}

void SAL_CALL MasterPropertySet::firePropertiesChangeEvent( const Sequence< ::rtl::OUString >&, const Reference< XPropertiesChangeListener >& )
    throw(RuntimeException)
{
    // todo
}

// XPropertyState
PropertyState SAL_CALL MasterPropertySet::getPropertyState( const ::rtl::OUString& PropertyName )
    throw(UnknownPropertyException, RuntimeException)
{
    PropertyDataHash::const_iterator aIter =  mpInfo->maMap.find( PropertyName );
    if( aIter == mpInfo->maMap.end())
        throw UnknownPropertyException( PropertyName, static_cast< XPropertySet* >( this ) );

    PropertyState aState;

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
        boost::scoped_ptr< osl::SolarGuard > pMutexGuard;
        if (pSlave->mpMutex)
            pMutexGuard.reset( new osl::SolarGuard(pSlave->mpMutex) );

        pSlave->_preGetPropertyState();
        pSlave->_getPropertyState( *((*aIter).second->mpInfo), aState );
        pSlave->_postGetPropertyState();
    }

    return aState;
}

Sequence< PropertyState > SAL_CALL MasterPropertySet::getPropertyStates( const Sequence< ::rtl::OUString >& rPropertyNames )
    throw(UnknownPropertyException, RuntimeException)
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
                    pSlave->mpSlave->_preGetPropertyState();
                    pSlave->SetInit ( sal_True );
                }
                pSlave->mpSlave->_getPropertyState( *((*aIter).second->mpInfo), *pState );
            }
        }
        _postGetPropertyState();
        SlaveMap::const_iterator aSlaveIter = maSlaveMap.begin(), aSlaveEnd = maSlaveMap.end();
        while (aSlaveIter != aSlaveEnd)
        {
            if ( (*aSlaveIter).second->IsInit())
            {
                (*aSlaveIter).second->mpSlave->_postGetPropertyState();
                (*aSlaveIter).second->SetInit ( sal_False );
            }
            ++aSlaveIter;
        }
    }
    return aStates;
}

void SAL_CALL MasterPropertySet::setPropertyToDefault( const ::rtl::OUString& rPropertyName )
    throw(UnknownPropertyException, RuntimeException)
{
    PropertyDataHash::const_iterator aIter = mpInfo->maMap.find ( rPropertyName );

    if( aIter == mpInfo->maMap.end())
        throw UnknownPropertyException( rPropertyName, static_cast< XPropertySet* >( this ) );
    _setPropertyToDefault( *((*aIter).second->mpInfo) );
}

Any SAL_CALL MasterPropertySet::getPropertyDefault( const ::rtl::OUString& rPropertyName )
    throw(UnknownPropertyException, WrappedTargetException, RuntimeException)
{
    PropertyDataHash::const_iterator aIter = mpInfo->maMap.find ( rPropertyName );

    if( aIter == mpInfo->maMap.end())
        throw UnknownPropertyException( rPropertyName, static_cast< XPropertySet* >( this ) );
    return _getPropertyDefault( *((*aIter).second->mpInfo) );
}

void MasterPropertySet::_preGetPropertyState ()
    throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException )
{
    OSL_FAIL( "you have to implement this yourself!");
}

void MasterPropertySet::_getPropertyState( const comphelper::PropertyInfo&, PropertyState& )
    throw(UnknownPropertyException )
{
    OSL_FAIL( "you have to implement this yourself!");
}

void MasterPropertySet::_postGetPropertyState ()
    throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException )
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
