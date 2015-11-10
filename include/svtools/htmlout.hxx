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

#ifndef INCLUDED_SVTOOLS_HTMLOUT_HXX
#define INCLUDED_SVTOOLS_HTMLOUT_HXX

#include <svtools/svtdllapi.h>
#include <tools/solar.h>
#include <rtl/textenc.h>
#include <rtl/string.hxx>
#include <svl/macitem.hxx>

#include "HtmlWriter.hxx"

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
    SVT_DLLPUBLIC static OString ConvertStringToHTML( const OUString& sSrc,
                        rtl_TextEncoding eDestEnc = RTL_TEXTENCODING_MS_1252,
                        OUString *pNonConvertableChars = nullptr );

    SVT_DLLPUBLIC static SvStream& Out_AsciiTag( SvStream&, const sal_Char* pStr,
                                   bool bOn = true,
                        rtl_TextEncoding eDestEnc = RTL_TEXTENCODING_MS_1252);
    SVT_DLLPUBLIC static SvStream& Out_Char( SvStream&, sal_Unicode cChar,
                        HTMLOutContext& rContext,
                        OUString *pNonConvertableChars = nullptr );
    SVT_DLLPUBLIC static SvStream& Out_String( SvStream&, const OUString&,
                        rtl_TextEncoding eDestEnc = RTL_TEXTENCODING_MS_1252,
                        OUString *pNonConvertableChars = nullptr );
    SVT_DLLPUBLIC static SvStream& Out_Hex( SvStream&, sal_uLong nHex, sal_uInt8 nLen,
                        rtl_TextEncoding eDestEnc = RTL_TEXTENCODING_MS_1252 );
    SVT_DLLPUBLIC static SvStream& Out_Color( SvStream&, const Color&,
                        rtl_TextEncoding eDestEnc = RTL_TEXTENCODING_MS_1252 );
    SVT_DLLPUBLIC static SvStream& Out_ImageMap( SvStream&, const OUString&, const ImageMap&, const OUString&,
                                   const HTMLOutEvent *pEventTable,
                                   bool bOutStarBasic,
                                   const sal_Char *pDelim = nullptr,
                                   const sal_Char *pIndentArea = nullptr,
                                   const sal_Char *pIndentMap = nullptr,
                        rtl_TextEncoding eDestEnc = RTL_TEXTENCODING_MS_1252,
                        OUString *pNonConvertableChars = nullptr );
    SVT_DLLPUBLIC static SvStream& FlushToAscii( SvStream&, HTMLOutContext& rContext );

    SVT_DLLPUBLIC static SvStream& OutScript( SvStream& rStrm,
                                const OUString& rBaseURL,
                                const OUString& rSource,
                                const OUString& rLanguage,
                                ScriptType eScriptType,
                                const OUString& rSrc,
                                const OUString *pSBLibrary = nullptr,
                                const OUString *pSBModule = nullptr,
                        rtl_TextEncoding eDestEnc = RTL_TEXTENCODING_MS_1252,
                        OUString *pNonConvertableChars = nullptr );

    // the 3rd parameter is an array of HTMLOutEvents which is terminated
    // by an entry that consists only of 0s
    SVT_DLLPUBLIC static SvStream& Out_Events( SvStream&, const SvxMacroTableDtor&,
                                 const HTMLOutEvent*, bool bOutStarBasic,
                        rtl_TextEncoding eDestEnc = RTL_TEXTENCODING_MS_1252,
                        OUString *pNonConvertableChars = nullptr );

    // <TD SDVAL="..." SDNUM="...">
    SVT_DLLPUBLIC static OString CreateTableDataOptionsValNum(
                bool bValue, double fVal, sal_uLong nFormat,
                SvNumberFormatter& rFormatter,
                rtl_TextEncoding eDestEnc = RTL_TEXTENCODING_MS_1252,
                OUString *pNonConvertableChars = nullptr);
};

struct HtmlWriterHelper
{
    SVT_DLLPUBLIC static void applyColor( HtmlWriter& rHtmlWriter, const OString &aAttributeName, const Color& rColor);
    SVT_DLLPUBLIC static void applyEvents(HtmlWriter& rHtmlWriter, const SvxMacroTableDtor& rMacroTable, const HTMLOutEvent* pEventTable, bool bOutStarBasic);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
