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



#ifndef SVX_DELAYEDLINK_HXX
#define SVX_DELAYEDLINK_HXX

#include <tools/link.hxx>

//........................................................................
namespace svxform
{
//........................................................................

    //====================================================================
    //= DelayedEvent
    //====================================================================
    /** small class which encapsulates posting a Link instance as ApplicationUserEvent

        No thread safety at all here, just a little saving of code to type multiple times
    */
    class DelayedEvent
    {
    public:
        DelayedEvent( const Link& _rHandler )
            :m_aHandler( _rHandler )
            ,m_nEventId( 0 )
        {
        }

        ~DelayedEvent()
        {
            CancelPendingCall();
        }

        /** calls the handler asynchronously

            If there's already a call pending, this previous call is cancelled.
        */
        void    Call( void* _pArg = NULL );

        /** cancels a call which is currently pending

            If no call is currently pending, then this is ignored.
        */
        void    CancelPendingCall();

    private:
        Link    m_aHandler;
        sal_uLong   m_nEventId;

    private:
        DECL_LINK( OnCall, void* );

    private:
        DelayedEvent();                                  // never implemented
        DelayedEvent( const DelayedEvent& );              // never implemented
        DelayedEvent& operator=( const DelayedEvent& );   // never implemented
    };

//........................................................................
} // namespace svxform
//........................................................................

#endif // SVX_DELAYEDLINK_HXX
