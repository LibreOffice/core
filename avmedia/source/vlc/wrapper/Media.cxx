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

struct libvlc_instance_t;
namespace VLC
{
    namespace
    {
        libvlc_media_t* ( *libvlc_media_new_path ) ( libvlc_instance_t *p_instance, const char *path );
        void ( *libvlc_media_release ) ( libvlc_media_t *p_md );
        void ( *libvlc_media_retain ) ( libvlc_media_t *p_md );

        ApiMap VLC_MEDIA_API[] =
        {
            SYM_MAP( libvlc_media_new_path ),
            SYM_MAP( libvlc_media_release ),
            SYM_MAP( libvlc_media_retain )
        };

        libvlc_media_t* InitMedia( const rtl::OUString& url, VLC::Instance& instance )
        {
            rtl::OString dest;
            url.convertToString(&dest, RTL_TEXTENCODING_UTF8, 0);

            return libvlc_media_new_path(instance, dest.getStr());
        }

    }


Media::Media( const rtl::OUString& url, Instance& instance )
{
    InitApiMap(VLC_MEDIA_API);
    mMedia = InitMedia( url, instance );
}

Media::Media( const Media& other )
    : mMedia( other.mMedia )
{
    libvlc_media_retain( mMedia );
}

const Media& Media::operator=( const Media& other )
{
    libvlc_media_release( mMedia );
    mMedia = other.mMedia;

    libvlc_media_retain( mMedia );
}

Media::~Media()
{
    libvlc_media_release( mMedia );
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
