/*************************************************************************
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
