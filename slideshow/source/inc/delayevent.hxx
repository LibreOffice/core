/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: delayevent.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: obo $ $Date: 2005-10-11 08:48:53 $
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
#if ! defined(INCLUDED_SLIDESHOW_DELAYEVENT_HXX)
#define INCLUDED_SLIDESHOW_DELAYEVENT_HXX

#include "event.hxx"
#include "boost/noncopyable.hpp"
#include "boost/function.hpp"
#if defined(VERBOSE) && defined(DBG_UTIL)
#include "boost/current_function.hpp"
#endif

namespace presentation {
namespace internal {

/** Event, which delays the functor call the given amount of time
 */
class Delay : public Event,
              private ::boost::noncopyable
{
public:
#if defined(VERBOSE) && defined(DBG_UTIL)
    template <typename FunctorT>
    Delay( FunctorT const& func,
           double nTimeout,
           char const* const origin ) : Event(origin),
#else
    template <typename FunctorT>
    Delay( FunctorT const& func,
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
    double  mnTimeout;
    ::boost::function0<void> maFunc;
    bool    mbWasFired;
};

#if defined(VERBOSE) && defined(DBG_UTIL)

template <typename FunctorT>
EventSharedPtr makeDelay_( FunctorT const& func, double nTimeout,
                           char const* const origin )
{
    return EventSharedPtr( new Delay( func, nTimeout, origin ) );
}
#define makeDelay(f, t) makeDelay_(f, t, BOOST_CURRENT_FUNCTION)
#define makeEvent(f) makeDelay_(f, 0.0, BOOST_CURRENT_FUNCTION)

#else

/** Generate delay event

    @param func
    Functor to call when the event fires.

    @param nTimeout
    Timeout in seconds, to wait until functor is called.

    @return generated delay event
*/
template <typename FunctorT>
EventSharedPtr makeDelay( FunctorT const& func, double nTimeout )
{
    return EventSharedPtr( new Delay( func, nTimeout ) );
}

/** Generate immediate event

    @param func
    Functor to call when the event fires.

    @return generated immediate event.
*/
template <typename FunctorT>
EventSharedPtr makeEvent( FunctorT const& func )
{
    return EventSharedPtr( new Delay( func, 0.0 ) );
}

#endif

} // namespace internal
} // namespace presentation

#endif /* INCLUDED_SLIDESHOW_DELAYEVENT_HXX */
