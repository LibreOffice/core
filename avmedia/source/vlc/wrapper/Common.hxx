/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_AVMEDIA_SOURCE_VLC_WRAPPER_COMMON_HXX
#define INCLUDED_AVMEDIA_SOURCE_VLC_WRAPPER_COMMON_HXX

namespace avmedia
{
namespace vlc
{
namespace wrapper
{
    class Common
    {
    public:
        static bool LoadSymbols();
        static const char* Version();
        static const char* LastErrorMessage();
    };
}
}
}

#endif
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */