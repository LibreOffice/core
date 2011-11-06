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



#ifndef _COMPHELPER_BROADCASTHELPER_HXX_
#define _COMPHELPER_BROADCASTHELPER_HXX_

#include <osl/mutex.hxx>
#include <cppuhelper/interfacecontainer.h>

//... namespace comphelper .......................................................
namespace comphelper
{
//.........................................................................

    //==================================================================================
    //= OMutexAndBroadcastHelper - a class which holds a Mutex and a OBroadcastHelper;
    //=                 needed because when deriving from OPropertySetHelper,
    //=                 the OBroadcastHelper has to be initialized before
    //=                 the OPropertySetHelper
    //==================================================================================
    class OMutexAndBroadcastHelper
    {
    protected:
        ::osl::Mutex                m_aMutex;
        ::cppu::OBroadcastHelper    m_aBHelper;

    public:
        OMutexAndBroadcastHelper() : m_aBHelper( m_aMutex ) { }

        ::osl::Mutex&                   GetMutex()                  { return m_aMutex; }
        ::cppu::OBroadcastHelper&       GetBroadcastHelper()        { return m_aBHelper; }
        const ::cppu::OBroadcastHelper& GetBroadcastHelper() const  { return m_aBHelper; }

    };

    // base class for all classes who are derived from OPropertySet and from OComponent
    // @deprecated, you should use cppu::BaseMutex instead (cppuhelper/basemutex.hxx)

    class OBaseMutex
    {
    protected:
        mutable ::osl::Mutex m_aMutex;
    };
}
#endif // _COMPHELPER_BROADCASTHELPER_HXX_
