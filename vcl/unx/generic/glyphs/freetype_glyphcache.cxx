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

#include <sal/config.h>

#include <o3tl/safeint.hxx>
#include <vcl/fontcharmap.hxx>

#include <unx/freetype_glyphcache.hxx>

#include <fontinstance.hxx>
#include <fontattributes.hxx>

#include <unotools/fontdefs.hxx>

#include <tools/poly.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>

#include <sal/log.hxx>

#include <langboost.hxx>
#include <PhysicalFontCollection.hxx>
#include <sft.hxx>

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H
#include FT_MULTIPLE_MASTERS_H
#include FT_OUTLINE_H
#include FT_SIZES_H
#include FT_SYNTHESIS_H
#include FT_TRUETYPE_TABLES_H

#include <rtl/instance.hxx>

#include <vector>

// TODO: move file mapping stuff to OSL
#include <unistd.h>
#include <dlfcn.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <unx/fontmanager.hxx>
#include <impfontcharmap.hxx>

static FT_Library aLibFT = nullptr;

// TODO: remove when the priorities are selected by UI
// if (AH==0) => disable autohinting
// if (AA==0) => disable antialiasing
// if (EB==0) => disable embedded bitmaps
// if (AA prio <= AH prio) => antialias + autohint
// if (AH<AA) => do not autohint when antialiasing
// if (EB<AH) => do not autohint for monochrome
static int nDefaultPrioEmbedded    = 2;
static int nDefaultPrioAntiAlias   = 1;

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

bool FreetypeFontFile::Map()
{
    if (mnRefCount++ == 0)
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
    if (--mnRefCount != 0)
        return;
    assert(mnRefCount >= 0 && "how did this go negative\n");
    if (mpFileMap)
    {
        munmap(mpFileMap, mnFileSize);
        mpFileMap = nullptr;
    }
}

FreetypeFontInfo::FreetypeFontInfo( const FontAttributes& rDevFontAttributes,
    FreetypeFontFile* const pFontFile, int nFaceNum, int nFaceVariation, sal_IntPtr nFontId)
:
    maFaceFT( nullptr ),
    mpFontFile(pFontFile),
    mnFaceNum( nFaceNum ),
    mnFaceVariation( nFaceVariation ),
    mnRefCount( 0 ),
    mnFontId( nFontId ),
    maDevFontAttributes( rDevFontAttributes )
{
    // prefer font with low ID
    maDevFontAttributes.IncreaseQualityBy( 10000 - nFontId );
    // prefer font with matching file names
    maDevFontAttributes.IncreaseQualityBy( mpFontFile->GetLangBoost() );
}

FreetypeFontInfo::~FreetypeFontInfo()
{
}

namespace
{
    void dlFT_Done_MM_Var(FT_Library library, FT_MM_Var *amaster)
    {
        static auto func = reinterpret_cast<void(*)(FT_Library, FT_MM_Var*)>(dlsym(nullptr, "FT_Done_MM_Var"));
        if (func)
            func(library, amaster);
        else
            free(amaster);
    }
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

        if (maFaceFT && mnFaceVariation)
        {
            FT_MM_Var *pFtMMVar;
            if (FT_Get_MM_Var(maFaceFT, &pFtMMVar) == 0)
            {
                if (o3tl::make_unsigned(mnFaceVariation) <= pFtMMVar->num_namedstyles)
                {
                    FT_Var_Named_Style *instance = &pFtMMVar->namedstyle[mnFaceVariation - 1];
                    FT_Set_Var_Design_Coordinates(maFaceFT, pFtMMVar->num_axis, instance->coords);
                }
                dlFT_Done_MM_Var(aLibFT, pFtMMVar);
            }
        }
    }

    ++mnRefCount;
    return maFaceFT;
}

void FreetypeFont::SetFontVariationsOnHBFont(hb_font_t* pHbFace) const
{
    sal_uInt32 nFaceVariation = mxFontInfo->GetFontFaceVariation();
    if (!(maFaceFT && nFaceVariation))
        return;

    FT_MM_Var *pFtMMVar;
    if (FT_Get_MM_Var(maFaceFT, &pFtMMVar) != 0)
        return;

    if (nFaceVariation <= pFtMMVar->num_namedstyles)
    {
        FT_Var_Named_Style *instance = &pFtMMVar->namedstyle[nFaceVariation - 1];
        std::vector<hb_variation_t> aVariations(pFtMMVar->num_axis);
        for (FT_UInt i = 0; i < pFtMMVar->num_axis; ++i)
        {
            aVariations[i].tag = pFtMMVar->axis[i].tag;
            aVariations[i].value = instance->coords[i] / 65536.0;
        }
        hb_font_set_variations(pHbFace, aVariations.data(), aVariations.size());
    }
    dlFT_Done_MM_Var(aLibFT, pFtMMVar);
}

