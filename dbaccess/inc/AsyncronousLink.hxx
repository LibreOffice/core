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



#ifndef DBAUI_ASYNCRONOUSLINK_HXX
#define DBAUI_ASYNCRONOUSLINK_HXX

#include <tools/link.hxx>
#include <osl/mutex.hxx>

namespace dbaui
{
    // =========================================================================
    // a helper for multi-threaded handling of async events
    // -------------------------------------------------------------------------
    /** handles asynchronous links which may be called in multi-threaded environments
        If you use an instance of this class as member of your own class, it will handle
        several crucial points for you (for instance the case that somebody posts the
        event while another thread tries to delete this event in the _destructor_ of the
        class).
    */
    class OAsyncronousLink
    {
        Link                m_aHandler;

    protected:
        ::osl::Mutex        m_aEventSafety;
        ::osl::Mutex        m_aDestructionSafety;
        sal_uLong               m_nEventId;

    public:
        /** constructs the object
            @param      _rHandler           The link to be called asyncronously
        */
        OAsyncronousLink( const Link& _rHandler );
        virtual ~OAsyncronousLink();

        bool    IsRunning() const { return m_nEventId != 0; }

        void Call( void* _pArgument = NULL );
        void CancelCall();

    protected:
        DECL_LINK(OnAsyncCall, void*);
    };
}
#endif // DBAUI_ASYNCRONOUSLINK_HXX

