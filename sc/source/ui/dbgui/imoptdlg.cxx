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

#include "imoptdlg.hxx"
#include "asciiopt.hxx"
#include "scresid.hxx"
#include "imoptdlg.hrc"
#include <comphelper/string.hxx>
#include <rtl/tencinfo.h>

static const sal_Char pStrFix[] = "FIX";

//------------------------------------------------------------------------
//  Der Options-String darf kein Semikolon mehr enthalten (wegen Pickliste)
//  darum ab Version 336 Komma stattdessen


ScImportOptions::ScImportOptions( const String& rStr )
{
    // Use the same string format as ScAsciiOptions,
    // because the import options string is passed here when a CSV file is loaded and saved again.
    // The old format is still supported because it might be used in macros.

    bFixedWidth = false;
    nFieldSepCode = 0;
    nTextSepCode = 0;
    eCharSet = RTL_TEXTENCODING_DONTKNOW;
    bSaveAsShown = sal_True;    // "true" if not in string (after CSV import)
    bQuoteAllText = false;
    bSaveFormulas = false;
    xub_StrLen nTokenCount = comphelper::string::getTokenCount(rStr, ',');
    if ( nTokenCount >= 3 )
    {
        // first 3 tokens: common
        String aToken( rStr.GetToken( 0, ',' ) );
        if( aToken.EqualsIgnoreCaseAscii( pStrFix ) )
            bFixedWidth = sal_True;
        else
            nFieldSepCode = ScAsciiOptions::GetWeightedFieldSep( aToken, true);
        nTextSepCode  = (sal_Unicode) rStr.GetToken(1,',').ToInt32();
        aStrFont      = rStr.GetToken(2,',');
        eCharSet      = ScGlobal::GetCharsetValue(aStrFont);

        if ( nTokenCount == 4 )
        {
            // compatibility with old options string: "Save as shown" as 4th token, numeric
            bSaveAsShown = (rStr.GetToken( 3, ',' ).ToInt32() ? sal_True : false);
            bQuoteAllText = sal_True;   // use old default then
        }
        else
        {
            // look at the same positions as in ScAsciiOptions
            if ( nTokenCount >= 7 )
                bQuoteAllText = rStr.GetToken(6, ',').EqualsAscii("true");
            if ( nTokenCount >= 9 )
                bSaveAsShown = rStr.GetToken(8, ',').EqualsAscii("true");
            if ( nTokenCount >= 10 )
                bSaveFormulas = rStr.GetToken(9, ',').EqualsAscii("true");
        }
    }
}

//------------------------------------------------------------------------

String ScImportOptions::BuildString() const
{
    OUString aResult;

    if( bFixedWidth )
        aResult += pStrFix;
    else
        aResult += OUString::number(nFieldSepCode);
    aResult += "," + OUString::number(nTextSepCode) + "," + aStrFont +
                                                 // use the same string format as ScAsciiOptions:
            ",1,,0," +                           // first row, no column info, default language
            OUString::boolean( bQuoteAllText ) + // same as "quoted field as text" in ScAsciiOptions
            ",true," +                           // "detect special numbers"
            OUString::boolean( bSaveAsShown ) +  // "save as shown": not in ScAsciiOptions
            "," +
            OUString::boolean( bSaveFormulas );  // "save formulas": not in ScAsciiOptions

    return aResult;
}

//------------------------------------------------------------------------

void ScImportOptions::SetTextEncoding( rtl_TextEncoding nEnc )
{
    eCharSet = (nEnc == RTL_TEXTENCODING_DONTKNOW ?
        osl_getThreadTextEncoding() : nEnc);
    aStrFont = ScGlobal::GetCharsetString( nEnc );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
