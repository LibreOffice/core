/*************************************************************************
 *
 *  $RCSfile: gcach_ftyp.cxx,v $
 *
 *  $Revision: 1.10 $
 *  last change: $Author: vg $ $Date: 2001-02-26 16:08:40 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include <gcach_ftyp.hxx>

#include <svapp.hxx>
#include <outfont.hxx>
#include <bitmap.hxx>
#include <bmpacc.hxx>

#include <rtl/ustring>      // used only for string=>hashvalue
#include <osl/file.hxx>
#include <poly.hxx>

#ifndef NO_FREETYPE_FONTS

#include "freetype/freetype.h"
#include "freetype/ftglyph.h"
#include "freetype/ftoutln.h"
#include "freetype/tttables.h"
#include "freetype/tttags.h"
#include "freetype/ttnameid.h"

#include <vector>

// -----------------------------------------------------------------------

static FT_Library aLibFT = 0;

// =======================================================================
// FreetypeManager
// =======================================================================

#include "freetype/internal/ftobjs.h"
#include "freetype/internal/sfnt.h"
#include "freetype/internal/ftstream.h"

// =======================================================================

size_t std::hash<FtFontInfo*>::operator()( const FtFontInfo* pFI ) const
{
    size_t nHash = ::rtl::OUString( pFI->aFontData.maName ).hashCode();
//###   nHash += ::rtl::OUString( pFI->aFontData.maStyleName ).hashCode();
    return nHash;
}

// -----------------------------------------------------------------------

bool std::equal_to<FtFontInfo*>::operator()( const FtFontInfo* pA, const FtFontInfo* pB ) const
{
    if( (pA->aFontData.maName == pB->aFontData.maName)  )
        return true;
    return false;
}

// -----------------------------------------------------------------------

inline int CalcSimilarity( const FtFontInfo* pA, const FtFontInfo* pB )
{
    int nSimilarity = 0;
    nSimilarity +=  3*(pA->aFontData.meItalic   == pB->aFontData.meItalic);
    nSimilarity +=  2*(pA->aFontData.meWeight   == pB->aFontData.meWeight);
    return nSimilarity;
}

// -----------------------------------------------------------------------

FreetypeManager::FreetypeManager()
{
    FT_Error rcFT = FT_Init_FreeType( &aLibFT );
}

// -----------------------------------------------------------------------

FreetypeManager::~FreetypeManager()
{
    FT_Error rcFT = FT_Done_FreeType( aLibFT );
}

// -----------------------------------------------------------------------

void FreetypeManager::AddFontFile( const String& rNormalizedName,
    int nFaceNum, int nFontId, const ImplFontData* pData )
{
    if( !rNormalizedName.Len() )
        return;

    FtFontInfo* const pFontInfo = new FtFontInfo;

    pFontInfo->aNativeFileName  = ::rtl::OString( rNormalizedName.GetBuffer(),
        rNormalizedName.Len(), RTL_TEXTENCODING_UNICODE );
    pFontInfo->nFaceNum = nFaceNum;
    pFontInfo->nFontId  = nFontId;

    ImplFontData& rData = pFontInfo->aFontData;
    rData               = *pData;
    rData.mpSysData     = SERVERFONT_MAGIC;
    rData.mpNext        = NULL;

    maFontList.insert( pFontInfo );
}

// -----------------------------------------------------------------------

long FreetypeManager::AddFontDir( const String& rNormalizedName )
{
    osl::Directory aDir( rNormalizedName );
    osl::FileBase::RC rcOSL = aDir.open();
    if( rcOSL != osl::FileBase::E_None )
        return 0;

    long nCount = 0;

    osl::DirectoryItem aDirItem;
    while( (rcOSL = aDir.getNextItem( aDirItem, 20 )) == osl::FileBase::E_None )
    {
        osl::FileStatus aFileStatus( FileStatusMask_NativePath );

        rcOSL = aDirItem.getFileStatus( aFileStatus );
        ::rtl::OUString aUFileName = aFileStatus.getNativePath();
        ::rtl::OString aCFileName = rtl::OUStringToOString( aUFileName, RTL_TEXTENCODING_DONTKNOW );
        const char* pszFontFileName = aCFileName.getStr();

        FT_FaceRec_* aFaceFT = NULL;
        for( int nFaceNum = 0, nMaxFaces = 1; nFaceNum < nMaxFaces; ++nFaceNum )
        {
            FT_Error rcFT = FT_New_Face( aLibFT, pszFontFileName, nFaceNum, &aFaceFT );
            if( (rcFT != FT_Err_Ok) || (aFaceFT == NULL) )
                break;

            if( !FT_IS_SCALABLE( aFaceFT ) )    // ignore non-TT fonts for now
                continue;

            nMaxFaces = aFaceFT->num_faces;

            FtFontInfo* const pFontInfo = new FtFontInfo;
            pFontInfo->aNativeFileName  = aCFileName;
            pFontInfo->nFaceNum         = nFaceNum;

            ImplFontData& rData = pFontInfo->aFontData;

            rData.mpNext        = NULL;
            rData.mpSysData     = SERVERFONT_MAGIC;

            // TODO: prefer unicode names if available
            // TODO: prefer locale specific names if available?
            rData.maName        = String::CreateFromAscii( aFaceFT->family_name );
            rData.maStyleName   = String::CreateFromAscii( aFaceFT->style_name );

            rData.mnWidth       = 0;
            rData.mnHeight      = 0;

            // TODO: extract better font characterization data from font
            rData.meFamily      = FAMILY_DONTKNOW;

            rData.meCharSet = RTL_TEXTENCODING_UNICODE;
            for( int i = aFaceFT->num_charmaps; --i >= 0; )
            {
                const FT_CharMap aCM = aFaceFT->charmaps[i];
                if( (aCM->platform_id == TT_PLATFORM_MICROSOFT)
                && (aCM->encoding_id == TT_MS_ID_SYMBOL_CS) )
                    rData.meCharSet = RTL_TEXTENCODING_SYMBOL;
            }
            rData.meScript      = SCRIPT_DONTKNOW;

            rData.mePitch       = FT_IS_FIXED_WIDTH( aFaceFT ) ? PITCH_FIXED : PITCH_VARIABLE;
            rData.meWidthType   = WIDTH_DONTKNOW;
            rData.meWeight      = (aFaceFT->style_flags & FT_STYLE_FLAG_BOLD) ? WEIGHT_BOLD : WEIGHT_NORMAL;
            rData.meItalic      = (aFaceFT->style_flags & FT_STYLE_FLAG_ITALIC) ? ITALIC_NORMAL : ITALIC_NONE;

            rData.mnVerticalOrientation = 0;
            rData.mbOrientation = true;
            rData.mbDevice      = false;
            rData.mnQuality     = 0;        // prefer client-side fonts if available

            FT_Done_Face( aFaceFT );

            maFontList.insert( pFontInfo );
            ++nCount;
        }
    }

    aDir.close();
    return nCount;
}

// -----------------------------------------------------------------------

long FreetypeManager::FetchFontList( ImplDevFontList* pToAdd ) const
{
    long nCount = 0;
    for( FontList::const_iterator it(maFontList.begin()); it != maFontList.end(); ++it, ++nCount )
        pToAdd->Add( new ImplFontData( (*it)->aFontData ) );
    return nCount;
}

// -----------------------------------------------------------------------

void FreetypeManager::ClearFontList( )
{
    for( FontList::iterator it(maFontList.begin()); it != maFontList.end(); ++it )
        delete *it;
    maFontList.clear();
}

// -----------------------------------------------------------------------

FreetypeServerFont* FreetypeManager::CreateFont( const ImplFontSelectData& rFSD )
{
    FtFontInfo aFontInfo;
    aFontInfo.aFontData.maStyleName = rFSD.maStyleName;
    aFontInfo.aFontData.meWeight    = rFSD.meWeight;
    aFontInfo.aFontData.meItalic    = rFSD.meItalic;

    for( xub_StrLen nBreaker1 = 0, nBreaker2 = 0; nBreaker2 != STRING_LEN; nBreaker1 = nBreaker2 + 1 )
    {
        nBreaker2 = rFSD.maName.Search( ';', nBreaker1 );
        if( nBreaker2 == STRING_NOTFOUND )
            nBreaker2 = STRING_LEN;
        aFontInfo.aFontData.maName = rFSD.maName.Copy( nBreaker1, nBreaker2-nBreaker1 );

        // find best match (e.g. only regular available but bold requested)
        typedef std::pair<FontList::const_iterator,FontList::const_iterator> CPair;

        const CPair aRange = maFontList.equal_range( &aFontInfo );
        int nBestMatch = 0;
        FontList::const_iterator it_best = aRange.first;
        for( FontList::const_iterator it = aRange.first; it != aRange.second; ++it )
        {
            const int nMatchVal = CalcSimilarity( (*it), &aFontInfo );
            if( nBestMatch < nMatchVal )
            {
                nBestMatch = nMatchVal;
                it_best = it;
            }
        }
        if( it_best != aRange.second )
        {
            FreetypeServerFont* pFont = new FreetypeServerFont( rFSD, **it_best );
            return pFont;
        }
    }

    return NULL;
}

// =======================================================================
// FreetypeServerFont
// =======================================================================

FreetypeServerFont::FreetypeServerFont( const ImplFontSelectData& rFSD, const FtFontInfo& rFI )
:   ServerFont(rFSD),
    mrFontInfo(const_cast<FtFontInfo&>(rFI)),
    maFaceFT(NULL)
{
    const char* pszFontFileName = rFI.aNativeFileName.getStr();
    FT_Error rc = FT_New_Face( aLibFT, pszFontFileName, rFI.nFaceNum, &maFaceFT );
    if( rc != FT_Err_Ok )
        return;

    FT_Encoding eEncoding = ft_encoding_unicode;
    if( mrFontInfo.aFontData.meCharSet == RTL_TEXTENCODING_SYMBOL )
        // TODO: for FT>=2.0 use "eEncoding = ft_encoding_symbol";
        eEncoding = ft_encoding_none;
    rc = FT_Select_Charmap( maFaceFT, eEncoding );

    mnWidth = rFSD.mnWidth;
    if( !mnWidth )
        mnWidth = rFSD.mnHeight;
    rc = FT_Set_Pixel_Sizes( maFaceFT, rFSD.mnHeight, mnWidth );

    //TODO: LanguageType aLanguage = GetLanguage();
    //TODO: GSUB glyph substitution
    //TODO: GPOS

    // TODO: query GASP table for load flags
    mnLoadFlags = FT_LOAD_DEFAULT;

    if( rFSD.mnOrientation != 0 )
    {
        mnLoadFlags |= FT_LOAD_NO_HINTING;

        FT_Matrix aMatrix;
        aMatrix.xx = +nCos;
        aMatrix.yy = +nCos;
        aMatrix.xy = -nSin;
        aMatrix.yx = +nSin;
        FT_Set_Transform( maFaceFT, &aMatrix, 0 );
    }

//  if( rFSD.mbVertical )
//      mnLoadFlags |= FT_LOAD_VERTICAL_LAYOUT;
}

// -----------------------------------------------------------------------

FreetypeServerFont::~FreetypeServerFont()
{
    if( maFaceFT )
        FT_Done_Face( maFaceFT );
}

// -----------------------------------------------------------------------

void FreetypeServerFont::FetchFontMetric( ImplFontMetricData& rTo, long& rFactor ) const
{
    rFactor = 0x100;

    rTo.mnWidth             = mnWidth;

    const FT_Size_Metrics& rMetrics = maFaceFT->size->metrics;
    rTo.mnAscent            = (+rMetrics.ascender + 32) >> 6;
    // TODO: change +desc to -desc for FT_Version>2.0beta8
    rTo.mnDescent           = (+rMetrics.descender + 32) >> 6;
    rTo.mnLeading           = (rMetrics.height - rMetrics.ascender - rMetrics.descender + 32) >> 6;
    rTo.mnSlant             = 0;

    rTo.maName              = mrFontInfo.aFontData.maName;
    rTo.maStyleName         = mrFontInfo.aFontData.maStyleName;

    rTo.mnFirstChar         = 0x0020;
    rTo.mnLastChar          = 0xFFFE;

    rTo.mnOrientation       = GetFontSelData().mnOrientation;

    rTo.meCharSet           = mrFontInfo.aFontData.meCharSet;
    rTo.meFamily            = mrFontInfo.aFontData.meFamily;
    rTo.meWeight            = mrFontInfo.aFontData.meWeight;
    rTo.meItalic            = mrFontInfo.aFontData.meItalic;
    rTo.mePitch             = mrFontInfo.aFontData.mePitch;

    rTo.meType              = TYPE_SCALABLE;
    rTo.mbDevice            = FALSE;

    rTo.mnStrikeoutSize     = rTo.mnAscent / 6;
    rTo.mnStrikeoutOffset   = +rTo.mnAscent / 3;

    const TT_OS2* pOS2 = (const TT_OS2*)FT_Get_Sfnt_Table( maFaceFT, ft_sfnt_os2 );
    if( pOS2 && (~pOS2->version != 0) )
    {
        const double fScale = (double)GetFontSelData().mnHeight / maFaceFT->units_per_EM;
        rTo.mnWidth         = (long)( pOS2->xAvgCharWidth * fScale + 0.5 );
        rTo.mnAscent        = (long)( +pOS2->usWinAscent * fScale + 0.5 );
        rTo.mnDescent       = (long)( +pOS2->usWinDescent * fScale + 0.5 );
        rTo.mnLeading       = (long)( (+pOS2->usWinAscent - pOS2->usWinDescent - maFaceFT->units_per_EM) * fScale + 0.5 );

        rTo.mnFirstChar     = pOS2->usFirstCharIndex;
        rTo.mnLastChar      = pOS2->usLastCharIndex;

        rTo.mnStrikeoutSize     = Min( 1L, (long)( pOS2->yStrikeoutSize * fScale + 0.5 ) );
        rTo.mnStrikeoutOffset   = (long)( pOS2->yStrikeoutPosition * fScale + 0.5 );
    }
}

// -----------------------------------------------------------------------

int FreetypeServerFont::GetGlyphIndex( sal_Unicode aChar ) const
{
    if( mrFontInfo.aFontData.meCharSet == RTL_TEXTENCODING_SYMBOL )
        aChar |= 0xF000;    // emulate W2K hig/low mapping of symbols

    int nGlyphIndex = FT_Get_Char_Index( maFaceFT, aChar );

    // do glyph substitution if necessary
    GlyphSubstitution::const_iterator it = aGlyphSubstitution.find( nGlyphIndex );
    if( it != aGlyphSubstitution.end() )
        nGlyphIndex = (*it).first;

    return nGlyphIndex;
}

// -----------------------------------------------------------------------

void FreetypeServerFont::InitGlyphData( int nGlyphIndex, GlyphData& rGD ) const
{
    FT_Error rc = FT_Load_Glyph( maFaceFT, nGlyphIndex, mnLoadFlags );

#if 0
    if( GetFontSelData().mbVertical && FT_HAS_VERTICAL(maFaceFT) )
        rGD.SetCharWidth( (maFaceFT->glyph->metrics.vertAdvance + 32) >> 6 );
    else
#endif
        rGD.SetCharWidth( (maFaceFT->glyph->metrics.horiAdvance + 32) >> 6 );

    FT_Glyph aGlyphFT;
    rc = FT_Get_Glyph( maFaceFT->glyph, &aGlyphFT );

    rGD.SetDelta( (aGlyphFT->advance.x + 0x8000) >> 16, -((aGlyphFT->advance.y + 0x8000) >> 16) );

    FT_BBox aBbox;
    FT_Glyph_Get_CBox( aGlyphFT, ft_glyph_bbox_pixels, &aBbox );
    if( aBbox.yMin > aBbox.yMax )   // circumvent freetype bug
    {
        int t; t = aBbox.yMin, aBbox.yMin=aBbox.yMax, aBbox.yMax=t;
    }
    rGD.SetOffset( aBbox.xMin, -aBbox.yMax );
    rGD.SetSize( Size( (aBbox.xMax-aBbox.xMin+1), (aBbox.yMax-aBbox.yMin) ) );

    FT_Done_Glyph( aGlyphFT );
}

// -----------------------------------------------------------------------

bool FreetypeServerFont::GetGlyphBitmap1( int nGlyphIndex, RawBitmap& rRawBitmap ) const
{
    FT_Error rc = FT_Load_Glyph( maFaceFT, nGlyphIndex, mnLoadFlags );
    if( rc != FT_Err_Ok )
        return false;

    // TODO: if( GetFontSelData().mbVertical ) ...

    FT_Glyph aGlyphFT;
    rc = FT_Get_Glyph( maFaceFT->glyph, &aGlyphFT );
    if( rc != FT_Err_Ok )
        return false;

    if( aGlyphFT->format != ft_glyph_format_bitmap )
    {
        if( aGlyphFT->format == ft_glyph_format_outline )
            ((FT_OutlineGlyphRec*)aGlyphFT )->outline.flags |= ft_outline_high_precision;
        rc = FT_Glyph_To_Bitmap( &aGlyphFT, ft_render_mode_mono, NULL, TRUE );
        if( rc != FT_Err_Ok )
            return false;
    }

    const FT_BitmapGlyph& rBmpGlyphFT = reinterpret_cast<const FT_BitmapGlyph&>(aGlyphFT);
    rRawBitmap.mnXOffset        = +rBmpGlyphFT->left;
    rRawBitmap.mnYOffset        = -rBmpGlyphFT->top;

    const FT_Bitmap& rBitmapFT  = rBmpGlyphFT->bitmap;
    rRawBitmap.mnHeight         = rBitmapFT.rows;
    rRawBitmap.mnWidth          = rBitmapFT.width;
    rRawBitmap.mnScanlineSize   = rBitmapFT.pitch;
    rRawBitmap.mnBitCount       = 1;

    const ULONG nNeededSize = rRawBitmap.mnScanlineSize * rRawBitmap.mnHeight;
    if( rRawBitmap.mnAllocated < nNeededSize )
    {
        delete[] rRawBitmap.mpBits;
        rRawBitmap.mnAllocated = 2*nNeededSize;
        rRawBitmap.mpBits = new unsigned char[ rRawBitmap.mnAllocated ];
    }

    const unsigned char* pSrc = rBitmapFT.buffer;
    unsigned char* pDest = rRawBitmap.mpBits;
     for( int i = nNeededSize; --i >= 0; )
         *(pDest++) = ~*(pSrc++);

    FT_Done_Glyph( aGlyphFT );
    return true;
}

// -----------------------------------------------------------------------

bool FreetypeServerFont::GetGlyphBitmap8( int nGlyphIndex, RawBitmap& rRawBitmap ) const
{
    FT_Int nLoadFlags = mnLoadFlags | FT_LOAD_NO_BITMAP;
    FT_Error rc = FT_Load_Glyph( maFaceFT, nGlyphIndex, nLoadFlags );
    if( rc != FT_Err_Ok )
        return false;

    // TODO: if( GetFontSelData().mbVertical ) ...

    FT_Glyph aGlyphFT;
    rc = FT_Get_Glyph( maFaceFT->glyph, &aGlyphFT );
    if( rc != FT_Err_Ok )
        return false;

    if( aGlyphFT->format == ft_glyph_format_outline )
        ((FT_OutlineGlyphRec*)aGlyphFT )->outline.flags |= ft_outline_high_precision;
    rc = FT_Glyph_To_Bitmap( &aGlyphFT, ft_render_mode_normal, NULL, TRUE );
    if( rc != FT_Err_Ok )
        return false;

    const FT_BitmapGlyph& rBmpGlyphFT = reinterpret_cast<const FT_BitmapGlyph&>(aGlyphFT);
    rRawBitmap.mnXOffset        = +rBmpGlyphFT->left;
    rRawBitmap.mnYOffset        = -rBmpGlyphFT->top;

    const FT_Bitmap& rBitmapFT  = rBmpGlyphFT->bitmap;
    rRawBitmap.mnHeight         = rBitmapFT.rows;
    rRawBitmap.mnWidth          = rBitmapFT.width;
    rRawBitmap.mnScanlineSize   = (rBitmapFT.pitch + 3) & -4;
    rRawBitmap.mnBitCount       = 8;

    const ULONG nNeededSize = rRawBitmap.mnScanlineSize * rRawBitmap.mnHeight;
    if( rRawBitmap.mnAllocated < nNeededSize )
    {
        delete[] rRawBitmap.mpBits;
        rRawBitmap.mnAllocated = 2*nNeededSize;
        rRawBitmap.mpBits = new unsigned char[ rRawBitmap.mnAllocated ];
    }

    const unsigned char* pSrc = rBitmapFT.buffer;
    unsigned char* pDest = rRawBitmap.mpBits;
    for( int y = rRawBitmap.mnHeight, x; --y >= 0 ; )
    {
        for( x = 0; x < rBitmapFT.width; ++x )
            *(pDest++) = *(pSrc++);
        for(; x < rRawBitmap.mnScanlineSize; ++x )
            *(pDest++) = 0;
    }

    FT_Done_Glyph( aGlyphFT );
    return true;
}

// -----------------------------------------------------------------------
// kerning stuff
// -----------------------------------------------------------------------

ULONG FreetypeServerFont::GetKernPairs( ImplKernPairData** ppKernPairs ) const
{
    *ppKernPairs = NULL;
    if( !FT_HAS_KERNING( maFaceFT ) )
        return 0;

    // first figure out which glyph pairs are involved in kerning
    SFNT_Interface* pSFNT = (SFNT_Interface*) FT_Get_Module_Interface( aLibFT, "sfnt" );
    DBG_ASSERT( (pSFNT!=NULL), "pSFNT==NULL!" );
    if( !pSFNT )
        return 0;

    FT_ULong nKernLength = 0;
    FT_Error rcFT = pSFNT->load_any( (TT_Face)maFaceFT, TTAG_kern, 0, NULL, &nKernLength );
    if( rcFT != FT_Err_Ok )
        return 0;

    FT_Byte* const pKern = new FT_Byte[ nKernLength ];
    rcFT = pSFNT->load_any( (TT_Face)maFaceFT, TTAG_kern, 0, pKern, &nKernLength );

    typedef std::vector<ImplKernPairData> KernVector;
    KernVector aKernGlyphVector;
    ImplKernPairData aKernPair;

    const FT_Byte* pBuffer = pKern;
    USHORT nVersion = NEXT_UShort( pBuffer );
    USHORT nTableCnt = NEXT_UShort( pBuffer );
    if( nVersion != 0 )     // ignore Apple's versions for now
        nTableCnt = 0;
    for( USHORT nTableIdx = 0; nTableIdx < nTableCnt; ++nTableIdx )
    {
        USHORT nSubVersion  = NEXT_UShort( pBuffer );
        USHORT nSubLength   = NEXT_UShort( pBuffer );
        USHORT nSubCoverage = NEXT_UShort( pBuffer );
        if( (nSubCoverage&0x03) != 0x01 )   // no interest in minimum info here
            continue;
        switch( nSubCoverage >> 8 )
        {
            case 0: // version 0, kerning format 0
            {
                USHORT nPairs = NEXT_UShort( pBuffer );
                pBuffer += 6;   // skip search hints
                aKernGlyphVector.reserve( aKernGlyphVector.size() + nPairs );
                for( int i = 0; i < nPairs; ++i )
                {
                    aKernPair.mnChar1   = NEXT_UShort( pBuffer );
                    aKernPair.mnChar2   = NEXT_UShort( pBuffer );
                    /*long nUnscaledKern=*/ NEXT_Short( pBuffer );
                    aKernGlyphVector.push_back( aKernPair );
                }
            }
            break;

            case 2: // version 0, kerning format 2
            {
                const FT_Byte* pSubTable = pBuffer;
                /*USHORT nRowWidth  =*/ NEXT_UShort( pBuffer );
                USHORT nOfsLeft     = NEXT_UShort( pBuffer );
                USHORT nOfsRight    = NEXT_UShort( pBuffer );
                USHORT nOfsArray    = NEXT_UShort( pBuffer );

                const FT_Byte* pTmp = pSubTable + nOfsLeft;
                USHORT nFirstLeft   = NEXT_UShort( pTmp );
                USHORT nLastLeft    = NEXT_UShort( pTmp ) + nFirstLeft - 1;

                pTmp = pSubTable + nOfsRight;
                USHORT nFirstRight  = NEXT_UShort( pTmp );
                USHORT nLastRight   = NEXT_UShort( pTmp ) + nFirstRight - 1;

                ULONG nPairs = (ULONG)(nLastLeft - nFirstLeft + 1) * (nLastRight - nFirstRight + 1);
                aKernGlyphVector.reserve( aKernGlyphVector.size() + nPairs );

                pTmp = pSubTable + nOfsArray;
                for( int nLeft = nFirstLeft; nLeft < nLastLeft; ++nLeft )
                {
                    aKernPair.mnChar1 = nLeft;
                    for( int nRight = 0; nRight < nLastRight; ++nRight )
                    {
                        if( NEXT_Short( pTmp ) != 0 )
                        {
                            aKernPair.mnChar2 = nRight;
                            aKernGlyphVector.push_back( aKernPair );
                        }
                    }
                }
            }
            break;
        }
    }

    delete[] pKern;

    // now create VCL's ImplKernPairData[] format for all glyph pairs
    ULONG nKernCount = aKernGlyphVector.size();
    if( nKernCount )
    {
        // prepare glyphindex to character mapping
        // TODO: this is needed to support VCL's existing kerning infrastructure,
        // eliminate it up by redesigning kerning infrastructure to work with glyph indizes
        typedef std::hash_multimap<USHORT,sal_Unicode> Cmap;
        Cmap aCmap;
        for( sal_Unicode aChar = 0x0001; aChar < 0xFFFE; ++aChar )
        {
            USHORT nGlyphIndex = GetGlyphIndex( aChar );
            if( nGlyphIndex )
                aCmap.insert( Cmap::value_type( nGlyphIndex, aChar ) );
        }

        // translate both glyph indizes in kerning pairs to characters
        // problem is that these are 1:n mappings...
        KernVector aKernCharVector;
        aKernCharVector.reserve( nKernCount );
        KernVector::iterator it;
        for( it = aKernGlyphVector.begin(); it != aKernGlyphVector.end(); ++it )
        {
            FT_Vector aKernVal;
            FT_Error rcFT = FT_Get_Kerning( maFaceFT, it->mnChar1, it->mnChar2,
                ft_kerning_default, &aKernVal );
            aKernPair.mnKern = aKernVal.x;
            if( (aKernPair.mnKern == 0) || (rcFT != FT_Err_Ok) )

                continue;

            typedef std::pair<Cmap::iterator,Cmap::iterator> CPair;
            const CPair p1 = aCmap.equal_range( it->mnChar1 );
            const CPair p2 = aCmap.equal_range( it->mnChar2 );
            for( Cmap::const_iterator i1 = p1.first; i1 != p1.second; ++i1 )
            {
                aKernPair.mnChar1 = (*i1).second;
                for( Cmap::const_iterator i2 = p2.first; i2 != p2.second; ++i2 )
                {
                    aKernPair.mnChar2 = (*i2).second;
                    aKernCharVector.push_back( aKernPair );
                }
            }
        }

        // now move the resulting vector into VCL's ImplKernPairData[] format
        nKernCount = aKernCharVector.size();
        ImplKernPairData* pTo = new ImplKernPairData[ nKernCount ];
        *ppKernPairs = pTo;
        for( it = aKernCharVector.begin(); it != aKernCharVector.end(); ++it, ++pTo )
        {
            pTo->mnChar1 = it->mnChar1;
            pTo->mnChar2 = it->mnChar2;
            pTo->mnKern = it->mnKern;
        }
    }

    return nKernCount;
}

