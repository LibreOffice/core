/*************************************************************************
 *
 *  $RCSfile: gcach_ftyp.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hdu $ $Date: 2000-11-16 13:42:52 $
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

// -----------------------------------------------------------------------

static FT_Library aLibFT = 0;

// =======================================================================
// FreetypeManager
// =======================================================================

size_t std::hash<FtFontInfo*>::operator()( const FtFontInfo* pFI ) const
{
    size_t nHash = ::rtl::OUString( pFI->aFontData.maName ).hashCode();
    nHash += ::rtl::OUString( pFI->aFontData.maStyleName ).hashCode();
    return nHash;
}

// -----------------------------------------------------------------------

bool std::equal_to<FtFontInfo*>::operator()( const FtFontInfo* pA, const FtFontInfo* pB ) const
{
    if( (pA->aFontData.maName       == pB->aFontData.maName)
    &&  (pA->aFontData.maStyleName  == pB->aFontData.maStyleName) )
        return true;
    return false;
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
        FT_Error rcFT = FT_New_Face( aLibFT, pszFontFileName, 0, &aFaceFT );
        if( (rcFT == FT_Err_Ok) && (aFaceFT != NULL) )
        {
            if( !FT_IS_SFNT( aFaceFT ) )    // ignore non-TT fonts for now
                continue;

            FtFontInfo* const pFontInfo = new FtFontInfo;
            pFontInfo->aNativeFileName = aCFileName;

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

            int i = aFaceFT->num_charmaps;
            while( (--i >= 0) && (aFaceFT->charmaps[i]->encoding != ft_encoding_none) );
            rData.meCharSet     = (i >= 0) ? RTL_TEXTENCODING_SYMBOL : RTL_TEXTENCODING_UNICODE;
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

            if( !maFontList.insert( pFontInfo ).second )
                delete pFontInfo;
            else
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
        pToAdd->Add( new ImplFontData( (*it)->aFontData ) );    // TODO: avoid copy if possible
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
    aFontInfo.aFontData.maName      = rFSD.maName;
    aFontInfo.aFontData.maStyleName = rFSD.maStyleName;

    FontList::const_iterator it = maFontList.find( &aFontInfo );
    if( it != maFontList.end() )
        return new FreetypeServerFont( rFSD, **it );

    return NULL;
}

// =======================================================================
// FreetypeServerFont
// =======================================================================

FreetypeServerFont::FreetypeServerFont( const ImplFontSelectData& rFSD, const FtFontInfo& rFI )
:   ServerFont(rFSD), mrFontInfo(rFI)
{
    const char* pszFontFileName = rFI.aNativeFileName.getStr();
    FT_Error rc = FT_New_Face( aLibFT, pszFontFileName, 0, &maFaceFT );

    FT_Encoding eEnc = /*(rFI.aFontData.meCharSet==RTL_TEXTENCODING_SYMBOL) ? ft_encoding_none :*/ ft_encoding_unicode;
    rc = FT_Select_Charmap( maFaceFT, eEnc );

    rc = FT_Set_Pixel_Sizes( maFaceFT, rFSD.mnHeight, rFSD.mnWidth );

    if( rFSD.mnOrientation != 0 )
    {
        FT_Matrix aMatrix;
        aMatrix.xx = +nCos;
        aMatrix.yy = +nCos;
        aMatrix.xy = -nSin;
        aMatrix.yx = +nSin;
        FT_Set_Transform( maFaceFT, &aMatrix, 0 );
    }
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

    rTo.mnWidth             = GetFontSelData().mnWidth;

    const FT_Size_Metrics& rMetrics = maFaceFT->size->metrics;
    rTo.mnAscent            = (rMetrics.height - rMetrics.descender + 32) >> 6;
    rTo.mnDescent           = (rMetrics.descender + 32) >> 6;
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
        double scale = (double)GetFontSelData().mnHeight / maFaceFT->units_per_EM;
        rTo.mnWidth         = (long)( pOS2->xAvgCharWidth * scale + 0.5 );
        rTo.mnAscent        = (long)( +pOS2->usWinAscent * scale + 0.5 );
        rTo.mnDescent       = (long)( +pOS2->usWinDescent * scale + 0.5 );
        rTo.mnLeading       = (long)( (+pOS2->usWinAscent - pOS2->usWinDescent - maFaceFT->units_per_EM ) * scale + 0.5 );

        rTo.mnFirstChar     = pOS2->usFirstCharIndex;
        rTo.mnLastChar      = pOS2->usLastCharIndex;

        rTo.mnStrikeoutSize     = Min( 1L, (long)( pOS2->yStrikeoutSize * scale + 0.5 ) );
        rTo.mnStrikeoutOffset   = (long)( pOS2->yStrikeoutPosition * scale + 0.5 );
    }

    // TODO: improve these metrics
    rTo.mnSuperscriptSize   = rTo.mnAscent / 2;
    rTo.mnSuperscriptOffset = +rTo.mnAscent / 2;
    rTo.mnSubscriptSize     = rTo.mnAscent / 2;
    rTo.mnSubscriptOffset   = rTo.mnDescent;
    rTo.mnUnderlineSize     = 0;    // TODO
    rTo.mnUnderlineOffset   = rTo.mnDescent;
    rTo.mnBUnderlineSize    = 0;    // TODO
    rTo.mnBUnderlineOffset  = rTo.mnDescent;
    rTo.mnDUnderlineSize    = 0;    // TODO
    rTo.mnDUnderlineOffset1 = rTo.mnDescent;
    rTo.mnDUnderlineOffset2 = 0;    // TODO
    rTo.mnWUnderlineSize    = rTo.mnDescent;
    rTo.mnWUnderlineOffset  = 0;    // TODO
    rTo.mnBStrikeoutSize    = rTo.mnStrikeoutSize * 2;
    rTo.mnBStrikeoutOffset  = rTo.mnStrikeoutOffset;
    rTo.mnDStrikeoutSize    = rTo.mnStrikeoutSize;
    rTo.mnDStrikeoutOffset1 = rTo.mnStrikeoutOffset - rTo.mnStrikeoutSize;
    rTo.mnDStrikeoutOffset2 = rTo.mnStrikeoutOffset + rTo.mnStrikeoutSize;
}

