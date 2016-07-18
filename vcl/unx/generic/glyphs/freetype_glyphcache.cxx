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


#include <vcl/fontcharmap.hxx>

#include "unx/freetype_glyphcache.hxx"

#include <vcl/svapp.hxx>
#include <fontinstance.hxx>
#include <impfont.hxx>
#include "fontattributes.hxx"

#include <config_features.h>
#include <config_graphite.h>
#if ENABLE_GRAPHITE
#  include <graphite_static.hxx>
#  include <graphite2/Font.h>
#  include <graphite_layout.hxx>
#endif
#include <unotools/fontdefs.hxx>

#include "tools/poly.hxx"
#include "basegfx/matrix/b2dhommatrix.hxx"
#include "basegfx/matrix/b2dhommatrixtools.hxx"
#include "basegfx/polygon/b2dpolypolygon.hxx"

#include "osl/file.hxx"
#include "osl/thread.hxx"

#include "langboost.hxx"
#include "PhysicalFontCollection.hxx"
#include "sft.hxx"

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H
#include FT_OUTLINE_H
#include FT_SIZES_H
#include FT_SYNTHESIS_H
#include FT_TRUETYPE_TABLES_H
#include FT_TRUETYPE_TAGS_H
#include FT_TRUETYPE_IDS_H

#include "rtl/instance.hxx"

#include <vector>

// TODO: move file mapping stuff to OSL
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include "unx/fontmanager.hxx"
#include "impfontcharmap.hxx"

static FT_Library aLibFT = nullptr;

// enable linking with old FT versions
static int nFTVERSION = 0;

typedef std::unordered_map<const char*, std::shared_ptr<FreetypeFontFile>, rtl::CStringHash, rtl::CStringEqual> FontFileList;

namespace { struct vclFontFileList : public rtl::Static< FontFileList, vclFontFileList > {}; }

// TODO: remove when the priorities are selected by UI
// if (AH==0) => disable autohinting
// if (AA==0) => disable antialiasing
// if (EB==0) => disable embedded bitmaps
// if (AA prio <= AH prio) => antialias + autohint
// if (AH<AA) => do not autohint when antialiasing
// if (EB<AH) => do not autohint for monochrome
static int nDefaultPrioEmbedded    = 2;
static int nDefaultPrioAutoHint    = 1;
static int nDefaultPrioAntiAlias   = 1;

// FreetypeManager

FreetypeFontFile::FreetypeFontFile( const OString& rNativeFileName )
:   maNativeFileName( rNativeFileName ),
    mpFileMap( nullptr ),
    mnFileSize( 0 ),
    mnRefCount( 0 ),
    mnLangBoost( 0 )
{
    // boost font preference if UI language is mentioned in filename
    int nPos = maNativeFileName.lastIndexOf( '_' );
    if( nPos == -1 || maNativeFileName[nPos+1] == '.' )
        mnLangBoost += 0x1000;     // no langinfo => good
    else
    {
        static const char* pLangBoost = nullptr;
        static bool bOnce = true;
        if( bOnce )
        {
            bOnce = false;
            pLangBoost = vcl::getLangBoost();
        }

        if( pLangBoost && !strncasecmp( pLangBoost, &maNativeFileName.getStr()[nPos+1], 3 ) )
           mnLangBoost += 0x2000;     // matching langinfo => better
    }
}

FreetypeFontFile* FreetypeFontFile::FindFontFile( const OString& rNativeFileName )
{
    // font file already known? (e.g. for ttc, synthetic, aliased fonts)
    const char* pFileName = rNativeFileName.getStr();
    FontFileList &rFontFileList = vclFontFileList::get();
    FontFileList::const_iterator it = rFontFileList.find( pFileName );
    if( it != rFontFileList.end() )
        return it->second.get();

    // no => create new one
    FreetypeFontFile* pFontFile = new FreetypeFontFile( rNativeFileName );
    pFileName = pFontFile->maNativeFileName.getStr();
    rFontFileList[pFileName].reset(pFontFile);
    return pFontFile;
}

bool FreetypeFontFile::Map()
{
    if( mnRefCount++ <= 0 )
    {
        const char* pFileName = maNativeFileName.getStr();
        int nFile = open( pFileName, O_RDONLY );
        if( nFile < 0 )
            return false;

        struct stat aStat;
        int nRet = fstat( nFile, &aStat );
        if (nRet < 0)
        {
            close (nFile);
            return false;
        }
        mnFileSize = aStat.st_size;
        mpFileMap = static_cast<unsigned char*>(
            mmap( nullptr, mnFileSize, PROT_READ, MAP_SHARED, nFile, 0 ));
        if( mpFileMap == MAP_FAILED )
            mpFileMap = nullptr;
        close( nFile );
    }

    return (mpFileMap != nullptr);
}

void FreetypeFontFile::Unmap()
{
    if( (--mnRefCount > 0) || (mpFileMap == nullptr) )
        return;

    munmap( mpFileMap, mnFileSize );
    mpFileMap = nullptr;
}

#if ENABLE_GRAPHITE
// wrap FreetypeFontInfo's table function
const void * graphiteFontTable(const void* appFaceHandle, unsigned int name, size_t *len)
{
    const FreetypeFontInfo * pFontInfo = static_cast<const FreetypeFontInfo*>(appFaceHandle);
    typedef union {
        char m_c[5];
        unsigned int m_id;
    } TableId;
    TableId tableId;
    tableId.m_id = name;
#ifndef OSL_BIGENDIAN
    TableId swapped;
    swapped.m_c[3] = tableId.m_c[0];
    swapped.m_c[2] = tableId.m_c[1];
    swapped.m_c[1] = tableId.m_c[2];
    swapped.m_c[0] = tableId.m_c[3];
    tableId.m_id = swapped.m_id;
#endif
    tableId.m_c[4] = '\0';
    sal_uLong nLength = 0;
    const void * pTable = static_cast<const void*>(pFontInfo->GetTable(tableId.m_c, &nLength));
    if (len) *len = static_cast<size_t>(nLength);
    return pTable;
}
#endif

FreetypeFontInfo::FreetypeFontInfo( const FontAttributes& rDevFontAttributes,
    const OString& rNativeFileName, int nFaceNum, sal_IntPtr nFontId)
:
    maFaceFT( nullptr ),
    mpFontFile( FreetypeFontFile::FindFontFile( rNativeFileName ) ),
    mnFaceNum( nFaceNum ),
    mnRefCount( 0 ),
#if ENABLE_GRAPHITE
    mbCheckedGraphite(false),
    mpGraphiteFace(nullptr),
#endif
    mnFontId( nFontId ),
    maDevFontAttributes( rDevFontAttributes ),
    mxFontCharMap( nullptr ),
    mpChar2Glyph( nullptr ),
    mpGlyph2Char( nullptr )
{
    // prefer font with low ID
    maDevFontAttributes.IncreaseQualityBy( 10000 - nFontId );
    // prefer font with matching file names
    maDevFontAttributes.IncreaseQualityBy( mpFontFile->GetLangBoost() );
}

FreetypeFontInfo::~FreetypeFontInfo()
{
    if( mxFontCharMap )
        mxFontCharMap = nullptr;
    delete mpChar2Glyph;
    delete mpGlyph2Char;
#if ENABLE_GRAPHITE
    delete mpGraphiteFace;
#endif
}

void FreetypeFontInfo::InitHashes() const
{
    // TODO: avoid pointers when empty stl::hash_* objects become cheap
    mpChar2Glyph = new Int2IntMap();
    mpGlyph2Char = new Int2IntMap();
}

