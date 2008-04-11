/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: fontmanager.cxx,v $
 * $Revision: 1.81 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_psprint.hxx"

#define Window XLIB_Window
#define Time XLIB_Time
#include <X11/Xlib.h>
#undef Window
#undef Time
#include <unistd.h>
#include <sys/stat.h>
#include <dirent.h>
#include <stdlib.h>
#include <osl/thread.h>
#include <unotools/atom.hxx>
#include <psprint/fontmanager.hxx>
#ifndef _PSPRINT_FONTCACHE_HXX_
#include <psprint/fontcache.hxx>
#endif
#include <tools/urlobj.hxx>
#include <tools/stream.hxx>
#include <psprint/helper.hxx>
#include <osl/file.hxx>
#include <osl/process.h>
#include <psprint/strhelper.hxx>
#include <psprint/ppdparser.hxx>
#include <rtl/tencinfo.h>
#include <tools/debug.hxx>
#include <tools/config.hxx>
#include <i18npool/mslangid.hxx>
#include <rtl/ustrbuf.hxx>
#include <rtl/strbuf.hxx>


#include <parseAFM.hxx>
#define NO_LIST
#include <sft.h>
#undef NO_LIST

#if OSL_DEBUG_LEVEL > 1
#include <sys/times.h>
#include <stdio.h>
#endif

#include <sal/alloca.h>

#include <set>
#include <hash_set>
#include <algorithm>

#include <adobeenc.tab> // get encoding table for AFM metrics

#ifdef CALLGRIND_COMPILE
#include <valgrind/callgrind.h>
#endif

#include <comphelper/processfactory.hxx>
#include <com/sun/star/beans/XMaterialHolder.hpp>
#include <com/sun/star/beans/NamedValue.hpp>

#define PRINTER_METRICDIR "fontmetric"

using namespace utl;
using namespace psp;
using namespace osl;
using namespace rtl;
using namespace com::sun::star::uno;
using namespace com::sun::star::beans;
using namespace com::sun::star::lang;

/*
 *  static helpers
 */

inline sal_uInt16 getUInt16BE( const sal_uInt8*& pBuffer )
{
    sal_uInt16 nRet = (sal_uInt16)pBuffer[1] |
        (((sal_uInt16)pBuffer[0]) << 8);
    pBuffer+=2;
    return nRet;
}

inline sal_uInt32 getUInt32BE( const sal_uInt8*& pBuffer )
{
    sal_uInt32 nRet = (((sal_uInt32)pBuffer[0]) << 24) |
                      (((sal_uInt32)pBuffer[1]) << 16) |
                      (((sal_uInt32)pBuffer[2]) << 8)  |
                      (((sal_uInt32)pBuffer[3]) );
    pBuffer += 4;
    return nRet;
}

static italic::type parseItalic( const ByteString& rItalic )
{
    italic::type eItalic = italic::Unknown;
    if( rItalic.EqualsIgnoreCaseAscii( "i" ) )
        eItalic = italic::Italic;
    else if( rItalic.EqualsIgnoreCaseAscii( "o" ) )
        eItalic = italic::Oblique;
    else
        eItalic = italic::Upright;
    return eItalic;
}

// -------------------------------------------------------------------------

static weight::type parseWeight( const ByteString& rWeight )
{
    weight::type eWeight = weight::Unknown;
    if( rWeight.Search( "bold" ) != STRING_NOTFOUND )
    {
        if( rWeight.Search( "emi" ) != STRING_NOTFOUND ) // semi, demi
            eWeight = weight::SemiBold;
        else if( rWeight.Search( "ultra" ) != STRING_NOTFOUND )
            eWeight = weight::UltraBold;
        else
            eWeight = weight::Bold;
    }
    else if( rWeight.Search( "heavy" ) != STRING_NOTFOUND )
        eWeight = weight::Bold;
    else if( rWeight.Search( "light" ) != STRING_NOTFOUND )
    {
        if( rWeight.Search( "emi" ) != STRING_NOTFOUND ) // semi, demi
            eWeight = weight::SemiLight;
        else if( rWeight.Search( "ultra" ) != STRING_NOTFOUND )
            eWeight = weight::UltraLight;
        else
            eWeight = weight::Light;
    }
    else if( rWeight.Search( "black" ) != STRING_NOTFOUND )
        eWeight = weight::Black;
    else if( rWeight.Equals( "demi" ) )
        eWeight = weight::SemiBold;
    else if( rWeight.Equals( "book" ) ||
             rWeight.Equals( "semicondensed" ) )
        eWeight = weight::Light;
    else if( rWeight.Equals( "medium" ) || rWeight.Equals( "roman" ) )
        eWeight = weight::Medium;
    else
        eWeight = weight::Normal;
    return eWeight;
}

// -------------------------------------------------------------------------

static width::type parseWidth( const ByteString& rWidth )
{
    width::type eWidth = width::Unknown;
    if( rWidth.Equals( "bold" ) ||
        rWidth.Equals( "semiexpanded" ) )
        eWidth = width::SemiExpanded;
    else if( rWidth.Equals( "condensed" ) ||
             rWidth.Equals( "narrow" ) )
        eWidth = width::Condensed;
    else if( rWidth.Equals( "double wide" ) ||
             rWidth.Equals( "extraexpanded" ) ||
             rWidth.Equals( "ultraexpanded" ) )
        eWidth = width::UltraExpanded;
    else if( rWidth.Equals( "expanded" ) ||
             rWidth.Equals( "wide" ) )
        eWidth = width::Expanded;
    else if( rWidth.Equals( "extracondensed" ) )
        eWidth = width::ExtraCondensed;
    else if( rWidth.Equals( "semicondensed" ) )
        eWidth = width::SemiCondensed;
    else if( rWidth.Equals( "ultracondensed" ) )
        eWidth = width::UltraCondensed;
    else
        eWidth = width::Normal;

    return eWidth;
}

// -------------------------------------------------------------------------
bool PrintFontManager::XLFDEntry::operator<(const PrintFontManager::XLFDEntry& rRight) const
{
    sal_Int32 nCmp = 0;
    if( (nMask & MaskFamily) && (rRight.nMask & MaskFamily) )
    {
        nCmp =  rtl_str_compareIgnoreAsciiCase_WithLength( aFamily.pData->buffer,
                                                           aFamily.pData->length,
                                                           rRight.aFamily.pData->buffer,
                                                           rRight.aFamily.pData->length );
        if( nCmp != 0 )
            return nCmp < 0;
    }

    if( (nMask & MaskFoundry) && (rRight.nMask & MaskFoundry) )
    {
        nCmp =  rtl_str_compareIgnoreAsciiCase_WithLength( aFoundry.pData->buffer,
                                                           aFoundry.pData->length,
                                                           rRight.aFoundry.pData->buffer,
                                                           rRight.aFoundry.pData->length );
        if( nCmp != 0 )
            return nCmp < 0;
    }

    if( (nMask & MaskItalic) && (rRight.nMask & MaskItalic) )
    {
        if( eItalic != rRight.eItalic )
            return (int)eItalic < (int)rRight.eItalic;
    }

    if( (nMask & MaskWeight) && (rRight.nMask & MaskWeight) )
    {
        if( eWeight != rRight.eWeight )
            return (int)eWeight < (int)rRight.eWeight;
    }

    if( (nMask & MaskWidth) && (rRight.nMask & MaskWidth) )
    {
        if( eWidth != rRight.eWidth )
            return (int)eWidth < (int)rRight.eWidth;
    }

    if( (nMask & MaskPitch) && (rRight.nMask & MaskPitch) )
    {
        if( ePitch != rRight.ePitch )
            return (int)ePitch < (int)rRight.ePitch;
    }

    if( (nMask & MaskAddStyle) && (rRight.nMask & MaskAddStyle) )
    {
        nCmp =  rtl_str_compareIgnoreAsciiCase_WithLength( aAddStyle.pData->buffer,
                                                           aAddStyle.pData->length,
                                                           rRight.aAddStyle.pData->buffer,
                                                           rRight.aAddStyle.pData->length );
        if( nCmp != 0 )
            return nCmp < 0;
    }

    if( (nMask & MaskEncoding) && (rRight.nMask & MaskEncoding) )
    {
        if( aEncoding != rRight.aEncoding )
            return aEncoding < rRight.aEncoding;
    }

    return false;
}

bool PrintFontManager::XLFDEntry::operator==(const PrintFontManager::XLFDEntry& rRight) const
{
    sal_Int32 nCmp = 0;
    if( (nMask & MaskFamily) && (rRight.nMask & MaskFamily) )
    {
        nCmp =  rtl_str_compareIgnoreAsciiCase_WithLength( aFamily.pData->buffer,
                                                           aFamily.pData->length,
                                                           rRight.aFamily.pData->buffer,
                                                           rRight.aFamily.pData->length );
        if( nCmp != 0 )
            return false;
    }

    if( (nMask & MaskFoundry) && (rRight.nMask & MaskFoundry) )
    {
        nCmp =  rtl_str_compareIgnoreAsciiCase_WithLength( aFoundry.pData->buffer,
                                                           aFoundry.pData->length,
                                                           rRight.aFoundry.pData->buffer,
                                                           rRight.aFoundry.pData->length );
        if( nCmp != 0 )
            return false;
    }

    if( (nMask & MaskItalic) && (rRight.nMask & MaskItalic) )
    {
        if( eItalic != rRight.eItalic )
            return false;
    }

    if( (nMask & MaskWeight) && (rRight.nMask & MaskWeight) )
    {
        if( eWeight != rRight.eWeight )
            return false;
    }

    if( (nMask & MaskWidth) && (rRight.nMask & MaskWidth) )
    {
        if( eWidth != rRight.eWidth )
            return false;
    }

    if( (nMask & MaskPitch) && (rRight.nMask & MaskPitch) )
    {
        if( ePitch != rRight.ePitch )
            return false;
    }

    if( (nMask & MaskAddStyle) && (rRight.nMask & MaskAddStyle) )
    {
        nCmp =  rtl_str_compareIgnoreAsciiCase_WithLength( aAddStyle.pData->buffer,
                                                           aAddStyle.pData->length,
                                                           rRight.aAddStyle.pData->buffer,
                                                           rRight.aAddStyle.pData->length );
        if( nCmp != 0 )
            return false;
    }

    if( (nMask & MaskEncoding) && (rRight.nMask & MaskEncoding) )
    {
        if( aEncoding != rRight.aEncoding )
            return false;
    }

    return true;
}

/*
 *  PrintFont implementations
 */
PrintFontManager::PrintFont::PrintFont( fonttype::type eType ) :
        m_eType( eType ),
        m_nFamilyName( 0 ),
        m_nPSName( 0 ),
        m_eItalic( italic::Unknown ),
        m_eWidth( width::Unknown ),
        m_eWeight( weight::Unknown ),
        m_ePitch( pitch::Unknown ),
        m_aEncoding( RTL_TEXTENCODING_DONTKNOW ),
        m_bFontEncodingOnly( false ),
        m_pMetrics( NULL ),
        m_nAscend( 0 ),
        m_nDescend( 0 ),
        m_nLeading( 0 ),
        m_nXMin( 0 ),
        m_nYMin( 0 ),
        m_nXMax( 0 ),
        m_nYMax( 0 ),
        m_bHaveVerticalSubstitutedGlyphs( false ),
        m_bUserOverride( false ),
        m_eEmbeddedbitmap( fcstatus::isunset ),
        m_eAntialias( fcstatus::isunset )
{
}

// -------------------------------------------------------------------------

PrintFontManager::PrintFont::~PrintFont()
{
    if( m_pMetrics )
        delete m_pMetrics;
}

// -------------------------------------------------------------------------

PrintFontManager::Type1FontFile::~Type1FontFile()
{
}

// -------------------------------------------------------------------------

PrintFontManager::TrueTypeFontFile::~TrueTypeFontFile()
{
}

// -------------------------------------------------------------------------

PrintFontManager::BuiltinFont::~BuiltinFont()
{
}

// -------------------------------------------------------------------------

bool PrintFontManager::Type1FontFile::queryMetricPage( int /*nPage*/, MultiAtomProvider* pProvider )
{
    return readAfmMetrics( PrintFontManager::get().getAfmFile( this ), pProvider, false, false );
}

// -------------------------------------------------------------------------

bool PrintFontManager::BuiltinFont::queryMetricPage( int /*nPage*/, MultiAtomProvider* pProvider )
{
    return readAfmMetrics( PrintFontManager::get().getAfmFile( this ), pProvider, false, false );
}

// -------------------------------------------------------------------------

bool PrintFontManager::TrueTypeFontFile::queryMetricPage( int nPage, MultiAtomProvider* pProvider )
{
    bool bSuccess = false;

    ByteString aFile( PrintFontManager::get().getFontFile( this ) );

    TrueTypeFont* pTTFont = NULL;

    if( OpenTTFontFile( aFile.GetBuffer(), m_nCollectionEntry < 0 ? 0 : m_nCollectionEntry, &pTTFont ) == SF_OK )
    {
        if( ! m_pMetrics )
        {
            m_pMetrics = new PrintFontMetrics;
            memset (m_pMetrics->m_aPages, 0, sizeof(m_pMetrics->m_aPages));
        }
        m_pMetrics->m_aPages[ nPage/8 ] |= (1 << ( nPage & 7 ));
        int i;
        sal_uInt16 table[256], table_vert[256];

        for( i = 0; i < 256; i++ )
            table[ i ] = 256*nPage + i;

        int nCharacters = nPage < 255 ? 256 : 254;
        MapString( pTTFont, table, nCharacters, NULL, 0 );
        TTSimpleGlyphMetrics* pMetrics = GetTTSimpleCharMetrics( pTTFont, nPage*256, nCharacters, 0 );
        if( pMetrics )
        {
            for( i = 0; i < nCharacters; i++ )
            {
                if( table[i] )
                {
                    CharacterMetric& rChar = m_pMetrics->m_aMetrics[ nPage*256 + i ];
                    rChar.width = pMetrics[ i ].adv;
                    rChar.height = m_aGlobalMetricX.height;
                }
            }

            free( pMetrics );
        }

        for( i = 0; i < 256; i++ )
            table_vert[ i ] = 256*nPage + i;
        MapString( pTTFont, table_vert, nCharacters, NULL, 1 );
        pMetrics = GetTTSimpleCharMetrics( pTTFont, nPage*256, nCharacters, 1 );
        if( pMetrics )
        {
            for( i = 0; i < nCharacters; i++ )
            {
                if( table_vert[i] )
                {
                    CharacterMetric& rChar = m_pMetrics->m_aMetrics[ nPage*256 + i + ( 1 << 16 ) ];
                    rChar.width = m_aGlobalMetricY.width;
                    rChar.height = pMetrics[ i ].adv;
                    if( table_vert[i] != table[i] )
                        m_pMetrics->m_bVerticalSubstitutions[ nPage*256 + i ] = 1;
                }
            }
            free( pMetrics );
        }

        if( ! m_pMetrics->m_bKernPairsQueried )
        {
            m_pMetrics->m_bKernPairsQueried = true;
            // this is really a hack
            // in future MapString/KernGlyphs should be used
            // but vcl is not in a state where that could be used
            // so currently we get kernpairs by accessing the raw data
            struct _TrueTypeFont* pImplTTFont = (struct _TrueTypeFont*)pTTFont;

            //-----------------------------------------------------------------
            // Kerning:  KT_MICROSOFT
            //-----------------------------------------------------------------
            if( pImplTTFont->nkern && pImplTTFont->kerntype == KT_MICROSOFT )
            {
                // create a glyph -> character mapping
                ::std::hash_map< sal_uInt16, sal_Unicode > aGlyphMap;
                ::std::hash_map< sal_uInt16, sal_Unicode >::iterator left, right;
                for( i = 21; i < 0xfffd; i++ )
                {
                    sal_uInt16 nGlyph = MapChar( pTTFont, (sal_Unicode)i, 0 ); // kerning for horz only
                    if( nGlyph != 0 )
                        aGlyphMap[ nGlyph ] = (sal_Unicode)i;
                }


                KernPair aPair;
                for( i = 0; i < (int)pImplTTFont->nkern; i++ )
                {
                    const sal_uInt8* pTable = pImplTTFont->kerntables[i];

                    /*sal_uInt16 nVersion     =*/ getUInt16BE( pTable );
                    /*sal_uInt16 nLength      =*/ getUInt16BE( pTable );
                    sal_uInt16 nCoverage    = getUInt16BE( pTable );

                    aPair.kern_x    = 0;
                    aPair.kern_y    = 0;
                    switch( nCoverage >> 8 )
                    {
                        case 0:
                        {
                            sal_uInt16 nPairs = getUInt16BE( pTable );
                            pTable += 6;
                            for( int n = 0; n < nPairs; n++ )
                            {
                                sal_uInt16 nLeftGlyph   = getUInt16BE( pTable );
                                sal_uInt16 nRightGlyph  = getUInt16BE( pTable );
                                sal_Int16 nKern         = (sal_Int16)getUInt16BE( pTable );

                                left = aGlyphMap.find( nLeftGlyph );
                                right = aGlyphMap.find( nRightGlyph );
                                if( left != aGlyphMap.end() && right != aGlyphMap.end() )
                                {
                                    aPair.first     = left->second;
                                    aPair.second    = right->second;
                                    switch( nCoverage & 1 )
                                    {
                                        case 1:
                                            aPair.kern_x = (int)nKern * 1000 / pImplTTFont->unitsPerEm;
                                            m_pMetrics->m_aXKernPairs.push_back( aPair );
                                            break;
                                        case 0:
                                            aPair.kern_y = (int)nKern * 1000 / pImplTTFont->unitsPerEm;
                                            m_pMetrics->m_aYKernPairs.push_back( aPair );
                                            break;
                                    }
                                }
                            }
                        }
                        break;

                        case 2:
                        {
                            const sal_uInt8* pSubTable = pTable;
                            /*sal_uInt16 nRowWidth    =*/ getUInt16BE( pTable );
                            sal_uInt16 nOfLeft      = getUInt16BE( pTable );
                            sal_uInt16 nOfRight     = getUInt16BE( pTable );
                            /*sal_uInt16 nOfArray     =*/ getUInt16BE( pTable );
                            const sal_uInt8* pTmp = pSubTable + nOfLeft;
                            sal_uInt16 nFirstLeft   = getUInt16BE( pTmp );
                            sal_uInt16 nLastLeft    = getUInt16BE( pTmp ) + nFirstLeft - 1;
                            pTmp = pSubTable + nOfRight;
                            sal_uInt16 nFirstRight  = getUInt16BE( pTmp );
                            sal_uInt16 nLastRight   = getUInt16BE( pTmp ) + nFirstRight -1;

                            // int nPairs = (int)(nLastLeft-nFirstLeft+1)*(int)(nLastRight-nFirstRight+1);
                            for( aPair.first = nFirstLeft; aPair.first < nLastLeft; aPair.first++ )
                            {
                                for( aPair.second = 0; aPair.second < nLastRight; aPair.second++ )
                                {
                                    sal_Int16 nKern = (sal_Int16)getUInt16BE( pTmp );
                                    switch( nCoverage & 1 )
                                    {
                                        case 1:
                                            aPair.kern_x = (int)nKern * 1000 / pImplTTFont->unitsPerEm;
                                            m_pMetrics->m_aXKernPairs.push_back( aPair );
                                            break;
                                        case 0:
                                            aPair.kern_y = (int)nKern * 1000 / pImplTTFont->unitsPerEm;
                                            m_pMetrics->m_aYKernPairs.push_back( aPair );
                                            break;
                                    }
                                }
                            }
                        }
                        break;
                    }
                }
            }

            //-----------------------------------------------------------------
            // Kerning:  KT_APPLE_NEW
            //-----------------------------------------------------------------
            if( pImplTTFont->nkern && pImplTTFont->kerntype == KT_APPLE_NEW )
            {
                // create a glyph -> character mapping
                ::std::hash_map< sal_uInt16, sal_Unicode > aGlyphMap;
                ::std::hash_map< sal_uInt16, sal_Unicode >::iterator left, right;
                for( i = 21; i < 0xfffd; i++ )
                {
                    sal_uInt16 nGlyph = MapChar( pTTFont, (sal_Unicode)i, 0 ); // kerning for horz only
                    if( nGlyph != 0 )
                        aGlyphMap[ nGlyph ] = (sal_Unicode)i;
                }

                // Loop through each of the 'kern' subtables
                KernPair aPair;
                for( i = 0; (unsigned int)i < pImplTTFont->nkern; i++ )
                {
                    const sal_uInt8* pTable = pImplTTFont->kerntables[i];

                    /*sal_uInt32 nLength      =*/ getUInt32BE( pTable );
                    sal_uInt16 nCoverage    = getUInt16BE( pTable );
                    /*sal_uInt16 nTupleIndex  =*/ getUInt16BE( pTable );

                    // Get kerning type
                    // sal_Bool bKernVertical     = nCoverage & 0x8000;
                    // sal_Bool bKernCrossStream  = nCoverage & 0x4000;
                    // sal_Bool bKernVariation    = nCoverage & 0x2000;

                    // Kerning sub-table format, 0 through 3
                    sal_uInt8 nSubTableFormat  = nCoverage & 0x00FF;

                    aPair.kern_x    = 0;
                    aPair.kern_y    = 0;
                    switch( nSubTableFormat )
                    {
                        case 0:
                        {
                            // Grab the # of kern pairs but skip over the:
                            //   searchRange
                            //   entrySelector
                            //   rangeShift
                            sal_uInt16 nPairs = getUInt16BE( pTable );
                            pTable += 6;

                            for( int n = 0; n < nPairs; n++ )
                            {
                                sal_uInt16 nLeftGlyph   = getUInt16BE( pTable );
                                sal_uInt16 nRightGlyph  = getUInt16BE( pTable );
                                sal_Int16  nKern         = (sal_Int16)getUInt16BE( pTable );

                                left = aGlyphMap.find( nLeftGlyph );
                                right = aGlyphMap.find( nRightGlyph );
                                if( left != aGlyphMap.end() && right != aGlyphMap.end() )
                                {
                                    aPair.first     = left->second;
                                    aPair.second    = right->second;

                                    // Only support horizontal kerning for now
                                    aPair.kern_x = (int)nKern * 1000 / pImplTTFont->unitsPerEm;
                                    aPair.kern_y = 0;
                                    m_pMetrics->m_aXKernPairs.push_back( aPair );

/*                                  switch( nCoverage & 1 )
                                    {
                                        case 1:
                                            aPair.kern_x = (int)nKern * 1000 / pImplTTFont->unitsPerEm;
                                            m_pMetrics->m_aXKernPairs.push_back( aPair );
                                            break;
                                        case 0:
                                            aPair.kern_y = (int)nKern * 1000 / pImplTTFont->unitsPerEm;
                                            m_pMetrics->m_aYKernPairs.push_back( aPair );
                                            break;
                                    }
*/
                                }
                            }
                        }
                        break;

                        case 2:
                        {
                            const sal_uInt8* pSubTable = pTable;
                            /*sal_uInt16 nRowWidth    =*/ getUInt16BE( pTable );
                            sal_uInt16 nOfLeft      = getUInt16BE( pTable );
                            sal_uInt16 nOfRight     = getUInt16BE( pTable );
                            /*sal_uInt16 nOfArray     =*/ getUInt16BE( pTable );
                            const sal_uInt8* pTmp = pSubTable + nOfLeft;
                            sal_uInt16 nFirstLeft   = getUInt16BE( pTmp );
                            sal_uInt16 nLastLeft    = getUInt16BE( pTmp ) + nFirstLeft - 1;
                            pTmp = pSubTable + nOfRight;
                            sal_uInt16 nFirstRight  = getUInt16BE( pTmp );
                            sal_uInt16 nLastRight   = getUInt16BE( pTmp ) + nFirstRight -1;

                            for( aPair.first = nFirstLeft; aPair.first < nLastLeft; aPair.first++ )
                            {
                                for( aPair.second = 0; aPair.second < nLastRight; aPair.second++ )
                                {
                                    sal_Int16 nKern = (sal_Int16)getUInt16BE( pTmp );
                                    switch( nCoverage & 1 )
                                    {
                                        case 1:
                                            aPair.kern_x = (int)nKern * 1000 / pImplTTFont->unitsPerEm;
                                            m_pMetrics->m_aXKernPairs.push_back( aPair );
                                            break;
                                        case 0:
                                            aPair.kern_y = (int)nKern * 1000 / pImplTTFont->unitsPerEm;
                                            m_pMetrics->m_aYKernPairs.push_back( aPair );
                                            break;
                                    }
                                }
                            }
                        }
                        break;

                        default:
                            fprintf( stderr, "Found unsupported Apple-style kern subtable type %d.\n", nSubTableFormat );
                            break;
                    }
                }
            }

#if OSL_DEBUG_LEVEL > 1
            fprintf( stderr, "found %d/%d kern pairs for %s\n",
                     m_pMetrics->m_aXKernPairs.size(),
                     m_pMetrics->m_aYKernPairs.size(),
                     OUStringToOString( pProvider->getString( ATOM_FAMILYNAME, m_nFamilyName ), RTL_TEXTENCODING_MS_1252 ).getStr() );
#else
            (void) pProvider; /* avoid warnings */
#endif
        }

        CloseTTFont( pTTFont );
        bSuccess = true;
    }
    return bSuccess;
}

