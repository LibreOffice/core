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



#ifndef INCLUDED_CANVAS_BASEMUTEXHELPER_HXX
#define INCLUDED_CANVAS_BASEMUTEXHELPER_HXX

#include <osl/mutex.hxx>


/* Definition of the BaseMutexHelper class */

namespace canvas
{
    /** Base class, deriving from ::comphelper::OBaseMutex and
        initializing its own baseclass with m_aMutex.

        This is necessary to make the CanvasBase, GraphicDeviceBase,
        etc. classes freely combinable - letting them perform this
        initialization would prohibit deriving e.g. CanvasBase from
        GraphicDeviceBase.
     */
    template< class Base > class BaseMutexHelper : public Base
    {
    public:
        typedef Base BaseType;

        /** Construct BaseMutexHelper

            This method is the whole purpose of this template:
            initializing a base class with the provided m_aMutex
            member (the WeakComponentImplHelper templates need that,
            as they require the lifetime of the mutex to extend
            theirs).
         */
        BaseMutexHelper() :
            BaseType( m_aMutex )
        {
        }

protected:
        mutable ::osl::Mutex m_aMutex;
    };
}

#endif /* INCLUDED_CANVAS_BASEMUTEXHELPER_HXX */
