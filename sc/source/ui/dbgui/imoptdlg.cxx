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



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"



#include "imoptdlg.hxx"
#include "scresid.hxx"
#include "imoptdlg.hrc"
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

    bFixedWidth = sal_False;
    nFieldSepCode = 0;
    nTextSepCode = 0;
    eCharSet = RTL_TEXTENCODING_DONTKNOW;
    bSaveAsShown = sal_True;    // "true" if not in string (after CSV import)
    bQuoteAllText = sal_False;
    xub_StrLen nTokenCount = rStr.GetTokenCount(',');
    if ( nTokenCount >= 3 )
    {
        // first 3 tokens: common
        String aToken( rStr.GetToken( 0, ',' ) );
        if( aToken.EqualsIgnoreCaseAscii( pStrFix ) )
            bFixedWidth = sal_True;
        else
            nFieldSepCode = (sal_Unicode) aToken.ToInt32();
        nTextSepCode  = (sal_Unicode) rStr.GetToken(1,',').ToInt32();
        aStrFont      = rStr.GetToken(2,',');
        eCharSet      = ScGlobal::GetCharsetValue(aStrFont);

        if ( nTokenCount == 4 )
        {
            // compatibility with old options string: "Save as shown" as 4th token, numeric
            bSaveAsShown = (rStr.GetToken( 3, ',' ).ToInt32() ? sal_True : sal_False);
            bQuoteAllText = sal_True;   // use old default then
        }
        else
        {
            // look at the same positions as in ScAsciiOptions
            if ( nTokenCount >= 7 )
                bQuoteAllText = rStr.GetToken(6, ',').EqualsAscii("true");
            if ( nTokenCount >= 9 )
                bSaveAsShown = rStr.GetToken(8, ',').EqualsAscii("true");
        }
    }
}

//------------------------------------------------------------------------

String ScImportOptions::BuildString() const
{
    String  aResult;

    if( bFixedWidth )
        aResult.AppendAscii( pStrFix );
    else
        aResult += String::CreateFromInt32(nFieldSepCode);
    aResult += ',';
    aResult += String::CreateFromInt32(nTextSepCode);
    aResult += ',';
    aResult += aStrFont;
                                                            // use the same string format as ScAsciiOptions:
    aResult.AppendAscii( ",1,,0," );                        // first row, no column info, default language
    aResult.AppendAscii(bQuoteAllText ? "true" : "false");  // same as "quoted field as text" in ScAsciiOptions
    aResult.AppendAscii( ",true," );                        // "detect special numbers"
    aResult.AppendAscii(bSaveAsShown ? "true" : "false");   // "save as shown": not in ScAsciiOptions

    return aResult;
}

//------------------------------------------------------------------------

void ScImportOptions::SetTextEncoding( rtl_TextEncoding nEnc )
{
    eCharSet = (nEnc == RTL_TEXTENCODING_DONTKNOW ?
        gsl_getSystemTextEncoding() : nEnc);
    aStrFont = ScGlobal::GetCharsetString( nEnc );
}