// -------------------------------------------------------------------------

/* #i73387# There seem to be fonts with a rather unwell chosen family name
*  consider e.g. "Helvetica Narrow" which defines its family as "Helvetica"
*  It can really only be distinguished by its PSName and FullName. Both of
*  which are not user presentable in OOo. So replace it by something sensible.
*
*  If other fonts feature this behaviour, insert them to the map.
*/
static bool familyNameOverride( const OUString& i_rPSname, OUString& o_rFamilyName )
{
    static std::hash_map< OUString, OUString, OUStringHash > aPSNameToFamily( 16 );
    if( aPSNameToFamily.empty() ) // initialization
    {
        aPSNameToFamily[ OUString( RTL_CONSTASCII_USTRINGPARAM( "Helvetica-Narrow" ) ) ] =
                         OUString( RTL_CONSTASCII_USTRINGPARAM( "Helvetica Narrow" ) );
        aPSNameToFamily[ OUString( RTL_CONSTASCII_USTRINGPARAM( "Helvetica-Narrow-Bold" ) ) ] =
                         OUString( RTL_CONSTASCII_USTRINGPARAM( "Helvetica Narrow" ) );
        aPSNameToFamily[ OUString( RTL_CONSTASCII_USTRINGPARAM( "Helvetica-Narrow-BoldOblique" ) ) ] =
                         OUString( RTL_CONSTASCII_USTRINGPARAM( "Helvetica Narrow" ) );
        aPSNameToFamily[ OUString( RTL_CONSTASCII_USTRINGPARAM( "Helvetica-Narrow-Oblique" ) ) ] =
                         OUString( RTL_CONSTASCII_USTRINGPARAM( "Helvetica Narrow" ) );
    }
    std::hash_map<OUString,OUString,OUStringHash>::const_iterator it =
       aPSNameToFamily.find( i_rPSname );
    bool bReplaced = (it != aPSNameToFamily.end() );
    if( bReplaced )
        o_rFamilyName = it->second;
    return bReplaced;
};

bool PrintFontManager::PrintFont::readAfmMetrics( const OString& rFileName, MultiAtomProvider* pProvider, bool bFillEncodingvector, bool bOnlyGlobalAttributes )
{
    PrintFontManager& rManager( PrintFontManager::get() );

    int i;
    FontInfo* pInfo = NULL;
    parseFile( rFileName.getStr(), &pInfo, P_ALL );
    if( ! pInfo || ! pInfo->numOfChars )
    {
        if( pInfo )
            freeFontInfo( pInfo );
        return false;
    }

    m_aEncodingVector.clear();
    // fill in global info

    // PSName
    OUString aPSName( OStringToOUString( pInfo->gfi->fontName, RTL_TEXTENCODING_ISO_8859_1 ) );
    m_nPSName = pProvider->getAtom( ATOM_PSNAME, aPSName, sal_True );

    // family name (if not already set)
    OUString aFamily;
    if( ! m_nFamilyName )
    {
        aFamily = OStringToOUString( pInfo->gfi->familyName, RTL_TEXTENCODING_ISO_8859_1 );
        if( ! aFamily.getLength() )
        {
            aFamily = OStringToOUString( pInfo->gfi->fontName, RTL_TEXTENCODING_ISO_8859_1 );
            sal_Int32 nIndex  = 0;
            aFamily = aFamily.getToken( 0, '-', nIndex );
        }
        familyNameOverride( aPSName, aFamily );
        m_nFamilyName = pProvider->getAtom( ATOM_FAMILYNAME, aFamily, sal_True );
    }
    else
        aFamily = pProvider->getString( ATOM_FAMILYNAME, m_nFamilyName );

    // style name: if fullname begins with family name
    // interpret the rest of fullname as style
    if( ! m_aStyleName.getLength() && pInfo->gfi->fullName && *pInfo->gfi->fullName )
    {
        OUString aFullName( OStringToOUString( pInfo->gfi->fullName, RTL_TEXTENCODING_ISO_8859_1 ) );
        if( aFullName.indexOf( aFamily ) == 0 )
            m_aStyleName = WhitespaceToSpace( aFullName.copy( aFamily.getLength() ) );
    }

    // italic
    if( pInfo->gfi->italicAngle > 0 )
        m_eItalic = italic::Oblique;
    else if( pInfo->gfi->italicAngle < 0 )
        m_eItalic = italic::Italic;
    else
        m_eItalic = italic::Upright;

    // weight
    ByteString aLowerWeight( pInfo->gfi->weight );
    aLowerWeight.ToLowerAscii();
    m_eWeight = parseWeight( aLowerWeight );

    // pitch
    m_ePitch = pInfo->gfi->isFixedPitch ? pitch::Fixed : pitch::Variable;

    // encoding - only set if unknown
    int nAdobeEncoding = 0;
    if( pInfo->gfi->encodingScheme )
    {
        if( !strcmp( pInfo->gfi->encodingScheme, "AdobeStandardEncoding" ) )
            nAdobeEncoding = 1;
        else if( !strcmp( pInfo->gfi->encodingScheme, "ISO10646-1" ) )
        {
            nAdobeEncoding = 1;
            m_aEncoding = RTL_TEXTENCODING_UNICODE;
        }
        else if( !strcmp( pInfo->gfi->encodingScheme, "Symbol") )
            nAdobeEncoding = 2;
        else if( !strcmp( pInfo->gfi->encodingScheme, "FontSpecific") )
            nAdobeEncoding = 3;

        if( m_aEncoding == RTL_TEXTENCODING_DONTKNOW )
            m_aEncoding = nAdobeEncoding == 1 ?
                RTL_TEXTENCODING_ADOBE_STANDARD : RTL_TEXTENCODING_SYMBOL;
    }
    else if( m_aEncoding == RTL_TEXTENCODING_DONTKNOW )
        m_aEncoding = RTL_TEXTENCODING_ADOBE_STANDARD;

    // try to parse the font name and decide wether it might be a
    // japanese font. Who invented this PITA ?
    OUString aPSNameLastToken( aPSName.copy( aPSName.lastIndexOf( '-' )+1 ) );
    if( ! aPSNameLastToken.compareToAscii( "H" )    ||
        ! aPSNameLastToken.compareToAscii( "V" )  )
    {
        static const char* pEncs[] =
            {
                "EUC",
                "RKSJ",
                "SJ"
            };
        static const rtl_TextEncoding aEncs[] =
            {
                RTL_TEXTENCODING_EUC_JP,
                RTL_TEXTENCODING_SHIFT_JIS,
                RTL_TEXTENCODING_JIS_X_0208
            };

        for( unsigned int enc = 0; enc < sizeof( aEncs )/sizeof(aEncs[0]) && m_aEncoding == RTL_TEXTENCODING_DONTKNOW; enc++ )
        {
            sal_Int32 nIndex = 0, nOffset = 1;
            do
            {
                OUString aToken( aPSName.getToken( nOffset, '-', nIndex ) );
                if( nIndex == -1 )
                    break;
                nOffset = 0;
                if( ! aToken.compareToAscii( pEncs[enc] ) )
                {
                    m_aEncoding = aEncs[ enc ];
                    m_bFontEncodingOnly = true;
                }
            } while( nIndex != -1 );
        }

        // default is jis
        if( m_aEncoding == RTL_TEXTENCODING_DONTKNOW )
            m_aEncoding = RTL_TEXTENCODING_JIS_X_0208;
#if OSL_DEBUG_LEVEL > 1
        fprintf( stderr, "Encoding %d for %s\n", m_aEncoding, pInfo->gfi->fontName );
#endif
    }

    // hack for GB encoded builtin fonts posing as FontSpecific
    if( m_eType == fonttype::Builtin && ( nAdobeEncoding == 3 || nAdobeEncoding == 0 ) )
    {
        int nLen = aFamily.getLength();
        if( nLen > 2 &&
            aFamily.getStr()[ nLen-2 ] == 'G' &&
            aFamily.getStr()[ nLen-1 ] == 'B' &&
            pInfo->numOfChars > 255 )
        {
            m_aEncoding = RTL_TEXTENCODING_GBK;
            m_bFontEncodingOnly = true;
#if OSL_DEBUG_LEVEL > 1
            fprintf( stderr, "found builtin font %s with GBK encoding\n", pInfo->gfi->fontName );
#endif
        }
    }

    // #i37313# check if Fontspecific is not rather some character encoding
    if( nAdobeEncoding == 3 && m_aEncoding == RTL_TEXTENCODING_SYMBOL )
    {
        bool bYFound = false;
        bool bQFound = false;
        CharMetricInfo* pChar = pInfo->cmi;
        for( int j = 0; j < pInfo->numOfChars && ! (bYFound && bQFound); j++ )
        {
            if( pChar[j].name )
            {
                if( pChar[j].name[0] == 'Y' && pChar[j].name[1] == 0 )
                    bYFound = true;
                else if( pChar[j].name[0] == 'Q' && pChar[j].name[1] == 0 )
                    bQFound = true;
            }
        }
        if( bQFound && bYFound )
        {
            #if OSL_DEBUG_LEVEL > 1
            fprintf( stderr, "setting FontSpecific font %s (file %s) to unicode\n",
                     pInfo->gfi->fontName,
                     rFileName.getStr()
                     );
            #endif
            nAdobeEncoding = 4;
            m_aEncoding = RTL_TEXTENCODING_UNICODE;
            bFillEncodingvector = false; // will be filled anyway, don't do the work twice
        }
    }

    // ascend
    m_nAscend = pInfo->gfi->fontBBox.ury;

    // descend
    // descends have opposite sign of our definition
    m_nDescend = -pInfo->gfi->fontBBox.lly;

    // fallback to ascender, descender
    // interesting: the BBox seems to describe Ascender and Descender better
    // as we understand it
    if( m_nAscend == 0 )
        m_nAscend = pInfo->gfi->ascender;
    if( m_nDescend == 0)
        m_nDescend = -pInfo->gfi->descender;

    m_nLeading = m_nAscend + m_nDescend - 1000;

    if( m_pMetrics )
        delete m_pMetrics;
    m_pMetrics = new PrintFontMetrics;
    // mark all pages as queried (or clear if only global font info queiried)
    memset( m_pMetrics->m_aPages, bOnlyGlobalAttributes ? 0 : 0xff, sizeof( m_pMetrics->m_aPages ) );

    m_aGlobalMetricX.width = m_aGlobalMetricY.width =
        pInfo->gfi->charwidth ? pInfo->gfi->charwidth : pInfo->gfi->fontBBox.urx;
    m_aGlobalMetricX.height = m_aGlobalMetricY.height =
        pInfo->gfi->capHeight ? pInfo->gfi->capHeight : pInfo->gfi->fontBBox.ury;

    m_nXMin = pInfo->gfi->fontBBox.llx;
    m_nYMin = pInfo->gfi->fontBBox.lly;
    m_nXMax = pInfo->gfi->fontBBox.urx;
    m_nYMax = pInfo->gfi->fontBBox.ury;

    if( bFillEncodingvector || !bOnlyGlobalAttributes )
    {
        // fill in character metrics

        // first transform the character codes to unicode
        // note: this only works with single byte encodings
        sal_Unicode* pUnicodes = (sal_Unicode*)alloca( pInfo->numOfChars * sizeof(sal_Unicode));
        CharMetricInfo* pChar = pInfo->cmi;

        for( i = 0; i < pInfo->numOfChars; i++, pChar++ )
        {
            if( nAdobeEncoding == 4 )
            {
                if( pChar->name )
                {
                    pUnicodes[i] = 0;
                    std::list< sal_Unicode > aCodes = rManager.getUnicodeFromAdobeName( pChar->name );
                    for( std::list< sal_Unicode >::const_iterator it = aCodes.begin(); it != aCodes.end(); ++it )
                    {
                        if( *it != 0 )
                        {
                            m_aEncodingVector[ *it ] = pChar->code;
                            if( pChar->code == -1 )
                                m_aNonEncoded[ *it ] = pChar->name;
                            if( ! pUnicodes[i] ) // map the first
                                pUnicodes[i] = *it;
                        }
                    }
                }
            }
            else if( pChar->code != -1 )
            {
                if( nAdobeEncoding == 3 && m_aEncoding == RTL_TEXTENCODING_SYMBOL )
                {
                    pUnicodes[i] = pChar->code + 0xf000;
                    if( bFillEncodingvector )
                        m_aEncodingVector[ pUnicodes[i] ] = pChar->code;
                    continue;
                }

                if( m_aEncoding == RTL_TEXTENCODING_UNICODE )
                {
                    pUnicodes[i] = (sal_Unicode)pChar->code;
                    continue;
                }

                ByteString aTranslate;
                if( pChar->code & 0xff000000 )
                    aTranslate += (char)(pChar->code >> 24 );
                if( pChar->code & 0xffff0000 )
                    aTranslate += (char)((pChar->code & 0x00ff0000) >> 16 );
                if( pChar->code & 0xffffff00 )
                    aTranslate += (char)((pChar->code & 0x0000ff00) >> 8 );
                aTranslate += (char)(pChar->code & 0xff);
                String aUni( aTranslate, m_aEncoding );
                pUnicodes[i] = *aUni.GetBuffer();
            }
            else
                pUnicodes[i] = 0;
        }

        // now fill in the character metrics
        // parseAFM.cxx effectively only supports direction 0 (horizontal)
        pChar = pInfo->cmi;
        CharacterMetric aMetric;
        for( i = 0; i < pInfo->numOfChars; i++, pChar++ )
        {
            if( pChar->code == -1 && ! pChar->name )
                continue;

            if( bFillEncodingvector && pChar->name )
            {
                std::list< sal_Unicode > aCodes = rManager.getUnicodeFromAdobeName( pChar->name );
                for( std::list< sal_Unicode >::const_iterator it = aCodes.begin(); it != aCodes.end(); ++it )
                {
                    if( *it != 0 )
                    {
                        m_aEncodingVector[ *it ] = pChar->code;
                        if( pChar->code == -1 )
                            m_aNonEncoded[ *it ] = pChar->name;
                    }
                }
            }

            aMetric.width   = pChar->wx ? pChar->wx : pChar->charBBox.urx;
            aMetric.height  = pChar->wy ? pChar->wy : pChar->charBBox.ury - pChar->charBBox.lly;
            if( aMetric.width == 0 && aMetric.height == 0 )
                // guess something for e.g. space
                aMetric.width = m_aGlobalMetricX.width/4;

            if( ( nAdobeEncoding == 0 ) ||
                ( ( nAdobeEncoding == 3 ) && ( m_aEncoding != RTL_TEXTENCODING_SYMBOL ) ) )
            {
                if( pChar->code != -1 )
                {
                    m_pMetrics->m_aMetrics[ pUnicodes[i] ] = aMetric;
                    if( bFillEncodingvector )
                        m_aEncodingVector[ pUnicodes[i] ] = pChar->code;
                }
                else if( pChar->name )
                {
                    std::list< sal_Unicode > aCodes = rManager.getUnicodeFromAdobeName( pChar->name );
                    for( std::list< sal_Unicode >::const_iterator it = aCodes.begin(); it != aCodes.end(); ++it )
                    {
                        if( *it != 0 )
                            m_pMetrics->m_aMetrics[ *it ] = aMetric;
                    }
                }
            }
            else if( nAdobeEncoding == 1 || nAdobeEncoding == 2 || nAdobeEncoding == 4)
            {
                if( pChar->name )
                {
                    std::list< sal_Unicode > aCodes = rManager.getUnicodeFromAdobeName( pChar->name );
                    for( std::list< sal_Unicode >::const_iterator it = aCodes.begin(); it != aCodes.end(); ++it )
                    {
                        if( *it != 0 )
                            m_pMetrics->m_aMetrics[ *it ] = aMetric;
                    }
                }
                else if( pChar->code != -1 )
                {
                    ::std::pair< ::std::hash_multimap< sal_uInt8, sal_Unicode >::const_iterator,
                          ::std::hash_multimap< sal_uInt8, sal_Unicode >::const_iterator >
                          aCodes = rManager.getUnicodeFromAdobeCode( pChar->code );
                    while( aCodes.first != aCodes.second )
                    {
                        if( (*aCodes.first).second != 0 )
                        {
                            m_pMetrics->m_aMetrics[ (*aCodes.first).second ] = aMetric;
                            if( bFillEncodingvector )
                                m_aEncodingVector[ (*aCodes.first).second ] = pChar->code;
                        }
                        ++aCodes.first;
                    }
                }
            }
            else if( nAdobeEncoding == 3 )
            {
                if( pChar->code != -1 )
                {
                    sal_Unicode code = 0xf000 + pChar->code;
                    m_pMetrics->m_aMetrics[ code ] = aMetric;
                    // maybe should try to find the name in the convtabs ?
                    if( bFillEncodingvector )
                        m_aEncodingVector[ code ] = pChar->code;
                }
            }
        }

        m_pMetrics->m_aXKernPairs.clear();
        m_pMetrics->m_aYKernPairs.clear();

        // now fill in the kern pairs
        // parseAFM.cxx effectively only supports direction 0 (horizontal)
        PairKernData* pKern = pInfo->pkd;
        KernPair aPair;
        for( i = 0; i < pInfo->numOfPairs; i++, pKern++ )
        {
            // #i37703# broken kern table
            if( ! pKern->name1 || ! pKern->name2 )
                continue;

            aPair.first = 0;
            aPair.second = 0;
            // currently we have to find the adobe character names
            // in the already parsed character metrics to find
            // the corresponding UCS2 code which is a bit dangerous
            // since the character names are not required
            // in the metric descriptions
            pChar = pInfo->cmi;
            for( int j = 0;
                 j < pInfo->numOfChars && ( aPair.first == 0 || aPair.second == 0 );
                 j++, pChar++ )
            {
                if( pChar->code != -1 )
                {
                    if( ! strcmp( pKern->name1, pChar->name ? pChar->name : "" ) )
                        aPair.first = pUnicodes[ j ];
                    if( ! strcmp( pKern->name2, pChar->name ? pChar->name : "" ) )
                        aPair.second = pUnicodes[ j ];
                }
            }
            if( aPair.first && aPair.second )
            {
                aPair.kern_x = pKern->xamt;
                aPair.kern_y = pKern->yamt;
                m_pMetrics->m_aXKernPairs.push_back( aPair );
            }
        }
        m_pMetrics->m_bKernPairsQueried = true;
    }

    freeFontInfo( pInfo );
    return true;
}