// -----------------------------------------------------------------------

int FreetypeServerFont::GetGlyphIndex( sal_Unicode aChar ) const
{
    if( mrFontInfo.aFontData.meCharSet == RTL_TEXTENCODING_SYMBOL )
        aChar += 0xF020;    //### HACK, what encoding can be used to avoid this?

    int nGlyphIndex = FT_Get_Char_Index( maFaceFT, aChar );
    return nGlyphIndex;
}

// -----------------------------------------------------------------------

void FreetypeServerFont::SetGlyphData( int nGlyphIndex, bool bWithBitmap, GlyphData& rGD ) const
{
    const FT_Int nLoadFlags = (GetFontSelData().mnOrientation == 0) ? FT_LOAD_DEFAULT : FT_LOAD_NO_HINTING;
    FT_Error rc = FT_Load_Glyph( maFaceFT, nGlyphIndex, nLoadFlags );

    rGD.SetCharWidth( (maFaceFT->glyph->metrics.horiAdvance + 32) >> 6 );

    FT_Glyph aGlyphFT;
    rc = FT_Get_Glyph( maFaceFT->glyph, &aGlyphFT );

    rGD.SetDelta( (aGlyphFT->advance.x + 0x8000) >> 16, -((aGlyphFT->advance.y + 0x8000) >> 16) );

    FT_BBox aBbox;
    const FT_UInt nBboxMode = /*(nLoadFlags & FT_LOAD_NO_HINTING) ? ft_glyph_bbox_pixels :*/ ft_glyph_bbox_gridfit;
    FT_Glyph_Get_CBox( aGlyphFT, nBboxMode, &aBbox );
    rGD.SetOffset( aBbox.xMin, -aBbox.yMax );

    if( nBboxMode != ft_glyph_bbox_gridfit)
        FT_Glyph_Get_CBox( aGlyphFT, ft_glyph_bbox_gridfit, &aBbox );
    const Size aSize( ( (aBbox.xMax - aBbox.xMin + 7 ) & ~7), (aBbox.yMax - aBbox.yMin) );
    rGD.SetSize( aSize );

    if( bWithBitmap && !rGD.GetBitmap() )
    {
        Bitmap* pBitmap = new Bitmap( aSize, 1);

        if( !pBitmap->IsEmpty())    // empty bitmap e.g. for SPACE character
        {
            // TODO: if( GetFontSelData().mbVertical ) ...
            rc = FT_Glyph_To_Bitmap( &aGlyphFT, ft_render_mode_mono, 0, TRUE );

            const FT_Bitmap& rBitmapFT = reinterpret_cast<FT_BitmapGlyph>(aGlyphFT)->bitmap;
            const unsigned char* pSourceBuffer = rBitmapFT.buffer;
            const int pitch = rBitmapFT.pitch;
            const int rows = rBitmapFT.rows;

            DBG_ASSERT( (aSize==Size(pitch*8,rows)), "FT::CreateGlyph size mismatch!" );

            BitmapWriteAccess* const wa = pBitmap->AcquireWriteAccess();

            long x, y;
            switch( wa->GetScanlineFormat())
            {
                case (BMP_FORMAT_1BIT_MSB_PAL | BMP_FORMAT_BOTTOM_UP):
                    for( y = 0; y < rows; ++y)
                    {
                        Scanline pDestBuffer = wa->GetScanline( y );
                        for( x = 0; x < pitch; ++x )
                            pDestBuffer[x] = ~pSourceBuffer[ y * pitch + x ];
                    }
                    break;

                case (BMP_FORMAT_1BIT_MSB_PAL | BMP_FORMAT_TOP_DOWN):
                    // TODO
                    DBG_ASSERT( FALSE, "CreateGlyph: BOTTOM_UP@1BIT_LSB failure!" );
                    break;

                case (BMP_FORMAT_1BIT_LSB_PAL | BMP_FORMAT_BOTTOM_UP):
                    // TODO
                    DBG_ASSERT( FALSE, "CreateGlyph: BOTTOM_UP@1BIT_LSB failure!" );
                    break;

                case (BMP_FORMAT_1BIT_LSB_PAL | BMP_FORMAT_TOP_DOWN):
                    // TODO
                    DBG_ASSERT( FALSE, "CreateGlyph: TOP_DOWN@1BIT_LSB failure!" );
                    break;

                default:
                    DBG_ASSERT( FALSE, "CreateGlyph: illegal ScanlineFormat!" );
                    break;
            }

            pBitmap->ReleaseAccess( wa);
        }

        rGD.SetBitmap( pBitmap);
    }

    FT_Done_Glyph( aGlyphFT );
}

