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

#include <memory>
#include <osl/thread.h>

#include <unx/fontmanager.hxx>
#include <impfontcharmap.hxx>
#include <unotools/syslocaleoptions.hxx>
#include <unx/gendata.hxx>
#include <unx/helper.hxx>
#include <vcl/fontcharmap.hxx>

#include <tools/urlobj.hxx>
#include <tools/UnixWrappers.h>

#include <o3tl/string_view.hxx>
#include <osl/file.hxx>

#include <rtl/ustrbuf.hxx>
#include <rtl/strbuf.hxx>

#include <sal/macros.h>
#include <sal/log.hxx>

#include <i18nlangtag/applelangid.hxx>

#include <sft.hxx>

#if OSL_DEBUG_LEVEL > 1
#include <sys/times.h>
#include <stdio.h>
#endif

#include <algorithm>
#include <set>

#ifdef CALLGRIND_COMPILE
#include <valgrind/callgrind.h>
#endif

#include <com/sun/star/beans/XMaterialHolder.hpp>

using namespace vcl;
using namespace psp;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;

/*
 *  PrintFont implementations
 */
PrintFontManager::PrintFont::PrintFont()
:   m_nDirectory(0)
,   m_nCollectionEntry(0)
,   m_nVariationEntry(0)
{
}

/*
 *  one instance only
 */
PrintFontManager& PrintFontManager::get()
{
    GenericUnixSalData* const pSalData(GetGenericUnixSalData());
    assert(pSalData);
    return *pSalData->GetPrintFontManager();
}

/*
 *  the PrintFontManager
 */

PrintFontManager::PrintFontManager()
    : m_nNextFontID( 1 )
    , m_nNextDirAtom( 1 )
    , m_aFontInstallerTimer("PrintFontManager m_aFontInstallerTimer")
{
    m_aFontInstallerTimer.SetInvokeHandler(LINK(this, PrintFontManager, autoInstallFontLangSupport));
    m_aFontInstallerTimer.SetTimeout(5000);
}

PrintFontManager::~PrintFontManager()
{
    m_aFontInstallerTimer.Stop();
    deinitFontconfig();
}

OString PrintFontManager::getDirectory( int nAtom ) const
{
    std::unordered_map< int, OString >::const_iterator it( m_aAtomToDir.find( nAtom ) );
    return it != m_aAtomToDir.end() ? it->second : OString();
}

int PrintFontManager::getDirectoryAtom( const OString& rDirectory )
{
    int nAtom = 0;
    std::unordered_map< OString, int >::const_iterator it
          ( m_aDirToAtom.find( rDirectory ) );
    if( it != m_aDirToAtom.end() )
        nAtom = it->second;
    else
    {
        nAtom = m_nNextDirAtom++;
        m_aDirToAtom[ rDirectory ] = nAtom;
        m_aAtomToDir[ nAtom ] = rDirectory;
    }
    return nAtom;
}

std::vector<fontID> PrintFontManager::findFontFileIDs( std::u16string_view rFileUrl ) const
{
    rtl_TextEncoding aEncoding = osl_getThreadTextEncoding();
    INetURLObject aPath( rFileUrl );
    OString aName(OUStringToOString(aPath.GetLastName(INetURLObject::DecodeMechanism::WithCharset, aEncoding), aEncoding));
    OString aDir( OUStringToOString(
        INetURLObject::decode( aPath.GetPath(), INetURLObject::DecodeMechanism::WithCharset, aEncoding ), aEncoding ) );

    auto dirIt = m_aDirToAtom.find(aDir);
    if (dirIt == m_aDirToAtom.end())
        return {};

    return findFontFileIDs(dirIt->second, aName);
}

