/*************************************************************************
 *
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

