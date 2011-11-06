/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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


