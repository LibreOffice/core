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


#include <cppuhelper/interfacecontainer.hxx>
#include <cppuhelper/queryinterface.hxx>
#include <cppuhelper/propshlp.hxx>

#include <osl/diagnose.h>
#include <osl/mutex.hxx>

#include <boost/scoped_array.hpp>
#include <boost/unordered_map.hpp>

#include <com/sun/star/lang/XEventListener.hpp>


using namespace osl;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;

namespace cppu
{
/**
 * Reallocate the sequence.
 */
static void realloc( Sequence< Reference< XInterface > > & rSeq, sal_Int32 nNewLen )
{
    rSeq.realloc( nNewLen );
}

/**
 * Remove an element from an interface sequence.
 */
static void sequenceRemoveElementAt( Sequence< Reference< XInterface > > & rSeq, sal_Int32 index )
{
    sal_Int32 nNewLen = rSeq.getLength() - 1;

    Sequence< Reference< XInterface > > aDestSeq( rSeq.getLength() - 1 );
    // getArray on a const sequence is faster
    const Reference< XInterface > * pSource = ((const Sequence< Reference< XInterface > > &)rSeq).getConstArray();
    Reference< XInterface > * pDest = aDestSeq.getArray();
    sal_Int32 i = 0;
    for( ; i < index; i++ )
        pDest[i] = pSource[i];
    for( sal_Int32 j = i ; j < nNewLen; j++ )
        pDest[j] = pSource[j+1];
    rSeq = aDestSeq;
}

#ifdef _MSC_VER
#pragma warning( disable: 4786 )
#endif

OInterfaceIteratorHelper::OInterfaceIteratorHelper( OInterfaceContainerHelper & rCont_ )
    : rCont( rCont_ )
{
    MutexGuard aGuard( rCont.rMutex );
    if( rCont.bInUse )
        // worst case, two iterators at the same time
        rCont.copyAndResetInUse();
    bIsList = rCont_.bIsList;
    aData = rCont_.aData;
    if( bIsList )
    {
        rCont.bInUse = sal_True;
        nRemain = aData.pAsSequence->getLength();
    }
    else if( aData.pAsInterface )
    {
        aData.pAsInterface->acquire();
        nRemain = 1;
    }
    else
        nRemain = 0;
}

OInterfaceIteratorHelper::~OInterfaceIteratorHelper()
{
    bool bShared;
    {
    MutexGuard aGuard( rCont.rMutex );
    // bResetInUse protect the iterator against recursion
    bShared = aData.pAsSequence == rCont.aData.pAsSequence && rCont.bIsList;
    if( bShared )
    {
        OSL_ENSURE( rCont.bInUse, "OInterfaceContainerHelper must be in use" );
        rCont.bInUse = sal_False;
    }
    }

    if( !bShared )
    {
        if( bIsList )
            // Sequence owned by the iterator
            delete aData.pAsSequence;
        else if( aData.pAsInterface )
            // Interface is acquired by the iterator
            aData.pAsInterface->release();
    }
}

XInterface * OInterfaceIteratorHelper::next()
{
    if( nRemain )
    {
        nRemain--;
        if( bIsList )
            // typecase to const,so the getArray method is faster
            return aData.pAsSequence->getConstArray()[nRemain].get();
        else if( aData.pAsInterface )
            return aData.pAsInterface;
    }
    // exception
    return 0;
}

void OInterfaceIteratorHelper::remove()
{
    if( bIsList )
    {
        OSL_ASSERT( nRemain >= 0 &&
                    nRemain < aData.pAsSequence->getLength() );
        XInterface * p = aData.pAsSequence->getConstArray()[nRemain].get();
        rCont.removeInterface( * reinterpret_cast< const Reference< XInterface > * >( &p ) );
    }
    else
    {
        OSL_ASSERT( 0 == nRemain );
        rCont.removeInterface( * reinterpret_cast< const Reference< XInterface > * >(&aData.pAsInterface));
    }
}

OInterfaceContainerHelper::OInterfaceContainerHelper( Mutex & rMutex_ )
    : rMutex( rMutex_ )
    , bInUse( sal_False )
    , bIsList( sal_False )
{
}

OInterfaceContainerHelper::~OInterfaceContainerHelper()
{
    OSL_ENSURE( !bInUse, "~OInterfaceContainerHelper but is in use" );
    if( bIsList )
        delete aData.pAsSequence;
    else if( aData.pAsInterface )
        aData.pAsInterface->release();
}

sal_Int32 OInterfaceContainerHelper::getLength() const
{
    MutexGuard aGuard( rMutex );
    if( bIsList )
        return aData.pAsSequence->getLength();
    else if( aData.pAsInterface )
        return 1;
    return 0;
}

Sequence< Reference<XInterface> > OInterfaceContainerHelper::getElements() const
{
    MutexGuard aGuard( rMutex );
    if( bIsList )
        return *aData.pAsSequence;
    else if( aData.pAsInterface )
    {
        Reference<XInterface> x( aData.pAsInterface );
        return Sequence< Reference< XInterface > >( &x, 1 );
    }
    return Sequence< Reference< XInterface > >();
}

void OInterfaceContainerHelper::copyAndResetInUse()
{
    OSL_ENSURE( bInUse, "OInterfaceContainerHelper not in use" );
    if( bInUse )
    {
        // this should be the worst case. If a iterator is active
        // and a new Listener is added.
        if( bIsList )
            aData.pAsSequence = new Sequence< Reference< XInterface > >( *aData.pAsSequence );
        else if( aData.pAsInterface )
            aData.pAsInterface->acquire();

        bInUse = sal_False;
    }
}

sal_Int32 OInterfaceContainerHelper::addInterface( const Reference<XInterface> & rListener )
{
    OSL_ASSERT( rListener.is() );
    MutexGuard aGuard( rMutex );
    if( bInUse )
        copyAndResetInUse();

    if( bIsList )
    {
        sal_Int32 nLen = aData.pAsSequence->getLength();
        realloc( *aData.pAsSequence, nLen +1 );
        aData.pAsSequence->getArray()[ nLen ] = rListener;
        return nLen +1;
    }
    else if( aData.pAsInterface )
    {
        Sequence< Reference< XInterface > > * pSeq = new Sequence< Reference< XInterface > >( 2 );
        Reference<XInterface> * pArray = pSeq->getArray();
        pArray[0] = aData.pAsInterface;
        pArray[1] = rListener;
        aData.pAsInterface->release();
        aData.pAsSequence = pSeq;
        bIsList = sal_True;
        return 2;
    }
    else
    {
        aData.pAsInterface = rListener.get();
        if( rListener.is() )
            rListener->acquire();
        return 1;
    }
}

sal_Int32 OInterfaceContainerHelper::removeInterface( const Reference<XInterface> & rListener )
{
    OSL_ASSERT( rListener.is() );
    MutexGuard aGuard( rMutex );
    if( bInUse )
        copyAndResetInUse();

    if( bIsList )
    {
        const Reference<XInterface> * pL = aData.pAsSequence->getConstArray();
        sal_Int32 nLen = aData.pAsSequence->getLength();
        sal_Int32 i;
        for( i = 0; i < nLen; i++ )
        {
            // It is not valid to compare the pointer directly, but it's faster.
            if( pL[i].get() == rListener.get() )
            {
                sequenceRemoveElementAt( *aData.pAsSequence, i );
                break;
            }
        }

        if( i == nLen )
        {
            // interface not found, use the correct compare method
            for( i = 0; i < nLen; i++ )
            {
                if( pL[i] == rListener )
                {
                    sequenceRemoveElementAt(*aData.pAsSequence, i );
                    break;
                }
            }
        }

        if( aData.pAsSequence->getLength() == 1 )
        {
            XInterface * p = aData.pAsSequence->getConstArray()[0].get();
            p->acquire();
            delete aData.pAsSequence;
            aData.pAsInterface = p;
            bIsList = sal_False;
            return 1;
        }
        else
            return aData.pAsSequence->getLength();
    }
    else if( aData.pAsInterface && Reference<XInterface>( aData.pAsInterface ) == rListener )
    {
        aData.pAsInterface->release();
        aData.pAsInterface = 0;
    }
    return aData.pAsInterface ? 1 : 0;
}

void OInterfaceContainerHelper::disposeAndClear( const EventObject & rEvt )
{
    ClearableMutexGuard aGuard( rMutex );
    OInterfaceIteratorHelper aIt( *this );
    // Release container, in case new entries come while disposing
    OSL_ENSURE( !bIsList || bInUse, "OInterfaceContainerHelper not in use" );
    if( !bIsList && aData.pAsInterface )
        aData.pAsInterface->release();
    // set the member to null, use the iterator to delete the values
    aData.pAsInterface = NULL;
    bIsList = sal_False;
    bInUse = sal_False;
    aGuard.clear();
    while( aIt.hasMoreElements() )
    {
        try
        {
            Reference<XEventListener > xLst( aIt.next(), UNO_QUERY );
            if( xLst.is() )
                xLst->disposing( rEvt );
        }
        catch ( RuntimeException & )
        {
            // be robust, if e.g. a remote bridge has disposed already.
            // there is no way to delegate the error to the caller :o(.
        }
    }
}


void OInterfaceContainerHelper::clear()
{
    ClearableMutexGuard aGuard( rMutex );
    OInterfaceIteratorHelper aIt( *this );
    // Release container, in case new entries come while disposing
    OSL_ENSURE( !bIsList || bInUse, "OInterfaceContainerHelper not in use" );
    if( !bIsList && aData.pAsInterface )
        aData.pAsInterface->release();
    // set the member to null, use the iterator to delete the values
    aData.pAsInterface = 0;
    bIsList = sal_False;
    bInUse = sal_False;
    // release mutex before aIt destructor call
    aGuard.clear();
}

// specialized class for type

typedef ::std::vector< std::pair < Type , void* > > t_type2ptr;

OMultiTypeInterfaceContainerHelper::OMultiTypeInterfaceContainerHelper( Mutex & rMutex_ )
    : rMutex( rMutex_ )
{
    m_pMap = new t_type2ptr();
}

OMultiTypeInterfaceContainerHelper::~OMultiTypeInterfaceContainerHelper()
{
    t_type2ptr * pMap = (t_type2ptr *)m_pMap;
    t_type2ptr::iterator iter = pMap->begin();
    t_type2ptr::iterator end = pMap->end();

    while( iter != end )
    {
        delete (OInterfaceContainerHelper*)(*iter).second;
        (*iter).second = 0;
        ++iter;
    }
    delete pMap;
}

Sequence< Type > OMultiTypeInterfaceContainerHelper::getContainedTypes() const
{
    t_type2ptr * pMap = (t_type2ptr *)m_pMap;
    t_type2ptr::size_type nSize;

    ::osl::MutexGuard aGuard( rMutex );
    nSize = pMap->size();
    if( nSize )
    {
        ::com::sun::star::uno::Sequence< Type > aInterfaceTypes( nSize );
        Type * pArray = aInterfaceTypes.getArray();

        t_type2ptr::iterator iter = pMap->begin();
        t_type2ptr::iterator end = pMap->end();

        sal_Int32 i = 0;
        while( iter != end )
        {
            // are interfaces added to this container?
            if( ((OInterfaceContainerHelper*)(*iter).second)->getLength() )
                // yes, put the type in the array
                pArray[i++] = (*iter).first;
            ++iter;
        }
        if( (t_type2ptr::size_type)i != nSize ) {
            // may be empty container, reduce the sequence to the right size
            aInterfaceTypes = ::com::sun::star::uno::Sequence< Type >( pArray, i );
        }
        return aInterfaceTypes;
    }
    return ::com::sun::star::uno::Sequence< Type >();
}

static t_type2ptr::iterator findType(t_type2ptr *pMap, const Type & rKey )
{
    t_type2ptr::iterator iter = pMap->begin();
    t_type2ptr::iterator end = pMap->end();

    while( iter != end )
    {
        if (iter->first == rKey)
            break;
        ++iter;
    }
    return iter;
}

OInterfaceContainerHelper * OMultiTypeInterfaceContainerHelper::getContainer( const Type & rKey ) const
{
    ::osl::MutexGuard aGuard( rMutex );

    t_type2ptr * pMap = (t_type2ptr *)m_pMap;
     t_type2ptr::iterator iter = findType( pMap, rKey );
    if( iter != pMap->end() )
            return (OInterfaceContainerHelper*) (*iter).second;
    return 0;
}

sal_Int32 OMultiTypeInterfaceContainerHelper::addInterface(
    const Type & rKey, const Reference< XInterface > & rListener )
{
    ::osl::MutexGuard aGuard( rMutex );
    t_type2ptr * pMap = (t_type2ptr *)m_pMap;
    t_type2ptr::iterator iter = findType( pMap, rKey );
    if( iter == pMap->end() )
    {
        OInterfaceContainerHelper * pLC = new OInterfaceContainerHelper( rMutex );
        pMap->push_back(std::pair<Type, void*>(rKey, pLC));
        return pLC->addInterface( rListener );
    }
    else
        return ((OInterfaceContainerHelper*)(*iter).second)->addInterface( rListener );
}

sal_Int32 OMultiTypeInterfaceContainerHelper::removeInterface(
    const Type & rKey, const Reference< XInterface > & rListener )
{
    ::osl::MutexGuard aGuard( rMutex );

    // search container with id nUik
    t_type2ptr * pMap = (t_type2ptr *)m_pMap;
    t_type2ptr::iterator iter = findType( pMap, rKey );
        // container found?
    if( iter != pMap->end() )
        return ((OInterfaceContainerHelper*)(*iter).second)->removeInterface( rListener );

    // no container with this id. Always return 0
    return 0;
}

void OMultiTypeInterfaceContainerHelper::disposeAndClear( const EventObject & rEvt )
{
    t_type2ptr::size_type nSize = 0;
    boost::scoped_array<OInterfaceContainerHelper *> ppListenerContainers;
    {
        ::osl::MutexGuard aGuard( rMutex );
        t_type2ptr * pMap = (t_type2ptr *)m_pMap;
        nSize = pMap->size();
        if( nSize )
        {
            typedef OInterfaceContainerHelper* ppp;
            ppListenerContainers.reset(new ppp[nSize]);
            //ppListenerContainers = new (ListenerContainer*)[nSize];

            t_type2ptr::iterator iter = pMap->begin();
            t_type2ptr::iterator end = pMap->end();

            t_type2ptr::size_type i = 0;
            while( iter != end )
            {
                ppListenerContainers[i++] = (OInterfaceContainerHelper*)(*iter).second;
                ++iter;
            }
        }
    }

    // create a copy, because do not fire event in a guarded section
    for( t_type2ptr::size_type i = 0;
            i < nSize; i++ )
    {
        if( ppListenerContainers[i] )
            ppListenerContainers[i]->disposeAndClear( rEvt );
    }
}

void OMultiTypeInterfaceContainerHelper::clear()
{
    ::osl::MutexGuard aGuard( rMutex );
    t_type2ptr * pMap = (t_type2ptr *)m_pMap;
    t_type2ptr::iterator iter = pMap->begin();
    t_type2ptr::iterator end = pMap->end();

    while( iter != end )
    {
        ((OInterfaceContainerHelper*)(*iter).second)->clear();
        ++iter;
    }
}

// specialized class for long

typedef ::std::vector< std::pair < sal_Int32 , void* > > t_long2ptr;

static t_long2ptr::iterator findLong(t_long2ptr *pMap, sal_Int32 nKey )
{
    t_long2ptr::iterator iter = pMap->begin();
    t_long2ptr::iterator end = pMap->end();

    while( iter != end )
    {
        if (iter->first == nKey)
            break;
        ++iter;
    }
    return iter;
}

OMultiTypeInterfaceContainerHelperInt32::OMultiTypeInterfaceContainerHelperInt32( Mutex & rMutex_ )
    : m_pMap( NULL )
    , rMutex( rMutex_ )
{
    // delay pMap allocation until necessary.
}

OMultiTypeInterfaceContainerHelperInt32::~OMultiTypeInterfaceContainerHelperInt32()
{
    if (!m_pMap)
        return;

    t_long2ptr * pMap = (t_long2ptr *)m_pMap;
    t_long2ptr::iterator iter = pMap->begin();
    t_long2ptr::iterator end = pMap->end();

    while( iter != end )
    {
        delete (OInterfaceContainerHelper*)(*iter).second;
        (*iter).second = 0;
        ++iter;
    }
    delete pMap;
}

Sequence< sal_Int32 > OMultiTypeInterfaceContainerHelperInt32::getContainedTypes() const
{
    t_long2ptr * pMap = (t_long2ptr *)m_pMap;
    t_long2ptr::size_type nSize;

    ::osl::MutexGuard aGuard( rMutex );
    nSize = pMap ? pMap->size() : 0;
    if( nSize )
    {
        ::com::sun::star::uno::Sequence< sal_Int32 > aInterfaceTypes( nSize );
        sal_Int32 * pArray = aInterfaceTypes.getArray();

        t_long2ptr::iterator iter = pMap->begin();
        t_long2ptr::iterator end = pMap->end();

        sal_Int32 i = 0;
        while( iter != end )
        {
            // are interfaces added to this container?
            if( ((OInterfaceContainerHelper*)(*iter).second)->getLength() )
                // yes, put the type in the array
                pArray[i++] = (*iter).first;
            ++iter;
        }
        if( (t_long2ptr::size_type)i != nSize ) {
            // may be empty container, reduce the sequence to the right size
            aInterfaceTypes = ::com::sun::star::uno::Sequence< sal_Int32 >( pArray, i );
        }
        return aInterfaceTypes;
    }
    return ::com::sun::star::uno::Sequence< sal_Int32 >();
}

OInterfaceContainerHelper * OMultiTypeInterfaceContainerHelperInt32::getContainer( const sal_Int32 & rKey ) const
{
    ::osl::MutexGuard aGuard( rMutex );

    if (!m_pMap)
        return 0;
    t_long2ptr * pMap = (t_long2ptr *)m_pMap;
     t_long2ptr::iterator iter = findLong( pMap, rKey );
    if( iter != pMap->end() )
            return (OInterfaceContainerHelper*) (*iter).second;
    return 0;
}

sal_Int32 OMultiTypeInterfaceContainerHelperInt32::addInterface(
    const sal_Int32 & rKey, const Reference< XInterface > & rListener )
{
    ::osl::MutexGuard aGuard( rMutex );
    if (!m_pMap)
        m_pMap = new t_long2ptr();
    t_long2ptr * pMap = (t_long2ptr *)m_pMap;
    t_long2ptr::iterator iter = findLong( pMap, rKey );
     if( iter == pMap->end() )
    {
        OInterfaceContainerHelper * pLC = new OInterfaceContainerHelper( rMutex );
        pMap->push_back(std::pair< sal_Int32, void* >(rKey, pLC));
        return pLC->addInterface( rListener );
    }
    else
        return ((OInterfaceContainerHelper*)(*iter).second)->addInterface( rListener );
}

sal_Int32 OMultiTypeInterfaceContainerHelperInt32::removeInterface(
    const sal_Int32 & rKey, const Reference< XInterface > & rListener )
{
    ::osl::MutexGuard aGuard( rMutex );

    if (!m_pMap)
        return 0;
    // search container with id nUik
    t_long2ptr * pMap = (t_long2ptr *)m_pMap;
    t_long2ptr::iterator iter = findLong( pMap, rKey );
        // container found?
    if( iter != pMap->end() )
        return ((OInterfaceContainerHelper*)(*iter).second)->removeInterface( rListener );

    // no container with this id. Always return 0
    return 0;
}

void OMultiTypeInterfaceContainerHelperInt32::disposeAndClear( const EventObject & rEvt )
{
    t_long2ptr::size_type nSize = 0;
    boost::scoped_array<OInterfaceContainerHelper *> ppListenerContainers;
    {
        ::osl::MutexGuard aGuard( rMutex );
        if (!m_pMap)
            return;

        t_long2ptr * pMap = (t_long2ptr *)m_pMap;
        nSize = pMap->size();
        if( nSize )
        {
            typedef OInterfaceContainerHelper* ppp;
            ppListenerContainers.reset(new ppp[nSize]);

            t_long2ptr::iterator iter = pMap->begin();
            t_long2ptr::iterator end = pMap->end();

            t_long2ptr::size_type i = 0;
            while( iter != end )
            {
                ppListenerContainers[i++] = (OInterfaceContainerHelper*)(*iter).second;
                ++iter;
            }
        }
    }

    // create a copy, because do not fire event in a guarded section
    for( t_long2ptr::size_type i = 0;
            i < nSize; i++ )
    {
        if( ppListenerContainers[i] )
            ppListenerContainers[i]->disposeAndClear( rEvt );
    }
}

void OMultiTypeInterfaceContainerHelperInt32::clear()
{
    ::osl::MutexGuard aGuard( rMutex );
    if (!m_pMap)
        return;
    t_long2ptr * pMap = (t_long2ptr *)m_pMap;
    t_long2ptr::iterator iter = pMap->begin();
    t_long2ptr::iterator end = pMap->end();

    while( iter != end )
    {
        ((OInterfaceContainerHelper*)(*iter).second)->clear();
        ++iter;
    }
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