FT_FaceRec_* FreetypeFontInfo::GetFaceFT()
{
    if (!maFaceFT && mpFontFile->Map())
    {
        FT_Error rc = FT_New_Memory_Face( aLibFT,
            mpFontFile->GetBuffer(),
            mpFontFile->GetFileSize(), mnFaceNum, &maFaceFT );
        if( (rc != FT_Err_Ok) || (maFaceFT->num_glyphs <= 0) )
            maFaceFT = nullptr;
    }

   ++mnRefCount;
   return maFaceFT;
}

#if ENABLE_GRAPHITE
GraphiteFaceWrapper * FreetypeFontInfo::GetGraphiteFace()
{
    if (mbCheckedGraphite)
        return mpGraphiteFace;
    // test for graphite here so that it is cached most efficiently
    if (GetTable("Silf"))
    {
        static const char* pGraphiteCacheStr = getenv( "SAL_GRAPHITE_CACHE_SIZE" );
        int graphiteSegCacheSize = pGraphiteCacheStr ? (atoi(pGraphiteCacheStr)) : 0;
        gr_face * pGraphiteFace;
        if (graphiteSegCacheSize > 500)
            pGraphiteFace = gr_make_face_with_seg_cache(this, graphiteFontTable, graphiteSegCacheSize, gr_face_cacheCmap);
        else
            pGraphiteFace = gr_make_face(this, graphiteFontTable, gr_face_cacheCmap);
        if (pGraphiteFace)
            mpGraphiteFace = new GraphiteFaceWrapper(pGraphiteFace);
    }
    mbCheckedGraphite = true;
    return mpGraphiteFace;
}
#endif

void FreetypeFontInfo::ReleaseFaceFT()
{
    if (--mnRefCount <= 0)
    {
        FT_Done_Face( maFaceFT );
        maFaceFT = nullptr;
        mpFontFile->Unmap();
    }
}

static unsigned GetUInt( const unsigned char* p ) { return((p[0]<<24)+(p[1]<<16)+(p[2]<<8)+p[3]);}
static unsigned GetUShort( const unsigned char* p ){ return((p[0]<<8)+p[1]);}
//static signed GetSShort( const unsigned char* p ){ return((short)((p[0]<<8)+p[1]));}

static const sal_uInt32 T_true = 0x74727565;        /* 'true' */
static const sal_uInt32 T_ttcf = 0x74746366;        /* 'ttcf' */
static const sal_uInt32 T_otto = 0x4f54544f;        /* 'OTTO' */

const unsigned char* FreetypeFontInfo::GetTable( const char* pTag, sal_uLong* pLength ) const
{
    const unsigned char* pBuffer = mpFontFile->GetBuffer();
    int nFileSize = mpFontFile->GetFileSize();
    if( !pBuffer || nFileSize<1024 )
        return nullptr;

    // we currently handle TTF, TTC and OTF headers
    unsigned nFormat = GetUInt( pBuffer );

    const unsigned char* p = pBuffer + 12;
    if( nFormat == T_ttcf )         // TTC_MAGIC
        p += GetUInt( p + 4 * mnFaceNum );
    else if( nFormat != 0x00010000 && nFormat != T_true && nFormat != T_otto) // TTF_MAGIC and Apple TTF Magic and PS-OpenType font
        return nullptr;

    // walk table directory until match
    int nTables = GetUShort( p - 8 );
    if( nTables >= 64 )  // something fishy?
        return nullptr;
    for( int i = 0; i < nTables; ++i, p+=16 )
    {
        if( p[0]==pTag[0] && p[1]==pTag[1] && p[2]==pTag[2] && p[3]==pTag[3] )
        {
            sal_uLong nLength = GetUInt( p + 12 );
            if( pLength != nullptr )
                *pLength = nLength;
            const unsigned char* pTable = pBuffer + GetUInt( p + 8 );
            if( (pTable + nLength) <= (mpFontFile->GetBuffer() + nFileSize) )
                return pTable;
        }
    }

    return nullptr;
}

void FreetypeFontInfo::AnnounceFont( PhysicalFontCollection* pFontCollection )
{
    FreetypeFontFace* pFD = new FreetypeFontFace( this, maDevFontAttributes );
    pFontCollection->Add( pFD );
}

FreetypeManager::FreetypeManager()
:   mnMaxFontId( 0 )
{
    /*FT_Error rcFT =*/ FT_Init_FreeType( &aLibFT );

    FT_Int nMajor = 0, nMinor = 0, nPatch = 0;
    FT_Library_Version(aLibFT, &nMajor, &nMinor, &nPatch);
    nFTVERSION = nMajor * 1000 + nMinor * 100 + nPatch;

    // TODO: remove when the priorities are selected by UI
    char* pEnv;
    pEnv = ::getenv( "SAL_EMBEDDED_BITMAP_PRIORITY" );
    if( pEnv )
        nDefaultPrioEmbedded  = pEnv[0] - '0';
    pEnv = ::getenv( "SAL_ANTIALIASED_TEXT_PRIORITY" );
    if( pEnv )
        nDefaultPrioAntiAlias = pEnv[0] - '0';
    pEnv = ::getenv( "SAL_AUTOHINTING_PRIORITY" );
    if( pEnv )
        nDefaultPrioAutoHint  = pEnv[0] - '0';

    vclFontFileList::get();
}

FT_Face ServerFont::GetFtFace() const
{
    FT_Activate_Size( maSizeFT );

    return maFaceFT;
}

FreetypeManager::~FreetypeManager()
{
    ClearFontList();
}

void FreetypeManager::AddFontFile( const OString& rNormalizedName,
    int nFaceNum, sal_IntPtr nFontId, const FontAttributes& rDevFontAttr)
{
    if( rNormalizedName.isEmpty() )
        return;

    if( maFontList.find( nFontId ) != maFontList.end() )
        return;

    FreetypeFontInfo* pFontInfo = new FreetypeFontInfo( rDevFontAttr,
        rNormalizedName, nFaceNum, nFontId);
    maFontList[ nFontId ] = pFontInfo;
    if( mnMaxFontId < nFontId )
        mnMaxFontId = nFontId;
}

void FreetypeManager::AnnounceFonts( PhysicalFontCollection* pToAdd ) const
{
    for( FontList::const_iterator it = maFontList.begin(); it != maFontList.end(); ++it )
    {
        FreetypeFontInfo* pFreetypeFontInfo = it->second;
        pFreetypeFontInfo->AnnounceFont( pToAdd );
    }
}

void FreetypeManager::ClearFontList( )
{
    for( FontList::iterator it = maFontList.begin(); it != maFontList.end(); ++it )
    {
        FreetypeFontInfo* pFreetypeFontInfo = it->second;
        delete pFreetypeFontInfo;
    }
    maFontList.clear();
}

ServerFont* FreetypeManager::CreateFont( const FontSelectPattern& rFSD )
{
    FreetypeFontInfo* pFontInfo = nullptr;

    // find a FontInfo matching to the font id
    sal_IntPtr nFontId = reinterpret_cast<sal_IntPtr>( rFSD.mpFontData );
    FontList::iterator it = maFontList.find( nFontId );
    if( it != maFontList.end() )
        pFontInfo = it->second;

    if( !pFontInfo )
        return nullptr;

    ServerFont* pNew = new ServerFont( rFSD, pFontInfo );

    return pNew;
}

FreetypeFontFace::FreetypeFontFace( FreetypeFontInfo* pFI, const FontAttributes& rDFA )
:   PhysicalFontFace( rDFA ),
    mpFreetypeFontInfo( pFI )
{
    SetBuiltInFontFlag( false );
    SetOrientationFlag( true );
}

LogicalFontInstance* FreetypeFontFace::CreateFontInstance( FontSelectPattern& rFSD ) const
{
    ServerFontInstance* pEntry = new ServerFontInstance( rFSD );
    return pEntry;
}

// ServerFont

