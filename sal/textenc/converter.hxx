/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef INCLUDED_SAL_TEXTENC_CONVERTER_HXX
#define INCLUDED_SAL_TEXTENC_CONVERTER_HXX

#include "sal/config.h"
#include "sal/saldllapi.h"
#include "sal/types.h"

// Internal, non-stable ABI

namespace sal { namespace detail { namespace textenc {

enum BadInputConversionAction
{
    BAD_INPUT_STOP,
    BAD_INPUT_CONTINUE,
    BAD_INPUT_NO_OUTPUT
};

BadInputConversionAction SAL_DLLPUBLIC
handleBadInputTextToUnicodeConversion(
    bool bUndefined, bool bMultiByte, char cByte, sal_uInt32 nFlags,
    sal_Unicode ** pDestBufPtr, sal_Unicode * pDestBufEnd, sal_uInt32 * pInfo);

BadInputConversionAction SAL_DLLPUBLIC
handleBadInputUnicodeToTextConversion(
    bool bUndefined, sal_uInt32 nUtf32, sal_uInt32 nFlags, char ** pDestBufPtr,
    char * pDestBufEnd, sal_uInt32 * pInfo, char const * pPrefix,
    sal_Size nPrefixLen, bool * pPrefixWritten);

} } }

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
