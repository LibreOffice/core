/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef _WRAPPER_INSTANCE_HXX
#define _WRAPPER_INSTANCE_HXX

struct libvlc_instance_t;

namespace VLC
{


    class Instance
    {
    public:
        static bool LoadSymbols();
        Instance( int argc, const char * const argv[] );
        Instance( const Instance& other );
        const Instance& operator=( const Instance& other );
        virtual ~Instance();

        inline operator libvlc_instance_t*()
        {
            return mInstance;
        }

    private:
        libvlc_instance_t *mInstance;
    };
}

#endif
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