ServerFont::ServerFont( const FontSelectPattern& rFSD, FreetypeFontInfo* pFI )
:   maGlyphList( 0),
    maFontSelData(rFSD),
    mnRefCount(1),
    mnBytesUsed( sizeof(ServerFont) ),
    mpPrevGCFont( nullptr ),
    mpNextGCFont( nullptr ),
    mnCos( 0x10000),
    mnSin( 0 ),
    mnPrioEmbedded(nDefaultPrioEmbedded),
    mnPrioAntiAlias(nDefaultPrioAntiAlias),
    mnPrioAutoHint(nDefaultPrioAutoHint),
    mpFontInfo( pFI ),
    mnLoadFlags( 0 ),
    maFaceFT( nullptr ),
    maSizeFT( nullptr ),
    mbFaceOk( false ),
    mbArtItalic( false ),
    mbArtBold( false ),
    mbUseGamma( false ),
    mpLayoutEngine( nullptr )
{
    // TODO: move update of mpFontInstance into FontEntry class when
    // it becomes responsible for the ServerFont instantiation
    static_cast<ServerFontInstance*>(rFSD.mpFontInstance)->SetServerFont( this );

    maFaceFT = pFI->GetFaceFT();

    if( rFSD.mnOrientation != 0 )
    {
        const double dRad = rFSD.mnOrientation * ( F_2PI / 3600.0 );
        mnCos = static_cast<long>( 0x10000 * cos( dRad ) + 0.5 );
        mnSin = static_cast<long>( 0x10000 * sin( dRad ) + 0.5 );
    }

    // set the pixel size of the font instance
    mnWidth = rFSD.mnWidth;
    if( !mnWidth )
        mnWidth = rFSD.mnHeight;
    mfStretch = (double)mnWidth / rFSD.mnHeight;
    // sanity check (e.g. #i66394#, #i66244#, #66537#)
    if( (mnWidth < 0) || (mfStretch > +64.0) || (mfStretch < -64.0) )
        return;

    if( !maFaceFT )
        return;

    FT_New_Size( maFaceFT, &maSizeFT );
    FT_Activate_Size( maSizeFT );
    FT_Error rc = FT_Set_Pixel_Sizes( maFaceFT, mnWidth, rFSD.mnHeight );
    if( rc != FT_Err_Ok )
        return;

    FT_Select_Charmap(maFaceFT, FT_ENCODING_UNICODE);

    if( mpFontInfo->IsSymbolFont() )
    {
        FT_Encoding eEncoding = FT_ENCODING_MS_SYMBOL;
        if (!FT_IS_SFNT(maFaceFT))
            eEncoding = FT_ENCODING_ADOBE_CUSTOM; // freetype wants this for PS symbol fonts

        FT_Select_Charmap(maFaceFT, eEncoding);
    }

    mbFaceOk = true;

    ApplyGSUB( rFSD );

    // TODO: query GASP table for load flags
    mnLoadFlags = FT_LOAD_DEFAULT;
#if 1 // #i97326# cairo sometimes uses FT_Set_Transform() on our FT_FACE
    // we are not using FT_Set_Transform() yet, so just ignore it for now
    mnLoadFlags |= FT_LOAD_IGNORE_TRANSFORM;
#endif

    mbArtItalic = (rFSD.GetItalic() != ITALIC_NONE && pFI->GetFontAttributes().GetItalic() == ITALIC_NONE);
    mbArtBold = (rFSD.GetWeight() > WEIGHT_MEDIUM && pFI->GetFontAttributes().GetWeight() <= WEIGHT_MEDIUM);
    if( mbArtBold )
    {
        //static const int TT_CODEPAGE_RANGE_874  = (1L << 16); // Thai
        //static const int TT_CODEPAGE_RANGE_932  = (1L << 17); // JIS/Japan
        //static const int TT_CODEPAGE_RANGE_936  = (1L << 18); // Chinese: Simplified
        //static const int TT_CODEPAGE_RANGE_949  = (1L << 19); // Korean Wansung
        //static const int TT_CODEPAGE_RANGE_950  = (1L << 20); // Chinese: Traditional
        //static const int TT_CODEPAGE_RANGE_1361 = (1L << 21); // Korean Johab
        static const int TT_CODEPAGE_RANGES1_CJKT = 0x3F0000; // all of the above
        const TT_OS2* pOs2 = static_cast<const TT_OS2*>(FT_Get_Sfnt_Table( maFaceFT, ft_sfnt_os2 ));
        if ((pOs2) && (pOs2->ulCodePageRange1 & TT_CODEPAGE_RANGES1_CJKT )
        && rFSD.mnHeight < 20)
        mbUseGamma = true;
    }

    if( ((mnCos != 0) && (mnSin != 0)) || (mnPrioEmbedded <= 0) )
        mnLoadFlags |= FT_LOAD_NO_BITMAP;
}

void ServerFont::SetFontOptions(const std::shared_ptr<FontConfigFontOptions>& xFontOptions)
{
    mxFontOptions = xFontOptions;

    if (!mxFontOptions)
        return;

    FontAutoHint eHint = mxFontOptions->GetUseAutoHint();
    if( eHint == AUTOHINT_DONTKNOW )
        eHint = mbUseGamma ? AUTOHINT_TRUE : AUTOHINT_FALSE;

    if( eHint == AUTOHINT_TRUE )
        mnLoadFlags |= FT_LOAD_FORCE_AUTOHINT;

    if( (mnSin != 0) && (mnCos != 0) ) // hinting for 0/90/180/270 degrees only
        mnLoadFlags |= FT_LOAD_NO_HINTING;
    mnLoadFlags |= FT_LOAD_IGNORE_GLOBAL_ADVANCE_WIDTH;

    if (mxFontOptions->DontUseAntiAlias())
      mnPrioAntiAlias = 0;
    if (mxFontOptions->DontUseEmbeddedBitmaps())
      mnPrioEmbedded = 0;
    if (mxFontOptions->DontUseHinting())
      mnPrioAutoHint = 0;

    if( mnPrioAutoHint <= 0 )
        mnLoadFlags |= FT_LOAD_NO_HINTING;

#if defined(FT_LOAD_TARGET_LIGHT) && defined(FT_LOAD_TARGET_NORMAL)
    if( !(mnLoadFlags & FT_LOAD_NO_HINTING) )
    {
       mnLoadFlags |= FT_LOAD_TARGET_NORMAL;
       switch (mxFontOptions->GetHintStyle())
       {
           case FontHintStyle::NONE:
                mnLoadFlags |= FT_LOAD_NO_HINTING;
                break;
           case FontHintStyle::Slight:
                mnLoadFlags |= FT_LOAD_TARGET_LIGHT;
                break;
           case FontHintStyle::Medium:
                break;
           case FontHintStyle::Full:
           default:
                break;
       }
    }
#endif

    if( mnPrioEmbedded <= 0 )
        mnLoadFlags |= FT_LOAD_NO_BITMAP;
}

const std::shared_ptr<FontConfigFontOptions>& ServerFont::GetFontOptions() const
{
    return mxFontOptions;
}

const OString& ServerFont::GetFontFileName() const
{
    return mpFontInfo->GetFontFileName();
}


ServerFont::~ServerFont()
{
    delete mpLayoutEngine;

    if( maSizeFT )
        FT_Done_Size( maSizeFT );

    mpFontInfo->ReleaseFaceFT();

    ReleaseFromGarbageCollect();
}


