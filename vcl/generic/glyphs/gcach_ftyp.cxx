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


#ifdef WNT
#include <svsys.h>
#undef CreateFont
#endif

#include "gcach_ftyp.hxx"

#include "vcl/svapp.hxx"
#include <outfont.hxx>
#include <impfont.hxx>
#include <config_graphite.h>
#if ENABLE_GRAPHITE
#include <graphite2/Font.h>
#include <graphite_layout.hxx>
#endif
#include <unotools/fontdefs.hxx>

#include "tools/poly.hxx"
#include "basegfx/matrix/b2dhommatrix.hxx"
#include "basegfx/matrix/b2dhommatrixtools.hxx"
#include "basegfx/polygon/b2dpolypolygon.hxx"

#include "osl/file.hxx"
#include "osl/thread.hxx"

#include "langboost.hxx"
#include "sft.hxx"

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H
#include FT_OUTLINE_H
#include FT_TRUETYPE_TABLES_H
#include FT_TRUETYPE_TAGS_H
#include FT_TRUETYPE_IDS_H

#ifdef ANDROID
#include FT_SIZES_H
#include FT_SYNTHESIS_H
#endif

#ifndef FT_RENDER_MODE_MONO  // happens in the MACOSX build
    #define FT_RENDER_MODE_MONO ft_render_mode_mono
#endif
#include "rtl/instance.hxx"

#define FTVERSION (1000*FREETYPE_MAJOR + 100*FREETYPE_MINOR + FREETYPE_PATCH)

#if FTVERSION >= 2200
typedef const FT_Vector* FT_Vector_CPtr;
#else // FTVERSION < 2200
typedef FT_Vector* FT_Vector_CPtr;
#endif

#include <vector>

// TODO: move file mapping stuff to OSL
#if defined(UNX)
    // PORTERS: dlfcn is used for getting symbols from FT versions newer than baseline
    #include <dlfcn.h>
    #include <unistd.h>
    #include <fcntl.h>
    #include <sys/stat.h>
    #include <sys/mman.h>
    #include "vcl/fontmanager.hxx"
#elif defined(WNT)
    #include <io.h>
    #define strncasecmp strnicmp
#endif

typedef const unsigned char* CPU8;
inline sal_uInt16 NEXT_U16( CPU8& p ) { p+=2; return (p[-2]<<8)|p[-1]; }
inline sal_Int16  NEXT_S16( CPU8& p ) { return (sal_Int16)NEXT_U16(p); }
inline sal_uInt32 NEXT_U32( CPU8& p ) { p+=4; return (p[-4]<<24)|(p[-3]<<16)|(p[-2]<<8)|p[-1]; }
//inline sal_Int32 NEXT_S32( U8*& p ) { return (sal_Int32)NEXT_U32(p); }

// -----------------------------------------------------------------------

// the gamma table makes artificial bold look better for CJK glyphs
static unsigned char aGammaTable[257];

static void InitGammaTable()
{
    static const int M_MAX = 255;
    static const int M_X   = 128;
    static const int M_Y   = 208;

    int x, a;
    for( x = 0; x < 256; x++)
    {
        if ( x <= M_X )
            a = ( x * M_Y + M_X / 2) / M_X;
        else
            a = M_Y + ( ( x - M_X ) * ( M_MAX - M_Y ) +
                ( M_MAX - M_X ) / 2 ) / ( M_MAX - M_X );

        aGammaTable[x] = (unsigned char)a;
    }
}

// -----------------------------------------------------------------------

static FT_Library aLibFT = 0;

// #110607# enable linking with old FT versions
static int nFTVERSION = 0;
static FT_Error (*pFTNewSize)(FT_Face,FT_Size*);
static FT_Error (*pFTActivateSize)(FT_Size);
static FT_Error (*pFTDoneSize)(FT_Size);
void (*pFTEmbolden)(FT_GlyphSlot);
static FT_UInt (*pFT_Face_GetCharVariantIndex)(FT_Face, FT_ULong, FT_ULong);
static bool bEnableSizeFT = false;

typedef ::boost::unordered_map<const char*, boost::shared_ptr<FtFontFile>, rtl::CStringHash, rtl::CStringEqual> FontFileList;

namespace { struct vclFontFileList : public rtl::Static< FontFileList, vclFontFileList > {}; }

// -----------------------------------------------------------------------

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

// =======================================================================
// FreetypeManager
// =======================================================================

FtFontFile::FtFontFile( const OString& rNativeFileName )
:   maNativeFileName( rNativeFileName ),
    mpFileMap( NULL ),
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
        static const char* pLangBoost = NULL;
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

// -----------------------------------------------------------------------

FtFontFile* FtFontFile::FindFontFile( const OString& rNativeFileName )
{
    // font file already known? (e.g. for ttc, synthetic, aliased fonts)
    const char* pFileName = rNativeFileName.getStr();
    FontFileList &rFontFileList = vclFontFileList::get();
    FontFileList::const_iterator it = rFontFileList.find( pFileName );
    if( it != rFontFileList.end() )
        return it->second.get();

    // no => create new one
    FtFontFile* pFontFile = new FtFontFile( rNativeFileName );
    pFileName = pFontFile->maNativeFileName.getStr();
    rFontFileList[pFileName].reset(pFontFile);
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
            mmap( NULL, mnFileSize, PROT_READ, MAP_SHARED, nFile, 0 );
        if( mpFileMap == MAP_FAILED )
            mpFileMap = NULL;
        close( nFile );
#elif defined(WNT)
        void* pFileDesc = ::CreateFile( pFileName, GENERIC_READ, FILE_SHARE_READ,
                        NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0 );
        if( pFileDesc == INVALID_HANDLE_VALUE)
            return false;

        mnFileSize = ::GetFileSize( pFileDesc, NULL );
        HANDLE aHandle = ::CreateFileMapping( pFileDesc, NULL, PAGE_READONLY, 0, mnFileSize, "TTF" );
        mpFileMap = (const unsigned char*)::MapViewOfFile( aHandle, FILE_MAP_READ, 0, 0, mnFileSize );
        ::CloseHandle( pFileDesc );
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
#elif defined(WNT)
    UnmapViewOfFile( (LPCVOID)mpFileMap );
#else
    delete[] mpFileMap;
#endif

    mpFileMap = NULL;
}