std::vector<fontID> PrintFontManager::addFontFile( std::u16string_view rFileUrl )
{
    rtl_TextEncoding aEncoding = osl_getThreadTextEncoding();
    INetURLObject aPath( rFileUrl );
    OString aName(OUStringToOString(aPath.GetLastName(INetURLObject::DecodeMechanism::WithCharset, aEncoding), aEncoding));
    OString aDir( OUStringToOString(
        INetURLObject::decode( aPath.GetPath(), INetURLObject::DecodeMechanism::WithCharset, aEncoding ), aEncoding ) );

    int nDirID = getDirectoryAtom( aDir );
    std::vector<fontID> aFontIds = findFontFileIDs( nDirID, aName );
    if( aFontIds.empty() )
    {
        addFontconfigFile(OUStringToOString(aPath.GetFull(), osl_getThreadTextEncoding()));

        std::vector<PrintFont> aNewFonts = analyzeFontFile(nDirID, aName);
        for (auto & font : aNewFonts)
        {
            fontID nFontId = m_nNextFontID++;
            m_aFonts[nFontId] = std::move(font);
            m_aFontFileToFontID[ aName ].insert( nFontId );
            aFontIds.push_back(nFontId);
        }
    }
    return aFontIds;
}

void PrintFontManager::removeFontFile(std::u16string_view rFileUrl)
{
    INetURLObject aPath(rFileUrl);
    rtl_TextEncoding aEncoding = osl_getThreadTextEncoding();
    if (auto ids = findFontFileIDs(rFileUrl); !ids.empty())
    {
        OString aName(OUStringToOString(
            aPath.GetLastName(INetURLObject::DecodeMechanism::WithCharset, aEncoding), aEncoding));

        for (auto nFontID : ids)
        {
            m_aFonts.erase(nFontID);
            m_aFontFileToFontID[aName].erase(nFontID);
        }
    }

    removeFontconfigFile(OUStringToOString(aPath.GetFull(), aEncoding));
}

std::vector<PrintFontManager::PrintFont> PrintFontManager::analyzeFontFile( int nDirID, const OString& rFontFile) const
{
    std::vector<PrintFontManager::PrintFont> aNewFonts;

    OString aDir( getDirectory( nDirID ) );

    OString aFullPath = aDir + "/" + rFontFile;

    bool bSupported;
    int nFD;
    int n;
    if (sscanf(aFullPath.getStr(), "/:FD:/%d%n", &nFD, &n) == 1 && aFullPath.getStr()[n] == '\0')
    {
        // Hack, pathname that actually means we will use a pre-opened file descriptor
        bSupported = true;
    }
    else
    {
        // #i1872# reject unreadable files
        if( wrap_access( aFullPath.getStr(), R_OK ) )
            return aNewFonts;

        bSupported = false;
        OString aExt( rFontFile.copy( rFontFile.lastIndexOf( '.' )+1 ) );
        if( aExt.equalsIgnoreAsciiCase("ttf")
             ||  aExt.equalsIgnoreAsciiCase("ttc")
             ||  aExt.equalsIgnoreAsciiCase("tte")   // #i33947# for Gaiji support
             ||  aExt.equalsIgnoreAsciiCase("otf") ) // check for TTF- and PS-OpenType too
            bSupported = true;
    }

    if (bSupported)
    {
        // get number of ttc entries
        int nLength = CountTTCFonts( aFullPath.getStr() );
        if (nLength > 0)
        {
            SAL_INFO("vcl.fonts", "ttc: " << aFullPath << " contains " << nLength << " fonts");

            for( int i = 0; i < nLength; i++ )
            {
                PrintFont aFont;
                aFont.m_nDirectory         = nDirID;
                aFont.m_aFontFile          = rFontFile;
                aFont.m_nCollectionEntry   = i;
                if (analyzeSfntFile(aFont))
                    aNewFonts.push_back(aFont);
            }
        }
        else
        {
            PrintFont aFont;
            aFont.m_nDirectory         = nDirID;
            aFont.m_aFontFile          = rFontFile;
            aFont.m_nCollectionEntry   = 0;

            // need to read the font anyway to get aliases inside the font file
            if (analyzeSfntFile(aFont))
                aNewFonts.push_back(aFont);
        }
    }
    return aNewFonts;
}

