/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: listenercontainerimpl.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2007-07-17 15:10:47 $
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