#if ENABLE_GRAPHITE
// wrap FtFontInfo's table function
const void * graphiteFontTable(const void* appFaceHandle, unsigned int name, size_t *len)
{
    const FtFontInfo * pFontInfo = reinterpret_cast<const FtFontInfo*>(appFaceHandle);
    typedef union {
        char m_c[5];
        unsigned int m_id;
    } TableId;
    TableId tableId;
    tableId.m_id = name;
#ifndef WORDS_BIGENDIAN
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

// =======================================================================

FtFontInfo::FtFontInfo( const ImplDevFontAttributes& rDevFontAttributes,
    const OString& rNativeFileName, int nFaceNum, sal_IntPtr nFontId, int nSynthetic,
    const ExtraKernInfo* pExtraKernInfo )
:
    maFaceFT( NULL ),
    mpFontFile( FtFontFile::FindFontFile( rNativeFileName ) ),
    mnFaceNum( nFaceNum ),
    mnRefCount( 0 ),
    mnSynthetic( nSynthetic ),
#if ENABLE_GRAPHITE
    mbCheckedGraphite(false),
    mpGraphiteFace(NULL),
#endif
    mnFontId( nFontId ),
    maDevFontAttributes( rDevFontAttributes ),
    mpFontCharMap( NULL ),
    mpChar2Glyph( NULL ),
    mpGlyph2Char( NULL ),
    mpExtraKernInfo( pExtraKernInfo )
{
    // prefer font with low ID
    maDevFontAttributes.mnQuality += 10000 - nFontId;
    // prefer font with matching file names
    maDevFontAttributes.mnQuality += mpFontFile->GetLangBoost();
    // prefer font with more external info
    if( pExtraKernInfo )
        maDevFontAttributes.mnQuality += 100;
}

// -----------------------------------------------------------------------

FtFontInfo::~FtFontInfo()
{
    if( mpFontCharMap )
        mpFontCharMap->DeReference();
    delete mpExtraKernInfo;
    delete mpChar2Glyph;
    delete mpGlyph2Char;
#if ENABLE_GRAPHITE
    delete mpGraphiteFace;
#endif
}

void FtFontInfo::InitHashes() const
{
    // TODO: avoid pointers when empty stl::hash_* objects become cheap
    mpChar2Glyph = new Int2IntMap();
    mpGlyph2Char = new Int2IntMap();
}

// -----------------------------------------------------------------------

FT_FaceRec_* FtFontInfo::GetFaceFT()
{
    // get faceFT once/multiple depending on availability of SizeFT APIs
    if( (mnRefCount++ <= 0) || !bEnableSizeFT )
    {
        if( !mpFontFile->Map() )
            return NULL;
        FT_Error rc = FT_New_Memory_Face( aLibFT,
            (FT_Byte*)mpFontFile->GetBuffer(),
            mpFontFile->GetFileSize(), mnFaceNum, &maFaceFT );
        if( (rc != FT_Err_Ok) || (maFaceFT->num_glyphs <= 0) )
            maFaceFT = NULL;
    }

   return maFaceFT;
}

#if ENABLE_GRAPHITE
GraphiteFaceWrapper * FtFontInfo::GetGraphiteFace()
{
    if (mbCheckedGraphite)
        return mpGraphiteFace;
    // test for graphite here so that it is cached most efficiently
    if (GetTable("Silf", 0))
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

// -----------------------------------------------------------------------

void FtFontInfo::ReleaseFaceFT( FT_FaceRec_* pFaceFT )
{
    // release last/each depending on SizeFT availability
    if( (--mnRefCount <= 0) || !bEnableSizeFT )
    {
        FT_Done_Face( pFaceFT );
        maFaceFT = NULL;
        mpFontFile->Unmap();
    }
}

// -----------------------------------------------------------------------

bool FtFontInfo::HasExtraKerning() const
{
    if( !mpExtraKernInfo )
        return false;
    // TODO: how to enable the line below without getting #i29881# back?
    // on the other hand being too optimistic doesn't cause problems
    // return mpExtraKernInfo->HasKernPairs();
    return true;
}

// -----------------------------------------------------------------------

int FtFontInfo::GetExtraKernPairs( ImplKernPairData** ppKernPairs ) const
{
    if( !mpExtraKernInfo )
        return 0;
    return mpExtraKernInfo->GetUnscaledKernPairs( ppKernPairs );
}

// -----------------------------------------------------------------------

static unsigned GetUInt( const unsigned char* p ) { return((p[0]<<24)+(p[1]<<16)+(p[2]<<8)+p[3]);}
static unsigned GetUShort( const unsigned char* p ){ return((p[0]<<8)+p[1]);}
//static signed GetSShort( const unsigned char* p ){ return((short)((p[0]<<8)+p[1]));}

static const sal_uInt32 T_true = 0x74727565;        /* 'true' */
static const sal_uInt32 T_ttcf = 0x74746366;        /* 'ttcf' */
static const sal_uInt32 T_otto = 0x4f54544f;        /* 'OTTO' */

const unsigned char* FtFontInfo::GetTable( const char* pTag, sal_uLong* pLength ) const
{
    const unsigned char* pBuffer = mpFontFile->GetBuffer();
    int nFileSize = mpFontFile->GetFileSize();
    if( !pBuffer || nFileSize<1024 )
        return NULL;

    // we currently handle TTF, TTC and OTF headers
    unsigned nFormat = GetUInt( pBuffer );

    const unsigned char* p = pBuffer + 12;
    if( nFormat == T_ttcf )         // TTC_MAGIC
        p += GetUInt( p + 4 * mnFaceNum );
    else if( nFormat != 0x00010000 && nFormat != T_true && nFormat != T_otto) // TTF_MAGIC and Apple TTF Magic and PS-OpenType font
        return NULL;

    // walk table directory until match
    int nTables = GetUShort( p - 8 );
    if( nTables >= 64 )  // something fishy?
        return NULL;
    for( int i = 0; i < nTables; ++i, p+=16 )
    {
        if( p[0]==pTag[0] && p[1]==pTag[1] && p[2]==pTag[2] && p[3]==pTag[3] )
        {
            sal_uLong nLength = GetUInt( p + 12 );
            if( pLength != NULL )
                *pLength = nLength;
            const unsigned char* pTable = pBuffer + GetUInt( p + 8 );
            if( (pTable + nLength) <= (mpFontFile->GetBuffer() + nFileSize) )
                return pTable;
        }
    }

    return NULL;
}

// -----------------------------------------------------------------------

void FtFontInfo::AnnounceFont( ImplDevFontList* pFontList )
{
    ImplFTSFontData* pFD = new ImplFTSFontData( this, maDevFontAttributes );
    pFontList->Add( pFD );
}

// =======================================================================

FreetypeManager::FreetypeManager()
:   mnMaxFontId( 0 )
{
    /*FT_Error rcFT =*/ FT_Init_FreeType( &aLibFT );

#ifdef ANDROID
    // For Android we use the bundled static libfreetype.a, and we
    // want to avoid accidentally finding the FT_* symbols in the
    // system FreeType code (which *is* present in a system library,
    // libskia.so, but is not a public API, and in fact does crash the
    // app if used).
    pFTNewSize = FT_New_Size;
    pFTActivateSize = FT_Activate_Size;
    pFTDoneSize = FT_Done_Size;
    pFTEmbolden = FT_GlyphSlot_Embolden;
    pFT_Face_GetCharVariantIndex = FT_Face_GetCharVariantIndex;
    nFTVERSION = FTVERSION;
#else
#ifdef RTLD_DEFAULT // true if a good dlfcn.h header was included
    // Get version of freetype library to enable workarounds.
    // Freetype <= 2.0.9 does not have FT_Library_Version().
    // Using dl_sym() instead of osl_getSymbol() because latter
    // isn't designed to work with oslModule=NULL
    void (*pFTLibraryVersion)(FT_Library library,
        FT_Int *amajor, FT_Int *aminor, FT_Int *apatch);
    pFTLibraryVersion = (void (*)(FT_Library library,
        FT_Int *amajor, FT_Int *aminor, FT_Int *apatch))(sal_IntPtr)dlsym( RTLD_DEFAULT, "FT_Library_Version" );

    pFTNewSize      = (FT_Error(*)(FT_Face,FT_Size*))(sal_IntPtr)dlsym( RTLD_DEFAULT, "FT_New_Size" );
    pFTActivateSize = (FT_Error(*)(FT_Size))(sal_IntPtr)dlsym( RTLD_DEFAULT, "FT_Activate_Size" );
    pFTDoneSize     = (FT_Error(*)(FT_Size))(sal_IntPtr)dlsym( RTLD_DEFAULT, "FT_Done_Size" );
    pFTEmbolden     = (void(*)(FT_GlyphSlot))(sal_IntPtr)dlsym( RTLD_DEFAULT, "FT_GlyphSlot_Embolden" );
    pFT_Face_GetCharVariantIndex = (FT_UInt(*)(FT_Face, FT_ULong, FT_ULong))(sal_IntPtr)dlsym( RTLD_DEFAULT, "FT_Face_GetCharVariantIndex" );

    bEnableSizeFT = (pFTNewSize!=NULL) && (pFTActivateSize!=NULL) && (pFTDoneSize!=NULL);

    FT_Int nMajor = 0, nMinor = 0, nPatch = 0;
    if( pFTLibraryVersion )
        pFTLibraryVersion( aLibFT, &nMajor, &nMinor, &nPatch );
    nFTVERSION = nMajor * 1000 + nMinor * 100 + nPatch;

    // disable artificial emboldening with the Freetype API for older versions
    if( nFTVERSION < 2110 )
        pFTEmbolden = NULL;
    // disable FT_Face_GetCharVariantIndex for older versions
    // https://bugzilla.mozilla.org/show_bug.cgi?id=618406#c8
    if( nFTVERSION < 2404 )
        pFT_Face_GetCharVariantIndex = NULL;

#else // RTLD_DEFAULT
    // assume systems where dlsym is not possible use supplied library
    nFTVERSION = FTVERSION;
#endif
#endif
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

    InitGammaTable();
    vclFontFileList::get();
}

// -----------------------------------------------------------------------

FT_Face ServerFont::GetFtFace() const
{
    if( maSizeFT )
        pFTActivateSize( maSizeFT );

    return maFaceFT;
}

// -----------------------------------------------------------------------

FreetypeManager::~FreetypeManager()
{
    ClearFontList();
}

// -----------------------------------------------------------------------

void FreetypeManager::AddFontFile( const OString& rNormalizedName,
    int nFaceNum, sal_IntPtr nFontId, const ImplDevFontAttributes& rDevFontAttr,
    const ExtraKernInfo* pExtraKernInfo )
{
    if( rNormalizedName.isEmpty() )
        return;

    if( maFontList.find( nFontId ) != maFontList.end() )
        return;

    FtFontInfo* pFontInfo = new FtFontInfo( rDevFontAttr,
        rNormalizedName, nFaceNum, nFontId, 0, pExtraKernInfo );
    maFontList[ nFontId ] = pFontInfo;
    if( mnMaxFontId < nFontId )
        mnMaxFontId = nFontId;
}

// -----------------------------------------------------------------------

void FreetypeManager::AnnounceFonts( ImplDevFontList* pToAdd ) const
{
    for( FontList::const_iterator it = maFontList.begin(); it != maFontList.end(); ++it )
    {
        FtFontInfo* pFtFontInfo = it->second;
        pFtFontInfo->AnnounceFont( pToAdd );
    }
}

// -----------------------------------------------------------------------

void FreetypeManager::ClearFontList( )
{
    for( FontList::iterator it = maFontList.begin(); it != maFontList.end(); ++it )
    {
        FtFontInfo* pFtFontInfo = it->second;
        delete pFtFontInfo;
    }
    maFontList.clear();
}

// -----------------------------------------------------------------------

ServerFont* FreetypeManager::CreateFont( const FontSelectPattern& rFSD )
{
    FtFontInfo* pFontInfo = NULL;

    // find a FontInfo matching to the font id
    sal_IntPtr nFontId = reinterpret_cast<sal_IntPtr>( rFSD.mpFontData );
    FontList::iterator it = maFontList.find( nFontId );
    if( it != maFontList.end() )
        pFontInfo = it->second;

    if( !pFontInfo )
        return NULL;

    ServerFont* pNew = new ServerFont( rFSD, pFontInfo );

    return pNew;
}

// =======================================================================

ImplFTSFontData::ImplFTSFontData( FtFontInfo* pFI, const ImplDevFontAttributes& rDFA )
:   PhysicalFontFace( rDFA, IFTSFONT_MAGIC ),
    mpFtFontInfo( pFI )
{
    mbDevice        = false;
    mbOrientation   = true;
}

// -----------------------------------------------------------------------

ImplFontEntry* ImplFTSFontData::CreateFontInstance( FontSelectPattern& rFSD ) const
{
    ImplServerFontEntry* pEntry = new ImplServerFontEntry( rFSD );
    return pEntry;
}

// =======================================================================
// ServerFont
// =======================================================================

ServerFont::ServerFont( const FontSelectPattern& rFSD, FtFontInfo* pFI )
:   maGlyphList( 0),
    maFontSelData(rFSD),
    mnRefCount(1),
    mnBytesUsed( sizeof(ServerFont) ),
    mpPrevGCFont( NULL ),
    mpNextGCFont( NULL ),
    mnCos( 0x10000),
    mnSin( 0 ),
    mbCollectedZW( false ),
    mnPrioEmbedded(nDefaultPrioEmbedded),
    mnPrioAntiAlias(nDefaultPrioAntiAlias),
    mnPrioAutoHint(nDefaultPrioAutoHint),
    mpFontInfo( pFI ),
    maFaceFT( NULL ),
    maSizeFT( NULL ),
    mbFaceOk( false ),
    maRecodeConverter( NULL ),
    mpLayoutEngine( NULL )
{
    // TODO: move update of mpFontEntry into FontEntry class when
    // it becomes reponsible for the ServerFont instantiation
    ((ImplServerFontEntry*)rFSD.mpFontEntry)->SetServerFont( this );

    if( rFSD.mnOrientation != 0 )
    {
        const double dRad = rFSD.mnOrientation * ( F_2PI / 3600.0 );
        mnCos = static_cast<long>( 0x10000 * cos( dRad ) + 0.5 );
        mnSin = static_cast<long>( 0x10000 * sin( dRad ) + 0.5 );
    }

    maFaceFT = pFI->GetFaceFT();

    if( !maFaceFT )
        return;

    // set the pixel size of the font instance
    mnWidth = rFSD.mnWidth;
    if( !mnWidth )
        mnWidth = rFSD.mnHeight;
    mfStretch = (double)mnWidth / rFSD.mnHeight;
    // sanity check (e.g. #i66394#, #i66244#, #66537#)
    if( (mnWidth < 0) || (mfStretch > +64.0) || (mfStretch < -64.0) )
        return;

    // perf: use maSizeFT if available
    if( bEnableSizeFT )
    {
        pFTNewSize( maFaceFT, &maSizeFT );
        pFTActivateSize( maSizeFT );
    }
    FT_Error rc = FT_Set_Pixel_Sizes( maFaceFT, mnWidth, rFSD.mnHeight );
    if( rc != FT_Err_Ok )
        return;

    // prepare for font encodings other than unicode or symbol
    FT_Encoding eEncoding = FT_ENCODING_UNICODE;
    if( mpFontInfo->IsSymbolFont() )
    {
        if( FT_IS_SFNT( maFaceFT ) )
            eEncoding = ft_encoding_symbol;
        else
            eEncoding = FT_ENCODING_ADOBE_CUSTOM; // freetype wants this for PS symbol fonts
    }
    rc = FT_Select_Charmap( maFaceFT, eEncoding );
    // no standard encoding applies => we need an encoding converter
    if( rc != FT_Err_Ok )
    {
        rtl_TextEncoding eRecodeFrom = RTL_TEXTENCODING_UNICODE;
        for( int i = maFaceFT->num_charmaps; --i >= 0; )
        {
            const FT_CharMap aCM = maFaceFT->charmaps[i];
            if( aCM->platform_id == TT_PLATFORM_MICROSOFT )
            {
                switch( aCM->encoding_id )
                {
                    case TT_MS_ID_SJIS:
                        eEncoding = FT_ENCODING_SJIS;
                        eRecodeFrom = RTL_TEXTENCODING_SHIFT_JIS;
                        break;
                    case TT_MS_ID_GB2312:
                        eEncoding = FT_ENCODING_GB2312;
                        eRecodeFrom = RTL_TEXTENCODING_GB_2312;
                        break;
                    case TT_MS_ID_BIG_5:
                        eEncoding = FT_ENCODING_BIG5;
                        eRecodeFrom = RTL_TEXTENCODING_BIG5;
                        break;
                    case TT_MS_ID_WANSUNG:
                        eEncoding = FT_ENCODING_WANSUNG;
                        eRecodeFrom = RTL_TEXTENCODING_MS_949;
                        break;
                    case TT_MS_ID_JOHAB:
                        eEncoding = FT_ENCODING_JOHAB;
                        eRecodeFrom = RTL_TEXTENCODING_MS_1361;
                        break;
                }
            }
            else if( aCM->platform_id == TT_PLATFORM_MACINTOSH )
            {
                switch( aCM->encoding_id )
                {
                    case TT_MAC_ID_ROMAN:
                        eEncoding = FT_ENCODING_APPLE_ROMAN;
                        eRecodeFrom = RTL_TEXTENCODING_UNICODE; // TODO: use better match
                        break;
                    // TODO: add other encodings when Mac-only
                    //       non-unicode fonts show up
                }
            }
            else if( aCM->platform_id == TT_PLATFORM_ADOBE )
            {
                switch( aCM->encoding_id )
                {
#ifdef TT_ADOBE_ID_LATIN1
                    case TT_ADOBE_ID_LATIN1:   // better unicode than nothing
                        eEncoding = FT_ENCODING_ADOBE_LATIN_1;
                        eRecodeFrom = RTL_TEXTENCODING_ISO_8859_1;
                        break;
#endif // TT_ADOBE_ID_LATIN1
                    case TT_ADOBE_ID_STANDARD:   // better unicode than nothing
                        eEncoding = FT_ENCODING_ADOBE_STANDARD;
                        eRecodeFrom = RTL_TEXTENCODING_UNICODE; // TODO: use better match
                        break;
                }
            }
        }

        if( FT_Err_Ok != FT_Select_Charmap( maFaceFT, eEncoding ) )
            return;

        if( eRecodeFrom != RTL_TEXTENCODING_UNICODE )
            maRecodeConverter = rtl_createUnicodeToTextConverter( eRecodeFrom );
    }

    mbFaceOk = true;

    ApplyGSUB( rFSD );

    // TODO: query GASP table for load flags
    mnLoadFlags = FT_LOAD_DEFAULT;
#if 1 // #i97326# cairo sometimes uses FT_Set_Transform() on our FT_FACE
    // we are not using FT_Set_Transform() yet, so just ignore it for now
    mnLoadFlags |= FT_LOAD_IGNORE_TRANSFORM;
#endif

    mbArtItalic = (rFSD.GetSlant() != ITALIC_NONE && pFI->GetFontAttributes().GetSlant() == ITALIC_NONE);
    mbArtBold = (rFSD.GetWeight() > WEIGHT_MEDIUM && pFI->GetFontAttributes().GetWeight() <= WEIGHT_MEDIUM);
    mbUseGamma = false;
    if( mbArtBold )
    {
        //static const int TT_CODEPAGE_RANGE_874  = (1L << 16); // Thai
        //static const int TT_CODEPAGE_RANGE_932  = (1L << 17); // JIS/Japan
        //static const int TT_CODEPAGE_RANGE_936  = (1L << 18); // Chinese: Simplified
        //static const int TT_CODEPAGE_RANGE_949  = (1L << 19); // Korean Wansung
        //static const int TT_CODEPAGE_RANGE_950  = (1L << 20); // Chinese: Traditional
        //static const int TT_CODEPAGE_RANGE_1361 = (1L << 21); // Korean Johab
        static const int TT_CODEPAGE_RANGES1_CJKT = 0x3F0000; // all of the above
        const TT_OS2* pOs2 = (const TT_OS2*)FT_Get_Sfnt_Table( maFaceFT, ft_sfnt_os2 );
        if ((pOs2) && (pOs2->ulCodePageRange1 & TT_CODEPAGE_RANGES1_CJKT )
        && rFSD.mnHeight < 20)
        mbUseGamma = true;
    }

    if( ((mnCos != 0) && (mnSin != 0)) || (mnPrioEmbedded <= 0) )
        mnLoadFlags |= FT_LOAD_NO_BITMAP;
}

void ServerFont::SetFontOptions( boost::shared_ptr<ImplFontOptions> pFontOptions)
{
    mpFontOptions = pFontOptions;

    if (!mpFontOptions)
        return;

    FontAutoHint eHint = mpFontOptions->GetUseAutoHint();
    if( eHint == AUTOHINT_DONTKNOW )
        eHint = mbUseGamma ? AUTOHINT_TRUE : AUTOHINT_FALSE;

    if( eHint == AUTOHINT_TRUE )
        mnLoadFlags |= FT_LOAD_FORCE_AUTOHINT;

    if( (mnSin != 0) && (mnCos != 0) ) // hinting for 0/90/180/270 degrees only
        mnLoadFlags |= FT_LOAD_NO_HINTING;
    mnLoadFlags |= FT_LOAD_IGNORE_GLOBAL_ADVANCE_WIDTH; //#88334#

    if( mpFontOptions->DontUseAntiAlias() )
      mnPrioAntiAlias = 0;
    if( mpFontOptions->DontUseEmbeddedBitmaps() )
      mnPrioEmbedded = 0;
    if( mpFontOptions->DontUseHinting() )
      mnPrioAutoHint = 0;

    if( mnPrioAutoHint <= 0 )
        mnLoadFlags |= FT_LOAD_NO_HINTING;

#if defined(FT_LOAD_TARGET_LIGHT) && defined(FT_LOAD_TARGET_NORMAL)
    if( !(mnLoadFlags & FT_LOAD_NO_HINTING) )
    {
       mnLoadFlags |= FT_LOAD_TARGET_NORMAL;
       switch( mpFontOptions->GetHintStyle() )
       {
           case HINT_NONE:
                mnLoadFlags |= FT_LOAD_NO_HINTING;
                break;
           case HINT_SLIGHT:
                mnLoadFlags |= FT_LOAD_TARGET_LIGHT;
                break;
           case HINT_MEDIUM:
                break;
           case HINT_FULL:
           default:
                break;
       }
    }
#endif

    if( mnPrioEmbedded <= 0 )
        mnLoadFlags |= FT_LOAD_NO_BITMAP;
}

boost::shared_ptr<ImplFontOptions> ServerFont::GetFontOptions() const
{
    return mpFontOptions;
}

const OString* ServerFont::GetFontFileName() const
{
    return mpFontInfo->GetFontFileName();
}

bool ServerFont::TestFont() const
{
    return mbFaceOk;
}

// -----------------------------------------------------------------------

ServerFont::~ServerFont()
{
    if( mpLayoutEngine )
        delete mpLayoutEngine;

    if( maRecodeConverter )
        rtl_destroyUnicodeToTextConverter( maRecodeConverter );

    if( maSizeFT )
        pFTDoneSize( maSizeFT );

    mpFontInfo->ReleaseFaceFT( maFaceFT );

    ReleaseFromGarbageCollect();
}

 // -----------------------------------------------------------------------

int ServerFont::GetEmUnits() const
{
    return maFaceFT->units_per_EM;
}

// -----------------------------------------------------------------------

void ServerFont::FetchFontMetric( ImplFontMetricData& rTo, long& rFactor ) const
{
    static_cast<ImplFontAttributes&>(rTo) = mpFontInfo->GetFontAttributes();

    rTo.mbScalableFont  = true;
    rTo.mbDevice        = true;
    rTo.mbKernableFont  = (FT_HAS_KERNING( maFaceFT ) != 0) || mpFontInfo->HasExtraKerning();
    rTo.mnOrientation = GetFontSelData().mnOrientation;

    //Always consider [star]symbol as symbol fonts
    if ( IsStarSymbol( rTo.GetFamilyName() ) )
        rTo.SetSymbolFlag( true );

    if( maSizeFT )
        pFTActivateSize( maSizeFT );

    rFactor = 0x100;

    const TT_OS2* pOS2 = (const TT_OS2*)FT_Get_Sfnt_Table( maFaceFT, ft_sfnt_os2 );
    const double fScale = (double)GetFontSelData().mnHeight / maFaceFT->units_per_EM;

    rTo.mnAscent = 0;
    rTo.mnDescent = 0;
    rTo.mnExtLeading = 0;
    rTo.mnSlant = 0;
    rTo.mnWidth = mnWidth;

    // Calculating ascender and descender:
    // FreeType >= 2.4.6 does the right thing, so we just use what it gives us,
    // for earlier versions we emulate its behaviour;
    // take them from 'hhea' table,
    // if zero take them from 'OS/2' table,
    // if zero take them from FreeType's font metrics
    if (nFTVERSION >= 2406)
    {
        const FT_Size_Metrics& rMetrics = maFaceFT->size->metrics;
        rTo.mnAscent = (rMetrics.ascender + 32) >> 6;
        rTo.mnDescent = (-rMetrics.descender + 32) >> 6;
        rTo.mnExtLeading = ((rMetrics.height + 32) >> 6) - (rTo.mnAscent + rTo.mnDescent);
    }
    else
    {
        const TT_HoriHeader* pHHea = (const TT_HoriHeader*)FT_Get_Sfnt_Table(maFaceFT, ft_sfnt_hhea);
        if (pHHea)
        {
            rTo.mnAscent = pHHea->Ascender * fScale + 0.5;
            rTo.mnDescent = -pHHea->Descender * fScale + 0.5;
            rTo.mnExtLeading = pHHea->Line_Gap * fScale + 0.5;
        }

        if (!(rTo.mnAscent || rTo.mnDescent))
        {
            if (pOS2 && (pOS2->version != 0xFFFF))
            {
                if (pOS2->sTypoAscender || pOS2->sTypoDescender)
                {
                    rTo.mnAscent = pOS2->sTypoAscender * fScale + 0.5;
                    rTo.mnDescent = -pOS2->sTypoDescender * fScale + 0.5;
                    rTo.mnExtLeading = pOS2->sTypoLineGap * fScale + 0.5;
                }
                else
                {
                    rTo.mnAscent = pOS2->usWinAscent * fScale + 0.5;
                    rTo.mnDescent = pOS2->usWinDescent * fScale + 0.5;
                    rTo.mnExtLeading = 0;
                }
            }
        }

        if (!(rTo.mnAscent || rTo.mnDescent))
        {
            const FT_Size_Metrics& rMetrics = maFaceFT->size->metrics;
            rTo.mnAscent = (rMetrics.ascender + 32) >> 6;
            rTo.mnDescent = (-rMetrics.descender + 32) >> 6;
            rTo.mnExtLeading = ((rMetrics.height + 32) >> 6) - (rTo.mnAscent + rTo.mnDescent);
        }
    }

    rTo.mnIntLeading = rTo.mnAscent + rTo.mnDescent - (maFaceFT->units_per_EM * fScale + 0.5);

    if( pOS2 && (pOS2->version != 0xFFFF) )
    {
        // map the panose info from the OS2 table to their VCL counterparts
        switch( pOS2->panose[0] )
        {
            case 1: rTo.SetFamilyType( FAMILY_ROMAN ); break;
            case 2: rTo.SetFamilyType( FAMILY_SWISS ); break;
            case 3: rTo.SetFamilyType( FAMILY_MODERN ); break;
            case 4: rTo.SetFamilyType( FAMILY_SCRIPT ); break;
            case 5: rTo.SetFamilyType( FAMILY_DECORATIVE ); break;
            // TODO: is it reasonable to override the attribute with DONTKNOW?
            case 0: // fall through
            default: rTo.meFamilyType = FAMILY_DONTKNOW; break;
        }

        switch( pOS2->panose[3] )
        {
            case 2: // fall through
            case 3: // fall through
            case 4: // fall through
            case 5: // fall through
            case 6: // fall through
            case 7: // fall through
            case 8: rTo.SetPitch( PITCH_VARIABLE ); break;
            case 9: rTo.SetPitch( PITCH_FIXED ); break;
            // TODO: is it reasonable to override the attribute with DONTKNOW?
            case 0: // fall through
            case 1: // fall through
            default: rTo.SetPitch( PITCH_DONTKNOW ); break;
        }
    }

    // initialize kashida width
    // TODO: what if there are different versions of this glyph available
    const int nKashidaGlyphId = GetRawGlyphIndex( 0x0640 );
    if( nKashidaGlyphId )
    {
        GlyphData aGlyphData;
        InitGlyphData( nKashidaGlyphId, aGlyphData );
        rTo.mnMinKashida = aGlyphData.GetMetric().GetCharWidth();
    }
}

// -----------------------------------------------------------------------

static inline void SplitGlyphFlags( const ServerFont& rFont, int& nGlyphIndex, int& nGlyphFlags )
{
    nGlyphFlags = nGlyphIndex & GF_FLAGMASK;
    nGlyphIndex &= GF_IDXMASK;

    if( nGlyphIndex & GF_ISCHAR )
        nGlyphIndex = rFont.GetRawGlyphIndex( nGlyphIndex );
}

// -----------------------------------------------------------------------

int ServerFont::ApplyGlyphTransform( int nGlyphFlags,
    FT_Glyph pGlyphFT, bool bForBitmapProcessing ) const
{
    int nAngle = GetFontSelData().mnOrientation;
    // shortcut most common case
    if( !nAngle && !nGlyphFlags )
        return nAngle;

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
        FT_Glyph_Transform( pGlyphFT, NULL, &aVector );

        // orthogonal transforms are better handled by bitmap operations
        if( bStretched || (bForBitmapProcessing && (nAngle % 900) != 0) )
        {
            // apply non-orthogonal or stretch transformations
            FT_Glyph_Transform( pGlyphFT, &aMatrix, NULL );
            nAngle = 0;
        }
    }
    else
    {
        // FT<=2005 ignores transforms for bitmaps, so do it manually
        FT_BitmapGlyph pBmpGlyphFT = reinterpret_cast<FT_BitmapGlyph>(pGlyphFT);
        pBmpGlyphFT->left += (aVector.x + 32) >> 6;
        pBmpGlyphFT->top  += (aVector.y + 32) >> 6;
    }

    return nAngle;
}

// -----------------------------------------------------------------------

int ServerFont::GetRawGlyphIndex(sal_UCS4 aChar, sal_UCS4 aVS) const
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

    // if needed recode from unicode to font encoding
    if( maRecodeConverter )
    {
        sal_Char aTempArray[8];
        sal_Size nTempSize;
        sal_uInt32 nCvtInfo;

        // assume that modern UCS4 fonts have unicode CMAPs
    // => no encoding remapping to unicode is needed
        if( aChar > 0xFFFF )
            return 0;

        sal_Unicode aUCS2Char = static_cast<sal_Unicode>(aChar);
        rtl_UnicodeToTextContext aContext = rtl_createUnicodeToTextContext( maRecodeConverter );
        int nChars = rtl_convertUnicodeToText( maRecodeConverter, aContext,
            &aUCS2Char, 1, aTempArray, sizeof(aTempArray),
            RTL_UNICODETOTEXT_FLAGS_UNDEFINED_QUESTIONMARK
            | RTL_UNICODETOTEXT_FLAGS_INVALID_QUESTIONMARK,
            &nCvtInfo, &nTempSize );
        rtl_destroyUnicodeToTextContext( maRecodeConverter, aContext );

        aChar = 0;
        for( int i = 0; i < nChars; ++i )
            aChar = aChar*256 + (aTempArray[i] & 0xFF);
    }

    int nGlyphIndex = 0;
    // If asked, check first for variant glyph with the given Unicode variation
    // selector. This is quite uncommon so we don't bother with caching here.
    if (aVS && pFT_Face_GetCharVariantIndex)
        nGlyphIndex = (*pFT_Face_GetCharVariantIndex)(maFaceFT, aChar, aVS);

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

    return nGlyphIndex;
}

