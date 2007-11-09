/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: delayevent.hxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: rt $ $Date: 2007-11-09 10:19:03 $
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
#ifndef INCLUDED_SLIDESHOW_DELAYEVENT_HXX
#define INCLUDED_SLIDESHOW_DELAYEVENT_HXX

#include "event.hxx"
#include <boost/noncopyable.hpp>
#include <boost/function.hpp>
#if defined(VERBOSE) && defined(DBG_UTIL)
#include "boost/current_function.hpp"
#endif

namespace slideshow {
namespace internal {

/** Event, which delays the functor call the given amount of time
 */
class Delay : public Event, private ::boost::noncopyable
{
public:
    typedef ::boost::function0<void> FunctorT;

    template <typename FuncT>
    Delay( FuncT const& func, double nTimeout )
        : mnTimeout(nTimeout), maFunc(func), mbWasFired(false) {}

#if defined(VERBOSE) && defined(DBG_UTIL)
    Delay( const boost::function0<void>& func,
           double nTimeout,
           char const* const  ) :
#else
    Delay( const boost::function0<void>& func,
           double nTimeout ) :
#endif
        mnTimeout(nTimeout),
        maFunc(func),
        mbWasFired(false) {}

    // Event:
    virtual bool fire();
    virtual bool isCharged() const;
    virtual double getActivationTime( double nCurrentTime ) const;
    // Disposable:
    virtual void dispose();

private:
    double const mnTimeout;
    FunctorT maFunc;
    bool mbWasFired;
};

#if OSL_DEBUG_LEVEL < 1

/** Generate delay event

    @param func
    Functor to call when the event fires.

    @param nTimeout
    Timeout in seconds, to wait until functor is called.

    @return generated delay event
*/
template <typename FuncT>
inline EventSharedPtr makeDelay( FuncT const& func, double nTimeout )
{
    return EventSharedPtr( new Delay( func, nTimeout ) );
}

/** Generate immediate event

    @param func
    Functor to call when the event fires.

    @return generated immediate event.
*/
template <typename FuncT>
inline EventSharedPtr makeEvent( FuncT const& func )
{
    return EventSharedPtr( new Delay( func, 0.0 ) );
}

#else // OSL_DEBUG_LEVEL > 1

class Delay_ : public Delay {
public:
    template <typename FuncT>
    Delay_( FuncT const& func, double nTimeout,
            char const* from_function, char const* from_file, int from_line )
        : Delay(func, nTimeout),
          FROM_FUNCTION(from_function),
          FROM_FILE(from_file), FROM_LINE(from_line) {}

    char const* const FROM_FUNCTION;
    char const* const FROM_FILE;
    int const FROM_LINE;
};

template <typename FuncT>
inline EventSharedPtr makeDelay_(
    FuncT const& func, double nTimeout,
    char const* from_function, char const* from_file, int from_line )
{
    return EventSharedPtr( new Delay_( func, nTimeout,
                                       from_function, from_file, from_line ) );
}

#define makeDelay(f, t) makeDelay_(f, t, \
BOOST_CURRENT_FUNCTION, __FILE__, __LINE__)
#define makeEvent(f) makeDelay_(f, 0.0, \
BOOST_CURRENT_FUNCTION, __FILE__, __LINE__)

#endif // OSL_DEBUG_LEVEL < 1

} // namespace internal
} // namespace presentation

#endif /* INCLUDED_SLIDESHOW_DELAYEVENT_HXX */
