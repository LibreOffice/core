/*************************************************************************
 *
 *  $RCSfile: fontmanager.cxx,v $
 *
 *  $Revision: 1.40 $
 *
 *  last change: $Author: kz $ $Date: 2003-08-25 13:58:44 $
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

#define Window XLIB_Window
#define Time XLIB_Time
#include <X11/Xlib.h>
#undef Window
#undef Time
#include <unistd.h>
#include <sys/stat.h>
#include <dirent.h>
#include <stdlib.h>

#ifndef _OSL_THREAD_H_
#include <osl/thread.h>
#endif
#ifndef _UTL_ATOM_HXX_
#include <unotools/atom.hxx>
#endif
#ifndef _PSPRINT_FONTMANAGER_HXX_
#include <psprint/fontmanager.hxx>
#endif
#ifndef _PSPRINT_FONTCACHE_HXX_
#include <psprint/fontcache.hxx>
#endif
#ifndef _URLOBJ_HXX
#include <tools/urlobj.hxx>
#endif
#ifndef _STREAM_HXX
#include <tools/stream.hxx>
#endif
#ifndef _PSPRINT_HELPER_HXX_
#include <psprint/helper.hxx>
#endif
#ifndef _OSL_FILE_HXX_
#include <osl/file.hxx>
#endif
#ifndef _OSL_PROCESS_H_
#include <osl/process.h>
#endif
#ifndef _PSPRINT_STRHELPER_HXX_
#include <psprint/strhelper.hxx>
#endif
#ifndef _PSPRINT_PPDPARSER_HXX_
#include <psprint/ppdparser.hxx>
#endif
#ifndef _RTL_TENCINFO_H
#include <rtl/tencinfo.h>
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _CONFIG_HXX
#include <tools/config.hxx>
#endif
#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif
#ifndef _RTL_STRBUF_HXX_
#include <rtl/strbuf.hxx>
#endif
#ifndef _TOOLS_INTN_HXX
#include <tools/intn.hxx>
#endif


#include <parseAFM.hxx>
#define NO_LIST
#include <sft.h>
#undef NO_LIST

#if OSL_DEBUG_LEVEL > 1
#include <sys/times.h>
#include <stdio.h>
#endif

#ifdef SOLARIS
#include <alloca.h>
#endif

#include <set>

#include <adobeenc.tab> // get encoding table for AFM metrics

#define PRINTER_METRICDIR "fontmetric"

using namespace utl;
using namespace psp;
using namespace osl;
using namespace rtl;

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
        m_bHaveVerticalSubstitutedGlyphs( false )
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

bool PrintFontManager::Type1FontFile::queryMetricPage( int nPage, MultiAtomProvider* pProvider )
{
    return readAfmMetrics( PrintFontManager::get().getAfmFile( this ), pProvider );
}

// -------------------------------------------------------------------------

bool PrintFontManager::BuiltinFont::queryMetricPage( int nPage, MultiAtomProvider* pProvider )
{
    return readAfmMetrics( PrintFontManager::get().getAfmFile( this ), pProvider );
}

// -------------------------------------------------------------------------

bool PrintFontManager::TrueTypeFontFile::queryMetricPage( int nPage, MultiAtomProvider* pProvider )
{
    bool bSuccess = false;

    ByteString aFile( PrintFontManager::get().getFontFile( this ) );

    TrueTypeFont* pTTFont = NULL;

    if( OpenTTFont( aFile.GetBuffer(), m_nCollectionEntry < 0 ? 0 : m_nCollectionEntry, &pTTFont ) == SF_OK )
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
                for( i = 0; i < pImplTTFont->nkern; i++ )
                {
                    const sal_uInt8* pTable = pImplTTFont->kerntables[i];

                    sal_uInt16 nVersion     = getUInt16BE( pTable );
                    sal_uInt16 nLength      = getUInt16BE( pTable );
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
                            sal_uInt16 nRowWidth    = getUInt16BE( pTable );
                            sal_uInt16 nOfLeft      = getUInt16BE( pTable );
                            sal_uInt16 nOfRight     = getUInt16BE( pTable );
                            sal_uInt16 nOfArray     = getUInt16BE( pTable );
                            const sal_uInt8* pTmp = pSubTable + nOfLeft;
                            sal_uInt16 nFirstLeft   = getUInt16BE( pTmp );
                            sal_uInt16 nLastLeft    = getUInt16BE( pTmp ) + nFirstLeft - 1;
                            pTmp = pSubTable + nOfRight;
                            sal_uInt16 nFirstRight  = getUInt16BE( pTmp );
                            sal_uInt16 nLastRight   = getUInt16BE( pTmp ) + nFirstRight -1;

                            int nPairs = (int)(nLastLeft-nFirstLeft+1)*(int)(nLastRight-nFirstRight+1);
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
#if OSL_DEBUG_LEVEL > 1
            fprintf( stderr, "found %d/%d kern pairs for %s\n",
                     m_pMetrics->m_aXKernPairs.size(),
                     m_pMetrics->m_aYKernPairs.size(),
                     OUStringToOString( pProvider->getString( ATOM_FAMILYNAME, m_nFamilyName ), RTL_TEXTENCODING_MS_1252 ).getStr() );
#endif
        }

        CloseTTFont( pTTFont );
        bSuccess = true;
    }
    return bSuccess;
}

// -------------------------------------------------------------------------

bool PrintFontManager::PrintFont::readAfmMetrics( const OString& rFileName, MultiAtomProvider* pProvider, bool bFillEncodingvector )
{
    PrintFontManager& rManager( PrintFontManager::get() );

    int i;
    FILE* fp = fopen( rFileName.getStr(), "r" );
    if( ! fp )
        return false;
    FontInfo* pInfo = NULL;
    int nResult = parseFile( fp, &pInfo, P_ALL );
    fclose( fp );
    if( ! pInfo || ! pInfo->numOfChars )
    {
        if( pInfo )
            freeFontInfo( pInfo );
        return false;
    }

    m_aEncodingVector.clear();
    // fill in global info

    // family name (if not already set)
    OUString aFamily;
    if( ! m_nFamilyName )
    {
        aFamily = OUString( OStringToOUString( pInfo->gfi->familyName, RTL_TEXTENCODING_ISO_8859_1 ) );
        if( ! aFamily.getLength() )
        {
            aFamily = OStringToOUString( pInfo->gfi->fontName, RTL_TEXTENCODING_ISO_8859_1 );
            sal_Int32 nIndex  = 0;
            aFamily = aFamily.getToken( 0, '-', nIndex );
        }
        m_nFamilyName = pProvider->getAtom( ATOM_FAMILYNAME, aFamily, sal_True );
    }
    else
        aFamily = pProvider->getString( ATOM_FAMILYNAME, m_nFamilyName );

    // PSName
    OUString aPSName( OStringToOUString( pInfo->gfi->fontName, RTL_TEXTENCODING_ISO_8859_1 ) );
    m_nPSName = pProvider->getAtom( ATOM_PSNAME, aPSName, sal_True );

    // italic
    if( pInfo->gfi->italicAngle > 0 )
        m_eItalic = italic::Oblique;
    else if( pInfo->gfi->italicAngle < 0 )
        m_eItalic = italic::Italic;
    else
        m_eItalic = italic::Upright;

    // weight
    m_eWeight = parseWeight( ByteString( pInfo->gfi->weight ).ToLowerAscii() );

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
                RTL_TEXTENCODING_MS_1252 : RTL_TEXTENCODING_SYMBOL;
    }
    else if( m_aEncoding == RTL_TEXTENCODING_DONTKNOW )
        m_aEncoding = RTL_TEXTENCODING_MS_1252;

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

        for( int enc = 0; enc < sizeof( aEncs )/sizeof(aEncs[0]) && m_aEncoding == RTL_TEXTENCODING_DONTKNOW; enc++ )
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
    // mark all pages as queried
    memset( m_pMetrics->m_aPages, 0xff, sizeof( m_pMetrics->m_aPages ) );

    m_aGlobalMetricX.width = m_aGlobalMetricY.width =
        pInfo->gfi->charwidth ? pInfo->gfi->charwidth : pInfo->gfi->fontBBox.urx;
    m_aGlobalMetricX.height = m_aGlobalMetricY.height =
        pInfo->gfi->capHeight ? pInfo->gfi->capHeight : pInfo->gfi->fontBBox.ury;

    m_nXMin = pInfo->gfi->fontBBox.llx;
    m_nYMin = pInfo->gfi->fontBBox.lly;
    m_nXMax = pInfo->gfi->fontBBox.urx;
    m_nYMax = pInfo->gfi->fontBBox.ury;

    // fill in character metrics

    // first transform the character codes to unicode
    // note: this only works with single byte encodings
    sal_Unicode* pUnicodes = (sal_Unicode*)alloca( pInfo->numOfChars * sizeof(sal_Unicode));
    CharMetricInfo* pChar = pInfo->cmi;

    for( i = 0; i < pInfo->numOfChars; i++, pChar++ )
    {
        if( pChar->code != -1 )
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
        else if( nAdobeEncoding == 1 || nAdobeEncoding == 2 )
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
        m_pAtoms( new MultiAtomProvider() ),
        m_nNextFontID( 1 ),
        m_nNextDirAtom( 1 ),
        m_pFontCache( NULL )
{
    for( int i = 0; i < sizeof( aAdobeCodes )/sizeof( aAdobeCodes[0] ); i++ )
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

int PrintFontManager::addFontFile( const ::rtl::OString& rFileName, int nFaceNum )
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
        if( analyzeFontFile( nDirID, aName, false, ::std::list<OString>(), aNewFonts ) )
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

bool PrintFontManager::analyzeFontFile( int nDirID, const OString& rFontFile, bool bReadFile, const ::std::list<OString>& rXLFDs, ::std::list< PrintFontManager::PrintFont* >& rNewFonts ) const
{
    rNewFonts.clear();
    rtl_TextEncoding aEncoding = osl_getThreadTextEncoding();

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

        ByteString aName( rFontFile );
        aName.Erase( aName.Len()-4 );
        aName.Append( ".afm" );

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

            if( rXLFDs.size() )
                getFontAttributesFromXLFD( pFont, rXLFDs.front() );
            else if( ! pFont->readAfmMetrics( getAfmFile( pFont ), m_pAtoms ) )
            {
                delete pFont;
                pFont = NULL;
            }
            if( pFont )
                rNewFonts.push_back( pFont );
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
        if( pFont->readAfmMetrics( aFilePath, m_pAtoms ) )
            rNewFonts.push_back( pFont );
        else
            delete pFont;
    }
    else if( aExt.EqualsIgnoreCaseAscii( "ttf" ) )
    {
        TrueTypeFontFile* pFont     = new TrueTypeFontFile();
        pFont->m_nDirectory         = nDirID;
        pFont->m_aFontFile          = rFontFile;
        pFont->m_nCollectionEntry   = -1;

        if( rXLFDs.size() )
            getFontAttributesFromXLFD( pFont, rXLFDs.front() );
        // need to read the font anyway to get aliases
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
            ::std::list<OString>::const_iterator xlfd_it = rXLFDs.begin();
            for( int i = 0; i < nLength; i++ )
            {
                OString aXLFD;
                if( xlfd_it != rXLFDs.end() )
                {
                    aXLFD = *xlfd_it;
                    ++xlfd_it;
                }
#if OSL_DEBUG_LEVEL > 1
                fprintf( stderr, "   XLFD=\"%s\"\n", aXLFD.getStr() );
#endif
                TrueTypeFontFile* pFont     = new TrueTypeFontFile();
                pFont->m_nDirectory         = nDirID;
                pFont->m_aFontFile          = rFontFile;
                pFont->m_nCollectionEntry   = i;
                if( aXLFD.getLength() )
                    getFontAttributesFromXLFD( pFont, aXLFD );
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
    ::std::hash_map< int, PrintFont* >::const_iterator it;
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
            ::std::hash_map< int, PrintFont* >::const_iterator it = m_aFonts.find( *font_it );
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

void PrintFontManager::getFontAttributesFromXLFD( PrintFont* pFont, const ByteString& rXLFD ) const
{
    if( rXLFD.GetTokenCount( '-' ) != 15 )
        return;

    ByteString aFamilyXLFD( WhitespaceToSpace( rXLFD.GetToken( 2, '-' ) ) );
    int nTokens = aFamilyXLFD.GetTokenCount( ' ' );
    ByteString aFamilyName;
    for( int nToken = 0; nToken < nTokens; nToken++ )
    {
        ByteString aToken( aFamilyXLFD.GetToken( nToken, ' ' ) );
        ByteString aNewToken( aToken.GetChar( 0 ) );
        aNewToken.ToUpperAscii();
        aNewToken += aToken.Copy( 1 );
        if( nToken > 0 )
            aFamilyName.Append( ' ' );
        aFamilyName += aNewToken;
    }

    ByteString aAddStyle( rXLFD.GetToken( 6, '-' ) );
    pFont->m_nFamilyName =
        m_pAtoms->getAtom( ATOM_FAMILYNAME,
                           String( aFamilyName, aAddStyle.Search( "utf8" ) != STRING_NOTFOUND ? RTL_TEXTENCODING_UTF8 : RTL_TEXTENCODING_ISO_8859_1 ),
                           sal_True );
    ByteString aToken;

    // evaluate weight
    aToken = rXLFD.GetToken( 3,  '-' ).ToLowerAscii();
    pFont->m_eWeight = parseWeight( aToken );

    // evaluate slant
    aToken = rXLFD.GetToken( 4, '-' );
    pFont->m_eItalic = parseItalic( aToken );

    // evaluate width
    aToken = rXLFD.GetToken( 5,  '-' ).ToLowerAscii();
    pFont->m_eWidth = parseWidth( aToken );

    // evaluate pitch
    aToken = rXLFD.GetToken( 11,  '-' ).ToLowerAscii();
    if( aToken.Equals( "c" ) || aToken.Equals( "m" ) )
        pFont->m_ePitch = pitch::Fixed;
    else
        pFont->m_ePitch = pitch::Variable;

    // get encoding
    aToken = rXLFD.GetToken( 6, '-' ).ToLowerAscii();
    if( aToken.Search( "symbol" ) != STRING_NOTFOUND )
        pFont->m_aEncoding = RTL_TEXTENCODING_SYMBOL;
    else
    {
        aToken =WhitespaceToSpace(  rXLFD.GetToken( 14 ) );
        if( aToken.EqualsIgnoreCaseAscii( "symbol" ) )
            pFont->m_aEncoding = RTL_TEXTENCODING_SYMBOL;
        else
        {
            aToken = rXLFD.GetToken( 13, '-' );
            aToken += '-';
            aToken += WhitespaceToSpace( rXLFD.GetToken( 14, '-' ) );
            pFont->m_aEncoding = rtl_getTextEncodingFromUnixCharset( aToken.GetBuffer() );
        }
    }

    // handle iso8859-1 as ms1252 to fill the "gap" starting at 0x80
    if( pFont->m_aEncoding == RTL_TEXTENCODING_ISO_8859_1 )
        pFont->m_aEncoding = RTL_TEXTENCODING_MS_1252;

    switch( pFont->m_eType )
    {
        case fonttype::Type1:
            static_cast<Type1FontFile*>(pFont)->m_aXLFD = rXLFD;
            break;
        case fonttype::TrueType:
            static_cast<TrueTypeFontFile*>(pFont)->m_aXLFD = rXLFD;
            break;
    }
}

// -------------------------------------------------------------------------

ByteString PrintFontManager::getXLFD( PrintFont* pFont ) const
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

    ByteString aXLFD( "-misc-" );
    ByteString aFamily( String( m_pAtoms->getString( ATOM_FAMILYNAME, pFont->m_nFamilyName ) ), RTL_TEXTENCODING_UTF8 );
    aFamily.SearchAndReplaceAll( '-',' ' );
    aFamily.SearchAndReplaceAll( '?',' ' );
    aFamily.SearchAndReplaceAll( '*',' ' );
    aXLFD += aFamily;
    aXLFD += '-';
    switch( pFont->m_eWeight )
    {
        case weight::Thin:              aXLFD += "thin";break;
        case weight::UltraLight:        aXLFD += "ultralight";break;
        case weight::Light:         aXLFD += "light";break;
        case weight::SemiLight:     aXLFD += "semilight";break;
        case weight::Normal:            aXLFD += "normal";break;
        case weight::Medium:            aXLFD += "medium";break;
        case weight::SemiBold:          aXLFD += "semibold";break;
        case weight::Bold:              aXLFD += "bold";break;
        case weight::UltraBold:     aXLFD += "ultrabold";break;
        case weight::Black:         aXLFD += "black";break;
    }
    aXLFD += '-';
    switch( pFont->m_eItalic )
    {
        case italic::Upright:           aXLFD += 'r';break;
        case italic::Oblique:           aXLFD += 'o';break;
        case italic::Italic:            aXLFD += 'i';break;
    }
    aXLFD += '-';
    switch( pFont->m_eWidth )
    {
        case width::UltraCondensed: aXLFD += "ultracondensed";break;
        case width::ExtraCondensed: aXLFD += "extracondensed";break;
        case width::Condensed:          aXLFD += "condensed";break;
        case width::SemiCondensed:      aXLFD += "semicondensed";break;
        case width::Normal:         aXLFD += "normal";break;
        case width::SemiExpanded:       aXLFD += "semiexpanded";break;
        case width::Expanded:           aXLFD += "expanded";break;
        case width::ExtraExpanded:      aXLFD += "extraexpanded";break;
        case width::UltraExpanded:      aXLFD += "ultraexpanded";break;
    }
    aXLFD += "-utf8-0-0-0-0-";
    aXLFD += pFont->m_ePitch == pitch::Fixed ? "m" : "p";
    aXLFD += "-0-";
    aXLFD += rtl_getBestUnixCharsetFromTextEncoding( pFont->m_aEncoding );

    return aXLFD;
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
        LanguageType aLang = GetSystemLanguage();
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
    if( OpenTTFont( aFile.GetBuffer(), pTTFontFile->m_nCollectionEntry < 0 ? 0 : pTTFontFile->m_nCollectionEntry, &pTTFont ) == SF_OK )
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
                // poor font does not have a family name
                // name it to file name minus ".tt{f|c}"
                pFont->m_nFamilyName = m_pAtoms->getAtom( ATOM_FAMILYNAME, OStringToOUString( pTTFontFile->m_aFontFile.copy( 0, pTTFontFile->m_aFontFile.getLength()-4 ), aEncoding ), sal_True );
        }
        pFont->m_aAliases.clear();
        for( ::std::list< OUString >::iterator it = aNames.begin(); it != aNames.end(); ++it )
        {
            if( it->getLength() )
            {
                int nAlias = m_pAtoms->getAtom( ATOM_FAMILYNAME, *it, sal_True );
                if( nAlias != pFont->m_nFamilyName )
                    pFont->m_aAliases.push_back( nAlias );
            }
        }


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

static void normPath( ByteString& rPath )
{
    char buf[PATH_MAX];

    // double slashes and slash at end are probably
    // removed by realpath anyway, but since this runs
    // on many different platforms let's play it safe
    while( rPath.SearchAndReplace( "//", "/" ) != STRING_NOTFOUND )
        ;
    if( rPath.Len() > 0 && rPath.GetChar( rPath.Len()-1 ) == '/' )
        rPath.Erase( rPath.Len()-1 );

    if( realpath( rPath.GetBuffer(), buf ) )
        rPath = buf;
}

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

    for( int i = 0; i < sizeof(pCommands)/sizeof(pCommands[0]); i++ )
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

void PrintFontManager::initialize( void* pInitDisplay )
{
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

    // part one - look for downloadable fonts
    rtl_TextEncoding aEncoding = osl_getThreadTextEncoding();
    const ::rtl::OUString &rSalPrivatePath = psp::getFontPath();

    // search for the fonts in SAL_PRIVATE_FONTPATH first; those are
    // the TrueType fonts installed with the office
    if( rSalPrivatePath.getLength() )
    {
        ByteString aPath = ::rtl::OUStringToOString( rSalPrivatePath, aEncoding );
        int nTokens = aPath.GetTokenCount( ';' );
        for( int i = 0; i < nTokens; i++ )
        {
            ByteString aToken( aPath.GetToken( i, ';' ) );
            normPath( aToken );
            m_aFontDirectories.push_back( aToken );
            m_aPrivateFontDirectories.push_back( getDirectoryAtom( aToken, true ) );
        }
    }

    Display *pDisplay = (Display*)pInitDisplay;

    if( ! pDisplay )
        pDisplay = XOpenDisplay( NULL );

    if( pDisplay )
    {
        // get font paths to look for fonts
        int nPaths = 0, i;
        char** pPaths = XGetFontPath( pDisplay, &nPaths );

        int nPos = 0;
        bool bServerDirs = false;
        for( i = 0; i < nPaths; i++ )
        {
            ByteString aPath( pPaths[i] );
            if( ! bServerDirs
                && ( nPos = aPath.Search( ':' ) ) != STRING_NOTFOUND
                && ( !aPath.Equals( ":unscaled", nPos, 9 ) ) )
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
    m_aFontDirectories.push_back( "/usr/openwin/lib/X11/fonts/Type1" );
    m_aFontDirectories.push_back( "/usr/openwin/lib/X11/fonts/Type1/sun" );
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
            normPath( aLine );
            // try to avoid bad fonts in some cases
            static bool bAvoid = (strncasecmp( lang, "ar", 2 ) == 0) || (strncasecmp( lang, "iw", 2 ) == 0) || (strncasecmp( lang, "hi", 2 ) == 0);
            if( bAvoid && aLine.Search( "iso_8859" ) != STRING_NOTFOUND )
                continue;
            m_aFontDirectories.push_back( aLine );
        }
    }

#endif /* SOLARIS */

    // search for font files in each path
    ::std::list< OString >::iterator dir_it;
    for( dir_it = m_aFontDirectories.begin(); dir_it != m_aFontDirectories.end(); ++dir_it )
    {
        OString aPath( *dir_it );
        // there may be ":unscaled" directories (see XFree86)
        // it should be safe to ignore them since they should not
        // contain any of our recognizeable fonts

        // ask the font cache whether it handles this directory
        std::list< PrintFont* > aCacheFonts;
        if( m_pFontCache->listDirectory( aPath, aCacheFonts ) )
        {
#ifdef DEBUG
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
#ifdef DEBUG
                if( (*it)->m_eType == fonttype::Builtin )
                    nBuiltinFonts++;
                nCached++;
                fprintf( stderr, "adding cached font %d: \"%s\" from %s\n", aFont,
                         OUStringToOString( getFontFamily( aFont ), RTL_TEXTENCODING_MS_1252 ).getStr(),
                         getFontFileSysPath( aFont ).getStr() );
#endif
            }
            continue;
        }

        DIR* pDIR = opendir( aPath.getStr() );
        struct dirent* pEntry = (struct dirent*)aDirEntBuffer;
        if( pDIR )
        {
            // read fonts.dir if possible
            ::std::hash_map< OString, ::std::list<OString>, OStringHash > aFontsDir;
            int nDirID = getDirectoryAtom( aPath, true );
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
                        if( analyzeFontFile( nDirID, aFileName, aXLFDs.size() ? false : true, aXLFDs, aNewFonts ) )
                        {
                            bool bUpdateFont = aXLFDs.size() < aNewFonts.size();
                            for( ::std::list< PrintFont* >::iterator it = aNewFonts.begin(); it != aNewFonts.end(); ++it )
                            {
                                fontID aFont = m_nNextFontID++;
                                m_aFonts[ aFont ] = *it;
                                m_aFontFileToFontID[ aFileName ].insert( aFont );
                                m_pFontCache->updateFontCacheEntry( *it, false );
                                nDirFonts++;
                                if( bUpdateFont && isPrivateFontFile( aFont ) )
                                    changeFontProperties( aFont, OStringToOUString( getXLFD( *it ), RTL_TEXTENCODING_UTF8 ) );
#if OSL_DEBUG_LEVEL > 1
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
#ifdef DEBUG
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
#ifdef DEBUG
                if( (*it)->m_eType == fonttype::Builtin )
                    nBuiltinFonts++;
                nCached++;
                fprintf( stderr, "adding cached font %d: \"%s\" from %s\n", aFont,
                         OUStringToOString( getFontFamily( aFont ), RTL_TEXTENCODING_MS_1252 ).getStr(),
                         getFontFileSysPath( aFont ).getStr() );
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

                        analyzeFontFile( nDirID, aFileName, true, aEmptyFontsDir, aNewFonts );
                        for( ::std::list< PrintFont* >::iterator it = aNewFonts.begin(); it != aNewFonts.end(); ++it )
                        {
                            if( findFontBuiltinID( (*it)->m_nPSName ) == 0 )
                            {
                                m_aFontFileToFontID[ aFileName ].insert( m_nNextFontID );
                                m_aFonts[ m_nNextFontID++ ] = *it;
                                m_pFontCache->updateFontCacheEntry( *it, false );
#if OSL_DEBUG_LEVEL > 1
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

void PrintFontManager::getFontList( ::std::list< fontID >& rFontIDs, const PPDParser* pParser ) const
{
    rFontIDs.clear();
    ::std::hash_map< fontID, PrintFont* >::const_iterator it;
    ::std::list< PrintFont* > aBuiltinFonts;

    for( it = m_aFonts.begin(); it != m_aFonts.end(); ++it )
    {
        if( pParser && it->second->m_eType == fonttype::Builtin )
        {
            int nFonts = pParser->getFonts();
            String aPSName = m_pAtoms->getString( ATOM_PSNAME, it->second->m_nPSName );
            for( int j = 0; j < nFonts; j++ )
            {
                if( aPSName.Equals( pParser->getFont( j ) ) )
                {
                    rFontIDs.push_back( it->first );
                    aBuiltinFonts.push_back( it->second );
                    break;
                }
            }
        }
        else
            rFontIDs.push_back( it->first );
    }

    if( pParser )
    {
        // remove doubles for builtins
        ::std::list< fontID >::iterator font_it;
        ::std::list< fontID >::iterator temp_it;
        font_it = rFontIDs.begin();
        while( font_it != rFontIDs.end() )
        {
            temp_it = font_it;
            ++temp_it;
            PrintFont* pFont = getFont( *font_it );
            if( pFont->m_eType != fonttype::Builtin )
            {
                const OUString& rFontFamily( m_pAtoms->getString( ATOM_FAMILYNAME, pFont->m_nFamilyName ) );

                for( ::std::list< PrintFont* >::const_iterator bit = aBuiltinFonts.begin();
                     bit != aBuiltinFonts.end(); ++bit )
                {
                    if( ! equalItalic  (pFont->m_eItalic, (*bit)->m_eItalic) )
                        continue;
                    if( ! equalWeight  (pFont->m_eWeight, (*bit)->m_eWeight) )
                        continue;
                    if( ! equalPitch   (pFont->m_ePitch,  (*bit)->m_ePitch) )
                        continue;
                    if( ! equalEncoding(pFont->m_aEncoding, (*bit)->m_aEncoding) )
                        continue;
                    const OUString& rBuiltinFamily( m_pAtoms->getString( ATOM_FAMILYNAME, (*bit)->m_nFamilyName ) );
                    if( rFontFamily.equalsIgnoreAsciiCase( rBuiltinFamily ) )
                    {
                        // remove double
                        rFontIDs.erase( font_it );
                        break;
                    }
                }
            }
            font_it = temp_it;
        }
    }
}

// -------------------------------------------------------------------------

void PrintFontManager::fillPrintFontInfo( PrintFont* pFont, FastPrintFontInfo& rInfo ) const
{
    ::std::hash_map< int, family::type >::const_iterator style_it =
          m_aFamilyTypes.find( pFont->m_nFamilyName );
    rInfo.m_eType           = pFont->m_eType;
    rInfo.m_aFamilyName     = m_pAtoms->getString( ATOM_FAMILYNAME, pFont->m_nFamilyName );
    rInfo.m_eFamilyStyle    = style_it != m_aFamilyTypes.end() ? style_it->second : family::Unknown;
    rInfo.m_eItalic         = pFont->m_eItalic;
    rInfo.m_eWidth          = pFont->m_eWidth;
    rInfo.m_eWeight         = pFont->m_eWeight;
    rInfo.m_ePitch          = pFont->m_ePitch;
    rInfo.m_aEncoding       = pFont->m_aEncoding;
    rInfo.m_aAliases.clear();
    for( ::std::list< int >::iterator it = pFont->m_aAliases.begin(); it != pFont->m_aAliases.end(); ++it )
        rInfo.m_aAliases.push_back( m_pAtoms->getString( ATOM_FAMILYNAME, *it ) );
}

// -------------------------------------------------------------------------

void PrintFontManager::fillPrintFontInfo( PrintFont* pFont, PrintFontInfo& rInfo ) const
{
    if( pFont->m_nAscend == 0 && pFont->m_nDescend == 0 )
    {
        // might be a truetype font not analyzed or type1 without metrics read
        if( pFont->m_eType == fonttype::Type1 )
            pFont->readAfmMetrics( getAfmFile( pFont ), m_pAtoms );
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

void PrintFontManager::getFontListWithInfo( ::std::list< PrintFontInfo >& rFonts, const PPDParser* pParser ) const
{
    rFonts.clear();
    ::std::list< fontID > aFontList;
    getFontList( aFontList, pParser );

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

void PrintFontManager::getFontListWithFastInfo( ::std::list< FastPrintFontInfo >& rFonts, const PPDParser* pParser ) const
{
    rFonts.clear();
    ::std::list< fontID > aFontList;
    getFontList( aFontList, pParser );

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
            if( pFont->m_eType == fonttype::Type1 )
                pFont->readAfmMetrics( getAfmFile( pFont ), m_pAtoms );
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
                if( OpenTTFont( aFile.GetBuffer(), pTTFontFile->m_nCollectionEntry < 0 ? 0 : pTTFontFile->m_nCollectionEntry, &pTTFont ) == SF_OK )
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

    if( pFont->m_nAscend == 0 && pFont->m_nDescend == 0 )
    {
        // might be a font not yet analyzed
        if( pFont->m_eType == fonttype::Type1 )
            pFont->readAfmMetrics( getAfmFile( pFont ), m_pAtoms );
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

    if( pFont->m_nAscend == 0 && pFont->m_nDescend == 0 )
    {
        // might be a font not yet analyzed
        if( pFont->m_eType == fonttype::Type1 )
            pFont->readAfmMetrics( getAfmFile( pFont ), m_pAtoms );
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
            if( it != pFont->m_pMetrics->m_aMetrics.end() )
                pArray[ code - minCharacter ] = it->second;
        }
    }
    return true;
}

// -------------------------------------------------------------------------

static bool createPath( const ByteString& rPath )
{
    bool bSuccess = false;
#if OSL_DEBUG_LEVEL > 1
    fprintf( stderr, "createPath( %s )\n", rPath.GetBuffer() );
#endif

    if( access( rPath.GetBuffer(), F_OK ) )
    {
        int nPos = rPath.SearchBackward( '/' );
        if( nPos != STRING_NOTFOUND )
            while( nPos > 0 && rPath.GetChar( nPos ) == '/' )
                nPos--;

        if( nPos != STRING_NOTFOUND && nPos != 0 && createPath( rPath.Copy( 0, nPos+1 ) ) )
        {
#if OSL_DEBUG_LEVEL > 1
            fprintf( stderr, "mkdir ", rPath.GetBuffer() );
#endif
            bSuccess = mkdir( rPath.GetBuffer(), 0777 ) ? false : true;
#if OSL_DEBUG_LEVEL > 1
            fprintf( stderr, "%s\n", bSuccess ? "succeeded" : "failed" );
#endif
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
    SvFileStream aFontsDir;
    int nDirID;
    INetURLObject aDir;
    for( ::std::list< int >::const_iterator dir_it = m_aPrivateFontDirectories.begin();
         ! ( aFontsDir.IsOpen() && aFontsDir.IsWritable() ) && dir_it != m_aPrivateFontDirectories.end(); ++dir_it )
    {
        // there must be a writable fonts.dir in that directory
        aDir = INetURLObject( OStringToOUString( getDirectory( *dir_it ), aEncoding ), INET_PROT_FILE, INetURLObject::ENCODE_ALL );
        nDirID = *dir_it;
        INetURLObject aFDir( aDir );
        ByteString aDirPath( aFDir.PathToFileName(), aEncoding );
        if( createPath( aDirPath ) )
        {
            aFDir.Append( String( RTL_CONSTASCII_USTRINGPARAM( "fonts.dir" ) ) );
            aFontsDir.Open( aFDir.PathToFileName(), STREAM_READ | STREAM_WRITE );
        }
    }
    if( aFontsDir.IsOpen() )
    {
        aFontsDir.SetLineDelimiter( LINEEND_LF );
        // we have a suitable path
        // read the fonts.dir
        ::std::list< ByteString > aLines;
        ::std::list< ByteString >::iterator line_it;
        ByteString aLine;
        while( ! aFontsDir.IsEof() )
        {
            aFontsDir.ReadLine( aLine );
            if( aLine.Len() )
                aLines.push_back( aLine );
        }
        if( aLines.begin() != aLines.end() )
            aLines.pop_front(); // not interested in the number of lines

        // copy the font files and add them to fonts.dir
        // do not overwrite existing files unless user wants it that way
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

            if( ! access( ByteString( aTo.PathToFileName(), aEncoding ).GetBuffer(), F_OK ) )
            {
                if( ! ( pCallback ? pCallback->queryOverwriteFile( aTo.PathToFileName() ) : false ) )
                    continue;
            }
            // look for afm if necessary
            OUString aAfmCopied;
            FileBase::RC nError;
            if( aFrom.getExtension().EqualsIgnoreCaseAscii( "pfa" ) ||
                aFrom.getExtension().EqualsIgnoreCaseAscii( "pfb" ) )
            {
                INetURLObject aFromAfm( aFrom );
                aFromAfm.setExtension( String( RTL_CONSTASCII_USTRINGPARAM( "afm" ) ) );
                if( access( ByteString( aFromAfm.PathToFileName(), aEncoding ).GetBuffer(), F_OK ) )
                {
                    aFromAfm.setExtension( String( RTL_CONSTASCII_USTRINGPARAM( "AFM" ) ) );
                    if( access( ByteString( aFromAfm.PathToFileName(), aEncoding ).GetBuffer(), F_OK ) )
                    {
                        aFromAfm.removeSegment();
                        aFromAfm.Append( String( RTL_CONSTASCII_USTRINGPARAM( "afm" ) ) );
                        aFromAfm.Append( aTo.GetName() );
                        aFromAfm.setExtension( String( RTL_CONSTASCII_USTRINGPARAM( "afm" ) ) );
                        if( access( ByteString( aFromAfm.PathToFileName(), aEncoding ).GetBuffer(), F_OK ) )
                        {
                            aFromAfm.setExtension( String( RTL_CONSTASCII_USTRINGPARAM( "AFM" ) ) );
                            if( access( ByteString( aFromAfm.PathToFileName(), aEncoding ).GetBuffer(), F_OK ) )
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
                aToAfm.setExtension( String( RTL_CONSTASCII_USTRINGPARAM( "afm" ) ) );
                OUString aFromPath, aToPath;
                if( bLinkOnly )
                {
                    ByteString aFromPath( aFromAfm.PathToFileName(), aEncoding );
                    ByteString aToPath( aToAfm.PathToFileName(), aEncoding );
                    nError = (FileBase::RC)symlink( aFromPath.GetBuffer(), aToPath.GetBuffer() );
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
                ByteString aFromPath( aFrom.PathToFileName(), aEncoding );
                ByteString aToPath( aTo.PathToFileName(), aEncoding );
                nError = (FileBase::RC)symlink( aFromPath.GetBuffer(), aToPath.GetBuffer() );
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
            if( analyzeFontFile( nDirID, OUStringToOString( aTo.GetName(), aEncoding ), true, ::std::list<OString>(), aNewFonts ) )
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

                    aLine = ByteString( aTo.GetName(), aEncoding );
                    aLine += ' ';
                    aLine += getXLFD( *it );

                    int nTTCnumber = -1;
                    if( (*it)->m_eType == fonttype::TrueType )
                        nTTCnumber = static_cast<TrueTypeFontFile*>(*it)->m_nCollectionEntry;

                    ByteString aFile( aTo.GetName(), aEncoding );
                    for( line_it = aLines.begin(); line_it != aLines.end(); ++line_it )
                    {
                        if( line_it->GetToken( 0, ' ' ).Equals( aFile ) )
                        {
                            if( nTTCnumber <= 0 )
                            {
                                *line_it = aLine;
                                break;
                            }
                            else
                                nTTCnumber--;
                        }
                    }
                    if( line_it == aLines.end() )
                        aLines.push_back( aLine );
                }
            }
        }
        aFontsDir.Seek( STREAM_SEEK_TO_BEGIN );
        aFontsDir.SetStreamSize( 0 );
        aFontsDir.WriteLine( ByteString::CreateFromInt32( aLines.size() ) );
        for( line_it = aLines.begin(); line_it != aLines.end(); ++line_it )
            aFontsDir.WriteLine( *line_it );

        // rehash X font path
        Display* pDisplay = XOpenDisplay( NULL );
        if( pDisplay )
        {
            int nPaths = 0;
            char** pFontPaths = XGetFontPath( pDisplay, &nPaths );
            XSetFontPath( pDisplay, pFontPaths, nPaths );
            if( pFontPaths && nPaths )
                XFreeFontPath( pFontPaths );
            XCloseDisplay( pDisplay );
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
    SvFileStream aFontsDir;
    INetURLObject aDir;
    rtl_TextEncoding aEncoding = osl_getThreadTextEncoding();
    for( ::std::list< OString >::const_iterator dir_it = m_aFontDirectories.begin();
         ! ( aFontsDir.IsOpen() && aFontsDir.IsWritable() ) && dir_it != m_aFontDirectories.end(); ++dir_it )
    {
        // there must be a writable fonts.dir in that directory
        aDir = INetURLObject( OStringToOUString( *dir_it, aEncoding ), INET_PROT_FILE, INetURLObject::ENCODE_ALL );
        INetURLObject aFDir( aDir );
        ByteString aDirPath( aFDir.PathToFileName(), aEncoding );
        if( createPath( aDirPath ) )
        {
            aFDir.Append( String( RTL_CONSTASCII_USTRINGPARAM( "fonts.dir" ) ) );
            aFontsDir.Open( aFDir.PathToFileName(), STREAM_READ | STREAM_WRITE );
        }
    }
    if( aFontsDir.IsOpen() && aFontsDir.IsWritable() )
    {
#if OSL_DEBUG_LEVEL > 1
        fprintf( stderr, "found writable %s\n", ByteString( aFontsDir.GetFileName(), osl_getThreadTextEncoding() ).GetBuffer() );
#endif
        bSuccess = true;
    }

    return bSuccess;
}

// -------------------------------------------------------------------------

bool PrintFontManager::checkChangeFontPropertiesPossible( fontID nFontID ) const
{
    bool bSuccess = false;
    PrintFont* pFont = getFont( nFontID );
    if( pFont )
    {
        OString aFontsDirPath;
        switch( pFont->m_eType )
        {
            case fonttype::Type1:
                aFontsDirPath = getDirectory( static_cast< Type1FontFile* >(pFont)->m_nDirectory );
                break;
            case fonttype::TrueType:
                aFontsDirPath = getDirectory( static_cast< TrueTypeFontFile* >(pFont)->m_nDirectory );
                break;
        }
        if( aFontsDirPath.getLength() )
        {
            OUString aUniPath, aFDPath;
            FileBase::getFileURLFromSystemPath( OStringToOUString( aFontsDirPath, osl_getThreadTextEncoding() ), aUniPath );
            aUniPath += OUString::createFromAscii( "/fonts.dir" );
            FileBase::getSystemPathFromFileURL( aUniPath, aFDPath );
            SvFileStream aFontsDir( aFDPath, STREAM_READ | STREAM_WRITE );
            if( aFontsDir.IsOpen() && aFontsDir.IsWritable() )
                bSuccess = true;
        }
    }
    return bSuccess;
}

// -------------------------------------------------------------------------

bool PrintFontManager::changeFontProperties( fontID nFontID, const ::rtl::OUString& rXLFD )
{
    bool bSuccess = false;
    if( ! checkChangeFontPropertiesPossible( nFontID ) )
        return bSuccess;

    rtl_TextEncoding aEncoding = osl_getThreadTextEncoding();
    PrintFont* pFont = getFont( nFontID );
    OString aFontsDirPath;
    ByteString aFontFile;

    int nTTCnumber = -1;
    switch( pFont->m_eType )
    {
        case fonttype::Type1:
            aFontsDirPath = getDirectory( static_cast< Type1FontFile* >(pFont)->m_nDirectory );
            aFontFile = static_cast< Type1FontFile* >(pFont)->m_aFontFile;
            break;
        case fonttype::TrueType:
            aFontsDirPath = getDirectory( static_cast< TrueTypeFontFile* >(pFont)->m_nDirectory );
            aFontFile = static_cast< TrueTypeFontFile* >(pFont)->m_aFontFile;
            nTTCnumber = static_cast< TrueTypeFontFile* >(pFont)->m_nCollectionEntry;
            break;
    }
    OUString aUniPath, aFDPath;
    FileBase::getFileURLFromSystemPath( OStringToOUString( aFontsDirPath, aEncoding ), aUniPath );
    aUniPath += OUString::createFromAscii( "/fonts.dir" );
    FileBase::getSystemPathFromFileURL( aUniPath, aFDPath );
    SvFileStream aFontsDir( aFDPath, STREAM_READ | STREAM_WRITE );
    aFontsDir.SetLineDelimiter( LINEEND_LF );
    if( aFontsDir.IsOpen() && aFontsDir.IsWritable() )
    {
        ByteString aXLFD( OUStringToOString( rXLFD, RTL_TEXTENCODING_UTF8 ) );
        ByteString aAddStyle = aXLFD.GetToken( '-', 6 );
        if( aAddStyle.Search( "utf8" ) == STRING_NOTFOUND )
        {
            aAddStyle.Append( aAddStyle.Len() ? ";utf8" : "utf8" );
            aXLFD.SetToken( 6, ';', aAddStyle );
        }
        ::std::list< ByteString > aLines;
        ByteString aLine;
        aFontsDir.ReadLine( aLine ); // pop line count
        while( ! aFontsDir.IsEof() )
        {
            aFontsDir.ReadLine( aLine );
            if( GetCommandLineToken( 0, aLine ) == aFontFile )
            {
                if( nTTCnumber <= 0 )
                {
                    bSuccess = true;
                    aLine = aFontFile;
                    aLine += ' ';
                    aLine += aXLFD;
                }
                else
                    nTTCnumber--;
            }
            if( aLine.Len() )
                aLines.push_back( aLine );
        }
        if( ! bSuccess )
        {
            bSuccess = true;
            aLine = aFontFile;
            aLine += ' ';
            aLine += aXLFD;
            aLines.push_back( aLine );
        }
        // write the file
        aFontsDir.Seek( STREAM_SEEK_TO_BEGIN );
        aFontsDir.SetStreamSize( 0 );
        // write number of fonts
        aFontsDir.WriteLine( ByteString::CreateFromInt32( aLines.size() ) );
        while( aLines.begin() != aLines.end() )
        {
            aFontsDir.WriteLine( aLines.front() );
            aLines.pop_front();
        }
        getFontAttributesFromXLFD( pFont, aXLFD );
        m_pFontCache->updateFontCacheEntry( pFont, true );
    }
    return bSuccess;
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
    bool bRet = analyzeFontFile( nDirID, aFile, true, ::std::list<OString>(), aFonts );
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
            INetURLObject aFontsDirPath( String( aFile, osl_getThreadTextEncoding() ), INET_PROT_FILE, INetURLObject::ENCODE_ALL );
            aFontsDirPath.CutName();
            aFontsDirPath.Append( String( RTL_CONSTASCII_USTRINGPARAM( "fonts.dir" ) ) );
            ByteString aFontsDirSysPath( aFontsDirPath.PathToFileName(), osl_getThreadTextEncoding() );
            if( ! access( aFontsDirSysPath.GetBuffer(), R_OK | W_OK ) )
            {
                SvFileStream aFontsDir( aFontsDirPath.PathToFileName(), STREAM_READ | STREAM_WRITE );
                aFontsDir.SetLineDelimiter( LINEEND_LF );
                if( aFontsDir.IsOpen() )
                {
                    ByteString aLine;
                    // skip entry count
                    aFontsDir.ReadLine( aLine );
                    ::std::list< ByteString > aLines;
                    int nPos = aFile.SearchBackward( '/' );
                    ByteString aFileName( aFile.Copy( nPos != STRING_NOTFOUND ? nPos+1 : 0 ) );
                    while( ! aFontsDir.IsEof() )
                    {
                        aFontsDir.ReadLine( aLine );
                        if( aLine.Len() && aLine.CompareTo( aFileName, aFileName.Len() ) != COMPARE_EQUAL )
                            aLines.push_back( aLine );
                    }
                    aFontsDir.SetStreamSize( 0 );
                    aFontsDir.Seek( 0 );
                    // write entry count
                    aFontsDir.WriteLine( ByteString::CreateFromInt32( aLines.size() ) );
                    while( aLines.begin() != aLines.end() )
                    {
                        aFontsDir.WriteLine( aLines.front() );
                        aLines.pop_front();
                    }
                }
            }
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
        if( OpenTTFont( aFile.GetBuffer(), pTTFontFile->m_nCollectionEntry < 0 ? 0 : pTTFontFile->m_nCollectionEntry, &pTTFont ) == SF_OK )
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
                                        long* pGlyphIDs,
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
    for( int i = 0; i < nGlyphs; i++ )
    {
        if( pNewEncoding[i] == 0 )
        {
            pOldIndex[ 0 ] = i;
        }
        else
        {
            DBG_ASSERT( !(pGlyphIDs[i] & 0xffff0000), "overlong glyph id" );
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
    if( OpenTTFont( aFromFile.GetBuffer(), pTTFontFile->m_nCollectionEntry < 0 ? 0 : pTTFontFile->m_nCollectionEntry, &pTTFont ) != SF_OK )
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

// -------------------------------------------------------------------------

const std::map< sal_Unicode, sal_Int32 >* PrintFontManager::getEncodingMap( fontID nFont, const std::map< sal_Unicode, rtl::OString >** pNonEncoded ) const
{
    PrintFont* pFont = getFont( nFont );
    if( !pFont ||
        (pFont->m_eType != fonttype::Type1 && pFont->m_eType != fonttype::Builtin)
        )
        return NULL;

    if( ! pFont->m_aEncodingVector.size() )
        pFont->readAfmMetrics( getAfmFile( pFont ), m_pAtoms, true );

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
        snprintf( (char*)aBuf, sizeof( aBuf ), "uni%.4hX\0", aChar );
        aRet.push_back( aBuf );
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

