/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

/* Typedefs and structures that represent the libvlc API / ABI */

#ifndef INCLUDED_AVMEDIA_SOURCE_VLC_WRAPPER_TYPES_HXX
#define INCLUDED_AVMEDIA_SOURCE_VLC_WRAPPER_TYPES_HXX

#include <sal/config.h>

#if defined(_WIN32)
        typedef __int64 libvlc_time_t;
#else
#include <stdint.h>
        typedef int64_t libvlc_time_t;
#endif

extern "C" {

// basic callback / event types we use
typedef int libvlc_event_type_t;
typedef struct libvlc_event_manager_t libvlc_event_manager_t;
typedef void ( *libvlc_callback_t ) ( const struct libvlc_event_t *, void * );

// the enumeration values we use cf. libvlc_events.h
#define libvlc_MediaPlayerPaused     0x105
#define libvlc_MediaPlayerEndReached 0x109

// event structure pieces we use
struct libvlc_event_t
{
    int   type;  // event type
    void *p_obj; // object emitting that event

    union // so far we don't need this.
    {
      struct {
        const char *dummy1;
        const char *dummy2;
      } padding;
    } u;
};

struct libvlc_track_description_t
{
    int i_id;
    char *psz_name;
    libvlc_track_description_t *p_next;
};

}

#endif // INCLUDED_AVMEDIA_SOURCE_VLC_WRAPPER_TYPES_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
