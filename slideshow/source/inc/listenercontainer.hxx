/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
#ifndef INCLUDED_SLIDESHOW_SOURCE_INC_LISTENERCONTAINER_HXX
#define INCLUDED_SLIDESHOW_SOURCE_INC_LISTENERCONTAINER_HXX

#include <osl/mutex.hxx>
#include <boost/next_prior.hpp>
#include <algorithm>
#include <vector>

namespace slideshow {
namespace internal {

struct EmptyBase
{
    struct EmptyGuard{ explicit EmptyGuard(EmptyBase) {} };
    struct EmptyClearableGuard
    {
        explicit EmptyClearableGuard(EmptyBase) {}
        static void clear() {}
    };

    typedef EmptyGuard           Guard;
    typedef EmptyClearableGuard ClearableGuard;
};

class MutexBase
{
public:
    struct Guard : public osl::MutexGuard
    {
        explicit Guard(MutexBase const& rBase) :
            osl::MutexGuard(rBase.maMutex)
        {}
    };
    struct ClearableGuard : public osl::ClearableMutexGuard
    {
        explicit ClearableGuard(MutexBase const& rBase) :
            osl::ClearableMutexGuard(rBase.maMutex)
        {}
    };

    mutable osl::Mutex maMutex;
};

template< typename result_type, typename ListenerTargetT > struct FunctionApply
{
    template<typename FuncT> static bool apply(
        FuncT           func,
        ListenerTargetT const& rArg )
    {
        return func(rArg);
    }
};

template<typename ListenerTargetT> struct FunctionApply<void,ListenerTargetT>
{
    template<typename FuncT> static bool apply(
        FuncT                  func,
        ListenerTargetT const& rArg )
    {
        func(rArg);
        return true;
    }
};

template< typename ListenerT > struct ListenerOperations
{
    /// Notify a single one of the listeners
    template< typename ContainerT,
              typename FuncT >
    static bool notifySingleListener( ContainerT& rContainer,
                                      FuncT       func )
    {
        // true: a handler in this queue processed the event
        // false: no handler in this queue finally processed the event
        return std::any_of( rContainer.begin(),
                            rContainer.end(),
                            func );
    }

    /// Notify all listeners
    template< typename ContainerT,
              typename FuncT >
    static bool notifyAllListeners( ContainerT& rContainer,
                                    FuncT       func )
    {
        bool bRet(false);
        for( const auto& rCurr : rContainer )
        {
            if( FunctionApply< typename ::std::result_of< FuncT( const typename ContainerT::value_type& ) >::type,
                               typename ContainerT::value_type >::apply(
                                   func,
                                   rCurr) )
            {
                bRet = true;
            }
        }

        // true: at least one handler returned true
        // false: not a single handler returned true
        return bRet;
    }

    /// Prune container from deceased listeners
    template< typename ContainerT >
    static void pruneListeners( ContainerT&, size_t )
    {
    }
};

template< typename ListenerTargetT >
struct ListenerOperations< boost::weak_ptr<ListenerTargetT> >
{
    template< typename ContainerT,
              typename FuncT >
    static bool notifySingleListener( ContainerT& rContainer,
                                      FuncT       func )
    {
        for( const auto& rCurr : rContainer )
        {
            boost::shared_ptr<ListenerTargetT> pListener( rCurr.lock() );

            if( pListener && func(pListener) )
                return true;
        }

        return false;
    }

    template< typename ContainerT,
              typename FuncT >
    static bool notifyAllListeners( ContainerT& rContainer,
                                    FuncT       func )
    {
        bool bRet(false);
        for( const auto& rCurr : rContainer )
        {
            boost::shared_ptr<ListenerTargetT> pListener( rCurr.lock() );

            if( pListener.get() &&
                FunctionApply< typename ::std::result_of< FuncT( const typename ContainerT::value_type& ) >::type,
                               boost::shared_ptr<ListenerTargetT> >::apply(func,pListener) )
            {
                bRet = true;
            }
        }

        return bRet;
    }
    template< typename ContainerT >
    static void pruneListeners( ContainerT& rContainer,
                                size_t      nSizeThreshold )
    {
        if( rContainer.size() <= nSizeThreshold )
            return;

        ContainerT aAliveListeners;
        aAliveListeners.reserve(rContainer.size());

        for( const auto& rCurr : rContainer )
        {
            if( !rCurr.expired() )
                aAliveListeners.push_back( rCurr );
        }

        std::swap( rContainer, aAliveListeners );
    }
};

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

#endif // INCLUDED_SLIDESHOW_SOURCE_INC_LISTENERCONTAINER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
