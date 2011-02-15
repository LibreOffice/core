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

#ifndef _RTFOUT_HXX
#define _RTFOUT_HXX

#include "svtools/svtdllapi.h"
#include <tools/solar.h>

#ifndef _RTL_TEXTENC_H_
#include <rtl/textenc.h>
#endif

class String;
class SvStream;

class SVT_DLLPUBLIC RTFOutFuncs
{
public:
#if defined(UNX)
    static const sal_Char sNewLine;     // nur \012 oder \015
#else
    static const sal_Char __FAR_DATA sNewLine[];    // \015\012
#endif

    static SvStream& Out_Char( SvStream&, sal_Unicode cChar,
                    int *pUCMode,
                    rtl_TextEncoding eDestEnc = RTL_TEXTENCODING_MS_1252,
                    sal_Bool bWriteHelpFile = sal_False );
    static SvStream& Out_String( SvStream&, const String&,
                    rtl_TextEncoding eDestEnc = RTL_TEXTENCODING_MS_1252,
                    sal_Bool bWriteHelpFile = sal_False );
    static SvStream& Out_Fontname( SvStream&, const String&,
                    rtl_TextEncoding eDestEnc = RTL_TEXTENCODING_MS_1252,
                    sal_Bool bWriteHelpFile = sal_False );

    static SvStream& Out_Hex( SvStream&, sal_uLong nHex, sal_uInt8 nLen );
};


#endif