// -----------------------------------------------------------------------

int ServerFont::FixupGlyphIndex( int nGlyphIndex, sal_UCS4 aChar ) const
{
    int nGlyphFlags = GF_NONE;

    // do glyph substitution if necessary
    // CJK vertical writing needs special treatment
    if( GetFontSelData().mbVertical )
    {
        // TODO: rethink when GSUB is used for non-vertical case
        GlyphSubstitution::const_iterator it = maGlyphSubstitution.find( nGlyphIndex );
        if( it == maGlyphSubstitution.end() )
        {
            int nTemp = GetVerticalChar( aChar );
            if( nTemp ) // is substitution possible
                nTemp = GetRawGlyphIndex( nTemp );
            if( nTemp ) // substitute manually if sensible
                nGlyphIndex = nTemp | (GF_GSUB | GF_ROTL);
            else
                nGlyphFlags |= GetVerticalFlags( aChar );
        }
        else
        {
            // for vertical GSUB also compensate for nOrientation=2700
            nGlyphIndex = (*it).second;
            nGlyphFlags |= GF_GSUB | GF_ROTL;
        }
    }

    if( nGlyphIndex != 0 )
        nGlyphIndex |= nGlyphFlags;

    return nGlyphIndex;
}


// -----------------------------------------------------------------------

