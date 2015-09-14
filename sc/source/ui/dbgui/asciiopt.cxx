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

#include "global.hxx"
#include "scresid.hxx"
#include "impex.hxx"
#include "asciiopt.hxx"
#include "asciiopt.hrc"
#include <comphelper/string.hxx>
#include <osl/thread.h>
#include <rtl/tencinfo.h>
#include <unotools/transliterationwrapper.hxx>
#include "editutil.hxx"

static const sal_Char pStrFix[] = "FIX";
static const sal_Char pStrMrg[] = "MRG";

ScAsciiOptions::ScAsciiOptions() :
    bFixedLen       ( false ),
    aFieldSeps      ( OUString(';') ),
    bMergeFieldSeps ( false ),
    bQuotedFieldAsText(false),
    bDetectSpecialNumber(false),
    cTextSep        ( cDefaultTextSep ),
    eCharSet        ( osl_getThreadTextEncoding() ),
    eLang           ( LANGUAGE_SYSTEM ),
    bCharSetSystem  ( false ),
    nStartRow       ( 1 ),
    nInfoCount      ( 0 ),
    pColStart       ( NULL ),
    pColFormat      ( NULL )
{
}

ScAsciiOptions::ScAsciiOptions(const ScAsciiOptions& rOpt) :
    bFixedLen       ( rOpt.bFixedLen ),
    aFieldSeps      ( rOpt.aFieldSeps ),
    bMergeFieldSeps ( rOpt.bMergeFieldSeps ),
    bQuotedFieldAsText(rOpt.bQuotedFieldAsText),
    bDetectSpecialNumber(rOpt.bDetectSpecialNumber),
    cTextSep        ( rOpt.cTextSep ),
    eCharSet        ( rOpt.eCharSet ),
    eLang           ( rOpt.eLang ),
    bCharSetSystem  ( rOpt.bCharSetSystem ),
    nStartRow       ( rOpt.nStartRow ),
    nInfoCount      ( rOpt.nInfoCount )
{
    if (nInfoCount)
    {
        pColStart = new sal_Int32[nInfoCount];
        pColFormat = new sal_uInt8[nInfoCount];
        for (sal_uInt16 i=0; i<nInfoCount; i++)
        {
            pColStart[i] = rOpt.pColStart[i];
            pColFormat[i] = rOpt.pColFormat[i];
        }
    }
    else
    {
        pColStart = NULL;
        pColFormat = NULL;
    }
}

ScAsciiOptions::~ScAsciiOptions()
{
    delete[] pColStart;
    delete[] pColFormat;
}

void ScAsciiOptions::SetColInfo( sal_uInt16 nCount, const sal_Int32* pStart, const sal_uInt8* pFormat )
{
    delete[] pColStart;
    delete[] pColFormat;

    nInfoCount = nCount;

    if (nInfoCount)
    {
        pColStart = new sal_Int32[nInfoCount];
        pColFormat = new sal_uInt8[nInfoCount];
        for (sal_uInt16 i=0; i<nInfoCount; i++)
        {
            pColStart[i] = pStart[i];
            pColFormat[i] = pFormat[i];
        }
    }
    else
    {
        pColStart = NULL;
        pColFormat = NULL;
    }
}

void ScAsciiOptions::SetColumnInfo( const ScCsvExpDataVec& rDataVec )
{
    delete[] pColStart;
    pColStart = NULL;
    delete[] pColFormat;
    pColFormat = NULL;

    nInfoCount = static_cast< sal_uInt16 >( rDataVec.size() );
    if( nInfoCount )
    {
        pColStart = new sal_Int32[ nInfoCount ];
        pColFormat = new sal_uInt8[ nInfoCount ];
        for( sal_uInt16 nIx = 0; nIx < nInfoCount; ++nIx )
        {
            pColStart[ nIx ] = rDataVec[ nIx ].mnIndex;
            pColFormat[ nIx ] = rDataVec[ nIx ].mnType;
        }
    }
}

ScAsciiOptions& ScAsciiOptions::operator=( const ScAsciiOptions& rCpy )
{
    SetColInfo( rCpy.nInfoCount, rCpy.pColStart, rCpy.pColFormat );

    bFixedLen       = rCpy.bFixedLen;
    aFieldSeps      = rCpy.aFieldSeps;
    bMergeFieldSeps = rCpy.bMergeFieldSeps;
    bQuotedFieldAsText = rCpy.bQuotedFieldAsText;
    cTextSep        = rCpy.cTextSep;
    eCharSet        = rCpy.eCharSet;
    bCharSetSystem  = rCpy.bCharSetSystem;
    nStartRow       = rCpy.nStartRow;

    return *this;
}