void FreetypeFontInfo::ReleaseFaceFT()
{
    if (--mnRefCount == 0)
    {
        if (maFaceFT)
        {
            FT_Done_Face(maFaceFT);
            maFaceFT = nullptr;
        }
        mpFontFile->Unmap();
    }
    assert(mnRefCount >= 0 && "how did this go negative\n");
}

static unsigned GetUInt( const unsigned char* p ) { return((p[0]<<24)+(p[1]<<16)+(p[2]<<8)+p[3]);}
static unsigned GetUShort( const unsigned char* p ){ return((p[0]<<8)+p[1]);}

const sal_uInt32 T_true = 0x74727565;        /* 'true' */
const sal_uInt32 T_ttcf = 0x74746366;        /* 'ttcf' */
const sal_uInt32 T_otto = 0x4f54544f;        /* 'OTTO' */

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
    rtl::Reference<FreetypeFontFace> pFD(new FreetypeFontFace( this, maDevFontAttributes ));
    pFontCollection->Add( pFD.get() );
}

void FreetypeManager::InitFreetype()
{
    /*FT_Error rcFT =*/ FT_Init_FreeType( &aLibFT );

    // TODO: remove when the priorities are selected by UI
    char* pEnv;
    pEnv = ::getenv( "SAL_EMBEDDED_BITMAP_PRIORITY" );
    if( pEnv )
        nDefaultPrioEmbedded  = pEnv[0] - '0';
    pEnv = ::getenv( "SAL_ANTIALIASED_TEXT_PRIORITY" );
    if( pEnv )
        nDefaultPrioAntiAlias = pEnv[0] - '0';
}

namespace
{
    bool DoesAlmostHorizontalDrainRenderingPool()
    {
        FT_Int nMajor, nMinor, nPatch;
        FT_Library_Version(aLibFT, &nMajor, &nMinor, &nPatch);
        if (nMajor > 2)
            return false;
        if (nMajor == 2 && nMinor <= 8)
            return true;
        return false;
    }
}

bool FreetypeFont::AlmostHorizontalDrainsRenderingPool(int nRatio, const FontSelectPattern& rFSD)
{
    static bool bAlmostHorizontalDrainsRenderingPool = DoesAlmostHorizontalDrainRenderingPool();
    if (nRatio > 100 && rFSD.maTargetName == "OpenSymbol" && bAlmostHorizontalDrainsRenderingPool)
    {
        // tdf#127189 FreeType <= 2.8 will fail to render stretched horizontal
        // brace glyphs in starmath at a fairly low stretch ratio. The failure
        // will set CAIRO_STATUS_FREETYPE_ERROR on the surface which cannot be
        // cleared, so all further painting to the surface fails.

        // This appears fixed in >= freetype 2.9

        // Restrict this bodge to a stretch ratio > ~10 of the OpenSymbol font
        // where it has been seen in practice.
        SAL_WARN("vcl", "rendering text would fail with stretch ratio of: " << nRatio << ", with FreeType <= 2.8");
        return true;
    }
    return false;
}

FT_Face FreetypeFont::GetFtFace() const
{
    FT_Activate_Size( maSizeFT );

    return maFaceFT;
}

void FreetypeManager::AddFontFile(const OString& rNormalizedName,
    int nFaceNum, int nVariantNum, sal_IntPtr nFontId, const FontAttributes& rDevFontAttr)
{
    if( rNormalizedName.isEmpty() )
        return;

    if( m_aFontInfoList.find( nFontId ) != m_aFontInfoList.end() )
        return;

    FreetypeFontInfo* pFontInfo = new FreetypeFontInfo( rDevFontAttr,
        FindFontFile(rNormalizedName), nFaceNum, nVariantNum, nFontId);
    m_aFontInfoList[ nFontId ].reset(pFontInfo);
    if( m_nMaxFontId < nFontId )
        m_nMaxFontId = nFontId;
}

