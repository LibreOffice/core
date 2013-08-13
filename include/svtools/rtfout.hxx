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

#ifndef _RTFOUT_HXX
#define _RTFOUT_HXX

#include "svtools/svtdllapi.h"
#include <tools/solar.h>

#include <rtl/textenc.h>

namespace rtl {
  class OUString;
};
class SvStream;

class SVT_DLLPUBLIC RTFOutFuncs
{
public:
#if defined(UNX)
    static const sal_Char sNewLine;     // nur \012 oder \015
#else
    static const sal_Char sNewLine[];   // \015\012
#endif

    static SvStream& Out_Char( SvStream&, sal_Unicode cChar,
                    int *pUCMode,
                    rtl_TextEncoding eDestEnc = RTL_TEXTENCODING_MS_1252,
                    sal_Bool bWriteHelpFile = sal_False );
    static SvStream& Out_String( SvStream&, const rtl::OUString&,
                    rtl_TextEncoding eDestEnc = RTL_TEXTENCODING_MS_1252,
                    sal_Bool bWriteHelpFile = sal_False );

    static SvStream& Out_Hex( SvStream&, sal_uLong nHex, sal_uInt8 nLen );
};


#endif


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
