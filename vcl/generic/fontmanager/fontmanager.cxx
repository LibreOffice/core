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

#include <unistd.h>
#include <sys/stat.h>
#include <dirent.h>
#include <stdlib.h>
#include <osl/thread.h>

#include "unotools/atom.hxx"

#include "fontcache.hxx"
#include "fontsubset.hxx"
#include "impfont.hxx"
#include "svdata.hxx"
#include "generic/geninst.h"
#include "fontmanager.hxx"
#include "vcl/strhelper.hxx"
#include "vcl/ppdparser.hxx"
#include <vcl/embeddedfontshelper.hxx>

#include "tools/urlobj.hxx"
#include "tools/stream.hxx"
#include "tools/debug.hxx"

#include "osl/file.hxx"
#include "osl/process.h"

#include "rtl/tencinfo.h"
#include "rtl/ustrbuf.hxx"
#include "rtl/strbuf.hxx"

#include <sal/macros.h>

#include "i18nlangtag/applelangid.hxx"
#include "i18nlangtag/mslangid.hxx"

#include "parseAFM.hxx"
#include "sft.hxx"

#if OSL_DEBUG_LEVEL > 1
#include <sys/times.h>
#include <stdio.h>
#endif

#include <algorithm>
#include <set>
#include <unordered_set>

#include "adobeenc.tab"

#ifdef CALLGRIND_COMPILE
#include <valgrind/callgrind.h>
#endif

#include <comphelper/processfactory.hxx>
#include <comphelper/string.hxx>
#include "com/sun/star/beans/XMaterialHolder.hpp"
#include "com/sun/star/beans/NamedValue.hpp"

using namespace vcl;
using namespace utl;
using namespace psp;
using namespace osl;
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

static FontWeight parseWeight( const OString& rWeight )
{
    FontWeight eWeight = WEIGHT_DONTKNOW;
    if (rWeight.indexOf("bold") != -1)
    {
        if (rWeight.indexOf("emi") != -1) // semi, demi
            eWeight = WEIGHT_SEMIBOLD;
        else if (rWeight.indexOf("ultra") != -1)
            eWeight = WEIGHT_ULTRABOLD;
        else
            eWeight = WEIGHT_BOLD;
    }
    else if (rWeight.indexOf("heavy") != -1)
        eWeight = WEIGHT_BOLD;
    else if (rWeight.indexOf("light") != -1)
    {
        if (rWeight.indexOf("emi") != -1) // semi, demi
            eWeight = WEIGHT_SEMILIGHT;
        else if (rWeight.indexOf("ultra") != -1)
            eWeight = WEIGHT_ULTRALIGHT;
        else
            eWeight = WEIGHT_LIGHT;
    }
    else if (rWeight.indexOf("black") != -1)
        eWeight = WEIGHT_BLACK;
    else if (rWeight == "demi")
        eWeight = WEIGHT_SEMIBOLD;
    else if ((rWeight == "book") ||
             (rWeight == "semicondensed"))
        eWeight = WEIGHT_LIGHT;
    else if ((rWeight == "medium") || (rWeight == "roman"))
        eWeight = WEIGHT_MEDIUM;
    else
        eWeight = WEIGHT_NORMAL;
    return eWeight;
}

/*
 *  PrintFont implementations
 */
PrintFontManager::PrintFont::PrintFont( fonttype::type eType ) :
        m_eType( eType ),
        m_nFamilyName( 0 ),
        m_nPSName( 0 ),
        m_eItalic( ITALIC_DONTKNOW ),
        m_eWidth( WIDTH_DONTKNOW ),
        m_eWeight( WEIGHT_DONTKNOW ),
        m_ePitch( PITCH_DONTKNOW ),
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
        m_bUserOverride( false )
{
}

PrintFontManager::PrintFont::~PrintFont()
{
    delete m_pMetrics;
}

PrintFontManager::Type1FontFile::~Type1FontFile()
{
}

PrintFontManager::TrueTypeFontFile::TrueTypeFontFile()
:   PrintFont( fonttype::TrueType )
,   m_nDirectory( 0 )
,   m_nCollectionEntry( 0 )
,   m_nTypeFlags( TYPEFLAG_INVALID )
{}

PrintFontManager::TrueTypeFontFile::~TrueTypeFontFile()
{
}

bool PrintFontManager::Type1FontFile::queryMetricPage( int /*nPage*/, MultiAtomProvider* pProvider )
{
    return readAfmMetrics( pProvider, false, false );
}

bool PrintFontManager::TrueTypeFontFile::queryMetricPage( int nPage, MultiAtomProvider* /*pProvider*/ )
{
    bool bSuccess = false;

    OString aFile( PrintFontManager::get().getFontFile( this ) );

    TrueTypeFont* pTTFont = NULL;

    if( OpenTTFontFile( aFile.getStr(), m_nCollectionEntry, &pTTFont ) == SF_OK )
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
        MapString( pTTFont, table, nCharacters, NULL, false );
        TTSimpleGlyphMetrics* pMetrics = GetTTSimpleCharMetrics( pTTFont, nPage*256, nCharacters, false );
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
        MapString( pTTFont, table_vert, nCharacters, NULL, true );
        pMetrics = GetTTSimpleCharMetrics( pTTFont, nPage*256, nCharacters, true );
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
                        m_pMetrics->m_bVerticalSubstitutions[ nPage*256 + i ] = true;
                }
            }
            free( pMetrics );
        }
        CloseTTFont( pTTFont );
        bSuccess = true;
    }
    return bSuccess;
}

/* #i73387# There seem to be fonts with a rather unwell chosen family name
*  consider e.g. "Helvetica Narrow" which defines its family as "Helvetica"
*  It can really only be distinguished by its PSName and FullName. Both of
*  which are not user presentable in OOo. So replace it by something sensible.
*
*  If other fonts feature this behaviour, insert them to the map.
*/
static bool familyNameOverride( const OUString& i_rPSname, OUString& o_rFamilyName )
{
    static std::unordered_map< OUString, OUString, OUStringHash > aPSNameToFamily( 16 );
    if( aPSNameToFamily.empty() ) // initialization
    {
        aPSNameToFamily[ "Helvetica-Narrow" ] = "Helvetica Narrow";
        aPSNameToFamily[ "Helvetica-Narrow-Bold" ] = "Helvetica Narrow";
        aPSNameToFamily[ "Helvetica-Narrow-BoldOblique" ] = "Helvetica Narrow";
        aPSNameToFamily[ "Helvetica-Narrow-Oblique" ] = "Helvetica Narrow";
    }
    std::unordered_map<OUString,OUString,OUStringHash>::const_iterator it =
       aPSNameToFamily.find( i_rPSname );
    bool bReplaced = (it != aPSNameToFamily.end() );
    if( bReplaced )
        o_rFamilyName = it->second;
    return bReplaced;
};

