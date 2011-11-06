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


#ifndef _ITHREADLISTENEROWNER_HXX
#define _ITHREADLISTENEROWNER_HXX

#include <ifinishedthreadlistener.hxx>

#include <boost/weak_ptr.hpp>

/** interface class of the owner of a thread listener

    OD 2007-01-30 #i73788#

    @author OD
*/
class IThreadListenerOwner
{
    public:

        inline virtual ~IThreadListenerOwner()
        {
        };

        virtual boost::weak_ptr< IFinishedThreadListener > GetThreadListenerWeakRef() = 0;

        virtual void NotifyAboutFinishedThread( const oslInterlockedCount nThreadID ) = 0;

    protected:

        inline IThreadListenerOwner()
        {
        };
};
#endif