void FreetypeManager::AnnounceFonts( PhysicalFontCollection* pToAdd ) const
{
    for (auto const& font : m_aFontInfoList)
    {
        FreetypeFontInfo* pFreetypeFontInfo = font.second.get();
        pFreetypeFontInfo->AnnounceFont( pToAdd );
    }
}

FreetypeFont* FreetypeManager::CreateFont(FreetypeFontInstance* pFontInstance)
{
    // find a FontInfo matching to the font id
    if (!pFontInstance)
        return nullptr;

    const PhysicalFontFace* pFontFace = pFontInstance->GetFontFace();
    if (!pFontFace)
        return nullptr;

    sal_IntPtr nFontId = pFontFace->GetFontId();
    FontInfoList::iterator it = m_aFontInfoList.find(nFontId);

    if (it == m_aFontInfoList.end())
        return nullptr;

    return new FreetypeFont(*pFontInstance, it->second);
}

FreetypeFontFace::FreetypeFontFace( FreetypeFontInfo* pFI, const FontAttributes& rDFA )
:   PhysicalFontFace( rDFA ),
    mpFreetypeFontInfo( pFI )
{
}

rtl::Reference<LogicalFontInstance> FreetypeFontFace::CreateFontInstance(const FontSelectPattern& rFSD) const
{
    return new FreetypeFontInstance(*this, rFSD);
}

// FreetypeFont

FreetypeFont::FreetypeFont(FreetypeFontInstance& rFontInstance, std::shared_ptr<FreetypeFontInfo>& rFI)
:   mrFontInstance(rFontInstance),
    mnCos( 0x10000),
    mnSin( 0 ),
    mnPrioAntiAlias(nDefaultPrioAntiAlias),
    mxFontInfo(rFI),
    mnLoadFlags( 0 ),
    maFaceFT( nullptr ),
    maSizeFT( nullptr ),
    mbFaceOk( false ),
    mbArtItalic( false ),
    mbArtBold(false)
{
    int nPrioEmbedded = nDefaultPrioEmbedded;

    maFaceFT = mxFontInfo->GetFaceFT();

    const FontSelectPattern& rFSD = rFontInstance.GetFontSelectPattern();

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
    mfStretch = static_cast<double>(mnWidth) / rFSD.mnHeight;
    // sanity check (e.g. #i66394#, #i66244#, #i66537#)
    if( (mnWidth < 0) || (mfStretch > +64.0) || (mfStretch < -64.0) )
        return;

    if( !maFaceFT )
        return;

    FT_New_Size( maFaceFT, &maSizeFT );
    FT_Activate_Size( maSizeFT );
    /* This might fail for color bitmap fonts, but that is fine since we will
     * not need any glyph data from FreeType in this case */
    /*FT_Error rc = */ FT_Set_Pixel_Sizes( maFaceFT, mnWidth, rFSD.mnHeight );

    FT_Select_Charmap(maFaceFT, FT_ENCODING_UNICODE);

    if( mxFontInfo->IsSymbolFont() )
    {
        FT_Encoding eEncoding = FT_ENCODING_MS_SYMBOL;
        FT_Select_Charmap(maFaceFT, eEncoding);
    }

    mbFaceOk = true;

    // TODO: query GASP table for load flags
    mnLoadFlags = FT_LOAD_DEFAULT | FT_LOAD_IGNORE_TRANSFORM;

    mbArtItalic = (rFSD.GetItalic() != ITALIC_NONE && mxFontInfo->GetFontAttributes().GetItalic() == ITALIC_NONE);
    mbArtBold = (rFSD.GetWeight() > WEIGHT_MEDIUM && mxFontInfo->GetFontAttributes().GetWeight() <= WEIGHT_MEDIUM);

    if( ((mnCos != 0) && (mnSin != 0)) || (nPrioEmbedded <= 0) )
        mnLoadFlags |= FT_LOAD_NO_BITMAP;
}

namespace
{
    std::unique_ptr<FontConfigFontOptions> GetFCFontOptions( const FontAttributes& rFontAttributes, int nSize)
    {
        psp::FastPrintFontInfo aInfo;

        aInfo.m_aFamilyName = rFontAttributes.GetFamilyName();
        aInfo.m_eItalic = rFontAttributes.GetItalic();
        aInfo.m_eWeight = rFontAttributes.GetWeight();
        aInfo.m_eWidth = rFontAttributes.GetWidthType();

        return psp::PrintFontManager::getFontOptions(aInfo, nSize);
    }
}