bool PrintFontManager::PrintFont::readAfmMetrics( MultiAtomProvider* pProvider, bool bFillEncodingvector, bool bOnlyGlobalAttributes )
{
    PrintFontManager& rManager( PrintFontManager::get() );
    const OString& rFileName = rManager.getAfmFile( this );

    FontInfo* pInfo = NULL;
    parseFile( rFileName.getStr(), &pInfo, P_ALL );
    if( ! pInfo || ! pInfo->numOfChars )
    {
        if( pInfo )
            freeFontInfo( pInfo );
        return false;
    }

    m_aEncodingVector.clear();
    m_aEncodingVectorPriority.clear();
    // fill in global info

    // PSName
    OUString aPSName( OStringToOUString( pInfo->gfi->fontName, RTL_TEXTENCODING_ISO_8859_1 ) );
    m_nPSName = pProvider->getAtom( ATOM_PSNAME, aPSName, true );

    // family name (if not already set)
    OUString aFamily;
    if( ! m_nFamilyName )
    {
        aFamily = OStringToOUString( pInfo->gfi->familyName, RTL_TEXTENCODING_ISO_8859_1 );
        if( aFamily.isEmpty() )
        {
            aFamily = OStringToOUString( pInfo->gfi->fontName, RTL_TEXTENCODING_ISO_8859_1 );
            sal_Int32 nIndex  = 0;
            aFamily = aFamily.getToken( 0, '-', nIndex );
        }
        familyNameOverride( aPSName, aFamily );
        m_nFamilyName = pProvider->getAtom( ATOM_FAMILYNAME, aFamily, true );
    }
    else
        aFamily = pProvider->getString( ATOM_FAMILYNAME, m_nFamilyName );

    // style name: if fullname begins with family name
    // interpret the rest of fullname as style
    if( m_aStyleName.isEmpty() && pInfo->gfi->fullName && *pInfo->gfi->fullName )
    {
        OUString aFullName( OStringToOUString( pInfo->gfi->fullName, RTL_TEXTENCODING_ISO_8859_1 ) );
        if( aFullName.startsWith( aFamily ) )
            m_aStyleName = WhitespaceToSpace( aFullName.copy( aFamily.getLength() ) );
    }

    // italic
    if( pInfo->gfi->italicAngle > 0 )
        m_eItalic = ITALIC_OBLIQUE;
    else if( pInfo->gfi->italicAngle < 0 )
        m_eItalic = ITALIC_NORMAL;
    else
        m_eItalic = ITALIC_NONE;

    // weight
    OString aWeight( pInfo->gfi->weight );
    m_eWeight = parseWeight( aWeight.toAsciiLowerCase() );

    // pitch
    m_ePitch = pInfo->gfi->isFixedPitch ? PITCH_FIXED : PITCH_VARIABLE;

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

    // try to parse the font name and decide whether it might be a
    // japanese font. Who invented this PITA ?
    OUString aPSNameLastToken( aPSName.copy( aPSName.lastIndexOf( '-' )+1 ) );
    if( aPSNameLastToken == "H"    ||
        aPSNameLastToken == "V"  )
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

        for( unsigned int enc = 0; enc < SAL_N_ELEMENTS( aEncs ) && m_aEncoding == RTL_TEXTENCODING_DONTKNOW; enc++ )
        {
            sal_Int32 nIndex = 0, nOffset = 1;
            do
            {
                OUString aToken( aPSName.getToken( nOffset, '-', nIndex ) );
                if( nIndex == -1 )
                    break;
                nOffset = 0;
                if( aToken.equalsAscii( pEncs[enc] ) )
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

    delete m_pMetrics;
    m_pMetrics = new PrintFontMetrics;
    // mark all pages as queried (or clear if only global font info queried)
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
        std::unique_ptr<sal_Unicode[]> const pUnicodes(new sal_Unicode[pInfo->numOfChars]);
        CharMetricInfo* pChar = pInfo->cmi;
        int i;

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
                    {
                        m_aEncodingVector[ pUnicodes[i] ] = pChar->code;
                        m_aEncodingVectorPriority.insert(pUnicodes[i]);
                    }
                    continue;
                }

                if( m_aEncoding == RTL_TEXTENCODING_UNICODE )
                {
                    pUnicodes[i] = (sal_Unicode)pChar->code;
                    continue;
                }

                OStringBuffer aTranslate;
                if( pChar->code & 0xff000000 )
                    aTranslate.append((char)(pChar->code >> 24));
                if( pChar->code & 0xffff0000 )
                    aTranslate.append((char)((pChar->code & 0x00ff0000) >> 16));
                if( pChar->code & 0xffffff00 )
                    aTranslate.append((char)((pChar->code & 0x0000ff00) >> 8 ));
                aTranslate.append((char)(pChar->code & 0xff));
                OUString aUni(OStringToOUString(aTranslate.makeStringAndClear(), m_aEncoding));
                pUnicodes[i] = aUni.toChar();
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
                    {
                        m_aEncodingVector[ pUnicodes[i] ] = pChar->code;
                        m_aEncodingVectorPriority.insert(pUnicodes[i]);
                    }
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
                    ::std::pair< std::unordered_multimap< sal_uInt8, sal_Unicode >::const_iterator,
                          std::unordered_multimap< sal_uInt8, sal_Unicode >::const_iterator >
                          aCodes = rManager.getUnicodeFromAdobeCode( pChar->code );
                    bool bFirst = true;
                    while( aCodes.first != aCodes.second )
                    {
                        if( (*aCodes.first).second != 0 )
                        {
                            m_pMetrics->m_aMetrics[ (*aCodes.first).second ] = aMetric;
                            if( bFillEncodingvector )
                            {
                                m_aEncodingVector[ (*aCodes.first).second ] = pChar->code;
                                if (bFirst) // arbitrarily prefer the first one
                                {
                                    m_aEncodingVectorPriority.insert((*aCodes.first).second);
                                    bFirst = false;
                                }
                            }
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
                    {
                        m_aEncodingVector[ code ] = pChar->code;
                        m_aEncodingVectorPriority.insert(code);
                    }
                }
            }
        }
    }

    freeFontInfo( pInfo );
    return true;
}

/*
 *  one instance only
 */
PrintFontManager& PrintFontManager::get()
{
    static PrintFontManager* pManager = NULL;
    if( ! pManager )
    {
        static PrintFontManager theManager;
        pManager = &theManager;
        pManager->initialize();
    }
    return *pManager;
}

/*
 *  the PrintFontManager
 */

PrintFontManager::PrintFontManager()
    : m_nNextFontID( 1 )
    , m_pAtoms( new MultiAtomProvider() )
    , m_nNextDirAtom( 1 )
    , m_pFontCache( NULL )
{
    for( unsigned int i = 0; i < SAL_N_ELEMENTS( aAdobeCodes ); i++ )
    {
        m_aUnicodeToAdobename.insert( std::unordered_multimap< sal_Unicode, OString >::value_type( aAdobeCodes[i].aUnicode, aAdobeCodes[i].pAdobename ) );
        m_aAdobenameToUnicode.insert( std::unordered_multimap< OString, sal_Unicode, OStringHash >::value_type( aAdobeCodes[i].pAdobename, aAdobeCodes[i].aUnicode ) );
        if( aAdobeCodes[i].aAdobeStandardCode )
        {
            m_aUnicodeToAdobecode.insert( std::unordered_multimap< sal_Unicode, sal_uInt8 >::value_type( aAdobeCodes[i].aUnicode, aAdobeCodes[i].aAdobeStandardCode ) );
            m_aAdobecodeToUnicode.insert( std::unordered_multimap< sal_uInt8, sal_Unicode >::value_type( aAdobeCodes[i].aAdobeStandardCode, aAdobeCodes[i].aUnicode ) );
        }
    }

#if defined(ENABLE_DBUS) && defined(ENABLE_PACKAGEKIT)
    m_aFontInstallerTimer.SetTimeoutHdl(LINK(this, PrintFontManager, autoInstallFontLangSupport));
    m_aFontInstallerTimer.SetTimeout(5000);
#endif
}

PrintFontManager::~PrintFontManager()
{
    m_aFontInstallerTimer.Stop();
    deinitFontconfig();
    for( std::unordered_map< fontID, PrintFont* >::const_iterator it = m_aFonts.begin(); it != m_aFonts.end(); ++it )
        delete (*it).second;
    delete m_pAtoms;
    delete m_pFontCache;
}

OString PrintFontManager::getDirectory( int nAtom ) const
{
    std::unordered_map< int, OString >::const_iterator it( m_aAtomToDir.find( nAtom ) );
    return it != m_aAtomToDir.end() ? it->second : OString();
}

