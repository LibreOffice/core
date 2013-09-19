/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef _WRAPPER_PLAYER_HXX
#define _WRAPPER_PLAYER_HXX
#if defined UNX
# include <stdint.h>
#endif

struct libvlc_media_player_t;

namespace rtl
{
    class OUString;
}

namespace avmedia
{
namespace vlc
{
namespace wrapper
{
    class Media;
    class Player
    {
    public:
        static bool LoadSymbols();
        Player( Media& media );
        Player( const Player& other );
        const Player& operator=( const Player& other );
        virtual ~Player();

        bool play();
        void pause();
        void stop();
        void setTime( int time );
        int getTime() const;
        bool isPlaying() const;

        float getRate() const;

        void setVolume( int volume );
        int getVolume() const;

        void setMute( bool mute);
        bool getMute() const;

        void setWindow( intptr_t id );

        bool takeSnapshot(const rtl::OUString& file);

        bool hasVout() const;

        void setScale( float factor );
        void setVideoSize( unsigned width, unsigned height );

        unsigned getWidth() const;
        unsigned getHeight() const;

        inline operator libvlc_media_player_t*()
        {
            return mPlayer;
        }

        void setMouseHandling(bool flag);
    private:
        libvlc_media_player_t *mPlayer;
    };
}
}
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
