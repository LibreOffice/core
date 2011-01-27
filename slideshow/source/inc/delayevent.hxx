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
#ifndef INCLUDED_SLIDESHOW_DELAYEVENT_HXX
#define INCLUDED_SLIDESHOW_DELAYEVENT_HXX

#include <boost/function.hpp>

#include "event.hxx"
#include "debug.hxx"
#include <boost/noncopyable.hpp>

namespace slideshow {
namespace internal {

/** Event, which delays the functor call the given amount of time
 */
class Delay : public Event, private ::boost::noncopyable
{
public:
    typedef ::boost::function0<void> FunctorT;

    template <typename FuncT>
        Delay( FuncT const& func,
               double nTimeout
#if OSL_DEBUG_LEVEL > 1
            ,  const ::rtl::OUString& rsDescription
            ) : Event(rsDescription),
#else
            ) :
#endif
            mnTimeout(nTimeout), maFunc(func), mbWasFired(false) {}

    Delay( const boost::function0<void>& func,
           double nTimeout
#if OSL_DEBUG_LEVEL > 1
        , const ::rtl::OUString& rsDescription
        ) : Event(rsDescription),
#else
        ) :
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

#if OSL_DEBUG_LEVEL <= 1

/** Generate delay event

    @param func
    Functor to call when the event fires.

    @param nTimeout
    Timeout in seconds, to wait until functor is called.

    @return generated delay event
*/
template <typename FuncT>
inline EventSharedPtr makeDelay_( FuncT const& func, double nTimeout )
{
    return EventSharedPtr( new Delay( func, nTimeout ) );
}

/** Generate immediate event

    @param func
    Functor to call when the event fires.

    @return generated immediate event.
*/
template <typename FuncT>
inline EventSharedPtr makeEvent_( FuncT const& func )
{
    return EventSharedPtr( new Delay( func, 0.0 ) );
}


// Strip away description.
#define makeDelay(f, t, d) makeDelay_(f, t)
#define makeEvent(f, d) makeEvent_(f)

#else // OSL_DEBUG_LEVEL > 1

class Delay_ : public Delay {
public:
    template <typename FuncT>
    Delay_( FuncT const& func, double nTimeout,
        char const* from_function, char const* from_file, int from_line,
        const ::rtl::OUString& rsDescription)
        : Delay(func, nTimeout, rsDescription),
          FROM_FUNCTION(from_function),
          FROM_FILE(from_file), FROM_LINE(from_line) {}

    char const* const FROM_FUNCTION;
    char const* const FROM_FILE;
    int const FROM_LINE;
};

template <typename FuncT>
inline EventSharedPtr makeDelay_(
    FuncT const& func, double nTimeout,
    char const* from_function, char const* from_file, int from_line,
    const ::rtl::OUString& rsDescription)
{
    return EventSharedPtr( new Delay_( func, nTimeout,
            from_function, from_file, from_line, rsDescription) );
}

#define makeDelay(f, t, d) makeDelay_(f, t,                   \
        BOOST_CURRENT_FUNCTION, __FILE__, __LINE__,           \
        ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(d)))
#define makeEvent(f, d) makeDelay_(f, 0.0,                  \
        BOOST_CURRENT_FUNCTION, __FILE__, __LINE__,         \
        ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(d)))

#endif // OSL_DEBUG_LEVEL <= 1

} // namespace internal
} // namespace presentation

#endif /* INCLUDED_SLIDESHOW_DELAYEVENT_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