int PrintFontManager::getDirectoryAtom( const OString& rDirectory, bool bCreate )
{
    int nAtom = 0;
    std::unordered_map< OString, int, OStringHash >::const_iterator it
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

std::vector<fontID> PrintFontManager::addFontFile( const OString& rFileName )
{
    rtl_TextEncoding aEncoding = osl_getThreadTextEncoding();
    INetURLObject aPath( OStringToOUString( rFileName, aEncoding ), INetURLObject::FSYS_DETECT );
    OString aName( OUStringToOString( aPath.GetName( INetURLObject::DECODE_WITH_CHARSET, aEncoding ), aEncoding ) );
    OString aDir( OUStringToOString(
        INetURLObject::decode( aPath.GetPath(), INetURLObject::DECODE_WITH_CHARSET, aEncoding ), aEncoding ) );

    int nDirID = getDirectoryAtom( aDir, true );
    std::vector<fontID> aFontIds = findFontFileIDs( nDirID, aName );
    if( aFontIds.empty() )
    {
        ::std::list< PrintFont* > aNewFonts;
        if( analyzeFontFile( nDirID, aName, aNewFonts ) )
        {
            for( ::std::list< PrintFont* >::iterator it = aNewFonts.begin();
                 it != aNewFonts.end(); ++it )
            {
                fontID nFontId = m_nNextFontID++;
                m_aFonts[nFontId] = *it;
                m_aFontFileToFontID[ aName ].insert( nFontId );
                m_pFontCache->updateFontCacheEntry( *it, true );
                aFontIds.push_back(nFontId);
            }
        }
    }
    return aFontIds;
}

enum fontFormat
{
    UNKNOWN, TRUETYPE, CFF, TYPE1
};

bool PrintFontManager::analyzeFontFile( int nDirID, const OString& rFontFile, ::std::list< PrintFontManager::PrintFont* >& rNewFonts, const char *pFormat ) const
{
    rNewFonts.clear();

    OString aDir( getDirectory( nDirID ) );

    OString aFullPath( aDir );
    aFullPath += "/";
    aFullPath += rFontFile;

    // #i1872# reject unreadable files
    if( access( aFullPath.getStr(), R_OK ) )
        return false;

    fontFormat eFormat = UNKNOWN;
    if (pFormat)
    {
        if (!strcmp(pFormat, "TrueType"))
            eFormat = TRUETYPE;
        else if (!strcmp(pFormat, "CFF"))
            eFormat = CFF;
        else if (!strcmp(pFormat, "Type 1"))
            eFormat = TYPE1;
    }
    if (eFormat == UNKNOWN)
    {
        OString aExt( rFontFile.copy( rFontFile.lastIndexOf( '.' )+1 ) );
        if( aExt.equalsIgnoreAsciiCase("pfb") || aExt.equalsIgnoreAsciiCase("pfa") )
            eFormat = TYPE1;
        else if( aExt.equalsIgnoreAsciiCase("ttf")
             ||  aExt.equalsIgnoreAsciiCase("ttc")
             ||  aExt.equalsIgnoreAsciiCase("tte") ) // #i33947# for Gaiji support
            eFormat = TRUETYPE;
        else if( aExt.equalsIgnoreAsciiCase("otf") ) // check for TTF- and PS-OpenType too
            eFormat = CFF;
    }

    if (eFormat == TYPE1)
    {
        // check for corresponding afm metric
        // first look for an adjacent file
        static const char* pSuffix[] = { ".afm", ".AFM" };

        for( unsigned int i = 0; i < SAL_N_ELEMENTS(pSuffix); i++ )
        {
            OString aName = OStringBuffer(
                rFontFile.copy(0, rFontFile.getLength() - 4)).
                append(pSuffix[i]).makeStringAndClear();

            OStringBuffer aFilePath(aDir);
            aFilePath.append('/').append(aName);

            OString aAfmFile;
            if( access( aFilePath.makeStringAndClear().getStr(), R_OK ) )
            {
                // try in subdirectory afm instead
                aFilePath.append(aDir).append("/afm/").append(aName);

                if (!access(aFilePath.getStr(), R_OK))
                    aAfmFile = OString("afm/") + aName;
            }
            else
                aAfmFile = aName;

            if( !aAfmFile.isEmpty() )
            {
                Type1FontFile* pFont = new Type1FontFile();
                pFont->m_nDirectory     = nDirID;

                pFont->m_aFontFile      = rFontFile;
                pFont->m_aMetricFile    = aAfmFile;

                if( ! pFont->readAfmMetrics( m_pAtoms, false, true ) )
                {
                    delete pFont;
                    pFont = NULL;
                }
                if( pFont )
                    rNewFonts.push_back( pFont );
                break;
            }
        }
    }
    else if (eFormat == TRUETYPE || eFormat == CFF)
    {
        // get number of ttc entries
        int nLength = CountTTCFonts( aFullPath.getStr() );
        if (nLength > 0)
        {
#if OSL_DEBUG_LEVEL > 1
            fprintf( stderr, "ttc: %s contains %d fonts\n", aFullPath.getStr(), nLength );
#endif

            sal_uInt64 fileSize = 0;

            OUString aURL;
            if (osl::File::getFileURLFromSystemPath(OStringToOUString(aFullPath, osl_getThreadTextEncoding()),
                aURL) == osl::File::E_None)
            {
                osl::File aFile(aURL);
                if (aFile.open(osl_File_OpenFlag_Read | osl_File_OpenFlag_NoLock) == osl::File::E_None)
                {
                    osl::DirectoryItem aItem;
                    osl::DirectoryItem::get( aURL, aItem );
                    osl::FileStatus aFileStatus( osl_FileStatus_Mask_FileSize );
                    aItem.getFileStatus( aFileStatus );
                    fileSize = aFileStatus.getFileSize();
                }
            }

            //Feel free to calc the exact max possible number of fonts a file
            //could contain given its physical size. But this will clamp it to
            //a sane starting point
            //http://processingjs.nihongoresources.com/the_smallest_font/
            //https://github.com/grzegorzrolek/null-ttf
            const int nMaxFontsPossible = fileSize / 528;
            if (nLength > nMaxFontsPossible)
                nLength = nMaxFontsPossible;

            for( int i = 0; i < nLength; i++ )
            {
                TrueTypeFontFile* pFont     = new TrueTypeFontFile();
                pFont->m_nDirectory         = nDirID;
                pFont->m_aFontFile          = rFontFile;
                pFont->m_nCollectionEntry   = i;
                if( ! analyzeTrueTypeFile( pFont ) )
                {
                    delete pFont;
                    pFont = NULL;
                }
                else
                    rNewFonts.push_back( pFont );
            }
        }
        else
        {
            TrueTypeFontFile* pFont     = new TrueTypeFontFile();
            pFont->m_nDirectory         = nDirID;
            pFont->m_aFontFile          = rFontFile;
            pFont->m_nCollectionEntry   = 0;

            // need to read the font anyway to get aliases inside the font file
            if( ! analyzeTrueTypeFile( pFont ) )
            {
                delete pFont;
                pFont = NULL;
            }
            else
                rNewFonts.push_back( pFont );
        }
    }
    return ! rNewFonts.empty();
}

fontID PrintFontManager::findFontFileID( int nDirID, const OString& rFontFile, int nFaceIndex ) const
{
    fontID nID = 0;

    std::unordered_map< OString, ::std::set< fontID >, OStringHash >::const_iterator set_it = m_aFontFileToFontID.find( rFontFile );
    if( set_it == m_aFontFileToFontID.end() )
        return nID;

    for( ::std::set< fontID >::const_iterator font_it = set_it->second.begin(); font_it != set_it->second.end() && ! nID; ++font_it )
    {
        std::unordered_map< fontID, PrintFont* >::const_iterator it = m_aFonts.find( *font_it );
        if( it == m_aFonts.end() )
            continue;
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
                    pFont->m_aFontFile == rFontFile && pFont->m_nCollectionEntry == nFaceIndex )
                        nID = it->first;
            }
            break;
            default:
                break;
        }
    }

    return nID;
}

