/*************************************************************************
 *
 *  $RCSfile: accessibleeventbuffer.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2003-04-24 17:25:25 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2002 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#if !defined INCLUDED_COMPHELPER_ACCESSIBLEEVENTBUFFER_HXX
#define INCLUDED_COMPHELPER_ACCESSIBLEEVENTBUFFER_HXX

#include "com/sun/star/uno/Reference.hxx"
#include "com/sun/star/uno/Sequence.hxx"

#include <vector>

namespace com { namespace sun { namespace star { namespace uno {
    class XInterface;
} } } }
namespace com { namespace sun { namespace star {
    namespace accessibility { struct AccessibleEventObject; }
} } }

namespace comphelper {

/**
   A buffer for AccessibleEventObjects about to be sent to
   XAccessibleEventListeners.

   This buffer records pairs of AccessibleEventObjects and sequences of
   XAccessibleEventListeners.  At a time when it is safe to do so (e.g., when
   no critical mutexes are held), all events can be notified at once.

   This class is not thread-safe in itself, but it is thread-compatible (i.e.,
   all multi-threaded uses of instances of this class have to ensure proper
   mutual exclusion).
 */
class AccessibleEventBuffer
{
public:
    /**
       Create an initially empty buffer.

       Internally uses ::std::vector and thus may throw exceptions thrown by
       operations on ::std::vector, especially ::std::bad_alloc.
     */
    AccessibleEventBuffer();

    /**
       Create a buffer with a copy of another buffer.

       The events from the other buffer are copied, not shared.

       Internally uses ::std::vector and thus may throw exceptions thrown by
       operations on ::std::vector, especially ::std::bad_alloc.
     */
    AccessibleEventBuffer(AccessibleEventBuffer const & rOther);

    /**
       Destroy the buffer.

       If the buffer contains any events that have not yet been sent, those
       events are lost.

       Does not throw any exceptions.
     */
    ~AccessibleEventBuffer();

    /**
       Copy another buffer into this buffer.

       If this buffer contained any events that had not yet been sent, those
       events are lost.  The events from the other buffer are copied, not
       shared.

       Internally uses ::std::vector and thus may throw exceptions thrown by
       operations on ::std::vector, especially ::std::bad_alloc.
     */
    AccessibleEventBuffer operator =(AccessibleEventBuffer const & rOther);

    /**
       Add an event to the buffer.

       Internally uses ::std::vector and thus may throw exceptions thrown by
       operations on ::std::vector, especially ::std::bad_alloc.
     */
    void addEvent(
        ::com::sun::star::accessibility::AccessibleEventObject const &
        rEvent,
        ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference<
        ::com::sun::star::uno::XInterface > > const & rListeners);

    /**
       Sends all the events accumulated in the buffer.

       If sending any of the events to a specific XAccessibleListener fails with
       a RuntimeException, that exception is ignored and the method carries on
       with the rest of the notifications.

       After sending all events, the buffer is cleared.
     */
    void sendEvents() const;

private:
    struct Entry;

    typedef ::std::vector< Entry > Entries;

    Entries m_aEntries;
};

}

#endif // INCLUDED_COMPHELPER_ACCESSIBLEEVENTBUFFER_HXX