void ServerFont::GetFontMetric( ImplFontMetricDataPtr& rxTo, long& rFactor ) const
{
    rxTo->FontAttributes::operator =(mpFontInfo->GetFontAttributes());

    rxTo->SetScalableFlag( true ); // FIXME: Shouldn't this check FT_IS_SCALABLE( maFaceFT )?
    rxTo->SetTrueTypeFlag( FT_IS_SFNT( maFaceFT ) != 0 );
    rxTo->SetBuiltInFontFlag( true );
    rxTo->SetKernableFlag( FT_HAS_KERNING( maFaceFT ) != 0 );
    rxTo->SetOrientation( GetFontSelData().mnOrientation );

    //Always consider [star]symbol as symbol fonts
    if ( IsStarSymbol( rxTo->GetFamilyName() ) )
        rxTo->SetSymbolFlag( true );

    FT_Activate_Size( maSizeFT );

    rFactor = 0x100;

    const TT_OS2* pOS2 = static_cast<const TT_OS2*>(FT_Get_Sfnt_Table( maFaceFT, ft_sfnt_os2 ));
    const double fScale = (double)GetFontSelData().mnHeight / maFaceFT->units_per_EM;

    rxTo->SetAscent( 0 );
    rxTo->SetDescent( 0 );
    rxTo->SetExternalLeading( 0 );
    rxTo->SetSlant( 0 );
    rxTo->SetWidth( mnWidth );

    // Calculating ascender and descender:
    // FreeType >= 2.4.6 does the right thing, so we just use what it gives us,
    // for earlier versions we emulate its behaviour;
    // take them from 'hhea' table,
    // if zero take them from 'OS/2' table,
    // if zero take them from FreeType's font metrics
    if (nFTVERSION >= 2406)
    {
        const FT_Size_Metrics& rMetrics = maFaceFT->size->metrics;
        rxTo->SetAscent( (rMetrics.ascender + 32) >> 6 );
        rxTo->SetDescent( (-rMetrics.descender + 32) >> 6 );
        rxTo->SetExternalLeading( ((rMetrics.height + 32) >> 6) - (rxTo->GetAscent() + rxTo->GetDescent()) );
    }
    else
    {
        const TT_HoriHeader* pHHea = static_cast<const TT_HoriHeader*>(FT_Get_Sfnt_Table(maFaceFT, ft_sfnt_hhea));
        if (pHHea)
        {
            rxTo->SetAscent( pHHea->Ascender * fScale + 0.5 );
            rxTo->SetDescent( -pHHea->Descender * fScale + 0.5 );
            rxTo->SetExternalLeading( pHHea->Line_Gap * fScale + 0.5 );
        }

        if (!(rxTo->GetAscent() || rxTo->GetDescent()))
        {
            if (pOS2 && (pOS2->version != 0xFFFF))
            {
                if (pOS2->sTypoAscender || pOS2->sTypoDescender)
                {
                    rxTo->SetAscent( pOS2->sTypoAscender * fScale + 0.5 );
                    rxTo->SetDescent( -pOS2->sTypoDescender * fScale + 0.5 );
                    rxTo->SetExternalLeading( pOS2->sTypoLineGap * fScale + 0.5 );
                }
                else
                {
                    rxTo->SetAscent( pOS2->usWinAscent * fScale + 0.5 );
                    rxTo->SetDescent( pOS2->usWinDescent * fScale + 0.5 );
                    rxTo->SetExternalLeading( 0 );
                }
            }
        }

        if (!(rxTo->GetAscent() || rxTo->GetDescent()))
        {
            const FT_Size_Metrics& rMetrics = maFaceFT->size->metrics;
            rxTo->SetAscent( (rMetrics.ascender + 32) >> 6 );
            rxTo->SetDescent( (-rMetrics.descender + 32) >> 6 );
            rxTo->SetExternalLeading( ((rMetrics.height + 32) >> 6) - (rxTo->GetAscent() + rxTo->GetDescent() ) );
        }
    }

    rxTo->SetInternalLeading( rxTo->GetAscent() + rxTo->GetDescent() - (maFaceFT->units_per_EM * fScale + 0.5) );

    if( pOS2 && (pOS2->version != 0xFFFF) )
    {
        // map the panose info from the OS2 table to their VCL counterparts
        switch( pOS2->panose[0] )
        {
            case 1: rxTo->SetFamilyType( FAMILY_ROMAN ); break;
            case 2: rxTo->SetFamilyType( FAMILY_SWISS ); break;
            case 3: rxTo->SetFamilyType( FAMILY_MODERN ); break;
            case 4: rxTo->SetFamilyType( FAMILY_SCRIPT ); break;
            case 5: rxTo->SetFamilyType( FAMILY_DECORATIVE ); break;
            // TODO: is it reasonable to override the attribute with DONTKNOW?
            case 0: // fall through
            default: rxTo->SetFamilyType( FAMILY_DONTKNOW ); break;
        }

        switch( pOS2->panose[3] )
        {
            case 2: // fall through
            case 3: // fall through
            case 4: // fall through
            case 5: // fall through
            case 6: // fall through
            case 7: // fall through
            case 8: rxTo->SetPitch( PITCH_VARIABLE ); break;
            case 9: rxTo->SetPitch( PITCH_FIXED ); break;
            // TODO: is it reasonable to override the attribute with DONTKNOW?
            case 0: // fall through
            case 1: // fall through
            default: rxTo->SetPitch( PITCH_DONTKNOW ); break;
        }
    }

    // initialize kashida width
    // TODO: what if there are different versions of this glyph available
    const int nKashidaGlyphId = GetRawGlyphIndex( 0x0640 );
    if( nKashidaGlyphId )
    {
        GlyphData aGlyphData;
        InitGlyphData( nKashidaGlyphId, aGlyphData );
        rxTo->SetMinKashida( aGlyphData.GetMetric().GetCharWidth() );
    }

}

static inline void SplitGlyphFlags( const ServerFont& rFont, sal_GlyphId& rGlyphId, int& nGlyphFlags )
{
    nGlyphFlags = rGlyphId & GF_FLAGMASK;
    rGlyphId &= GF_IDXMASK;

    if( rGlyphId & GF_ISCHAR )
        rGlyphId = rFont.GetRawGlyphIndex( rGlyphId );
}

void ServerFont::ApplyGlyphTransform( int nGlyphFlags,
    FT_Glyph pGlyphFT, bool bForBitmapProcessing ) const
{
    int nAngle = GetFontSelData().mnOrientation;
    // shortcut most common case
    if( !nAngle && !nGlyphFlags )
        return;

    const FT_Size_Metrics& rMetrics = maFaceFT->size->metrics;
    FT_Vector aVector;
    FT_Matrix aMatrix;

    bool bStretched = false;

    switch( nGlyphFlags & GF_ROTMASK )
    {
    default:    // straight
        aVector.x = 0;
        aVector.y = 0;
        aMatrix.xx = +mnCos;
        aMatrix.yy = +mnCos;
        aMatrix.xy = -mnSin;
        aMatrix.yx = +mnSin;
        break;
    case GF_ROTL:    // left
        nAngle += 900;
        bStretched = (mfStretch != 1.0);
        aVector.x  = (FT_Pos)(+rMetrics.descender * mfStretch);
        aVector.y  = -rMetrics.ascender;
        aMatrix.xx = (FT_Pos)(-mnSin / mfStretch);
        aMatrix.yy = (FT_Pos)(-mnSin * mfStretch);
        aMatrix.xy = (FT_Pos)(-mnCos * mfStretch);
        aMatrix.yx = (FT_Pos)(+mnCos / mfStretch);
        break;
    case GF_ROTR:    // right
        nAngle -= 900;
        bStretched = (mfStretch != 1.0);
        aVector.x = -maFaceFT->glyph->metrics.horiAdvance;
        aVector.x += (FT_Pos)(rMetrics.descender * mnSin/65536.0);
        aVector.y  = (FT_Pos)(-rMetrics.descender * mfStretch * mnCos/65536.0);
        aMatrix.xx = (FT_Pos)(+mnSin / mfStretch);
        aMatrix.yy = (FT_Pos)(+mnSin * mfStretch);
        aMatrix.xy = (FT_Pos)(+mnCos * mfStretch);
        aMatrix.yx = (FT_Pos)(-mnCos / mfStretch);
        break;
    }

    while( nAngle < 0 )
        nAngle += 3600;

    if( pGlyphFT->format != FT_GLYPH_FORMAT_BITMAP )
    {
        FT_Glyph_Transform( pGlyphFT, nullptr, &aVector );

        // orthogonal transforms are better handled by bitmap operations
        if( bStretched || (bForBitmapProcessing && (nAngle % 900) != 0) )
        {
            // apply non-orthogonal or stretch transformations
            FT_Glyph_Transform( pGlyphFT, &aMatrix, nullptr );
        }
    }
    else
    {
        // FT<=2005 ignores transforms for bitmaps, so do it manually
        FT_BitmapGlyph pBmpGlyphFT = reinterpret_cast<FT_BitmapGlyph>(pGlyphFT);
        pBmpGlyphFT->left += (aVector.x + 32) >> 6;
        pBmpGlyphFT->top  += (aVector.y + 32) >> 6;
    }
}