int ServerFont::GetGlyphIndex( sal_UCS4 aChar ) const
{
    int nGlyphIndex = GetRawGlyphIndex( aChar );
    nGlyphIndex = FixupGlyphIndex( nGlyphIndex, aChar );
    return nGlyphIndex;
}

// -----------------------------------------------------------------------

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

// -----------------------------------------------------------------------

void ServerFont::InitGlyphData( int nGlyphIndex, GlyphData& rGD ) const
{
    if( maSizeFT )
        pFTActivateSize( maSizeFT );

    int nGlyphFlags;
    SplitGlyphFlags( *this, nGlyphIndex, nGlyphFlags );

    int nLoadFlags = mnLoadFlags;

//  if( mbArtItalic )
//      nLoadFlags |= FT_LOAD_NO_BITMAP;

    FT_Error rc = -1;
    rc = FT_Load_Glyph( maFaceFT, nGlyphIndex, nLoadFlags );

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
    if( mbArtBold && pFTEmbolden )
        (*pFTEmbolden)( maFaceFT->glyph );

    const int nCharWidth = bOriginallyZeroWidth ? 0 : lcl_GetCharWidth( maFaceFT, mfStretch, nGlyphFlags );
    rGD.SetCharWidth( nCharWidth );

    FT_Glyph pGlyphFT;
    rc = FT_Get_Glyph( maFaceFT->glyph, &pGlyphFT );

    ApplyGlyphTransform( nGlyphFlags, pGlyphFT, false );
    if( mbArtBold && pFTEmbolden && (nFTVERSION < 2200) ) // #i71094# workaround staircase bug
        pGlyphFT->advance.y = 0;
    rGD.SetDelta( (pGlyphFT->advance.x + 0x8000) >> 16, -((pGlyphFT->advance.y + 0x8000) >> 16) );

    FT_BBox aBbox;
    FT_Glyph_Get_CBox( pGlyphFT, FT_GLYPH_BBOX_PIXELS, &aBbox );
    if( aBbox.yMin > aBbox.yMax )   // circumvent freetype bug
    {
        int t=aBbox.yMin; aBbox.yMin=aBbox.yMax, aBbox.yMax=t;
    }

    rGD.SetOffset( aBbox.xMin, -aBbox.yMax );
    rGD.SetSize( Size( (aBbox.xMax-aBbox.xMin+1), (aBbox.yMax-aBbox.yMin) ) );

    FT_Done_Glyph( pGlyphFT );
}

