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

#include <imoptdlg.hxx>
#include <asciiopt.hxx>
#include <comphelper/string.hxx>
#include <osl/thread.h>
#include <global.hxx>

static const sal_Char pStrFix[] = "FIX";

//  The option string can no longer contain a semicolon (because of pick list),
//  therefore, starting with version 336 comma instead

ScImportOptions::ScImportOptions( const OUString& rStr )
{
    // Use the same string format as ScAsciiOptions,
    // because the import options string is passed here when a CSV file is loaded and saved again.
    // The old format is still supported because it might be used in macros.

    bFixedWidth = false;
    nFieldSepCode = 0;
    nTextSepCode = 0;
    eCharSet = RTL_TEXTENCODING_DONTKNOW;
    bSaveAsShown = true;    // "true" if not in string (after CSV import)
    bQuoteAllText = false;
    bSaveNumberAsSuch = true;
    bSaveFormulas = false;
    bRemoveSpace = false;
    sal_Int32 nTokenCount = comphelper::string::getTokenCount(rStr, ',');
    if ( nTokenCount >= 3 )
    {
        sal_Int32 nIdx{ 0 };
        // first 3 tokens: common
        OUString aToken( rStr.getToken( 0, ',', nIdx ) );
        if( aToken.equalsIgnoreAsciiCase( pStrFix ) )
            bFixedWidth = true;
        else
            nFieldSepCode = ScAsciiOptions::GetWeightedFieldSep( aToken, true);
        nTextSepCode  = static_cast<sal_Unicode>(rStr.getToken(0, ',', nIdx).toInt32());
        aStrFont      = rStr.getToken(0, ',', nIdx);
        eCharSet      = ScGlobal::GetCharsetValue(aStrFont);

        if ( nTokenCount == 4 )
        {
            // compatibility with old options string: "Save as shown" as 4th token, numeric
            bSaveAsShown = rStr.getToken(0, ',', nIdx).toInt32() != 0;
            bQuoteAllText = true;   // use old default then
        }
        else
        {
            // look at the same positions as in ScAsciiOptions
            if ( nTokenCount >= 7 )
                bQuoteAllText = rStr.getToken(3, ',', nIdx) == "true";  // 7th token
            if ( nTokenCount >= 8 )
                bSaveNumberAsSuch = rStr.getToken(0, ',', nIdx) == "true";
            if ( nTokenCount >= 9 )
                bSaveAsShown = rStr.getToken(0, ',', nIdx) == "true";
            if ( nTokenCount >= 10 )
                bSaveFormulas = rStr.getToken(0, ',', nIdx) == "true";
            if ( nTokenCount >= 11 )
                bRemoveSpace = rStr.getToken(0, ',', nIdx) == "true";
        }
    }
}

OUString ScImportOptions::BuildString() const
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
            "," +
            OUString::boolean( bSaveNumberAsSuch ) + // "save number as such": not in ScAsciiOptions
            "," +
            OUString::boolean( bSaveAsShown ) +  // "save as shown": not in ScAsciiOptions
            "," +
            OUString::boolean( bSaveFormulas ) +  // "save formulas": not in ScAsciiOptions
            "," +
            OUString::boolean( bRemoveSpace );    // same as "Remove space" in ScAsciiOptions

    return aResult;
}

void ScImportOptions::SetTextEncoding( rtl_TextEncoding nEnc )
{
    eCharSet = (nEnc == RTL_TEXTENCODING_DONTKNOW ?
        osl_getThreadTextEncoding() : nEnc);
    aStrFont = ScGlobal::GetCharsetString( nEnc );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
