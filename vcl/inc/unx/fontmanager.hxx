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

#ifndef INCLUDED_VCL_INC_FONTMANAGER_HXX
#define INCLUDED_VCL_INC_FONTMANAGER_HXX

#include <config_options.h>
#include <tools/fontenum.hxx>
#include <vcl/dllapi.h>
#include <vcl/glyphitem.hxx>
#include <vcl/timer.hxx>
#include <com/sun/star/lang/Locale.hpp>
#include <unx/fc_fontoptions.hxx>

#include <map>
#include <set>
#include <memory>
#include <string_view>
#include <vector>
#include <unordered_map>

/*
 *  some words on metrics: every length returned by PrintFontManager and
 *  friends are PostScript afm style, that is they are 1/1000 font height
 */

class FontAttributes;
class FontSubsetInfo;
class FontConfigFontOptions;
class FontSelectPattern;
class GenericUnixSalData;

namespace psp {
class PPDParser;

typedef int fontID;

/*
 *  the difference between FastPrintFontInfo and PrintFontInfo
 *  is that the information in FastPrintFontInfo can usually
 *  be gathered without opening either the font file, they are
 *  gathered from fonts.dir alone.
 *  if only FastPrintFontInfo is gathered and PrintFontInfo
 *  on demand and for less fonts, then performance in startup
 *  increases considerably
 */

struct FastPrintFontInfo
{
    fontID                         m_nID; // FontID

    // font attributes
    OUString                       m_aFamilyName;
    OUString                       m_aStyleName;
    std::vector< OUString >        m_aAliases;
    FontFamily                     m_eFamilyStyle;
    FontItalic                     m_eItalic;
    FontWidth                      m_eWidth;
    FontWeight                     m_eWeight;
    FontPitch                      m_ePitch;
    rtl_TextEncoding               m_aEncoding;

    FastPrintFontInfo()
        : m_nID(0)
        , m_eFamilyStyle(FAMILY_DONTKNOW)
        , m_eItalic(ITALIC_DONTKNOW)
        , m_eWidth(WIDTH_DONTKNOW)
        , m_eWeight(WEIGHT_DONTKNOW)
        , m_ePitch(PITCH_DONTKNOW)
        , m_aEncoding(RTL_TEXTENCODING_DONTKNOW)
    {}
};

struct PrintFontInfo : public FastPrintFontInfo
{
    int                                     m_nAscend;
    int                                     m_nDescend;

    PrintFontInfo() :
            FastPrintFontInfo(),
            m_nAscend( 0 ),
            m_nDescend( 0 )
    {}
};

// a class to manage printable fonts

class VCL_PLUGIN_PUBLIC PrintFontManager
{
    struct PrintFont;
    friend struct PrintFont;

    struct VCL_DLLPRIVATE PrintFont
    {
        // font attributes
        OUString          m_aFamilyName;
        std::vector<OUString> m_aAliases;
        OUString          m_aPSName;
        OUString          m_aStyleName;
        FontFamily        m_eFamilyStyle;
        FontItalic        m_eItalic;
        FontWidth         m_eWidth;
        FontWeight        m_eWeight;
        FontPitch         m_ePitch;
        rtl_TextEncoding  m_aEncoding;
        int               m_nAscend;
        int               m_nDescend;
        int               m_nLeading;
        int               m_nXMin; // font bounding box
        int               m_nYMin;
        int               m_nXMax;
        int               m_nYMax;

        int               m_nDirectory;       // atom containing system dependent path
        OString           m_aFontFile;        // relative to directory
        int               m_nCollectionEntry; // 0 for regular fonts, 0 to ... for fonts stemming from collections
        int               m_nVariationEntry;  // 0 for regular fonts, 0 to ... for fonts stemming from font variations

        explicit PrintFont();
    };

    fontID                                      m_nNextFontID;
    std::unordered_map< fontID, std::unique_ptr<PrintFont> >    m_aFonts;
    // for speeding up findFontFileID
    std::unordered_map< OString, std::set< fontID > >
                                                m_aFontFileToFontID;

    std::unordered_map< OString, int >
    m_aDirToAtom;
    std::unordered_map< int, OString >          m_aAtomToDir;
    int                                         m_nNextDirAtom;

    OString getFontFile(const PrintFont* pFont) const;

    std::vector<std::unique_ptr<PrintFont>> analyzeFontFile(int nDirID, const OString& rFileName, const char *pFormat=nullptr) const;
    static OUString convertSfntName( void* pNameRecord ); // actually a NameRecord* format font subsetting code
    static void analyzeSfntFamilyName( void const * pTTFont, std::vector< OUString >& rnames ); // actually a TrueTypeFont* from font subsetting code
    bool analyzeSfntFile(PrintFont* pFont) const;
    // finds the font id for the nFaceIndex face in this font file
    // There may be multiple font ids for font collections
    fontID findFontFileID(int nDirID, const OString& rFile, int nFaceIndex, int nVariationIndex) const;

    // There may be multiple font ids for font collections
    std::vector<fontID> findFontFileIDs( int nDirID, const OString& rFile ) const;

    static FontFamily matchFamilyName( std::u16string_view rFamily );

    PrintFont* getFont( fontID nID ) const
    {
        auto it = m_aFonts.find( nID );
        return it == m_aFonts.end() ? nullptr : it->second.get();
    }
    static void fillPrintFontInfo(PrintFont* pFont, FastPrintFontInfo& rInfo);
    void fillPrintFontInfo( PrintFont* pFont, PrintFontInfo& rInfo ) const;

