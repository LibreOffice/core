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

#ifndef _HTMLOUT_HXX
#define _HTMLOUT_HXX

#include "svtools/svtdllapi.h"
#include <tools/solar.h>
#include <rtl/textenc.h>
#include <rtl/string.hxx>
#include <svl/macitem.hxx>

class Color;
class ImageMap;
class SvStream;
class SvxMacroTableDtor;
class SvNumberFormatter;

struct HTMLOutEvent
{
    const sal_Char *pBasicName;
    const sal_Char *pJavaName;
    sal_uInt16 nEvent;
};

struct SVT_DLLPUBLIC HTMLOutContext
{
    rtl_TextEncoding m_eDestEnc;
    rtl_TextToUnicodeConverter m_hConv;
    rtl_TextToUnicodeContext   m_hContext;

    HTMLOutContext( rtl_TextEncoding eDestEnc );
    ~HTMLOutContext();
};

struct HTMLOutFuncs
{
#if defined(UNX)
    static const sal_Char sNewLine;     // nur \012 oder \015
#else
    static const sal_Char sNewLine[];   // \015\012
#endif

    SVT_DLLPUBLIC static OString ConvertStringToHTML( const OUString& sSrc,
                        rtl_TextEncoding eDestEnc = RTL_TEXTENCODING_MS_1252,
                        OUString *pNonConvertableChars = 0 );

    SVT_DLLPUBLIC static SvStream& Out_AsciiTag( SvStream&, const sal_Char* pStr,
                                   sal_Bool bOn = sal_True,
                        rtl_TextEncoding eDestEnc = RTL_TEXTENCODING_MS_1252);
    SVT_DLLPUBLIC static SvStream& Out_Char( SvStream&, sal_Unicode cChar,
                        HTMLOutContext& rContext,
                        OUString *pNonConvertableChars = 0 );
    SVT_DLLPUBLIC static SvStream& Out_String( SvStream&, const OUString&,
                        rtl_TextEncoding eDestEnc = RTL_TEXTENCODING_MS_1252,
                        OUString *pNonConvertableChars = 0 );
    SVT_DLLPUBLIC static SvStream& Out_Hex( SvStream&, sal_uLong nHex, sal_uInt8 nLen,
                        rtl_TextEncoding eDestEnc = RTL_TEXTENCODING_MS_1252 );
    SVT_DLLPUBLIC static SvStream& Out_Color( SvStream&, const Color&,
                        rtl_TextEncoding eDestEnc = RTL_TEXTENCODING_MS_1252 );
    SVT_DLLPUBLIC static SvStream& Out_ImageMap( SvStream&, const OUString&, const ImageMap&, const OUString&,
                                   const HTMLOutEvent *pEventTable,
                                   sal_Bool bOutStarBasic,
                                   const sal_Char *pDelim = 0,
                                   const sal_Char *pIndentArea = 0,
                                   const sal_Char *pIndentMap = 0,
                        rtl_TextEncoding eDestEnc = RTL_TEXTENCODING_MS_1252,
                        OUString *pNonConvertableChars = 0 );
    SVT_DLLPUBLIC static SvStream& FlushToAscii( SvStream&, HTMLOutContext& rContext );

    SVT_DLLPUBLIC static SvStream& OutScript( SvStream& rStrm,
                                const OUString& rBaseURL,
                                const OUString& rSource,
                                const OUString& rLanguage,
                                ScriptType eScriptType,
                                const OUString& rSrc,
                                const OUString *pSBLibrary = 0,
                                const OUString *pSBModule = 0,
                        rtl_TextEncoding eDestEnc = RTL_TEXTENCODING_MS_1252,
                        OUString *pNonConvertableChars = 0 );

    // der 3. Parameter ist ein Array von HTMLOutEvents, das mit einem
    // nur aus 0 bestehen Eintrag terminiert ist.
    SVT_DLLPUBLIC static SvStream& Out_Events( SvStream&, const SvxMacroTableDtor&,
                                 const HTMLOutEvent*, sal_Bool bOutStarBasic,
                        rtl_TextEncoding eDestEnc = RTL_TEXTENCODING_MS_1252,
                        OUString *pNonConvertableChars = 0 );

    // <TD SDVAL="..." SDNUM="...">
    SVT_DLLPUBLIC static OString CreateTableDataOptionsValNum(
                sal_Bool bValue, double fVal, sal_uLong nFormat,
                SvNumberFormatter& rFormatter,
                rtl_TextEncoding eDestEnc = RTL_TEXTENCODING_MS_1252,
                OUString *pNonConvertableChars = 0);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
