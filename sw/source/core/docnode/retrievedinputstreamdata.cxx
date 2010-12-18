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
#include "precompiled_sw.hxx"
#include <retrievedinputstreamdata.hxx>
#include <retrieveinputstreamconsumer.hxx>
#include <vcl/svapp.hxx>

/** implementation of class <SwRetrievedInputStreamDataManager>

    OD 2007-01-30 #i73788#
*/
SwRetrievedInputStreamDataManager* SwRetrievedInputStreamDataManager::mpManager = 0;
SwRetrievedInputStreamDataManager::tDataKey SwRetrievedInputStreamDataManager::mnNextKeyValue = 1;
osl::Mutex SwRetrievedInputStreamDataManager::maGetManagerMutex;

SwRetrievedInputStreamDataManager& SwRetrievedInputStreamDataManager::GetManager()
{
    osl::MutexGuard aGuard(maGetManagerMutex);

    if ( mpManager == 0 )
    {
        mpManager = new SwRetrievedInputStreamDataManager();
    }

    return *mpManager;
}

SwRetrievedInputStreamDataManager::tDataKey SwRetrievedInputStreamDataManager::ReserveData(
                        boost::weak_ptr< SwAsyncRetrieveInputStreamThreadConsumer > pThreadConsumer )
{
    osl::MutexGuard aGuard(maMutex);

    // create empty data container for given thread Consumer
    tDataKey nDataKey( mnNextKeyValue );
    tData aNewEntry( pThreadConsumer );
    maInputStreamData[ nDataKey ] = aNewEntry;

    // prepare next data key value
    if ( mnNextKeyValue < SAL_MAX_UINT64 )
    {
        ++mnNextKeyValue;
    }
    else
    {
        mnNextKeyValue = 1;
    }

    return nDataKey;
}

void SwRetrievedInputStreamDataManager::PushData(
        const tDataKey nDataKey,
        com::sun::star::uno::Reference<com::sun::star::io::XInputStream> xInputStream,
        const sal_Bool bIsStreamReadOnly )
{
    osl::MutexGuard aGuard(maMutex);

    std::map< tDataKey, tData >::iterator aIter = maInputStreamData.find( nDataKey );

    if ( aIter != maInputStreamData.end() )
    {
        // Fill data container.
        (*aIter).second.mxInputStream = xInputStream;
        (*aIter).second.mbIsStreamReadOnly = bIsStreamReadOnly;

        // post user event to process the retrieved input stream data
        if ( GetpApp() )
        {

            tDataKey* pDataKey = new tDataKey;
            *pDataKey = nDataKey;
            GetpApp()->PostUserEvent( LINK( this, SwRetrievedInputStreamDataManager, LinkedInputStreamReady ), pDataKey );
        }
        else
        {
            // no application available -> discard data
            maInputStreamData.erase( aIter );
        }
    }
}

bool SwRetrievedInputStreamDataManager::PopData( const tDataKey nDataKey,
                                                 tData& rData )
{
    osl::MutexGuard aGuard(maMutex);

    bool bDataProvided( false );

    std::map< tDataKey, tData >::iterator aIter = maInputStreamData.find( nDataKey );

    if ( aIter != maInputStreamData.end() )
    {
        rData.mpThreadConsumer = (*aIter).second.mpThreadConsumer;
        rData.mxInputStream = (*aIter).second.mxInputStream;
        rData.mbIsStreamReadOnly = (*aIter).second.mbIsStreamReadOnly;

        maInputStreamData.erase( aIter );

        bDataProvided = true;
    }

    return bDataProvided;
}

/** callback function, which is triggered by input stream data manager on
    filling of the data container to provide retrieved input stream to the
    thread Consumer using <Application::PostUserEvent(..)>

    OD 2007-01-29 #i73788#
    Note: This method has to be run in the main thread.

    @author OD
*/
IMPL_LINK( SwRetrievedInputStreamDataManager,
           LinkedInputStreamReady,
           SwRetrievedInputStreamDataManager::tDataKey*,
           pDataKey )
{
    if ( !pDataKey )
    {
        return 0;
    }

    osl::MutexGuard aGuard(maMutex);

    SwRetrievedInputStreamDataManager& rDataManager =
                            SwRetrievedInputStreamDataManager::GetManager();
    SwRetrievedInputStreamDataManager::tData aInputStreamData;
    if ( rDataManager.PopData( *pDataKey, aInputStreamData ) )
    {
        boost::shared_ptr< SwAsyncRetrieveInputStreamThreadConsumer > pThreadConsumer =
                                    aInputStreamData.mpThreadConsumer.lock();
        if ( pThreadConsumer )
        {
            pThreadConsumer->ApplyInputStream( aInputStreamData.mxInputStream,
                                               aInputStreamData.mbIsStreamReadOnly );
        }
    }
    delete pDataKey;

    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