// -------------------------------------------------------------------------

OString PrintFontManager::s_aEmptyOString;

/*
 *  one instance only
 */
PrintFontManager& PrintFontManager::get()
{
    static PrintFontManager* theManager = NULL;
    if( ! theManager )
    {
        theManager = new PrintFontManager();
        theManager->initialize();
    }
    return *theManager;
}

// -------------------------------------------------------------------------

/*
 *  the PrintFontManager
 */

PrintFontManager::PrintFontManager() :
        m_nNextFontID( 1 ),
        m_pAtoms( new MultiAtomProvider() ),
        m_nNextDirAtom( 1 ),
        m_pFontCache( NULL ),
        m_bFontconfigSuccess( false )
{
    for( unsigned int i = 0; i < sizeof( aAdobeCodes )/sizeof( aAdobeCodes[0] ); i++ )
    {
        m_aUnicodeToAdobename.insert( ::std::hash_multimap< sal_Unicode, ::rtl::OString >::value_type( aAdobeCodes[i].aUnicode, aAdobeCodes[i].pAdobename ) );
        m_aAdobenameToUnicode.insert( ::std::hash_multimap< ::rtl::OString, sal_Unicode, ::rtl::OStringHash >::value_type( aAdobeCodes[i].pAdobename, aAdobeCodes[i].aUnicode ) );
        if( aAdobeCodes[i].aAdobeStandardCode )
        {
            m_aUnicodeToAdobecode.insert( ::std::hash_multimap< sal_Unicode, sal_uInt8 >::value_type( aAdobeCodes[i].aUnicode, aAdobeCodes[i].aAdobeStandardCode ) );
            m_aAdobecodeToUnicode.insert( ::std::hash_multimap< sal_uInt8, sal_Unicode >::value_type( aAdobeCodes[i].aAdobeStandardCode, aAdobeCodes[i].aUnicode ) );
        }
#if 0
        m_aUnicodeToAdobename[ aAdobeCodes[i].aUnicode ] = aAdobeCodes[i].pAdobename;
        m_aAdobenameToUnicode[ aAdobeCodes[i].pAdobename ] = aAdobeCodes[i].aUnicode;
        if( aAdobeCodes[i].aAdobeStandardCode )
        {
            m_aUnicodeToAdobecode[ aAdobeCodes[i].aUnicode ] = aAdobeCodes[i].aAdobeStandardCode;
            m_aAdobecodeToUnicode[ aAdobeCodes[i].aAdobeStandardCode ] = aAdobeCodes[i].aUnicode;
        }
#endif
    }
}

// -------------------------------------------------------------------------

PrintFontManager::~PrintFontManager()
{
    deinitFontconfig();
    for( ::std::hash_map< fontID, PrintFont* >::const_iterator it = m_aFonts.begin(); it != m_aFonts.end(); ++it )
        delete (*it).second;
    delete m_pAtoms;
    if( m_pFontCache )
        delete m_pFontCache;
}

// -------------------------------------------------------------------------

const OString& PrintFontManager::getDirectory( int nAtom ) const
{
    ::std::hash_map< int, OString >::const_iterator it( m_aAtomToDir.find( nAtom ) );
    return it != m_aAtomToDir.end() ? it->second : s_aEmptyOString;
}

// -------------------------------------------------------------------------

int PrintFontManager::getDirectoryAtom( const OString& rDirectory, bool bCreate )
{
    int nAtom = 0;
    ::std::hash_map< OString, int, OStringHash >::const_iterator it
          ( m_aDirToAtom.find( rDirectory ) );
    if( it != m_aDirToAtom.end() )
        nAtom = it->second;
    else if( bCreate )
    {
        nAtom = m_nNextDirAtom++;
        m_aDirToAtom[ rDirectory ] = nAtom;
        m_aAtomToDir[ nAtom ] = rDirectory;
    }
    return nAtom;
}

// -------------------------------------------------------------------------

int PrintFontManager::addFontFile( const ::rtl::OString& rFileName, int /*nFaceNum*/ )
{
    rtl_TextEncoding aEncoding = osl_getThreadTextEncoding();
    INetURLObject aPath( OStringToOUString( rFileName, aEncoding ), INET_PROT_FILE, INetURLObject::ENCODE_ALL );
    OString aName( OUStringToOString( aPath.GetName(), aEncoding ) );
    OString aDir( OUStringToOString( aPath.GetPath(), aEncoding ) );

    int nDirID = getDirectoryAtom( aDir, true );
    fontID nFontId = findFontFileID( nDirID, aName );
    if( !nFontId )
    {
        ::std::list< PrintFont* > aNewFonts;
        if( analyzeFontFile( nDirID, aName, ::std::list<OString>(), aNewFonts ) )
        {
            for( ::std::list< PrintFont* >::iterator it = aNewFonts.begin();
                 it != aNewFonts.end(); ++it )
            {
                m_aFonts[ nFontId = m_nNextFontID++ ] = *it;
                m_aFontFileToFontID[ aName ].insert( nFontId );
                m_pFontCache->updateFontCacheEntry( *it, true );
            }
        }
    }
    return nFontId;
}

// -------------------------------------------------------------------------

bool PrintFontManager::analyzeFontFile( int nDirID, const OString& rFontFile, const ::std::list<OString>& rXLFDs, ::std::list< PrintFontManager::PrintFont* >& rNewFonts ) const
{
    rNewFonts.clear();

    OString aDir( getDirectory( nDirID ) );

    OString aFullPath( aDir );
    aFullPath += "/";
    aFullPath += rFontFile;

    // #i1872# reject unreadable files
    if( access( aFullPath.getStr(), R_OK ) )
        return false;

    ByteString aExt( rFontFile.copy( rFontFile.lastIndexOf( '.' )+1 ) );
    if( aExt.EqualsIgnoreCaseAscii( "pfb" ) || aExt.EqualsIgnoreCaseAscii( "pfa" ) )
    {
        // check for corresponding afm metric
        // first look for an adjacent file
        static const char* pSuffix[] = { ".afm", ".AFM" };

        for( unsigned int i = 0; i < sizeof(pSuffix)/sizeof(pSuffix[0]); i++ )
        {
            ByteString aName( rFontFile );
            aName.Erase( aName.Len()-4 );
            aName.Append( pSuffix[i] );

            ByteString aFilePath( aDir );
            aFilePath.Append( '/' );
            aFilePath.Append( aName );

            ByteString aAfmFile;
            if( access( aFilePath.GetBuffer(), R_OK ) )
            {
                // try in subdirectory afm instead
                aFilePath = aDir;
                aFilePath.Append( "/afm/" );
                aFilePath.Append( aName );

                if( ! access( aFilePath.GetBuffer(), R_OK ) )
                {
                    aAfmFile = "afm/";
                    aAfmFile += aName;
                }
            }
            else
                aAfmFile = aName;

            if( aAfmFile.Len() )
            {
                Type1FontFile* pFont = new Type1FontFile();
                pFont->m_nDirectory     = nDirID;

                pFont->m_aFontFile      = rFontFile;
                pFont->m_aMetricFile    = aAfmFile;

                if( ! pFont->readAfmMetrics( getAfmFile( pFont ), m_pAtoms, false, true ) )
                {
                    delete pFont;
                    pFont = NULL;
                }
                if( pFont && rXLFDs.size() )
                    getFontAttributesFromXLFD( pFont, rXLFDs );
                if( pFont )
                    rNewFonts.push_back( pFont );
                break;
            }
        }
    }
    else if( aExt.EqualsIgnoreCaseAscii( "afm" ) )
    {
        ByteString aFilePath( aDir );
        aFilePath.Append( '/' );
        aFilePath.Append( ByteString( rFontFile ) );
        BuiltinFont* pFont = new BuiltinFont();
        pFont->m_nDirectory     = nDirID;
        pFont->m_aMetricFile    = rFontFile;
        if( pFont->readAfmMetrics( aFilePath, m_pAtoms, false, true ) )
            rNewFonts.push_back( pFont );
        else
            delete pFont;
    }
    else if( aExt.EqualsIgnoreCaseAscii( "ttf" )
         ||  aExt.EqualsIgnoreCaseAscii( "tte" )   // #i33947# for Gaiji support
         ||  aExt.EqualsIgnoreCaseAscii( "otf" ) ) // #112957# allow GLYF-OTF
    {
        TrueTypeFontFile* pFont     = new TrueTypeFontFile();
        pFont->m_nDirectory         = nDirID;
        pFont->m_aFontFile          = rFontFile;
        pFont->m_nCollectionEntry   = -1;

        if( rXLFDs.size() )
            getFontAttributesFromXLFD( pFont, rXLFDs );
        // need to read the font anyway to get aliases inside the font file
        if( ! analyzeTrueTypeFile( pFont ) )
        {
            delete pFont;
            pFont = NULL;
        }
        else
            rNewFonts.push_back( pFont );
    }
    else if( aExt.EqualsIgnoreCaseAscii( "ttc" ) )
    {
        // get number of ttc entries
        int nLength = CountTTCFonts( aFullPath.getStr() );
        if( nLength )
        {
#if OSL_DEBUG_LEVEL > 1
            fprintf( stderr, "%s contains %d fonts\n", aFullPath.getStr(), nLength );
#endif
            for( int i = 0; i < nLength; i++ )
            {
                TrueTypeFontFile* pFont     = new TrueTypeFontFile();
                pFont->m_nDirectory         = nDirID;
                pFont->m_aFontFile          = rFontFile;
                pFont->m_nCollectionEntry   = i;
                if( nLength == 1 )
                    getFontAttributesFromXLFD( pFont, rXLFDs );
                if( ! analyzeTrueTypeFile( pFont ) )
                {
                    delete pFont;
                    pFont = NULL;
                }
                else
                    rNewFonts.push_back( pFont );
            }
        }
#if OSL_DEBUG_LEVEL > 1
        else
            fprintf( stderr, "CountTTCFonts( \"%s/%s\" ) failed\n", getDirectory(nDirID).getStr(), rFontFile.getStr() );
#endif
    }
    return ! rNewFonts.empty();
}

// -------------------------------------------------------------------------

fontID PrintFontManager::findFontBuiltinID( int nPSNameAtom ) const
{
    fontID nID = 0;
    ::std::hash_map< fontID, PrintFont* >::const_iterator it;
    for( it = m_aFonts.begin(); nID == 0 && it != m_aFonts.end(); ++it )
    {
        if( it->second->m_eType == fonttype::Builtin &&
            it->second->m_nPSName == nPSNameAtom )
            nID = it->first;
    }
    return nID;
}

// -------------------------------------------------------------------------

fontID PrintFontManager::findFontFileID( int nDirID, const OString& rFontFile ) const
{
    fontID nID = 0;

    ::std::hash_map< OString, ::std::set< fontID >, OStringHash >::const_iterator set_it = m_aFontFileToFontID.find( rFontFile );
    if( set_it != m_aFontFileToFontID.end() )
    {
        for( ::std::set< fontID >::const_iterator font_it = set_it->second.begin(); font_it != set_it->second.end() && ! nID; ++font_it )
        {
            ::std::hash_map< fontID, PrintFont* >::const_iterator it = m_aFonts.find( *font_it );
            if( it != m_aFonts.end() )
            {
                switch( it->second->m_eType )
                {
                    case fonttype::Type1:
                    {
                        Type1FontFile* const pFont = static_cast< Type1FontFile* const >((*it).second);
                        if( pFont->m_nDirectory == nDirID &&
                            pFont->m_aFontFile == rFontFile )
                            nID = it->first;
                    }
                    break;
                    case fonttype::TrueType:
                    {
                        TrueTypeFontFile* const pFont = static_cast< TrueTypeFontFile* const >((*it).second);
                        if( pFont->m_nDirectory == nDirID &&
                            pFont->m_aFontFile == rFontFile )
                            nID = it->first;
                    }
                    break;
                    case fonttype::Builtin:
                        if( static_cast<const BuiltinFont*>((*it).second)->m_nDirectory == nDirID &&
                            static_cast<const BuiltinFont*>((*it).second)->m_aMetricFile == rFontFile )
                            nID = it->first;
                        break;
                    default:
                        break;
                }
            }
        }
    }
    return nID;
}

// -------------------------------------------------------------------------

bool PrintFontManager::parseXLFD( const OString& rXLFD, XLFDEntry& rEntry )
{
    sal_Int32 nIndex = 0;
    OString aFoundry        = WhitespaceToSpace( rXLFD.getToken( 1, '-', nIndex ) );
    if( nIndex < 0 ) return false;
    OString aFamilyXLFD     = WhitespaceToSpace( rXLFD.getToken( 0, '-', nIndex ) );
    if( nIndex < 0 ) return false;
    OString aWeight         = rXLFD.getToken( 0, '-', nIndex ).toAsciiLowerCase();
    if( nIndex < 0 ) return false;
    OString aSlant          = rXLFD.getToken( 0, '-', nIndex ).toAsciiLowerCase();
    if( nIndex < 0 ) return false;
    OString aWidth          = rXLFD.getToken( 0, '-', nIndex ).toAsciiLowerCase();
    if( nIndex < 0 ) return false;
    OString aAddStyle       = rXLFD.getToken( 0, '-', nIndex ).toAsciiLowerCase();
    if( nIndex < 0 ) return false;
    OString aPitch          = rXLFD.getToken( 4, '-', nIndex ).toAsciiLowerCase();
    if( nIndex < 0 ) return false;
    OString aRegEnc         = WhitespaceToSpace( rXLFD.getToken( 1, '-', nIndex ).toAsciiLowerCase() );
    if( nIndex < 0 ) return false;
    OString aEnc            = WhitespaceToSpace( rXLFD.getToken( 0, '-', nIndex ).toAsciiLowerCase() );

    // capitalize words
    sal_Int32 nFamIndex = 0;
    OStringBuffer aFamilyName;
    while( nFamIndex >= 0 )
    {
        OString aToken = aFamilyXLFD.getToken( 0, ' ', nFamIndex );
        sal_Char aFirst = aToken.toChar();
        if( aFirst >= 'a' && aFirst <= 'z' )
            aFirst = aFirst - 'a' + 'A';
        OStringBuffer aNewToken( aToken.getLength() );
        aNewToken.append( aToken );
        aNewToken.setCharAt( 0, aFirst );
        if( aFamilyName.getLength() > 0 )
            aFamilyName.append( ' ' );
        aFamilyName.append( aNewToken.makeStringAndClear() );
    }

    rEntry.aFoundry     = aFoundry;
    rEntry.aFamily      = aFamilyName.makeStringAndClear();
    rEntry.aAddStyle    = aAddStyle;
    // evaluate weight
    rEntry.eWeight = parseWeight( aWeight );
    // evaluate slant
    rEntry.eItalic = parseItalic( aSlant );
    // evaluate width
    rEntry.eWidth = parseWidth( aWidth );

    // evaluate pitch
    if( aPitch.toChar() == 'c' || aPitch.toChar() == 'm' )
        rEntry.ePitch = pitch::Fixed;
    else
        rEntry.ePitch = pitch::Variable;

    OString aToken = aEnc.toAsciiLowerCase();
    // get encoding
    if( aAddStyle.indexOf( "symbol" ) != -1 )
        rEntry.aEncoding = RTL_TEXTENCODING_SYMBOL;
    else
    {
        if( aToken.equals( "symbol" ) )
            rEntry.aEncoding = RTL_TEXTENCODING_SYMBOL;
        else
        {
            OStringBuffer aCharset( aRegEnc.getLength() + aEnc.getLength() + 1 );
            aCharset.append( aRegEnc );
            aCharset.append( '-' );
            aCharset.append( aEnc );
            rEntry.aEncoding = rtl_getTextEncodingFromUnixCharset( aCharset.getStr() );
        }
    }

    // set correct mask flags
    rEntry.nMask = 0;
    if( rEntry.aFoundry != "*" )        rEntry.nMask |= XLFDEntry::MaskFoundry;
    if( rEntry.aFamily != "*" )         rEntry.nMask |= XLFDEntry::MaskFamily;
    if( rEntry.aAddStyle != "*" )       rEntry.nMask |= XLFDEntry::MaskAddStyle;
    if( aWeight != "*" )                rEntry.nMask |= XLFDEntry::MaskWeight;
    if( aSlant != "*" )                 rEntry.nMask |= XLFDEntry::MaskItalic;
    if( aWidth != "*" )                 rEntry.nMask |= XLFDEntry::MaskWidth;
    if( aPitch != "*" )                 rEntry.nMask |= XLFDEntry::MaskPitch;
    if( aRegEnc != "*" && aEnc != "*" ) rEntry.nMask |= XLFDEntry::MaskEncoding;

    return true;
}

// -------------------------------------------------------------------------

void PrintFontManager::parseXLFD_appendAliases( const std::list< OString >& rXLFDs, std::list< XLFDEntry >& rEntries ) const
{
    for( std::list< OString >::const_iterator it = rXLFDs.begin(); it != rXLFDs.end(); ++it )
    {
        XLFDEntry aEntry;
        if( ! parseXLFD(*it, aEntry) )
            continue;
        rEntries.push_back( aEntry );
        std::map< XLFDEntry, std::list< XLFDEntry > >::const_iterator alias_it =
            m_aXLFD_Aliases.find( aEntry );
        if( alias_it != m_aXLFD_Aliases.end() )
        {
            rEntries.insert( rEntries.end(), alias_it->second.begin(), alias_it->second.end() );
        }
    }
}