const FontConfigFontOptions* FreetypeFont::GetFontOptions() const
{
    if (!mxFontOptions)
    {
        mxFontOptions = GetFCFontOptions(mxFontInfo->GetFontAttributes(), mrFontInstance.GetFontSelectPattern().mnHeight);
        mxFontOptions->SyncPattern(GetFontFileName(), GetFontFaceIndex(), GetFontFaceVariation(), NeedsArtificialBold());
    }
    return mxFontOptions.get();
}

const OString& FreetypeFont::GetFontFileName() const
{
    return mxFontInfo->GetFontFileName();
}

int FreetypeFont::GetFontFaceIndex() const
{
    return mxFontInfo->GetFontFaceIndex();
}

int FreetypeFont::GetFontFaceVariation() const
{
    return mxFontInfo->GetFontFaceVariation();
}

FreetypeFont::~FreetypeFont()
{
    if( maSizeFT )
        FT_Done_Size( maSizeFT );

    mxFontInfo->ReleaseFaceFT();
}

void FreetypeFont::GetFontMetric(ImplFontMetricDataRef const & rxTo) const
{
    rxTo->FontAttributes::operator =(mxFontInfo->GetFontAttributes());

    rxTo->SetOrientation(mrFontInstance.GetFontSelectPattern().mnOrientation);

    //Always consider [star]symbol as symbol fonts
    if ( IsStarSymbol( rxTo->GetFamilyName() ) )
        rxTo->SetSymbolFlag( true );

    FT_Activate_Size( maSizeFT );

    rxTo->ImplCalcLineSpacing(&mrFontInstance);

    rxTo->SetSlant( 0 );
    rxTo->SetWidth( mnWidth );

    const TT_OS2* pOS2 = static_cast<const TT_OS2*>(FT_Get_Sfnt_Table( maFaceFT, ft_sfnt_os2 ));
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
    rxTo->SetMinKashida(mrFontInstance.GetKashidaWidth());
}