// -----------------------------------------------------------------------

bool ServerFont::GetAntialiasAdvice( void ) const
{
    if( GetFontSelData().mbNonAntialiased || (mnPrioAntiAlias<=0) )
        return false;
    bool bAdviseAA = true;
    // TODO: also use GASP info
    return bAdviseAA;
}

// -----------------------------------------------------------------------

bool ServerFont::GetGlyphBitmap1( int nGlyphIndex, RawBitmap& rRawBitmap ) const
{
    if( maSizeFT )
        pFTActivateSize( maSizeFT );

    int nGlyphFlags;
    SplitGlyphFlags( *this, nGlyphIndex, nGlyphFlags );

    FT_Int nLoadFlags = mnLoadFlags;
    // #i70930# force mono-hinting for monochrome text
    if( nFTVERSION >= 2110 ) //#i71947# unless it looks worse
    {
        nLoadFlags &= ~0xF0000;
        nLoadFlags |= FT_LOAD_TARGET_MONO;
    }

    if( mbArtItalic )
        nLoadFlags |= FT_LOAD_NO_BITMAP;

    // for 0/90/180/270 degree fonts enable hinting even if not advisable
    // non-hinted and non-antialiased bitmaps just look too ugly
    if( (mnCos==0 || mnSin==0) && (mnPrioAutoHint > 0) )
        nLoadFlags &= ~FT_LOAD_NO_HINTING;

    if( mnPrioEmbedded <= mnPrioAutoHint )
        nLoadFlags |= FT_LOAD_NO_BITMAP;

    FT_Error rc = -1;
    rc = FT_Load_Glyph( maFaceFT, nGlyphIndex, nLoadFlags );

    if( rc != FT_Err_Ok )
        return false;

    if( mbArtBold && pFTEmbolden )
        (*pFTEmbolden)( maFaceFT->glyph );

    FT_Glyph pGlyphFT;
    rc = FT_Get_Glyph( maFaceFT->glyph, &pGlyphFT );
    if( rc != FT_Err_Ok )
        return false;

    int nAngle = ApplyGlyphTransform( nGlyphFlags, pGlyphFT, true );

    if( mbArtItalic )
    {
        FT_Matrix aMatrix;
        aMatrix.xx = aMatrix.yy = 0x10000L;
        aMatrix.xy = 0x6000L, aMatrix.yx = 0;
        FT_Glyph_Transform( pGlyphFT, &aMatrix, NULL );
    }

    // Check for zero area bounding boxes as this crashes some versions of FT.
    // This also provides a handy short cut as much of the code following
    //  becomes an expensive nop when a glyph covers no pixels.
    FT_BBox cbox;
    FT_Glyph_Get_CBox(pGlyphFT, ft_glyph_bbox_unscaled, &cbox);

    if( (cbox.xMax - cbox.xMin) == 0 || (cbox.yMax - cbox.yMin == 0) )
    {
        nAngle = 0;
        memset(&rRawBitmap, 0, sizeof rRawBitmap);
        FT_Done_Glyph( pGlyphFT );
        return true;
    }

    if( pGlyphFT->format != FT_GLYPH_FORMAT_BITMAP )
    {
        if( pGlyphFT->format == FT_GLYPH_FORMAT_OUTLINE )
            ((FT_OutlineGlyphRec*)pGlyphFT)->outline.flags |= FT_OUTLINE_HIGH_PRECISION;
        FT_Render_Mode nRenderMode = FT_RENDER_MODE_MONO;

        rc = FT_Glyph_To_Bitmap( &pGlyphFT, nRenderMode, NULL, sal_True );
        if( rc != FT_Err_Ok )
        {
            FT_Done_Glyph( pGlyphFT );
            return false;
        }
    }

    const FT_BitmapGlyph pBmpGlyphFT = reinterpret_cast<const FT_BitmapGlyph>(pGlyphFT);
    // NOTE: autohinting in FT<=2.0.2 miscalculates the offsets below by +-1
    rRawBitmap.mnXOffset        = +pBmpGlyphFT->left;
    rRawBitmap.mnYOffset        = -pBmpGlyphFT->top;

    const FT_Bitmap& rBitmapFT  = pBmpGlyphFT->bitmap;
    rRawBitmap.mnHeight         = rBitmapFT.rows;
    rRawBitmap.mnBitCount       = 1;
    if( mbArtBold && !pFTEmbolden )
    {
        rRawBitmap.mnWidth = rBitmapFT.width + 1;
        int nLineBytes = (rRawBitmap.mnWidth + 7) >> 3;
        rRawBitmap.mnScanlineSize  = (nLineBytes > rBitmapFT.pitch) ? nLineBytes : rBitmapFT.pitch;
    }
    else
    {
        rRawBitmap.mnWidth          = rBitmapFT.width;
        rRawBitmap.mnScanlineSize   = rBitmapFT.pitch;
    }

    const sal_uLong nNeededSize = rRawBitmap.mnScanlineSize * rRawBitmap.mnHeight;

    if( rRawBitmap.mnAllocated < nNeededSize )
    {
        rRawBitmap.mnAllocated = 2*nNeededSize;
        rRawBitmap.mpBits.reset(new unsigned char[ rRawBitmap.mnAllocated ]);
    }

    if( !mbArtBold || pFTEmbolden )
    {
        memcpy( rRawBitmap.mpBits.get(), rBitmapFT.buffer, nNeededSize );
    }
    else
    {
        memset( rRawBitmap.mpBits.get(), 0, nNeededSize );
        const unsigned char* pSrcLine = rBitmapFT.buffer;
        unsigned char* pDstLine = rRawBitmap.mpBits.get();
        for( int h = rRawBitmap.mnHeight; --h >= 0; )
        {
            memcpy( pDstLine, pSrcLine, rBitmapFT.pitch );
            pDstLine += rRawBitmap.mnScanlineSize;
            pSrcLine += rBitmapFT.pitch;
        }

        unsigned char* p = rRawBitmap.mpBits.get();
        for( sal_uLong y=0; y < rRawBitmap.mnHeight; y++ )
        {
            unsigned char nLastByte = 0;
            for( sal_uLong x=0; x < rRawBitmap.mnScanlineSize; x++ )
            {
            unsigned char nTmp = p[x] << 7;
            p[x] |= (p[x] >> 1) | nLastByte;
            nLastByte = nTmp;
            }
            p += rRawBitmap.mnScanlineSize;
        }
    }

    FT_Done_Glyph( pGlyphFT );

    // special case for 0/90/180/270 degree orientation
    switch( nAngle )
    {
        case  -900:
        case  +900:
        case +1800:
        case +2700:
            rRawBitmap.Rotate( nAngle );
            break;
    }

    return true;
}