// -------------------------------------------------------------------------

void PrintFontManager::getFontAttributesFromXLFD( PrintFont* pFont, const std::list< OString >& rXLFDs ) const
{
    bool bFamilyName = false;

    std::list< XLFDEntry > aXLFDs;

    parseXLFD_appendAliases( rXLFDs, aXLFDs );

    for( std::list< XLFDEntry >::const_iterator it = aXLFDs.begin();
         it != aXLFDs.end(); ++it )
    {
        // set family name or alias
        int nFam =
            m_pAtoms->getAtom( ATOM_FAMILYNAME,
                               OStringToOUString( it->aFamily, it->aAddStyle.indexOf( "utf8" ) != -1 ? RTL_TEXTENCODING_UTF8 : RTL_TEXTENCODING_ISO_8859_1 ),
                               sal_True );
        if( ! bFamilyName )
        {
            bFamilyName = true;
            pFont->m_nFamilyName = nFam;
            switch( pFont->m_eType )
            {
                case fonttype::Type1:
                    static_cast<Type1FontFile*>(pFont)->m_aXLFD = rXLFDs.front();
                    break;
                case fonttype::TrueType:
                    static_cast<TrueTypeFontFile*>(pFont)->m_aXLFD = rXLFDs.front();
                    break;
                default:
                    break;
            }
        }
        else
        {
            // make sure that aliases are unique
            if( nFam != pFont->m_nFamilyName )
            {
                std::list< int >::const_iterator al_it;
                for( al_it = pFont->m_aAliases.begin(); al_it != pFont->m_aAliases.end() && *al_it != nFam; ++al_it )
                    ;
                if( al_it == pFont->m_aAliases.end() )
                    pFont->m_aAliases.push_back( nFam );

            }
            // for the rest of the attributes there can only be one value;
            // we'll trust the first one
            continue;
        }

        // fill in weight
        pFont->m_eWeight    = it->eWeight;
        // fill in slant
        pFont->m_eItalic    = it->eItalic;
        // fill in width
        pFont->m_eWidth     = it->eWidth;
        // fill in pitch
        pFont->m_ePitch     = it->ePitch;
        // fill in encoding
        pFont->m_aEncoding  = it->aEncoding;
    }

    // handle iso8859-1 as ms1252 to fill the "gap" starting at 0x80
    if( pFont->m_aEncoding == RTL_TEXTENCODING_ISO_8859_1 )
        pFont->m_aEncoding = RTL_TEXTENCODING_MS_1252;
    if( rXLFDs.begin() != rXLFDs.end() )
    {
        switch( pFont->m_eType )
        {
            case fonttype::Type1:
                static_cast<Type1FontFile*>(pFont)->m_aXLFD = rXLFDs.front();
                break;
            case fonttype::TrueType:
                static_cast<TrueTypeFontFile*>(pFont)->m_aXLFD = rXLFDs.front();
                break;
            default: break;
        }
    }
}

// -------------------------------------------------------------------------

OString PrintFontManager::getXLFD( PrintFont* pFont ) const
{
    if( pFont->m_eType == fonttype::Type1 )
    {
        if( static_cast<Type1FontFile*>(pFont)->m_aXLFD.getLength() )
            return static_cast<Type1FontFile*>(pFont)->m_aXLFD;
    }
    if( pFont->m_eType == fonttype::TrueType )
    {
        if( static_cast<TrueTypeFontFile*>(pFont)->m_aXLFD.getLength() )
            return static_cast<TrueTypeFontFile*>(pFont)->m_aXLFD;
    }

    OStringBuffer aXLFD( 128 );

    aXLFD.append( "-misc-" );
    ByteString aFamily( String( m_pAtoms->getString( ATOM_FAMILYNAME, pFont->m_nFamilyName ) ), RTL_TEXTENCODING_UTF8 );
    aFamily.SearchAndReplaceAll( '-',' ' );
    aFamily.SearchAndReplaceAll( '?',' ' );
    aFamily.SearchAndReplaceAll( '*',' ' );
    aXLFD.append( OString( aFamily ) );
    aXLFD.append( '-' );
    switch( pFont->m_eWeight )
    {
        case weight::Thin:          aXLFD.append("thin");break;
        case weight::UltraLight:    aXLFD.append("ultralight");break;
        case weight::Light:         aXLFD.append("light");break;
        case weight::SemiLight:     aXLFD.append("semilight");break;
        case weight::Normal:        aXLFD.append("normal");break;
        case weight::Medium:        aXLFD.append("medium");break;
        case weight::SemiBold:      aXLFD.append("semibold");break;
        case weight::Bold:          aXLFD.append("bold");break;
        case weight::UltraBold:     aXLFD.append("ultrabold");break;
        case weight::Black:         aXLFD.append("black");break;
        default: break;
    }
    aXLFD.append('-');
    switch( pFont->m_eItalic )
    {
        case italic::Upright:       aXLFD.append('r');break;
        case italic::Oblique:       aXLFD.append('o');break;
        case italic::Italic:        aXLFD.append('i');break;
        default: break;
    }
    aXLFD.append('-');
    switch( pFont->m_eWidth )
    {
        case width::UltraCondensed: aXLFD.append("ultracondensed");break;
        case width::ExtraCondensed: aXLFD.append("extracondensed");break;
        case width::Condensed:      aXLFD.append("condensed");break;
        case width::SemiCondensed:  aXLFD.append("semicondensed");break;
        case width::Normal:         aXLFD.append("normal");break;
        case width::SemiExpanded:   aXLFD.append("semiexpanded");break;
        case width::Expanded:       aXLFD.append("expanded");break;
        case width::ExtraExpanded:  aXLFD.append("extraexpanded");break;
        case width::UltraExpanded:  aXLFD.append("ultraexpanded");break;
        default: break;
    }
    aXLFD.append("-utf8-0-0-0-0-");
    aXLFD.append( pFont->m_ePitch == pitch::Fixed ? "m" : "p" );
    aXLFD.append("-0-");
    const char* pEnc = rtl_getBestUnixCharsetFromTextEncoding( pFont->m_aEncoding );
    if( ! pEnc )
    {
        if( pFont->m_aEncoding == RTL_TEXTENCODING_ADOBE_STANDARD )
            pEnc = "adobe-standard";
        else
            pEnc = "iso8859-1";
    }
    aXLFD .append( pEnc );

    return aXLFD.makeStringAndClear();
}

// -------------------------------------------------------------------------

OUString PrintFontManager::convertTrueTypeName( void* pRecord ) const
{
    NameRecord* pNameRecord = (NameRecord*)pRecord;
    OUString aValue;
    if(
       ( pNameRecord->platformID == 3 && ( pNameRecord->encodingID == 0 || pNameRecord->encodingID == 1 ) )  // MS, Unicode
       ||
       ( pNameRecord->platformID == 0 ) // Apple, Unicode
       )
    {
        OUStringBuffer aName( pNameRecord->slen/2 );
        const sal_uInt8* pNameBuffer = pNameRecord->sptr;
        for(int n = 0; n < pNameRecord->slen/2; n++ )
            aName.append( (sal_Unicode)getUInt16BE( pNameBuffer ) );
        aValue = aName.makeStringAndClear();
    }
    else if( pNameRecord->platformID == 3 )
    {
        if( pNameRecord->encodingID >= 2 && pNameRecord->encodingID <= 6 )
        {
            /*
             *  and now for a special kind of madness:
             *  some fonts encode their byte value string as BE uint16
             *  (leading to stray zero bytes in the string)
             *  while others code two bytes as a uint16 and swap to BE
             */
            OStringBuffer aName;
            const sal_uInt8* pNameBuffer = pNameRecord->sptr;
            for(int n = 0; n < pNameRecord->slen/2; n++ )
            {
                sal_Unicode aCode = (sal_Unicode)getUInt16BE( pNameBuffer );
                sal_Char aChar = aCode >> 8;
                if( aChar )
                    aName.append( aChar );
                aChar = aCode & 0x00ff;
                if( aChar )
                    aName.append( aChar );
            }
            switch( pNameRecord->encodingID )
            {
                case 2:
                    aValue = OStringToOUString( aName.makeStringAndClear(), RTL_TEXTENCODING_MS_932 );
                    break;
                case 3:
                    aValue = OStringToOUString( aName.makeStringAndClear(), RTL_TEXTENCODING_MS_936 );
                    break;
                case 4:
                    aValue = OStringToOUString( aName.makeStringAndClear(), RTL_TEXTENCODING_MS_950 );
                    break;
                case 5:
                    aValue = OStringToOUString( aName.makeStringAndClear(), RTL_TEXTENCODING_MS_949 );
                    break;
                case 6:
                    aValue = OStringToOUString( aName.makeStringAndClear(), RTL_TEXTENCODING_MS_1361 );
                    break;
            }
        }
    }
    return aValue;
}

// -------------------------------------------------------------------------

void PrintFontManager::analyzeTrueTypeFamilyName( void* pTTFont, ::std::list< OUString >& rNames ) const
{
    OUString aFamily;

    rNames.clear();
    ::std::set< OUString > aSet;

    NameRecord* pNameRecords = NULL;
    int nNameRecords = GetTTNameRecords( (TrueTypeFont*)pTTFont, &pNameRecords );
    if( nNameRecords && pNameRecords )
    {
        LanguageType aLang = MsLangId::getSystemLanguage();
        int nLastMatch = -1;
        for( int i = 0; i < nNameRecords; i++ )
        {
            if( pNameRecords[i].nameID != 1 || pNameRecords[i].sptr == NULL )
                continue;
            int nMatch = -1;
            if( pNameRecords[i].platformID == 0 ) // Unicode
                nMatch = 4000;
            else if( pNameRecords[i].platformID == 3 )
            {
                // this bases on the LanguageType actually being a Win LCID
                if( pNameRecords[i].languageID == aLang )
                    nMatch = 8000;
                else if( pNameRecords[i].languageID == LANGUAGE_ENGLISH_US )
                    nMatch = 2000;
                else if( pNameRecords[i].languageID == LANGUAGE_ENGLISH ||
                         pNameRecords[i].languageID == LANGUAGE_ENGLISH_UK )
                    nMatch = 1500;
                else
                    nMatch = 1000;
            }
            OUString aName = convertTrueTypeName( pNameRecords + i );
            aSet.insert( aName );
            if( nMatch > nLastMatch )
            {
                nLastMatch = nMatch;
                aFamily = aName;
            }
        }
        DisposeNameRecords( pNameRecords, nNameRecords );
    }
    if( aFamily.getLength() )
    {
        rNames.push_front( aFamily );
        for( ::std::set< OUString >::const_iterator it = aSet.begin(); it != aSet.end(); ++it )
            if( *it != aFamily )
                rNames.push_back( *it );
    }
    return;
}

// -------------------------------------------------------------------------

bool PrintFontManager::analyzeTrueTypeFile( PrintFont* pFont ) const
{
    bool bSuccess = false;
    rtl_TextEncoding aEncoding = osl_getThreadTextEncoding();
    ByteString aFile = getFontFile( pFont );
    TrueTypeFont* pTTFont = NULL;

    TrueTypeFontFile* pTTFontFile = static_cast< TrueTypeFontFile* >(pFont);
    if( OpenTTFontFile( aFile.GetBuffer(), pTTFontFile->m_nCollectionEntry < 0 ? 0 : pTTFontFile->m_nCollectionEntry, &pTTFont ) == SF_OK )
    {
        TTGlobalFontInfo aInfo;
        GetTTGlobalFontInfo( pTTFont, & aInfo );

        ::std::list< OUString > aNames;
        analyzeTrueTypeFamilyName( pTTFont, aNames );

        // set family name from XLFD if possible
        if( ! pFont->m_nFamilyName )
        {
            if( aNames.begin() != aNames.end() )
            {
                pFont->m_nFamilyName = m_pAtoms->getAtom( ATOM_FAMILYNAME, aNames.front(), sal_True );
                aNames.pop_front();
            }
            else
            {
                 sal_Int32   dotIndex;

                 // poor font does not have a family name
                 // name it to file name minus the extension
                 dotIndex = pTTFontFile->m_aFontFile.lastIndexOf( '.' );
                 if ( dotIndex == -1 )
                     dotIndex = pTTFontFile->m_aFontFile.getLength();

                 pFont->m_nFamilyName = m_pAtoms->getAtom( ATOM_FAMILYNAME, OStringToOUString( pTTFontFile->m_aFontFile.copy( 0, dotIndex ), aEncoding ), sal_True );
            }
        }
        for( ::std::list< OUString >::iterator it = aNames.begin(); it != aNames.end(); ++it )
        {
            if( it->getLength() )
            {
                int nAlias = m_pAtoms->getAtom( ATOM_FAMILYNAME, *it, sal_True );
                if( nAlias != pFont->m_nFamilyName )
                {
                    std::list< int >::const_iterator al_it;
                    for( al_it = pFont->m_aAliases.begin(); al_it != pFont->m_aAliases.end() && *al_it != nAlias; ++al_it )
                        ;
                    if( al_it == pFont->m_aAliases.end() )
                        pFont->m_aAliases.push_back( nAlias );
                }
            }
        }

        if( aInfo.usubfamily )
            pFont->m_aStyleName = OUString( aInfo.usubfamily );

        pFont->m_nPSName = m_pAtoms->getAtom( ATOM_PSNAME, String( ByteString( aInfo.psname ), aEncoding ), sal_True );
        switch( aInfo.weight )
        {
            case FW_THIN:           pFont->m_eWeight = weight::Thin; break;
            case FW_EXTRALIGHT: pFont->m_eWeight = weight::UltraLight; break;
            case FW_LIGHT:          pFont->m_eWeight = weight::Light; break;
            case FW_MEDIUM:     pFont->m_eWeight = weight::Medium; break;
            case FW_SEMIBOLD:       pFont->m_eWeight = weight::SemiBold; break;
            case FW_BOLD:           pFont->m_eWeight = weight::Bold; break;
            case FW_EXTRABOLD:      pFont->m_eWeight = weight::UltraBold; break;
            case FW_BLACK:          pFont->m_eWeight = weight::Black; break;

            case FW_NORMAL:
            default:        pFont->m_eWeight = weight::Normal; break;
        }

        switch( aInfo.width )
        {
            case FWIDTH_ULTRA_CONDENSED:    pFont->m_eWidth = width::UltraCondensed; break;
            case FWIDTH_EXTRA_CONDENSED:    pFont->m_eWidth = width::ExtraCondensed; break;
            case FWIDTH_CONDENSED:          pFont->m_eWidth = width::Condensed; break;
            case FWIDTH_SEMI_CONDENSED: pFont->m_eWidth = width::SemiCondensed; break;
            case FWIDTH_SEMI_EXPANDED:      pFont->m_eWidth = width::SemiExpanded; break;
            case FWIDTH_EXPANDED:           pFont->m_eWidth = width::Expanded; break;
            case FWIDTH_EXTRA_EXPANDED: pFont->m_eWidth = width::ExtraExpanded; break;
            case FWIDTH_ULTRA_EXPANDED: pFont->m_eWidth = width::UltraExpanded; break;

            case FWIDTH_NORMAL:
            default:                        pFont->m_eWidth = width::Normal; break;
        }

        pFont->m_ePitch = aInfo.pitch ? pitch::Fixed : pitch::Variable;
        pFont->m_eItalic = aInfo.italicAngle == 0 ? italic::Upright : ( aInfo.italicAngle < 0 ? italic::Italic : italic::Oblique );
        // #104264# there are fonts that set italic angle 0 although they are
        // italic; use macstyle bit here
        if( aInfo.italicAngle == 0 && (aInfo.macStyle & 2) )
            pFont->m_eItalic = italic::Italic;

        pFont->m_aEncoding = aInfo.symbolEncoded ? RTL_TEXTENCODING_SYMBOL : RTL_TEXTENCODING_UCS2;

        pFont->m_aGlobalMetricY.width = pFont->m_aGlobalMetricX.width = aInfo.xMax - aInfo.xMin;
        pFont->m_aGlobalMetricY.height = pFont->m_aGlobalMetricX.height = aInfo.yMax - aInfo.yMin;

        if( aInfo.winAscent && aInfo.winDescent )
        {
            pFont->m_nAscend    = aInfo.winAscent;
            pFont->m_nDescend   = aInfo.winDescent;
            pFont->m_nLeading   = pFont->m_nAscend + pFont->m_nDescend - 1000;
        }
        else if( aInfo.typoAscender && aInfo.typoDescender )
        {
            pFont->m_nLeading   = aInfo.typoLineGap;
            pFont->m_nAscend    = aInfo.typoAscender;
            pFont->m_nDescend   = -aInfo.typoDescender;
        }
        else
        {
            pFont->m_nLeading   = aInfo.linegap;
            pFont->m_nAscend    = aInfo.ascender;
            pFont->m_nDescend   = -aInfo.descender;
        }

        // last try: font bounding box
        if( pFont->m_nAscend == 0 )
            pFont->m_nAscend = aInfo.yMax;
        if( pFont->m_nDescend == 0 )
            pFont->m_nDescend = -aInfo.yMin;
        if( pFont->m_nLeading == 0 )
            pFont->m_nLeading = 15 * (pFont->m_nAscend+pFont->m_nDescend) / 100;

        if( pFont->m_nAscend )
            pFont->m_aGlobalMetricX.height = pFont->m_aGlobalMetricY.height = pFont->m_nAscend + pFont->m_nDescend;

        // get bounding box
        pFont->m_nXMin = aInfo.xMin;
        pFont->m_nYMin = aInfo.yMin;
        pFont->m_nXMax = aInfo.xMax;
        pFont->m_nYMax = aInfo.yMax;

        // get type flags
        pTTFontFile->m_nTypeFlags = (unsigned int)aInfo.typeFlags;

        // get vertical substitutions flag
        pFont->m_bHaveVerticalSubstitutedGlyphs = DoesVerticalSubstitution( pTTFont, 1 );

        CloseTTFont( pTTFont );
        bSuccess = true;
    }
#if OSL_DEBUG_LEVEL > 1
    else
        fprintf( stderr, "could not OpenTTFont \"%s\"\n", aFile.GetBuffer() );
#endif

    return bSuccess;
}

// -------------------------------------------------------------------------

void PrintFontManager::getServerDirectories()
{
#ifdef LINUX
    /*
     *  chkfontpath exists on some (RH derived) Linux distributions
     */
    static const char* pCommands[] = {
        "/usr/sbin/chkfontpath 2>/dev/null", "chkfontpath 2>/dev/null"
    };
    ::std::list< ByteString > aLines;

    for( unsigned int i = 0; i < sizeof(pCommands)/sizeof(pCommands[0]); i++ )
    {
        FILE* pPipe = popen( pCommands[i], "r" );
        aLines.clear();
        if( pPipe )
        {
            char line[1024];
            char* pSearch;
            while( fgets( line, sizeof(line), pPipe ) )
            {
                int nLen = strlen( line );
                if( line[nLen-1] == '\n' )
                    line[nLen-1] = 0;
                pSearch = strstr( line, ": " );
                if( pSearch )
                    aLines.push_back( pSearch+2 );
            }
            if( ! pclose( pPipe ) )
                break;
        }
    }

    for( ::std::list< ByteString >::iterator it = aLines.begin(); it != aLines.end(); ++it )
    {
        if( ! access( it->GetBuffer(), F_OK ) )
        {
            m_aFontDirectories.push_back( *it );
#if OSL_DEBUG_LEVEL > 1
            fprintf( stderr, "adding fs dir %s\n", it->GetBuffer() );
#endif
        }
    }
#endif
}

void PrintFontManager::initFontsAlias()
{
    m_aXLFD_Aliases.clear();
    rtl_TextEncoding aEnc = osl_getThreadTextEncoding();
    for( std::list< OString >::const_iterator dir_it = m_aFontDirectories.begin();
         dir_it != m_aFontDirectories.end(); ++dir_it )
    {
        OStringBuffer aDirName(512);
        aDirName.append( *dir_it );
        aDirName.append( "/fonts.alias" );
        SvFileStream aStream( OStringToOUString( aDirName.makeStringAndClear(), aEnc ), STREAM_READ );
        if( ! aStream.IsOpen() )
            continue;

        do
        {
            ByteString aLine;
            aStream.ReadLine( aLine );

            // get the alias and the pattern it gets translated to
            ByteString aAlias   = GetCommandLineToken( 0, aLine );
            ByteString aMap     = GetCommandLineToken( 1, aLine );

            // remove eventual quotes
            aAlias.EraseLeadingChars( '"' );
            aAlias.EraseTrailingChars( '"' );
            aMap.EraseLeadingChars( '"' );
            aMap.EraseTrailingChars( '"' );

            XLFDEntry aAliasEntry, aMapEntry;
            parseXLFD( aAlias, aAliasEntry );
            parseXLFD( aMap, aMapEntry );

            if( aAliasEntry.nMask && aMapEntry.nMask )
                m_aXLFD_Aliases[ aMapEntry ].push_back( aAliasEntry );
        } while( ! aStream.IsEof() );
    }
}

