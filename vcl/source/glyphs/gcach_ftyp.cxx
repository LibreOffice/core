/*************************************************************************
 *
 *  $RCSfile: gcach_ftyp.cxx,v $
 *
 *  $Revision: 1.53 $
 *  last change: $Author: hdu $ $Date: 2001-07-18 15:34:49 $
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

#if defined(WIN32)
#ifndef _SV_SVSYS_HXX

#include <svsys.h>
#undef CreateFont
#endif
#endif

#include <gcach_ftyp.hxx>

#include <svapp.hxx>
#include <outfont.hxx>
#include <bitmap.hxx>
#include <bmpacc.hxx>
#include <poly.hxx>

#include <osl/file.hxx>
#include <osl/thread.hxx>

// VERSION_MINOR in freetype.h is too coarse, we need to fine-tune ourselves:
#if (SUPD <= 632)
    #define FTVERSION 198
#else
    #define FTVERSION 202
#endif

#include "freetype/freetype.h"
#include "freetype/ftglyph.h"
#include "freetype/ftoutln.h"
#include "freetype/tttables.h"
#include "freetype/tttags.h"
#include "freetype/ttnameid.h"

#include <vector>

// TODO: move file mapping stuff to OSL
#if defined(UNX)
    #include <unistd.h>
    #include <fcntl.h>
    #include <sys/stat.h>
    #include <sys/mman.h>
#elif defined(WIN32)
    #include <io.h>
#endif

#include "freetype/internal/ftobjs.h"
#include "freetype/internal/sfnt.h"
#include "freetype/internal/ftstream.h"

#include <svapp.hxx>
#include <settings.hxx>
#include <tools/lang.hxx>

// -----------------------------------------------------------------------

static FT_Library aLibFT = 0;

struct EqStr{ bool operator()(const char* a, const char* b) const { return !strcmp(a,b); } };
typedef ::std::hash_map<const char*,FtFontFile*,::std::hash<const char*>, EqStr> FontFileList;
static FontFileList aFontFileList;

// =======================================================================
// FreetypeManager
// =======================================================================

FtFontFile::FtFontFile( const ::rtl::OString& rNativeFileName )
:   maNativeFileName( rNativeFileName ),
    mpFileMap( NULL ),
    mnFileSize( 0 ),
    mnRefCount( 0 )
{}

// -----------------------------------------------------------------------

FtFontFile* FtFontFile::FindFontFile( const ::rtl::OString& rNativeFileName )
{
    // font file already known? (e.g. for ttc, synthetic, aliased fonts)
    const char* pFileName = rNativeFileName.getStr();
    FontFileList::const_iterator it = aFontFileList.find( pFileName );
    if( it != aFontFileList.end() )
        return (*it).second;

    // no => create new one
    FtFontFile* pFontFile = new FtFontFile( rNativeFileName );
    aFontFileList[ pFileName ] = pFontFile;
    return pFontFile;
}

// -----------------------------------------------------------------------

bool FtFontFile::Map()
{
    if( mnRefCount++ <= 0 )
    {
        const char* pFileName = maNativeFileName.getStr();
#if defined(UNX)
        int nFile = open( pFileName, O_RDONLY );
        if( nFile < 0 )
            return false;

        struct stat aStat;
        fstat( nFile, &aStat );
        mnFileSize = aStat.st_size;
        mpFileMap = (const unsigned char*)
            mmap( NULL, mnFileSize, PROT_READ, MAP_SHARED, nFile, NULL );
        close( nFile );
#elif defined(WIN32)
        void* pFileDesc = CreateFile( pFileName, GENERIC_READ, FILE_SHARE_READ,
                        NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0 );
        if( pFileDesc == INVALID_HANDLE_VALUE)
            return false;

        mnFileSize = GetFileSize( pFileDesc, NULL );
        HANDLE aHandle = CreateFileMapping( pFileDesc, NULL, PAGE_READONLY, 0, mnFileSize, "TTF" );
        mpFileMap = (const unsigned char*) MapViewOfFile( aHandle, FILE_MAP_READ, 0, 0, mnFileSize );
        CloseHandle( pFileDesc );
#else
        FILE* pFile = fopen( pFileName, "rb" );
        if( !pFile )
            return false;

        struct stat aStat;
        stat( pFileName, &aStat );
        mnFileSize = aStat.st_size;
        mpFileMap = new unsigned char[ mnFileSize ];
        if( mnFileSize != fread( mpFileMap, 1, mnFileSize, pFile ) )
        {
            delete[] mpFileMap;
            mpFileMap = NULL;
        }
        fclose( pFile );
#endif
    }

    return (mpFileMap != NULL);
}

// -----------------------------------------------------------------------

void FtFontFile::Unmap()
{
    if( (--mnRefCount > 0) || (mpFileMap == NULL) )
        return;

#if defined(UNX)
    munmap( (char*)mpFileMap, mnFileSize );
#elif defined(WIN32)
    UnmapViewOfFile( (LPCVOID)mpFileMap );
#else
    delete[] mpFileMap;
#endif

    mpFileMap = NULL;
}

// =======================================================================

FtFontInfo::FtFontInfo( const ImplFontData& rFontData,
    const ::rtl::OString& rNativeFileName, int nFaceNum, int nFontId, int nSynthetic )
:   mpFontFile( FtFontFile::FindFontFile( rNativeFileName ) ),
    maFontData( rFontData ),
    mnFaceNum( nFaceNum ),
    mnFontId( nFontId ),
    mnSynthetic( nSynthetic )
{
    maFontData.mpSysData = (void*)this;
    maFontData.mpNext    = NULL;

    // using unicode emulation for non-symbol fonts
    if( maFontData.meCharSet != RTL_TEXTENCODING_SYMBOL )
        maFontData.meCharSet = RTL_TEXTENCODING_UNICODE;
}

// -----------------------------------------------------------------------

size_t std::hash<FtFontInfo*>::operator()( const FtFontInfo* pFI ) const
{
    size_t nHash = (size_t)pFI;
    return nHash;
}

// -----------------------------------------------------------------------

bool std::equal_to<FtFontInfo*>::operator()( const FtFontInfo* pA, const FtFontInfo* pB ) const
{
    return (pA == pB);
}

// -----------------------------------------------------------------------

static unsigned GetUInt( const unsigned char* p ) { return((p[0]<<24)+(p[1]<<16)+(p[2]<<8)+p[3]);}
static unsigned GetUShort( const unsigned char* p ){ return((p[0]<<8)+p[1]);}
static signed GetSShort( const unsigned char* p ){ return((short)((p[0]<<8)+p[1]));}

// -----------------------------------------------------------------------

const unsigned char* FtFontInfo::GetTable( const char* pTag, ULONG* pLength ) const
{
    const unsigned char* pBuffer = GetBuffer();
    int nFileSize = GetFileSize();
    if( !pBuffer || nFileSize<1024 )
        return NULL;

    // we currently only handle TTF and TTC headers
    unsigned nFormat = GetUInt( pBuffer );
    const unsigned char* p = pBuffer + 12;
    if( nFormat == 0x74746366 )         // TTC_MAGIC
        p += GetUInt( p + 4 * mnFaceNum );
    else if( nFormat != 0x00010000 )    // TTF_MAGIC
        return NULL;

    // walk table directory until match
    int nTables = GetUShort( p - 8 );
    if( nTables >= 64 )  // something fishy?
        return NULL;
    for( int i = 0; i < nTables; ++i, p+=16 )
    {
        if( p[0]==pTag[0] && p[1]==pTag[1] && p[2]==pTag[2] && p[3]==pTag[3] )
        {
            ULONG nLength = GetUInt( p + 12 );
            if( pLength != NULL )
                *pLength = nLength;
            const unsigned char* pTable = pBuffer + GetUInt( p + 8 );
            if( (pTable + nLength) <= (GetBuffer() + nFileSize) )
                return pTable;
        }
    }

    return NULL;
}

// =======================================================================

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

void* FreetypeManager::GetFontHandle (int nFontId)
{
    for( FontList::const_iterator it = maFontList.begin(); it != maFontList.end(); ++it )
        if( nFontId == (*it)->GetFontId() )
            return (void*)*it;
    return NULL;
}

// -----------------------------------------------------------------------

void FreetypeManager::AddFontFile( const rtl::OString& rNormalizedName,
    int nFaceNum, int nFontId, const ImplFontData* pData )
{
    if( !rNormalizedName.getLength() )
        return;

    maFontList.insert( new FtFontInfo( *pData, rNormalizedName, nFaceNum, nFontId, 0 ) );
}

// -----------------------------------------------------------------------

long FreetypeManager::AddFontDir( const String& rUrlName )
{
    osl::Directory aDir( rUrlName );
    osl::FileBase::RC rcOSL = aDir.open();
    if( rcOSL != osl::FileBase::E_None )
        return 0;

    long nCount = 0;

    osl::DirectoryItem aDirItem;
    rtl_TextEncoding theEncoding = osl_getThreadTextEncoding();
    while( (rcOSL = aDir.getNextItem( aDirItem, 20 )) == osl::FileBase::E_None )
    {
        osl::FileStatus aFileStatus( FileStatusMask_FileURL );
        rcOSL = aDirItem.getFileStatus( aFileStatus );

        ::rtl::OUString aUSytemPath;
        OSL_VERIFY(  osl_File_E_None
            == ::osl::FileBase::getSystemPathFromFileURL( aFileStatus.getFileURL(), aUSytemPath ));
        ::rtl::OString aCFileName = rtl::OUStringToOString( aUSytemPath, theEncoding );

        const char* pszFontFileName = aCFileName.getStr();

        FT_FaceRec_* aFaceFT = NULL;
        for( int nFaceNum = 0, nMaxFaces = 1; nFaceNum < nMaxFaces; ++nFaceNum )
        {
            FT_Error rcFT = FT_New_Face( aLibFT, pszFontFileName, nFaceNum, &aFaceFT );
            if( (rcFT != FT_Err_Ok) || (aFaceFT == NULL) )
                break;

            if( !FT_IS_SCALABLE( aFaceFT ) )    // ignore non-scalabale fonts
                continue;

            nMaxFaces = aFaceFT->num_faces;

            ImplFontData aFontData;

            // TODO: prefer unicode names if available
            // TODO: prefer locale specific names if available?
            aFontData.maName        = String::CreateFromAscii( aFaceFT->family_name );
            aFontData.maStyleName   = String::CreateFromAscii( aFaceFT->style_name );

            aFontData.mnWidth   = 0;
            aFontData.mnHeight  = 0;

            // TODO: extract better font characterization data from font
            aFontData.meFamily  = FAMILY_DONTKNOW;

            aFontData.meCharSet = RTL_TEXTENCODING_UNICODE;
            for( int i = aFaceFT->num_charmaps; --i >= 0; )
            {
                const FT_CharMap aCM = aFaceFT->charmaps[i];
#if (FTVERSION < 200)
                if( aCM->encoding == ft_encoding_none )
#else
                if( (aCM->platform_id == TT_PLATFORM_MICROSOFT)
                &&  (aCM->encoding_id == TT_MS_ID_SYMBOL_CS) )
#endif
                    aFontData.meCharSet = RTL_TEXTENCODING_SYMBOL;
            }

            aFontData.mePitch       = FT_IS_FIXED_WIDTH( aFaceFT ) ? PITCH_FIXED : PITCH_VARIABLE;
            aFontData.meWidthType   = WIDTH_DONTKNOW;
            aFontData.meWeight      = FT_STYLE_FLAG_BOLD & aFaceFT->style_flags ? WEIGHT_BOLD : WEIGHT_NORMAL;
            aFontData.meItalic      = FT_STYLE_FLAG_ITALIC & aFaceFT->style_flags ? ITALIC_NORMAL : ITALIC_NONE;

            FT_Done_Face( aFaceFT );

            aFontData.mnVerticalOrientation = 0;
            aFontData.mbOrientation = true;
            aFontData.mbDevice      = false;
            aFontData.mnQuality     = 0;    // prefer client-side fonts if available

            AddFontFile( aCFileName, nFaceNum, 0, &aFontData );
            ++nCount;
        }
    }

    aDir.close();
    return nCount;
}

// -----------------------------------------------------------------------

long FreetypeManager::FetchFontList( ImplDevFontList* pToAdd ) const
{
    const char* pLangBoostName = "soui.ttf";
    const LanguageType aLang = Application::GetSettings().GetUILanguage();
    switch( aLang )
    {
        case LANGUAGE_JAPANESE:
            pLangBoostName = "soui.ttf";    // japanese is default
            break;
        case LANGUAGE_CHINESE:
        case LANGUAGE_CHINESE_SIMPLIFIED:
        case LANGUAGE_CHINESE_SINGAPORE:
            pLangBoostName = "soui_zhs.ttf";
            break;
        case LANGUAGE_CHINESE_TRADITIONAL:
        case LANGUAGE_CHINESE_HONGKONG:
        case LANGUAGE_CHINESE_MACAU:
            pLangBoostName = "soui_zht.ttf";
            break;
        case LANGUAGE_KOREAN:
        case LANGUAGE_KOREAN_JOHAB:
            pLangBoostName = "soui_kor.ttf";
            break;
    }
    const rtl::OString aLangBoostName( pLangBoostName );

    long nCount = 0;
    for( FontList::const_iterator it(maFontList.begin()); it != maFontList.end(); ++it, ++nCount )
    {
        ImplFontData* pFontData = new ImplFontData( (*it)->GetFontData() );

        // boost UI font quality if UI language matches
        const ::rtl::OString aCFileName = (*it)->GetFontFileName()->toAsciiLowerCase();
        if( 0 < aCFileName.indexOf( aLangBoostName ) )
            pFontData->mnQuality += 10;

        pToAdd->Add( pFontData );
    }

    return nCount;
}

// -----------------------------------------------------------------------

void FreetypeManager::ClearFontList( )
{
    for( FontList::iterator it(maFontList.begin()); it != maFontList.end(); )
        delete *(it++);
    maFontList.clear();
}

// -----------------------------------------------------------------------

FreetypeServerFont* FreetypeManager::CreateFont( const ImplFontSelectData& rFSD )
{
    FtFontInfo* pFI = (FtFontInfo*)rFSD.mpFontData->mpSysData;
    if( maFontList.find( pFI ) != maFontList.end() )
    {
        FreetypeServerFont* pFont = new FreetypeServerFont( rFSD, pFI );
        return pFont;
    }

    return NULL;
}

// =======================================================================
// FreetypeServerFont
// =======================================================================

FreetypeServerFont::FreetypeServerFont( const ImplFontSelectData& rFSD, FtFontInfo* pFI )
:   ServerFont(rFSD),
    mpFontInfo(pFI),
    maFaceFT(NULL)
{
    if( !pFI->MapFile() )
        return;

    FT_Error rc = FT_New_Memory_Face( aLibFT, (FT_Byte*)pFI->GetBuffer(),
        pFI->GetFileSize(), mpFontInfo->GetFaceNum(), &maFaceFT );
    if( rc != FT_Err_Ok || maFaceFT->num_glyphs == 0 )
        return;

    FT_Encoding eEncoding = ft_encoding_unicode;
    if( mpFontInfo->GetFontData().meCharSet == RTL_TEXTENCODING_SYMBOL )
    {
#if (FTVERSION < 200)
        eEncoding = ft_encoding_none;
#else
        if( FT_IS_SFNT( maFaceFT ) )
            eEncoding = ft_encoding_symbol;
        else
            eEncoding = ft_encoding_adobe_custom; // freetype wants this for PS symbol fonts
#endif
    }
    rc = FT_Select_Charmap( maFaceFT, eEncoding );

    mnWidth = rFSD.mnWidth;
    if( !mnWidth )
        mnWidth = rFSD.mnHeight;
    rc = FT_Set_Pixel_Sizes( maFaceFT, mnWidth, rFSD.mnHeight );

    ApplyGSUB( rFSD );

    // TODO: query GASP table for load flags
    mnLoadFlags = FT_LOAD_DEFAULT;

    if( rFSD.mnOrientation != 0 )       // no embedded bitmap for rotated text
        mnLoadFlags |= FT_LOAD_NO_BITMAP;
    if( nSin != 0 && nCos != 0 )        // hinting for 0/90/180/270 degrees only
        mnLoadFlags |= FT_LOAD_NO_HINTING;
    mnLoadFlags |= FT_LOAD_IGNORE_GLOBAL_ADVANCE_WIDTH; //#88334#

#if (FTVERSION < 205) && !defined(TT_CONFIG_OPTION_BYTECODE_INTERPRETER)
    mnLoadFlags |= FT_LOAD_NO_HINTING;  // TODO: enable when AH improves
#endif
}

// -----------------------------------------------------------------------

bool FreetypeServerFont::TestFont() const
{
    return (maFaceFT != NULL) && (maFaceFT->num_glyphs >= 0);
}

// -----------------------------------------------------------------------

FreetypeServerFont::~FreetypeServerFont()
{
    mpFontInfo->Unmap();

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
#if (FTVERSION < 200)
    rTo.mnDescent           = (+rMetrics.descender + 32) >> 6;
#else
    rTo.mnDescent           = (-rMetrics.descender + 32) >> 6;
#endif
    rTo.mnLeading           = ((rMetrics.height + 32) >> 6) - (rTo.mnAscent + rTo.mnDescent);
    rTo.mnSlant             = 0;

    rTo.maName              = mpFontInfo->GetFontData().maName;
    rTo.maStyleName         = mpFontInfo->GetFontData().maStyleName;

    rTo.mnFirstChar         = 0x0020;
    rTo.mnLastChar          = 0xFFFE;

    rTo.mnOrientation       = GetFontSelData().mnOrientation;

    rTo.meCharSet           = mpFontInfo->GetFontData().meCharSet;
    rTo.meFamily            = mpFontInfo->GetFontData().meFamily;
    rTo.meWeight            = mpFontInfo->GetFontData().meWeight;
    rTo.meItalic            = mpFontInfo->GetFontData().meItalic;
    rTo.mePitch             = mpFontInfo->GetFontData().mePitch;

    rTo.meType              = TYPE_SCALABLE;
    rTo.mbDevice            = FALSE;

    const TT_OS2* pOS2 = (const TT_OS2*)FT_Get_Sfnt_Table( maFaceFT, ft_sfnt_os2 );
    if( pOS2 && (~pOS2->version != 0) )
    {
        const double fScale = (double)GetFontSelData().mnHeight / maFaceFT->units_per_EM;
        rTo.mnAscent        = (long)( +pOS2->usWinAscent * fScale + 0.5 );
        rTo.mnDescent       = (long)( +pOS2->usWinDescent * fScale + 0.5 );
        rTo.mnLeading       = (long)( (+pOS2->usWinAscent + pOS2->usWinDescent - maFaceFT->units_per_EM) * fScale + 0.5 );

        rTo.mnFirstChar     = pOS2->usFirstCharIndex;
        rTo.mnLastChar      = pOS2->usLastCharIndex;
    }
}

// -----------------------------------------------------------------------

static int SetVerticalFlags( sal_Unicode nChar )
{
    if ( (nChar >= 0x1100 && nChar <= 0x11f9)   // Hangul Jamo
    ||  (nChar >= 0x3000 && nChar <= 0xfaff) )  // other CJK
    {
        if( nChar == 0x2010 || nChar == 0x2015
        ||  nChar == 0x2016 || nChar == 0x2026
        || (nChar >= 0x3008 && nChar <= 0x3017)
        ||  nChar >= 0xFF00 )
            return 0;   // not rotated
        else if ( nChar == 0x30fc )
            return +2;  // right
        return +1;      // left
    }

    return 0;
}

static inline void SetGlyphFlags( int& nGlyphIndex, int nFlags )
{
    nGlyphIndex |= (nFlags << 28);
}

static inline void SplitGlyphFlags( int& nGlyphIndex, int& nGlyphFlags )
{
    nGlyphFlags = (nGlyphIndex >> 28);
    nGlyphIndex &= 0x00ffffff;
}

static void SetTransform( int nSin, int nCos, int nHeight, int nGlyphFlags, FT_Glyph& rGlyphFT )
{
    FT_Vector aVector;
    FT_Matrix aMatrix;
    switch( nGlyphFlags )
    {
    case +1:    // left
        aMatrix.xx = -nSin;
        aMatrix.yy = -nSin;
        aMatrix.xy = -nCos;
        aMatrix.yx = +nCos;
        aVector.x  = +(nHeight * nCos) >> 10;
        aVector.y  = +(nHeight * nSin) >> 10;
        break;
    case +2:    // right
        aMatrix.xx = -nSin;
        aMatrix.yy = -nSin;
        aMatrix.xy = +nCos;
        aMatrix.yx = -nCos;
        aVector.x  = -(nHeight * nCos) >> 10;
        aVector.y  = -(nHeight * nSin) >> 10;
        break;
    default:    // straight
        aMatrix.xx = +nCos;
        aMatrix.yy = +nCos;
        aMatrix.xy = -nSin;
        aMatrix.yx = +nSin;
        aVector.x = 0;
        aVector.y = 0;
        break;
    }

    FT_Glyph_Transform( rGlyphFT, &aMatrix, &aVector );
}

// -----------------------------------------------------------------------

int FreetypeServerFont::GetGlyphIndex( sal_Unicode aChar ) const
{
    if( mpFontInfo->GetFontData().meCharSet == RTL_TEXTENCODING_SYMBOL )
        if( FT_IS_SFNT( maFaceFT ) )
            aChar |= 0xF000;    // emulate W2K high/low mapping of symbols
        else
        {
            if( (aChar&0xFF00) == 0xF000 )
                aChar &= 0xFF;    // PS font symbol mapping
            else if( aChar > 0xFF )
                return 0;
        }

    int nGlyphIndex = FT_Get_Char_Index( maFaceFT, aChar );

    // do glyph substitution if necessary
    GlyphSubstitution::const_iterator it = aGlyphSubstitution.find( nGlyphIndex );
    // use OpenType substitution if available
    if( it != aGlyphSubstitution.end() )
        nGlyphIndex = (*it).second;

    // CJK vertical writing needs special treatment
    if( nGlyphIndex!=0 && GetFontSelData().mbVertical )
    {
        int nVertFlags = SetVerticalFlags( aChar );
        SetGlyphFlags( nGlyphIndex, nVertFlags );
    }

    return nGlyphIndex;
}

// -----------------------------------------------------------------------

void FreetypeServerFont::InitGlyphData( int nGlyphIndex, GlyphData& rGD ) const
{
    int nGlyphFlags;
    SplitGlyphFlags( nGlyphIndex, nGlyphFlags );

    FT_Error rc = FT_Load_Glyph( maFaceFT, nGlyphIndex, mnLoadFlags );
    if( rc != FT_Err_Ok )
    {
        // we get here e.g. when a PS font lacks the default glyph
        rGD.SetCharWidth( 0 );
        rGD.SetDelta( 0, 0 );
        rGD.SetOffset( 0, 0 );
        rGD.SetSize( Size( 0, 0 ) );
        return;
    }

    rGD.SetCharWidth( (maFaceFT->glyph->metrics.horiAdvance + 32) >> 6 );

    FT_Glyph aGlyphFT;
    rc = FT_Get_Glyph( maFaceFT->glyph, &aGlyphFT );

    if( (nCos!=0x10000) || (nGlyphFlags!=0) )
        SetTransform( nSin, nCos, GetFontSelData().mnHeight,
            nGlyphFlags, aGlyphFT );

    rGD.SetDelta( (aGlyphFT->advance.x + 0x8000) >> 16, -((aGlyphFT->advance.y + 0x8000) >> 16) );

    FT_BBox aBbox;
    FT_Glyph_Get_CBox( aGlyphFT, ft_glyph_bbox_pixels, &aBbox );
    if( aBbox.yMin > aBbox.yMax )   // circumvent freetype bug
    {
        int t=aBbox.yMin; aBbox.yMin=aBbox.yMax, aBbox.yMax=t;
    }
    rGD.SetOffset( aBbox.xMin, -aBbox.yMax );
    rGD.SetSize( Size( (aBbox.xMax-aBbox.xMin+1), (aBbox.yMax-aBbox.yMin) ) );

    FT_Done_Glyph( aGlyphFT );
}

// -----------------------------------------------------------------------

bool FreetypeServerFont::GetAntialiasAdvice( void ) const
{
    if( GetFontSelData().mbNonAntialiased )
        return false;
    // TODO: also use GASP & EBDT tables
    bool bAdviseAA = (mnLoadFlags & FT_LOAD_NO_HINTING) != 0;
    int nHeight = GetFontSelData().mnHeight;
    bAdviseAA |= (nHeight > 12);
    bAdviseAA |= (nHeight < 8);
    return bAdviseAA;
}

// -----------------------------------------------------------------------

bool FreetypeServerFont::GetGlyphBitmap1( int nGlyphIndex, RawBitmap& rRawBitmap ) const
{
    int nGlyphFlags;
    SplitGlyphFlags( nGlyphIndex, nGlyphFlags );
    FT_Int nLoadFlags = mnLoadFlags;
    if( nGlyphFlags != 0 )
        nLoadFlags |= FT_LOAD_NO_BITMAP;
#if (FTVERSION >= 202)
    if( nCos==0 || nSin==0 )
        nLoadFlags &= ~FT_LOAD_NO_HINTING;
#endif

    FT_Error rc = -1;
#if (FTVERSION < 205)
    // #88364# freetype<=204 prefers autohinting to embedded bitmaps
    // => first we have to try without hinting
    if( (nLoadFlags & (FT_LOAD_NO_HINTING | FT_LOAD_NO_BITMAP)) == 0 )
    {
        rc = FT_Load_Glyph( maFaceFT, nGlyphIndex, nLoadFlags|FT_LOAD_NO_HINTING );
        if( (rc==FT_Err_Ok) && (maFaceFT->glyph->format!=ft_glyph_format_bitmap) )
            rc = -1; // mark as "loading embedded bitmap" was unsuccessful
    }
#endif

    if( rc != FT_Err_Ok )
        rc = FT_Load_Glyph( maFaceFT, nGlyphIndex, nLoadFlags );
    if( rc != FT_Err_Ok )
        return false;

    FT_Glyph aGlyphFT;
    rc = FT_Get_Glyph( maFaceFT->glyph, &aGlyphFT );
    if( rc != FT_Err_Ok )
        return false;

    if( aGlyphFT->format != ft_glyph_format_bitmap )
    {
        if( (nCos!=0x10000) || (nGlyphFlags!=0) )
            SetTransform( nSin, nCos, GetFontSelData().mnHeight,
                nGlyphFlags, aGlyphFT );

        if( aGlyphFT->format == ft_glyph_format_outline )
            ((FT_OutlineGlyphRec*)aGlyphFT )->outline.flags |= ft_outline_high_precision;
        rc = FT_Glyph_To_Bitmap( &aGlyphFT, ft_render_mode_mono, NULL, TRUE );
        if( rc != FT_Err_Ok )
            return false;
    }

    const FT_BitmapGlyph& rBmpGlyphFT = reinterpret_cast<const FT_BitmapGlyph&>(aGlyphFT);
    // autohinting in FT<=2.0.2 miscalculates the offsets below by +-1
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

    memcpy( rRawBitmap.mpBits, rBitmapFT.buffer, nNeededSize );

    FT_Done_Glyph( aGlyphFT );
    return true;
}

// -----------------------------------------------------------------------

bool FreetypeServerFont::GetGlyphBitmap8( int nGlyphIndex, RawBitmap& rRawBitmap ) const
{
    int nGlyphFlags;
    SplitGlyphFlags( nGlyphIndex, nGlyphFlags );
    FT_Int nLoadFlags = mnLoadFlags;
    if( nGlyphFlags != 0 )
        nLoadFlags |= FT_LOAD_NO_BITMAP;
#if (FTVERSION < 205) && !defined(TT_CONFIG_OPTION_BYTECODE_INTERPRETER)
    // autohinting in FT<=2.0.4 makes antialiased glyphs look worse
    nLoadFlags |= FT_LOAD_NO_HINTING;
#endif

    FT_Error rc = FT_Load_Glyph( maFaceFT, nGlyphIndex, nLoadFlags );
    if( rc != FT_Err_Ok )
        return false;

    FT_Glyph aGlyphFT;
    rc = FT_Get_Glyph( maFaceFT->glyph, &aGlyphFT );
    if( rc != FT_Err_Ok )
        return false;

    if( aGlyphFT->format == ft_glyph_format_outline )
    {
        if( (nCos!=0x10000) || (nGlyphFlags!=0) )
            SetTransform( nSin, nCos, GetFontSelData().mnHeight,
                nGlyphFlags, aGlyphFT );

        ((FT_OutlineGlyph)aGlyphFT)->outline.flags |= ft_outline_high_precision;
    }

    bool bEmbedded = (aGlyphFT->format == ft_glyph_format_bitmap);
    if( !bEmbedded )
    {
        rc = FT_Glyph_To_Bitmap( &aGlyphFT, ft_render_mode_normal, NULL, TRUE );
        if( rc != FT_Err_Ok )
            return false;
    }

    const FT_BitmapGlyph& rBmpGlyphFT = reinterpret_cast<const FT_BitmapGlyph&>(aGlyphFT);
    rRawBitmap.mnXOffset        = +rBmpGlyphFT->left;
    rRawBitmap.mnYOffset        = -rBmpGlyphFT->top;

    const FT_Bitmap& rBitmapFT  = rBmpGlyphFT->bitmap;
    rRawBitmap.mnHeight         = rBitmapFT.rows;
    rRawBitmap.mnWidth          = rBitmapFT.width;
    rRawBitmap.mnScanlineSize   = ((bEmbedded?rBitmapFT.width:rBitmapFT.pitch) + 3) & -4;
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
    if( !bEmbedded )
    {
        for( int y = rRawBitmap.mnHeight, x; --y >= 0 ; )
        {
            for( x = 0; x < rBitmapFT.width; ++x )
                *(pDest++) = *(pSrc++);
            for(; x < rRawBitmap.mnScanlineSize; ++x )
                *(pDest++) = 0;
        }
    }
    else
    {
        for( int y = rRawBitmap.mnHeight, x; --y >= 0 ; )
        {
            unsigned char nSrc;
            for( x = 0; x < rBitmapFT.width; ++x, nSrc+=nSrc )
            {
                if( (x & 7) == 0 )
                    nSrc = *(pSrc++);
                *(pDest++) = (0x7F - nSrc) >> 8;
            }
            for(; x < rRawBitmap.mnScanlineSize; ++x )
                *(pDest++) = 0;
        }
    }

    FT_Done_Glyph( aGlyphFT );
    return true;
}

// -----------------------------------------------------------------------
// determine unicode ranges in font
// -----------------------------------------------------------------------

ULONG FreetypeServerFont::GetFontCodeRanges( sal_uInt32* pCodes ) const
{
    int nRangeCount = 0;

    const unsigned char* pCmap = NULL;
    ULONG nLength = 0;
    if( FT_IS_SFNT( maFaceFT ) )
        pCmap = mpFontInfo->GetTable( "cmap", &nLength );
    else if( mpFontInfo->GetFontData().meCharSet == RTL_TEXTENCODING_SYMBOL )
    {
        // postscript symbol font
        nRangeCount = 1;
        if( pCodes )
        {
            pCodes[ 0 ] = 0xF020;
            pCodes[ 1 ] = 0xF100;
        }
    }

    if( pCmap && GetUShort( pCmap )==0 )
    {
        int nSubTables  = GetUShort( pCmap + 2 );
        const unsigned char* p = pCmap + 4;
        int nOffset = 0;
        int nFormat = -1;
        for( ; --nSubTables>=0; p+=8 )
        {
            int nPlatform = GetUShort( p );
            int nEncoding = GetUShort( p+2 );
            if( nEncoding!=0 && nEncoding!=1 )  // unicode encodings?
                continue;
            nOffset       = GetUInt( p+4 );
            nFormat       = GetUShort( pCmap + nOffset );
            if( nFormat==4 )
                break;
        }

        if( nFormat==4 && (nOffset+16)<nLength )
        {
            // analyze most common unicode mapping table
            int nSegCount = GetUShort( pCmap + nOffset + 6 );
            nRangeCount = nSegCount/2 - 1;
            if( pCodes )
            {
                const unsigned char* pLimit = pCmap + nOffset + 14;
                const unsigned char* pBegin = pLimit + 2 + nSegCount;
                for( int i = 0; i < nRangeCount; ++i )
                {
                    *(pCodes++) = GetUShort( pBegin + 2*i );
                    *(pCodes++) = GetUShort( pLimit + 2*i ) + 1;
                }
            }
        }
    }

    if( !nRangeCount )
    {
        // unknown format, platform or encoding => use the brute force method
        for( sal_uInt32 cCode = 0x0020;; )
        {
            for(; cCode<0xFFF0 && !GetGlyphIndex( cCode ); ++cCode );
            if( cCode >= 0xFFF0 )
                break;
            ++nRangeCount;
            if( pCodes )
                *(pCodes++) = cCode;
            for(; cCode<0xFFF0 && GetGlyphIndex( cCode ); ++cCode );
            if( pCodes )
                *(pCodes++) = cCode;
        }
    }

    return nRangeCount;
}
// -----------------------------------------------------------------------
// kerning stuff
// -----------------------------------------------------------------------

ULONG FreetypeServerFont::GetKernPairs( ImplKernPairData** ppKernPairs ) const
{
    *ppKernPairs = NULL;
    if( !FT_HAS_KERNING( maFaceFT ) || !FT_IS_SFNT( maFaceFT ) )
        return 0;

    // first figure out which glyph pairs are involved in kerning
    ULONG nKernLength = 0;
    const FT_Byte* const pKern = mpFontInfo->GetTable( "kern", &nKernLength );
    if( !pKern )
        return 0;

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
                    aKernPair.mnChar1 = NEXT_UShort( pBuffer );
                    aKernPair.mnChar2 = NEXT_UShort( pBuffer );
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

    // now create VCL's ImplKernPairData[] format for all glyph pairs
    ULONG nKernCount = aKernGlyphVector.size();
    if( nKernCount )
    {
        // prepare glyphindex to character mapping
        // TODO: this is needed to support VCL's existing kerning infrastructure,
        // eliminate it up by redesigning kerning infrastructure to work with glyph indizes
        typedef std::hash_multimap<USHORT,sal_Unicode> Cmap;
        Cmap aCmap;
        for( sal_Unicode aChar = 0x0020; aChar < 0xFFFE; ++aChar )
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
            aKernPair.mnKern = aKernVal.x >> 6;
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

// -----------------------------------------------------------------------

bool FreetypeServerFont::ApplyGSUB( const ImplFontSelectData& rFSD )
{
#define MKTAG(s) ((((((s[0]<<8)+s[1])<<8)+s[2])<<8)+s[3])

    typedef std::vector<ULONG> ReqFeatureTagList;
    ReqFeatureTagList aReqFeatureTagList;
    if( rFSD.mbVertical )
        aReqFeatureTagList.push_back( MKTAG("vert") );
    ULONG nRequestedScript = 0;     //MKTAG("hani");//### TODO: where to get script?
    ULONG nRequestedLangsys = 0;    //MKTAG("ZHT"); //### TODO: where to get langsys?
    // TODO: request more features depending on script and language system

    if( aReqFeatureTagList.size() == 0) // nothing to do
        return true;

    // load GSUB table into memory
    ULONG nLength = 0;
    const FT_Byte* const pGsubBase = mpFontInfo->GetTable( "GSUB", &nLength );
    if( !pGsubBase )
        return false;

    // parse GSUB header
    const FT_Byte* pGsubHeader = pGsubBase;
    const ULONG nVersion            = NEXT_Long( pGsubHeader );
    const USHORT nOfsScriptList     = NEXT_UShort( pGsubHeader );
    const USHORT nOfsFeatureTable   = NEXT_UShort( pGsubHeader );
    const USHORT nOfsLookupList     = NEXT_UShort( pGsubHeader );

    typedef std::vector<USHORT> UshortList;
    UshortList aFeatureIndexList;
    UshortList aFeatureOffsetList;

    // parse Script Table
    const FT_Byte* pScriptHeader = pGsubBase + nOfsScriptList;
    const USHORT nCntScript = NEXT_UShort( pScriptHeader );
    for( USHORT nScriptIndex = 0; nScriptIndex < nCntScript; ++nScriptIndex )
    {
        const ULONG nTag            = NEXT_Long( pScriptHeader ); // e.g. hani/arab/kana/hang
        const USHORT nOfsScriptTable= NEXT_UShort( pScriptHeader );
        if( (nTag != nRequestedScript) && (nRequestedScript != 0) )
            continue;

        const FT_Byte* pScriptTable     = pGsubBase + nOfsScriptList + nOfsScriptTable;
        const USHORT nDefaultLangsysOfs = NEXT_UShort( pScriptTable );
        const USHORT nCntLangSystem     = NEXT_UShort( pScriptTable );
        USHORT nLangsysOffset = 0;
        for( USHORT nLangsysIndex = 0; nLangsysIndex < nCntLangSystem; ++nLangsysIndex )
        {
            const ULONG nTag    = NEXT_Long( pScriptTable );    // e.g. KOR/ZHS/ZHT/JAN
            const USHORT nOffset= NEXT_UShort( pScriptTable );
            if( (nTag != nRequestedLangsys) && (nRequestedLangsys != 0) )
                continue;
            nLangsysOffset = nOffset;
            break;
        }

        if( (nDefaultLangsysOfs != 0) && (nDefaultLangsysOfs != nLangsysOffset) )
        {
            const FT_Byte* pLangSys = pGsubBase + nOfsScriptList + nOfsScriptTable + nDefaultLangsysOfs;
            const USHORT nLookupOrder   = NEXT_UShort( pLangSys );
            const USHORT nReqFeatureIdx = NEXT_UShort( pLangSys );
            const USHORT nCntFeature    = NEXT_UShort( pLangSys );
            aFeatureIndexList.push_back( nReqFeatureIdx );
            for( USHORT i = 0; i < nCntFeature; ++i )
            {
                const USHORT nFeatureIndex = NEXT_UShort( pLangSys );
                aFeatureIndexList.push_back( nFeatureIndex );
            }
        }

        if( nLangsysOffset != 0 )
        {
            const FT_Byte* pLangSys = pGsubBase + nOfsScriptList + nOfsScriptTable + nLangsysOffset;
            const USHORT nLookupOrder   = NEXT_UShort( pLangSys );
            const USHORT nReqFeatureIdx = NEXT_UShort( pLangSys );
            const USHORT nCntFeature    = NEXT_UShort( pLangSys );
            aFeatureIndexList.push_back( nReqFeatureIdx );
            for( USHORT i = 0; i < nCntFeature; ++i )
            {
                const USHORT nFeatureIndex = NEXT_UShort( pLangSys );
                aFeatureIndexList.push_back( nFeatureIndex );
            }
        }
    }

    if( !aFeatureIndexList.size() )
        return true;

    UshortList aLookupIndexList;
    UshortList aLookupOffsetList;

    // parse Feature Table
    const FT_Byte* pFeatureHeader = pGsubBase + nOfsFeatureTable;
    const USHORT nCntFeature = NEXT_UShort( pFeatureHeader );
    for( USHORT nFeatureIndex = 0; nFeatureIndex < nCntFeature; ++nFeatureIndex )
    {
        const ULONG nTag    = NEXT_Long( pFeatureHeader ); // e.g. locl/vert/trad/smpl/liga/fina/...
        const USHORT nOffset= NEXT_UShort( pFeatureHeader );

        // feature (required && (requested || available))?
        if( (aFeatureIndexList[0] != nFeatureIndex)
        &&  (!std::count( aReqFeatureTagList.begin(), aReqFeatureTagList.end(), nTag))
        ||  (!std::count( aFeatureIndexList.begin(), aFeatureIndexList.end(), nFeatureIndex) ) )
            continue;

        const FT_Byte* pFeatureTable = pGsubBase + nOfsFeatureTable + nOffset;
        const USHORT nCntLookups = NEXT_UShort( pFeatureTable );
        for( USHORT i = 0; i < nCntLookups; ++i )
        {
            const USHORT nLookupIndex = NEXT_UShort( pFeatureTable );
            aLookupIndexList.push_back( nLookupIndex );
        }
        if( nCntLookups == 0 ) //### hack needed by Mincho/Gothic/Mingliu/Simsun/...
            aLookupIndexList.push_back( 0 );
    }

    // parse Lookup List
    const FT_Byte* pLookupHeader = pGsubBase + nOfsLookupList;
    const USHORT nCntLookupTable = NEXT_UShort( pLookupHeader );
    for( USHORT nLookupIdx = 0; nLookupIdx < nCntLookupTable; ++nLookupIdx )
    {
        const USHORT nOffset = NEXT_UShort( pLookupHeader );
        if( std::count( aLookupIndexList.begin(), aLookupIndexList.end(), nLookupIdx ) )
            aLookupOffsetList.push_back( nOffset );
    }

    UshortList::const_iterator it = aLookupOffsetList.begin();
    for(; it != aLookupOffsetList.end(); ++it )
    {
        const USHORT nOfsLookupTable = *it;
        const FT_Byte* pLookupTable = pGsubBase + nOfsLookupList + nOfsLookupTable;
        const USHORT eLookupType        = NEXT_UShort( pLookupTable );
        const USHORT eLookupFlag        = NEXT_UShort( pLookupTable );
        const USHORT nCntLookupSubtable = NEXT_UShort( pLookupTable );

        // TODO: switch( eLookupType )
        if( eLookupType != 1 )  // TODO: once we go beyond SingleSubst
            continue;

        for( USHORT nSubTableIdx = 0; nSubTableIdx < nCntLookupSubtable; ++nSubTableIdx )
        {
            const USHORT nOfsSubLookupTable = NEXT_UShort( pLookupTable );
            const FT_Byte* pSubLookup = pGsubBase + nOfsLookupList + nOfsLookupTable + nOfsSubLookupTable;


            const USHORT nFmtSubstitution   = NEXT_UShort( pSubLookup );
            const USHORT nOfsCoverage       = NEXT_UShort( pSubLookup );

            typedef std::pair<USHORT,USHORT> GlyphSubst;
            typedef std::vector<GlyphSubst> SubstVector;
            SubstVector aSubstVector;

            const FT_Byte* pCoverage    = pGsubBase + nOfsLookupList + nOfsLookupTable + nOfsSubLookupTable + nOfsCoverage;
            const USHORT nFmtCoverage   = NEXT_UShort( pCoverage );
            switch( nFmtCoverage )
            {
                case 1:         // Coverage Format 1
                    {
                        const USHORT nCntGlyph = NEXT_UShort( pCoverage );
                        aSubstVector.reserve( nCntGlyph );
                        for( USHORT i = 0; i < nCntGlyph; ++i )
                        {
                            const USHORT nGlyphId = NEXT_UShort( pCoverage );
                            aSubstVector.push_back( GlyphSubst( nGlyphId, 0 ) );
                        }
                    }
                    break;

                case 2:         // Coverage Format 2
                    {
                        const USHORT nCntRange = NEXT_UShort( pCoverage );
                        for( int i = nCntRange; --i >= 0; )
                        {
                            const USHORT nGlyph0 = NEXT_UShort( pCoverage );
                            const USHORT nGlyph1 = NEXT_UShort( pCoverage );
                            const USHORT nCovIdx = NEXT_UShort( pCoverage );
                            for( USHORT j = nGlyph0; j <= nGlyph1; ++j )
                                aSubstVector.push_back( GlyphSubst( j + nCovIdx, 0 ) );
                        }
                    }
                    break;
            }

            SubstVector::iterator it( aSubstVector.begin() );

            switch( nFmtSubstitution )
            {
                case 1:     // Single Substitution Format 1
                    {
                        const USHORT nDeltaGlyphId = NEXT_UShort( pSubLookup );
                        for(; it != aSubstVector.end(); ++it )
                            (*it).second = (*it).first + nDeltaGlyphId;
                    }
                    break;

                case 2:     // Single Substitution Format 2
                    {
                        const USHORT nCntGlyph = NEXT_UShort( pSubLookup );
                        for( int i = nCntGlyph; (it != aSubstVector.end()) && (--i>=0); ++it )
                        {
                            const USHORT nGlyphId = NEXT_UShort( pSubLookup );
                            (*it).second = nGlyphId;
                        }
                    }
                    break;
            }

            DBG_ASSERT( (it == aSubstVector.end()), "lookup<->coverage table mismatch" );
            // now apply the glyph substitutions that have been collected in this subtable
            for( it = aSubstVector.begin(); it != aSubstVector.end(); ++it )
                aGlyphSubstitution[ (*it).first ] =  (*it).second;
        }
    }

    return true;
}

// =======================================================================