std::vector<fontID> PrintFontManager::findFontFileIDs( int nDirID, const OString& rFontFile ) const
{
    std::vector<fontID> aIds;

    std::unordered_map< OString, ::std::set< fontID >, OStringHash >::const_iterator set_it = m_aFontFileToFontID.find( rFontFile );
    if( set_it == m_aFontFileToFontID.end() )
        return aIds;

    for( ::std::set< fontID >::const_iterator font_it = set_it->second.begin(); font_it != set_it->second.end(); ++font_it )
    {
        std::unordered_map< fontID, PrintFont* >::const_iterator it = m_aFonts.find( *font_it );
        if( it == m_aFonts.end() )
            continue;
        switch( it->second->m_eType )
        {
            case fonttype::Type1:
            {
                Type1FontFile* const pFont = static_cast< Type1FontFile* const >((*it).second);
                if( pFont->m_nDirectory == nDirID &&
                    pFont->m_aFontFile == rFontFile )
                    aIds.push_back(it->first);
            }
            break;
            case fonttype::TrueType:
            {
                TrueTypeFontFile* const pFont = static_cast< TrueTypeFontFile* const >((*it).second);
                if( pFont->m_nDirectory == nDirID &&
                    pFont->m_aFontFile == rFontFile )
                    aIds.push_back(it->first);
            }
            break;
            default:
                break;
        }
    }

    return aIds;
}

OUString PrintFontManager::convertTrueTypeName( void* pRecord )
{
    NameRecord* pNameRecord = static_cast<NameRecord*>(pRecord);
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
    else if( pNameRecord->platformID == 1 )
    {
        OString aName(reinterpret_cast<char*>(pNameRecord->sptr), pNameRecord->slen);
        rtl_TextEncoding eEncoding = RTL_TEXTENCODING_DONTKNOW;
        switch (pNameRecord->encodingID)
        {
            case 0:
                eEncoding = RTL_TEXTENCODING_APPLE_ROMAN;
                break;
            case 1:
                eEncoding = RTL_TEXTENCODING_APPLE_JAPANESE;
                break;
            case 2:
                eEncoding = RTL_TEXTENCODING_APPLE_CHINTRAD;
                break;
            case 3:
                eEncoding = RTL_TEXTENCODING_APPLE_KOREAN;
                break;
            case 4:
                eEncoding = RTL_TEXTENCODING_APPLE_ARABIC;
                break;
            case 5:
                eEncoding = RTL_TEXTENCODING_APPLE_HEBREW;
                break;
            case 6:
                eEncoding = RTL_TEXTENCODING_APPLE_GREEK;
                break;
            case 7:
                eEncoding = RTL_TEXTENCODING_APPLE_CYRILLIC;
                break;
            case 9:
                eEncoding = RTL_TEXTENCODING_APPLE_DEVANAGARI;
                break;
            case 10:
                eEncoding = RTL_TEXTENCODING_APPLE_GURMUKHI;
                break;
            case 11:
                eEncoding = RTL_TEXTENCODING_APPLE_GUJARATI;
                break;
            case 21:
                eEncoding = RTL_TEXTENCODING_APPLE_THAI;
                break;
            case 25:
                eEncoding = RTL_TEXTENCODING_APPLE_CHINSIMP;
                break;
            case 29:
                eEncoding = RTL_TEXTENCODING_APPLE_CENTEURO;
                break;
            case 32:    //Uninterpreted
                eEncoding = RTL_TEXTENCODING_UTF8;
                break;
            default:
                SAL_WARN("vcl", "Unimplemented mac encoding " << pNameRecord->encodingID << " to unicode conversion");
                break;
        }
        if (eEncoding != RTL_TEXTENCODING_DONTKNOW)
            aValue = OStringToOUString(aName, eEncoding);
    }

    return aValue;
}

//fdo#33349.There exists an archaic Berling Antiqua font which has a "Times New
//Roman" name field in it. We don't want the "Times New Roman" name to take
//precedence in this case. We take Berling Antiqua as a higher priority name,
//and erase the "Times New Roman" name
namespace
{
    bool isBadTNR(const OUString &rName, ::std::set< OUString >& rSet)
    {
        bool bRet = false;
        if ( rName == "Berling Antiqua" )
        {
            ::std::set< OUString >::iterator aEnd = rSet.end();
            ::std::set< OUString >::iterator aI = rSet.find("Times New Roman");
            if (aI != aEnd)
            {
                bRet = true;
                rSet.erase(aI);
            }
        }
        return bRet;
    }
}

void PrintFontManager::analyzeTrueTypeFamilyName( void* pTTFont, ::std::list< OUString >& rNames )
{
    OUString aFamily;

    rNames.clear();
    ::std::set< OUString > aSet;

    NameRecord* pNameRecords = NULL;
    int nNameRecords = GetTTNameRecords( static_cast<TrueTypeFont*>(pTTFont), &pNameRecords );
    if( nNameRecords && pNameRecords )
    {
        LanguageTag aSystem("");
        LanguageType eLang = aSystem.getLanguageType();
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
                if (pNameRecords[i].languageID == eLang)
                    nMatch = 8000;
                else if( pNameRecords[i].languageID == LANGUAGE_ENGLISH_US )
                    nMatch = 2000;
                else if( pNameRecords[i].languageID == LANGUAGE_ENGLISH ||
                         pNameRecords[i].languageID == LANGUAGE_ENGLISH_UK )
                    nMatch = 1500;
                else
                    nMatch = 1000;
            }
            else if (pNameRecords[i].platformID == 1)
            {
                AppleLanguageId aAppleId = static_cast<AppleLanguageId>(pNameRecords[i].languageID);
                LanguageTag aApple(makeLanguageTagFromAppleLanguageId(aAppleId));
                if (aApple == aSystem)
                    nMatch = 8000;
                else if (aAppleId == AppleLanguageId::ENGLISH)
                    nMatch = 2000;
                else
                    nMatch = 1000;
            }
            OUString aName = convertTrueTypeName( pNameRecords + i );
            aSet.insert( aName );
            if (aName.isEmpty())
                continue;
            if( nMatch > nLastMatch || isBadTNR(aName, aSet) )
            {
                nLastMatch = nMatch;
                aFamily = aName;
            }
        }
    }
    DisposeNameRecords( pNameRecords, nNameRecords );
    if( !aFamily.isEmpty() )
    {
        rNames.push_front( aFamily );
        for( ::std::set< OUString >::const_iterator it = aSet.begin(); it != aSet.end(); ++it )
            if( *it != aFamily )
                rNames.push_back( *it );
    }
    return;
}

