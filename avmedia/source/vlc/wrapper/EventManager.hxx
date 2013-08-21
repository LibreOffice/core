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
#include <boost/shared_ptr.hpp>

#include "Player.hxx"

struct libvlc_event_manager_t;
struct libvlc_event_t;

namespace VLC
{
    class EventHandler;
    class EventManager
    {
        friend void EventManagerEventHandler( const libvlc_event_t *event, void *pData );
    public:
        typedef boost::function<void()> Callback;

        EventManager( VLC::Player& player, boost::shared_ptr<VLC::EventHandler> eh );
        virtual ~EventManager();

        void onPaused( const Callback& callback = Callback() );
        void onEndReached( const Callback& callback = Callback() );

    private:
        boost::shared_ptr<VLC::EventHandler> mEventHandler;
        typedef boost::function< void() > TCallback;
        libvlc_event_manager_t *mManager;
        TCallback mOnPaused;
        TCallback mOnEndReached;

        void registerSignal( int signal, const Callback& callback );
    };
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
