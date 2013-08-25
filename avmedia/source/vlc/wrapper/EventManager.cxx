/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <vlc/libvlc.h>
#include <vlc/libvlc_media.h>
#include <vlc/libvlc_events.h>

#include "EventManager.hxx"
#include "SymbolLoader.hxx"
#include "EventHandler.hxx"

typedef void ( *libvlc_callback_t ) ( const struct libvlc_event_t *, void * );

namespace VLC
{
    namespace
    {
        libvlc_event_manager_t* ( *libvlc_media_player_event_manager ) ( libvlc_media_player_t *p_mi );
        int ( *libvlc_event_attach ) ( libvlc_event_manager_t *p_event_manager,
                                       libvlc_event_type_t i_event_type,
                                       libvlc_callback_t f_callback,
                                       void *user_data );
        void ( *libvlc_event_detach ) ( libvlc_event_manager_t *p_event_manager,
                                        libvlc_event_type_t i_event_type,
                                        libvlc_callback_t f_callback,
                                        void *p_user_data );
    }

void EventManager::Handler( const libvlc_event_t *event, void *pData )
{
    EventManager *instance = static_cast<EventManager*>( pData );
    switch ( event->type )
    {
    case libvlc_MediaPlayerPaused:
        instance->mEventHandler->mCallbackQueue.push( instance->mOnPaused );
        break;
    case libvlc_MediaPlayerEndReached:
        instance->mEventHandler->mCallbackQueue.push( instance->mOnEndReached );
        break;
    }
}

bool EventManager::LoadSymbols()
{
    ApiMap VLC_EVENT_MANAGER_API[] =
    {
        SYM_MAP( libvlc_media_player_event_manager ),
        SYM_MAP( libvlc_event_attach ),
        SYM_MAP( libvlc_event_detach )
    };

    return InitApiMap( VLC_EVENT_MANAGER_API );
}

EventManager::EventManager( VLC::Player& player, boost::shared_ptr<VLC::EventHandler> eh )
    : mEventHandler( eh )
    , mManager( libvlc_media_player_event_manager( player ) )
{

}

EventManager::~EventManager()
{
}

void EventManager::registerSignal( int signal, const Callback& callback )
{
    if ( callback.empty() )
        libvlc_event_detach( mManager, signal, Handler, this );
    else
        libvlc_event_attach( mManager, signal, Handler, this );
}

void EventManager::onPaused( const EventManager::Callback& callback )
{
    mOnPaused = callback;
    registerSignal( libvlc_MediaPlayerPaused, callback );
}

void EventManager::onEndReached( const Callback& callback )
{
    mOnEndReached = callback;
    registerSignal( libvlc_MediaPlayerEndReached, callback );
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