bool PrintFontManager::analyzeTrueTypeFile( PrintFont* pFont ) const
{
    bool bSuccess = false;
    rtl_TextEncoding aEncoding = osl_getThreadTextEncoding();
    OString aFile = getFontFile( pFont );
    TrueTypeFont* pTTFont = NULL;

    TrueTypeFontFile* pTTFontFile = static_cast< TrueTypeFontFile* >(pFont);
    if( OpenTTFontFile( aFile.getStr(), pTTFontFile->m_nCollectionEntry, &pTTFont ) == SF_OK )
    {
        TTGlobalFontInfo aInfo;
        GetTTGlobalFontInfo( pTTFont, & aInfo );

        ::std::list< OUString > aNames;
        analyzeTrueTypeFamilyName( pTTFont, aNames );

        // set family name from XLFD if possible
        if( ! pFont->m_nFamilyName )
        {
            if( !aNames.empty() )
            {
                pFont->m_nFamilyName = m_pAtoms->getAtom( ATOM_FAMILYNAME, aNames.front(), true );
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

                 pFont->m_nFamilyName = m_pAtoms->getAtom( ATOM_FAMILYNAME, OStringToOUString( pTTFontFile->m_aFontFile.copy( 0, dotIndex ), aEncoding ), true );
            }
        }
        for( ::std::list< OUString >::iterator it = aNames.begin(); it != aNames.end(); ++it )
        {
            if( !it->isEmpty() )
            {
                int nAlias = m_pAtoms->getAtom( ATOM_FAMILYNAME, *it, true );
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

        SAL_WARN_IF( !aInfo.psname, "vcl", "No PostScript name in font:" << aFile.getStr() );

        OUString sPSName = aInfo.psname ?
            OUString(aInfo.psname, rtl_str_getLength(aInfo.psname), aEncoding) :
            m_pAtoms->getString(ATOM_FAMILYNAME, pFont->m_nFamilyName); // poor font does not have a postscript name

        pFont->m_nPSName = m_pAtoms->getAtom( ATOM_PSNAME, sPSName, true );

        switch( aInfo.weight )
        {
            case FW_THIN:           pFont->m_eWeight = WEIGHT_THIN; break;
            case FW_EXTRALIGHT: pFont->m_eWeight = WEIGHT_ULTRALIGHT; break;
            case FW_LIGHT:          pFont->m_eWeight = WEIGHT_LIGHT; break;
            case FW_MEDIUM:     pFont->m_eWeight = WEIGHT_MEDIUM; break;
            case FW_SEMIBOLD:       pFont->m_eWeight = WEIGHT_SEMIBOLD; break;
            case FW_BOLD:           pFont->m_eWeight = WEIGHT_BOLD; break;
            case FW_EXTRABOLD:      pFont->m_eWeight = WEIGHT_ULTRABOLD; break;
            case FW_BLACK:          pFont->m_eWeight = WEIGHT_BLACK; break;

            case FW_NORMAL:
            default:        pFont->m_eWeight = WEIGHT_NORMAL; break;
        }

        switch( aInfo.width )
        {
            case FWIDTH_ULTRA_CONDENSED:    pFont->m_eWidth = WIDTH_ULTRA_CONDENSED; break;
            case FWIDTH_EXTRA_CONDENSED:    pFont->m_eWidth = WIDTH_EXTRA_CONDENSED; break;
            case FWIDTH_CONDENSED:          pFont->m_eWidth = WIDTH_CONDENSED; break;
            case FWIDTH_SEMI_CONDENSED: pFont->m_eWidth = WIDTH_SEMI_CONDENSED; break;
            case FWIDTH_SEMI_EXPANDED:      pFont->m_eWidth = WIDTH_SEMI_EXPANDED; break;
            case FWIDTH_EXPANDED:           pFont->m_eWidth = WIDTH_EXPANDED; break;
            case FWIDTH_EXTRA_EXPANDED: pFont->m_eWidth = WIDTH_EXTRA_EXPANDED; break;
            case FWIDTH_ULTRA_EXPANDED: pFont->m_eWidth = WIDTH_ULTRA_EXPANDED; break;

            case FWIDTH_NORMAL:
            default:                        pFont->m_eWidth = WIDTH_NORMAL; break;
        }

        pFont->m_ePitch = aInfo.pitch ? PITCH_FIXED : PITCH_VARIABLE;
        pFont->m_eItalic = aInfo.italicAngle == 0 ? ITALIC_NONE : ( aInfo.italicAngle < 0 ? ITALIC_NORMAL : ITALIC_OBLIQUE );
        // #104264# there are fonts that set italic angle 0 although they are
        // italic; use macstyle bit here
        if( aInfo.italicAngle == 0 && (aInfo.macStyle & 2) )
            pFont->m_eItalic = ITALIC_NORMAL;

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
    else
        SAL_WARN("vcl", "Could not OpenTTFont \"" << aFile.getStr() << "\"\n");

    return bSuccess;
}

static bool AreFCSubstitutionsEnabled()
{
    return (SalGenericInstance::FetchFontSubstitutionFlags() & 3) == 0;
}

void PrintFontManager::initialize()
{
    #ifdef CALLGRIND_COMPILE
    CALLGRIND_TOGGLE_COLLECT();
    CALLGRIND_ZERO_STATS();
    #endif

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

    // initialize can be called more than once, e.g.
    // gtk-fontconfig-timestamp changes to reflect new font installed and
    // PrintFontManager::initialize called again
    {
        for( std::unordered_map< fontID, PrintFont* >::const_iterator it = m_aFonts.begin(); it != m_aFonts.end(); ++it )
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

    struct tms tms;

    aStart = times( &tms );
#endif

    // first try fontconfig
    initFontconfig();

    // part one - look for downloadable fonts
    rtl_TextEncoding aEncoding = osl_getThreadTextEncoding();
    const OUString &rSalPrivatePath = psp::getFontPath();

    // search for the fonts in SAL_PRIVATE_FONTPATH first; those are
    // the fonts installed with the office
    if( !rSalPrivatePath.isEmpty() )
    {
        OString aPath = OUStringToOString( rSalPrivatePath, aEncoding );
        const bool bAreFCSubstitutionsEnabled = AreFCSubstitutionsEnabled();
        sal_Int32 nIndex = 0;
        do
        {
            OString aToken = aPath.getToken( 0, ';', nIndex );
            normPath( aToken );
            if ( aToken.isEmpty() )
            {
                continue;
            }
            // if registering an app-specific fontdir with fontconfig fails
            // and fontconfig-based substitutions are enabled
            // then trying to use these app-specific fonts doesn't make sense
            if( !addFontconfigDir( aToken ) )
                if( bAreFCSubstitutionsEnabled )
                    continue;
            m_aFontDirectories.push_back( aToken );
            m_aPrivateFontDirectories.push_back( getDirectoryAtom( aToken, true ) );
        } while( nIndex >= 0 );
    }

    // protect against duplicate paths
    std::unordered_map< OString, int, OStringHash > visited_dirs;

    // Don't search directories that fontconfig already did
    countFontconfigFonts( visited_dirs );

    // search for font files in each path
    std::list< OString >::iterator dir_it;
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
#if OSL_DEBUG_LEVEL > 1
                else
                    fprintf(stderr, "Un-cached type '%d'\n", (*it)->m_eType);
#if OSL_DEBUG_LEVEL > 2
                fprintf( stderr, "adding cached font %d: %s\n", aFont, getFontFileSysPath( aFont ).getStr() );
#endif
#endif
            }
            if( ! m_pFontCache->scanAdditionalFiles( aPath ) )
                continue;
        }

    }

#if OSL_DEBUG_LEVEL > 1
    aStep1 = times( &tms );
#endif

    // part three - fill in family styles
    std::unordered_map< fontID, PrintFont* >::iterator font_it;
    for (font_it = m_aFonts.begin(); font_it != m_aFonts.end(); ++font_it)
    {
        std::unordered_map< int, FontFamily >::const_iterator it =
              m_aFamilyTypes.find( font_it->second->m_nFamilyName );
        if (it != m_aFamilyTypes.end())
            continue;
        const OUString& rFamily =
            m_pAtoms->getString( ATOM_FAMILYNAME, font_it->second->m_nFamilyName);
        FontFamily eType = matchFamilyName( rFamily );
        m_aFamilyTypes[ font_it->second->m_nFamilyName ] = eType;
    }

#if OSL_DEBUG_LEVEL > 1
    aStep2 = times( &tms );
    fprintf( stderr, "PrintFontManager::initialize: collected %" SAL_PRI_SIZET "u fonts\n", m_aFonts.size() );
    double fTick = (double)sysconf( _SC_CLK_TCK );
    fprintf( stderr, "Step 1 took %lf seconds\n", (double)(aStep1 - aStart)/fTick );
    fprintf( stderr, "Step 2 took %lf seconds\n", (double)(aStep2 - aStep1)/fTick );
#endif

    m_pFontCache->flush();

    #ifdef CALLGRIND_COMPILE
    CALLGRIND_DUMP_STATS();
    CALLGRIND_TOGGLE_COLLECT();
    #endif
}

void PrintFontManager::getFontList( ::std::list< fontID >& rFontIDs )
{
    rFontIDs.clear();
    std::unordered_map< fontID, PrintFont* >::const_iterator it;

    for( it = m_aFonts.begin(); it != m_aFonts.end(); ++it )
        rFontIDs.push_back( it->first );
}

void PrintFontManager::fillPrintFontInfo( PrintFont* pFont, FastPrintFontInfo& rInfo ) const
{
    std::unordered_map< int, FontFamily >::const_iterator style_it =
          m_aFamilyTypes.find( pFont->m_nFamilyName );
    rInfo.m_eType           = pFont->m_eType;
    rInfo.m_aFamilyName     = m_pAtoms->getString( ATOM_FAMILYNAME, pFont->m_nFamilyName );
    rInfo.m_aStyleName      = pFont->m_aStyleName;
    rInfo.m_eFamilyStyle    = style_it != m_aFamilyTypes.end() ? style_it->second : FAMILY_DONTKNOW;
    rInfo.m_eItalic         = pFont->m_eItalic;
    rInfo.m_eWidth          = pFont->m_eWidth;
    rInfo.m_eWeight         = pFont->m_eWeight;
    rInfo.m_ePitch          = pFont->m_ePitch;
    rInfo.m_aEncoding       = pFont->m_aEncoding;

    rInfo.m_bEmbeddable  = (pFont->m_eType == fonttype::Type1);
    rInfo.m_bSubsettable = (pFont->m_eType == fonttype::TrueType); // TODO: rename to SfntType

    rInfo.m_aAliases.clear();
    for( ::std::list< int >::iterator it = pFont->m_aAliases.begin(); it != pFont->m_aAliases.end(); ++it )
        rInfo.m_aAliases.push_back( m_pAtoms->getString( ATOM_FAMILYNAME, *it ) );
}

void PrintFontManager::fillPrintFontInfo( PrintFont* pFont, PrintFontInfo& rInfo ) const
{
    if( ( pFont->m_nAscend == 0 && pFont->m_nDescend == 0 ) ||
        ! pFont->m_pMetrics || pFont->m_pMetrics->isEmpty()
        )
    {
        // might be a truetype font not analyzed or type1 without metrics read
        if( pFont->m_eType == fonttype::Type1 )
            pFont->readAfmMetrics( m_pAtoms, false, false );
        else if( pFont->m_eType == fonttype::TrueType )
            analyzeTrueTypeFile( pFont );
    }

    fillPrintFontInfo( pFont, static_cast< FastPrintFontInfo& >( rInfo ) );

    rInfo.m_nAscend         = pFont->m_nAscend;
    rInfo.m_nDescend        = pFont->m_nDescend;
    rInfo.m_nLeading        = pFont->m_nLeading;
    rInfo.m_nWidth          = pFont->m_aGlobalMetricX.width < pFont->m_aGlobalMetricY.width ? pFont->m_aGlobalMetricY.width : pFont->m_aGlobalMetricX.width;
}

bool PrintFontManager::getFontInfo( fontID nFontID, PrintFontInfo& rInfo ) const
{
    PrintFont* pFont = getFont( nFontID );
    if( pFont )
    {
        rInfo.m_nID = nFontID;
        fillPrintFontInfo( pFont, rInfo );
    }
    return pFont != nullptr;
}

bool PrintFontManager::getFontFastInfo( fontID nFontID, FastPrintFontInfo& rInfo ) const
{
    PrintFont* pFont = getFont( nFontID );
    if( pFont )
    {
        rInfo.m_nID = nFontID;
        fillPrintFontInfo( pFont, rInfo );
    }
    return pFont != nullptr;
}

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
                pFont->readAfmMetrics( m_pAtoms, false, true );
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

int PrintFontManager::getFontFaceNumber( fontID nFontID ) const
{
    int nRet = 0;
    PrintFont* pFont = getFont( nFontID );
    if( pFont && pFont->m_eType == fonttype::TrueType )
        nRet = static_cast< TrueTypeFontFile* >(pFont)->m_nCollectionEntry;
    if (nRet < 0)
        nRet = 0;
    return nRet;
}

FontFamily PrintFontManager::matchFamilyName( const OUString& rFamily )
{
    typedef struct {
        const char*  mpName;
        sal_uInt16   mnLength;
        FontFamily   meType;
    } family_t;

#define InitializeClass( p, a ) p, sizeof(p) - 1, a
    const family_t pFamilyMatch[] =  {
        { InitializeClass( "arial",                  FAMILY_SWISS )  },
        { InitializeClass( "arioso",                 FAMILY_SCRIPT ) },
        { InitializeClass( "avant garde",            FAMILY_SWISS )  },
        { InitializeClass( "avantgarde",             FAMILY_SWISS )  },
        { InitializeClass( "bembo",                  FAMILY_ROMAN )  },
        { InitializeClass( "bookman",                FAMILY_ROMAN )  },
        { InitializeClass( "conga",                  FAMILY_ROMAN )  },
        { InitializeClass( "courier",                FAMILY_MODERN ) },
        { InitializeClass( "curl",                   FAMILY_SCRIPT ) },
        { InitializeClass( "fixed",                  FAMILY_MODERN ) },
        { InitializeClass( "gill",                   FAMILY_SWISS )  },
        { InitializeClass( "helmet",                 FAMILY_MODERN ) },
        { InitializeClass( "helvetica",              FAMILY_SWISS )  },
        { InitializeClass( "international",          FAMILY_MODERN ) },
        { InitializeClass( "lucida",                 FAMILY_SWISS )  },
        { InitializeClass( "new century schoolbook", FAMILY_ROMAN )  },
        { InitializeClass( "palatino",               FAMILY_ROMAN )  },
        { InitializeClass( "roman",                  FAMILY_ROMAN )  },
        { InitializeClass( "sans serif",             FAMILY_SWISS )  },
        { InitializeClass( "sansserif",              FAMILY_SWISS )  },
        { InitializeClass( "serf",                   FAMILY_ROMAN )  },
        { InitializeClass( "serif",                  FAMILY_ROMAN )  },
        { InitializeClass( "times",                  FAMILY_ROMAN )  },
        { InitializeClass( "utopia",                 FAMILY_ROMAN )  },
        { InitializeClass( "zapf chancery",          FAMILY_SCRIPT ) },
        { InitializeClass( "zapfchancery",           FAMILY_SCRIPT ) }
    };

    OString aFamily = OUStringToOString( rFamily, RTL_TEXTENCODING_ASCII_US );
    sal_uInt32 nLower = 0;
    sal_uInt32 nUpper = SAL_N_ELEMENTS(pFamilyMatch);

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

    return FAMILY_DONTKNOW;
}

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
            default: break;
        }
    }
    return aMetricPath;
}