// -----------------------------------------------------------------------
// kerning helper functions
// -----------------------------------------------------------------------

ULONG FreetypeServerFont::GetKernPairs( ImplKernPairData** ppKernPairs ) const
{
    ULONG nKernCount = 0;
    if( !FT_HAS_KERNING( maFaceFT ) )
        ppKernPairs = NULL;
    else
    {
        // TODO...
    }
    return nKernCount;
}

// -----------------------------------------------------------------------
// outline helper functions
// -----------------------------------------------------------------------

extern "C"
{
    int FT_move_to( FT_Vector* p0, void* vpPolyArgs );
    int FT_line_to( FT_Vector* p1, void* vpPolyArgs );
    int FT_conic_to( FT_Vector* p1, FT_Vector* p2, void* vpPolyArgs );
    int FT_cubic_to( FT_Vector* p1, FT_Vector* p2, FT_Vector* p3, void* vpPolyArgs );
};  // end extern "C"

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

static int FT_move_to( FT_Vector* const p0, void* vpPolyArgs )
{
    PolyArgs& rA = *reinterpret_cast<PolyArgs*>(vpPolyArgs);

    // move_to implies a new polygon => finish old polygon first
    rA.ClosePolygon();

    rA.AddPoint( p0->x, p0->y, POLY_NORMAL );
    return 0;
}

static int FT_line_to( FT_Vector* const p1, void* vpPolyArgs )
{
    PolyArgs& rA = *reinterpret_cast<PolyArgs*>(vpPolyArgs);
    rA.AddPoint( p1->x, p1->y , POLY_NORMAL );
    return 0;
}

static int FT_conic_to( FT_Vector* const p1, FT_Vector* const p2, void* vpPolyArgs )
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

static int FT_cubic_to( FT_Vector* const p1, FT_Vector* const p2, FT_Vector* const p3, void* vpPolyArgs )
{
    PolyArgs* const pA = reinterpret_cast<PolyArgs*>(vpPolyArgs);
    pA->AddPoint( p1->x, p1->y, POLY_CONTROL );
    pA->AddPoint( p2->x, p2->y, POLY_CONTROL );
    pA->AddPoint( p3->x, p3->y, POLY_NORMAL );
    return 0;
}

// -----------------------------------------------------------------------

bool FreetypeServerFont::GetGlyphOutline( int nGlyphIndex, bool bOptimize, PolyPolygon& rPolyPoly ) const
{
    FT_Error rc = FT_Load_Glyph( maFaceFT, nGlyphIndex, FT_LOAD_NO_HINTING | FT_LOAD_NO_BITMAP );

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

    if( bOptimize)
        rPolyPoly.Optimize( /*POLY_OPTIMIZE_NO_SAME |*/ POLY_OPTIMIZE_REDUCE | POLY_OPTIMIZE_EDGES );

    return true;
}

// =======================================================================

#endif  // NO_FREETYPE_FONTS