sal_GlyphId ServerFont::GetRawGlyphIndex(sal_UCS4 aChar, sal_UCS4 aVS) const
{
    if( mpFontInfo->IsSymbolFont() )
    {
        if( !FT_IS_SFNT( maFaceFT ) )
        {
            if( (aChar & 0xFF00) == 0xF000 )
                aChar &= 0xFF;    // PS font symbol mapping
            else if( aChar > 0xFF )
                return 0;
        }
    }

    int nGlyphIndex = 0;
#if HAVE_FT_FACE_GETCHARVARIANTINDEX
    // If asked, check first for variant glyph with the given Unicode variation
    // selector. This is quite uncommon so we don't bother with caching here.
    // Disabled for buggy FreeType versions:
    // https://bugzilla.mozilla.org/show_bug.cgi?id=618406#c8
    if (aVS && nFTVERSION >= 2404)
        nGlyphIndex = FT_Face_GetCharVariantIndex(maFaceFT, aChar, aVS);
#endif

    if (nGlyphIndex == 0)
    {
        // cache glyph indexes in font info to share between different sizes
        nGlyphIndex = mpFontInfo->GetGlyphIndex( aChar );
        if( nGlyphIndex < 0 )
        {
            nGlyphIndex = FT_Get_Char_Index( maFaceFT, aChar );
            if( !nGlyphIndex)
            {
                // check if symbol aliasing helps
                if( (aChar <= 0x00FF) && mpFontInfo->IsSymbolFont() )
                    nGlyphIndex = FT_Get_Char_Index( maFaceFT, aChar | 0xF000 );
            }
            mpFontInfo->CacheGlyphIndex( aChar, nGlyphIndex );
        }
    }

    return sal_GlyphId( nGlyphIndex);
}

sal_GlyphId ServerFont::FixupGlyphIndex( sal_GlyphId aGlyphId, sal_UCS4 aChar ) const
{
    int nGlyphFlags = GF_NONE;

    // do glyph substitution if necessary
    // CJK vertical writing needs special treatment
    if( GetFontSelData().mbVertical )
    {
        // TODO: rethink when GSUB is used for non-vertical case
        GlyphSubstitution::const_iterator it = maGlyphSubstitution.find( aGlyphId );
        if( it == maGlyphSubstitution.end() )
        {
            nGlyphFlags |= GetVerticalFlags( aChar );
        }
        else
        {
            // for vertical GSUB also compensate for nOrientation=2700
            aGlyphId = (*it).second;
            nGlyphFlags |= GF_GSUB | GF_ROTL;
        }
    }

    if( aGlyphId != 0 )
        aGlyphId |= nGlyphFlags;

    return aGlyphId;
}

sal_GlyphId ServerFont::GetGlyphIndex( sal_UCS4 aChar ) const
{
    sal_GlyphId aGlyphId = GetRawGlyphIndex( aChar );
    aGlyphId = FixupGlyphIndex( aGlyphId, aChar );
    return aGlyphId;
}

static int lcl_GetCharWidth( FT_FaceRec_* pFaceFT, double fStretch, int nGlyphFlags )
{
    int nCharWidth = pFaceFT->glyph->metrics.horiAdvance;

    if( nGlyphFlags & GF_ROTMASK )  // for bVertical rotated glyphs
    {
        const FT_Size_Metrics& rMetrics = pFaceFT->size->metrics;
        nCharWidth = (int)((rMetrics.height + rMetrics.descender) * fStretch);
    }

    return (nCharWidth + 32) >> 6;
}

void ServerFont::InitGlyphData( sal_GlyphId aGlyphId, GlyphData& rGD ) const
{
    FT_Activate_Size( maSizeFT );

    int nGlyphFlags;
    SplitGlyphFlags( *this, aGlyphId, nGlyphFlags );

    int nLoadFlags = mnLoadFlags;

//  if( mbArtItalic )
//      nLoadFlags |= FT_LOAD_NO_BITMAP;

    FT_Error rc = FT_Load_Glyph( maFaceFT, aGlyphId, nLoadFlags );

    if( rc != FT_Err_Ok )
    {
        // we get here e.g. when a PS font lacks the default glyph
        rGD.SetCharWidth( 0 );
        rGD.SetDelta( 0, 0 );
        rGD.SetOffset( 0, 0 );
        rGD.SetSize( Size( 0, 0 ) );
        return;
    }

    const bool bOriginallyZeroWidth = (maFaceFT->glyph->metrics.horiAdvance == 0);
    if (mbArtBold)
        FT_GlyphSlot_Embolden(maFaceFT->glyph);

    const int nCharWidth = bOriginallyZeroWidth ? 0 : lcl_GetCharWidth( maFaceFT, mfStretch, nGlyphFlags );
    rGD.SetCharWidth( nCharWidth );

    FT_Glyph pGlyphFT;
    FT_Get_Glyph( maFaceFT->glyph, &pGlyphFT );

    ApplyGlyphTransform( nGlyphFlags, pGlyphFT, false );
    rGD.SetDelta( (pGlyphFT->advance.x + 0x8000) >> 16, -((pGlyphFT->advance.y + 0x8000) >> 16) );

    FT_BBox aBbox;
    FT_Glyph_Get_CBox( pGlyphFT, FT_GLYPH_BBOX_PIXELS, &aBbox );
    if( aBbox.yMin > aBbox.yMax )   // circumvent freetype bug
    {
        int t=aBbox.yMin;
        aBbox.yMin=aBbox.yMax;
        aBbox.yMax=t;
    }

    rGD.SetOffset( aBbox.xMin, -aBbox.yMax );
    rGD.SetSize( Size( (aBbox.xMax-aBbox.xMin+1), (aBbox.yMax-aBbox.yMin) ) );

    FT_Done_Glyph( pGlyphFT );
}

bool ServerFont::GetAntialiasAdvice() const
{
    if( GetFontSelData().mbNonAntialiased || (mnPrioAntiAlias<=0) )
        return false;
    bool bAdviseAA = true;
    // TODO: also use GASP info
    return bAdviseAA;
}

// determine unicode ranges in font

const FontCharMapPtr ServerFont::GetFontCharMap() const
{
    const FontCharMapPtr xFCMap = mpFontInfo->GetFontCharMap();
    return xFCMap;
}