fontID PrintFontManager::findFontFileID(int nDirID, const OString& rFontFile, int nFaceIndex, int nVariationIndex) const
{
    fontID nID = 0;

    auto set_it = m_aFontFileToFontID.find( rFontFile );
    if( set_it == m_aFontFileToFontID.end() )
        return nID;

    for (fontID elem : set_it->second)
    {
        auto it = m_aFonts.find(elem);
        if( it == m_aFonts.end() )
            continue;
        const PrintFont& rFont = (*it).second;
        if (rFont.m_nDirectory == nDirID &&
            rFont.m_aFontFile == rFontFile &&
            rFont.m_nCollectionEntry == nFaceIndex &&
            rFont.m_nVariationEntry == nVariationIndex)
        {
            nID = it->first;
            if (nID)
                break;
        }
    }

    return nID;
}

std::vector<fontID> PrintFontManager::findFontFileIDs( int nDirID, const OString& rFontFile ) const
{
    std::vector<fontID> aIds;

    auto set_it = m_aFontFileToFontID.find( rFontFile );
    if( set_it == m_aFontFileToFontID.end() )
        return aIds;

    for (auto const& elem : set_it->second)
    {
        auto it = m_aFonts.find(elem);
        if( it == m_aFonts.end() )
            continue;
        const PrintFont& rFont = (*it).second;
        if (rFont.m_nDirectory == nDirID &&
            rFont.m_aFontFile == rFontFile)
            aIds.push_back(it->first);
    }

    return aIds;
}

namespace {

OUString analyzeSfntFamilyName(void const * pTTFont)
{
    return analyzeSfntName(static_cast<TrueTypeFont const *>(pTTFont), 1, SvtSysLocaleOptions().GetRealUILanguageTag());
}

}

bool PrintFontManager::analyzeSfntFile( PrintFont& rFont ) const
{
    bool bSuccess = false;
    rtl_TextEncoding aEncoding = osl_getThreadTextEncoding();
    OString aFile = getFontFile( rFont );
    TrueTypeFont* pTTFont = nullptr;

    auto& rDFA = rFont.m_aFontAttributes;
    rDFA.SetQuality(512);

    auto const e = OpenTTFontFile( aFile.getStr(), rFont.m_nCollectionEntry, &pTTFont );
    if( e == SFErrCodes::Ok )
    {
        TTGlobalFontInfo aInfo;
        GetTTGlobalFontInfo( pTTFont, & aInfo );

        if (rDFA.GetFamilyName().isEmpty())
        {
            OUString aFamily = analyzeSfntFamilyName(pTTFont);
            if (aFamily.isEmpty())
            {
                 // poor font does not have a family name
                 // name it to file name minus the extension
                 sal_Int32 dotIndex = rFont.m_aFontFile.lastIndexOf('.');
                 if ( dotIndex == -1 )
                     dotIndex = rFont.m_aFontFile.getLength();
                 aFamily = OStringToOUString(rFont.m_aFontFile.subView(0, dotIndex), aEncoding);
            }

            rDFA.SetFamilyName(aFamily);
        }

        if( !aInfo.usubfamily.isEmpty() )
            rDFA.SetStyleName(aInfo.usubfamily);

        rDFA.SetFamilyType(matchFamilyName(rDFA.GetFamilyName()));

        rDFA.SetWeight(AnalyzeTTFWeight(pTTFont));

        switch( aInfo.width )
        {
            case FWIDTH_ULTRA_CONDENSED:    rDFA.SetWidthType(WIDTH_ULTRA_CONDENSED); break;
            case FWIDTH_EXTRA_CONDENSED:    rDFA.SetWidthType(WIDTH_EXTRA_CONDENSED); break;
            case FWIDTH_CONDENSED:          rDFA.SetWidthType(WIDTH_CONDENSED); break;
            case FWIDTH_SEMI_CONDENSED:     rDFA.SetWidthType(WIDTH_SEMI_CONDENSED); break;
            case FWIDTH_SEMI_EXPANDED:      rDFA.SetWidthType(WIDTH_SEMI_EXPANDED); break;
            case FWIDTH_EXPANDED:           rDFA.SetWidthType(WIDTH_EXPANDED); break;
            case FWIDTH_EXTRA_EXPANDED:     rDFA.SetWidthType(WIDTH_EXTRA_EXPANDED); break;
            case FWIDTH_ULTRA_EXPANDED:     rDFA.SetWidthType(WIDTH_ULTRA_EXPANDED); break;

            case FWIDTH_NORMAL:
            default:                        rDFA.SetWidthType(WIDTH_NORMAL); break;
        }

        rDFA.SetPitch(aInfo.pitch ? PITCH_FIXED : PITCH_VARIABLE);
        rDFA.SetItalic(aInfo.italicAngle == 0 ? ITALIC_NONE : (aInfo.italicAngle < 0 ? ITALIC_NORMAL : ITALIC_OBLIQUE));
        // #104264# there are fonts that set italic angle 0 although they are
        // italic; use macstyle bit here
        if( aInfo.italicAngle == 0 && (aInfo.macStyle & 2) )
            rDFA.SetItalic(ITALIC_NORMAL);

        rDFA.SetMicrosoftSymbolEncoded(aInfo.microsoftSymbolEncoded);

        CloseTTFont( pTTFont );
        bSuccess = true;
    }
    else
        SAL_WARN("vcl.fonts", "Could not OpenTTFont \"" << aFile << "\": " << int(e));

    return bSuccess;
}