// -----------------------------------------------------------------------
// outline stuff
// -----------------------------------------------------------------------

class PolyArgs
{
public:
                PolyArgs( PolyPolygon& rPolyPoly, USHORT nMaxPoints, long nHeight );
                ~PolyArgs();

    void        AddPoint( long nX, long nY, PolyFlags);
    void        ClosePolygon();

    long        GetPosX() const { return maPosition.x;}
    long        GetPosY() const { return maPosition.y;}

private:
    PolyPolygon& mrPolyPoly;

    Point*      mpPointAry;
    BYTE*       mpFlagAry;

    FT_Vector   maPosition;
    USHORT      mnMaxPoints;
    USHORT      mnPoints;
    USHORT      mnPoly;
    long        mnHeight;
    bool        bHasOffline;
};

// -----------------------------------------------------------------------

PolyArgs::PolyArgs( PolyPolygon& rPolyPoly, USHORT nMaxPoints, long nHeight )
:   mrPolyPoly(rPolyPoly),
    mnMaxPoints(nMaxPoints),
    mnPoints(0),
    mnPoly(0),
    mnHeight(nHeight),
    bHasOffline(false)
{
    mpPointAry  = new Point [ mnMaxPoints ];
    mpFlagAry   = new BYTE  [ mnMaxPoints ];

    mrPolyPoly.Clear();
}