const FontCharMapPtr& FreetypeFontInfo::GetFontCharMap()
{
    // check if the charmap is already cached
    if( mxFontCharMap )
        return mxFontCharMap;

    // get the charmap and cache it
    CmapResult aCmapResult;
    bool bOK = GetFontCodeRanges( aCmapResult );
    if( bOK )
    {
        FontCharMapPtr xFontCharMap( new FontCharMap ( aCmapResult ) );
        mxFontCharMap = xFontCharMap;
    }
    else
    {
        FontCharMapPtr xFontCharMap( new FontCharMap() );
        mxFontCharMap = xFontCharMap;
    }
    // mxFontCharMap on either branch now has a refcount of 1
    return mxFontCharMap;
}

// TODO: merge into method GetFontCharMap()
bool FreetypeFontInfo::GetFontCodeRanges( CmapResult& rResult ) const
{
    rResult.mbSymbolic = IsSymbolFont();

    // TODO: is the full CmapResult needed on platforms calling this?
    if( FT_IS_SFNT( maFaceFT ) )
    {
        sal_uLong nLength = 0;
        const unsigned char* pCmap = GetTable( "cmap", &nLength );
        if( pCmap && (nLength > 0) )
            if( ParseCMAP( pCmap, nLength, rResult ) )
                return true;
    }

    typedef std::vector<sal_uInt32> U32Vector;
    U32Vector aCodes;

    // FT's coverage is available since FT>=2.1.0 (OOo-baseline>=2.1.4 => ok)
    aCodes.reserve( 0x1000 );
    FT_UInt nGlyphIndex;
    for( sal_uInt32 cCode = FT_Get_First_Char( maFaceFT, &nGlyphIndex );; )
    {
        if( !nGlyphIndex )
            break;
        aCodes.push_back( cCode );  // first code inside range
        sal_uInt32 cNext;
        do cNext = FT_Get_Next_Char( maFaceFT, cCode, &nGlyphIndex ); while( cNext == ++cCode );
        aCodes.push_back( cCode );  // first code outside range
        cCode = cNext;
    }

    const int nCount = aCodes.size();
    if( !nCount) {
        if( !rResult.mbSymbolic )
            return false;

        // we usually get here for Type1 symbol fonts
        aCodes.push_back( 0xF020 );
        aCodes.push_back( 0xF100 );
    }

    sal_uInt32* pCodes = new sal_uInt32[ nCount ];
    for( int i = 0; i < nCount; ++i )
        pCodes[i] = aCodes[i];
    rResult.mpRangeCodes = pCodes;
    rResult.mnRangeCount = nCount / 2;
    return true;
}

bool ServerFont::GetFontCapabilities(vcl::FontCapabilities &rFontCapabilities) const
{
    bool bRet = false;

    sal_uLong nLength = 0;
    // load GSUB table
    const FT_Byte* pGSUB = mpFontInfo->GetTable("GSUB", &nLength);
    if (pGSUB)
        vcl::getTTScripts(rFontCapabilities.maGSUBScriptTags, pGSUB, nLength);

    // load OS/2 table
    const FT_Byte* pOS2 = mpFontInfo->GetTable("OS/2", &nLength);
    if (pOS2)
    {
        bRet = vcl::getTTCoverage(
            rFontCapabilities.oUnicodeRange,
            rFontCapabilities.oCodePageRange,
            pOS2, nLength);
    }

    return bRet;
}

// outline stuff

class PolyArgs
{
public:
                PolyArgs( tools::PolyPolygon& rPolyPoly, sal_uInt16 nMaxPoints );
                ~PolyArgs();

    void        AddPoint( long nX, long nY, PolyFlags);
    void        ClosePolygon();

    long        GetPosX() const { return maPosition.x;}
    long        GetPosY() const { return maPosition.y;}

private:
    tools::PolyPolygon& mrPolyPoly;

    Point*      mpPointAry;
    sal_uInt8*       mpFlagAry;

    FT_Vector   maPosition;
    sal_uInt16      mnMaxPoints;
    sal_uInt16      mnPoints;
    sal_uInt16      mnPoly;
    bool        bHasOffline;
};

PolyArgs::PolyArgs( tools::PolyPolygon& rPolyPoly, sal_uInt16 nMaxPoints )
:   mrPolyPoly(rPolyPoly),
    mnMaxPoints(nMaxPoints),
    mnPoints(0),
    mnPoly(0),
    bHasOffline(false)
{
    mpPointAry  = new Point[ mnMaxPoints ];
    mpFlagAry   = new sal_uInt8 [ mnMaxPoints ];
    maPosition.x = maPosition.y = 0;
}

PolyArgs::~PolyArgs()
{
    delete[] mpFlagAry;
    delete[] mpPointAry;
}

void PolyArgs::AddPoint( long nX, long nY, PolyFlags aFlag )
{
    SAL_WARN_IF( (mnPoints >= mnMaxPoints), "vcl", "FTGlyphOutline: AddPoint overflow!" );
    if( mnPoints >= mnMaxPoints )
        return;

    maPosition.x = nX;
    maPosition.y = nY;
    mpPointAry[ mnPoints ] = Point( nX, nY );
    mpFlagAry[ mnPoints++ ]= aFlag;
    bHasOffline |= (aFlag != POLY_NORMAL);
}

void PolyArgs::ClosePolygon()
{
    if( !mnPoly++ )
        return;

    // freetype seems to always close the polygon with an ON_CURVE point
    // PolyPoly wants to close the polygon itself => remove last point
    SAL_WARN_IF( (mnPoints < 2), "vcl", "FTGlyphOutline: PolyFinishNum failed!" );
    --mnPoints;
    SAL_WARN_IF( (mpPointAry[0]!=mpPointAry[mnPoints]), "vcl", "FTGlyphOutline: PolyFinishEq failed!" );
    SAL_WARN_IF( (mpFlagAry[0]!=POLY_NORMAL), "vcl", "FTGlyphOutline: PolyFinishFE failed!" );
    SAL_WARN_IF( (mpFlagAry[mnPoints]!=POLY_NORMAL), "vcl", "FTGlyphOutline: PolyFinishFS failed!" );

    tools::Polygon aPoly( mnPoints, mpPointAry, (bHasOffline ? mpFlagAry : nullptr) );

    // #i35928#
    // This may be a invalid polygons, e.g. the last point is a control point.
    // So close the polygon (and add the first point again) if the last point
    // is a control point or different from first.
    // #i48298#
    // Now really duplicating the first point, to close or correct the
    // polygon. Also no longer duplicating the flags, but enforcing
    // POLY_NORMAL for the newly added last point.
    const sal_uInt16 nPolySize(aPoly.GetSize());
    if(nPolySize)
    {
        if((aPoly.HasFlags() && POLY_CONTROL == aPoly.GetFlags(nPolySize - 1))
            || (aPoly.GetPoint(nPolySize - 1) != aPoly.GetPoint(0)))
        {
            aPoly.SetSize(nPolySize + 1);
            aPoly.SetPoint(aPoly.GetPoint(0), nPolySize);

            if(aPoly.HasFlags())
            {
                aPoly.SetFlags(nPolySize, POLY_NORMAL);
            }
        }
    }

    mrPolyPoly.Insert( aPoly );
    mnPoints = 0;
    bHasOffline = false;
}

