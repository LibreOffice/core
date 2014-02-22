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

#include "xistring.hxx"
#include "xlstyle.hxx"
#include "xistream.hxx"
#include "xiroot.hxx"



/** All allowed flags for import. */
const XclStrFlags nAllowedFlags = EXC_STR_8BITLENGTH | EXC_STR_SMARTFLAGS | EXC_STR_SEPARATEFORMATS;



XclImpString::XclImpString()
{
}

XclImpString::XclImpString( const OUString& rString ) :
    maString( rString )
{
}

XclImpString::~XclImpString()
{
}

void XclImpString::Read( XclImpStream& rStrm, XclStrFlags nFlags )
{
    if( !::get_flag( nFlags, EXC_STR_SEPARATEFORMATS ) )
        maFormats.clear();

    SAL_WARN_IF(
        (nFlags & ~nAllowedFlags) != 0, "sc.filter",
        "XclImpString::Read - unknown flag");
    bool b16BitLen = !::get_flag( nFlags, EXC_STR_8BITLENGTH );

    switch( rStrm.GetRoot().GetBiff() )
    {
        case EXC_BIFF2:
        case EXC_BIFF3:
        case EXC_BIFF4:
        case EXC_BIFF5:
            
            maString = rStrm.ReadByteString( b16BitLen );
        break;

        case EXC_BIFF8:
        {
            
            sal_uInt16 nChars = b16BitLen ? rStrm.ReaduInt16() : rStrm.ReaduInt8();
            sal_uInt8 nFlagField = 0;
            if( nChars || !::get_flag( nFlags, EXC_STR_SMARTFLAGS ) )
                rStrm >> nFlagField;

            bool b16Bit, bRich, bFarEast;
            sal_uInt16 nRunCount;
            sal_uInt32 nExtInf;
            rStrm.ReadUniStringExtHeader( b16Bit, bRich, bFarEast, nRunCount, nExtInf, nFlagField );
            

            
            maString = rStrm.ReadRawUniString( nChars, b16Bit );

            
            if( nRunCount > 0 )
                ReadFormats( rStrm, nRunCount );

            
            rStrm.Ignore( nExtInf );
        }
        break;

        default:
            DBG_ERROR_BIFF();
    }
}

void XclImpString::AppendFormat( XclFormatRunVec& rFormats, sal_uInt16 nChar, sal_uInt16 nFontIdx )
{
    
    OSL_ENSURE( rFormats.empty() || (rFormats.back().mnChar <= nChar), "XclImpString::AppendFormat - wrong char order" );
    if( rFormats.empty() || (rFormats.back().mnChar < nChar) )
        rFormats.push_back( XclFormatRun( nChar, nFontIdx ) );
    else
        rFormats.back().mnFontIdx = nFontIdx;
}

void XclImpString::ReadFormats( XclImpStream& rStrm, XclFormatRunVec& rFormats )
{
    bool bBiff8 = rStrm.GetRoot().GetBiff() == EXC_BIFF8;
    sal_uInt16 nRunCount = bBiff8 ? rStrm.ReaduInt16() : rStrm.ReaduInt8();
    ReadFormats( rStrm, rFormats, nRunCount );
}

void XclImpString::ReadFormats( XclImpStream& rStrm, XclFormatRunVec& rFormats, sal_uInt16 nRunCount )
{
    rFormats.clear();
    rFormats.reserve( nRunCount );
    /*  #i33341# real life -- same character index may occur several times
        -> use AppendFormat() to validate formats */
    if( rStrm.GetRoot().GetBiff() == EXC_BIFF8 )
    {
        for( sal_uInt16 nIdx = 0; nIdx < nRunCount; ++nIdx )
        {
            sal_uInt16 nChar, nFontIdx;
            rStrm >> nChar >> nFontIdx;
            AppendFormat( rFormats, nChar, nFontIdx );
        }
    }
    else
    {
        for( sal_uInt16 nIdx = 0; nIdx < nRunCount; ++nIdx )
        {
            sal_uInt8 nChar, nFontIdx;
            rStrm >> nChar >> nFontIdx;
            AppendFormat( rFormats, nChar, nFontIdx );
        }
    }
}

void XclImpString::ReadObjFormats( XclImpStream& rStrm, XclFormatRunVec& rFormats, sal_uInt16 nFormatSize )
{
    
    sal_uInt16 nRunCount = nFormatSize / 8;
    rFormats.clear();
    rFormats.reserve( nRunCount );
    for( sal_uInt16 nIdx = 0; nIdx < nRunCount; ++nIdx )
    {
        sal_uInt16 nChar, nFontIdx;
        rStrm >> nChar >> nFontIdx;
        rStrm.Ignore( 4 );
        AppendFormat( rFormats, nChar, nFontIdx );
    }
}



XclImpStringIterator::XclImpStringIterator( const XclImpString& rString ) :
    mrText( rString.GetText() ),
    mrFormats( rString.GetFormats() ),
    mnPortion( 0 ),
    mnTextBeg( 0 ),
    mnTextEnd( 0 ),
    mnFormatsBeg( 0 ),
    mnFormatsEnd( 0 )
{
    
    if( !mrFormats.empty() && (mrFormats.front().mnChar == 0) )
        ++mnFormatsEnd;
    
    mnTextEnd = (mnFormatsEnd < mrFormats.size() ?
        mrFormats[ mnFormatsEnd ].mnChar : mrText.getLength() );
}

OUString XclImpStringIterator::GetPortionText() const
{
    return mrText.copy( mnTextBeg, mnTextEnd - mnTextBeg );
}

sal_uInt16 XclImpStringIterator::GetPortionFont() const
{
    return (mnFormatsBeg < mnFormatsEnd) ? mrFormats[ mnFormatsBeg ].mnFontIdx : EXC_FONT_NOTFOUND;
}

XclImpStringIterator& XclImpStringIterator::operator++()
{
    if( Is() )
    {
        ++mnPortion;
        do
        {
            
            if( mnFormatsBeg < mnFormatsEnd )
                ++mnFormatsBeg;
            if( mnFormatsEnd < mrFormats.size() )
                ++mnFormatsEnd;
            
            mnTextBeg = mnTextEnd;
            mnTextEnd = (mnFormatsEnd < mrFormats.size()) ?
                mrFormats[ mnFormatsEnd ].mnChar : mrText.getLength();
        }
        while( Is() && (mnTextBeg == mnTextEnd) );
    }
    return *this;
}



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
