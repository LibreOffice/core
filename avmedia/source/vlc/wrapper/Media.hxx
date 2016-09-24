/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_AVMEDIA_SOURCE_VLC_WRAPPER_MEDIA_HXX
#define INCLUDED_AVMEDIA_SOURCE_VLC_WRAPPER_MEDIA_HXX

struct libvlc_media_t;

namespace rtl { class OUString; }

namespace avmedia
{
namespace vlc
{
namespace wrapper
{
    class Instance;
    class Media
    {
    public:
        static bool LoadSymbols();
        Media( const rtl::OUString& url, Instance& instance );
        Media( const Media& other );
        Media& operator=( const Media& other );

        int getDuration() const;

        virtual ~Media();

        inline operator libvlc_media_t*()
        {
            return mMedia;
        }

    private:
        libvlc_media_t *mMedia;
    };
}
}
}

#endif
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
