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
#include "Common.hxx"

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
    libvlc_media_t* ( *libvlc_media_new_location ) (libvlc_instance_t *p_instance, const char *psz_mrl);
    void ( *libvlc_media_release ) ( libvlc_media_t *p_md );
    void ( *libvlc_media_retain ) ( libvlc_media_t *p_md );
    libvlc_time_t ( *libvlc_media_get_duration ) ( libvlc_media_t *p_md );
    void ( *libvlc_media_parse ) ( libvlc_media_t *p_md );
    int ( *libvlc_media_is_parsed ) ( libvlc_media_t *p_md );
    char* ( *libvlc_media_get_mrl )(libvlc_media_t *p_md);


    libvlc_media_t* InitMedia( const rtl::OUString& url, Instance& instance )
    {
        rtl::OString dest;
        url.convertToString(&dest, RTL_TEXTENCODING_UTF8, 0);

        return libvlc_media_new_location(instance, dest.getStr());
    }
}

bool Media::LoadSymbols()
{
    ApiMap VLC_MEDIA_API[] =
    {
        SYM_MAP( libvlc_media_new_path ),
        SYM_MAP( libvlc_media_release ),
        SYM_MAP( libvlc_media_retain ),
        SYM_MAP( libvlc_media_get_duration ),
        SYM_MAP( libvlc_media_parse ),
        SYM_MAP( libvlc_media_is_parsed ),
        SYM_MAP( libvlc_media_get_mrl ),
        SYM_MAP( libvlc_media_new_location )
    };

    return InitApiMap( VLC_MEDIA_API );
}

Media::Media( const rtl::OUString& url, Instance& instance )
    : mMedia( InitMedia( url, instance ) )
{
    if (mMedia == nullptr)
    {
        // TODO: Error
    }
}

Media::Media( const Media& other )
{
    operator=( other );
}

Media& Media::operator=( const Media& other )
{
    libvlc_media_release( mMedia );
    mMedia = other.mMedia;

    libvlc_media_retain( mMedia );
    return *this;
}

int Media::getDuration() const
{
    if ( !libvlc_media_is_parsed( mMedia ) )
        libvlc_media_parse( mMedia );

    const int duration = libvlc_media_get_duration( mMedia );
    if (duration == -1)
    {
        SAL_WARN("avmedia", Common::LastErrorMessage());
        return 0;
    }
    else if (duration == 0)
    {
        // A duration must be greater than 0
        return 1;
    }

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