void PrintFontManager::initialize( void* pInitDisplay )
{
    #ifdef CALLGRIND_COMPILE
    CALLGRIND_TOGGLE_COLLECT();
    CALLGRIND_ZERO_STATS();
    #endif

    long aDirEntBuffer[ (sizeof(struct dirent)+_PC_NAME_MAX)+1 ];

    if( ! m_pFontCache )
    {
#if OSL_DEBUG_LEVEL > 1
        fprintf( stderr, "creating font cache ... " );
        clock_t aStart;
        struct tms tms;
        aStart = times( &tms );
#endif
        m_pFontCache = new FontCache();
#if OSL_DEBUG_LEVEL > 1
        clock_t aStop = times( &tms );
        fprintf( stderr, "done in %lf s\n", (double)(aStop - aStart)/(double)sysconf( _SC_CLK_TCK ) );
#endif
    }

    // initialize may be called twice in the future
    {
        for( ::std::hash_map< fontID, PrintFont* >::const_iterator it = m_aFonts.begin(); it != m_aFonts.end(); ++it )
            delete (*it).second;
        m_nNextFontID = 1;
        m_aFonts.clear();
        m_aFontDirectories.clear();
        m_aPrivateFontDirectories.clear();
        m_aOverrideFonts.clear();
    }

#if OSL_DEBUG_LEVEL > 1
    clock_t aStart;
    clock_t aStep1;
    clock_t aStep2;
    clock_t aStep3;
    int nBuiltinFonts = 0;
    int nCached = 0;

    struct tms tms;

    aStart = times( &tms );
#endif

    // first try fontconfig
    m_bFontconfigSuccess = initFontconfig();

    // part one - look for downloadable fonts
    rtl_TextEncoding aEncoding = osl_getThreadTextEncoding();
    const ::rtl::OUString &rSalPrivatePath = psp::getFontPath();

    // search for the fonts in SAL_PRIVATE_FONTPATH first; those are
    // the TrueType fonts installed with the office
    if( rSalPrivatePath.getLength() )
    {
        OString aPath = rtl::OUStringToOString( rSalPrivatePath, aEncoding );
        sal_Int32 nIndex = 0;
        do
        {
            OString aToken = aPath.getToken( 0, ';', nIndex );
            normPath( aToken );
        addFontconfigDir( aToken );
        m_aFontDirectories.push_back( aToken );
            m_aPrivateFontDirectories.push_back( getDirectoryAtom( aToken, true ) );
        } while( nIndex >= 0 );
    }

    // now that all global and local font dirs are known to fontconfig
    // check that there are fonts actually managed by fontconfig
    if( m_bFontconfigSuccess )
        m_bFontconfigSuccess = (countFontconfigFonts() > 0);

    // don't search through many directories fontconfig already told us about
    if( ! m_bFontconfigSuccess )
    {
        Display *pDisplay = (Display*)pInitDisplay;

        if( ! pDisplay )
        {
            int nParams = osl_getCommandArgCount();
            OUString aParam;
            bool bIsHeadless = false;
            for( int i = 0; i < nParams; i++ )
            {
                osl_getCommandArg( i, &aParam.pData );
                if( aParam.equalsAscii( "-headless" ) )
                {
                    bIsHeadless = true;
                    break;
                }
            }
            if( ! bIsHeadless )
                pDisplay = XOpenDisplay( NULL );
        }

        if( pDisplay )
        {
            // get font paths to look for fonts
            int nPaths = 0, i;
            char** pPaths = XGetFontPath( pDisplay, &nPaths );

            bool bServerDirs = false;
            for( i = 0; i < nPaths; i++ )
            {
                OString aPath( pPaths[i] );
                sal_Int32 nPos = 0;
                if( ! bServerDirs
                    && ( nPos = aPath.indexOf( ':' ) ) > 0
                    && ( !aPath.copy(nPos).equals( ":unscaled" ) ) )
                {
                    bServerDirs = true;
                    getServerDirectories();
                }
                else
                {
                    normPath( aPath );
                    m_aFontDirectories.push_back( aPath );
                }
            }

            if( nPaths )
                XFreeFontPath( pPaths );

            if( ! pInitDisplay )
                XCloseDisplay( pDisplay );
        }

        // insert some standard directories
        m_aFontDirectories.push_back( "/usr/openwin/lib/X11/fonts/TrueType" );
        m_aFontDirectories.push_back( "/usr/openwin/lib/X11/fonts/Type1" );
        m_aFontDirectories.push_back( "/usr/openwin/lib/X11/fonts/Type1/sun" );
        m_aFontDirectories.push_back( "/usr/X11R6/lib/X11/fonts/truetype" );
        m_aFontDirectories.push_back( "/usr/X11R6/lib/X11/fonts/Type1" );

#ifdef SOLARIS
        /* cde specials, from /usr/dt/bin/Xsession: here are the good fonts,
           the OWfontpath file may contain as well multiple lines as a comma
           separated list of fonts in each line. to make it even more weird
           environment variables are allowed as well */

        const char* lang = getenv("LANG");
        if ( lang != NULL )
        {
            String aOpenWinDir( String::CreateFromAscii( "/usr/openwin/lib/locale/" ) );
            aOpenWinDir.AppendAscii( lang );
            aOpenWinDir.AppendAscii( "/OWfontpath" );

            SvFileStream aStream( aOpenWinDir, STREAM_READ );

            // TODO: replace environment variables
            while( aStream.IsOpen() && ! aStream.IsEof() )
            {
                ByteString aLine;
                aStream.ReadLine( aLine );
                // need an OString for normpath
                OString aNLine( aLine );
                normPath( aNLine );
                aLine = aNLine;
                // try to avoid bad fonts in some cases
                static bool bAvoid = (strncasecmp( lang, "ar", 2 ) == 0) || (strncasecmp( lang, "he", 2 ) == 0) || strncasecmp( lang, "iw", 2 ) == 0 || (strncasecmp( lang, "hi", 2 ) == 0);
                if( bAvoid && aLine.Search( "iso_8859" ) != STRING_NOTFOUND )
                    continue;
                m_aFontDirectories.push_back( aLine );
            }
        }
#endif /* SOLARIS */
    } // ! m_bFontconfigSuccess

    // fill XLFD aliases from fonts.alias files
    initFontsAlias();

    // search for font files in each path
    std::list< OString >::iterator dir_it;
    // protect against duplicate paths
    std::hash_map< OString, int, OStringHash > visited_dirs;
    for( dir_it = m_aFontDirectories.begin(); dir_it != m_aFontDirectories.end(); ++dir_it )
    {
        OString aPath( *dir_it );
        // see if we were here already
        if( visited_dirs.find( aPath ) != visited_dirs.end() )
            continue;
        visited_dirs[ aPath ] = 1;

        // there may be ":unscaled" directories (see XFree86)
        // it should be safe to ignore them since they should not
        // contain any of our recognizeable fonts

        // ask the font cache whether it handles this directory
        std::list< PrintFont* > aCacheFonts;
        if( m_pFontCache->listDirectory( aPath, aCacheFonts ) )
        {
#if OSL_DEBUG_LEVEL > 1
            fprintf( stderr, "adding cache directory: %s\n", aPath.getStr() );
#endif
            for( ::std::list< PrintFont* >::iterator it = aCacheFonts.begin(); it != aCacheFonts.end(); ++it )
            {
                fontID aFont = m_nNextFontID++;
                m_aFonts[ aFont ] = *it;
                if( (*it)->m_eType == fonttype::Type1 )
                    m_aFontFileToFontID[ static_cast<Type1FontFile*>(*it)->m_aFontFile ].insert( aFont );
                else if( (*it)->m_eType == fonttype::TrueType )
                    m_aFontFileToFontID[ static_cast<TrueTypeFontFile*>(*it)->m_aFontFile ].insert( aFont );
                else if( (*it)->m_eType == fonttype::Builtin )
                    m_aFontFileToFontID[ static_cast<BuiltinFont*>(*it)->m_aMetricFile ].insert( aFont );
#if OSL_DEBUG_LEVEL > 1
                if( (*it)->m_eType == fonttype::Builtin )
                    nBuiltinFonts++;
                nCached++;
#if OSL_DEBUG_LEVEL > 2
                fprintf( stderr, "adding cached font %d: \"%s\" from %s\n", aFont,
                         OUStringToOString( getFontFamily( aFont ), RTL_TEXTENCODING_MS_1252 ).getStr(),
                         getFontFileSysPath( aFont ).getStr() );
#endif
#endif
            }
            if( ! m_pFontCache->scanAdditionalFiles( aPath ) )
                continue;
        }

        DIR* pDIR = opendir( aPath.getStr() );
        struct dirent* pEntry = (struct dirent*)aDirEntBuffer;
        if( pDIR )
        {
            // read fonts.dir if possible
            ::std::hash_map< OString, ::std::list<OString>, OStringHash > aFontsDir;
            int nDirID = getDirectoryAtom( aPath, true );
            // #i38367# no fonts.dir in our own directories anymore
            std::list< int >::const_iterator priv_dir;
            for( priv_dir = m_aPrivateFontDirectories.begin();
                 priv_dir != m_aPrivateFontDirectories.end() && *priv_dir != nDirID;
                 ++priv_dir )
                 ;

            if( priv_dir == m_aPrivateFontDirectories.end() )
            {
                ByteString aGccDummy( aPath );
                String aFontsDirPath( aGccDummy, aEncoding );
                aFontsDirPath.AppendAscii( "/fonts.dir" );
                SvFileStream aStream( aFontsDirPath, STREAM_READ );
                if( aStream.IsOpen() )
                {
                    ByteString aLine;
                    while( ! aStream.IsEof() )
                    {
                        aStream.ReadLine( aLine );
                        ByteString aFileName( GetCommandLineToken( 0, aLine ) );
                        ByteString aXLFD( aLine.Copy( aFileName.Len() ) );
                        if( aFileName.Len() && aXLFD.Len() )
                            aFontsDir[ aFileName ].push_back(aXLFD);
                    }
                }
            }

            int nDirFonts = 0;
            while( ! readdir_r( pDIR, (struct dirent*)aDirEntBuffer, &pEntry ) && pEntry )
            {
                OString aFileName( pEntry->d_name );
                // ignore .afm files here
                if( aFileName.getLength() > 3 &&
                    aFileName.lastIndexOf( ".afm" ) == aFileName.getLength()-4 )
                    continue;

                struct stat aStat;
                ByteString aFilePath( aPath );
                aFilePath.Append( '/' );
                aFilePath.Append( ByteString( aFileName ) );
                if( ! stat( aFilePath.GetBuffer(), &aStat )     &&
                    S_ISREG( aStat.st_mode ) )
                {
                    if( findFontFileID( nDirID, aFileName ) == 0 )
                    {
                        ::std::list<OString> aXLFDs;
                        ::std::hash_map< OString, ::std::list<OString>, OStringHash >::const_iterator it =
                              aFontsDir.find( aFileName );
                        if( it != aFontsDir.end() )
                            aXLFDs = (*it).second;

                        // fill in font attributes from XLFD rather
                        // than reading every file
                        ::std::list< PrintFont* > aNewFonts;
                        if( analyzeFontFile( nDirID, aFileName, aXLFDs, aNewFonts ) )
                        {
                            for( ::std::list< PrintFont* >::iterator font_it = aNewFonts.begin(); font_it != aNewFonts.end(); ++font_it )
                            {
                                fontID aFont = m_nNextFontID++;
                                m_aFonts[ aFont ] = *font_it;
                                m_aFontFileToFontID[ aFileName ].insert( aFont );
                                m_pFontCache->updateFontCacheEntry( *font_it, false );
                                nDirFonts++;
#if OSL_DEBUG_LEVEL > 2
                                fprintf( stderr, "adding font %d: \"%s\" from %s\n", aFont,
                                         OUStringToOString( getFontFamily( aFont ), RTL_TEXTENCODING_MS_1252 ).getStr(),
                                         getFontFileSysPath( aFont ).getStr() );
#endif
                            }
                        }
                    }
                }
            }
            closedir( pDIR );
            m_pFontCache->updateDirTimestamp( nDirID );
            if( ! nDirFonts )
                m_pFontCache->markEmptyDir( nDirID );
        }
    }

#if OSL_DEBUG_LEVEL > 1
    aStep1 = times( &tms );
#endif

    // part two - look for metrics for builtin printer fonts
    std::list< OUString > aMetricDirs;
    psp::getPrinterPathList( aMetricDirs, PRINTER_METRICDIR );

    std::list< OString > aEmptyFontsDir;
    for( std::list< OUString >::const_iterator met_dir_it = aMetricDirs.begin(); met_dir_it != aMetricDirs.end(); ++met_dir_it )
    {
        OString aDir = OUStringToOString( *met_dir_it, aEncoding );

        // ask the font cache whether it handles this directory
        std::list< PrintFont* > aCacheFonts;

        if( m_pFontCache->listDirectory( aDir, aCacheFonts ) )
        {
#if OSL_DEBUG_LEVEL > 1
            fprintf( stderr, "adding cache directory: %s\n", aDir.getStr() );
#endif
            for( ::std::list< PrintFont* >::iterator it = aCacheFonts.begin(); it != aCacheFonts.end(); ++it )
            {
                fontID aFont = m_nNextFontID++;
                m_aFonts[ aFont ] = *it;
                if( (*it)->m_eType == fonttype::Type1 )
                    m_aFontFileToFontID[ static_cast<Type1FontFile*>(*it)->m_aFontFile ].insert( aFont );
                else if( (*it)->m_eType == fonttype::TrueType )
                    m_aFontFileToFontID[ static_cast<TrueTypeFontFile*>(*it)->m_aFontFile ].insert( aFont );
                else if( (*it)->m_eType == fonttype::Builtin )
                    m_aFontFileToFontID[ static_cast<BuiltinFont*>(*it)->m_aMetricFile ].insert( aFont );
#if OSL_DEBUG_LEVEL > 1
                if( (*it)->m_eType == fonttype::Builtin )
                    nBuiltinFonts++;
                nCached++;
#if OSL_DEBUG_LEVEL > 2
                fprintf( stderr, "adding cached font %d: \"%s\" from %s\n", aFont,
                         OUStringToOString( getFontFamily( aFont ), RTL_TEXTENCODING_MS_1252 ).getStr(),
                         getFontFileSysPath( aFont ).getStr() );
#endif
#endif
            }
            continue;
        }

        DIR* pDIR = opendir( aDir.getStr() );
        if( pDIR )
        {
            struct dirent* pDirEntry = (struct dirent*)aDirEntBuffer;
            int nDirID = getDirectoryAtom( aDir, true );
            int nDirFonts = 0;

            while( ! readdir_r( pDIR, (struct dirent*)aDirEntBuffer, &pDirEntry ) && pDirEntry )
            {
                ByteString aFile( aDir );
                aFile += '/';
                aFile += pDirEntry->d_name;
                struct stat aStat;
                if( ! stat( aFile.GetBuffer(), &aStat )
                    && S_ISREG( aStat.st_mode )
                    )
                {
                    OString aFileName( pDirEntry->d_name, strlen( pDirEntry->d_name ) );
                    OString aExt( aFileName.copy( aFileName.lastIndexOf( '.' )+1 ) );
                    if( aExt.equalsIgnoreAsciiCase( "afm" ) )
                    {
                        ::std::list< PrintFont* > aNewFonts;

                        analyzeFontFile( nDirID, aFileName, aEmptyFontsDir, aNewFonts );
                        for( ::std::list< PrintFont* >::iterator it = aNewFonts.begin(); it != aNewFonts.end(); ++it )
                        {
                            if( findFontBuiltinID( (*it)->m_nPSName ) == 0 )
                            {
                                m_aFontFileToFontID[ aFileName ].insert( m_nNextFontID );
                                m_aFonts[ m_nNextFontID++ ] = *it;
                                m_pFontCache->updateFontCacheEntry( *it, false );
#if OSL_DEBUG_LEVEL > 2
                                nBuiltinFonts++;
#endif
                            }
                            else
                                delete *it;
                        }
                    }
                }
            }
            closedir( pDIR );
            if( ! nDirFonts )
                m_pFontCache->markEmptyDir( nDirID );
        }
    }

#if OSL_DEBUG_LEVEL > 1
    aStep2 = times( &tms );
#endif

    // part three - fill in family styles
    ::std::hash_map< fontID, PrintFont* >::iterator font_it;
    for (font_it = m_aFonts.begin(); font_it != m_aFonts.end(); ++font_it)
    {
        ::std::hash_map< int, family::type >::const_iterator it =
              m_aFamilyTypes.find( font_it->second->m_nFamilyName );
        if (it != m_aFamilyTypes.end())
            continue;
        const ::rtl::OUString& rFamily =
            m_pAtoms->getString( ATOM_FAMILYNAME, font_it->second->m_nFamilyName);
        family::type eType = matchFamilyName( rFamily );
        m_aFamilyTypes[ font_it->second->m_nFamilyName ] = eType;
    }

#if OSL_DEBUG_LEVEL > 1
    aStep3 = times( &tms );
    fprintf( stderr, "PrintFontManager::initialize: collected %d fonts (%d builtin, %d cached)\n", m_aFonts.size(), nBuiltinFonts, nCached );
    double fTick = (double)sysconf( _SC_CLK_TCK );
    fprintf( stderr, "Step 1 took %lf seconds\n", (double)(aStep1 - aStart)/fTick );
    fprintf( stderr, "Step 2 took %lf seconds\n", (double)(aStep2 - aStep1)/fTick );
    fprintf( stderr, "Step 3 took %lf seconds\n", (double)(aStep3 - aStep2)/fTick );
#endif

    m_pFontCache->flush();

    #ifdef CALLGRIND_COMPILE
    CALLGRIND_DUMP_STATS();
    CALLGRIND_TOGGLE_COLLECT();
    #endif
}

// -------------------------------------------------------------------------
inline bool
equalPitch (psp::pitch::type from, psp::pitch::type to)
{
    return from == to;
}

inline bool
equalWeight (psp::weight::type from, psp::weight::type to)
{
    return from > to ? (from - to) <= 3 : (to - from) <= 3;
}

inline bool
equalItalic (psp::italic::type from, psp::italic::type to)
{
    if ( (from == psp::italic::Italic) || (from == psp::italic::Oblique) )
        return (to == psp::italic::Italic) || (to == psp::italic::Oblique);
    return to == from;
}
inline bool
equalEncoding (rtl_TextEncoding from, rtl_TextEncoding to)
{
    if ((from == RTL_TEXTENCODING_ISO_8859_1) || (from == RTL_TEXTENCODING_MS_1252))
        return (to == RTL_TEXTENCODING_ISO_8859_1) || (to == RTL_TEXTENCODING_MS_1252);
    return from == to;
}

namespace {
    struct BuiltinFontIdentifier
    {
        OUString            aFamily;
        italic::type        eItalic;
        weight::type        eWeight;
        pitch::type         ePitch;
        rtl_TextEncoding    aEncoding;

        BuiltinFontIdentifier( const OUString& rFam,
                               italic::type eIt,
                               weight::type eWg,
                               pitch::type ePt,
                               rtl_TextEncoding enc ) :
            aFamily( rFam ),
            eItalic( eIt ),
            eWeight( eWg ),
            ePitch( ePt ),
            aEncoding( enc )
        {}

        bool operator==( const BuiltinFontIdentifier& rRight ) const
        {
            return equalItalic( eItalic, rRight.eItalic ) &&
                   equalWeight( eWeight, rRight.eWeight ) &&
                   equalPitch( ePitch, rRight.ePitch ) &&
                   equalEncoding( aEncoding, rRight.aEncoding ) &&
                   aFamily.equalsIgnoreAsciiCase( rRight.aFamily );
        }
    };