void FreetypeFont::ApplyGlyphTransform(bool bVertical, FT_Glyph pGlyphFT ) const
{
    // shortcut most common case
    if (!mrFontInstance.GetFontSelectPattern().mnOrientation && !bVertical)
        return;

    const FT_Size_Metrics& rMetrics = maFaceFT->size->metrics;
    FT_Vector aVector;
    FT_Matrix aMatrix;

    bool bStretched = false;

    if (!bVertical)
    {
        // straight
        aVector.x = 0;
        aVector.y = 0;
        aMatrix.xx = +mnCos;
        aMatrix.yy = +mnCos;
        aMatrix.xy = -mnSin;
        aMatrix.yx = +mnSin;
    }
    else
    {
        // left
        bStretched = (mfStretch != 1.0);
        aVector.x  = static_cast<FT_Pos>(+rMetrics.descender * mfStretch);
        aVector.y  = -rMetrics.ascender;
        aMatrix.xx = static_cast<FT_Pos>(-mnSin / mfStretch);
        aMatrix.yy = static_cast<FT_Pos>(-mnSin * mfStretch);
        aMatrix.xy = static_cast<FT_Pos>(-mnCos * mfStretch);
        aMatrix.yx = static_cast<FT_Pos>(+mnCos / mfStretch);
    }

    if( pGlyphFT->format != FT_GLYPH_FORMAT_BITMAP )
    {
        FT_Glyph_Transform( pGlyphFT, nullptr, &aVector );

        // orthogonal transforms are better handled by bitmap operations
        if( bStretched )
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

bool FreetypeFont::GetGlyphBoundRect(sal_GlyphId nID, tools::Rectangle& rRect, bool bVertical) const
{
    FT_Activate_Size( maSizeFT );

    FT_Error rc = FT_Load_Glyph(maFaceFT, nID, mnLoadFlags);

    if (rc != FT_Err_Ok)
        return false;

    if (mbArtBold)
        FT_GlyphSlot_Embolden(maFaceFT->glyph);

    FT_Glyph pGlyphFT;
    rc = FT_Get_Glyph(maFaceFT->glyph, &pGlyphFT);
    if (rc != FT_Err_Ok)
        return false;

    ApplyGlyphTransform(bVertical, pGlyphFT);

    FT_BBox aBbox;
    FT_Glyph_Get_CBox( pGlyphFT, FT_GLYPH_BBOX_PIXELS, &aBbox );
    FT_Done_Glyph( pGlyphFT );

    tools::Rectangle aRect(aBbox.xMin, -aBbox.yMax, aBbox.xMax, -aBbox.yMin);
    if (mnCos != 0x10000 && mnSin != 0)
    {
        const double nCos = mnCos / 65536.0;
        const double nSin = mnSin / 65536.0;
        rRect.SetLeft(  nCos*aRect.Left() + nSin*aRect.Top() );
        rRect.SetTop( -nSin*aRect.Left() - nCos*aRect.Top() );
        rRect.SetRight(  nCos*aRect.Right() + nSin*aRect.Bottom() );
        rRect.SetBottom( -nSin*aRect.Right() - nCos*aRect.Bottom() );
    }
    else
        rRect = aRect;
    return true;
}

bool FreetypeFont::GetAntialiasAdvice() const
{
    // TODO: also use GASP info
    return !mrFontInstance.GetFontSelectPattern().mbNonAntialiased && (mnPrioAntiAlias > 0);
}

// determine unicode ranges in font

FontCharMapRef FreetypeFont::GetFontCharMap() const
{
    return mxFontInfo->GetFontCharMap();
}

bool FreetypeFont::GetFontCapabilities(vcl::FontCapabilities &rFontCapabilities) const
{
    return mxFontInfo->GetFontCapabilities(rFontCapabilities);
}

FontCharMapRef FreetypeFontInfo::GetFontCharMap() const
{
    // check if the charmap is already cached
    if( mxFontCharMap.is() )
        return mxFontCharMap;

    // get the charmap and cache it
    CmapResult aCmapResult;
    aCmapResult.mbSymbolic = IsSymbolFont();

    sal_uLong nLength = 0;
    const unsigned char* pCmap = GetTable("cmap", &nLength);
    if (pCmap && (nLength > 0) && ParseCMAP(pCmap, nLength, aCmapResult))
    {
        FontCharMapRef xFontCharMap( new FontCharMap ( aCmapResult ) );
        mxFontCharMap = xFontCharMap;
    }
    else
    {
        FontCharMapRef xFontCharMap( new FontCharMap() );
        mxFontCharMap = xFontCharMap;
    }
    // mxFontCharMap on either branch now has a refcount of 1
    return mxFontCharMap;
}

bool FreetypeFontInfo::GetFontCapabilities(vcl::FontCapabilities &rFontCapabilities) const
{
    bool bRet = false;

    sal_uLong nLength = 0;

    // load OS/2 table
    const FT_Byte* pOS2 = GetTable("OS/2", &nLength);
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

namespace {

class PolyArgs
{
public:
                PolyArgs( tools::PolyPolygon& rPolyPoly, sal_uInt16 nMaxPoints );

    void        AddPoint( long nX, long nY, PolyFlags);
    void        ClosePolygon();

    long        GetPosX() const { return maPosition.x;}
    long        GetPosY() const { return maPosition.y;}

private:
    tools::PolyPolygon& mrPolyPoly;

    std::unique_ptr<Point[]>
                    mpPointAry;
    std::unique_ptr<PolyFlags[]>
                    mpFlagAry;

    FT_Vector       maPosition;
    sal_uInt16      mnMaxPoints;
    sal_uInt16      mnPoints;
    sal_uInt16      mnPoly;
    bool            bHasOffline;

    PolyArgs(const PolyArgs&) = delete;
    PolyArgs& operator=(const PolyArgs&) = delete;
};

}

PolyArgs::PolyArgs( tools::PolyPolygon& rPolyPoly, sal_uInt16 nMaxPoints )
:   mrPolyPoly(rPolyPoly),
    mnMaxPoints(nMaxPoints),
    mnPoints(0),
    mnPoly(0),
    bHasOffline(false)
{
    mpPointAry.reset( new Point[ mnMaxPoints ] );
    mpFlagAry.reset( new PolyFlags [ mnMaxPoints ] );
    maPosition.x = maPosition.y = 0;
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
    bHasOffline |= (aFlag != PolyFlags::Normal);
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
    SAL_WARN_IF( (mpFlagAry[0]!=PolyFlags::Normal), "vcl", "FTGlyphOutline: PolyFinishFE failed!" );
    SAL_WARN_IF( (mpFlagAry[mnPoints]!=PolyFlags::Normal), "vcl", "FTGlyphOutline: PolyFinishFS failed!" );

    tools::Polygon aPoly( mnPoints, mpPointAry.get(), (bHasOffline ? mpFlagAry.get() : nullptr) );

    // #i35928#
    // This may be an invalid polygon, e.g. the last point is a control point.
    // So close the polygon (and add the first point again) if the last point
    // is a control point or different from first.
    // #i48298#
    // Now really duplicating the first point, to close or correct the
    // polygon. Also no longer duplicating the flags, but enforcing
    // PolyFlags::Normal for the newly added last point.
    const sal_uInt16 nPolySize(aPoly.GetSize());
    if(nPolySize)
    {
        if((aPoly.HasFlags() && PolyFlags::Control == aPoly.GetFlags(nPolySize - 1))
            || (aPoly.GetPoint(nPolySize - 1) != aPoly.GetPoint(0)))
        {
            aPoly.SetSize(nPolySize + 1);
            aPoly.SetPoint(aPoly.GetPoint(0), nPolySize);

            if(aPoly.HasFlags())
            {
                aPoly.SetFlags(nPolySize, PolyFlags::Normal);
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

    rA.AddPoint( p0->x, p0->y, PolyFlags::Normal );
    return 0;
}

static int FT_line_to( const FT_Vector* p1, void* vpPolyArgs )
{
    PolyArgs& rA = *static_cast<PolyArgs*>(vpPolyArgs);
    rA.AddPoint( p1->x, p1->y, PolyFlags::Normal );
    return 0;
}

static int FT_conic_to( const FT_Vector* p1, const FT_Vector* p2, void* vpPolyArgs )
{
    PolyArgs& rA = *static_cast<PolyArgs*>(vpPolyArgs);

    // VCL's Polygon only knows cubic beziers
    const long nX1 = (2 * rA.GetPosX() + 4 * p1->x + 3) / 6;
    const long nY1 = (2 * rA.GetPosY() + 4 * p1->y + 3) / 6;
    rA.AddPoint( nX1, nY1, PolyFlags::Control );

    const long nX2 = (2 * p2->x + 4 * p1->x + 3) / 6;
    const long nY2 = (2 * p2->y + 4 * p1->y + 3) / 6;
    rA.AddPoint( nX2, nY2, PolyFlags::Control );

    rA.AddPoint( p2->x, p2->y, PolyFlags::Normal );
    return 0;
}

static int FT_cubic_to( const FT_Vector* p1, const FT_Vector* p2, const FT_Vector* p3, void* vpPolyArgs )
{
    PolyArgs& rA = *static_cast<PolyArgs*>(vpPolyArgs);
    rA.AddPoint( p1->x, p1->y, PolyFlags::Control );
    rA.AddPoint( p2->x, p2->y, PolyFlags::Control );
    rA.AddPoint( p3->x, p3->y, PolyFlags::Normal );
    return 0;
}

} // extern "C"

bool FreetypeFont::GetGlyphOutline(sal_GlyphId nId, basegfx::B2DPolyPolygon& rB2DPolyPoly, bool bIsVertical) const
{
    if( maSizeFT )
        FT_Activate_Size( maSizeFT );

    rB2DPolyPoly.clear();

    FT_Int nLoadFlags = FT_LOAD_DEFAULT | FT_LOAD_IGNORE_TRANSFORM;

#ifdef FT_LOAD_TARGET_LIGHT
    // enable "light hinting" if available
    nLoadFlags |= FT_LOAD_TARGET_LIGHT;
#endif

    FT_Error rc = FT_Load_Glyph(maFaceFT, nId, nLoadFlags);
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

    ApplyGlyphTransform(bIsVertical, pGlyphFT);

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
    rB2DPolyPoly.transform(basegfx::utils::createScaleB2DHomMatrix( +1.0/(1<<6), -1.0/(1<<6) ));

    return true;
}

const unsigned char* FreetypeFont::GetTable(const char* pName, sal_uLong* pLength) const
{
    return mxFontInfo->GetTable( pName, pLength );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
