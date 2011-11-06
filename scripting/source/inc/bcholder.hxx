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



#ifndef SCRIPTING_BCHOLDER_HXX
#define SCRIPTING_BCHOLDER_HXX

#include <osl/mutex.hxx>
#include <cppuhelper/interfacecontainer.h>


//.........................................................................
namespace scripting_helper
{
//.........................................................................

    //  ----------------------------------------------------
    //  class OMutexHolder
    //  ----------------------------------------------------

    class OMutexHolder
    {
    protected:
        ::osl::Mutex m_aMutex;
    };

    //  ----------------------------------------------------
    //  class OBroadcastHelperHolder
    //  ----------------------------------------------------

    class OBroadcastHelperHolder
    {
    protected:
        ::cppu::OBroadcastHelper    m_aBHelper;

    public:
        OBroadcastHelperHolder( ::osl::Mutex& rMutex ) : m_aBHelper( rMutex ) { }

        ::cppu::OBroadcastHelper&       GetBroadcastHelper()        { return m_aBHelper; }
        const ::cppu::OBroadcastHelper& GetBroadcastHelper() const  { return m_aBHelper; }
    };

//.........................................................................
}   // namespace scripting_helper
//.........................................................................

#endif // SCRIPTING_BCHOLDER_HXX
