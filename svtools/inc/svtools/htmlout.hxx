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

#ifndef _HTMLOUT_HXX
#define _HTMLOUT_HXX

#include "svtools/svtdllapi.h"
#include <tools/solar.h>
#include <rtl/textenc.h>
#include <rtl/string.hxx>
#include <svl/macitem.hxx>

class Color;
class ImageMap;
class String;
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

    SVT_DLLPUBLIC static rtl::OString ConvertStringToHTML( const String& sSrc,
                        rtl_TextEncoding eDestEnc = RTL_TEXTENCODING_MS_1252,
                        String *pNonConvertableChars = 0 );

    SVT_DLLPUBLIC static SvStream& Out_AsciiTag( SvStream&, const sal_Char* pStr,
                                   sal_Bool bOn = sal_True,
                        rtl_TextEncoding eDestEnc = RTL_TEXTENCODING_MS_1252);
    SVT_DLLPUBLIC static SvStream& Out_Char( SvStream&, sal_Unicode cChar,
                        HTMLOutContext& rContext,
                        String *pNonConvertableChars = 0 );
    SVT_DLLPUBLIC static SvStream& Out_String( SvStream&, const String&,
                        rtl_TextEncoding eDestEnc = RTL_TEXTENCODING_MS_1252,
                        String *pNonConvertableChars = 0 );
    SVT_DLLPUBLIC static SvStream& Out_Hex( SvStream&, sal_uLong nHex, sal_uInt8 nLen,
                        rtl_TextEncoding eDestEnc = RTL_TEXTENCODING_MS_1252 );
    SVT_DLLPUBLIC static SvStream& Out_Color( SvStream&, const Color&,
                        rtl_TextEncoding eDestEnc = RTL_TEXTENCODING_MS_1252 );
    SVT_DLLPUBLIC static SvStream& Out_ImageMap( SvStream&, const String&, const ImageMap&, const String&,
                                   const HTMLOutEvent *pEventTable,
                                   sal_Bool bOutStarBasic,
                                   const sal_Char *pDelim = 0,
                                   const sal_Char *pIndentArea = 0,
                                   const sal_Char *pIndentMap = 0,
                        rtl_TextEncoding eDestEnc = RTL_TEXTENCODING_MS_1252,
                        String *pNonConvertableChars = 0 );
    SVT_DLLPUBLIC static SvStream& FlushToAscii( SvStream&, HTMLOutContext& rContext );

    SVT_DLLPUBLIC static SvStream& OutScript( SvStream& rStrm,
                                const String& rBaseURL,
                                const String& rSource,
                                const String& rLanguage,
                                ScriptType eScriptType,
                                const String& rSrc,
                                const String *pSBLibrary = 0,
                                const String *pSBModule = 0,
                        rtl_TextEncoding eDestEnc = RTL_TEXTENCODING_MS_1252,
                        String *pNonConvertableChars = 0 );

    // der 3. Parameter ist ein Array von HTMLOutEvents, das mit einem
    // nur aus 0 bestehen Eintrag terminiert ist.
    SVT_DLLPUBLIC static SvStream& Out_Events( SvStream&, const SvxMacroTableDtor&,
                                 const HTMLOutEvent*, sal_Bool bOutStarBasic,
                        rtl_TextEncoding eDestEnc = RTL_TEXTENCODING_MS_1252,
                        String *pNonConvertableChars = 0 );

    // <TD SDVAL="..." SDNUM="...">
    SVT_DLLPUBLIC static rtl::OString CreateTableDataOptionsValNum(
                sal_Bool bValue, double fVal, sal_uLong nFormat,
                SvNumberFormatter& rFormatter,
                rtl_TextEncoding eDestEnc = RTL_TEXTENCODING_MS_1252,
                String *pNonConvertableChars = 0);

    SVT_DLLPUBLIC static sal_Bool PrivateURLToInternalImg( String& rURL );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
