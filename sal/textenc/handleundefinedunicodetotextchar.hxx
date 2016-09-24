/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SAL_TEXTENC_HANDLEUNDEFINEDUNICODETOTEXTCHAR_HXX
#define INCLUDED_SAL_TEXTENC_HANDLEUNDEFINEDUNICODETOTEXTCHAR_HXX

#include "sal/config.h"

#include "sal/saldllapi.h"
#include "sal/types.h"

// Internal, non-stable ABI

namespace sal { namespace detail { namespace textenc {

// True means "continue," false means "break:"
bool SAL_DLLPUBLIC handleUndefinedUnicodeToTextChar(
    sal_Unicode const ** ppSrcBuf, sal_Unicode const * pEndSrcBuf,
    char ** ppDestBuf, char const * pEndDestBuf, sal_uInt32 nFlags,
    sal_uInt32 * pInfo);

} } }

#endif