void PrintFontManager::initialize()
{
    #ifdef CALLGRIND_COMPILE
    CALLGRIND_TOGGLE_COLLECT();
    CALLGRIND_ZERO_STATS();
    #endif

    // initialize can be called more than once, e.g.
    // gtk-fontconfig-timestamp changes to reflect new font installed and
    // PrintFontManager::initialize called again
    {
        m_nNextFontID = 1;
        m_aFonts.clear();
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
        sal_Int32 nIndex = 0;
        do
        {
            OString aToken = aPath.getToken( 0, ';', nIndex );
            normPath( aToken );
            if (!aToken.isEmpty())
                addFontconfigDir(aToken);
        } while( nIndex >= 0 );
    }

    countFontconfigFonts();

#if OSL_DEBUG_LEVEL > 1
    aStep1 = times( &tms );

    aStep2 = times( &tms );
    SAL_INFO("vcl.fonts", "PrintFontManager::initialize: collected "
            << m_aFonts.size()
            << " fonts.");
    double fTick = (double)sysconf( _SC_CLK_TCK );
    SAL_INFO("vcl.fonts", "Step 1 took "
            << ((double)(aStep1 - aStart)/fTick)
            << " seconds.");
    SAL_INFO("vcl.fonts", "Step 2 took "
            << ((double)(aStep2 - aStep1)/fTick)
            << " seconds.");
#endif

    #ifdef CALLGRIND_COMPILE
    CALLGRIND_DUMP_STATS();
    CALLGRIND_TOGGLE_COLLECT();
    #endif
}

void PrintFontManager::getFontList( ::std::vector< fontID >& rFontIDs )
{
    rFontIDs.clear();

    for (auto const& font : m_aFonts)
        rFontIDs.push_back(font.first);
}

int PrintFontManager::getFontFaceNumber( fontID nFontID ) const
{
    int nRet = 0;
    const PrintFont* pFont = getFont( nFontID );
    if (pFont)
    {
        nRet = pFont->m_nCollectionEntry;
        if (nRet < 0)
            nRet = 0;
    }
    return nRet;
}

int PrintFontManager::getFontFaceVariation( fontID nFontID ) const
{
    int nRet = 0;
    const PrintFont* pFont = getFont( nFontID );
    if (pFont)
    {
        nRet = pFont->m_nVariationEntry;
        if (nRet < 0)
            nRet = 0;
    }
    return nRet;
}

FontFamily PrintFontManager::matchFamilyName( std::u16string_view rFamily )
{
    struct family_t {
        const char*  mpName;
        sal_uInt16   mnLength;
        FontFamily   meType;
    };

#define InitializeClass( p, a ) p, sizeof(p) - 1, a
    static const family_t pFamilyMatch[] =  {
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

OString PrintFontManager::getFontFile(const PrintFont& rFont) const
{
    std::unordered_map< int, OString >::const_iterator it = m_aAtomToDir.find(rFont.m_nDirectory);
    assert(it != m_aAtomToDir.end());
    OString aPath = it->second + "/" + rFont.m_aFontFile;
    return aPath;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