    struct BuiltinFontIdentifierHash
    {
        size_t operator()( const BuiltinFontIdentifier& rFont ) const
        {
            return rFont.aFamily.hashCode() ^ rFont.eItalic ^ rFont.eWeight ^ rFont.ePitch ^ rFont.aEncoding;
        }
    };
}

void PrintFontManager::getFontList( ::std::list< fontID >& rFontIDs, const PPDParser* pParser, bool bUseOverrideMetrics )
{
    rFontIDs.clear();
    std::hash_map< fontID, PrintFont* >::const_iterator it;

    /*
    * Note: there are two easy steps making this faster:
    * first: insert the printer builtins first, then the not builtins,
    * if they do not match.
    * drawback: this would change the sequence of fonts; this could have
    * subtle, unknown consequences in vcl font matching
    * second: instead of comparing attributes to see whether a softfont
    * is duplicate to a builtin one could simply compare the PSName (which is
    * supposed to be unique), which at this point is just an int.
    * drawback: this could change which fonts are listed; especially TrueType
    * fonts often have a rather dubious PSName, so this could change the
    * font list not so subtle.
    * Until getFontList for a printer becomes a performance issue (which is
    * currently not the case), best stay with the current algorithm.
    */

    // fill sets of printer supported fonts
    if( pParser )
    {
        std::set<int> aBuiltinPSNames;
        std::hash_set< BuiltinFontIdentifier,
                       BuiltinFontIdentifierHash
                       > aBuiltinFonts;

        std::map<int, fontID > aOverridePSNames;
        if( bUseOverrideMetrics )
        {
            readOverrideMetrics();
            for( std::vector<fontID>::const_iterator over = m_aOverrideFonts.begin();
                 over != m_aOverrideFonts.end(); ++over )
            {
                std::hash_map<fontID,PrintFont*>::const_iterator font_it = m_aFonts.find( *over );
                DBG_ASSERT( font_it != m_aFonts.end(), "override to nonexistant font" );
                if( font_it != m_aFonts.end() )
                    aOverridePSNames[ font_it->second->m_nPSName ] = *over;
            }
        }

        int nFonts = pParser->getFonts();
        for( int i = 0; i < nFonts; i++ )
            aBuiltinPSNames.insert( m_pAtoms->getAtom( ATOM_PSNAME, pParser->getFont( i ) ) );
        for( it = m_aFonts.begin(); it != m_aFonts.end(); ++it )
        {
            PrintFont* pFont = it->second;
            if( it->second->m_eType == fonttype::Builtin &&
                aBuiltinPSNames.find( pFont->m_nPSName ) != aBuiltinPSNames.end() )
            {
                bool bInsert = true;
                if( bUseOverrideMetrics )
                {
                    // in override case only use the override fonts, not their counterparts
                    std::map<int,fontID>::const_iterator over = aOverridePSNames.find( pFont->m_nPSName );
                    if( over != aOverridePSNames.end() && over->second != it->first )
                        bInsert = false;
                }
                else
                {
                    // do not insert override fonts in non override case
                    if( std::find( m_aOverrideFonts.begin(), m_aOverrideFonts.end(), it->first ) != m_aOverrideFonts.end() )
                        bInsert = false;
                }
                if( bInsert )
                {
                    aBuiltinFonts.insert( BuiltinFontIdentifier(
                        m_pAtoms->getString( ATOM_FAMILYNAME, pFont->m_nFamilyName ),
                        pFont->m_eItalic,
                        pFont->m_eWeight,
                        pFont->m_ePitch,
                        pFont->m_aEncoding
                        ) );
                }
            }
        }
        for( it = m_aFonts.begin(); it != m_aFonts.end(); ++it )
        {
            PrintFont* pFont = it->second;
            if( it->second->m_eType == fonttype::Builtin )
            {
                if( aBuiltinPSNames.find( pFont->m_nPSName ) != aBuiltinPSNames.end() )
                {
                    bool bInsert = true;
                    if( bUseOverrideMetrics )
                    {
                        // in override case only use the override fonts, not their counterparts
                        std::map<int,fontID>::const_iterator over = aOverridePSNames.find( pFont->m_nPSName );
                        if( over != aOverridePSNames.end() && over->second != it->first )
                            bInsert = false;
                    }
                    else
                    {
                        // do not insert override fonts in non override case
                        if( std::find( m_aOverrideFonts.begin(), m_aOverrideFonts.end(), it->first ) != m_aOverrideFonts.end() )
                            bInsert = false;
                    }
                    if( bInsert )
                        rFontIDs.push_back( it->first );
                }
            }
            else if( aBuiltinFonts.find( BuiltinFontIdentifier(
                m_pAtoms->getString( ATOM_FAMILYNAME, pFont->m_nFamilyName ),
                pFont->m_eItalic,
                pFont->m_eWeight,
                pFont->m_ePitch,
                pFont->m_aEncoding
                ) ) == aBuiltinFonts.end() )
            {
                rFontIDs.push_back( it->first );
            }
        }
    }
    else // no specific printer
    {
        for( it = m_aFonts.begin(); it != m_aFonts.end(); ++it )
            rFontIDs.push_back( it->first );
    }
}

// -------------------------------------------------------------------------

void PrintFontManager::fillPrintFontInfo( PrintFont* pFont, FastPrintFontInfo& rInfo ) const
{
    ::std::hash_map< int, family::type >::const_iterator style_it =
          m_aFamilyTypes.find( pFont->m_nFamilyName );
    rInfo.m_eType           = pFont->m_eType;
    rInfo.m_aFamilyName     = m_pAtoms->getString( ATOM_FAMILYNAME, pFont->m_nFamilyName );
    rInfo.m_aStyleName      = pFont->m_aStyleName;
    rInfo.m_eFamilyStyle    = style_it != m_aFamilyTypes.end() ? style_it->second : family::Unknown;
    rInfo.m_eItalic         = pFont->m_eItalic;
    rInfo.m_eWidth          = pFont->m_eWidth;
    rInfo.m_eWeight         = pFont->m_eWeight;
    rInfo.m_ePitch          = pFont->m_ePitch;
    rInfo.m_aEncoding       = pFont->m_aEncoding;
    rInfo.m_eEmbeddedbitmap = pFont->m_eEmbeddedbitmap;
    rInfo.m_eAntialias      = pFont->m_eAntialias;
    rInfo.m_aAliases.clear();
    for( ::std::list< int >::iterator it = pFont->m_aAliases.begin(); it != pFont->m_aAliases.end(); ++it )
        rInfo.m_aAliases.push_back( m_pAtoms->getString( ATOM_FAMILYNAME, *it ) );
}

// -------------------------------------------------------------------------

void PrintFontManager::fillPrintFontInfo( PrintFont* pFont, PrintFontInfo& rInfo ) const
{
    if( ( pFont->m_nAscend == 0 && pFont->m_nDescend == 0 ) ||
        ! pFont->m_pMetrics || pFont->m_pMetrics->isEmpty()
        )
    {
        // might be a truetype font not analyzed or type1 without metrics read
        if( pFont->m_eType == fonttype::Type1 )
            pFont->readAfmMetrics( getAfmFile( pFont ), m_pAtoms, false, false );
        else if( pFont->m_eType == fonttype::TrueType )
            analyzeTrueTypeFile( pFont );
    }

    fillPrintFontInfo( pFont, static_cast< FastPrintFontInfo& >( rInfo ) );

    rInfo.m_nAscend         = pFont->m_nAscend;
    rInfo.m_nDescend        = pFont->m_nDescend;
    rInfo.m_nLeading        = pFont->m_nLeading;
    rInfo.m_nWidth          = pFont->m_aGlobalMetricX.width < pFont->m_aGlobalMetricY.width ? pFont->m_aGlobalMetricY.width : pFont->m_aGlobalMetricX.width;
}

// -------------------------------------------------------------------------

void PrintFontManager::getFontListWithInfo( ::std::list< PrintFontInfo >& rFonts, const PPDParser* pParser, bool bUseOverrideMetrics )
{
    rFonts.clear();
    ::std::list< fontID > aFontList;
    getFontList( aFontList, pParser, bUseOverrideMetrics );

    ::std::list< fontID >::iterator it;
    for( it = aFontList.begin(); it != aFontList.end(); ++it )
    {
        PrintFontInfo aInfo;
        aInfo.m_nID = *it;
        fillPrintFontInfo( getFont( *it ), aInfo );
        rFonts.push_back( aInfo );
    }
}

// -------------------------------------------------------------------------

void PrintFontManager::getFontListWithFastInfo( ::std::list< FastPrintFontInfo >& rFonts, const PPDParser* pParser, bool bUseOverrideMetrics )
{
    rFonts.clear();
    ::std::list< fontID > aFontList;
    getFontList( aFontList, pParser, bUseOverrideMetrics );

    ::std::list< fontID >::iterator it;
    for( it = aFontList.begin(); it != aFontList.end(); ++it )
    {
        FastPrintFontInfo aInfo;
        aInfo.m_nID = *it;
        fillPrintFontInfo( getFont( *it ), aInfo );
        rFonts.push_back( aInfo );
    }
}

// -------------------------------------------------------------------------

bool PrintFontManager::getFontInfo( fontID nFontID, PrintFontInfo& rInfo ) const
{
    PrintFont* pFont = getFont( nFontID );
    if( pFont )
    {
        rInfo.m_nID = nFontID;
        fillPrintFontInfo( pFont, rInfo );
    }
    return pFont ? true : false;
}

// -------------------------------------------------------------------------

bool PrintFontManager::getFontFastInfo( fontID nFontID, FastPrintFontInfo& rInfo ) const
{
    PrintFont* pFont = getFont( nFontID );
    if( pFont )
    {
        rInfo.m_nID = nFontID;
        fillPrintFontInfo( pFont, rInfo );
    }
    return pFont ? true : false;
}

// -------------------------------------------------------------------------

bool PrintFontManager::getFontBoundingBox( fontID nFontID, int& xMin, int& yMin, int& xMax, int& yMax )
{
    bool bSuccess = false;
    PrintFont* pFont = getFont( nFontID );
    if( pFont )
    {
        if( pFont->m_nXMin == 0 && pFont->m_nYMin == 0 && pFont->m_nXMax == 0 && pFont->m_nYMax == 0 )
        {
            // might be a truetype font not analyzed or type1 without metrics read
            if( pFont->m_eType == fonttype::Type1 || pFont->m_eType == fonttype::Builtin )
                pFont->readAfmMetrics( getAfmFile( pFont ), m_pAtoms, false, true );
            else if( pFont->m_eType == fonttype::TrueType )
                analyzeTrueTypeFile( pFont );
        }
        bSuccess = true;
        xMin = pFont->m_nXMin;
        yMin = pFont->m_nYMin;
        xMax = pFont->m_nXMax;
        yMax = pFont->m_nYMax;
    }
    return bSuccess;
}

// -------------------------------------------------------------------------

int PrintFontManager::getFontFaceNumber( fontID nFontID ) const
{
    int nRet = -1;
    PrintFont* pFont = getFont( nFontID );
    if( pFont && pFont->m_eType == fonttype::TrueType )
        nRet = static_cast< TrueTypeFontFile* >(pFont)->m_nCollectionEntry;
    return nRet;
}

// -------------------------------------------------------------------------


family::type PrintFontManager::matchFamilyName( const ::rtl::OUString& rFamily ) const
{
    typedef struct {
        const char*  mpName;
        sal_uInt16   mnLength;
        family::type meType;
    } family_t;

#define InitializeClass( p, a ) p, sizeof(p) - 1, a
    const family_t pFamilyMatch[] =  {
        { InitializeClass( "arial",                  family::Swiss )  },
        { InitializeClass( "arioso",                 family::Script ) },
        { InitializeClass( "avant garde",            family::Swiss )  },
        { InitializeClass( "avantgarde",             family::Swiss )  },
        { InitializeClass( "bembo",                  family::Roman )  },
        { InitializeClass( "bookman",                family::Roman )  },
        { InitializeClass( "conga",                  family::Roman )  },
        { InitializeClass( "courier",                family::Modern ) },
        { InitializeClass( "curl",                   family::Script ) },
        { InitializeClass( "fixed",                  family::Modern ) },
        { InitializeClass( "gill",                   family::Swiss )  },
        { InitializeClass( "helmet",                 family::Modern ) },
        { InitializeClass( "helvetica",              family::Swiss )  },
        { InitializeClass( "international",          family::Modern ) },
        { InitializeClass( "lucida",                 family::Swiss )  },
        { InitializeClass( "new century schoolbook", family::Roman )  },
        { InitializeClass( "palatino",               family::Roman )  },
        { InitializeClass( "roman",                  family::Roman )  },
        { InitializeClass( "sans serif",             family::Swiss )  },
        { InitializeClass( "sansserif",              family::Swiss )  },
        { InitializeClass( "serf",                   family::Roman )  },
        { InitializeClass( "serif",                  family::Roman )  },
        { InitializeClass( "times",                  family::Roman )  },
        { InitializeClass( "utopia",                 family::Roman )  },
        { InitializeClass( "zapf chancery",          family::Script ) },
        { InitializeClass( "zapfchancery",           family::Script ) }
    };

    rtl::OString aFamily = rtl::OUStringToOString( rFamily, RTL_TEXTENCODING_ASCII_US );
    sal_uInt32 nLower = 0;
    sal_uInt32 nUpper = sizeof(pFamilyMatch) / sizeof(pFamilyMatch[0]);

    while( nLower < nUpper )
    {
        sal_uInt32 nCurrent = (nLower + nUpper) / 2;
        const family_t* pHaystack = pFamilyMatch + nCurrent;
        sal_Int32  nComparison =
            rtl_str_compareIgnoreAsciiCase_WithLength
            (
             aFamily.getStr(), aFamily.getLength(),
             pHaystack->mpName, pHaystack->mnLength
             );

        if( nComparison < 0 )
            nUpper = nCurrent;
        else
            if( nComparison > 0 )
                nLower = nCurrent + 1;
            else
                return pHaystack->meType;
    }

    return family::Unknown;
}

// -------------------------------------------------------------------------

family::type PrintFontManager::getFontFamilyType( fontID nFontID ) const
{
    PrintFont* pFont = getFont( nFontID );
    if( !pFont )
        return family::Unknown;

    ::std::hash_map< int, family::type >::const_iterator it =
          m_aFamilyTypes.find( pFont->m_nFamilyName );
    return (it != m_aFamilyTypes.end()) ? it->second : family::Unknown;
}


// -------------------------------------------------------------------------

const ::rtl::OUString& PrintFontManager::getFontFamily( fontID nFontID ) const
{
    PrintFont* pFont = getFont( nFontID );
    return m_pAtoms->getString( ATOM_FAMILYNAME, pFont ? pFont->m_nFamilyName : INVALID_ATOM );
}

// -------------------------------------------------------------------------

OString PrintFontManager::getAfmFile( PrintFont* pFont ) const
{
    OString aMetricPath;
    if( pFont )
    {
        switch( pFont->m_eType )
        {
            case fonttype::Type1:
            {
                Type1FontFile* pPSFont = static_cast< Type1FontFile* >(pFont);
                aMetricPath = getDirectory( pPSFont->m_nDirectory );
                aMetricPath += "/";
                aMetricPath += pPSFont->m_aMetricFile;
            }
            break;
            case fonttype::Builtin:
            {
                BuiltinFont* pBuiltinFont = static_cast< BuiltinFont* >(pFont);
                aMetricPath = getDirectory( pBuiltinFont->m_nDirectory );
                aMetricPath += "/";
                aMetricPath += pBuiltinFont->m_aMetricFile;
            }
            break;
            default: break;
        }
    }
    return aMetricPath;
}

// -------------------------------------------------------------------------

OString PrintFontManager::getFontFile( PrintFont* pFont ) const
{
    OString aPath;

    if( pFont && pFont->m_eType == fonttype::Type1 )
    {
        Type1FontFile* pPSFont = static_cast< Type1FontFile* >(pFont);
        ::std::hash_map< int, OString >::const_iterator it = m_aAtomToDir.find( pPSFont->m_nDirectory );
        aPath = it->second;
        aPath += "/";
        aPath += pPSFont->m_aFontFile;
    }
    else if( pFont && pFont->m_eType == fonttype::TrueType )
    {
        TrueTypeFontFile* pTTFont = static_cast< TrueTypeFontFile* >(pFont);
        ::std::hash_map< int, OString >::const_iterator it = m_aAtomToDir.find( pTTFont->m_nDirectory );
        aPath = it->second;
        aPath += "/";
        aPath += pTTFont->m_aFontFile;
    }
    return aPath;
}

// -------------------------------------------------------------------------

const ::rtl::OUString& PrintFontManager::getPSName( fontID nFontID ) const
{
    PrintFont* pFont = getFont( nFontID );
    if( pFont && pFont->m_nPSName == 0 )
    {
        if( pFont->m_eType == fonttype::TrueType )
            analyzeTrueTypeFile( pFont );
    }

    return m_pAtoms->getString( ATOM_PSNAME, pFont ? pFont->m_nPSName : INVALID_ATOM );
}

// -------------------------------------------------------------------------

const CharacterMetric& PrintFontManager::getGlobalFontMetric( fontID nFontID, bool bHorizontal ) const
{
    static CharacterMetric aMetric;
    PrintFont* pFont = getFont( nFontID );
    return pFont ? ( bHorizontal ? pFont->m_aGlobalMetricX : pFont->m_aGlobalMetricY ) : aMetric;
}

// -------------------------------------------------------------------------

int PrintFontManager::getFontAscend( fontID nFontID ) const
{
    PrintFont* pFont = getFont( nFontID );
    if( pFont->m_nAscend == 0 && pFont->m_nDescend == 0 )
    {
        // might be a truetype font not yet analyzed
        if( pFont->m_eType == fonttype::TrueType )
            analyzeTrueTypeFile( pFont );
        else if( pFont->m_eType == fonttype::Type1 || pFont->m_eType == fonttype::Builtin )
            pFont->readAfmMetrics( getAfmFile( pFont ), m_pAtoms, false, true );
    }
    return pFont->m_nAscend;
}

// -------------------------------------------------------------------------

int PrintFontManager::getFontDescend( fontID nFontID ) const
{
    PrintFont* pFont = getFont( nFontID );
    if( pFont->m_nAscend == 0 && pFont->m_nDescend == 0 )
    {
        // might be a truetype font not yet analyzed
        if( pFont->m_eType == fonttype::TrueType )
            analyzeTrueTypeFile( pFont );
        else if( pFont->m_eType == fonttype::Type1 || pFont->m_eType == fonttype::Builtin )
            pFont->readAfmMetrics( getAfmFile( pFont ), m_pAtoms, false, true );
    }
    return pFont->m_nDescend;
}

// -------------------------------------------------------------------------

int PrintFontManager::getFontLeading( fontID nFontID ) const
{
    PrintFont* pFont = getFont( nFontID );
    if( pFont->m_nAscend == 0 && pFont->m_nDescend == 0 )
    {
        // might be a truetype font not yet analyzed
        if( pFont->m_eType == fonttype::TrueType )
            analyzeTrueTypeFile( pFont );
    }
    return pFont->m_nLeading;
}

// -------------------------------------------------------------------------

bool PrintFontManager::hasVerticalSubstitutions( fontID nFontID ) const
{
    PrintFont* pFont = getFont( nFontID );
    if( pFont->m_nAscend == 0 && pFont->m_nDescend == 0 )
    {
        // might be a truetype font not yet analyzed
        if( pFont->m_eType == fonttype::TrueType )
            analyzeTrueTypeFile( pFont );
    }
    return pFont->m_bHaveVerticalSubstitutedGlyphs;
}

// -------------------------------------------------------------------------

void PrintFontManager::hasVerticalSubstitutions( fontID nFontID,
    const sal_Unicode* pCharacters, int nCharacters, bool* pHasSubst ) const
{
    PrintFont* pFont = getFont( nFontID );
    if( pFont->m_nAscend == 0 && pFont->m_nDescend == 0 )
    {
        // might be a truetype font not yet analyzed
        if( pFont->m_eType == fonttype::TrueType )
            analyzeTrueTypeFile( pFont );
    }

    if( ! pFont->m_bHaveVerticalSubstitutedGlyphs )
        memset( pHasSubst, 0, sizeof(bool)*nCharacters );
    else
    {
        for( int i = 0; i < nCharacters; i++ )
        {
            sal_Unicode code = pCharacters[i];
            if( ! pFont->m_pMetrics ||
                ! ( pFont->m_pMetrics->m_aPages[ code >> 11 ] & ( 1 << ( ( code >> 8 ) & 7 ) ) ) )
                pFont->queryMetricPage( code >> 8, m_pAtoms );
            ::std::hash_map< sal_Unicode, bool >::const_iterator it = pFont->m_pMetrics->m_bVerticalSubstitutions.find( code );
            pHasSubst[i] = it != pFont->m_pMetrics->m_bVerticalSubstitutions.end();
        }
    }
}

