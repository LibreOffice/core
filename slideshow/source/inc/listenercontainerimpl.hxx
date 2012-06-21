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
#ifndef INCLUDED_SLIDESHOW_LISTENERCONTAINERIMPL_HXX
#define INCLUDED_SLIDESHOW_LISTENERCONTAINERIMPL_HXX

#include <sal/config.h>
#include <boost/weak_ptr.hpp>

namespace slideshow {
namespace internal {

////////////////////////////////////////////////////////////////////////////

struct EmptyBase
{
    struct EmptyGuard{ explicit EmptyGuard(EmptyBase) {} };
    struct EmptyClearableGuard
    {
        explicit EmptyClearableGuard(EmptyBase) {}
        void clear() {}
        void reset() {}
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

////////////////////////////////////////////////////////////////////////////

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

////////////////////////////////////////////////////////////////////////////

template< typename ListenerT > struct ListenerOperations
{
    /// Notify a single one of the listeners
    template< typename ContainerT,
              typename FuncT >
    static bool notifySingleListener( ContainerT& rContainer,
                                      FuncT       func )
    {
        const typename ContainerT::const_iterator aEnd( rContainer.end() );

        // true: a handler in this queue processed the event
        // false: no handler in this queue finally processed the event
        return (std::find_if( rContainer.begin(),
                              aEnd,
                              func ) != aEnd);
    }

    /// Notify all listeners
    template< typename ContainerT,
              typename FuncT >
    static bool notifyAllListeners( ContainerT& rContainer,
                                    FuncT       func )
    {
        bool bRet(false);
        typename ContainerT::const_iterator       aCurr( rContainer.begin() );
        typename ContainerT::const_iterator const aEnd ( rContainer.end() );
        while( aCurr != aEnd )
        {
            if( FunctionApply< typename FuncT::result_type,
                               typename ContainerT::value_type >::apply(
                                   func,
                                   *aCurr) )
            {
                bRet = true;
            }

            ++aCurr;
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

// specializations for weak_ptr
// ----------------------------
template< typename ListenerTargetT >
struct ListenerOperations< boost::weak_ptr<ListenerTargetT> >
{
    template< typename ContainerT,
              typename FuncT >
    static bool notifySingleListener( ContainerT& rContainer,
                                      FuncT       func )
    {
        typename ContainerT::const_iterator       aCurr( rContainer.begin() );
        typename ContainerT::const_iterator const aEnd ( rContainer.end() );
        while( aCurr != aEnd )
        {
            boost::shared_ptr<ListenerTargetT> pListener( aCurr->lock() );

            if( pListener && func(pListener) )
                return true;

            ++aCurr;
        }

        return false;
    }

    template< typename ContainerT,
              typename FuncT >
    static bool notifyAllListeners( ContainerT& rContainer,
                                    FuncT       func )
    {
        bool bRet(false);
        typename ContainerT::const_iterator       aCurr( rContainer.begin() );
        typename ContainerT::const_iterator const aEnd ( rContainer.end() );
        while( aCurr != aEnd )
        {
            boost::shared_ptr<ListenerTargetT> pListener( aCurr->lock() );

            if( pListener.get() &&
                FunctionApply< typename FuncT::result_type,
                               boost::shared_ptr<ListenerTargetT> >::apply(func,pListener) )
            {
                bRet = true;
            }

            ++aCurr;
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

        typename ContainerT::const_iterator       aCurr( rContainer.begin() );
        typename ContainerT::const_iterator const aEnd ( rContainer.end() );
        while( aCurr != aEnd )
        {
            if( !aCurr->expired() )
                aAliveListeners.push_back( *aCurr );

            ++aCurr;
        }

        std::swap( rContainer, aAliveListeners );
    }
};

} // namespace internal
} // namespace Presentation

#endif /* INCLUDED_SLIDESHOW_LISTENERCONTAINERIMPL_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