// -----------------------------------------------------------------------

bool ServerFont::GetGlyphBitmap8( int nGlyphIndex, RawBitmap& rRawBitmap ) const
{
    if( maSizeFT )
        pFTActivateSize( maSizeFT );

    int nGlyphFlags;
    SplitGlyphFlags( *this, nGlyphIndex, nGlyphFlags );

    FT_Int nLoadFlags = mnLoadFlags;

    if( mbArtItalic )
        nLoadFlags |= FT_LOAD_NO_BITMAP;

    if( (nGlyphFlags & GF_UNHINTED) || (mnPrioAutoHint < mnPrioAntiAlias) )
        nLoadFlags |= FT_LOAD_NO_HINTING;

    if( mnPrioEmbedded <= mnPrioAntiAlias )
        nLoadFlags |= FT_LOAD_NO_BITMAP;

    FT_Error rc = -1;
    rc = FT_Load_Glyph( maFaceFT, nGlyphIndex, nLoadFlags );

    if( rc != FT_Err_Ok )
        return false;

    if( mbArtBold && pFTEmbolden )
        (*pFTEmbolden)( maFaceFT->glyph );

    FT_Glyph pGlyphFT;
    rc = FT_Get_Glyph( maFaceFT->glyph, &pGlyphFT );
    if( rc != FT_Err_Ok )
        return false;

    int nAngle = ApplyGlyphTransform( nGlyphFlags, pGlyphFT, true );

    if( mbArtItalic )
    {
        FT_Matrix aMatrix;
        aMatrix.xx = aMatrix.yy = 0x10000L;
        aMatrix.xy = 0x6000L, aMatrix.yx = 0;
        FT_Glyph_Transform( pGlyphFT, &aMatrix, NULL );
    }

    if( pGlyphFT->format == FT_GLYPH_FORMAT_OUTLINE )
        ((FT_OutlineGlyph)pGlyphFT)->outline.flags |= FT_OUTLINE_HIGH_PRECISION;

    bool bEmbedded = (pGlyphFT->format == FT_GLYPH_FORMAT_BITMAP);
    if( !bEmbedded )
    {
        rc = FT_Glyph_To_Bitmap( &pGlyphFT, FT_RENDER_MODE_NORMAL, NULL, sal_True );
        if( rc != FT_Err_Ok )
        {
            FT_Done_Glyph( pGlyphFT );
            return false;
        }
    }

    const FT_BitmapGlyph pBmpGlyphFT = reinterpret_cast<const FT_BitmapGlyph>(pGlyphFT);
    rRawBitmap.mnXOffset        = +pBmpGlyphFT->left;
    rRawBitmap.mnYOffset        = -pBmpGlyphFT->top;

    const FT_Bitmap& rBitmapFT  = pBmpGlyphFT->bitmap;
    rRawBitmap.mnHeight         = rBitmapFT.rows;
    rRawBitmap.mnWidth          = rBitmapFT.width;
    rRawBitmap.mnBitCount       = 8;
    rRawBitmap.mnScanlineSize   = bEmbedded ? rBitmapFT.width : rBitmapFT.pitch;
    if( mbArtBold && !pFTEmbolden )
    {
        ++rRawBitmap.mnWidth;
            ++rRawBitmap.mnScanlineSize;
    }
    rRawBitmap.mnScanlineSize = (rRawBitmap.mnScanlineSize + 3) & -4;

    const sal_uLong nNeededSize = rRawBitmap.mnScanlineSize * rRawBitmap.mnHeight;
    if( rRawBitmap.mnAllocated < nNeededSize )
    {
        rRawBitmap.mnAllocated = 2*nNeededSize;
        rRawBitmap.mpBits.reset(new unsigned char[ rRawBitmap.mnAllocated ]);
    }

    const unsigned char* pSrc = rBitmapFT.buffer;
    unsigned char* pDest = rRawBitmap.mpBits.get();
    if( !bEmbedded )
    {
        for( int y = rRawBitmap.mnHeight, x; --y >= 0 ; )
        {
            for( x = 0; x < rBitmapFT.width; ++x )
                *(pDest++) = *(pSrc++);
            for(; x < int(rRawBitmap.mnScanlineSize); ++x )
                *(pDest++) = 0;
        }
    }
    else
    {
        for( int y = rRawBitmap.mnHeight, x; --y >= 0 ; )
        {
            unsigned char nSrc = 0;
            for( x = 0; x < rBitmapFT.width; ++x, nSrc+=nSrc )
            {
                if( (x & 7) == 0 )
                    nSrc = *(pSrc++);
                *(pDest++) = (0x7F - nSrc) >> 8;
            }
            for(; x < int(rRawBitmap.mnScanlineSize); ++x )
                *(pDest++) = 0;
        }
    }

    if( mbArtBold && !pFTEmbolden )
    {
        // overlay with glyph image shifted by one left pixel
        unsigned char* p = rRawBitmap.mpBits.get();
        for( sal_uLong y=0; y < rRawBitmap.mnHeight; y++ )
        {
            unsigned char nLastByte = 0;
            for( sal_uLong x=0; x < rRawBitmap.mnWidth; x++ )
            {
                unsigned char nTmp = p[x];
                p[x] |= p[x] | nLastByte;
                nLastByte = nTmp;
            }
            p += rRawBitmap.mnScanlineSize;
        }
    }

    if( !bEmbedded && mbUseGamma )
    {
        unsigned char* p = rRawBitmap.mpBits.get();
        for( sal_uLong y=0; y < rRawBitmap.mnHeight; y++ )
        {
            for( sal_uLong x=0; x < rRawBitmap.mnWidth; x++ )
            {
                p[x] = aGammaTable[ p[x] ];
            }
            p += rRawBitmap.mnScanlineSize;
        }
    }

    FT_Done_Glyph( pGlyphFT );

    // special case for 0/90/180/270 degree orientation
    switch( nAngle )
    {
        case  -900:
        case  +900:
        case +1800:
        case +2700:
            rRawBitmap.Rotate( nAngle );
            break;
    }

    return true;
}

// -----------------------------------------------------------------------
// determine unicode ranges in font
// -----------------------------------------------------------------------

const ImplFontCharMap* ServerFont::GetImplFontCharMap( void ) const
{
    const ImplFontCharMap* pIFCMap = mpFontInfo->GetImplFontCharMap();
    return pIFCMap;
}

const ImplFontCharMap* FtFontInfo::GetImplFontCharMap( void )
{
    // check if the charmap is already cached
    if( mpFontCharMap )
        return mpFontCharMap;

    // get the charmap and cache it
    CmapResult aCmapResult;
    bool bOK = GetFontCodeRanges( aCmapResult );
    if( bOK )
        mpFontCharMap = new ImplFontCharMap( aCmapResult );
    else
               mpFontCharMap = ImplFontCharMap::GetDefaultMap();
    mpFontCharMap->AddReference();
    return mpFontCharMap;
}

// TODO: merge into method GetFontCharMap()
bool FtFontInfo::GetFontCodeRanges( CmapResult& rResult ) const
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
        sal_uInt32 cNext = cCode;
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
            rFontCapabilities.maUnicodeRange,
            rFontCapabilities.maCodePageRange,
            pOS2, nLength);
    }

    return bRet;
}

// -----------------------------------------------------------------------