// -------------------------------------------------------------------------

OUString PrintFontManager::getFontXLFD( fontID nFontID ) const
{
    PrintFont* pFont = getFont( nFontID );
    OUString aRet;
    if( pFont )
    {
        ByteString aXLFD( getXLFD( pFont ) );
        rtl_TextEncoding aEncoding = aXLFD.GetToken( 6, '-' ).Search( "utf8" ) != STRING_NOTFOUND ? RTL_TEXTENCODING_UTF8 : RTL_TEXTENCODING_ISO_8859_1;
        aRet = OStringToOUString( aXLFD, aEncoding );
    }
    return aRet;
}

// -------------------------------------------------------------------------

const ::std::list< KernPair >& PrintFontManager::getKernPairs( fontID nFontID, bool bVertical ) const
{
    static ::std::list< KernPair > aEmpty;

    PrintFont* pFont = getFont( nFontID );
    if( ! pFont )
        return aEmpty;

    if( ! pFont->m_pMetrics || ! pFont->m_pMetrics->m_bKernPairsQueried )
        pFont->queryMetricPage( 0, m_pAtoms );
    if( ! pFont->m_pMetrics || ! pFont->m_pMetrics->m_bKernPairsQueried )
        return aEmpty;
    return bVertical ? pFont->m_pMetrics->m_aYKernPairs : pFont->m_pMetrics->m_aXKernPairs;
}

// -------------------------------------------------------------------------

bool PrintFontManager::isFontDownloadingAllowed( fontID nFont ) const
{
    static const char* pEnable = getenv( "PSPRINT_ENABLE_TTF_COPYRIGHTAWARENESS" );
    bool bRet = true;

    if( pEnable && *pEnable )
    {
        PrintFont* pFont = getFont( nFont );
        if( pFont && pFont->m_eType == fonttype::TrueType )
        {
            TrueTypeFontFile* pTTFontFile = static_cast<TrueTypeFontFile*>(pFont);
            if( pTTFontFile->m_nTypeFlags & 0x80000000 )
            {
                TrueTypeFont* pTTFont = NULL;
                ByteString aFile = getFontFile( pFont );
                if( OpenTTFontFile( aFile.GetBuffer(), pTTFontFile->m_nCollectionEntry < 0 ? 0 : pTTFontFile->m_nCollectionEntry, &pTTFont ) == SF_OK )
                {
                    // get type flags
                    TTGlobalFontInfo aInfo;
                    GetTTGlobalFontInfo( pTTFont, & aInfo );
                    pTTFontFile->m_nTypeFlags = (unsigned int)aInfo.typeFlags;
                    CloseTTFont( pTTFont );
                }
            }

            unsigned int nCopyrightFlags = pTTFontFile->m_nTypeFlags & 0x0e;

            // font embedding is allowed if either
            //   no restriction at all (bit 1 clear)
            //   printing allowed (bit 1 set, bit 2 set )
            bRet = ! ( nCopyrightFlags & 0x02 ) || ( nCopyrightFlags & 0x04 );
        }
    }
    return bRet;
}

// -------------------------------------------------------------------------

bool PrintFontManager::getMetrics( fontID nFontID, const sal_Unicode* pString, int nLen, CharacterMetric* pArray, bool bVertical ) const
{
    PrintFont* pFont = getFont( nFontID );
    if( ! pFont )
        return false;

    if( ( pFont->m_nAscend == 0 && pFont->m_nDescend == 0 )
        || ! pFont->m_pMetrics || pFont->m_pMetrics->isEmpty()
        )
    {
        // might be a font not yet analyzed
        if( pFont->m_eType == fonttype::Type1 || pFont->m_eType == fonttype::Builtin )
            pFont->readAfmMetrics( getAfmFile( pFont ), m_pAtoms, false, false );
        else if( pFont->m_eType == fonttype::TrueType )
            analyzeTrueTypeFile( pFont );
    }

    for( int i = 0; i < nLen; i++ )
    {
        if( ! pFont->m_pMetrics ||
            ! ( pFont->m_pMetrics->m_aPages[ pString[i] >> 11 ] & ( 1 << ( ( pString[i] >> 8 ) & 7 ) ) ) )
            pFont->queryMetricPage( pString[i] >> 8, m_pAtoms );
        pArray[i].width = pArray[i].height = -1;
        if( pFont->m_pMetrics )
        {
            int effectiveCode = pString[i];
            effectiveCode |= bVertical ? 1 << 16 : 0;
            ::std::hash_map< int, CharacterMetric >::const_iterator it =
                  pFont->m_pMetrics->m_aMetrics.find( effectiveCode );
        // if no vertical metrics are available assume rotated horizontal metrics
        if( bVertical && (it == pFont->m_pMetrics->m_aMetrics.end()) )
                  it = pFont->m_pMetrics->m_aMetrics.find( pString[i] );
        // the character metrics are in it->second
            if( it != pFont->m_pMetrics->m_aMetrics.end() )
                pArray[ i ] = it->second;
        }
    }

    return true;
}

// -------------------------------------------------------------------------

bool PrintFontManager::getMetrics( fontID nFontID, sal_Unicode minCharacter, sal_Unicode maxCharacter, CharacterMetric* pArray, bool bVertical ) const
{
    PrintFont* pFont = getFont( nFontID );
    if( ! pFont )
        return false;

    if( ( pFont->m_nAscend == 0 && pFont->m_nDescend == 0 )
        || ! pFont->m_pMetrics || pFont->m_pMetrics->isEmpty()
        )
    {
        // might be a font not yet analyzed
        if( pFont->m_eType == fonttype::Type1 || pFont->m_eType == fonttype::Builtin )
            pFont->readAfmMetrics( getAfmFile( pFont ), m_pAtoms, false, false );
        else if( pFont->m_eType == fonttype::TrueType )
            analyzeTrueTypeFile( pFont );
    }

    for( sal_Unicode code = minCharacter; code <= maxCharacter; code++ )
    {
        if( ! pFont->m_pMetrics ||
            ! ( pFont->m_pMetrics->m_aPages[ code >> 11 ] & ( 1 << ( ( code >> 8 ) & 7 ) ) ) )
            pFont->queryMetricPage( code >> 8, m_pAtoms );
        pArray[ code - minCharacter ].width     = -1;
        pArray[ code - minCharacter ].height    = -1;
        if( pFont->m_pMetrics )
        {
            int effectiveCode = code;
            effectiveCode |= bVertical ? 1 << 16 : 0;
            ::std::hash_map< int, CharacterMetric >::const_iterator it =
                  pFont->m_pMetrics->m_aMetrics.find( effectiveCode );
            // if no vertical metrics are available assume rotated horizontal metrics
            if( bVertical && (it == pFont->m_pMetrics->m_aMetrics.end()) )
                it = pFont->m_pMetrics->m_aMetrics.find( code );
            // the character metrics are in it->second
            if( it != pFont->m_pMetrics->m_aMetrics.end() )
                pArray[ code - minCharacter ] = it->second;
        }
    }
    return true;
}

// -------------------------------------------------------------------------

static bool createWriteablePath( const ByteString& rPath )
{
    bool bSuccess = false;

    if( access( rPath.GetBuffer(), W_OK ) )
    {
        int nPos = rPath.SearchBackward( '/' );
        if( nPos != STRING_NOTFOUND )
            while( nPos > 0 && rPath.GetChar( nPos ) == '/' )
                nPos--;

        if( nPos != STRING_NOTFOUND && nPos != 0 && createWriteablePath( rPath.Copy( 0, nPos+1 ) ) )
        {
            bSuccess = mkdir( rPath.GetBuffer(), 0777 ) ? false : true;
        }
    }
    else
        bSuccess = true;

    return bSuccess;
}


// -------------------------------------------------------------------------

int PrintFontManager::importFonts( const ::std::list< OString >& rFiles, bool bLinkOnly, ImportFontCallback* pCallback )
{
    int nSuccess = 0;

    // find a directory with write access
    rtl_TextEncoding aEncoding = osl_getThreadTextEncoding();
    bool bCanWrite = false;
    int nDirID = 0;
    INetURLObject aDir;
    for( ::std::list< int >::const_iterator dir_it = m_aPrivateFontDirectories.begin();
         ! bCanWrite && dir_it != m_aPrivateFontDirectories.end(); ++dir_it )
    {
        // check if we can create files in that directory
        ByteString aDirPath = getDirectory( *dir_it );
        if( createWriteablePath( aDirPath ) )
        {
            aDir = INetURLObject( OStringToOUString( aDirPath, aEncoding ), INET_PROT_FILE, INetURLObject::ENCODE_ALL );
            nDirID = *dir_it;
            bCanWrite = true;
        }
    }
    if( bCanWrite )
    {
        for( ::std::list< OString >::const_iterator font_it = rFiles.begin();
             font_it != rFiles.end(); ++font_it )
        {
            INetURLObject aFrom( OStringToOUString( *font_it, aEncoding ), INET_PROT_FILE, INetURLObject::ENCODE_ALL );
            INetURLObject aTo( aDir );
            aTo.Append( aFrom.GetName() );

            if( pCallback )
                pCallback->progress( aTo.PathToFileName() );

            if( pCallback && pCallback->isCanceled() )
                break;

            if( ! access( ByteString( String(aTo.PathToFileName()), aEncoding ).GetBuffer(), F_OK ) )
            {
                if( ! ( pCallback ? pCallback->queryOverwriteFile( aTo.PathToFileName() ) : false ) )
                    continue;
            }
            // look for afm if necessary
            OUString aAfmCopied;
            FileBase::RC nError;
            if( aFrom.getExtension().equalsIgnoreAsciiCaseAscii( "pfa" ) ||
                aFrom.getExtension().equalsIgnoreAsciiCaseAscii( "pfb" ) )
            {
                INetURLObject aFromAfm( aFrom );
                aFromAfm.setExtension( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "afm" ) ) );
                if( access( ByteString( String(aFromAfm.PathToFileName()), aEncoding ).GetBuffer(), F_OK ) )
                {
                    aFromAfm.setExtension( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "AFM" ) ) );
                    if( access( ByteString( String(aFromAfm.PathToFileName()), aEncoding ).GetBuffer(), F_OK ) )
                    {
                        aFromAfm.removeSegment();
                        aFromAfm.Append( String( RTL_CONSTASCII_USTRINGPARAM( "afm" ) ) );
                        aFromAfm.Append( aTo.GetName() );
                        aFromAfm.setExtension( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "afm" ) ) );
                        if( access( ByteString( String(aFromAfm.PathToFileName()), aEncoding ).GetBuffer(), F_OK ) )
                        {
                            aFromAfm.setExtension( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "AFM" ) ) );
                            if( access( ByteString( String(aFromAfm.PathToFileName()), aEncoding ).GetBuffer(), F_OK ) )
                            {
                                // give up
                                if( pCallback )
                                    pCallback->importFontFailed( aTo.PathToFileName(), ImportFontCallback::NoAfmMetric );
                                continue;
                            }
                        }
                    }
                }
                INetURLObject aToAfm( aTo );
                aToAfm.setExtension( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "afm" ) ) );
                OUString aFromPath, aToPath;
                if( bLinkOnly )
                {
                    ByteString aLinkFromPath( String(aFromAfm.PathToFileName()),
                        aEncoding );
                    ByteString aLinkToPath( String(aToAfm.PathToFileName()),
                        aEncoding );
                    nError = (FileBase::RC)symlink( aLinkFromPath.GetBuffer(), aLinkToPath.GetBuffer() );
                }
                else
                    nError = File::copy( aFromAfm.GetMainURL(INetURLObject::DECODE_TO_IURI), aToAfm.GetMainURL(INetURLObject::DECODE_TO_IURI) );
                if( nError )
                {
                    if( pCallback )
                        pCallback->importFontFailed( aTo.PathToFileName(), ImportFontCallback::AfmCopyFailed );
                    continue;
                }
                aAfmCopied = aToPath;
            }
            if( bLinkOnly )
            {
                ByteString aFromPath( String(aFrom.PathToFileName()),
                    aEncoding );
                ByteString aToPath( String(aTo.PathToFileName()), aEncoding );
                nError = (FileBase::RC)symlink( aFromPath.GetBuffer(),
                    aToPath.GetBuffer() );
            }
            else
                nError = File::copy( aFrom.GetMainURL(INetURLObject::DECODE_TO_IURI), aTo.GetMainURL(INetURLObject::DECODE_TO_IURI) );
            // copy font file
            if( nError )
            {
                if( aAfmCopied.getLength() )
                    File::remove( aAfmCopied );
                if( pCallback )
                    pCallback->importFontFailed( aTo.PathToFileName(), ImportFontCallback::FontCopyFailed );
                continue;
            }

            ::std::list< PrintFont* > aNewFonts;
            ::std::list< PrintFont* >::iterator it;
            if( analyzeFontFile( nDirID, OUStringToOString( aTo.GetName(), aEncoding ), ::std::list<OString>(), aNewFonts ) )
            {
                // remove all fonts for the same file
                // discarding their font ids
                ::std::hash_map< fontID, PrintFont* >::iterator current, next;
                current = m_aFonts.begin();
                OString aFileName( OUStringToOString( aTo.GetName(), aEncoding ) );
                while( current != m_aFonts.end() )
                {
                    bool bRemove = false;
                    switch( current->second->m_eType )
                    {
                        case fonttype::Type1:
                            if( static_cast<Type1FontFile*>(current->second)->m_aFontFile == aFileName )
                                bRemove = true;
                            break;
                        case fonttype::TrueType:
                            if( static_cast<TrueTypeFontFile*>(current->second)->m_aFontFile == aFileName )
                                bRemove = true;
                            break;
                        default: break;
                    }
                    if( bRemove )
                    {
                        next = current;
                        ++next;
                        m_aFontFileToFontID[ aFileName ].erase( current->first );
                        delete current->second;
                        m_aFonts.erase( current );
                        current = next;
                    }
                    else
                        ++current;
                }

                DBG_ASSERT( !findFontFileID( nDirID, aFileName ), "not all fonts removed for file" );

                nSuccess++;
                for( it = aNewFonts.begin(); it != aNewFonts.end(); ++it )
                {
                    m_aFontFileToFontID[ aFileName ].insert( m_nNextFontID );
                    m_aFonts[ m_nNextFontID++ ] = *it;
                    m_pFontCache->updateFontCacheEntry( *it, false );
                }
            }
        }

        m_pFontCache->updateDirTimestamp( nDirID );
        m_pFontCache->flush();
    }
    else if( pCallback )
        pCallback->importFontsFailed( ImportFontCallback::NoWritableDirectory );

    return nSuccess;
}

// -------------------------------------------------------------------------

bool PrintFontManager::checkImportPossible() const
{
    bool bSuccess = false;

    // find a directory with write access
    ByteString aDir;
    for( std::list< int >::const_iterator dir_it = m_aPrivateFontDirectories.begin();
         dir_it != m_aPrivateFontDirectories.end(); ++dir_it )
    {
        aDir = getDirectory( *dir_it );
        if( createWriteablePath( aDir ) )
        {
            bSuccess = true;
            break;
        }
    }

#if OSL_DEBUG_LEVEL > 1
    if( bSuccess )
        fprintf( stderr, "found writable %s\n", aDir.GetBuffer() );
#endif

    return bSuccess;
}

// -------------------------------------------------------------------------

bool PrintFontManager::checkChangeFontPropertiesPossible( fontID /*nFontID*/ ) const
{
    // since font properties are changed in the font cache file only nowadays
    // they can always be changed
    return true;
}

// -------------------------------------------------------------------------

bool PrintFontManager::changeFontProperties( fontID nFontID, const ::rtl::OUString& rXLFD )
{
    ByteString aXLFD( OUStringToOString( rXLFD, RTL_TEXTENCODING_UTF8 ) );
    ByteString aAddStyle = aXLFD.GetToken( '-', 6 );
    if( aAddStyle.Search( "utf8" ) == STRING_NOTFOUND )
    {
        aAddStyle.Append( aAddStyle.Len() ? ";utf8" : "utf8" );
        aXLFD.SetToken( 6, ';', aAddStyle );
    }
    PrintFont* pFont = getFont( nFontID );
    std::list< OString > aDummyList;
    aDummyList.push_back( aXLFD );
    getFontAttributesFromXLFD( pFont, aDummyList );
    pFont->m_bUserOverride = true;
    m_pFontCache->updateFontCacheEntry( pFont, true );

    return true;
}

// -------------------------------------------------------------------------

bool PrintFontManager::
getImportableFontProperties(
                            const OString& rFile,
                            ::std::list< FastPrintFontInfo >& rFontProps
                            )
{
    rFontProps.clear();
    int nIndex = rFile.lastIndexOf( '/' );
    OString aDir, aFile( rFile.copy( nIndex+1 ) );
    if( nIndex != -1 )
        aDir = rFile.copy( 0, nIndex );
    int nDirID = getDirectoryAtom( aDir, true );
    ::std::list< PrintFont* > aFonts;
    bool bRet = analyzeFontFile( nDirID, aFile, ::std::list<OString>(), aFonts );
    while( aFonts.begin() != aFonts.end() )
    {
        PrintFont* pFont = aFonts.front();
        aFonts.pop_front();
        FastPrintFontInfo aInfo;
        fillPrintFontInfo( pFont, aInfo );
        rFontProps.push_back( aInfo );
        delete pFont;
    }
    return bRet;
}

// -------------------------------------------------------------------------

bool PrintFontManager::getFileDuplicates( fontID nFont, ::std::list< fontID >& rFonts ) const
{
    bool bRet = false;

    rFonts.clear();

    PrintFont* pSearchFont = getFont( nFont );
    if( ! pSearchFont ||
        pSearchFont->m_eType != fonttype::TrueType ||
        static_cast<TrueTypeFontFile*>(pSearchFont)->m_nCollectionEntry == -1
        )
        return false;

    OString aFile( getFontFileSysPath( nFont ) );
    if( ! aFile.getLength() )
        return false;

    for( ::std::hash_map< fontID, PrintFont* >::const_iterator it = m_aFonts.begin(); it != m_aFonts.end(); ++it )
    {
        if( nFont != it->first )
        {
            OString aCompFile( getFontFile( it->second ) );
            if( aCompFile == aFile )
            {
                rFonts.push_back( it->first );
                bRet = true;
            }
        }
    }
    return bRet;
}

// -------------------------------------------------------------------------

bool PrintFontManager::removeFonts( const ::std::list< fontID >& rFonts )
{
    bool bRet = true;
    ::std::list< fontID > aDuplicates;
    for( ::std::list< fontID >::const_iterator it = rFonts.begin(); it != rFonts.end(); ++it )
    {
        ::std::hash_map< fontID, PrintFont* >::const_iterator haveFont = m_aFonts.find( *it );
        if( haveFont == m_aFonts.end() )
            continue;

        PrintFont* pFont = haveFont->second;
        bool bRemoveDuplicates = getFileDuplicates( *it, aDuplicates );
        ByteString aFile( getFontFile( pFont ) );
        if( aFile.Len() )
        {
#if OSL_DEBUG_LEVEL > 1
            fprintf( stderr, "try unlink( \"%s\" ) ... ", aFile.GetBuffer() );
#endif
            if( unlink( aFile.GetBuffer() ) )
            {
                bRet = false;
#if OSL_DEBUG_LEVEL > 1
                fprintf( stderr, "failed\n" );
#endif
                continue;
            }
#if OSL_DEBUG_LEVEL > 1
            fprintf( stderr, "succeeded\n" );
#endif
            OString aAfm( getAfmFile( pFont ) );
            if( aAfm.getLength() )
            {
#if OSL_DEBUG_LEVEL > 1
                fprintf( stderr, "unlink( \"%s\" )\n", aAfm.getStr() );
#endif
                unlink( aAfm.getStr() );
            }
            m_aFonts.erase( *it );
            delete pFont;
            if( bRemoveDuplicates )
            {
                for( ::std::list< fontID >::iterator dup = aDuplicates.begin(); dup != aDuplicates.end(); ++dup )
                {
                    m_aFontFileToFontID[ aFile ].erase( *dup );
                    PrintFont* pDup = m_aFonts[ *dup ];
                    m_aFonts.erase( *dup );
                    delete pDup;
                }
            }
        }
    }
    return bRet;
}

