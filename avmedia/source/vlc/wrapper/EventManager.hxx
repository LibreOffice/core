/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef _WRAPPER_EVENT_MANAGER_HXX
#define _WRAPPER_EVENT_MANAGER_HXX

#include <boost/function.hpp>
#include <boost/noncopyable.hpp>
#include "Player.hxx"

struct libvlc_event_manager_t;
struct libvlc_event_t;

namespace avmedia
{
namespace vlc
{
namespace wrapper
{
    class EventHandler;
    class EventManager : boost::noncopyable
    {

    public:
        static bool LoadSymbols();
        typedef boost::function<void()> Callback;

        EventManager( Player& player, EventHandler& eh );

        void onPaused( const Callback& callback = Callback() );
        void onEndReached( const Callback& callback = Callback() );

    private:
        EventHandler& mEventHandler;
        typedef boost::function< void() > TCallback;
        libvlc_event_manager_t *mManager;
        TCallback mOnPaused;
        TCallback mOnEndReached;

        void registerSignal( int signal, const Callback& callback );

        static void Handler( const libvlc_event_t *event, void *pData );
    };
}
}
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
