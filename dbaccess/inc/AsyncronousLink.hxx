/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: AsyncronousLink.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2007-07-06 07:47:48 $
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

#ifndef _LINK_HXX
#include <tools/link.hxx>
#endif
#ifndef _VOS_MUTEX_HXX_
#include <vos/mutex.hxx>
#endif
#include "dbaccessdllapi.h"

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
    class DBACCESS_DLLPUBLIC OAsyncronousLink
    {
        Link                    m_aHandler;

    protected:
        ::vos::OMutex*      m_pEventSafety;
        ::vos::OMutex*      m_pDestructionSafety;
        BOOL                m_bOwnMutexes;
        ULONG               m_nEventId;

    public:
        /** constructs the object
            @param      _rHandler           The link to be called asyncronously
            @param      _pEventSafety       A pointer to a mutex to be used for event safety. See below.
            @param      _pDestructionSafety A pointer to a mutex to be used for destruction safety. See below.

            For a really safe behaviour two mutexes are required. In case your class needs more than one
            OAsyncronousLink instance, you may want to share these mutexes (and hold them as members of
            your class). Thus, if _pEventSafety and _pDestructionSafety are both non-NULL, they will be
            used for the thread-safety of the link. Otherwise the instance will create it's own mutexes.
            <BR>
            If you use this "mutex feature" be aware of the lifetime of all involved objects : the mutexes
            you specify upon construction should live (at least) as long as the OAsyncronousLink object does.
        */
        OAsyncronousLink(const Link& _rHandler, ::vos::OMutex* _pEventSafety = NULL, ::vos::OMutex* _pDestructionSafety = NULL);
        virtual ~OAsyncronousLink();

        bool    IsRunning() const { return m_nEventId != 0; }


        void Call(void* _pArgument = NULL);
        void CancelCall();

    protected:
        DECL_LINK(OnAsyncCall, void*);
    };
}
#endif // DBAUI_ASYNCRONOUSLINK_HXX