OString PrintFontManager::getFontFile( PrintFont* pFont ) const
{
    OString aPath;

    if( pFont && pFont->m_eType == fonttype::Type1 )
    {
        Type1FontFile* pPSFont = static_cast< Type1FontFile* >(pFont);
        std::unordered_map< int, OString >::const_iterator it = m_aAtomToDir.find( pPSFont->m_nDirectory );
        aPath = it->second;
        aPath += "/";
        aPath += pPSFont->m_aFontFile;
    }
    else if( pFont && pFont->m_eType == fonttype::TrueType )
    {
        TrueTypeFontFile* pTTFont = static_cast< TrueTypeFontFile* >(pFont);
        std::unordered_map< int, OString >::const_iterator it = m_aAtomToDir.find( pTTFont->m_nDirectory );
        aPath = it->second;
        aPath += "/";
        aPath += pTTFont->m_aFontFile;
    }
    return aPath;
}

const OUString& PrintFontManager::getPSName( fontID nFontID ) const
{
    PrintFont* pFont = getFont( nFontID );
    if( pFont && pFont->m_nPSName == 0 )
    {
        if( pFont->m_eType == fonttype::TrueType )
            analyzeTrueTypeFile( pFont );
    }

    return m_pAtoms->getString( ATOM_PSNAME, pFont ? pFont->m_nPSName : INVALID_ATOM );
}

int PrintFontManager::getFontAscend( fontID nFontID ) const
{
    PrintFont* pFont = getFont( nFontID );
    if (pFont && pFont->m_nAscend == 0 && pFont->m_nDescend == 0)
    {
        // might be a truetype font not yet analyzed
        if( pFont->m_eType == fonttype::TrueType )
            analyzeTrueTypeFile( pFont );
        else if( pFont->m_eType == fonttype::Type1 )
            pFont->readAfmMetrics( m_pAtoms, false, true );
    }
    return pFont ? pFont->m_nAscend : 0;
}

