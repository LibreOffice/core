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
#include <comphelper/interfacecontainer2.hxx>
#include <cppuhelper/queryinterface.hxx>
#include <cppuhelper/propshlp.hxx>

#include <osl/diagnose.h>
#include <osl/mutex.hxx>

#include <memory>

#include <com/sun/star/lang/XEventListener.hpp>


using namespace osl;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;

namespace comphelper
{

OInterfaceIteratorHelper2::OInterfaceIteratorHelper2( OInterfaceContainerHelper2 & rCont_ )
    : rCont( rCont_ ),
      bIsList( rCont_.bIsList )
{
    MutexGuard aGuard( rCont.rMutex );
    if( rCont.bInUse )
        // worst case, two iterators at the same time
        rCont.copyAndResetInUse();
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

OInterfaceIteratorHelper2::~OInterfaceIteratorHelper2()
{
    bool bShared;
    {
    MutexGuard aGuard( rCont.rMutex );
    // bResetInUse protect the iterator against recursion
    bShared = aData.pAsVector == rCont.aData.pAsVector && rCont.bIsList;
    if( bShared )
    {
        OSL_ENSURE( rCont.bInUse, "OInterfaceContainerHelper2 must be in use" );
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

XInterface * OInterfaceIteratorHelper2::next()
{
    if( nRemain )
    {
        nRemain--;
        if( bIsList )
            return (*aData.pAsVector)[nRemain].get();
        else if( aData.pAsInterface )
            return aData.pAsInterface;
    }
    // exception
    return nullptr;
}

void OInterfaceIteratorHelper2::remove()
{
    if( bIsList )
    {
        OSL_ASSERT( nRemain >= 0 &&
                    nRemain < static_cast<sal_Int32>(aData.pAsVector->size()) );
        rCont.removeInterface(  (*aData.pAsVector)[nRemain] );
    }
    else
    {
        OSL_ASSERT( 0 == nRemain );
        rCont.removeInterface( aData.pAsInterface );
    }
}

OInterfaceContainerHelper2::OInterfaceContainerHelper2( Mutex & rMutex_ )
    : rMutex( rMutex_ )
    , bInUse( false )
    , bIsList( false )
{
}

OInterfaceContainerHelper2::~OInterfaceContainerHelper2()
{
    OSL_ENSURE( !bInUse, "~OInterfaceContainerHelper2 but is in use" );
    if( bIsList )
        delete aData.pAsVector;
    else if( aData.pAsInterface )
        aData.pAsInterface->release();
}

sal_Int32 OInterfaceContainerHelper2::getLength() const
{
    MutexGuard aGuard( rMutex );
    if( bIsList )
        return aData.pAsVector->size();
    else if( aData.pAsInterface )
        return 1;
    return 0;
}

std::vector< Reference<XInterface> > OInterfaceContainerHelper2::getElements() const
{
    std::vector< Reference<XInterface> > rVec;
    MutexGuard aGuard( rMutex );
    if( bIsList )
        rVec = *aData.pAsVector;
    else if( aData.pAsInterface )
    {
        rVec.emplace_back( aData.pAsInterface );
    }
    return rVec;
}

void OInterfaceContainerHelper2::copyAndResetInUse()
{
    OSL_ENSURE( bInUse, "OInterfaceContainerHelper2 not in use" );
    if( bInUse )
    {
        // this should be the worst case. If a iterator is active
        // and a new Listener is added.
        if( bIsList )
            aData.pAsVector = new std::vector< Reference< XInterface > >( *aData.pAsVector );
        else if( aData.pAsInterface )
            aData.pAsInterface->acquire();

        bInUse = false;
    }
}

sal_Int32 OInterfaceContainerHelper2::addInterface( const Reference<XInterface> & rListener )
{
    OSL_ASSERT( rListener.is() );
    MutexGuard aGuard( rMutex );
    if( bInUse )
        copyAndResetInUse();

    if( bIsList )
    {
        aData.pAsVector->push_back(  rListener );
        return aData.pAsVector->size();
    }
    else if( aData.pAsInterface )
    {
        std::vector< Reference< XInterface > > * pVec = new std::vector< Reference< XInterface > >( 2 );
        (*pVec)[0] = aData.pAsInterface;
        (*pVec)[1] = rListener;
        aData.pAsInterface->release();
        aData.pAsVector = pVec;
        bIsList = true;
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

sal_Int32 OInterfaceContainerHelper2::removeInterface( const Reference<XInterface> & rListener )
{
    OSL_ASSERT( rListener.is() );
    MutexGuard aGuard( rMutex );
    if( bInUse )
        copyAndResetInUse();

    if( bIsList )
    {
        // It is not valid to compare the pointer directly, but it's faster.
        auto it = std::find_if(aData.pAsVector->begin(), aData.pAsVector->end(),
            [&rListener](const css::uno::Reference<css::uno::XInterface>& rItem) {
                return rItem.get() == rListener.get(); });

        // interface not found, use the correct compare method
        if (it == aData.pAsVector->end())
            it = std::find(aData.pAsVector->begin(), aData.pAsVector->end(), rListener);

        if (it != aData.pAsVector->end())
            aData.pAsVector->erase(it);

        if( aData.pAsVector->size() == 1 )
        {
            XInterface * p = (*aData.pAsVector)[0].get();
            p->acquire();
            delete aData.pAsVector;
            aData.pAsInterface = p;
            bIsList = false;
            return 1;
        }
        else
            return aData.pAsVector->size();
    }
    else if( aData.pAsInterface && Reference<XInterface>( aData.pAsInterface ) == rListener )
    {
        aData.pAsInterface->release();
        aData.pAsInterface = nullptr;
    }
    return aData.pAsInterface ? 1 : 0;
}

void OInterfaceContainerHelper2::disposeAndClear( const EventObject & rEvt )
{
    ClearableMutexGuard aGuard( rMutex );
    OInterfaceIteratorHelper2 aIt( *this );
    // Release container, in case new entries come while disposing
    OSL_ENSURE( !bIsList || bInUse, "OInterfaceContainerHelper2 not in use" );
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


void OInterfaceContainerHelper2::clear()
{
    ClearableMutexGuard aGuard( rMutex );
    OInterfaceIteratorHelper2 aIt( *this );
    // Release container, in case new entries come while disposing
    OSL_ENSURE( !bIsList || bInUse, "OInterfaceContainerHelper2 not in use" );
    if( !bIsList && aData.pAsInterface )
        aData.pAsInterface->release();
    // set the member to null, use the iterator to delete the values
    aData.pAsInterface = nullptr;
    bIsList = false;
    bInUse = false;
    // release mutex before aIt destructor call
    aGuard.clear();
}


}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
