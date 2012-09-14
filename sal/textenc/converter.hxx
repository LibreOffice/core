/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef INCLUDED_SAL_TEXTENC_CONVERTER_HXX
#define INCLUDED_SAL_TEXTENC_CONVERTER_HXX

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