sal_uLong ServerFont::GetKernPairs( ImplKernPairData** ppKernPairs ) const
{
    // if no kerning info is available in the font file
    *ppKernPairs = NULL;
    if( !FT_HAS_KERNING( maFaceFT ) || !FT_IS_SFNT( maFaceFT ) )
    {
        // then we have may have extra kerning info from e.g. psprint
        int nCount = mpFontInfo->GetExtraKernPairs( ppKernPairs );
        // scale the kern values to match the font size
        const FontSelectPattern& rFSD = GetFontSelData();
        int nFontWidth = rFSD.mnWidth ? rFSD.mnWidth : rFSD.mnHeight;
        ImplKernPairData* pKernPair = *ppKernPairs;
        for( int i = nCount; --i >= 0; ++pKernPair )
        {
            long& rVal = pKernPair->mnKern;
            rVal = ((rVal * nFontWidth) + 500) / 1000;
        }
        return nCount;
    }

    // when font faces of different sizes share the same maFaceFT
    // then we have to make sure that it uses the correct maSizeFT
    if( maSizeFT )
        pFTActivateSize( maSizeFT );

    // first figure out which glyph pairs are involved in kerning
    sal_uLong nKernLength = 0;
    const FT_Byte* const pKern = mpFontInfo->GetTable( "kern", &nKernLength );
    if( !pKern )
        return 0;

    // combine TTF/OTF tables from the font file to build a vector of
    // unicode kerning pairs using Freetype's glyph kerning calculation
    // for the kerning value

    // TODO: is it worth to share the glyph->unicode mapping between
    // different instances of the same font face?

    typedef std::vector<ImplKernPairData> KernVector;
    KernVector aKernGlyphVector;
    ImplKernPairData aKernPair;
    aKernPair.mnKern = 0; // To prevent "is used uninitialized" warning...

    const FT_Byte* pBuffer = pKern;
    sal_uLong nVersion = GetUShort( pBuffer+0 );
    sal_uInt16 nTableCnt = GetUShort( pBuffer+2 );

    // Microsoft/Old TrueType style kern table
    if ( nVersion == 0 )
    {
        pBuffer += 4;

        for( sal_uInt16 nTableIdx = 0; nTableIdx < nTableCnt; ++nTableIdx )
        {
            // sal_uInt16 nSubVersion  = GetUShort( pBuffer+0 );
            // sal_uInt16 nSubLength   = GetUShort( pBuffer+2 );
            sal_uInt16 nSubCoverage = GetUShort( pBuffer+4 );
            pBuffer += 6;
            if( (nSubCoverage&0x03) != 0x01 )   // no interest in minimum info here
                continue;
            switch( nSubCoverage >> 8 )
            {
                case 0: // version 0, kerning format 0
                {
                    sal_uInt16 nPairs = GetUShort( pBuffer );
                    pBuffer += 8;   // skip search hints
                    aKernGlyphVector.reserve( aKernGlyphVector.size() + nPairs );
                    for( int i = 0; i < nPairs; ++i )
                    {
                        aKernPair.mnChar1 = GetUShort( pBuffer+0 );
                        aKernPair.mnChar2 = GetUShort( pBuffer+2 );
                        //long nUnscaledKern= GetSShort( pBuffer );
                        pBuffer += 6;
                        aKernGlyphVector.push_back( aKernPair );
                    }
                }
                break;

                case 2: // version 0, kerning format 2
                {
                    const FT_Byte* pSubTable = pBuffer;
                    //sal_uInt16 nRowWidth  = GetUShort( pBuffer+0 );
                    sal_uInt16 nOfsLeft     = GetUShort( pBuffer+2 );
                    sal_uInt16 nOfsRight    = GetUShort( pBuffer+4 );
                    sal_uInt16 nOfsArray    = GetUShort( pBuffer+6 );
                    pBuffer += 8;

                    const FT_Byte* pTmp = pSubTable + nOfsLeft;
                    sal_uInt16 nFirstLeft   = GetUShort( pTmp+0 );
                    sal_uInt16 nLastLeft    = GetUShort( pTmp+2 ) + nFirstLeft - 1;

                    pTmp = pSubTable + nOfsRight;
                    sal_uInt16 nFirstRight  = GetUShort( pTmp+0 );
                    sal_uInt16 nLastRight   = GetUShort( pTmp+2 ) + nFirstRight - 1;

                    sal_uLong nPairs = (sal_uLong)(nLastLeft - nFirstLeft + 1) * (nLastRight - nFirstRight + 1);
                    aKernGlyphVector.reserve( aKernGlyphVector.size() + nPairs );

                    pTmp = pSubTable + nOfsArray;
                    for( int nLeft = nFirstLeft; nLeft < nLastLeft; ++nLeft )
                    {
                        aKernPair.mnChar1 = nLeft;
                        for( int nRight = 0; nRight < nLastRight; ++nRight )
                        {
                            if( GetUShort( pTmp ) != 0 )
                            {
                                aKernPair.mnChar2 = nRight;
                                aKernGlyphVector.push_back( aKernPair );
                            }
                            pTmp += 2;
                        }
                    }
                }
                break;
            }
        }
    }

    // Apple New style kern table
    pBuffer = pKern;
    nVersion = NEXT_U32( pBuffer );
    nTableCnt = NEXT_U32( pBuffer );
    if ( nVersion == 0x00010000 )
    {
        for( sal_uInt16 nTableIdx = 0; nTableIdx < nTableCnt; ++nTableIdx )
        {
            /*sal_uLong  nLength  =*/ NEXT_U32( pBuffer );
            sal_uInt16 nCoverage   = NEXT_U16( pBuffer );
            /*sal_uInt16 nTupleIndex =*/ NEXT_U16( pBuffer );

            // Kerning sub-table format, 0 through 3
            sal_uInt8 nSubTableFormat  = nCoverage & 0x00FF;

            switch( nSubTableFormat )
            {
                case 0: // version 0, kerning format 0
                {
                    sal_uInt16 nPairs = NEXT_U16( pBuffer );
                    pBuffer += 6;   // skip search hints
                    aKernGlyphVector.reserve( aKernGlyphVector.size() + nPairs );
                    for( int i = 0; i < nPairs; ++i )
                    {
                        aKernPair.mnChar1 = NEXT_U16( pBuffer );
                        aKernPair.mnChar2 = NEXT_U16( pBuffer );
                        /*long nUnscaledKern=*/ NEXT_S16( pBuffer );
                        aKernGlyphVector.push_back( aKernPair );
                    }
                }
                break;

                case 2: // version 0, kerning format 2
                {
                    const FT_Byte* pSubTable = pBuffer;
                    /*sal_uInt16 nRowWidth  =*/ NEXT_U16( pBuffer );
                    sal_uInt16 nOfsLeft     = NEXT_U16( pBuffer );
                    sal_uInt16 nOfsRight    = NEXT_U16( pBuffer );
                    sal_uInt16 nOfsArray    = NEXT_U16( pBuffer );

                    const FT_Byte* pTmp = pSubTable + nOfsLeft;
                    sal_uInt16 nFirstLeft   = NEXT_U16( pTmp );
                    sal_uInt16 nLastLeft    = NEXT_U16( pTmp ) + nFirstLeft - 1;

                    pTmp = pSubTable + nOfsRight;
                    sal_uInt16 nFirstRight  = NEXT_U16( pTmp );
                    sal_uInt16 nLastRight   = NEXT_U16( pTmp ) + nFirstRight - 1;

                    sal_uLong nPairs = (sal_uLong)(nLastLeft - nFirstLeft + 1) * (nLastRight - nFirstRight + 1);
                    aKernGlyphVector.reserve( aKernGlyphVector.size() + nPairs );

                    pTmp = pSubTable + nOfsArray;
                    for( int nLeft = nFirstLeft; nLeft < nLastLeft; ++nLeft )
                    {
                        aKernPair.mnChar1 = nLeft;
                        for( int nRight = 0; nRight < nLastRight; ++nRight )
                        {
                            if( NEXT_S16( pTmp ) != 0 )
                            {
                                aKernPair.mnChar2 = nRight;
                                aKernGlyphVector.push_back( aKernPair );
                            }
                        }
                    }
                }
                break;

                default:
                    fprintf( stderr, "gcach_ftyp.cxx:  Found unsupported Apple-style kern subtable type %d.\n", nSubTableFormat );
                    break;
            }
        }
    }

    // now create VCL's ImplKernPairData[] format for all glyph pairs
    sal_uLong nKernCount = aKernGlyphVector.size();
    if( nKernCount )
    {
        // prepare glyphindex to character mapping
        // TODO: this is needed to support VCL's existing kerning infrastructure,
        // eliminate it up by redesigning kerning infrastructure to work with glyph indizes
        typedef boost::unordered_multimap<sal_uInt16,sal_Unicode> Cmap;
        Cmap aCmap;
        for( sal_Unicode aChar = 0x0020; aChar < 0xFFFE; ++aChar )
        {
            sal_uInt16 nGlyphIndex = GetGlyphIndex( aChar );
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
                FT_KERNING_DEFAULT, &aKernVal );
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
                PolyArgs( PolyPolygon& rPolyPoly, sal_uInt16 nMaxPoints );
                ~PolyArgs();

    void        AddPoint( long nX, long nY, PolyFlags);
    void        ClosePolygon();

    long        GetPosX() const { return maPosition.x;}
    long        GetPosY() const { return maPosition.y;}

private:
    PolyPolygon& mrPolyPoly;

    Point*      mpPointAry;
    sal_uInt8*       mpFlagAry;

    FT_Vector   maPosition;
    sal_uInt16      mnMaxPoints;
    sal_uInt16      mnPoints;
    sal_uInt16      mnPoly;
    bool        bHasOffline;
};

// -----------------------------------------------------------------------

PolyArgs::PolyArgs( PolyPolygon& rPolyPoly, sal_uInt16 nMaxPoints )
:   mrPolyPoly(rPolyPoly),
    mnMaxPoints(nMaxPoints),
    mnPoints(0),
    mnPoly(0),
    bHasOffline(false)
{
    mpPointAry  = new Point[ mnMaxPoints ];
    mpFlagAry   = new sal_uInt8 [ mnMaxPoints ];
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
    if( mnPoints >= mnMaxPoints )
        return;

    maPosition.x = nX;
    maPosition.y = nY;
    mpPointAry[ mnPoints ] = Point( nX, nY );
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

// -----------------------------------------------------------------------

extern "C" {

// TODO: wait till all compilers accept that calling conventions
// for functions are the same independent of implementation constness,
// then uncomment the const-tokens in the function interfaces below
static int FT_move_to( FT_Vector_CPtr p0, void* vpPolyArgs )
{
    PolyArgs& rA = *reinterpret_cast<PolyArgs*>(vpPolyArgs);

    // move_to implies a new polygon => finish old polygon first
    rA.ClosePolygon();

    rA.AddPoint( p0->x, p0->y, POLY_NORMAL );
    return 0;
}

static int FT_line_to( FT_Vector_CPtr p1, void* vpPolyArgs )
{
    PolyArgs& rA = *reinterpret_cast<PolyArgs*>(vpPolyArgs);
    rA.AddPoint( p1->x, p1->y, POLY_NORMAL );
    return 0;
}

static int FT_conic_to( FT_Vector_CPtr p1, FT_Vector_CPtr p2, void* vpPolyArgs )
{
    PolyArgs& rA = *reinterpret_cast<PolyArgs*>(vpPolyArgs);

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

static int FT_cubic_to( FT_Vector_CPtr p1, FT_Vector_CPtr p2, FT_Vector_CPtr p3, void* vpPolyArgs )
{
    PolyArgs& rA = *reinterpret_cast<PolyArgs*>(vpPolyArgs);
    rA.AddPoint( p1->x, p1->y, POLY_CONTROL );
    rA.AddPoint( p2->x, p2->y, POLY_CONTROL );
    rA.AddPoint( p3->x, p3->y, POLY_NORMAL );
    return 0;
}

} // extern "C"

// -----------------------------------------------------------------------

bool ServerFont::GetGlyphOutline( int nGlyphIndex,
    ::basegfx::B2DPolyPolygon& rB2DPolyPoly ) const
{
    if( maSizeFT )
        pFTActivateSize( maSizeFT );

    rB2DPolyPoly.clear();

    int nGlyphFlags;
    SplitGlyphFlags( *this, nGlyphIndex, nGlyphFlags );

    FT_Int nLoadFlags = FT_LOAD_DEFAULT | FT_LOAD_IGNORE_TRANSFORM;

#ifdef FT_LOAD_TARGET_LIGHT
    // enable "light hinting" if available
    nLoadFlags |= FT_LOAD_TARGET_LIGHT;
#endif

    FT_Error rc = FT_Load_Glyph( maFaceFT, nGlyphIndex, nLoadFlags );
    if( rc != FT_Err_Ok )
        return false;

    if( mbArtBold && pFTEmbolden )
        (*pFTEmbolden)( maFaceFT->glyph );

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
        aMatrix.xy = 0x6000L, aMatrix.yx = 0;
        FT_Glyph_Transform( pGlyphFT, &aMatrix, NULL );
    }

    FT_Outline& rOutline = reinterpret_cast<FT_OutlineGlyphRec*>(pGlyphFT)->outline;
    if( !rOutline.n_points )    // blank glyphs are ok
    {
        FT_Done_Glyph( pGlyphFT );
        return true;
    }

    long nMaxPoints = 1 + rOutline.n_points * 3;
    PolyPolygon aToolPolyPolygon;
    PolyArgs aPolyArg( aToolPolyPolygon, nMaxPoints );

    /*int nAngle =*/ ApplyGlyphTransform( nGlyphFlags, pGlyphFT, false );

    FT_Outline_Funcs aFuncs;
    aFuncs.move_to  = &FT_move_to;
    aFuncs.line_to  = &FT_line_to;
    aFuncs.conic_to = &FT_conic_to;
    aFuncs.cubic_to = &FT_cubic_to;
    aFuncs.shift    = 0;
    aFuncs.delta    = 0;
    rc = FT_Outline_Decompose( &rOutline, &aFuncs, (void*)&aPolyArg );
    aPolyArg.ClosePolygon();    // close last polygon
    FT_Done_Glyph( pGlyphFT );

    // convert to basegfx polypolygon
    // TODO: get rid of the intermediate tools polypolygon
    rB2DPolyPoly = aToolPolyPolygon.getB2DPolyPolygon();
    rB2DPolyPoly.transform(basegfx::tools::createScaleB2DHomMatrix( +1.0/(1<<6), -1.0/(1<<6) ));

    return true;
}

// -----------------------------------------------------------------------

bool ServerFont::ApplyGSUB( const FontSelectPattern& rFSD )
{
#define MKTAG(s) ((((((s[0]<<8)+s[1])<<8)+s[2])<<8)+s[3])

    typedef std::vector<sal_uLong> ReqFeatureTagList;
    ReqFeatureTagList aReqFeatureTagList;
    if( rFSD.mbVertical )
        aReqFeatureTagList.push_back( MKTAG("vert") );
    sal_uLong nRequestedScript = 0;     //MKTAG("hani");//### TODO: where to get script?
    sal_uLong nRequestedLangsys = 0;    //MKTAG("ZHT"); //### TODO: where to get langsys?
    // TODO: request more features depending on script and language system

    if( aReqFeatureTagList.empty()) // nothing to do
        return true;

    // load GSUB table into memory
    sal_uLong nLength = 0;
    const FT_Byte* const pGsubBase = mpFontInfo->GetTable( "GSUB", &nLength );
    if( !pGsubBase )
        return false;

    // parse GSUB header
    const FT_Byte* pGsubHeader = pGsubBase;
    const sal_uInt16 nOfsScriptList     = GetUShort( pGsubHeader+4 );
    const sal_uInt16 nOfsFeatureTable   = GetUShort( pGsubHeader+6 );
    const sal_uInt16 nOfsLookupList     = GetUShort( pGsubHeader+8 );
    pGsubHeader += 10;

    typedef std::vector<sal_uInt16> UshortList;
    UshortList aFeatureIndexList;

    // parse Script Table
    const FT_Byte* pScriptHeader = pGsubBase + nOfsScriptList;
    const sal_uInt16 nCntScript = GetUShort( pScriptHeader+0 );
    pScriptHeader += 2;
    for( sal_uInt16 nScriptIndex = 0; nScriptIndex < nCntScript; ++nScriptIndex )
    {
        const sal_uLong nScriptTag      = GetUInt( pScriptHeader+0 ); // e.g. hani/arab/kana/hang
        const sal_uInt16 nOfsScriptTable= GetUShort( pScriptHeader+4 );
        pScriptHeader += 6; //###
        if( (nScriptTag != nRequestedScript) && (nRequestedScript != 0) )
            continue;

        const FT_Byte* pScriptTable     = pGsubBase + nOfsScriptList + nOfsScriptTable;
        const sal_uInt16 nDefaultLangsysOfs = GetUShort( pScriptTable+0 );
        const sal_uInt16 nCntLangSystem     = GetUShort( pScriptTable+2 );
        pScriptTable += 4;
        sal_uInt16 nLangsysOffset = 0;

        for( sal_uInt16 nLangsysIndex = 0; nLangsysIndex < nCntLangSystem; ++nLangsysIndex )
        {
            const sal_uLong nTag    = GetUInt( pScriptTable+0 );    // e.g. KOR/ZHS/ZHT/JAN
            const sal_uInt16 nOffset= GetUShort( pScriptTable+4 );
            pScriptTable += 6;
            if( (nTag != nRequestedLangsys) && (nRequestedLangsys != 0) )
                continue;
            nLangsysOffset = nOffset;
            break;
        }

        if( (nDefaultLangsysOfs != 0) && (nDefaultLangsysOfs != nLangsysOffset) )
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
        return true;

    UshortList aLookupIndexList;
    UshortList aLookupOffsetList;

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

    UshortList::const_iterator lookup_it = aLookupOffsetList.begin();
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
            typedef std::vector<GlyphSubst> SubstVector;
            SubstVector aSubstVector;

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

            SubstVector::iterator it( aSubstVector.begin() );

            switch( nFmtSubstitution )
            {
                case 1:     // Single Substitution Format 1
                    {
                        const sal_uInt16 nDeltaGlyphId = GetUShort( pSubLookup );
                        pSubLookup += 2;
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

            DBG_ASSERT( (it == aSubstVector.end()), "lookup<->coverage table mismatch" );
            // now apply the glyph substitutions that have been collected in this subtable
            for( it = aSubstVector.begin(); it != aSubstVector.end(); ++it )
                maGlyphSubstitution[ (*it).first ] =  (*it).second;
        }
    }

    return true;
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

// =======================================================================

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