// -------------------------------------------------------------------------

bool PrintFontManager::isPrivateFontFile( fontID nFont ) const
{
    bool bRet = false;
    int nDirID = -1;
    PrintFont* pFont = getFont( nFont );
    if( pFont )
    {
        switch( pFont->m_eType )
        {
            case fonttype::Type1: nDirID = static_cast< Type1FontFile* >(pFont)->m_nDirectory;break;
            case fonttype::TrueType: nDirID = static_cast< TrueTypeFontFile* >(pFont)->m_nDirectory;break;
            default: break;
        }
    }
    if( nDirID != -1 )
    {
        for( ::std::list< int >::const_iterator it = m_aPrivateFontDirectories.begin(); it != m_aPrivateFontDirectories.end(); ++it )
        {
            if( nDirID == *it )
            {
                bRet = true;
                break;
            }
        }
    }
    return bRet;
}

// -------------------------------------------------------------------------

bool PrintFontManager::getAlternativeFamilyNames( fontID nFont, ::std::list< OUString >& rNames ) const
{
    rNames.clear();

    PrintFont* pFont = getFont( nFont );
    if( pFont && pFont->m_eType == fonttype::TrueType )
    {
        TrueTypeFontFile* pTTFontFile = static_cast< TrueTypeFontFile* >(pFont);
        ByteString aFile( getFontFile( pFont ) );
        TrueTypeFont* pTTFont;
        if( OpenTTFontFile( aFile.GetBuffer(), pTTFontFile->m_nCollectionEntry < 0 ? 0 : pTTFontFile->m_nCollectionEntry, &pTTFont ) == SF_OK )
        {
            NameRecord* pNameRecords = NULL;
            int nNameRecords = GetTTNameRecords( pTTFont, &pNameRecords );
            for( int i = 0; i < nNameRecords; i++ )
            {
                if( pNameRecords[i].nameID != 1 ) // family name
                    continue;

                OUString aFamily( convertTrueTypeName( pNameRecords+i ) );
                if( aFamily.getLength()
                    &&
                    m_pAtoms->getAtom( ATOM_FAMILYNAME, aFamily, sal_True ) != pFont->m_nFamilyName
                    )
                {
                    rNames.push_back( aFamily );
                }
            }

            if( nNameRecords )
                DisposeNameRecords( pNameRecords, nNameRecords );
            CloseTTFont( pTTFont );
        }
    }
    return rNames.begin() != rNames.end();
}

// -------------------------------------------------------------------------

bool PrintFontManager::createFontSubset(
                                        fontID nFont,
                                        const OUString& rOutFile,
                                        sal_Int32* pGlyphIDs,
                                        sal_uInt8* pNewEncoding,
                                        sal_Int32* pWidths,
                                        int nGlyphs,
                                        bool bVertical
                                        )
{
    PrintFont* pFont = getFont( nFont );
    if( !pFont || pFont->m_eType != fonttype::TrueType )
        return false;

    OUString aSysPath;
    if( osl_File_E_None != osl_getSystemPathFromFileURL( rOutFile.pData, &aSysPath.pData ) )
        return false;

    rtl_TextEncoding aEncoding = osl_getThreadTextEncoding();
    ByteString aFromFile = getFontFile( pFont );
    ByteString aToFile( OUStringToOString( aSysPath, aEncoding ) );

    sal_uInt8  pEnc[256];
    sal_uInt16 pGID[256];
    sal_uInt8  pOldIndex[256];

    memset( pEnc, 0, sizeof( pEnc ) );
    memset( pGID, 0, sizeof( pGID ) );
    memset( pOldIndex, 0, sizeof( pOldIndex ) );
    int nChar = 1;
    int i;
    for( i = 0; i < nGlyphs; i++ )
    {
        if( pNewEncoding[i] == 0 )
        {
            pOldIndex[ 0 ] = i;
        }
        else
        {
            DBG_ASSERT( !(pGlyphIDs[i] & 0x007f0000), "overlong glyph id" );
            DBG_ASSERT( (int)pNewEncoding[i] < nGlyphs, "encoding wrong" );
            DBG_ASSERT( pEnc[pNewEncoding[i]] == 0 && pGID[pNewEncoding[i]] == 0, "duplicate encoded glyph" );
            pEnc[ pNewEncoding[i] ] = pNewEncoding[i];
            pGID[ pNewEncoding[i] ] = (sal_uInt16)pGlyphIDs[ i ];
            pOldIndex[ pNewEncoding[i] ] = i;
            nChar++;
        }
    }
    nGlyphs = nChar; // either input value or increased by one

    if( nGlyphs > 256 )
        return false;

    TrueTypeFont* pTTFont = NULL;
    TrueTypeFontFile* pTTFontFile = static_cast< TrueTypeFontFile* >(pFont);
    if( OpenTTFontFile( aFromFile.GetBuffer(), pTTFontFile->m_nCollectionEntry < 0 ? 0 : pTTFontFile->m_nCollectionEntry, &pTTFont ) != SF_OK )
        return false;

    TTSimpleGlyphMetrics* pMetrics = GetTTSimpleGlyphMetrics( pTTFont,
                                                              pGID,
                                                              nGlyphs,
                                                              bVertical ? 1 : 0 );
    if( pMetrics )
    {
        for( i = 0; i < nGlyphs; i++ )
            pWidths[pOldIndex[i]] = pMetrics[i].adv;
        free( pMetrics );
    }
    else
    {
        CloseTTFont( pTTFont );
        return false;
    }

    bool bSuccess = ( SF_OK == CreateTTFromTTGlyphs( pTTFont,
                                                     aToFile.GetBuffer(),
                                                     pGID,
                                                     pEnc,
                                                     nGlyphs,
                                                     0,
                                                     NULL,
                                                     0 ) );
    CloseTTFont( pTTFont );

    return bSuccess;
}

void PrintFontManager::getGlyphWidths( fontID nFont,
                                       bool bVertical,
                                       std::vector< sal_Int32 >& rWidths,
                                       std::map< sal_Unicode, sal_uInt32 >& rUnicodeEnc )
{
    PrintFont* pFont = getFont( nFont );
    if( !pFont ||
        (pFont->m_eType != fonttype::TrueType && pFont->m_eType != fonttype::Type1) )
        return;
    if( pFont->m_eType == fonttype::TrueType )
    {
        TrueTypeFont* pTTFont = NULL;
        TrueTypeFontFile* pTTFontFile = static_cast< TrueTypeFontFile* >(pFont);
        ByteString aFromFile = getFontFile( pFont );
        if( OpenTTFontFile( aFromFile.GetBuffer(), pTTFontFile->m_nCollectionEntry < 0 ? 0 : pTTFontFile->m_nCollectionEntry, &pTTFont ) != SF_OK )
            return;
        int nGlyphs = GetTTGlyphCount( pTTFont );
        if( nGlyphs > 0 )
        {
            rWidths.resize(nGlyphs);
            std::vector<sal_uInt16> aGlyphIds(nGlyphs);
            for( int i = 0; i < nGlyphs; i++ )
                aGlyphIds[i] = sal_uInt16(i);
            TTSimpleGlyphMetrics* pMetrics = GetTTSimpleGlyphMetrics( pTTFont,
                                                                      &aGlyphIds[0],
                                                                      nGlyphs,
                                                                      bVertical ? 1 : 0 );
            if( pMetrics )
            {
                for( int i = 0; i< nGlyphs; i++ )
                    rWidths[i] = pMetrics[i].adv;
                free( pMetrics );
                rUnicodeEnc.clear();
            }
        }
        CloseTTFont( pTTFont );
    }
    else if( pFont->m_eType == fonttype::Type1 )
    {
        if( ! pFont->m_aEncodingVector.size() )
            pFont->readAfmMetrics( getAfmFile( pFont ), m_pAtoms, true, true );
        if( pFont->m_pMetrics )
        {
            rUnicodeEnc.clear();
            rWidths.clear();
            rWidths.reserve( pFont->m_pMetrics->m_aMetrics.size() );
            for( std::hash_map< int, CharacterMetric >::const_iterator it =
                 pFont->m_pMetrics->m_aMetrics.begin();
                 it != pFont->m_pMetrics->m_aMetrics.end(); ++it )
            {
                if( (it->first & 0x00010000) == 0 || bVertical )
                {
                    rUnicodeEnc[ sal_Unicode(it->first & 0x0000ffff) ] = sal_uInt32(rWidths.size());
                    rWidths.push_back( it->second.width );
                }
            }
        }
    }
}

// -------------------------------------------------------------------------

const std::map< sal_Unicode, sal_Int32 >* PrintFontManager::getEncodingMap( fontID nFont, const std::map< sal_Unicode, rtl::OString >** pNonEncoded ) const
{
    PrintFont* pFont = getFont( nFont );
    if( !pFont ||
        (pFont->m_eType != fonttype::Type1 && pFont->m_eType != fonttype::Builtin)
        )
        return NULL;

    if( ! pFont->m_aEncodingVector.size() )
        pFont->readAfmMetrics( getAfmFile( pFont ), m_pAtoms, true, true );

    if( pNonEncoded )
        *pNonEncoded = pFont->m_aNonEncoded.size() ? &pFont->m_aNonEncoded : NULL;

    return pFont->m_aEncodingVector.size() ? &pFont->m_aEncodingVector : NULL;
}

// -------------------------------------------------------------------------

std::list< OString > PrintFontManager::getAdobeNameFromUnicode( sal_Unicode aChar ) const
{
    std::pair< std::hash_multimap< sal_Unicode, rtl::OString >::const_iterator,
        std::hash_multimap< sal_Unicode, rtl::OString >::const_iterator > range
        =  m_aUnicodeToAdobename.equal_range( aChar );

    std::list< OString > aRet;
    for( ; range.first != range.second; ++range.first )
        aRet.push_back( range.first->second );

    if( aRet.begin() == aRet.end() && aChar != 0 )
    {
        sal_Char aBuf[8];
        sal_Int32 nChars = snprintf( (char*)aBuf, sizeof( aBuf ), "uni%.4hX", aChar );
        aRet.push_back( OString( aBuf, nChars ) );
    }

    return aRet;
}

// -------------------------------------------------------------------------
std::list< sal_Unicode >  PrintFontManager::getUnicodeFromAdobeName( const rtl::OString& rName ) const
{
    std::pair< std::hash_multimap< rtl::OString, sal_Unicode, rtl::OStringHash >::const_iterator,
        std::hash_multimap< rtl::OString, sal_Unicode, rtl::OStringHash >::const_iterator > range
        =  m_aAdobenameToUnicode.equal_range( rName );

    std::list< sal_Unicode > aRet;
    for( ; range.first != range.second; ++range.first )
        aRet.push_back( range.first->second );

    if( aRet.begin() == aRet.end() )
    {
        if( rName.getLength() == 7 && rName.indexOf( "uni" ) == 0 )
        {
            sal_Unicode aCode = (sal_Unicode)rName.copy( 3 ).toInt32( 16 );
            aRet.push_back( aCode );
        }
    }

    return aRet;
}

// -------------------------------------------------------------------------
namespace
{
    OUString getString( const Any& rAny )
    {
        OUString aStr;
        rAny >>= aStr;
        return aStr;
    }
    bool getBool( const Any& rAny )
    {
        sal_Bool bBool = sal_False;
        rAny >>= bBool;
        return static_cast<bool>(bBool);
    }
    sal_Int32 getInt( const Any& rAny )
    {
        sal_Int32 n = 0;
        rAny >>= n;
        return n;
    }
}
bool PrintFontManager::readOverrideMetrics()
{
    if( ! m_aOverrideFonts.empty() )
        return false;

    Reference< XMultiServiceFactory > xFact( comphelper::getProcessServiceFactory() );
    if( !xFact.is() )
        return false;
    Reference< XMaterialHolder > xMat(
                xFact->createInstance( OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.psprint.CompatMetricOverride" ) ) ),
                UNO_QUERY );
    if( !xMat.is() )
        return false;

    Any aAny( xMat->getMaterial() );
    Sequence< Any > aOverrideFonts;
    if( ! (aAny >>= aOverrideFonts ) )
        return false;
    sal_Int32 nFonts = aOverrideFonts.getLength();
    for( sal_Int32 i = 0; i < nFonts; i++ )
    {
        Sequence< NamedValue > aMetrics;
        if( ! (aOverrideFonts.getConstArray()[i] >>= aMetrics) )
            continue;
        BuiltinFont* pFont = new BuiltinFont();
        pFont->m_nDirectory = 0;
        pFont->m_bUserOverride = false;
        pFont->m_eEmbeddedbitmap = fcstatus::isunset;
        pFont->m_eAntialias = fcstatus::isunset;
        pFont->m_pMetrics = new PrintFontMetrics;
        memset( pFont->m_pMetrics->m_aPages, 0xff, sizeof( pFont->m_pMetrics->m_aPages ) );
        pFont->m_pMetrics->m_bKernPairsQueried = true;
        sal_Int32 nProps = aMetrics.getLength();
        const NamedValue* pProps = aMetrics.getConstArray();
        for( sal_Int32 n = 0; n < nProps; n++ )
        {
            if( pProps[n].Name.equalsAscii( "FamilyName" ) )
                pFont->m_nFamilyName = m_pAtoms->getAtom( ATOM_FAMILYNAME,
                                                          getString(pProps[n].Value),
                                                          sal_True );
            else if( pProps[n].Name.equalsAscii( "PSName" ) )
                pFont->m_nPSName = m_pAtoms->getAtom( ATOM_PSNAME,
                                                      getString(pProps[n].Value),
                                                      sal_True );
            else if( pProps[n].Name.equalsAscii( "StyleName" ) )
                pFont->m_aStyleName = getString(pProps[n].Value);
            else if( pProps[n].Name.equalsAscii( "Italic" ) )
                pFont->m_eItalic = static_cast<italic::type>(getInt(pProps[n].Value));
            else if( pProps[n].Name.equalsAscii( "Width" ) )
                pFont->m_eWidth = static_cast<width::type>(getInt(pProps[n].Value));
            else if( pProps[n].Name.equalsAscii( "Weight" ) )
                pFont->m_eWeight = static_cast<weight::type>(getInt(pProps[n].Value));
            else if( pProps[n].Name.equalsAscii( "Pitch" ) )
                pFont->m_ePitch = static_cast<pitch::type>(getInt(pProps[n].Value));
            else if( pProps[n].Name.equalsAscii( "Encoding" ) )
                pFont->m_aEncoding = static_cast<rtl_TextEncoding>(getInt(pProps[n].Value));
            else if( pProps[n].Name.equalsAscii( "FontEncodingOnly" ) )
                pFont->m_bFontEncodingOnly = getBool(pProps[n].Value);
            else if( pProps[n].Name.equalsAscii( "GlobalMetricXWidth" ) )
                pFont->m_aGlobalMetricX.width = getInt(pProps[n].Value);
            else if( pProps[n].Name.equalsAscii( "GlobalMetricXHeight" ) )
                pFont->m_aGlobalMetricX.height = getInt(pProps[n].Value);
            else if( pProps[n].Name.equalsAscii( "GlobalMetricYWidth" ) )
                pFont->m_aGlobalMetricY.width = getInt(pProps[n].Value);
            else if( pProps[n].Name.equalsAscii( "GlobalMetricYHeight" ) )
                pFont->m_aGlobalMetricY.height = getInt(pProps[n].Value);
            else if( pProps[n].Name.equalsAscii( "Ascend" ) )
                pFont->m_nAscend = getInt(pProps[n].Value);
            else if( pProps[n].Name.equalsAscii( "Descend" ) )
                pFont->m_nDescend = getInt(pProps[n].Value);
            else if( pProps[n].Name.equalsAscii( "Leading" ) )
                pFont->m_nLeading = getInt(pProps[n].Value);
            else if( pProps[n].Name.equalsAscii( "XMin" ) )
                pFont->m_nXMin = getInt(pProps[n].Value);
            else if( pProps[n].Name.equalsAscii( "YMin" ) )
                pFont->m_nYMin = getInt(pProps[n].Value);
            else if( pProps[n].Name.equalsAscii( "XMax" ) )
                pFont->m_nXMax = getInt(pProps[n].Value);
            else if( pProps[n].Name.equalsAscii( "YMax" ) )
                pFont->m_nYMax = getInt(pProps[n].Value);
            else if( pProps[n].Name.equalsAscii( "VerticalSubstitutes" ) )
                pFont->m_bHaveVerticalSubstitutedGlyphs = getBool(pProps[n].Value);
            else if( pProps[n].Name.equalsAscii( "EncodingVector" ) )
            {
                Sequence< NamedValue > aEncoding;
                pProps[n].Value >>= aEncoding;
                sal_Int32 nEnc = aEncoding.getLength();
                const NamedValue* pEnc = aEncoding.getConstArray();
                for( sal_Int32 m = 0; m < nEnc; m++ )
                {
                    sal_Unicode cCode = *pEnc[m].Name.getStr();
                    sal_Int32 nGlyph = getInt(pEnc[m].Value);
                    pFont->m_aEncodingVector[ cCode ] = nGlyph;
                }
            }
            else if( pProps[n].Name.equalsAscii( "NonEncoded" ) )
            {
                Sequence< NamedValue > aEncoding;
                pProps[n].Value >>= aEncoding;
                sal_Int32 nEnc = aEncoding.getLength();
                const NamedValue* pEnc = aEncoding.getConstArray();
                for( sal_Int32 m = 0; m < nEnc; m++ )
                {
                    sal_Unicode cCode = *pEnc[m].Name.getStr();
                    OUString aGlyphName( getString(pEnc[m].Value) );
                    pFont->m_aNonEncoded[ cCode ] = OUStringToOString(aGlyphName,RTL_TEXTENCODING_ASCII_US);
                }
            }
            else if( pProps[n].Name.equalsAscii( "CharacterMetrics" ) )
            {
                // fill pFont->m_pMetrics->m_aMetrics
                // expect triples of int: int -> CharacterMetric.{ width, height }
                Sequence< sal_Int32 > aSeq;
                pProps[n].Value >>= aSeq;
                sal_Int32 nInts = aSeq.getLength();
                const sal_Int32* pInts = aSeq.getConstArray();
                for( sal_Int32 m = 0; m < nInts; m+=3 )
                {
                    pFont->m_pMetrics->m_aMetrics[ pInts[m] ].width = static_cast<short int>(pInts[m+1]);
                    pFont->m_pMetrics->m_aMetrics[ pInts[m] ].height = static_cast<short int>(pInts[m+2]);
                }
            }
            else if( pProps[n].Name.equalsAscii( "XKernPairs" ) )
            {
                // fill pFont->m_pMetrics->m_aXKernPairs
                // expection name: <unicode1><unicode2> value: ((height << 16)| width)
                Sequence< NamedValue > aKern;
                pProps[n].Value >>= aKern;
                KernPair aPair;
                const NamedValue* pVals = aKern.getConstArray();
                int nPairs = aKern.getLength();
                for( int m = 0; m < nPairs; m++ )
                {
                    if( pVals[m].Name.getLength() == 2 )
                    {
                        aPair.first = pVals[m].Name.getStr()[0];
                        aPair.second = pVals[m].Name.getStr()[1];
                        sal_Int32 nKern = getInt( pVals[m].Value );
                        aPair.kern_x = static_cast<short int>(nKern & 0xffff);
                        aPair.kern_y = static_cast<short int>((sal_uInt32(nKern) >> 16) & 0xffff);
                        pFont->m_pMetrics->m_aXKernPairs.push_back( aPair );
                    }
                }
            }
        }
        // sanity check
        if( pFont->m_nPSName                        &&
            pFont->m_nFamilyName                    &&
            ! pFont->m_pMetrics->m_aMetrics.empty() )
        {
            m_aOverrideFonts.push_back( m_nNextFontID );
            m_aFonts[ m_nNextFontID++ ] = pFont;
        }
        else
        {
            DBG_ASSERT( 0, "override font failed" );
            delete pFont;
        }
    }

    return true;
}
