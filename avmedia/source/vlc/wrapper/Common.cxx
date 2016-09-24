/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include "Common.hxx"
#include "SymbolLoader.hxx"

namespace
{
    const char AVMEDIA_NO_ERROR[] = "No error";

    const char* ( *libvlc_get_version ) (void);
    char *  ( * libvlc_errmsg ) (void);
}

namespace avmedia
{
namespace vlc
{
namespace wrapper
{
bool Common::LoadSymbols()
{
    ApiMap VLC_COMMON_API[] =
    {
        SYM_MAP( libvlc_get_version ),
        SYM_MAP( libvlc_errmsg )
    };

    return InitApiMap( VLC_COMMON_API );
}

const char* Common::Version()
{
    return libvlc_get_version();
}

const char* Common::LastErrorMessage()
{
    const char *errorMsg = libvlc_errmsg();
    return errorMsg == nullptr ? AVMEDIA_NO_ERROR : errorMsg;
}
}
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