int PrintFontManager::getFontDescend( fontID nFontID ) const
{
    PrintFont* pFont = getFont( nFontID );
    if (pFont && pFont->m_nAscend == 0 && pFont->m_nDescend == 0)
    {
        // might be a truetype font not yet analyzed
        if( pFont->m_eType == fonttype::TrueType )
            analyzeTrueTypeFile( pFont );
        else if( pFont->m_eType == fonttype::Type1 )
            pFont->readAfmMetrics( m_pAtoms, false, true );
    }
    return pFont ? pFont->m_nDescend : 0;
}

void PrintFontManager::hasVerticalSubstitutions( fontID nFontID,
    const sal_Unicode* pCharacters, int nCharacters, bool* pHasSubst ) const
{
    PrintFont* pFont = getFont( nFontID );
    if (pFont && pFont->m_nAscend == 0 && pFont->m_nDescend == 0)
    {
        // might be a truetype font not yet analyzed
        if( pFont->m_eType == fonttype::TrueType )
            analyzeTrueTypeFile( pFont );
    }

    if (!pFont || !pFont->m_bHaveVerticalSubstitutedGlyphs)
        memset( pHasSubst, 0, sizeof(bool)*nCharacters );
    else
    {
        for( int i = 0; i < nCharacters; i++ )
        {
            sal_Unicode code = pCharacters[i];
            if( ! pFont->m_pMetrics ||
                ! ( pFont->m_pMetrics->m_aPages[ code >> 11 ] & ( 1 << ( ( code >> 8 ) & 7 ) ) ) )
                pFont->queryMetricPage( code >> 8, m_pAtoms );
            std::unordered_map< sal_Unicode, bool >::const_iterator it = pFont->m_pMetrics->m_bVerticalSubstitutions.find( code );
            pHasSubst[i] = it != pFont->m_pMetrics->m_bVerticalSubstitutions.end();
        }
    }
}

bool PrintFontManager::isFontDownloadingAllowedForPrinting( fontID nFont ) const
{
    static const char* pEnable = getenv( "PSPRINT_ENABLE_TTF_COPYRIGHTAWARENESS" );
    bool bRet = true;

    if( pEnable && *pEnable )
    {
        PrintFont* pFont = getFont( nFont );
        if( pFont && pFont->m_eType == fonttype::TrueType )
        {
            TrueTypeFontFile* pTTFontFile = static_cast<TrueTypeFontFile*>(pFont);
            if( pTTFontFile->m_nTypeFlags & TYPEFLAG_INVALID )
            {
                TrueTypeFont* pTTFont = NULL;
                OString aFile = getFontFile( pFont );
                if( OpenTTFontFile( aFile.getStr(), pTTFontFile->m_nCollectionEntry, &pTTFont ) == SF_OK )
                {
                    // get type flags
                    TTGlobalFontInfo aInfo;
                    GetTTGlobalFontInfo( pTTFont, & aInfo );
                    pTTFontFile->m_nTypeFlags = (unsigned int)aInfo.typeFlags;
                    CloseTTFont( pTTFont );
                }
            }

            unsigned int nCopyrightFlags = pTTFontFile->m_nTypeFlags & TYPEFLAG_COPYRIGHT_MASK;

            // http://www.microsoft.com/typography/tt/ttf_spec/ttch02.doc
            // Font embedding is allowed if not restricted completely (only bit 1 set).
            // Preview&Print (bit 2), Editable (bit 3) or Installable (==0) fonts are ok.
            bRet = ( nCopyrightFlags & 0x02 ) != 0x02;
        }
    }
    return bRet;
}

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
        if( pFont->m_eType == fonttype::Type1 )
            pFont->readAfmMetrics( m_pAtoms, false, false );
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
            std::unordered_map< int, CharacterMetric >::const_iterator it =
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

bool PrintFontManager::getMetrics( fontID nFontID, sal_Unicode minCharacter, sal_Unicode maxCharacter, CharacterMetric* pArray, bool bVertical ) const
{
    OSL_PRECOND(minCharacter <= maxCharacter, "invalid char. range");
    if (minCharacter > maxCharacter)
        return false;

    PrintFont* pFont = getFont( nFontID );
    if( ! pFont )
        return false;

    if( ( pFont->m_nAscend == 0 && pFont->m_nDescend == 0 )
        || ! pFont->m_pMetrics || pFont->m_pMetrics->isEmpty()
        )
    {
        // might be a font not yet analyzed
        if( pFont->m_eType == fonttype::Type1 )
            pFont->readAfmMetrics( m_pAtoms, false, false );
        else if( pFont->m_eType == fonttype::TrueType )
            analyzeTrueTypeFile( pFont );
    }

    sal_Unicode code = minCharacter;
    do
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
            std::unordered_map< int, CharacterMetric >::const_iterator it =
                  pFont->m_pMetrics->m_aMetrics.find( effectiveCode );
            // if no vertical metrics are available assume rotated horizontal metrics
            if( bVertical && (it == pFont->m_pMetrics->m_aMetrics.end()) )
                it = pFont->m_pMetrics->m_aMetrics.find( code );
            // the character metrics are in it->second
            if( it != pFont->m_pMetrics->m_aMetrics.end() )
                pArray[ code - minCharacter ] = it->second;
        }
    } while( code++ != maxCharacter );

    return true;
}

