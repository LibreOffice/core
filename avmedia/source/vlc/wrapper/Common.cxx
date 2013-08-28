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

namespace VLC
{
namespace
{
    const char* (*libvlc_get_version)(void);
}

bool Common::LoadSymbols()
{
    ApiMap VLC_COMMON_API[] =
    {
        SYM_MAP( libvlc_get_version )
    };

    return InitApiMap( VLC_COMMON_API );
}

const char* Common::Version()
{
    return libvlc_get_version();
}
}