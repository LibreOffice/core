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

#include <global.hxx>
#include <asciiopt.hxx>
#include <comphelper/string.hxx>
#include <osl/thread.h>

static const sal_Char pStrFix[] = "FIX";
static const sal_Char pStrMrg[] = "MRG";

ScAsciiOptions::ScAsciiOptions() :
    bFixedLen       ( false ),
    aFieldSeps      ( OUString(';') ),
    bMergeFieldSeps ( false ),
    bRemoveSpace    ( false ),
    bQuotedFieldAsText(false),
    bDetectSpecialNumber(false),
    bSkipEmptyCells(false),
    bSaveAsShown(true),
    bSaveFormulas(false),
    cTextSep        ( cDefaultTextSep ),
    eCharSet        ( osl_getThreadTextEncoding() ),
    eLang           ( LANGUAGE_SYSTEM ),
    bCharSetSystem  ( false ),
    nStartRow       ( 1 )
{
}

void ScAsciiOptions::SetColumnInfo( const ScCsvExpDataVec& rDataVec )
{
    sal_uInt16 nInfoCount = static_cast< sal_uInt16 >( rDataVec.size() );
    mvColStart.resize(nInfoCount);
    mvColFormat.resize(nInfoCount);
    for( sal_uInt16 nIx = 0; nIx < nInfoCount; ++nIx )
    {
        mvColStart[ nIx ] = rDataVec[ nIx ].mnIndex;
        mvColFormat[ nIx ] = rDataVec[ nIx ].mnType;
    }
}

static OUString lcl_decodeSepString( const OUString & rSepNums, bool & o_bMergeFieldSeps )
{
    OUString aFieldSeps;
    if ( rSepNums.isEmpty() )
        return aFieldSeps;

    sal_Int32 nPos = 0;
    do
    {
        const OUString aCode = rSepNums.getToken( 0, '/', nPos );
        if ( aCode == pStrMrg )
            o_bMergeFieldSeps = true;
        else
        {
            sal_Int32 nVal = aCode.toInt32();
            if ( nVal )
                aFieldSeps += OUStringLiteral1(nVal);
        }
    }
    while ( nPos >= 0 );

    return aFieldSeps;
}

// The options string must not contain semicolons (because of the pick list),
// use comma as separator.

void ScAsciiOptions::ReadFromString( const OUString& rString )
{
    sal_Int32 nPos = rString.isEmpty() ? -1 : 0;

    // Token 0: Field separator.
    if ( nPos >= 0 )
    {
        bFixedLen = bMergeFieldSeps = false;

        const OUString aToken = rString.getToken(0, ',', nPos);
        if ( aToken == pStrFix )
            bFixedLen = true;
        aFieldSeps = lcl_decodeSepString( aToken, bMergeFieldSeps);
    }

    // Token 1: Text separator.
    if ( nPos >= 0 )
    {
        const sal_Int32 nVal = rString.getToken(0, ',', nPos).toInt32();
        cTextSep = static_cast<sal_Unicode>(nVal);
    }

    // Token 2: Text encoding.
    if ( nPos >= 0 )
    {
        eCharSet = ScGlobal::GetCharsetValue( rString.getToken(0, ',', nPos) );
    }

    // Token 3: Number of start row.
    if ( nPos >= 0 )
    {
        nStartRow = rString.getToken(0, ',', nPos).toInt32();
    }

    // Token 4: Column info.
    if ( nPos >= 0 )
    {
        const OUString aToken = rString.getToken(0, ',', nPos);
        sal_Int32 nSub = comphelper::string::getTokenCount(aToken, '/');
        sal_Int32 nInfoCount = nSub / 2;
        mvColStart.resize(nInfoCount);
        mvColFormat.resize(nInfoCount);
        sal_Int32 nP = 0;
        for (sal_Int32 nInfo=0; nInfo<nInfoCount; ++nInfo)
        {
            mvColStart[nInfo]  = aToken.getToken(0, '/', nP).toInt32();
            mvColFormat[nInfo] = static_cast<sal_uInt8>(aToken.getToken(0, '/', nP).toInt32());
        }
    }

    // Token 5: Language.
    if (nPos >= 0)
    {
        eLang = static_cast<LanguageType>(rString.getToken(0, ',', nPos).toInt32());
    }

    // Token 6: Import quoted field as text.
    if (nPos >= 0)
    {
        bQuotedFieldAsText = rString.getToken(0, ',', nPos) == "true";
    }

    // Token 7: Detect special numbers.
    if (nPos >= 0)
    {
        bDetectSpecialNumber = rString.getToken(0, ',', nPos) == "true";
    }
    else
        bDetectSpecialNumber = true;    // default of versions that didn't add the parameter

    // Token 8: used for "Save as shown" in export options
    if ( nPos >= 0 )
    {
        bSaveAsShown = rString.getToken(0, ',', nPos) == "true";
    }
    else
        bSaveAsShown = true;    // default value

    // Token 9: used for "Save cell formulas" in export options
    if ( nPos >= 0 )
    {
        bSaveFormulas = rString.getToken(0, ',', nPos) == "true";
    }
    else
        bSaveFormulas = false;

    // Token 10: Boolean for Trim spaces.
    if (nPos >= 0)
    {
        bRemoveSpace = rString.getToken(0, ',', nPos) == "true";
    }
    else
        bRemoveSpace = false;
}

