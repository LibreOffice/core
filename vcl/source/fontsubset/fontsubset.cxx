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


#include <tools/debug.hxx>

#include <fontsubset.hxx>
#include <sft.hxx>



FontSubsetInfo::FontSubsetInfo()
:   m_nAscent( 0)
,   m_nDescent( 0)
,   m_nCapHeight( 0)
,   m_nFontType( FontSubsetInfo::NO_FONT)
,   mpInFontBytes( NULL)
,   mnInByteLength( 0)
,   meInFontType( FontSubsetInfo::NO_FONT)
,   mpSftTTFont( NULL)
{}



FontSubsetInfo::~FontSubsetInfo()
{}




bool FontSubsetInfo::LoadFont(
    FontSubsetInfo::FontType eInFontType,
    const unsigned char* pInFontBytes, int nInByteLength)
{
    DBG_ASSERT( (mpSftTTFont == NULL), "Subset from SFT and from mapped font-file requested");
    meInFontType = eInFontType;
    mpInFontBytes = pInFontBytes;
    mnInByteLength = nInByteLength;
    return (mnInByteLength > 0);
}




bool FontSubsetInfo::LoadFont( vcl::_TrueTypeFont* pSftTTFont )
{
    DBG_ASSERT( (mpInFontBytes == NULL), "Subset from SFT and from mapped font-file requested");
    mpSftTTFont = pSftTTFont;
    meInFontType = ANY_SFNT;
    return (mpSftTTFont == NULL);
}



bool FontSubsetInfo::CreateFontSubset(
    int nReqFontTypeMask,
    FILE* pOutFile, const char* pReqFontName,
    const sal_GlyphId* pReqGlyphIds, const sal_uInt8* pReqEncodedIds, int nReqGlyphCount,
    sal_Int32* pOutGlyphWidths)
{
    
    mnReqFontTypeMask = nReqFontTypeMask;
    mpOutFile       = pOutFile;
    mpReqFontName   = pReqFontName;
    mpReqGlyphIds   = pReqGlyphIds;
    mpReqEncodedIds = pReqEncodedIds;
    mnReqGlyphCount = nReqGlyphCount;

    

    
    bool bOK = false;

    
    switch( meInFontType) {
    case SFNT_TTF:
    case SFNT_CFF:
    case ANY_SFNT:
        bOK = CreateFontSubsetFromSfnt( pOutGlyphWidths);
        break;
    case CFF_FONT:
        bOK = CreateFontSubsetFromCff( pOutGlyphWidths);
        break;
    case TYPE1_PFA:
    case TYPE1_PFB:
    case ANY_TYPE1:
        bOK = CreateFontSubsetFromType1( pOutGlyphWidths);
        break;
        
    case NO_FONT:
        
    default:
        OSL_FAIL( "unhandled type in CreateFontSubset()");
        break;
    }

    return bOK;
}




bool FontSubsetInfo::CreateFontSubsetFromSfnt( sal_Int32* pOutGlyphWidths )
{
    
    int nCffLength = 0;
    const sal_uInt8* pCffBytes = NULL;
    if( GetSfntTable( mpSftTTFont, O_CFF, &pCffBytes, &nCffLength))
    {
        LoadFont( CFF_FONT, pCffBytes, nCffLength);
        const bool bOK = CreateFontSubsetFromCff( pOutGlyphWidths);
        return bOK;
    }

    
    
#if 1 
    sal_uInt16 aShortGlyphIds[256];
    for( int i = 0; i < mnReqGlyphCount; ++i)
        aShortGlyphIds[i] = (sal_uInt16)mpReqGlyphIds[i];
    
    sal_uInt8* pEncArray = const_cast<sal_uInt8*>( mpReqEncodedIds );
#endif
    int nSFTErr = vcl::SF_BADARG;
    if( (mnReqFontTypeMask & TYPE42_FONT) != 0 )
    {
        nSFTErr = CreateT42FromTTGlyphs( mpSftTTFont, mpOutFile, mpReqFontName,
            aShortGlyphIds, pEncArray, mnReqGlyphCount );
    }
    else if( (mnReqFontTypeMask & TYPE3_FONT) != 0 )
    {
        nSFTErr = CreateT3FromTTGlyphs( mpSftTTFont, mpOutFile, mpReqFontName,
            aShortGlyphIds, pEncArray, mnReqGlyphCount,
                    0 /* 0 = horizontal, 1 = vertical */ );
    }
    else if( (mnReqFontTypeMask & SFNT_TTF) != 0 )
    {
        
        
    }

    return (nSFTErr != vcl::SF_OK);
}




bool FontSubsetInfo::CreateFontSubsetFromType1( sal_Int32* pOutGlyphWidths)
{
    (void)pOutGlyphWidths;
    fprintf(stderr,"CreateFontSubsetFromType1: replace dummy implementation\n");
    return false;
}



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