// TODO: move most of this stuff into the central font-subsetting code
bool PrintFontManager::createFontSubset(
                                        FontSubsetInfo& rInfo,
                                        fontID nFont,
                                        const OUString& rOutFile,
                                        const sal_GlyphId* pGlyphIds,
                                        const sal_uInt8* pNewEncoding,
                                        sal_Int32* pWidths,
                                        int nGlyphs,
                                        bool bVertical
                                        )
{
    PrintFont* pFont = getFont( nFont );
    if( !pFont )
        return false;

    switch( pFont->m_eType )
    {
        case psp::fonttype::TrueType: rInfo.m_nFontType = FontSubsetInfo::SFNT_TTF; break;
        case psp::fonttype::Type1: rInfo.m_nFontType = FontSubsetInfo::ANY_TYPE1; break;
        default:
            return false;
    }
    // TODO: remove when Type1 subsetting gets implemented
    if( pFont->m_eType != fonttype::TrueType )
        return false;

    // reshuffle array of requested glyphs to make sure glyph0==notdef
    sal_uInt8  pEnc[256];
    sal_uInt16 pGID[256];
    sal_uInt8  pOldIndex[256];
    memset( pEnc, 0, sizeof( pEnc ) );
    memset( pGID, 0, sizeof( pGID ) );
    memset( pOldIndex, 0, sizeof( pOldIndex ) );
    if( nGlyphs > 256 )
        return false;
    int nChar = 1;
    for( int i = 0; i < nGlyphs; i++ )
    {
        if( pNewEncoding[i] == 0 )
        {
            pOldIndex[ 0 ] = i;
        }
        else
        {
            DBG_ASSERT( !(pGlyphIds[i] & 0x007f0000), "overlong glyph id" );
            DBG_ASSERT( (int)pNewEncoding[i] < nGlyphs, "encoding wrong" );
            DBG_ASSERT( pEnc[pNewEncoding[i]] == 0 && pGID[pNewEncoding[i]] == 0, "duplicate encoded glyph" );
            pEnc[ pNewEncoding[i] ] = pNewEncoding[i];
            pGID[ pNewEncoding[i] ] = (sal_uInt16)pGlyphIds[ i ];
            pOldIndex[ pNewEncoding[i] ] = i;
            nChar++;
        }
    }
    nGlyphs = nChar; // either input value or increased by one

    // prepare system name for read access for subset source file
    // TODO: since this file is usually already mmapped there is no need to open it again
    const OString aFromFile = getFontFile( pFont );

    TrueTypeFont* pTTFont = NULL; // TODO: rename to SfntFont
    TrueTypeFontFile* pTTFontFile = static_cast< TrueTypeFontFile* >(pFont);
    if( OpenTTFontFile( aFromFile.getStr(), pTTFontFile->m_nCollectionEntry, &pTTFont ) != SF_OK )
        return false;

    // prepare system name for write access for subset file target
    OUString aSysPath;
    if( osl_File_E_None != osl_getSystemPathFromFileURL( rOutFile.pData, &aSysPath.pData ) )
        return false;
    const rtl_TextEncoding aEncoding = osl_getThreadTextEncoding();
    const OString aToFile( OUStringToOString( aSysPath, aEncoding ) );

    // do CFF subsetting if possible
    int nCffLength = 0;
    const sal_uInt8* pCffBytes = NULL;
    if( GetSfntTable( pTTFont, O_CFF, &pCffBytes, &nCffLength ) )
    {
        rInfo.LoadFont( FontSubsetInfo::CFF_FONT, pCffBytes, nCffLength );
#if 1 // TODO: remove 16bit->long conversion when related methods handle non-16bit glyphids
        sal_GlyphId aRequestedGlyphIds[256];
        for( int i = 0; i < nGlyphs; ++i )
            aRequestedGlyphIds[i] = pGID[i];
#endif
        // create subset file at requested path
        FILE* pOutFile = fopen( aToFile.getStr(), "wb" );
        if (!pOutFile)
        {
            CloseTTFont( pTTFont );
            return false;
        }
        // create font subset
        const char* pGlyphSetName = NULL; // TODO: better name?
        const bool bOK = rInfo.CreateFontSubset(
            FontSubsetInfo::TYPE1_PFB,
            pOutFile, pGlyphSetName,
            aRequestedGlyphIds, pEnc, nGlyphs, pWidths );
        fclose( pOutFile );
        // cleanup before early return
        CloseTTFont( pTTFont );
        return bOK;
    }

    // do TTF->Type42 or Type3 subsetting
    // fill in font info
    psp::PrintFontInfo aFontInfo;
    if( ! getFontInfo( nFont, aFontInfo ) )
        return false;

    rInfo.m_nAscent     = aFontInfo.m_nAscend;
    rInfo.m_nDescent    = aFontInfo.m_nDescend;
    rInfo.m_aPSName     = getPSName( nFont );

    int xMin, yMin, xMax, yMax;
    getFontBoundingBox( nFont, xMin, yMin, xMax, yMax );
    rInfo.m_aFontBBox   = Rectangle( Point( xMin, yMin ), Size( xMax-xMin, yMax-yMin ) );
    rInfo.m_nCapHeight  = yMax; // Well ...

    // fill in glyph advance widths
    TTSimpleGlyphMetrics* pMetrics = GetTTSimpleGlyphMetrics( pTTFont,
                                                              pGID,
                                                              nGlyphs,
                                                              bVertical );
    if( pMetrics )
    {
        for( int i = 0; i < nGlyphs; i++ )
            pWidths[pOldIndex[i]] = pMetrics[i].adv;
        free( pMetrics );
    }
    else
    {
        CloseTTFont( pTTFont );
        return false;
    }

    bool bSuccess = ( SF_OK == CreateTTFromTTGlyphs( pTTFont,
                                                     aToFile.getStr(),
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
        OString aFromFile = getFontFile( pFont );
        if( OpenTTFontFile( aFromFile.getStr(), pTTFontFile->m_nCollectionEntry, &pTTFont ) != SF_OK )
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
                                                                      bVertical );
            if( pMetrics )
            {
                for( int i = 0; i< nGlyphs; i++ )
                    rWidths[i] = pMetrics[i].adv;
                free( pMetrics );
                rUnicodeEnc.clear();
            }

            // fill the unicode map
            // TODO: isn't this map already available elsewhere in the fontmanager?
            const sal_uInt8* pCmapData = NULL;
            int nCmapSize = 0;
            if( GetSfntTable( pTTFont, O_cmap, &pCmapData, &nCmapSize ) )
            {
                CmapResult aCmapResult;
                if( ParseCMAP( pCmapData, nCmapSize, aCmapResult ) )
                {
                    FontCharMapPtr pCharMap( new FontCharMap(aCmapResult) );
                    for( sal_uInt32 cOld = 0;;)
                    {
                        // get next unicode covered by font
                        const sal_uInt32 c = pCharMap->GetNextChar( cOld );
                        if( c == cOld )
                            break;
                        cOld = c;
#if 1 // TODO: remove when sal_Unicode covers all of unicode
                        if( c > (sal_Unicode)~0 )
                            break;
#endif
                        // get the matching glyph index
                        const sal_GlyphId aGlyphId = pCharMap->GetGlyphIndex( c );
                        // update the requested map
                        rUnicodeEnc[ (sal_Unicode)c ] = aGlyphId;
                    }

                    pCharMap = 0;
                }
            }
        }
        CloseTTFont( pTTFont );
    }
    else if( pFont->m_eType == fonttype::Type1 )
    {
        if( pFont->m_aEncodingVector.empty() )
            pFont->readAfmMetrics( m_pAtoms, true, true );
        if( pFont->m_pMetrics )
        {
            rUnicodeEnc.clear();
            rWidths.clear();
            rWidths.reserve( pFont->m_pMetrics->m_aMetrics.size() );
            for( std::unordered_map< int, CharacterMetric >::const_iterator it =
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

const std::map< sal_Unicode, sal_Int32 >* PrintFontManager::getEncodingMap( fontID nFont, const std::map< sal_Unicode, OString >** pNonEncoded, std::set<sal_Unicode> const** ppPriority ) const
{
    PrintFont* pFont = getFont( nFont );
    if( !pFont || pFont->m_eType != fonttype::Type1 )
        return NULL;

    if( pFont->m_aEncodingVector.empty() )
        pFont->readAfmMetrics( m_pAtoms, true, true );

    if( pNonEncoded )
        *pNonEncoded = !pFont->m_aNonEncoded.empty() ? &pFont->m_aNonEncoded : NULL;

    if (ppPriority)
    {
        *ppPriority = &pFont->m_aEncodingVectorPriority;
    }

    return !pFont->m_aEncodingVector.empty() ? &pFont->m_aEncodingVector : NULL;
}

std::list< OString > PrintFontManager::getAdobeNameFromUnicode( sal_Unicode aChar ) const
{
    std::pair< std::unordered_multimap< sal_Unicode, OString >::const_iterator,
        std::unordered_multimap< sal_Unicode, OString >::const_iterator > range
        =  m_aUnicodeToAdobename.equal_range( aChar );

    std::list< OString > aRet;
    for( ; range.first != range.second; ++range.first )
        aRet.push_back( range.first->second );

    if( aRet.empty() && aChar != 0 )
    {
        sal_Char aBuf[8];
        sal_Int32 nChars = snprintf( aBuf, sizeof( aBuf ), "uni%.4hX", aChar );
        aRet.push_back( OString( aBuf, nChars ) );
    }

    return aRet;
}

std::list< sal_Unicode >  PrintFontManager::getUnicodeFromAdobeName( const OString& rName ) const
{
    std::pair< std::unordered_multimap< OString, sal_Unicode, OStringHash >::const_iterator,
        std::unordered_multimap< OString, sal_Unicode, OStringHash >::const_iterator > range
        =  m_aAdobenameToUnicode.equal_range( rName );

    std::list< sal_Unicode > aRet;
    for( ; range.first != range.second; ++range.first )
        aRet.push_back( range.first->second );

    if( aRet.empty() )
    {
        if( rName.getLength() == 7 && rName.startsWith( "uni" ) )
        {
            sal_Unicode aCode = (sal_Unicode)rName.copy( 3 ).toUInt32( 16 );
            aRet.push_back( aCode );
        }
    }

    return aRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
