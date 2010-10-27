/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
#ifndef INCLUDED_SLIDESHOW_LISTENERCONTAINER_HXX
#define INCLUDED_SLIDESHOW_LISTENERCONTAINER_HXX

#include <osl/mutex.hxx>
#include <boost/utility.hpp>
#include <algorithm>
#include <vector>

#include "listenercontainerimpl.hxx"

namespace slideshow {
namespace internal {

/** Container for objects that can be notified.

    This templatized container holds listener objects, than can get
    notified (by calling certain methods on them).

    @tpl Listener
    Type for the listener objects to be held

    @tpl ContainerT
    Full type of the container to store the listener objects. Defaults
    to std::vector<ListenerT>

    @tpl MaxDeceasedListenerUllage
    Threshold, from which upwards the listener container gets
    pruned. Avoids frequent copying of nearly empty containers.

    @attention internal class, not to be used. functionality is
    incomplete, please use the Thread(Un)safeListenerContainer types
    from below
*/
template< typename ListenerT,
          typename MutexHolderBaseT,
          typename ContainerT=std::vector<ListenerT>,
          size_t MaxDeceasedListenerUllage=16 > class ListenerContainerBase : public MutexHolderBaseT
{
    typedef typename MutexHolderBaseT::Guard           Guard;
    typedef typename MutexHolderBaseT::ClearableGuard  ClearableGuard;

public:
    typedef ListenerT        listener_type;
    typedef ContainerT       container_type;
    typedef MutexHolderBaseT mutex_type;

    /** Check whether listener container is empty

        @return true, if currently no listeners registered. Note that
        in a multi-threaded scenario, without external synchronisation
        to this object, the return value might become wrong at any time.
     */
    bool isEmpty() const
    {
        Guard aGuard(*this);
        return maListeners.empty();
    }

    /** Check whether given listener is already added

        @return true, if given listener is already added.
     */
    bool isAdded( listener_type const& rListener ) const
    {
        Guard aGuard(*this);

        const typename container_type::const_iterator aEnd( maListeners.end() );
        if( std::find( maListeners.begin(),
                       aEnd,
                       rListener ) != aEnd )
        {
            return true; // already added
        }

        return false;
    }

    /** Add new listener

        @param rListener
        Listener to add

        @return false, if the listener is already added, true
        otherwise
     */
    bool add( listener_type const& rListener )
    {
        Guard aGuard(*this);

        // ensure uniqueness
        if( isAdded(rListener) )
            return false; // already added

        maListeners.push_back( rListener );

        ListenerOperations<ListenerT>::pruneListeners(
            maListeners,
            MaxDeceasedListenerUllage);

        return true;
    }

    /** Add new listener into sorted container

        The stored listeners are kept sorted (using this method
        requires listener_type to have operator< defined on it). Make
        sure to call addSorted() for <em>each</em> listener to add to
        this container - sorting is performed under the assumption
        that existing entries are already sorted.

        @param rListener
        Listener to add

        @return false, if the listener is already added, true
        otherwise
     */
    bool addSorted( listener_type const& rListener )
    {
        Guard aGuard(*this);

        // ensure uniqueness
        if( isAdded(rListener) )
            return false; // already added

        maListeners.push_back( rListener );

        // a single entry does not need to be sorted
        if( maListeners.size() > 1 )
        {
            std::inplace_merge(
                maListeners.begin(),
                boost::prior(maListeners.end()),
                maListeners.end() );
        }

        ListenerOperations<ListenerT>::pruneListeners(
            maListeners,
            MaxDeceasedListenerUllage);

        return true;
    }

    /** Remove listener from container

        @param rListener
        The listener to remove

        @return false, if listener not found in container, true
        otherwise
     */
    bool remove( listener_type const& rListener )
    {
        Guard aGuard(*this);

        const typename container_type::iterator aEnd( maListeners.end() );
        typename container_type::iterator       aIter;
        if( (aIter=std::remove(maListeners.begin(),
                               aEnd,
                               rListener)) == aEnd )
        {
            return false; // listener not found
        }

        maListeners.erase( aIter, aEnd );

        return true;
    }

    /// Removes all listeners in one go
    void clear()
    {
        Guard aGuard(*this);

        maListeners.clear();
    }

    /** Apply functor to one listener

        This method applies functor to one of the listeners. Starting
        with the first entry of the container, the functor is called
        with the listener entries, until it returns true.

        @param func
        Given functor is called with listeners, until it returns true

        @return true, if functor was successfully applied to a
        listener
     */
    template< typename FuncT > bool apply( FuncT func ) const
    {
        ClearableGuard aGuard(*this);

        // generate a local copy of all handlers, to make method
        // reentrant and thread-safe.
        container_type const local( maListeners );
        aGuard.clear();

        const bool bRet(
            ListenerOperations<ListenerT>::notifySingleListener(
                local,
                func ));

        {
            Guard aGuard2(*this);
            ListenerOperations<ListenerT>::pruneListeners(
                const_cast<container_type&>(maListeners),
                MaxDeceasedListenerUllage);
        }

        return bRet;
    }

    /** Apply functor to all listeners

        This method applies functor to all of the listeners. Starting
        with the first entry of the container, the functor is called
        with the listener entries.

        @param func
        Given functor is called with listeners.

        @return true, if functor was successfully applied to at least
        one listener
     */
    template< typename FuncT > bool applyAll( FuncT func ) const
    {
        ClearableGuard aGuard(*this);

        // generate a local copy of all handlers, to make method
        // reentrant and thread-safe.
        container_type const local( maListeners );
        aGuard.clear();

        const bool bRet(
            ListenerOperations<ListenerT>::notifyAllListeners(
                local,
                func ));

        {
            Guard aGuard2(*this);
            ListenerOperations<ListenerT>::pruneListeners(
                const_cast<container_type&>(maListeners),
                MaxDeceasedListenerUllage);
        }

        return bRet;
    }

private:
    ContainerT  maListeners;
};

////////////////////////////////////////////////////////////////////////////

/** ListenerContainer variant that serialized access

    This ListenerContainer is safe to use in a multi-threaded
    context. It serializes access to the object, and avoids
    dead-locking by releasing the object mutex before calling
    listeners.
 */
template< typename ListenerT,
          typename ContainerT=std::vector<ListenerT> >
class ThreadSafeListenerContainer : public ListenerContainerBase<ListenerT,
                                                                 MutexBase,
                                                                 ContainerT>
{
};

////////////////////////////////////////////////////////////////////////////

/** ListenerContainer variant that does not serialize access

    This ListenerContainer version is not safe to use in a
    multi-threaded scenario, but has less overhead.
 */
template< typename ListenerT,
          typename ContainerT=std::vector<ListenerT> >
class ThreadUnsafeListenerContainer : public ListenerContainerBase<ListenerT,
                                                                   EmptyBase,
                                                                   ContainerT>
{
};

} // namespace internal
} // namespace slideshow

#endif /* INCLUDED_SLIDESHOW_LISTENERCONTAINER_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