// -----------------------------------------------------------------------

PolyArgs::~PolyArgs()
{
    delete[] mpFlagAry;
    delete[] mpPointAry;
}

// -----------------------------------------------------------------------

void PolyArgs::AddPoint( long nX, long nY, PolyFlags aFlag )
{
    DBG_ASSERT( (mnPoints < mnMaxPoints), "FTGlyphOutline: AddPoint overflow!" );

    maPosition.x = nX;
    maPosition.y = nY;
    mpPointAry[ mnPoints ] = Point( (nX + 32) >> 6, mnHeight - ((nY + 32) >> 6) );
    mpFlagAry[ mnPoints++ ]= aFlag;
    bHasOffline |= (aFlag != POLY_NORMAL);
}

// -----------------------------------------------------------------------

void PolyArgs::ClosePolygon()
{
    if( !mnPoly++ )
        return;

    // freetype seems to always close the polygon with an ON_CURVE point
    // PolyPoly wants to close the polygon itself => remove last point
    DBG_ASSERT( (mnPoints >= 2), "FTGlyphOutline: PolyFinishNum failed!" );
    --mnPoints;
    DBG_ASSERT( (mpPointAry[0]==mpPointAry[mnPoints]), "FTGlyphOutline: PolyFinishEq failed!" );
    DBG_ASSERT( (mpFlagAry[0]==POLY_NORMAL), "FTGlyphOutline: PolyFinishFE failed!" );
    DBG_ASSERT( (mpFlagAry[mnPoints]==POLY_NORMAL), "FTGlyphOutline: PolyFinishFS failed!" );

    Polygon aPoly( mnPoints, mpPointAry, (bHasOffline ? mpFlagAry : NULL) );
    mrPolyPoly.Insert( aPoly );

    mnPoints = 0;
    bHasOffline = false;
}