extern "C" {

// TODO: wait till all compilers accept that calling conventions
// for functions are the same independent of implementation constness,
// then uncomment the const-tokens in the function interfaces below
static int FT_move_to( const FT_Vector* p0, void* vpPolyArgs )
{
    PolyArgs& rA = *static_cast<PolyArgs*>(vpPolyArgs);

    // move_to implies a new polygon => finish old polygon first
    rA.ClosePolygon();

    rA.AddPoint( p0->x, p0->y, POLY_NORMAL );
    return 0;
}

static int FT_line_to( const FT_Vector* p1, void* vpPolyArgs )
{
    PolyArgs& rA = *static_cast<PolyArgs*>(vpPolyArgs);
    rA.AddPoint( p1->x, p1->y, POLY_NORMAL );
    return 0;
}

static int FT_conic_to( const FT_Vector* p1, const FT_Vector* p2, void* vpPolyArgs )
{
    PolyArgs& rA = *static_cast<PolyArgs*>(vpPolyArgs);

    // VCL's Polygon only knows cubic beziers
    const long nX1 = (2 * rA.GetPosX() + 4 * p1->x + 3) / 6;
    const long nY1 = (2 * rA.GetPosY() + 4 * p1->y + 3) / 6;
    rA.AddPoint( nX1, nY1, POLY_CONTROL );

    const long nX2 = (2 * p2->x + 4 * p1->x + 3) / 6;
    const long nY2 = (2 * p2->y + 4 * p1->y + 3) / 6;
    rA.AddPoint( nX2, nY2, POLY_CONTROL );

    rA.AddPoint( p2->x, p2->y, POLY_NORMAL );
    return 0;
}

static int FT_cubic_to( const FT_Vector* p1, const FT_Vector* p2, const FT_Vector* p3, void* vpPolyArgs )
{
    PolyArgs& rA = *static_cast<PolyArgs*>(vpPolyArgs);
    rA.AddPoint( p1->x, p1->y, POLY_CONTROL );
    rA.AddPoint( p2->x, p2->y, POLY_CONTROL );
    rA.AddPoint( p3->x, p3->y, POLY_NORMAL );
    return 0;
}

} // extern "C"

bool ServerFont::GetGlyphOutline( sal_GlyphId aGlyphId,
    basegfx::B2DPolyPolygon& rB2DPolyPoly ) const
{
    if( maSizeFT )
        FT_Activate_Size( maSizeFT );

    rB2DPolyPoly.clear();

    int nGlyphFlags;
    SplitGlyphFlags( *this, aGlyphId, nGlyphFlags );

    FT_Int nLoadFlags = FT_LOAD_DEFAULT | FT_LOAD_IGNORE_TRANSFORM;

#ifdef FT_LOAD_TARGET_LIGHT
    // enable "light hinting" if available
    nLoadFlags |= FT_LOAD_TARGET_LIGHT;
#endif

    FT_Error rc = FT_Load_Glyph( maFaceFT, aGlyphId, nLoadFlags );
    if( rc != FT_Err_Ok )
        return false;

    if (mbArtBold)
        FT_GlyphSlot_Embolden(maFaceFT->glyph);

    FT_Glyph pGlyphFT;
    rc = FT_Get_Glyph( maFaceFT->glyph, &pGlyphFT );
    if( rc != FT_Err_Ok )
        return false;

    if( pGlyphFT->format != FT_GLYPH_FORMAT_OUTLINE )
    {
        FT_Done_Glyph( pGlyphFT );
        return false;
    }

    if( mbArtItalic )
    {
        FT_Matrix aMatrix;
        aMatrix.xx = aMatrix.yy = 0x10000L;
        aMatrix.xy = 0x6000L;
        aMatrix.yx = 0;
        FT_Glyph_Transform( pGlyphFT, &aMatrix, nullptr );
    }

    FT_Outline& rOutline = reinterpret_cast<FT_OutlineGlyphRec*>(pGlyphFT)->outline;
    if( !rOutline.n_points )    // blank glyphs are ok
    {
        FT_Done_Glyph( pGlyphFT );
        return true;
    }

    long nMaxPoints = 1 + rOutline.n_points * 3;
    tools::PolyPolygon aToolPolyPolygon;
    PolyArgs aPolyArg( aToolPolyPolygon, nMaxPoints );

    /*int nAngle =*/ ApplyGlyphTransform( nGlyphFlags, pGlyphFT, false );

    FT_Outline_Funcs aFuncs;
    aFuncs.move_to  = &FT_move_to;
    aFuncs.line_to  = &FT_line_to;
    aFuncs.conic_to = &FT_conic_to;
    aFuncs.cubic_to = &FT_cubic_to;
    aFuncs.shift    = 0;
    aFuncs.delta    = 0;
    FT_Outline_Decompose( &rOutline, &aFuncs, static_cast<void*>(&aPolyArg) );
    aPolyArg.ClosePolygon();    // close last polygon
    FT_Done_Glyph( pGlyphFT );

    // convert to basegfx polypolygon
    // TODO: get rid of the intermediate tools polypolygon
    rB2DPolyPoly = aToolPolyPolygon.getB2DPolyPolygon();
    rB2DPolyPoly.transform(basegfx::tools::createScaleB2DHomMatrix( +1.0/(1<<6), -1.0/(1<<6) ));

    return true;
}

