/*************************************************************************
 *
 *  $RCSfile: gcach_ftyp.cxx,v $
 *
 *  $Revision: 1.98 $
 *  last change: $Author: hr $ $Date: 2003-07-16 17:46:21 $
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

#include "freetype/freetype.h"
#include "freetype/ftglyph.h"
#include "freetype/ftoutln.h"
#include "freetype/tttables.h"
#include "freetype/tttags.h"
#include "freetype/ttnameid.h"

#ifndef FREETYPE_PATCH
    // VERSION_MINOR in freetype.h is too coarse
    // if patch-level is not available we need to fine-tune the version ourselves
    #define FTVERSION 2005
#else
    #define FTVERSION (1000*FREETYPE_MAJOR + 100*FREETYPE_MINOR + FREETYPE_PATCH)
#endif

#include <vector>

// TODO: move file mapping stuff to OSL
#if defined(UNX)
    #if !defined(MACOSX) && !defined(HPUX)
        // PORTERS: dlfcn is used for code dependend on FT version
        #include <dlfcn.h>
    #endif
    #include <unistd.h>
    #include <fcntl.h>
    #include <sys/stat.h>
    #include <sys/mman.h>
#elif defined(WIN32)
    #include <io.h>
    #define strncasecmp strnicmp
#endif

#include "freetype/internal/ftobjs.h"
#include "freetype/internal/sfnt.h"
#include "freetype/internal/ftstream.h"

#include <svapp.hxx>
#include <settings.hxx>
#include <tools/lang.hxx>

// -----------------------------------------------------------------------

static FT_Library aLibFT = 0;

// #110607# enable linking with old FT versions
static int nFTVERSION = 0;
static FT_Error (*pFTNewSize)(FT_Face,FT_Size*);
static FT_Error (*pFTActivateSize)(FT_Size);
static FT_Error (*pFTDoneSize)(FT_Size);
static bool bEnableSizeFT = false;

struct EqStr{ bool operator()(const char* a, const char* b) const { return !strcmp(a,b); } };
typedef ::std::hash_map<const char*,FtFontFile*,::std::hash<const char*>, EqStr> FontFileList;
static FontFileList aFontFileList;

// -----------------------------------------------------------------------

// TODO: remove when the priorities are selected by UI
// if (AH==0) => disable autohinting
// if (AA==0) => disable antialiasing
// if (EB==0) => disable embedded bitmaps
// if (AA prio <= AH prio) => antialias + autohint
// if (AH<AA) => do not autohint when antialiasing
// if (EB<AH) => do not autohint for monochrome
static int nPrioEmbedded    = 2;
static int nPrioAutoHint    = 1;
static int nPrioAntiAlias   = 1;

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
    pFileName = pFontFile->maNativeFileName.getStr();
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
            mmap( NULL, mnFileSize, PROT_READ, MAP_SHARED, nFile, 0 );
        close( nFile );
#elif defined(WIN32)
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
    mnSynthetic( nSynthetic ),
    maFaceFT( NULL ),
    mnRefCount( 0 )
{
    maFontData.mpSysData = (void*)nFontId;
    maFontData.mpNext    = NULL;

    // using unicode emulation for non-symbol fonts
    if( maFontData.meCharSet != RTL_TEXTENCODING_SYMBOL )
        maFontData.meCharSet = RTL_TEXTENCODING_UNICODE;

    // boost low font IDs
    maFontData.mnQuality += 0x1000 - nFontId;

    const char* pLangBoost = NULL;
    const LanguageType aLang = Application::GetSettings().GetUILanguage();
    switch( aLang )
    {
        case LANGUAGE_JAPANESE:
            pLangBoost = "jan";    // japanese is default
            break;
        case LANGUAGE_CHINESE:
        case LANGUAGE_CHINESE_SIMPLIFIED:
        case LANGUAGE_CHINESE_SINGAPORE:
            pLangBoost = "zhs";
            break;
        case LANGUAGE_CHINESE_TRADITIONAL:
        case LANGUAGE_CHINESE_HONGKONG:
        case LANGUAGE_CHINESE_MACAU:
            pLangBoost = "zht";
            break;
        case LANGUAGE_KOREAN:
        case LANGUAGE_KOREAN_JOHAB:
            pLangBoost = "kor";
            break;
    }

    // boost font preference if UI language is mentioned in filename
    int nPos = rNativeFileName.lastIndexOf( '_' );
    if( nPos == -1 || rNativeFileName[nPos+1] == '.' )
        maFontData.mnQuality += 0x1000;     // no langinfo => good
    else
    {
        if( pLangBoost && !strncasecmp( pLangBoost, &rNativeFileName.getStr()[nPos+1], 3 ) )
           maFontData.mnQuality += 0x2000;  // correct langinfo => better
    }
}

// -----------------------------------------------------------------------

FT_FaceRec_* FtFontInfo::GetFaceFT()
{
    // get faceFT once/multiple depending on SizeFT availability
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

static unsigned GetUInt( const unsigned char* p ) { return((p[0]<<24)+(p[1]<<16)+(p[2]<<8)+p[3]);}
static unsigned GetUShort( const unsigned char* p ){ return((p[0]<<8)+p[1]);}
static signed GetSShort( const unsigned char* p ){ return((short)((p[0]<<8)+p[1]));}

// -----------------------------------------------------------------------

const unsigned char* FtFontInfo::GetTable( const char* pTag, ULONG* pLength ) const
{
    const unsigned char* pBuffer = mpFontFile->GetBuffer();
    int nFileSize = mpFontFile->GetFileSize();
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
            if( (pTable + nLength) <= (mpFontFile->GetBuffer() + nFileSize) )
                return pTable;
        }
    }

    return NULL;
}

// =======================================================================

FreetypeManager::FreetypeManager()
:   mnNextFontId( 0x1000 ),
    mnMaxFontId( 0 )
{
    FT_Error rcFT = FT_Init_FreeType( &aLibFT );

#ifdef RTLD_DEFAULT // true if a good dlfcn.h header was included
    // Get version of freetype library to enable workarounds.
    // Freetype <= 2.0.9 does not have FT_Library_Version().
    // Using dl_sym() instead of osl_getSymbol() because latter
    // isn't designed to work with oslModule=NULL
    void (*pFTLibraryVersion)(FT_Library library,
        FT_Int *amajor, FT_Int *aminor, FT_Int *apatch);
    pFTLibraryVersion = (void (*)(FT_Library library,
        FT_Int *amajor, FT_Int *aminor, FT_Int *apatch)) dlsym( RTLD_DEFAULT, "FT_Library_Version" );

    pFTNewSize      = (FT_Error(*)(FT_Face,FT_Size*)) dlsym( RTLD_DEFAULT, "FT_New_Size" );
    pFTActivateSize = (FT_Error(*)(FT_Size)) dlsym( RTLD_DEFAULT, "FT_Activate_Size" );
    pFTDoneSize     = (FT_Error(*)(FT_Size)) dlsym( RTLD_DEFAULT, "FT_Done_Size" );

    bEnableSizeFT = (pFTNewSize!=NULL) && (pFTActivateSize!=NULL) && (pFTDoneSize!=NULL);

    FT_Int nMajor = 0, nMinor = 0, nPatch = 0;
    if( pFTLibraryVersion )
        pFTLibraryVersion( aLibFT, &nMajor, &nMinor, &nPatch );
    nFTVERSION = nMajor * 1000 + nMinor * 100 + nPatch;

    // disable embedded bitmaps for Freetype-2.1.3 unless explicitly
    // requested by env var below because it crashes StarOffice on RH9
    // TODO: investigate
    if( nFTVERSION == 2103 )
        nPrioEmbedded = 0;

#else // RTLD_DEFAULT
    // assume systems where dlsym is not possible use supplied library
    nFTVERSION = FTVERSION;
#endif

    // TODO: remove when the priorities are selected by UI
    char* pEnv;
    pEnv = ::getenv( "SAL_EMBEDDED_BITMAP_PRIORITY" );
    if( pEnv )
        nPrioEmbedded  = pEnv[0] - '0';
    pEnv = ::getenv( "SAL_ANTIALIASED_TEXT_PRIORITY" );
    if( pEnv )
        nPrioAntiAlias = pEnv[0] - '0';
    pEnv = ::getenv( "SAL_AUTOHINTING_PRIORITY" );
    if( pEnv )
        nPrioAutoHint  = pEnv[0] - '0';
}

// -----------------------------------------------------------------------

FreetypeManager::~FreetypeManager()
{
    FT_Error rcFT = FT_Done_FreeType( aLibFT );
}

// -----------------------------------------------------------------------

void* FreetypeManager::GetFontHandle( int nFontId )
{
    return (void*)nFontId;
}

// -----------------------------------------------------------------------

void FreetypeManager::AddFontFile( const rtl::OString& rNormalizedName,
    int nFaceNum, int nFontId, const ImplFontData* pData )
{
    if( !rNormalizedName.getLength() )
        return;

    if( maFontList.find( nFontId ) != maFontList.end() )
        return;

    FtFontInfo* pFI = new FtFontInfo( *pData, rNormalizedName, nFaceNum, nFontId, 0 );
    maFontList[ nFontId ] = pFI;
    if( mnMaxFontId < nFontId )
        mnMaxFontId = nFontId;
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
#if (FTVERSION < 2000)
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

            aFontData.mbOrientation = true;
            aFontData.mbDevice      = false;
            aFontData.mnQuality     = 0;    // prefer client-side fonts if available

            AddFontFile( aCFileName, nFaceNum, ++mnNextFontId, &aFontData );
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
    {
        const FtFontInfo& rFFI = *it->second;
        ImplFontData* pFontData = new ImplFontData( rFFI.GetFontData() );
        pToAdd->Add( pFontData );
    }

    return nCount;
}

// -----------------------------------------------------------------------

void FreetypeManager::ClearFontList( )
{
    for( FontList::iterator it = maFontList.begin(); it != maFontList.end(); ++it )
        delete it->second;
    maFontList.clear();
}

// -----------------------------------------------------------------------

FreetypeServerFont* FreetypeManager::CreateFont( const ImplFontSelectData& rFSD )
{
    int nFontId = (int)rFSD.mpFontData->mpSysData;
    FontList::iterator it = maFontList.find( nFontId );
    if( it != maFontList.end() )
    {
        FtFontInfo* pFI = it->second;
        FreetypeServerFont* pFont = new FreetypeServerFont( rFSD, pFI );
        return pFont;
    }

    return NULL;
}

// =======================================================================
// FreetypeServerFont
// =======================================================================

FreetypeServerFont::FreetypeServerFont( const ImplFontSelectData& rFSD, FtFontInfo* pFI )
:   ServerFont( rFSD ),
    mpFontInfo( pFI ),
    maFaceFT( NULL ),
    maSizeFT( NULL ),
    maRecodeConverter( NULL ),
    mpLayoutEngine( NULL )
{
#ifdef HDU_DEBUG
    fprintf( stderr, "FTSF::FTSF(\"%s\", h=%d, w=%d, cs=%d)\n",
        pFI->GetFontFileName()->getStr(), rFSD.mnHeight, rFSD.mnWidth,
        pFI->GetFontData().meCharSet );
#endif

    maFaceFT = pFI->GetFaceFT();
    if( !maFaceFT )
        return;

    FT_Encoding eEncoding = ft_encoding_unicode;
    if( mpFontInfo->GetFontData().meCharSet == RTL_TEXTENCODING_SYMBOL )
    {
#if (FTVERSION < 2000)
        eEncoding = ft_encoding_none;
#else
        if( FT_IS_SFNT( maFaceFT ) )
            eEncoding = ft_encoding_symbol;
        else
            eEncoding = ft_encoding_adobe_custom; // freetype wants this for PS symbol fonts
#endif
    }
    FT_Error rc = FT_Select_Charmap( maFaceFT, eEncoding );

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
                        eEncoding = ft_encoding_sjis;
                        eRecodeFrom = RTL_TEXTENCODING_SHIFT_JIS;
                        break;
                    case TT_MS_ID_GB2312:
                        eEncoding = ft_encoding_gb2312;
                        eRecodeFrom = RTL_TEXTENCODING_GB_2312;
                        break;
                    case TT_MS_ID_BIG_5:
                        eEncoding = ft_encoding_big5;
                        eRecodeFrom = RTL_TEXTENCODING_BIG5;
                        break;
                    case TT_MS_ID_WANSUNG:
                        eEncoding = ft_encoding_wansung;
                        eRecodeFrom = RTL_TEXTENCODING_MS_949;
                        break;
                    case TT_MS_ID_JOHAB:
                        eEncoding = ft_encoding_johab;
                        eRecodeFrom = RTL_TEXTENCODING_MS_1361;
                        break;
                }
            }
            else if( aCM->platform_id == TT_PLATFORM_MACINTOSH )
            {
                switch( aCM->encoding_id )
                {
                    case TT_MAC_ID_ROMAN:   // better unicode than nothing
                        eEncoding = ft_encoding_apple_roman;
                        break;
                    // TODO: add other encodings when Mac-only non-unicode
                    //       fonts show up
                }
            }
        }

        if( FT_Err_Ok != FT_Select_Charmap( maFaceFT, eEncoding ) )
        {
            maFaceFT->num_glyphs = 0;
            return;
        }

        if( eRecodeFrom != RTL_TEXTENCODING_UNICODE )
            maRecodeConverter = rtl_createUnicodeToTextConverter( eRecodeFrom );
    }

    if( bEnableSizeFT )
    {
        pFTNewSize( maFaceFT, &maSizeFT );
        pFTActivateSize( maSizeFT );
    }

    mnWidth = rFSD.mnWidth;
    if( !mnWidth )
        mnWidth = rFSD.mnHeight;
    mfStretch = (double)mnWidth / rFSD.mnHeight;
    rc = FT_Set_Pixel_Sizes( maFaceFT, mnWidth, rFSD.mnHeight );

    ApplyGSUB( rFSD );

    // TODO: query GASP table for load flags
    mnLoadFlags = FT_LOAD_DEFAULT;

    if( (nSin != 0) && (nCos != 0) ) // hinting for 0/90/180/270 degrees only
        mnLoadFlags |= FT_LOAD_NO_HINTING;
    mnLoadFlags |= FT_LOAD_IGNORE_GLOBAL_ADVANCE_WIDTH; //#88334#

#if (FTVERSION >= 2005) || defined(TT_CONFIG_OPTION_BYTECODE_INTERPRETER)
    if( nPrioAutoHint <= 0 )
#endif
        mnLoadFlags |= FT_LOAD_NO_HINTING;

    if( ((nCos != 0) && (nSin != 0)) || (nPrioEmbedded <= 0) )
        mnLoadFlags |= FT_LOAD_NO_BITMAP;
}

// -----------------------------------------------------------------------

bool FreetypeServerFont::TestFont() const
{
    return (maFaceFT != NULL) && (maFaceFT->num_glyphs > 0);
}

// -----------------------------------------------------------------------

FreetypeServerFont::~FreetypeServerFont()
{
    if( mpLayoutEngine )
        delete mpLayoutEngine;

    if( maRecodeConverter )
        rtl_destroyUnicodeToTextConverter( maRecodeConverter );

    if( maSizeFT )
        pFTDoneSize( maSizeFT );

    mpFontInfo->ReleaseFaceFT( maFaceFT );
}

 // -----------------------------------------------------------------------

int FreetypeServerFont::GetEmUnits() const
{
    return maFaceFT->units_per_EM;
}

// -----------------------------------------------------------------------

void FreetypeServerFont::FetchFontMetric( ImplFontMetricData& rTo, long& rFactor ) const
{
    if( maSizeFT )
        pFTActivateSize( maSizeFT );

    rFactor = 0x100;

    rTo.mnWidth             = mnWidth;

    const FT_Size_Metrics& rMetrics = maFaceFT->size->metrics;
    rTo.mnAscent            = (+rMetrics.ascender + 32) >> 6;
#if (FTVERSION < 2000)
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
    const TT_HoriHeader* pHHEA = (const TT_HoriHeader*)FT_Get_Sfnt_Table( maFaceFT, ft_sfnt_hhea );
    if( pOS2 && (~pOS2->version != 0) )
    {
        // #108862# sanity check, some fonts treat descent as signed !!!
        int nDescent = pOS2->usWinDescent;
        if( nDescent > 5*maFaceFT->units_per_EM )
            nDescent = (short)pOS2->usWinDescent;  // interpret it as signed!

        const double fScale = (double)GetFontSelData().mnHeight / maFaceFT->units_per_EM;
        rTo.mnAscent        = (long)( +pOS2->usWinAscent * fScale + 0.5 );
        rTo.mnDescent       = (long)( +nDescent * fScale + 0.5 );
        rTo.mnLeading       = (long)( (+pOS2->usWinAscent + pOS2->usWinDescent - maFaceFT->units_per_EM) * fScale + 0.5 );

        // Check for CJK capabilities of the current font
        // #107888# workaround for Asian...
        BOOL bCJKCapable = ((( pOS2->ulUnicodeRange2 & 0x2fff0000 ) | ( pOS2->ulUnicodeRange3 & 0x00000001 )) != 0 );
        BOOL bHasKoreanRange = ((( pOS2->ulUnicodeRange1 & 0x10000000 ) | ( pOS2->ulUnicodeRange2 & 0x00100000 ) |
                                 ( pOS2->ulUnicodeRange2 & 0x01000000 )) != 0 );

        if ( bCJKCapable && pHHEA )
        {
            // formula taken from www.microsoft.com/typography/otspec/recom.htm
            long externalLeading = std::max(0, +pHHEA->Line_Gap - ((+pOS2->usWinAscent + +pOS2->usWinDescent)
                - (+pHHEA->Ascender - +pHHEA->Descender)));

            rTo.mnAscent    += (long)(+externalLeading * fScale + 0.5 );
            rTo.mnLeading   += (long)(+externalLeading * fScale + 0.5 );
            // #109280# korean only: increase descent for wavelines and improved line space
            if( bHasKoreanRange )
                rTo.mnDescent    += (long)(+externalLeading * fScale + 0.5 );
        }

        rTo.mnFirstChar     = pOS2->usFirstCharIndex;
        rTo.mnLastChar      = pOS2->usLastCharIndex;
    }
}

// -----------------------------------------------------------------------

static inline void SplitGlyphFlags( const FreetypeServerFont& rFont, int& nGlyphIndex, int& nGlyphFlags )
{
    nGlyphFlags = nGlyphIndex & GF_FLAGMASK;

    if( !(nGlyphIndex & GF_ISCHAR) )
        nGlyphIndex &= GF_IDXMASK;
    else
        nGlyphIndex = rFont.GetRawGlyphIndex( nGlyphIndex & GF_IDXMASK );
}

// -----------------------------------------------------------------------

int FreetypeServerFont::ApplyGlyphTransform( int nGlyphFlags, FT_GlyphRec_* pGlyphFT ) const
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
        aMatrix.xx = +nCos;
        aMatrix.yy = +nCos;
        aMatrix.xy = -nSin;
        aMatrix.yx = +nSin;
        break;
    case GF_ROTL:    // left
        nAngle += 900;
        bStretched = (mfStretch != 1.0);
        aVector.x = +rMetrics.descender * mfStretch;
        aVector.y = -rMetrics.ascender;
        aMatrix.xx = -nSin / mfStretch;
        aMatrix.yy = -nSin * mfStretch;
        aMatrix.xy = -nCos * mfStretch;
        aMatrix.yx = +nCos / mfStretch;
        break;
    case GF_ROTR:    // right
        nAngle -= 900;
        bStretched = (mfStretch != 1.0);
        aVector.x = -maFaceFT->glyph->metrics.horiAdvance;
        aVector.x += (rMetrics.descender * nSin/65536.0);
        aVector.y = -(rMetrics.descender * mfStretch * nCos/65536.0);
        aMatrix.xx = +nSin / mfStretch;
        aMatrix.yy = +nSin * mfStretch;
        aMatrix.xy = +nCos * mfStretch;
        aMatrix.yx = -nCos / mfStretch;
        break;
    }

    while( nAngle < 0 )
        nAngle += 3600;

    if( pGlyphFT->format != ft_glyph_format_bitmap )
    {
        FT_Glyph_Transform( pGlyphFT, NULL, &aVector );

        // orthogonal transforms are handled by bitmap operations
        // apply non-orthogonal or stretch transformations here
        if( (nAngle % 900) != 0 || bStretched )
        {
            FT_Glyph_Transform( pGlyphFT, &aMatrix, NULL );
            nAngle = 0;
        }
    }
    else
    {
        // FT<=2005 ignores transforms for bitmaps, so do it manually
        FT_BitmapGlyph& rBmpGlyphFT = reinterpret_cast<FT_BitmapGlyph&>(pGlyphFT);
        rBmpGlyphFT->left += (aVector.x + 32) >> 6;
        rBmpGlyphFT->top  += (aVector.y + 32) >> 6;
    }

    return nAngle;
}

// -----------------------------------------------------------------------

int FreetypeServerFont::GetRawGlyphIndex( sal_Unicode aChar ) const
{
    if( mpFontInfo->GetFontData().meCharSet == RTL_TEXTENCODING_SYMBOL )
    {
        if( FT_IS_SFNT( maFaceFT ) )
            aChar |= 0xF000;    // emulate W2K high/low mapping of symbols
        else
        {
            if( (aChar & 0xFF00) == 0xF000 )
                aChar &= 0xFF;    // PS font symbol mapping
            else if( aChar > 0xFF )
                return 0;
        }
    }

    // need to recode from unicode to font encoding?
    if( maRecodeConverter )
    {
        sal_Char aTempArray[8];
        sal_Size nTempSize;
        sal_uInt32 nCvtInfo;

        rtl_UnicodeToTextContext aContext = rtl_createUnicodeToTextContext( maRecodeConverter );
        int nChars = rtl_convertUnicodeToText( maRecodeConverter, aContext,
            &aChar, 1, aTempArray, sizeof(aTempArray),
            RTL_UNICODETOTEXT_FLAGS_UNDEFINED_QUESTIONMARK
            | RTL_UNICODETOTEXT_FLAGS_INVALID_QUESTIONMARK,
            &nCvtInfo, &nTempSize );
        rtl_destroyUnicodeToTextContext( maRecodeConverter, aContext );

        aChar = 0;
        for( int i = 0; i < nChars; ++i )
            aChar = aChar*256 + (aTempArray[i] & 0xFF);
    }

    // cache glyph indexes in font info to share between different sizes
    int nGlyphIndex = mpFontInfo->GetGlyphIndex( aChar );
    if( nGlyphIndex < 0 )
    {
        nGlyphIndex = FT_Get_Char_Index( maFaceFT, aChar );
        mpFontInfo->CacheGlyphIndex( aChar, nGlyphIndex );
    }

    return nGlyphIndex;
}

// -----------------------------------------------------------------------

int FreetypeServerFont::FixupGlyphIndex( int nGlyphIndex, sal_Unicode aChar ) const
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

#if !defined(TT_CONFIG_OPTION_BYTECODE_INTERPRETER)
    // #95556# autohinting not yet optimized for non-western glyph styles
    if( !(mnLoadFlags & FT_LOAD_NO_HINTING)
    &&  ( (aChar >= 0x0600 && aChar < 0x1E00)   // south-east asian + arabic
        ||(aChar >= 0x2900 && aChar < 0xD800)   // CJKV
        ||(aChar >= 0xF800) ) )                 // presentation + symbols
        nGlyphFlags |= GF_UNHINTED;
#endif

    if( nGlyphIndex != 0 )
        nGlyphIndex |= nGlyphFlags;

    return nGlyphIndex;
}


// -----------------------------------------------------------------------

int FreetypeServerFont::GetGlyphIndex( sal_Unicode aChar ) const
{
    int nGlyphIndex = GetRawGlyphIndex( aChar );
    nGlyphIndex = FixupGlyphIndex( nGlyphIndex, aChar );
    return nGlyphIndex;
}

// -----------------------------------------------------------------------

void FreetypeServerFont::InitGlyphData( int nGlyphIndex, GlyphData& rGD ) const
{
    if( maSizeFT )
        pFTActivateSize( maSizeFT );

    int nGlyphFlags;
    SplitGlyphFlags( *this, nGlyphIndex, nGlyphFlags );

    int nLoadFlags = mnLoadFlags;
    if( nGlyphFlags & GF_UNHINTED )
        nLoadFlags |= FT_LOAD_NO_HINTING;

    FT_Error rc = -1;
#if (FTVERSION <= 2008)
    // #88364# freetype<=2005 prefers autohinting to embedded bitmaps
    // => first we have to try without hinting
    if( (nLoadFlags & (FT_LOAD_NO_HINTING | FT_LOAD_NO_BITMAP)) == 0 )
    {
        rc = FT_Load_Glyph( maFaceFT, nGlyphIndex, nLoadFlags|FT_LOAD_NO_HINTING );
        if( (rc==FT_Err_Ok) && (maFaceFT->glyph->format!=ft_glyph_format_bitmap) )
            rc = -1; // mark as "loading embedded bitmap" was unsuccessful
        nLoadFlags |= FT_LOAD_NO_BITMAP;
    }

    if( rc != FT_Err_Ok )
#endif
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

    int nCharWidth = maFaceFT->glyph->metrics.horiAdvance;
    if( nGlyphFlags & GF_ROTMASK ) {  // for bVertical rotated glyphs
        const FT_Size_Metrics& rMetrics = maFaceFT->size->metrics;
#if (FTVERSION < 2000)
        nCharWidth = (rMetrics.height - rMetrics.descender) * mfStretch;
#else
        nCharWidth = (rMetrics.height + rMetrics.descender) * mfStretch;
#endif
    }
    rGD.SetCharWidth( (nCharWidth + 32) >> 6 );

    FT_Glyph pGlyphFT;
    rc = FT_Get_Glyph( maFaceFT->glyph, &pGlyphFT );

    int nAngle = ApplyGlyphTransform( nGlyphFlags, pGlyphFT );
    rGD.SetDelta( (pGlyphFT->advance.x + 0x8000) >> 16, -((pGlyphFT->advance.y + 0x8000) >> 16) );

    FT_BBox aBbox;
    FT_Glyph_Get_CBox( pGlyphFT, ft_glyph_bbox_pixels, &aBbox );
    if( aBbox.yMin > aBbox.yMax )   // circumvent freetype bug
    {
        int t=aBbox.yMin; aBbox.yMin=aBbox.yMax, aBbox.yMax=t;
    }
    rGD.SetOffset( aBbox.xMin, -aBbox.yMax );
    rGD.SetSize( Size( (aBbox.xMax-aBbox.xMin+1), (aBbox.yMax-aBbox.yMin) ) );

    FT_Done_Glyph( pGlyphFT );
}

// -----------------------------------------------------------------------

bool FreetypeServerFont::GetAntialiasAdvice( void ) const
{
    if( GetFontSelData().mbNonAntialiased || (nPrioAntiAlias<=0) )
        return false;
    bool bAdviseAA = true;
    // TODO: also use GASP info
    return bAdviseAA;
}

// -----------------------------------------------------------------------

bool FreetypeServerFont::GetGlyphBitmap1( int nGlyphIndex, RawBitmap& rRawBitmap ) const
{
    if( maSizeFT )
        pFTActivateSize( maSizeFT );

    int nGlyphFlags;
    SplitGlyphFlags( *this, nGlyphIndex, nGlyphFlags );

    FT_Int nLoadFlags = mnLoadFlags;

#if (FTVERSION >= 2002)
    // for 0/90/180/270 degree fonts enable autohinting even if not advisable
    // non-hinted and non-antialiased bitmaps just look too ugly
    if( (nCos==0 || nSin==0) && (nPrioAutoHint > 0) )
        nLoadFlags &= ~FT_LOAD_NO_HINTING;
#endif

    if( nPrioEmbedded <= nPrioAutoHint )
        nLoadFlags |= FT_LOAD_NO_BITMAP;

    FT_Error rc = -1;
#if (FTVERSION <= 2008)
    // #88364# freetype<=2005 prefers autohinting to embedded bitmaps
    // => first we have to try without hinting
    if( (nLoadFlags & (FT_LOAD_NO_HINTING | FT_LOAD_NO_BITMAP)) == 0 )
    {
        rc = FT_Load_Glyph( maFaceFT, nGlyphIndex, nLoadFlags|FT_LOAD_NO_HINTING );
        if( (rc==FT_Err_Ok) && (maFaceFT->glyph->format!=ft_glyph_format_bitmap) )
            rc = -1; // mark as "loading embedded bitmap" was unsuccessful
        nLoadFlags |= FT_LOAD_NO_BITMAP;
    }

    if( rc != FT_Err_Ok )
#endif
        rc = FT_Load_Glyph( maFaceFT, nGlyphIndex, nLoadFlags );
    if( rc != FT_Err_Ok )
        return false;

    FT_Glyph pGlyphFT;
    rc = FT_Get_Glyph( maFaceFT->glyph, &pGlyphFT );
    if( rc != FT_Err_Ok )
        return false;

    int nAngle = ApplyGlyphTransform( nGlyphFlags, pGlyphFT );

    if( pGlyphFT->format != ft_glyph_format_bitmap )
    {
        if( pGlyphFT->format == ft_glyph_format_outline )
            ((FT_OutlineGlyphRec*)pGlyphFT)->outline.flags |= ft_outline_high_precision;
#ifdef MACOSX
        FT_Render_Mode nRenderMode = (FT_Render_Mode)((nFTVERSION<2103) ? 1 : ft_render_mode_mono); // #i15743#
#else
        FT_Render_Mode nRenderMode = (FT_Render_Mode)((nFTVERSION<2103) ? 1 : FT_RENDER_MODE_MONO); // #i15743#
#endif
        rc = FT_Glyph_To_Bitmap( &pGlyphFT, nRenderMode, NULL, TRUE );
        if( rc != FT_Err_Ok )
            return false;
    }

    const FT_BitmapGlyph& rBmpGlyphFT = reinterpret_cast<const FT_BitmapGlyph&>(pGlyphFT);
    // NOTE: autohinting in FT<=2.0.2 miscalculates the offsets below by +-1
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

bool FreetypeServerFont::GetGlyphBitmap8( int nGlyphIndex, RawBitmap& rRawBitmap ) const
{
    if( maSizeFT )
        pFTActivateSize( maSizeFT );

    int nGlyphFlags;
    SplitGlyphFlags( *this, nGlyphIndex, nGlyphFlags );

    FT_Int nLoadFlags = mnLoadFlags;

#if (FTVERSION <= 2004) && !defined(TT_CONFIG_OPTION_BYTECODE_INTERPRETER)
    // autohinting in FT<=2.0.4 makes antialiased glyphs look worse
    nLoadFlags |= FT_LOAD_NO_HINTING;
#else
    if( (nGlyphFlags & GF_UNHINTED) || (nPrioAutoHint < nPrioAntiAlias) )
        nLoadFlags |= FT_LOAD_NO_HINTING;
#endif

    if( nPrioEmbedded <= nPrioAntiAlias )
        nLoadFlags |= FT_LOAD_NO_BITMAP;

    FT_Error rc = -1;
#if (FTVERSION <= 2008)
    // #88364# freetype<=2005 prefers autohinting to embedded bitmaps
    // => first we have to try without hinting
    if( (nLoadFlags & (FT_LOAD_NO_HINTING | FT_LOAD_NO_BITMAP)) == 0 )
    {
        rc = FT_Load_Glyph( maFaceFT, nGlyphIndex, nLoadFlags|FT_LOAD_NO_HINTING );
        if( (rc==FT_Err_Ok) && (maFaceFT->glyph->format!=ft_glyph_format_bitmap) )
            rc = -1; // mark as "loading embedded bitmap" was unsuccessful
        nLoadFlags |= FT_LOAD_NO_BITMAP;
    }

    if( rc != FT_Err_Ok )
#endif
        rc = FT_Load_Glyph( maFaceFT, nGlyphIndex, nLoadFlags );

    if( rc != FT_Err_Ok )
        return false;

    FT_Glyph pGlyphFT;
    rc = FT_Get_Glyph( maFaceFT->glyph, &pGlyphFT );
    if( rc != FT_Err_Ok )
        return false;

    int nAngle = ApplyGlyphTransform( nGlyphFlags, pGlyphFT );

    if( pGlyphFT->format == ft_glyph_format_outline )
        ((FT_OutlineGlyph)pGlyphFT)->outline.flags |= ft_outline_high_precision;

    bool bEmbedded = (pGlyphFT->format == ft_glyph_format_bitmap);
    if( !bEmbedded )
    {
        rc = FT_Glyph_To_Bitmap( &pGlyphFT, ft_render_mode_normal, NULL, TRUE );
        if( rc != FT_Err_Ok )
            return false;
    }

    const FT_BitmapGlyph& rBmpGlyphFT = reinterpret_cast<const FT_BitmapGlyph&>(pGlyphFT);
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

ULONG FreetypeServerFont::GetFontCodeRanges( sal_uInt32* pCodes ) const
{
    int nRangeCount = 0;

#if 0 && (FTVERSION >= 2102)
    // TODO: enable new version when it is fast enough for big fonts
    // TODO: implement Get_Next_Missing_Char() and use this
    FT_UInt nGlyphIndex = 0;
    sal_uInt32 aChar = FT_Get_First_Char( maFaceFT, &nGlyphIndex );
    if( nGlyphIndex )
        nRangeCount = 1;
    if( pCodes )
        *(pCodes++) = aChar;            // start of first range
    for(;;)
    {
        sal_uInt32 bChar = FT_Get_Next_Char( maFaceFT, aChar, &nGlyphIndex );
        if( !nGlyphIndex )
            break;
        if( bChar != aChar+1 )
        {
            ++nRangeCount;
            if( pCodes )
            {
                *(pCodes++) = aChar+1;  // end of old range
                *(pCodes++) = bChar;    // start of new range
            }
        }
        aChar = bChar;
    }
    if( pCodes && nRangeCount )
        *(pCodes++) = aChar+1;          // end of last range
#else
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
#endif

    return nRangeCount;
}
// -----------------------------------------------------------------------
// kerning stuff
// -----------------------------------------------------------------------

int FreetypeServerFont::GetGlyphKernValue( int nGlyphLeft, int nGlyphRight ) const
{
    if( maSizeFT )
        pFTActivateSize( maSizeFT );

    FT_Vector aKernVal;
    FT_Error rcFT = FT_Get_Kerning( maFaceFT, nGlyphLeft, nGlyphRight,
                ft_kerning_default, &aKernVal );
    int nResult = (rcFT == FT_Err_Ok) ? (aKernVal.x + 32) >> 6 : 0;
    return nResult;
}

// -----------------------------------------------------------------------

ULONG FreetypeServerFont::GetKernPairs( ImplKernPairData** ppKernPairs ) const
{
    if( maSizeFT )
        pFTActivateSize( maSizeFT );

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
    USHORT nVersion = GetUShort( pBuffer+0 );
    USHORT nTableCnt = GetUShort( pBuffer+2 );
    pBuffer += 4;
    if( nVersion != 0 )     // ignore Apple's versions for now
        nTableCnt = 0;
    for( USHORT nTableIdx = 0; nTableIdx < nTableCnt; ++nTableIdx )
    {
        USHORT nSubVersion  = GetUShort( pBuffer+0 );
        USHORT nSubLength   = GetUShort( pBuffer+2 );
        USHORT nSubCoverage = GetUShort( pBuffer+4 );
        pBuffer += 6;
        if( (nSubCoverage&0x03) != 0x01 )   // no interest in minimum info here
            continue;
        switch( nSubCoverage >> 8 )
        {
            case 0: // version 0, kerning format 0
            {
                USHORT nPairs = GetUShort( pBuffer );
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
                //USHORT nRowWidth  = GetUShort( pBuffer+0 );
                USHORT nOfsLeft     = GetUShort( pBuffer+2 );
                USHORT nOfsRight    = GetUShort( pBuffer+4 );
                USHORT nOfsArray    = GetUShort( pBuffer+6 );
                pBuffer += 8;

                const FT_Byte* pTmp = pSubTable + nOfsLeft;
                USHORT nFirstLeft   = GetUShort( pTmp+0 );
                USHORT nLastLeft    = GetUShort( pTmp+2 ) + nFirstLeft - 1;

                pTmp = pSubTable + nOfsRight;
                USHORT nFirstRight  = GetUShort( pTmp+0 );
                USHORT nLastRight   = GetUShort( pTmp+2 ) + nFirstRight - 1;

                ULONG nPairs = (ULONG)(nLastLeft - nFirstLeft + 1) * (nLastRight - nFirstRight + 1);
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
                PolyArgs( PolyPolygon& rPolyPoly, USHORT nMaxPoints );
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

PolyArgs::PolyArgs( PolyPolygon& rPolyPoly, USHORT nMaxPoints )
:   mrPolyPoly(rPolyPoly),
    mnMaxPoints(nMaxPoints),
    mnPoints(0),
    mnPoly(0),
    bHasOffline(false)
{
    mpPointAry  = new Point[ mnMaxPoints ];
    mpFlagAry   = new BYTE [ mnMaxPoints ];
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
    mrPolyPoly.Insert( aPoly );
    mnPoints = 0;
    bHasOffline = false;
}

// -----------------------------------------------------------------------

extern "C" {

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

}; // extern "C"

// -----------------------------------------------------------------------

bool FreetypeServerFont::GetGlyphOutline( int nGlyphIndex, PolyPolygon& rPolyPoly ) const
{
    rPolyPoly.Clear();

    int nGlyphFlags;
    SplitGlyphFlags( *this, nGlyphIndex, nGlyphFlags );

    FT_Int nLoadFlags = FT_LOAD_NO_HINTING | FT_LOAD_NO_BITMAP;
    FT_Error rc = FT_Load_Glyph( maFaceFT, nGlyphIndex, nLoadFlags );
    if( rc != FT_Err_Ok )
        return false;

    FT_Glyph pGlyphFT;
    rc = FT_Get_Glyph( maFaceFT->glyph, &pGlyphFT );
    if( rc != FT_Err_Ok )
        return false;

    if( pGlyphFT->format != ft_glyph_format_outline )
        return false;

    FT_Outline& rOutline = reinterpret_cast<FT_OutlineGlyphRec*>(pGlyphFT)->outline;
    if( !rOutline.n_points )    // blank glyphs are ok
        return true;

    long nMaxPoints = 1 + rOutline.n_points * 3;
    PolyArgs aPolyArg( rPolyPoly, nMaxPoints );

    int nAngle = ApplyGlyphTransform( nGlyphFlags, pGlyphFT );

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

    rPolyPoly.Scale( +1.0/(1<<6), -1.0/(1<<6) );

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
    const ULONG nVersion            = GetUInt( pGsubHeader+0 );
    const USHORT nOfsScriptList     = GetUShort( pGsubHeader+4 );
    const USHORT nOfsFeatureTable   = GetUShort( pGsubHeader+6 );
    const USHORT nOfsLookupList     = GetUShort( pGsubHeader+8 );
    pGsubHeader += 10;

    typedef std::vector<USHORT> UshortList;
    UshortList aFeatureIndexList;
    UshortList aFeatureOffsetList;

    // parse Script Table
    const FT_Byte* pScriptHeader = pGsubBase + nOfsScriptList;
    const USHORT nCntScript = GetUShort( pScriptHeader+0 );
    pScriptHeader += 2;
    for( USHORT nScriptIndex = 0; nScriptIndex < nCntScript; ++nScriptIndex )
    {
        const ULONG nTag            = GetUInt( pScriptHeader+0 ); // e.g. hani/arab/kana/hang
        const USHORT nOfsScriptTable= GetUShort( pScriptHeader+4 );
        pScriptHeader += 6; //###
        if( (nTag != nRequestedScript) && (nRequestedScript != 0) )
            continue;

        const FT_Byte* pScriptTable     = pGsubBase + nOfsScriptList + nOfsScriptTable;
        const USHORT nDefaultLangsysOfs = GetUShort( pScriptTable+0 );
        const USHORT nCntLangSystem     = GetUShort( pScriptTable+2 );
        pScriptTable += 4;
        USHORT nLangsysOffset = 0;

        for( USHORT nLangsysIndex = 0; nLangsysIndex < nCntLangSystem; ++nLangsysIndex )
        {
            const ULONG nTag    = GetUInt( pScriptTable+0 );    // e.g. KOR/ZHS/ZHT/JAN
            const USHORT nOffset= GetUShort( pScriptTable+4 );
            pScriptTable += 6;
            if( (nTag != nRequestedLangsys) && (nRequestedLangsys != 0) )
                continue;
            nLangsysOffset = nOffset;
            break;
        }

        if( (nDefaultLangsysOfs != 0) && (nDefaultLangsysOfs != nLangsysOffset) )
        {
            const FT_Byte* pLangSys = pGsubBase + nOfsScriptList + nOfsScriptTable + nDefaultLangsysOfs;
            const USHORT nLookupOrder   = GetUShort( pLangSys+0 );
            const USHORT nReqFeatureIdx = GetUShort( pLangSys+2 );
            const USHORT nCntFeature    = GetUShort( pLangSys+4 );
            pLangSys += 6;
            aFeatureIndexList.push_back( nReqFeatureIdx );
            for( USHORT i = 0; i < nCntFeature; ++i )
            {
                const USHORT nFeatureIndex = GetUShort( pLangSys );
                pLangSys += 2;
                aFeatureIndexList.push_back( nFeatureIndex );
            }
        }

        if( nLangsysOffset != 0 )
        {
            const FT_Byte* pLangSys = pGsubBase + nOfsScriptList + nOfsScriptTable + nLangsysOffset;
            const USHORT nLookupOrder   = GetUShort( pLangSys+0 );
            const USHORT nReqFeatureIdx = GetUShort( pLangSys+2 );
            const USHORT nCntFeature    = GetUShort( pLangSys+4 );
            pLangSys += 6;
            aFeatureIndexList.push_back( nReqFeatureIdx );
            for( USHORT i = 0; i < nCntFeature; ++i )
            {
                const USHORT nFeatureIndex = GetUShort( pLangSys );
                pLangSys += 2;
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
    const USHORT nCntFeature = GetUShort( pFeatureHeader );
    pFeatureHeader += 2;
    for( USHORT nFeatureIndex = 0; nFeatureIndex < nCntFeature; ++nFeatureIndex )
    {
        const ULONG nTag    = GetUInt( pFeatureHeader+0 ); // e.g. locl/vert/trad/smpl/liga/fina/...
        const USHORT nOffset= GetUShort( pFeatureHeader+4 );
        pFeatureHeader += 6;

        // feature (required && (requested || available))?
        if( (aFeatureIndexList[0] != nFeatureIndex)
        &&  (!std::count( aReqFeatureTagList.begin(), aReqFeatureTagList.end(), nTag))
        ||  (!std::count( aFeatureIndexList.begin(), aFeatureIndexList.end(), nFeatureIndex) ) )
            continue;

        const FT_Byte* pFeatureTable = pGsubBase + nOfsFeatureTable + nOffset;
        const USHORT nCntLookups = GetUShort( pFeatureTable+0 );
        pFeatureTable += 2;
        for( USHORT i = 0; i < nCntLookups; ++i )
        {
            const USHORT nLookupIndex = GetUShort( pFeatureTable );
            pFeatureTable += 2;
            aLookupIndexList.push_back( nLookupIndex );
        }
        if( nCntLookups == 0 ) //### hack needed by Mincho/Gothic/Mingliu/Simsun/...
            aLookupIndexList.push_back( 0 );
    }

    // parse Lookup List
    const FT_Byte* pLookupHeader = pGsubBase + nOfsLookupList;
    const USHORT nCntLookupTable = GetUShort( pLookupHeader );
    pLookupHeader += 2;
    for( USHORT nLookupIdx = 0; nLookupIdx < nCntLookupTable; ++nLookupIdx )
    {
        const USHORT nOffset = GetUShort( pLookupHeader );
        pLookupHeader += 2;
        if( std::count( aLookupIndexList.begin(), aLookupIndexList.end(), nLookupIdx ) )
            aLookupOffsetList.push_back( nOffset );
    }

    UshortList::const_iterator it = aLookupOffsetList.begin();
    for(; it != aLookupOffsetList.end(); ++it )
    {
        const USHORT nOfsLookupTable = *it;
        const FT_Byte* pLookupTable = pGsubBase + nOfsLookupList + nOfsLookupTable;
        const USHORT eLookupType        = GetUShort( pLookupTable+0 );
        const USHORT eLookupFlag        = GetUShort( pLookupTable+2 );
        const USHORT nCntLookupSubtable = GetUShort( pLookupTable+4 );
        pLookupTable += 6;

        // TODO: switch( eLookupType )
        if( eLookupType != 1 )  // TODO: once we go beyond SingleSubst
            continue;

        for( USHORT nSubTableIdx = 0; nSubTableIdx < nCntLookupSubtable; ++nSubTableIdx )
        {
            const USHORT nOfsSubLookupTable = GetUShort( pLookupTable );
            pLookupTable += 2;
            const FT_Byte* pSubLookup = pGsubBase + nOfsLookupList + nOfsLookupTable + nOfsSubLookupTable;

            const USHORT nFmtSubstitution   = GetUShort( pSubLookup+0 );
            const USHORT nOfsCoverage       = GetUShort( pSubLookup+2 );
            pSubLookup += 4;

            typedef std::pair<USHORT,USHORT> GlyphSubst;
            typedef std::vector<GlyphSubst> SubstVector;
            SubstVector aSubstVector;

            const FT_Byte* pCoverage    = pGsubBase + nOfsLookupList + nOfsLookupTable + nOfsSubLookupTable + nOfsCoverage;
            const USHORT nFmtCoverage   = GetUShort( pCoverage+0 );
            pCoverage += 2;
            switch( nFmtCoverage )
            {
                case 1:         // Coverage Format 1
                    {
                        const USHORT nCntGlyph = GetUShort( pCoverage );
                        pCoverage += 2;
                        aSubstVector.reserve( nCntGlyph );
                        for( USHORT i = 0; i < nCntGlyph; ++i )
                        {
                            const USHORT nGlyphId = GetUShort( pCoverage );
                            pCoverage += 2;
                            aSubstVector.push_back( GlyphSubst( nGlyphId, 0 ) );
                        }
                    }
                    break;

                case 2:         // Coverage Format 2
                    {
                        const USHORT nCntRange = GetUShort( pCoverage );
                        pCoverage += 2;
                        for( int i = nCntRange; --i >= 0; )
                        {
                            const USHORT nGlyph0 = GetUShort( pCoverage+0 );
                            const USHORT nGlyph1 = GetUShort( pCoverage+2 );
                            const USHORT nCovIdx = GetUShort( pCoverage+4 );
                            pCoverage += 6;
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
                        const USHORT nDeltaGlyphId = GetUShort( pSubLookup );
                        pSubLookup += 2;
                        for(; it != aSubstVector.end(); ++it )
                            (*it).second = (*it).first + nDeltaGlyphId;
                    }
                    break;

                case 2:     // Single Substitution Format 2
                    {
                        const USHORT nCntGlyph = GetUShort( pSubLookup );
                        pSubLookup += 2;
                        for( int i = nCntGlyph; (it != aSubstVector.end()) && (--i>=0); ++it )
                        {
                            const USHORT nGlyphId = GetUShort( pSubLookup );
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

// =======================================================================
