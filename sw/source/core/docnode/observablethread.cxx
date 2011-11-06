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
#include <observablethread.hxx>

#include <boost/shared_ptr.hpp>

/** class for an observable thread

    OD 2007-01-29 #i73788#

    @author OD
*/
ObservableThread::ObservableThread()
    : mnRefCount( 0 ),
      mnThreadID( 0 ),
      mpThreadListener()
{
}

ObservableThread::~ObservableThread()
{
}

oslInterlockedCount ObservableThread::acquire()
{
    return osl_incrementInterlockedCount( &mnRefCount );
}

oslInterlockedCount ObservableThread::release()
{
    oslInterlockedCount nCount( osl_decrementInterlockedCount( &mnRefCount ) );
    if ( nCount == 0 )
    {
        delete this;
        return nCount;
    }

    return nCount;
}

void ObservableThread::SetListener( boost::weak_ptr< IFinishedThreadListener > pThreadListener,
                                    const oslInterlockedCount nThreadID )
{
    mpThreadListener = pThreadListener;
    mnThreadID = nThreadID;
}

void SAL_CALL ObservableThread::run()
{
    acquire();

    threadFunction();
}

void SAL_CALL ObservableThread::onTerminated()
{
    threadFinished();

    // notify observer
    boost::shared_ptr< IFinishedThreadListener > pThreadListener = mpThreadListener.lock();
    if ( pThreadListener )
    {
        pThreadListener->NotifyAboutFinishedThread( mnThreadID );
    }

    release();
}

void ObservableThread::threadFinished()
{
    // empty default implementation
}
