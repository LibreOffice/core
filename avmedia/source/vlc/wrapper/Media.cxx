/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <rtl/ustring.h>
#include "Media.hxx"
#include "SymbolLoader.hxx"
#include "Instance.hxx"
#include "Types.hxx"

struct libvlc_instance_t;

namespace avmedia
{
namespace vlc
{
namespace wrapper
{
namespace
{
    libvlc_media_t* ( *libvlc_media_new_path ) ( libvlc_instance_t *p_instance, const char *path );
    void ( *libvlc_media_release ) ( libvlc_media_t *p_md );
    void ( *libvlc_media_retain ) ( libvlc_media_t *p_md );
    libvlc_time_t ( *libvlc_media_get_duration ) ( libvlc_media_t *p_md );

    libvlc_media_t* InitMedia( const rtl::OUString& url, Instance& instance )
    {
        rtl::OString dest;
        url.convertToString(&dest, RTL_TEXTENCODING_UTF8, 0);

        return libvlc_media_new_path(instance, dest.getStr());
    }
}

bool Media::LoadSymbols()
{
    ApiMap VLC_MEDIA_API[] =
    {
        SYM_MAP( libvlc_media_new_path ),
        SYM_MAP( libvlc_media_release ),
        SYM_MAP( libvlc_media_retain ),
        SYM_MAP( libvlc_media_get_duration )
    };

    return InitApiMap( VLC_MEDIA_API );
}

Media::Media( const rtl::OUString& url, Instance& instance )
    : mMedia( InitMedia( url, instance ) )
{
}

Media::Media( const Media& other )
{
    operator=( other );
}

const Media& Media::operator=( const Media& other )
{
    libvlc_media_release( mMedia );
    mMedia = other.mMedia;

    libvlc_media_retain( mMedia );
    return *this;
}

int Media::getDuration() const
{
    const int duration = libvlc_media_get_duration( mMedia );
    if (duration == -1)
        return 0;

    return duration;
}

Media::~Media()
{
    libvlc_media_release( mMedia );
}
}
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
