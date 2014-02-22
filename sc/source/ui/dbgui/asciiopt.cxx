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
    sal_Int32 nSub = comphelper::string::getTokenCount( rSepNums, '/');
    for (sal_Int32 i=0; i<nSub; ++i)
    {
        OUString aCode = rSepNums.getToken( i, '/' );
        if ( aCode.equalsAscii(pStrMrg) )
            o_bMergeFieldSeps = true;
        else
        {
            sal_Int32 nVal = aCode.toInt32();
            if ( nVal )
                aFieldSeps += OUString((sal_Unicode) nVal);
        }
    }
    return aFieldSeps;
}




void ScAsciiOptions::ReadFromString( const OUString& rString )
{
    sal_Int32 nCount = comphelper::string::getTokenCount(rString, ',');
    OUString aToken;

    
    if ( nCount >= 1 )
    {
        bFixedLen = bMergeFieldSeps = false;

        aToken = rString.getToken(0,',');
        if ( aToken.equalsAscii(pStrFix) )
            bFixedLen = true;
        aFieldSeps = lcl_decodeSepString( aToken, bMergeFieldSeps);
    }

    
    if ( nCount >= 2 )
    {
        aToken = rString.getToken(1,',');
        sal_Int32 nVal = aToken.toInt32();
        cTextSep = (sal_Unicode) nVal;
    }

    
    if ( nCount >= 3 )
    {
        aToken = rString.getToken(2,',');
        eCharSet = ScGlobal::GetCharsetValue( aToken );
    }

    
    if ( nCount >= 4 )
    {
        aToken = rString.getToken(3,',');
        nStartRow = aToken.toInt32();
    }

    
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

    
    if (nCount >= 6)
    {
        aToken = rString.getToken(5, ',');
        eLang = static_cast<LanguageType>(aToken.toInt32());
    }

    
    if (nCount >= 7)
    {
        aToken = rString.getToken(6, ',');
        bQuotedFieldAsText = aToken.equalsAscii("true") ? true : false;
    }

    
    if (nCount >= 8)
    {
        aToken = rString.getToken(7, ',');
        bDetectSpecialNumber = aToken.equalsAscii("true") ? true : false;
    }
    else
        bDetectSpecialNumber = true;    

    
    
}


OUString ScAsciiOptions::WriteToString() const
{
    OUString aOutStr;

    
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

    
    aOutStr += "," + OUString::number(cTextSep) + ",";

    
    if ( bCharSetSystem )           
        aOutStr += ScGlobal::GetCharsetString( RTL_TEXTENCODING_DONTKNOW );
    else
        aOutStr += ScGlobal::GetCharsetString( eCharSet );

    
    aOutStr += "," + OUString::number(nStartRow) + ",";

    
    OSL_ENSURE( !nInfoCount || (pColStart && pColFormat), "NULL pointer in ScAsciiOptions column info" );
    for (sal_uInt16 nInfo=0; nInfo<nInfoCount; nInfo++)
    {
        if (nInfo)
            aOutStr += "/";
        aOutStr += OUString::number(pColStart[nInfo]) +
                   "/" +
                   OUString::number(pColFormat[nInfo]);
    }

    
    

    aOutStr += "," +
               
               OUString::number(eLang) + "," +
               
               OUString::boolean( bQuotedFieldAsText ) + "," +
               
               OUString::boolean( bDetectSpecialNumber );

    
    

    return aOutStr;
}


sal_Unicode ScAsciiOptions::GetWeightedFieldSep( const OUString & rFieldSeps, bool bDecodeNumbers )
{
    bool bMergeFieldSeps = false;
    OUString aFieldSeps( bDecodeNumbers ? lcl_decodeSepString( rFieldSeps, bMergeFieldSeps) : rFieldSeps);
    if (aFieldSeps.isEmpty())
    {
        SAL_WARN( "sc.ui", "ScAsciiOptions::GetWeightedFieldSep - no separator at all, using ',' comma");
        return ',';
    }
    else if (aFieldSeps.getLength() == 1)
        return aFieldSeps[0];
    else
    {
        
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
