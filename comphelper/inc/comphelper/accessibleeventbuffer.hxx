/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: accessibleeventbuffer.hxx,v $
 * $Revision: 1.4 $
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
