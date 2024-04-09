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
#include <o3tl/string_view.hxx>
#include <sfx2/objsh.hxx>

constexpr std::u16string_view pStrFix = u"FIX";
constexpr std::u16string_view pStrMrg = u"MRG";
constexpr std::u16string_view pStrDet = u"DETECT";

ScAsciiOptions::ScAsciiOptions() :
    bFixedLen       ( false ),
    aFieldSeps      ( OUString(';') ),
    bMergeFieldSeps ( false ),
    bRemoveSpace    ( false ),
    bQuotedFieldAsText(false),
    bDetectSpecialNumber(false),
    bDetectScientificNumber(true),
    bEvaluateFormulas(true),
    bSkipEmptyCells(false),
    bSaveAsShown(true),
    bSaveFormulas(false),
    bIncludeBOM(false),
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

static OUString lcl_decodeSepString( std::u16string_view rSepNums, bool & o_bMergeFieldSeps )
{
    if ( rSepNums.empty() )
        return OUString();

    OUStringBuffer aFieldSeps;
    sal_Int32 nPos = 0;
    do
    {
        const std::u16string_view aCode = o3tl::getToken(rSepNums, 0, '/', nPos );
        if ( aCode == pStrMrg )
            o_bMergeFieldSeps = true;
        else
        {
            sal_Int32 nVal = o3tl::toInt32(aCode);
            if ( nVal )
                aFieldSeps.append(sal_Unicode(nVal));
        }
    }
    while ( nPos >= 0 );

    return aFieldSeps.makeStringAndClear();
}

// The options string must not contain semicolons (because of the pick list),
// use comma as separator.

void ScAsciiOptions::ReadFromString( std::u16string_view rString, SvStream* pStream4Detect )
{
    sal_Int32 nPos = rString.empty() ? -1 : 0;
    bool bDetectSep = false;

    // Token 0: Field separator.
    if ( nPos >= 0 )
    {
        bFixedLen = bMergeFieldSeps = false;

        const std::u16string_view aToken = o3tl::getToken(rString, 0, ',', nPos);
        if ( aToken == pStrDet)
            bDetectSep = true;
        else
        {
            if ( aToken == pStrFix )
                bFixedLen = true;
            aFieldSeps = lcl_decodeSepString( aToken, bMergeFieldSeps);
        }
    }

    // Token 1: Text separator.
    if ( nPos >= 0 )
    {
        const sal_Int32 nVal = o3tl::toInt32(o3tl::getToken(rString, 0, ',', nPos));
        cTextSep = static_cast<sal_Unicode>(nVal);
    }

    // Token 2: Text encoding.
    if ( nPos >= 0 )
    {
        const std::u16string_view aToken = o3tl::getToken(rString, 0, ',', nPos);
        SvStreamEndian endian;
        bool bDetectCharSet = aToken == pStrDet;
        if ( bDetectCharSet && pStream4Detect )
        {
            SfxObjectShell::DetectCharSet(*pStream4Detect, eCharSet, endian);
            if (eCharSet == RTL_TEXTENCODING_UNICODE)
                pStream4Detect->SetEndian(endian);
        }
        else if (!bDetectCharSet)
            eCharSet = ScGlobal::GetCharsetValue( aToken );
    }

    if (bDetectSep && pStream4Detect)
        SfxObjectShell::DetectCsvSeparators(*pStream4Detect, eCharSet, aFieldSeps, cTextSep);

    // Token 3: Number of start row.
    if ( nPos >= 0 )
    {
        nStartRow = o3tl::toInt32(o3tl::getToken(rString, 0, ',', nPos));
    }

    // Token 4: Column info.
    if ( nPos >= 0 )
    {
        const std::u16string_view aToken = o3tl::getToken(rString, 0, ',', nPos);
        const sal_Int32 nInfoCount = comphelper::string::getTokenCount(aToken, '/')/2;
        mvColStart.resize(nInfoCount);
        mvColFormat.resize(nInfoCount);
        sal_Int32 nP = 0;
        for (sal_Int32 nInfo=0; nInfo<nInfoCount; ++nInfo)
        {
            mvColStart[nInfo]  = o3tl::toInt32(o3tl::getToken(aToken, 0, '/', nP));
            mvColFormat[nInfo] = static_cast<sal_uInt8>(o3tl::toInt32(o3tl::getToken(aToken, 0, '/', nP)));
        }
    }

    // Token 5: Language.
    if (nPos >= 0)
    {
        eLang = static_cast<LanguageType>(o3tl::toInt32(o3tl::getToken(rString, 0, ',', nPos)));
    }

    // Token 6: Import quoted field as text.
    if (nPos >= 0)
    {
        bQuotedFieldAsText = o3tl::getToken(rString, 0, ',', nPos) == u"true";
    }

    // Token 7: Detect special numbers.
    if (nPos >= 0)
    {
        bDetectSpecialNumber = o3tl::getToken(rString, 0, ',', nPos) == u"true";
    }
    else
        bDetectSpecialNumber = true;    // default of versions that didn't add the parameter

    // Token 8: used for "Save as shown" in export options
    if ( nPos >= 0 )
    {
        bSaveAsShown = o3tl::getToken(rString, 0, ',', nPos) == u"true";
    }
    else
        bSaveAsShown = true;    // default value

    // Token 9: used for "Save cell formulas" in export options
    if ( nPos >= 0 )
    {
        bSaveFormulas = o3tl::getToken(rString, 0, ',', nPos) == u"true";
    }
    else
        bSaveFormulas = false;

    // Token 10: Boolean for Trim spaces.
    if (nPos >= 0)
    {
        bRemoveSpace = o3tl::getToken(rString, 0, ',', nPos) == u"true";
    }
    else
        bRemoveSpace = false;

    // Token 11: sheet to export for --convert-to csv
    // Does not need to be evaluated here but may be present.
    if (nPos >= 0)
    {
        o3tl::getToken(rString, 0, ',', nPos);
    }

    // Token 12: evaluate formulas.
    if (nPos >= 0)
    {
        // If present, defaults to "false".
        bEvaluateFormulas = o3tl::getToken(rString, 0, ',', nPos) == u"true";
    }
    else
        bEvaluateFormulas = true;   // default of versions that didn't add the parameter

    // Token 13: include BOM.
    bIncludeBOM = nPos >= 0 && o3tl::getToken(rString, 0, ',', nPos) == u"true";

    // Token 14: Detect scientific numbers.
    if (nPos >= 0)
    {
        bDetectScientificNumber = o3tl::getToken(rString, 0, ',', nPos) == u"true";
    }
    else
        bDetectScientificNumber = true;    // default of versions that didn't add the parameter

}