// -----------------------------------------------------------------------

// TODO: wait till all compilers accept that calling conventions
// for functions are the same independent of implementation constness,
// then uncomment the const-tokens in the function interfaces below
static int FT_move_to( FT_Vector* /*const*/ p0, void* vpPolyArgs )
{
    PolyArgs& rA = *reinterpret_cast<PolyArgs*>(vpPolyArgs);

    // move_to implies a new polygon => finish old polygon first
    rA.ClosePolygon();

    rA.AddPoint( p0->x, p0->y, POLY_NORMAL );
    return 0;
}

static int FT_line_to( FT_Vector* /*const*/ p1, void* vpPolyArgs )
{
    PolyArgs& rA = *reinterpret_cast<PolyArgs*>(vpPolyArgs);
    rA.AddPoint( p1->x, p1->y, POLY_NORMAL );
    return 0;
}

static int FT_conic_to( FT_Vector* /*const*/ p1, FT_Vector* /*const*/ p2, void* vpPolyArgs )
{
    PolyArgs& rA = *reinterpret_cast<PolyArgs*>(vpPolyArgs);

    // VCL's Polygon only provides cubic beziers
    const long nX1 = (2 * rA.GetPosX() + 4 * p1->x + 3) / 6;
    const long nY1 = (2 * rA.GetPosY() + 4 * p1->y + 3) / 6;
    rA.AddPoint( nX1, nY1, POLY_CONTROL );

    const long nX2 = (2 * p2->x + 4 * p1->x + 3) / 6;
    const long nY2 = (2 * p2->y + 4 * p1->y + 3) / 6;
    rA.AddPoint( nX2, nY2, POLY_CONTROL );

    rA.AddPoint( p2->x, p2->y, POLY_NORMAL );
    return 0;
}