void ServerFont::ApplyGSUB( const FontSelectPattern& rFSD )
{
#define MKTAG(s) ((((((s[0]<<8)+s[1])<<8)+s[2])<<8)+s[3])

    std::vector<sal_uInt32> aReqFeatureTagList;
    if( rFSD.mbVertical )
        aReqFeatureTagList.push_back( MKTAG("vert") );
    // TODO: request more features depending on script and language system

    if( aReqFeatureTagList.empty()) // nothing to do
        return;

    // load GSUB table into memory
    sal_uLong nLength = 0;
    const FT_Byte* const pGsubBase = mpFontInfo->GetTable( "GSUB", &nLength );
    if( !pGsubBase )
        return;

    // parse GSUB header
    const FT_Byte* pGsubHeader = pGsubBase;
    const sal_uInt16 nOfsScriptList     = GetUShort( pGsubHeader+4 );
    const sal_uInt16 nOfsFeatureTable   = GetUShort( pGsubHeader+6 );
    const sal_uInt16 nOfsLookupList     = GetUShort( pGsubHeader+8 );

    std::vector<sal_uInt16> aFeatureIndexList;

    // parse Script Table
    const FT_Byte* pScriptHeader = pGsubBase + nOfsScriptList;
    const sal_uInt16 nCntScript = GetUShort( pScriptHeader+0 );
    pScriptHeader += 2;
    for( sal_uInt16 nScriptIndex = 0; nScriptIndex < nCntScript; ++nScriptIndex )
    {
        const sal_uInt16 nOfsScriptTable= GetUShort( pScriptHeader+4 );
        pScriptHeader += 6;

        const FT_Byte* pScriptTable     = pGsubBase + nOfsScriptList + nOfsScriptTable;
        const sal_uInt16 nDefaultLangsysOfs = GetUShort( pScriptTable+0 );
        const sal_uInt16 nCntLangSystem     = GetUShort( pScriptTable+2 );
        pScriptTable += 4;
        sal_uInt16 nLangsysOffset = 0;

        if (nCntLangSystem != 0)
        {
            nLangsysOffset = GetUShort( pScriptTable+4 );
        }

        if (nDefaultLangsysOfs != 0 && nDefaultLangsysOfs != nLangsysOffset)
        {
            const FT_Byte* pLangSys = pGsubBase + nOfsScriptList + nOfsScriptTable + nDefaultLangsysOfs;
            const sal_uInt16 nReqFeatureIdx = GetUShort( pLangSys+2 );
            const sal_uInt16 nCntFeature    = GetUShort( pLangSys+4 );
            pLangSys += 6;
            aFeatureIndexList.push_back( nReqFeatureIdx );
            for( sal_uInt16 i = 0; i < nCntFeature; ++i )
            {
                const sal_uInt16 nFeatureIndex = GetUShort( pLangSys );
                pLangSys += 2;
                aFeatureIndexList.push_back( nFeatureIndex );
            }
        }

        if( nLangsysOffset != 0 )
        {
            const FT_Byte* pLangSys = pGsubBase + nOfsScriptList + nOfsScriptTable + nLangsysOffset;
            const sal_uInt16 nReqFeatureIdx = GetUShort( pLangSys+2 );
            const sal_uInt16 nCntFeature    = GetUShort( pLangSys+4 );
            pLangSys += 6;
            aFeatureIndexList.push_back( nReqFeatureIdx );
            for( sal_uInt16 i = 0; i < nCntFeature; ++i )
            {
                const sal_uInt16 nFeatureIndex = GetUShort( pLangSys );
                pLangSys += 2;
                aFeatureIndexList.push_back( nFeatureIndex );
            }
        }
    }

    if( aFeatureIndexList.empty() )
        return;

    std::vector<sal_uInt16> aLookupIndexList;
    std::vector<sal_uInt16> aLookupOffsetList;

    // parse Feature Table
    const FT_Byte* pFeatureHeader = pGsubBase + nOfsFeatureTable;
    const sal_uInt16 nCntFeature = GetUShort( pFeatureHeader );
    pFeatureHeader += 2;
    for( sal_uInt16 nFeatureIndex = 0; nFeatureIndex < nCntFeature; ++nFeatureIndex )
    {
        const sal_uLong nTag    = GetUInt( pFeatureHeader+0 ); // e.g. locl/vert/trad/smpl/liga/fina/...
        const sal_uInt16 nOffset= GetUShort( pFeatureHeader+4 );
        pFeatureHeader += 6;

        // short circuit some feature lookups
        if( aFeatureIndexList[0] != nFeatureIndex ) // required feature?
        {
            const int nRequested = std::count( aFeatureIndexList.begin(), aFeatureIndexList.end(), nFeatureIndex);
            if( !nRequested )  // ignore features that are not requested
                continue;
            const int nAvailable = std::count( aReqFeatureTagList.begin(), aReqFeatureTagList.end(), nTag);
            if( !nAvailable )  // some fonts don't provide features they request!
                continue;
        }

        const FT_Byte* pFeatureTable = pGsubBase + nOfsFeatureTable + nOffset;
        pFeatureTable += 2; // ignore FeatureParams
        const sal_uInt16 nCntLookups = GetUShort( pFeatureTable+0 );
        pFeatureTable += 2;
        for( sal_uInt16 i = 0; i < nCntLookups; ++i )
        {
            const sal_uInt16 nLookupIndex = GetUShort( pFeatureTable );
            pFeatureTable += 2;
            aLookupIndexList.push_back( nLookupIndex );
        }
        if( nCntLookups == 0 ) //### hack needed by Mincho/Gothic/Mingliu/Simsun/...
            aLookupIndexList.push_back( 0 );
    }

    // parse Lookup List
    const FT_Byte* pLookupHeader = pGsubBase + nOfsLookupList;
    const sal_uInt16 nCntLookupTable = GetUShort( pLookupHeader );
    pLookupHeader += 2;
    for( sal_uInt16 nLookupIdx = 0; nLookupIdx < nCntLookupTable; ++nLookupIdx )
    {
        const sal_uInt16 nOffset = GetUShort( pLookupHeader );
        pLookupHeader += 2;
        if( std::count( aLookupIndexList.begin(), aLookupIndexList.end(), nLookupIdx ) )
            aLookupOffsetList.push_back( nOffset );
    }

    std::vector<sal_uInt16>::const_iterator lookup_it = aLookupOffsetList.begin();
    for(; lookup_it != aLookupOffsetList.end(); ++lookup_it )
    {
        const sal_uInt16 nOfsLookupTable = *lookup_it;
        const FT_Byte* pLookupTable = pGsubBase + nOfsLookupList + nOfsLookupTable;
        const sal_uInt16 eLookupType        = GetUShort( pLookupTable+0 );
        const sal_uInt16 nCntLookupSubtable = GetUShort( pLookupTable+4 );
        pLookupTable += 6;

        // TODO: switch( eLookupType )
        if( eLookupType != 1 )  // TODO: once we go beyond SingleSubst
            continue;

        for( sal_uInt16 nSubTableIdx = 0; nSubTableIdx < nCntLookupSubtable; ++nSubTableIdx )
        {
            const sal_uInt16 nOfsSubLookupTable = GetUShort( pLookupTable );
            pLookupTable += 2;
            const FT_Byte* pSubLookup = pGsubBase + nOfsLookupList + nOfsLookupTable + nOfsSubLookupTable;

            const sal_uInt16 nFmtSubstitution   = GetUShort( pSubLookup+0 );
            const sal_uInt16 nOfsCoverage       = GetUShort( pSubLookup+2 );
            pSubLookup += 4;

            typedef std::pair<sal_uInt16,sal_uInt16> GlyphSubst;
            std::vector<GlyphSubst> aSubstVector;

            const FT_Byte* pCoverage    = pGsubBase + nOfsLookupList + nOfsLookupTable + nOfsSubLookupTable + nOfsCoverage;
            const sal_uInt16 nFmtCoverage   = GetUShort( pCoverage+0 );
            pCoverage += 2;
            switch( nFmtCoverage )
            {
                case 1:         // Coverage Format 1
                    {
                        const sal_uInt16 nCntGlyph = GetUShort( pCoverage );
                        pCoverage += 2;
                        aSubstVector.reserve( nCntGlyph );
                        for( sal_uInt16 i = 0; i < nCntGlyph; ++i )
                        {
                            const sal_uInt16 nGlyphId = GetUShort( pCoverage );
                            pCoverage += 2;
                            aSubstVector.push_back( GlyphSubst( nGlyphId, 0 ) );
                        }
                    }
                    break;

                case 2:         // Coverage Format 2
                    {
                        const sal_uInt16 nCntRange = GetUShort( pCoverage );
                        pCoverage += 2;
                        for( int i = nCntRange; --i >= 0; )
                        {
                            const sal_uInt32 nGlyph0 = GetUShort( pCoverage+0 );
                            const sal_uInt32 nGlyph1 = GetUShort( pCoverage+2 );
                            const sal_uInt16 nCovIdx = GetUShort( pCoverage+4 );
                            pCoverage += 6;
                            for( sal_uInt32 j = nGlyph0; j <= nGlyph1; ++j )
                                aSubstVector.push_back( GlyphSubst( static_cast<sal_uInt16>(j + nCovIdx), 0 ) );
                        }
                    }
                    break;
            }

            std::vector<GlyphSubst>::iterator it( aSubstVector.begin() );

            switch( nFmtSubstitution )
            {
                case 1:     // Single Substitution Format 1
                    {
                        const sal_uInt16 nDeltaGlyphId = GetUShort( pSubLookup );
                        for(; it != aSubstVector.end(); ++it )
                            (*it).second = (*it).first + nDeltaGlyphId;
                    }
                    break;

                case 2:     // Single Substitution Format 2
                    {
                        const sal_uInt16 nCntGlyph = GetUShort( pSubLookup );
                        pSubLookup += 2;
                        for( int i = nCntGlyph; (it != aSubstVector.end()) && (--i>=0); ++it )
                        {
                            const sal_uInt16 nGlyphId = GetUShort( pSubLookup );
                            pSubLookup += 2;
                            (*it).second = nGlyphId;
                        }
                    }
                    break;
            }

            SAL_WARN_IF( (it != aSubstVector.end()), "vcl", "lookup<->coverage table mismatch" );
            // now apply the glyph substitutions that have been collected in this subtable
            for( it = aSubstVector.begin(); it != aSubstVector.end(); ++it )
                maGlyphSubstitution[ (*it).first ] =  (*it).second;
        }
    }
}

const unsigned char* ServerFont::GetTable(const char* pName, sal_uLong* pLength)
{
    return mpFontInfo->GetTable( pName, pLength );
}

#if ENABLE_GRAPHITE
GraphiteFaceWrapper* ServerFont::GetGraphiteFace() const
{
    return mpFontInfo->GetGraphiteFace();
}
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
