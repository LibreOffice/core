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

#pragma once

#include <sal/config.h>

#include <string_view>

#include <svtools/svtdllapi.h>
#include <rtl/string.hxx>
#include <svl/macitem.hxx>

class Color;
class ImageMap;
class HtmlWriter;
class SvStream;
class SvNumberFormatter;

struct HTMLOutEvent
{
    const char *pBasicName;
    const char *pJavaName;
    SvMacroItemId nEvent;
};

struct HTMLOutFuncs
{
    SVT_DLLPUBLIC static OString ConvertStringToHTML( std::u16string_view sSrc,
                        OUString *pNonConvertableChars = nullptr );

    SVT_DLLPUBLIC static SvStream& Out_AsciiTag( SvStream&, std::string_view rStr,
                                   bool bOn = true);
    SVT_DLLPUBLIC static SvStream& Out_Char( SvStream&, sal_uInt32 cChar,
                        OUString *pNonConvertableChars = nullptr );
    SVT_DLLPUBLIC static SvStream& Out_String( SvStream&, std::u16string_view,
                        OUString *pNonConvertableChars = nullptr );
    SVT_DLLPUBLIC static SvStream& Out_Hex( SvStream&, sal_uInt32 nHex, sal_uInt8 nLen );
    SVT_DLLPUBLIC static SvStream& Out_Color( SvStream&, const Color&, bool bXHTML = false );
    SVT_DLLPUBLIC static SvStream& Out_ImageMap( SvStream&, const OUString&, const ImageMap&, const OUString&,
                                   const HTMLOutEvent *pEventTable,
                                   bool bOutStarBasic,
                                   const char *pDelim,
                                   const char *pIndentArea,
                                   const char *pIndentMap );
    SVT_DLLPUBLIC static SvStream& FlushToAscii( SvStream& );

    SVT_DLLPUBLIC static SvStream& OutScript( SvStream& rStrm,
                                const OUString& rBaseURL,
                                std::u16string_view rSource,
                                std::u16string_view rLanguage,
                                ScriptType eScriptType,
                                const OUString& rSrc,
                                const OUString *pSBLibrary,
                                const OUString *pSBModule );

    // the 3rd parameter is an array of HTMLOutEvents which is terminated
    // by an entry that consists only of 0s
    SVT_DLLPUBLIC static SvStream& Out_Events( SvStream&, const SvxMacroTableDtor&,
                                    const HTMLOutEvent*, bool bOutStarBasic,
                                    OUString *pNonConvertableChars = nullptr );

    // <TD SDVAL="..." SDNUM="...">
    SVT_DLLPUBLIC static OString CreateTableDataOptionsValNum(
                bool bValue, double fVal, sal_uInt32 nFormat,
                SvNumberFormatter& rFormatter,
                OUString *pNonConvertableChars = nullptr);
    SVT_DLLPUBLIC static bool PrivateURLToInternalImg( OUString& rURL );
};

struct HtmlWriterHelper
{
    SVT_DLLPUBLIC static void applyColor( HtmlWriter& rHtmlWriter, std::string_view aAttributeName, const Color& rColor);
    SVT_DLLPUBLIC static void applyEvents(HtmlWriter& rHtmlWriter, const SvxMacroTableDtor& rMacroTable, const HTMLOutEvent* pEventTable, bool bOutStarBasic);
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