bool ScAsciiOptions::operator==( const ScAsciiOptions& rCmp ) const
{
    if ( bFixedLen       == rCmp.bFixedLen &&
         aFieldSeps      == rCmp.aFieldSeps &&
         bMergeFieldSeps == rCmp.bMergeFieldSeps &&
         bQuotedFieldAsText == rCmp.bQuotedFieldAsText &&
         cTextSep        == rCmp.cTextSep &&
         eCharSet        == rCmp.eCharSet &&
         bCharSetSystem  == rCmp.bCharSetSystem &&
         nStartRow       == rCmp.nStartRow &&
         nInfoCount      == rCmp.nInfoCount )
    {
        OSL_ENSURE( !nInfoCount || (pColStart && pColFormat && rCmp.pColStart && rCmp.pColFormat),
                     "NULL pointer in ScAsciiOptions::operator==() column info" );
        for (sal_uInt16 i=0; i<nInfoCount; i++)
            if ( pColStart[i] != rCmp.pColStart[i] ||
                 pColFormat[i] != rCmp.pColFormat[i] )
                return false;

        return true;
    }
    return false;
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
                aFieldSeps += OUString((sal_Unicode) nVal);
        }
    }
    while ( nPos >= 0 );

    return aFieldSeps;
}

// The options string must not contain semicolons (because of the pick list),
// use comma as separator.

void ScAsciiOptions::ReadFromString( const OUString& rString )
{
    sal_Int32 nCount = comphelper::string::getTokenCount(rString, ',');
    OUString aToken;

    // Field separator.
    if ( nCount >= 1 )
    {
        bFixedLen = bMergeFieldSeps = false;

        aToken = rString.getToken(0,',');
        if ( aToken == pStrFix )
            bFixedLen = true;
        aFieldSeps = lcl_decodeSepString( aToken, bMergeFieldSeps);
    }

    // Text separator.
    if ( nCount >= 2 )
    {
        aToken = rString.getToken(1,',');
        sal_Int32 nVal = aToken.toInt32();
        cTextSep = (sal_Unicode) nVal;
    }

    // Text encoding.
    if ( nCount >= 3 )
    {
        aToken = rString.getToken(2,',');
        eCharSet = ScGlobal::GetCharsetValue( aToken );
    }

    // Number of start row.
    if ( nCount >= 4 )
    {
        aToken = rString.getToken(3,',');
        nStartRow = aToken.toInt32();
    }

    // Column info.
    if ( nCount >= 5 )
    {
        delete[] pColStart;
        delete[] pColFormat;

        aToken = rString.getToken(4,',');
        sal_Int32 nSub = comphelper::string::getTokenCount(aToken, '/');
        nInfoCount = nSub / 2;
        if (nInfoCount)
        {
            pColStart = new sal_Int32[nInfoCount];
            pColFormat = new sal_uInt8[nInfoCount];
            for (sal_uInt16 nInfo=0; nInfo<nInfoCount; nInfo++)
            {
                pColStart[nInfo]  = (sal_Int32) aToken.getToken( 2*nInfo, '/' ).toInt32();
                pColFormat[nInfo] = (sal_uInt8) aToken.getToken( 2*nInfo+1, '/' ).toInt32();
            }
        }
        else
        {
            pColStart = NULL;
            pColFormat = NULL;
        }
    }

    // Language
    if (nCount >= 6)
    {
        aToken = rString.getToken(5, ',');
        eLang = static_cast<LanguageType>(aToken.toInt32());
    }

    // Import quoted field as text.
    if (nCount >= 7)
    {
        aToken = rString.getToken(6, ',');
        bQuotedFieldAsText = aToken == "true";
    }

    // Detect special numbers.
    if (nCount >= 8)
    {
        aToken = rString.getToken(7, ',');
        bDetectSpecialNumber = aToken == "true";
    }
    else
        bDetectSpecialNumber = true;    // default of versions that didn't add the parameter

    // 9th token is used for "Save as shown" in export options
    // 10th token is used for "Save cell formulas" in export options
}

OUString ScAsciiOptions::WriteToString() const
{
    OUString aOutStr;

    // Field separator.
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

    // Text delimiter.
    aOutStr += "," + OUString::number(cTextSep) + ",";

    // Text encoding.
    if ( bCharSetSystem )           // force "SYSTEM"
        aOutStr += ScGlobal::GetCharsetString( RTL_TEXTENCODING_DONTKNOW );
    else
        aOutStr += ScGlobal::GetCharsetString( eCharSet );

    // Number of start row.
    aOutStr += "," + OUString::number(nStartRow) + ",";

    // Column info.
    OSL_ENSURE( !nInfoCount || (pColStart && pColFormat), "NULL pointer in ScAsciiOptions column info" );
    for (sal_uInt16 nInfo=0; nInfo<nInfoCount; nInfo++)
    {
        if (nInfo)
            aOutStr += "/";
        aOutStr += OUString::number(pColStart[nInfo]) +
                   "/" +
                   OUString::number(pColFormat[nInfo]);
    }

    // #i112025# the options string is used in macros and linked sheets,
    // so new options must be added at the end, to remain compatible

    aOutStr += "," +
               // Language
               OUString::number(eLang) + "," +
               // Import quoted field as text.
               OUString::boolean( bQuotedFieldAsText ) + "," +
               // Detect special numbers.
               OUString::boolean( bDetectSpecialNumber );

    // 9th token is used for "Save as shown" in export options
    // 10th token is used for "Save cell formulas" in export options

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
