/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


#include "precompiled_sw.hxx"
#include <retrieveinputstreamconsumer.hxx>
#include <ndgrf.hxx>
#include <retrieveinputstream.hxx>
#include <swthreadmanager.hxx>

/** class to provide creation of a thread to retrieve an input stream given by
    an URL and to consume the retrieved input stream.

    OD 2007-01-29 #i73788#

    @author OD
*/
SwAsyncRetrieveInputStreamThreadConsumer::SwAsyncRetrieveInputStreamThreadConsumer(
                                                            SwGrfNode& rGrfNode )
    : mrGrfNode( rGrfNode ),
      mnThreadID( 0 )
{
}

SwAsyncRetrieveInputStreamThreadConsumer::~SwAsyncRetrieveInputStreamThreadConsumer()
{
    SwThreadManager::GetThreadManager().RemoveThread( mnThreadID );
}

void SwAsyncRetrieveInputStreamThreadConsumer::CreateThread( const String& rURL )
{
    // Get new data container for input stream data
    SwRetrievedInputStreamDataManager::tDataKey nDataKey =
            SwRetrievedInputStreamDataManager::GetManager().ReserveData(
                                                mrGrfNode.GetThreadConsumer() );

    rtl::Reference< ObservableThread > pNewThread =
        SwAsyncRetrieveInputStreamThread::createThread( nDataKey, rURL );

    // Add thread to thread manager and pass ownership of thread to thread manager.
    mnThreadID = SwThreadManager::GetThreadManager().AddThread( pNewThread );
}

void SwAsyncRetrieveInputStreamThreadConsumer::ApplyInputStream(
    com::sun::star::uno::Reference<com::sun::star::io::XInputStream> xInputStream,
    const sal_Bool bIsStreamReadOnly )
{
    mrGrfNode.ApplyInputStream( xInputStream, bIsStreamReadOnly );
}