static int FT_cubic_to( FT_Vector* /*const*/ p1, FT_Vector* /*const*/ p2, FT_Vector* /*const*/ p3, void* vpPolyArgs )
{
    PolyArgs* const pA = reinterpret_cast<PolyArgs*>(vpPolyArgs);
    pA->AddPoint( p1->x, p1->y, POLY_CONTROL );
    pA->AddPoint( p2->x, p2->y, POLY_CONTROL );
    pA->AddPoint( p3->x, p3->y, POLY_NORMAL );
    return 0;
}

// -----------------------------------------------------------------------

bool FreetypeServerFont::GetGlyphOutline( int nGlyphIndex, PolyPolygon& rPolyPoly ) const
{
    FT_Int nLoadFlags = FT_LOAD_NO_HINTING | FT_LOAD_NO_BITMAP;
    FT_Error rc = FT_Load_Glyph( maFaceFT, nGlyphIndex, nLoadFlags );

    FT_Glyph aGlyphFT;
    rc = FT_Get_Glyph( maFaceFT->glyph, &aGlyphFT );

    if( aGlyphFT->format != ft_glyph_format_outline )
        return false;

    FT_Outline& rOutline = reinterpret_cast<FT_OutlineGlyphRec*>( aGlyphFT ) -> outline;
    const long nMaxPoints = rOutline.n_points * 2;
    const long nHeight = GetFontSelData().mnHeight;
    PolyArgs aPolyArg( rPolyPoly, nMaxPoints, nHeight );

    FT_Outline_Funcs aFuncs;
    aFuncs.move_to  = &FT_move_to;
    aFuncs.line_to  = &FT_line_to;
    aFuncs.conic_to = &FT_conic_to;
    aFuncs.cubic_to = &FT_cubic_to;
    aFuncs.shift    = 0;
    aFuncs.delta    = 0;
    rc = FT_Outline_Decompose( &rOutline, &aFuncs, (void*)&aPolyArg );
    aPolyArg.ClosePolygon();    // close last polygon

    FT_Done_Glyph( aGlyphFT );

    return true;
}

// =======================================================================

#endif  // NO_FREETYPE_FONTS
