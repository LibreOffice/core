/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include "imoptdlg.hxx"
#include "asciiopt.hxx"
#include "scresid.hxx"
#include <comphelper/string.hxx>
#include <osl/thread.h>
#include <rtl/tencinfo.h>

static const sal_Char pStrFix[] = "FIX";






ScImportOptions::ScImportOptions( const OUString& rStr )
{
    
    
    

    bFixedWidth = false;
    nFieldSepCode = 0;
    nTextSepCode = 0;
    eCharSet = RTL_TEXTENCODING_DONTKNOW;
    bSaveAsShown = true;    
    bQuoteAllText = false;
    bSaveFormulas = false;
    sal_Int32 nTokenCount = comphelper::string::getTokenCount(rStr, ',');
    if ( nTokenCount >= 3 )
    {
        
        OUString aToken( rStr.getToken( 0, ',' ) );
        if( aToken.equalsIgnoreAsciiCase( pStrFix ) )
            bFixedWidth = true;
        else
            nFieldSepCode = ScAsciiOptions::GetWeightedFieldSep( aToken, true);
        nTextSepCode  = (sal_Unicode) rStr.getToken(1,',').toInt32();
        aStrFont      = rStr.getToken(2,',');
        eCharSet      = ScGlobal::GetCharsetValue(aStrFont);

        if ( nTokenCount == 4 )
        {
            
            bSaveAsShown = (rStr.getToken( 3, ',' ).toInt32() ? sal_True : false);
            bQuoteAllText = true;   
        }
        else
        {
            
            if ( nTokenCount >= 7 )
                bQuoteAllText = rStr.getToken(6, ',').equalsAscii("true");
            if ( nTokenCount >= 9 )
                bSaveAsShown = rStr.getToken(8, ',').equalsAscii("true");
            if ( nTokenCount >= 10 )
                bSaveFormulas = rStr.getToken(9, ',').equalsAscii("true");
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
                                                 
            ",1,,0," +                           
            OUString::boolean( bQuoteAllText ) + 
            ",true," +                           
            OUString::boolean( bSaveAsShown ) +  
            "," +
            OUString::boolean( bSaveFormulas );  

    return aResult;
}



void ScImportOptions::SetTextEncoding( rtl_TextEncoding nEnc )
{
    eCharSet = (nEnc == RTL_TEXTENCODING_DONTKNOW ?
        osl_getThreadTextEncoding() : nEnc);
    aStrFont = ScGlobal::GetCharsetString( nEnc );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
