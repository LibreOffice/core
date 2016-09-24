/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SAL_TEXTENC_CONVERTSIMPLE_HXX
#define INCLUDED_SAL_TEXTENC_CONVERTSIMPLE_HXX

#include "sal/config.h"

#include "sal/saldllapi.h"
#include "sal/types.h"

// Internal, non-stable ABI

namespace sal { namespace detail { namespace textenc {

sal_Size SAL_DLLPUBLIC convertCharToUnicode(
    void const * pData, void * pContext, char const * pSrcBuf,
    sal_Size nSrcBytes, sal_Unicode * pDestBuf, sal_Size nDestChars,
    sal_uInt32 nFlags, sal_uInt32 * pInfo, sal_Size * pSrcCvtBytes);

sal_Size SAL_DLLPUBLIC convertUnicodeToChar(
    void const * pData, void * pContext, sal_Unicode const * pSrcBuf,
    sal_Size nSrcChars, char * pDestBuf, sal_Size nDestBytes, sal_uInt32 nFlags,
    sal_uInt32 * pInfo, sal_Size * pSrcCvtChars);

} } }

#endif