    OString getDirectory( int nAtom ) const;
    int getDirectoryAtom( const OString& rDirectory );

    /* try to initialize fonts from libfontconfig

    called from <code>initialize()</code>
    */
    static void initFontconfig();
    void countFontconfigFonts( std::unordered_map<OString, int>& o_rVisitedPaths );
    /* deinitialize fontconfig
     */
    static void deinitFontconfig();

    /* register an application specific font directory for libfontconfig

    since fontconfig is asked for font substitutes before OOo will check for font availability
    and fontconfig will happily substitute fonts it doesn't know (e.g. "Arial Narrow" -> "DejaVu Sans Book"!)
    it becomes necessary to tell the library about all the hidden font treasures
    */
    static void addFontconfigDir(const OString& rDirectory);

    std::set<OString> m_aPreviousLangSupportRequests;
    std::vector<OUString> m_aCurrentRequests;
    Timer m_aFontInstallerTimer;

    DECL_LINK( autoInstallFontLangSupport, Timer*, void );
    PrintFontManager();
public:
    ~PrintFontManager();
    friend class ::GenericUnixSalData;
    static PrintFontManager& get(); // one instance only

    // There may be multiple font ids for font collections
    std::vector<fontID> addFontFile( const OUString& rFileUrl );

    void initialize();

    // returns the ids of all managed fonts.
    void getFontList( std::vector< fontID >& rFontIDs );

    // get font info for a specific font
    bool getFontInfo( fontID nFontID, PrintFontInfo& rInfo ) const;
    // get fast font info for a specific font
    bool getFontFastInfo( fontID nFontID, FastPrintFontInfo& rInfo ) const;

    // routines to get font info in small pieces

    // get a specific fonts PSName name
    OUString getPSName( fontID nFontID ) const;

    // get a specific fonts italic type
    FontItalic getFontItalic( fontID nFontID ) const
    {
        PrintFont* pFont = getFont( nFontID );
        return pFont ? pFont->m_eItalic : ITALIC_DONTKNOW;
    }

    // get a specific fonts weight type
    FontWeight getFontWeight( fontID nFontID ) const
    {
        PrintFont* pFont = getFont( nFontID );
        return pFont ? pFont->m_eWeight : WEIGHT_DONTKNOW;
    }

    // get a specific fonts system dependent filename
    OString getFontFileSysPath( fontID nFontID ) const
    {
        return getFontFile( getFont( nFontID ) );
    }

    // get the ttc face number
    int getFontFaceNumber( fontID nFontID ) const;

    // get the ttc face variation
    int getFontFaceVariation( fontID nFontID ) const;

    // get a specific fonts ascend
    int getFontAscend( fontID nFontID ) const;

    // get a specific fonts descent
    int getFontDescend( fontID nFontID ) const;

    // get a fonts glyph bounding box
    void getFontBoundingBox( fontID nFont, int& xMin, int& yMin, int& xMax, int& yMax );

    // creates a new font subset of an existing SFNT font
    // returns true in case of success, else false
    // nFont: the font to be subsetted
    // rOutFile: the file to put the new subset into;
    //           must be a valid osl file URL
    // pGlyphIDs: input array of glyph ids for new font
    // pNewEncoding: the corresponding encoding in the new font
    // pWidths: output array of widths of requested glyphs
    // nGlyphs: number of glyphs in arrays
    // pCapHeight:: capital height of the produced font
    // pXMin, pYMin, pXMax, pYMax: outgoing font bounding box
    // TODO: callers of this method should use its FontSubsetInfo counterpart directly
    bool createFontSubset( FontSubsetInfo&,
                           fontID nFont,
                           const OUString& rOutFile,
                           const sal_GlyphId* pGlyphIDs,
                           const sal_uInt8* pNewEncoding,
                           sal_Int32* pWidths,
                           int nGlyphs
                           );
    void getGlyphWidths( fontID nFont,
                         bool bVertical,
                         std::vector< sal_Int32 >& rWidths,
                         std::map< sal_Unicode, sal_uInt32 >& rUnicodeEnc );

    // font administration functions

    /*  system dependent font matching

    <p>
    <code>matchFont</code> matches a pattern of font characteristics
    and returns the closest match if possible. If a match was found
    the <code>FastPrintFontInfo</code> passed in as parameter
    will be update to the found matching font.
    </p>
    <p>
    implementation note: currently the function is only implemented
    for fontconfig.
    </p>

    @param rInfo
    out of the FastPrintFontInfo structure the following
    fields will be used for the match:
    <ul>
    <li>family name</li>
    <li>italic</li>
    <li>width</li>
    <li>weight</li>
    <li>pitch</li>
    </ul>

    @param rLocale
    if <code>rLocal</code> contains non empty strings the corresponding
    locale will be used for font matching also; e.g. "Sans" can result
    in different fonts in e.g. english and japanese
     */
    void matchFont( FastPrintFontInfo& rInfo, const css::lang::Locale& rLocale );

    static std::unique_ptr<FontConfigFontOptions> getFontOptions(const FontAttributes& rFontAttributes, int nSize);

    void Substitute(FontSelectPattern &rPattern, OUString& rMissingCodes);

};

} // namespace

#endif // INCLUDED_VCL_INC_FONTMANAGER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
