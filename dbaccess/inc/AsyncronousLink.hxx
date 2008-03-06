/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: AsyncronousLink.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-06 17:50:48 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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
        ULONG               m_nEventId;

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

