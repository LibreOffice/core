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
#include <cppuhelper/propshlp.hxx>
#include <comphelper/sequence.hxx>

#include <osl/diagnose.h>
#include <osl/mutex.hxx>
#include <sal/log.hxx>

#include <memory>

#include <com/sun/star/lang/XEventListener.hpp>


using namespace osl;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;

namespace cppu
{

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
        rCont.bInUse = true;
        nRemain = aData.pAsVector->size();
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
    bShared = aData.pAsVector == rCont.aData.pAsVector && rCont.bIsList;
    if( bShared )
    {
        OSL_ENSURE( rCont.bInUse, "OInterfaceContainerHelper must be in use" );
        rCont.bInUse = false;
    }
    }

    if( !bShared )
    {
        if( bIsList )
            // Sequence owned by the iterator
            delete aData.pAsVector;
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
            return (*aData.pAsVector)[nRemain].get();
        if( aData.pAsInterface )
            return aData.pAsInterface;
    }
    // exception
    return nullptr;
}

void OInterfaceIteratorHelper::remove()
{
    if( bIsList )
    {
        OSL_ASSERT( nRemain >= 0 &&
                    nRemain < static_cast<sal_Int32>(aData.pAsVector->size()) );
        XInterface * p = (*aData.pAsVector)[nRemain].get();
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
    , bInUse( false )
    , bIsList( false )
{
}

OInterfaceContainerHelper::~OInterfaceContainerHelper()
{
    OSL_ENSURE( !bInUse, "~OInterfaceContainerHelper but is in use" );
    if( bIsList )
        delete aData.pAsVector;
    else if( aData.pAsInterface )
        aData.pAsInterface->release();
}

sal_Int32 OInterfaceContainerHelper::getLength() const
{
    MutexGuard aGuard( rMutex );
    if( bIsList )
        return aData.pAsVector->size();
    if( aData.pAsInterface )
        return 1;
    return 0;
}

Sequence< Reference<XInterface> > OInterfaceContainerHelper::getElements() const
{
    MutexGuard aGuard( rMutex );
    if( bIsList )
        return comphelper::containerToSequence(*aData.pAsVector);
    if( aData.pAsInterface )
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
        // this should be the worst case. If an iterator is active
        // and a new Listener is added.
        if( bIsList )
            aData.pAsVector= new std::vector< Reference< XInterface > >( *aData.pAsVector );
        else if( aData.pAsInterface )
            aData.pAsInterface->acquire();

        bInUse = false;
    }
}

sal_Int32 OInterfaceContainerHelper::addInterface( const Reference<XInterface> & rListener )
{
    SAL_WARN_IF( !rListener.is(), "cppuhelper", "rListener is empty" );
    MutexGuard aGuard( rMutex );
    if( bInUse )
        copyAndResetInUse();

    if( bIsList )
    {
        aData.pAsVector->push_back(rListener);
        return aData.pAsVector->size();
    }
    if( aData.pAsInterface )
    {
        Reference<XInterface> tmp(aData.pAsInterface);
        aData.pAsInterface->release();
        aData.pAsVector = new std::vector<Reference<XInterface>>(2);
        (*aData.pAsVector)[0] = std::move(tmp);
        (*aData.pAsVector)[1] = rListener;
        bIsList = true;
        return 2;
    }
    aData.pAsInterface = rListener.get();
    if( rListener.is() )
        rListener->acquire();
    return 1;
}

sal_Int32 OInterfaceContainerHelper::removeInterface( const Reference<XInterface> & rListener )
{
    SAL_WARN_IF( !rListener.is(), "cppuhelper", "rListener is empty" );
    MutexGuard aGuard( rMutex );
    if( bInUse )
        copyAndResetInUse();

    if( bIsList )
    {
        // It is not valid to compare the pointer directly, but it's faster.
        auto findIt = std::find_if(aData.pAsVector->begin(), aData.pAsVector->end(),
                    [&](const Reference<XInterface>& r)
                    { return r.get() == rListener.get(); });
        if (findIt != aData.pAsVector->end())
        {
            aData.pAsVector->erase(findIt);
        }
        else
        {
            // interface not found, use the correct compare method
            for( auto it = aData.pAsVector->begin(); it != aData.pAsVector->end(); ++it )
            {
                if( *it == rListener )
                {
                    aData.pAsVector->erase(it);
                    break;
                }
            }
        }

        if( aData.pAsVector->size() == 1 )
        {
            XInterface * p = (*aData.pAsVector)[0].get();
            p->acquire();
            delete aData.pAsVector;
            aData.pAsInterface = p;
            bIsList = false;
            return 1;
        }
        return aData.pAsVector->size();
    }
    if( aData.pAsInterface && Reference<XInterface>( aData.pAsInterface ) == rListener )
    {
        aData.pAsInterface->release();
        aData.pAsInterface = nullptr;
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
    aData.pAsInterface = nullptr;
    bIsList = false;
    bInUse = false;
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
    MutexGuard aGuard( rMutex );
    // Release container, in case new entries come while disposing
    OSL_ENSURE( !bIsList || bInUse, "OInterfaceContainerHelper not in use" );
    if (bInUse)
        copyAndResetInUse();
    if (bIsList)
        delete aData.pAsVector;
    else if (aData.pAsInterface)
        aData.pAsInterface->release();
    aData.pAsInterface = nullptr;
    bIsList = false;
}

// specialized class for type

typedef std::vector< std::pair < Type , void* > > t_type2ptr;

OMultiTypeInterfaceContainerHelper::OMultiTypeInterfaceContainerHelper( Mutex & rMutex_ )
    : rMutex( rMutex_ )
{
    m_pMap = new t_type2ptr;
}

OMultiTypeInterfaceContainerHelper::~OMultiTypeInterfaceContainerHelper()
{
    t_type2ptr * pMap = static_cast<t_type2ptr *>(m_pMap);

    for (auto& rItem : *pMap)
    {
        delete static_cast<OInterfaceContainerHelper*>(rItem.second);
        rItem.second = nullptr;
    }
    delete pMap;
}

Sequence< Type > OMultiTypeInterfaceContainerHelper::getContainedTypes() const
{
    t_type2ptr * pMap = static_cast<t_type2ptr *>(m_pMap);
    t_type2ptr::size_type nSize;

    ::osl::MutexGuard aGuard( rMutex );
    nSize = pMap->size();
    if( nSize )
    {
        css::uno::Sequence< Type > aInterfaceTypes( nSize );
        Type * pArray = aInterfaceTypes.getArray();

        sal_Int32 i = 0;
        for (const auto& rItem : *pMap)
        {
            // are interfaces added to this container?
            if( static_cast<OInterfaceContainerHelper*>(rItem.second)->getLength() )
                // yes, put the type in the array
                pArray[i++] = rItem.first;
        }
        if( static_cast<t_type2ptr::size_type>(i) != nSize ) {
            // may be empty container, reduce the sequence to the right size
            aInterfaceTypes = css::uno::Sequence< Type >( pArray, i );
        }
        return aInterfaceTypes;
    }
    return css::uno::Sequence< Type >();
}

static t_type2ptr::iterator findType(t_type2ptr *pMap, const Type & rKey )
{
    return std::find_if(pMap->begin(), pMap->end(),
        [&rKey](const t_type2ptr::value_type& rItem) { return rItem.first == rKey; });
}

OInterfaceContainerHelper * OMultiTypeInterfaceContainerHelper::getContainer( const Type & rKey ) const
{
    ::osl::MutexGuard aGuard( rMutex );

    t_type2ptr * pMap = static_cast<t_type2ptr *>(m_pMap);
    t_type2ptr::iterator iter = findType( pMap, rKey );
    if( iter != pMap->end() )
            return static_cast<OInterfaceContainerHelper*>((*iter).second);
    return nullptr;
}

sal_Int32 OMultiTypeInterfaceContainerHelper::addInterface(
    const Type & rKey, const Reference< XInterface > & rListener )
{
    ::osl::MutexGuard aGuard( rMutex );
    t_type2ptr * pMap = static_cast<t_type2ptr *>(m_pMap);
    t_type2ptr::iterator iter = findType( pMap, rKey );
    if( iter == pMap->end() )
    {
        OInterfaceContainerHelper * pLC = new OInterfaceContainerHelper( rMutex );
        pMap->push_back(std::pair<Type, void*>(rKey, pLC));
        return pLC->addInterface( rListener );
    }
    return static_cast<OInterfaceContainerHelper*>((*iter).second)->addInterface( rListener );
}

sal_Int32 OMultiTypeInterfaceContainerHelper::removeInterface(
    const Type & rKey, const Reference< XInterface > & rListener )
{
    ::osl::MutexGuard aGuard( rMutex );

    // search container with id nUik
    t_type2ptr * pMap = static_cast<t_type2ptr *>(m_pMap);
    t_type2ptr::iterator iter = findType( pMap, rKey );
        // container found?
    if( iter != pMap->end() )
        return static_cast<OInterfaceContainerHelper*>((*iter).second)->removeInterface( rListener );

    // no container with this id. Always return 0
    return 0;
}

void OMultiTypeInterfaceContainerHelper::disposeAndClear( const EventObject & rEvt )
{
    t_type2ptr::size_type nSize = 0;
    std::unique_ptr<OInterfaceContainerHelper *[]> ppListenerContainers;
    {
        ::osl::MutexGuard aGuard( rMutex );
        t_type2ptr * pMap = static_cast<t_type2ptr *>(m_pMap);
        nSize = pMap->size();
        if( nSize )
        {
            typedef OInterfaceContainerHelper* ppp;
            ppListenerContainers.reset(new ppp[nSize]);
            //ppListenerContainers = new (ListenerContainer*)[nSize];

            t_type2ptr::size_type i = 0;
            for (const auto& rItem : *pMap)
            {
                ppListenerContainers[i++] = static_cast<OInterfaceContainerHelper*>(rItem.second);
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
    t_type2ptr * pMap = static_cast<t_type2ptr *>(m_pMap);

    for (auto& rItem : *pMap)
    {
        static_cast<OInterfaceContainerHelper*>(rItem.second)->clear();
    }
}

// specialized class for long

typedef std::vector< std::pair < sal_Int32 , void* > > t_long2ptr;

static t_long2ptr::iterator findLong(t_long2ptr *pMap, sal_Int32 nKey )
{
    return std::find_if(pMap->begin(), pMap->end(),
        [&nKey](const t_long2ptr::value_type& rItem) { return rItem.first == nKey; });
}

OMultiTypeInterfaceContainerHelperInt32::OMultiTypeInterfaceContainerHelperInt32( Mutex & rMutex_ )
    : m_pMap( nullptr )
    , rMutex( rMutex_ )
{
    // delay pMap allocation until necessary.
}

OMultiTypeInterfaceContainerHelperInt32::~OMultiTypeInterfaceContainerHelperInt32()
{
    if (!m_pMap)
        return;

    t_long2ptr * pMap = static_cast<t_long2ptr *>(m_pMap);

    for (auto& rItem : *pMap)
    {
        delete static_cast<OInterfaceContainerHelper*>(rItem.second);
        rItem.second = nullptr;
    }
    delete pMap;
}

Sequence< sal_Int32 > OMultiTypeInterfaceContainerHelperInt32::getContainedTypes() const
{
    t_long2ptr * pMap = static_cast<t_long2ptr *>(m_pMap);
    t_long2ptr::size_type nSize;

    ::osl::MutexGuard aGuard( rMutex );
    nSize = pMap ? pMap->size() : 0;
    if( nSize )
    {
        css::uno::Sequence< sal_Int32 > aInterfaceTypes( nSize );
        sal_Int32 * pArray = aInterfaceTypes.getArray();

        sal_Int32 i = 0;
        for (const auto& rItem : *pMap)
        {
            // are interfaces added to this container?
            if( static_cast<OInterfaceContainerHelper*>(rItem.second)->getLength() )
                // yes, put the type in the array
                pArray[i++] = rItem.first;
        }
        if( static_cast<t_long2ptr::size_type>(i) != nSize ) {
            // may be empty container, reduce the sequence to the right size
            aInterfaceTypes = css::uno::Sequence< sal_Int32 >( pArray, i );
        }
        return aInterfaceTypes;
    }
    return css::uno::Sequence< sal_Int32 >();
}

OInterfaceContainerHelper * OMultiTypeInterfaceContainerHelperInt32::getContainer( const sal_Int32 & rKey ) const
{
    ::osl::MutexGuard aGuard( rMutex );

    if (!m_pMap)
        return nullptr;
    t_long2ptr * pMap = static_cast<t_long2ptr *>(m_pMap);
    t_long2ptr::iterator iter = findLong( pMap, rKey );
    if( iter != pMap->end() )
            return static_cast<OInterfaceContainerHelper*>((*iter).second);
    return nullptr;
}

sal_Int32 OMultiTypeInterfaceContainerHelperInt32::addInterface(
    const sal_Int32 & rKey, const Reference< XInterface > & rListener )
{
    ::osl::MutexGuard aGuard( rMutex );
    if (!m_pMap)
        m_pMap = new t_long2ptr;
    t_long2ptr * pMap = static_cast<t_long2ptr *>(m_pMap);
    t_long2ptr::iterator iter = findLong( pMap, rKey );
    if( iter == pMap->end() )
    {
        OInterfaceContainerHelper * pLC = new OInterfaceContainerHelper( rMutex );
        pMap->push_back(std::pair< sal_Int32, void* >(rKey, pLC));
        return pLC->addInterface( rListener );
    }
    return static_cast<OInterfaceContainerHelper*>((*iter).second)->addInterface( rListener );
}

sal_Int32 OMultiTypeInterfaceContainerHelperInt32::removeInterface(
    const sal_Int32 & rKey, const Reference< XInterface > & rListener )
{
    ::osl::MutexGuard aGuard( rMutex );

    if (!m_pMap)
        return 0;
    // search container with id nUik
    t_long2ptr * pMap = static_cast<t_long2ptr *>(m_pMap);
    t_long2ptr::iterator iter = findLong( pMap, rKey );
        // container found?
    if( iter != pMap->end() )
        return static_cast<OInterfaceContainerHelper*>((*iter).second)->removeInterface( rListener );

    // no container with this id. Always return 0
    return 0;
}

void OMultiTypeInterfaceContainerHelperInt32::disposeAndClear( const EventObject & rEvt )
{
    t_long2ptr::size_type nSize = 0;
    std::unique_ptr<OInterfaceContainerHelper *[]> ppListenerContainers;
    {
        ::osl::MutexGuard aGuard( rMutex );
        if (!m_pMap)
            return;

        t_long2ptr * pMap = static_cast<t_long2ptr *>(m_pMap);
        nSize = pMap->size();
        if( nSize )
        {
            typedef OInterfaceContainerHelper* ppp;
            ppListenerContainers.reset(new ppp[nSize]);

            t_long2ptr::size_type i = 0;
            for (const auto& rItem : *pMap)
            {
                ppListenerContainers[i++] = static_cast<OInterfaceContainerHelper*>(rItem.second);
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
    t_long2ptr * pMap = static_cast<t_long2ptr *>(m_pMap);

    for (auto& rItem : *pMap)
    {
        static_cast<OInterfaceContainerHelper*>(rItem.second)->clear();
    }
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