OUString ScAsciiOptions::WriteToString() const
{
    OUStringBuffer aOutStr;

    // Token 0: Field separator.
    if ( bFixedLen )
        aOutStr.append(pStrFix);
    else if ( aFieldSeps.isEmpty() )
        aOutStr.append("0");
    else
    {
        sal_Int32 nLen = aFieldSeps.getLength();
        for (sal_Int32 i=0; i<nLen; i++)
        {
            if (i)
                aOutStr.append("/");
            aOutStr.append(OUString::number(aFieldSeps[i]));
        }
        if ( bMergeFieldSeps )
        {
            aOutStr.append(OUString::Concat("/") + pStrMrg);
        }
    }

    // Token 1: Text Quote character.
    aOutStr.append("," + OUString::number(cTextSep) + ",");

    //Token 2: Text encoding.
    if ( bCharSetSystem )           // force "SYSTEM"
        aOutStr.append(ScGlobal::GetCharsetString( RTL_TEXTENCODING_DONTKNOW ));
    else
        aOutStr.append(ScGlobal::GetCharsetString( eCharSet ));

    //Token 3: Number of start row.
    aOutStr.append("," + OUString::number(nStartRow) + ",");

    //Token 4: Column info.
    for (size_t nInfo=0; nInfo<mvColStart.size(); nInfo++)
    {
        if (nInfo)
            aOutStr.append("/");
        aOutStr.append(OUString::number(mvColStart[nInfo]) +
                "/" +
               OUString::number(mvColFormat[nInfo]));
    }

    // #i112025# the options string is used in macros and linked sheets,
    // so new options must be added at the end, to remain compatible
    // Always keep in sync with ScImportOptions.

    aOutStr.append("," +
               // Token 5: Language
               OUString::number(static_cast<sal_uInt16>(eLang)) + "," +
               // Token 6: Import quoted field as text.
               OUString::boolean( bQuotedFieldAsText ) + "," +
               // Token 7: Detect special numbers.
               OUString::boolean( bDetectSpecialNumber ) + "," +
               // Token 8: used for "Save as shown" in export options
               OUString::boolean( bSaveAsShown ) +"," +
               // Token 9: used for "Save cell formulas" in export options
               OUString::boolean( bSaveFormulas ) + "," +
               // Token 10: Trim Space
               OUString::boolean( bRemoveSpace ) +
               // Token 11: sheet to export, always 0 for current sheet
               ",0," +
               // Token 12: evaluate formulas in import
               OUString::boolean( bEvaluateFormulas ) + "," +
               // Token 13: include BOM
               OUString::boolean(bIncludeBOM) + "," +
               // Token 14: Detect scientific numbers.
               OUString::boolean( bDetectScientificNumber )
            );
    return aOutStr.makeStringAndClear();
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
