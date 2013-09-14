/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <rtl/ustring.h>
#include "Types.hxx"
#include "Player.hxx"
#include "Media.hxx"
#include "SymbolLoader.hxx"

struct libvlc_media_t;
namespace VLC
{
    namespace
    {
        void ( *libvlc_media_player_retain ) ( libvlc_media_player_t *p_mi );
        libvlc_media_player_t * ( *libvlc_media_player_new_from_media ) ( libvlc_media_t *p_md );
        void ( *libvlc_media_player_release ) ( libvlc_media_player_t *p_mi );
        int ( *libvlc_media_player_play ) ( libvlc_media_player_t *p_mi );
        void ( *libvlc_media_player_pause ) ( libvlc_media_player_t *p_mi );
        int ( *libvlc_media_player_is_playing ) ( libvlc_media_player_t *p_mi );
        void ( *libvlc_media_player_stop ) ( libvlc_media_player_t *p_mi );
        void ( *libvlc_media_player_set_time ) ( libvlc_media_player_t *p_mi, libvlc_time_t i_time );
        libvlc_time_t ( *libvlc_media_player_get_time ) ( libvlc_media_player_t *p_mi );
        float ( *libvlc_media_player_get_rate )( libvlc_media_player_t *p_mi );
        int ( *libvlc_audio_set_volume ) ( libvlc_media_player_t *p_mi, int i_volume );
        int ( *libvlc_audio_get_volume ) ( libvlc_media_player_t *p_mi );
        int ( *libvlc_audio_get_mute ) ( libvlc_media_player_t *p_mi );
        void ( *libvlc_audio_set_mute ) ( libvlc_media_player_t *p_mi, int status );
        int ( *libvlc_video_take_snapshot ) ( libvlc_media_player_t *p_mi, unsigned num,
                                const char *psz_filepath, unsigned int i_width,
                                unsigned int i_height );
#if defined UNX
        void ( *libvlc_media_player_set_xwindow ) ( libvlc_media_player_t *p_mi, uint32_t drawable );
#elif defined MACOSX
        void ( *libvlc_media_player_set_nsobject ) ( libvlc_media_player_t *p_mi, void *drawable );
#elif defined WNT
        void ( *libvlc_media_player_set_hwnd ) ( libvlc_media_player_t *p_mi, void *drawable );
#else
#error unknown OS
#endif
        unsigned ( *libvlc_media_player_has_vout ) ( libvlc_media_player_t *p_mi );
        void ( *libvlc_video_set_mouse_input ) ( libvlc_media_player_t *p_mi, unsigned on);
    }

    bool Player::LoadSymbols()
    {
        ApiMap VLC_PLAYER_API[] =
        {
            SYM_MAP( libvlc_media_player_new_from_media ),
            SYM_MAP( libvlc_media_player_release ),
            SYM_MAP( libvlc_media_player_play ),
            SYM_MAP( libvlc_media_player_pause ),
            SYM_MAP( libvlc_media_player_is_playing ),
            SYM_MAP( libvlc_media_player_stop ),
            SYM_MAP( libvlc_media_player_set_time ),
            SYM_MAP( libvlc_media_player_get_time ),
            SYM_MAP( libvlc_media_player_get_rate ),
            SYM_MAP( libvlc_audio_set_volume ),
            SYM_MAP( libvlc_audio_get_volume ),
            SYM_MAP( libvlc_audio_set_mute ),
            SYM_MAP( libvlc_audio_get_mute ),
            SYM_MAP( libvlc_video_take_snapshot ),
#if defined UNX
            SYM_MAP( libvlc_media_player_set_xwindow ),
#elif defined MACOSX
            SYM_MAP( libvlc_media_player_set_nsobject ),
#elif defined WNT
            SYM_MAP( libvlc_media_player_set_hwnd ),
#endif
            SYM_MAP( libvlc_media_player_has_vout ),
            SYM_MAP( libvlc_video_set_mouse_input ),
            SYM_MAP( libvlc_media_player_retain )
        };

        return InitApiMap( VLC_PLAYER_API );
    }

    Player::Player(Media& media)
        : mPlayer( libvlc_media_player_new_from_media( media ) )
    {
    }

    Player::Player( const Player& other )
    {
        operator=( other );
    }

    const Player& Player::operator=( const Player& other )
    {
        libvlc_media_player_release( mPlayer );
        mPlayer = other.mPlayer;
        libvlc_media_player_retain( mPlayer );
        return *this;
    }

    Player::~Player()
    {
        libvlc_media_player_release( mPlayer );
    }

    bool Player::play()
    {
        return libvlc_media_player_play( mPlayer ) == 0;
    }

    void Player::pause()
    {
        libvlc_media_player_pause( mPlayer );
    }

    void Player::stop()
    {
        libvlc_media_player_stop( mPlayer );
    }

    void Player::setTime( int time )
    {
        libvlc_media_player_set_time( mPlayer, time );
    }

    int Player::getTime() const
    {
        const int time = libvlc_media_player_get_time( mPlayer );

        return ( time == -1 ? 0 : time );
    }

    void Player::setMouseHandling(bool flag)
    {
        libvlc_video_set_mouse_input( mPlayer, flag );
    }

    bool Player::isPlaying() const
    {
        return libvlc_media_player_is_playing( mPlayer ) == 1;
    }

    float Player::getRate() const
    {
        return libvlc_media_player_get_rate( mPlayer );
    }

    void Player::setVolume( int volume )
    {
        libvlc_audio_set_volume( mPlayer, volume );
    }

    int Player::getVolume() const
    {
        return libvlc_audio_get_volume( mPlayer );
    }

    void Player::setMute( bool mute)
    {
        libvlc_audio_set_mute( mPlayer, mute );
    }

    bool Player::getMute() const
    {
        return libvlc_audio_get_mute( mPlayer );
    }


    void Player::setWindow( intptr_t id )
    {
#if defined UNX
        libvlc_media_player_set_xwindow( mPlayer, (uint32_t) id );
#elif defined MACOSX
        libvlc_media_player_set_nsobject( mPlayer, reinterpret_cast<void*>( id ) );
#elif defined WNT
        libvlc_media_player_set_hwnd( mPlayer, reinterpret_cast<void*>( id ) );
#endif
    }

    bool Player::takeSnapshot(const rtl::OUString& file)
    {
        rtl::OString dest;
        file.convertToString( &dest, RTL_TEXTENCODING_UTF8, 0 );
        return libvlc_video_take_snapshot( mPlayer, 0, dest.getStr(), 480, 360 ) == 0;
    }

    bool Player::hasVout() const
    {
        return libvlc_media_player_has_vout( mPlayer );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
