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
#ifndef INCLUDED_SLIDESHOW_SOURCE_INC_DELAYEVENT_HXX
#define INCLUDED_SLIDESHOW_SOURCE_INC_DELAYEVENT_HXX

#include "event.hxx"

#include <functional>

namespace slideshow::internal {

/** Event, which delays the functor call the given amount of time
 */
class Delay : public Event
{
public:
    typedef ::std::function<void ()> FunctorT;

    template <typename FuncT>
        Delay( FuncT const& func,
               double nTimeout
            ,  const OUString& rsDescription
            ) : Event(rsDescription),
            mnTimeout(nTimeout), maFunc(func), mbWasFired(false) {}

    Delay( const std::function<void ()>& func,
           double nTimeout
        , const OUString& rsDescription
        ) : Event(rsDescription),
        mnTimeout(nTimeout),
        maFunc(func),
        mbWasFired(false) {}
    Delay(const Delay&) = delete;
    Delay& operator=(const Delay&) = delete;

    // Event:
    virtual bool fire() override;
    virtual bool isCharged() const override;
    virtual double getActivationTime( double nCurrentTime ) const override;
    // Disposable:
    virtual void dispose() override;

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
inline EventSharedPtr makeDelay_( FuncT const& func, double nTimeout, OUString const& rsDescription )
{
    return std::make_shared<Delay>( func, nTimeout, rsDescription );
}

/** Generate immediate event

    @param func
    Functor to call when the event fires.

    @return generated immediate event.
*/
template <typename FuncT>
inline EventSharedPtr makeEvent_( FuncT const& func, OUString const& rsDescription)
{
    return std::make_shared<Delay>( func, 0.0, rsDescription );
}


#define makeDelay(f, t, d) makeDelay_(f, t, d)
#define makeEvent(f, d) makeEvent_(f, d)

#else // OSL_DEBUG_LEVEL > 0

class Delay_ : public Delay {
public:
    template <typename FuncT>
    Delay_( FuncT const& func, double nTimeout,
        char const* from_function, char const* from_file, int from_line,
        const OUString& rsDescription)
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
    const OUString& rsDescription)
{
    return EventSharedPtr( new Delay_( func, nTimeout,
            from_function, from_file, from_line, rsDescription) );
}

#define makeDelay(f, t, d) makeDelay_(f, t,                   \
        __func__, __FILE__, __LINE__,           \
        d)
#define makeEvent(f, d) makeDelay_(f, 0.0,                  \
        __func__, __FILE__, __LINE__,         \
        d)

#endif // OSL_DEBUG_LEVEL <= 1

} // namespace presentation::internal

#endif // INCLUDED_SLIDESHOW_SOURCE_INC_DELAYEVENT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
