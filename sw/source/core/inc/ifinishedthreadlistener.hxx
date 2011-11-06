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


#ifndef _IFINISHEDTHREADLISTENER_HXX
#define _IFINISHEDTHREADLISTENER_HXX

#include <osl/interlck.h>

/** interface class to listen on the finish of a thread

    OD 2007-03-30 #i73788#
    Note: The thread provides its ThreadID on the finish notification

    @author OD
*/
class IFinishedThreadListener
{
    public:

        inline virtual ~IFinishedThreadListener()
        {
        };

        virtual void NotifyAboutFinishedThread( const oslInterlockedCount nThreadID ) = 0;

    protected:

        inline IFinishedThreadListener()
        {
        };
};
#endif