OUString ScAsciiOptions::WriteToString() const
{
    OUString aOutStr;

    // Token 0: Field separator.
    if ( bFixedLen )
        aOutStr += pStrFix;
    else if ( aFieldSeps.isEmpty() )
        aOutStr += "0";
    else
    {
        sal_Int32 nLen = aFieldSeps.getLength();
        for (sal_Int32 i=0; i<nLen; i++)
        {
            if (i)
                aOutStr += "/";
            aOutStr += OUString::number(aFieldSeps[i]);
        }
        if ( bMergeFieldSeps )
        {
            aOutStr += "/";
            aOutStr += pStrMrg;
        }
    }

    // Token 1: Text Quote character.
    aOutStr += "," + OUString::number(cTextSep) + ",";

    //Token 2: Text encoding.
    if ( bCharSetSystem )           // force "SYSTEM"
        aOutStr += ScGlobal::GetCharsetString( RTL_TEXTENCODING_DONTKNOW );
    else
        aOutStr += ScGlobal::GetCharsetString( eCharSet );

    //Token 3: Number of start row.
    aOutStr += "," + OUString::number(nStartRow) + ",";

    //Token 4: Column info.
    for (size_t nInfo=0; nInfo<mvColStart.size(); nInfo++)
    {
        if (nInfo)
            aOutStr += "/";
        aOutStr += OUString::number(mvColStart[nInfo]) +
                   "/" +
                   OUString::number(mvColFormat[nInfo]);
    }

    // #i112025# the options string is used in macros and linked sheets,
    // so new options must be added at the end, to remain compatible

    aOutStr += "," +
               //Token 5: Language
               OUString::number(static_cast<sal_uInt16>(eLang)) + "," +
               //Token 6: Import quoted field as text.
               OUString::boolean( bQuotedFieldAsText ) + "," +
               //Token 7: Detect special numbers.
               OUString::boolean( bDetectSpecialNumber ) + "," +
               // Token 8: used for "Save as shown" in export options
               OUString::boolean( bSaveAsShown ) + "," +
               // Token 9: used for "Save cell formulas" in export options
               OUString::boolean( bSaveFormulas ) + "," +
               //Token 10: Trim Space
               OUString::boolean( bRemoveSpace );
    return aOutStr;
}

// static
sal_Unicode ScAsciiOptions::GetWeightedFieldSep( const OUString & rFieldSeps, bool bDecodeNumbers )
{
    bool bMergeFieldSeps = false;
    OUString aFieldSeps( bDecodeNumbers ? lcl_decodeSepString( rFieldSeps, bMergeFieldSeps) : rFieldSeps);
    if (aFieldSeps.isEmpty())
    {
        return 0;
    }
    else if (aFieldSeps.getLength() == 1)
        return aFieldSeps[0];
    else
    {
        // There can be only one separator for output. See also fdo#53449
        if (aFieldSeps.indexOf(',') != -1)
            return ',';
        else if (aFieldSeps.indexOf('\t') != -1)
            return '\t';
        else if (aFieldSeps.indexOf(';') != -1)
            return ';';
        else if (aFieldSeps.indexOf(' ') != -1)
            return ' ';
        else
            return aFieldSeps[0];
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
