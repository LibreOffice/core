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

#include <list>
#include <map>
#include <set>
#include <unordered_map>

#include <vcl/dllapi.h>
#include <vcl/helper.hxx>
#include <vcl/timer.hxx>
#include <vcl/vclenum.hxx>
#include <com/sun/star/lang/Locale.hpp>
#include "salglyphid.hxx"

#include <vector>

#define ATOM_FAMILYNAME                     2
#define ATOM_PSNAME                         3

/*
 *  some words on metrics: every length returned by PrintFontManager and
 *  friends are PostScript afm style, that is they are 1/1000 font height
 */

namespace utl { class MultiAtomProvider; }

class FontSubsetInfo;
class FontConfigFontOptions;
class FontSelectPattern;

namespace psp {
class PPDParser;

namespace fonttype
{
enum type {
    Unknown = 0,
    Type1 = 1,
    TrueType = 2,
};
}

/*
 *  the difference between FastPrintFontInfo and PrintFontInfo
 *  is that the information in FastPrintFontInfo can usually
 *  be gathered without openening either the font file or
 *  an afm metric file. they are gathered from fonts.dir alone.
 *  if only FastPrintFontInfo is gathered and PrintFontInfo
 *  on demand and for less fonts, then performance in startup
 *  increases considerably
 */

struct FastPrintFontInfo
{
    fontID                         m_nID; // FontID
    fonttype::type                 m_eType;

    // font attributes
    OUString                       m_aFamilyName;
    OUString                       m_aStyleName;
    std::list< OUString >          m_aAliases;
    FontFamily                     m_eFamilyStyle;
    FontItalic                     m_eItalic;
    FontWidth                      m_eWidth;
    FontWeight                     m_eWeight;
    FontPitch                      m_ePitch;
    rtl_TextEncoding               m_aEncoding;
    bool                           m_bSubsettable;
    bool                           m_bEmbeddable;

    FastPrintFontInfo()
        : m_nID(0)
        , m_eType(fonttype::Unknown)
        , m_eFamilyStyle(FAMILY_DONTKNOW)
        , m_eItalic(ITALIC_DONTKNOW)
        , m_eWidth(WIDTH_DONTKNOW)
        , m_eWeight(WEIGHT_DONTKNOW)
        , m_ePitch(PITCH_DONTKNOW)
        , m_aEncoding(RTL_TEXTENCODING_DONTKNOW)
        , m_bSubsettable(false)
        , m_bEmbeddable(false)
    {}
};

struct PrintFontInfo : public FastPrintFontInfo
{
    int                                     m_nAscend;
    int                                     m_nDescend;
    int                                     m_nLeading;
    int                                     m_nWidth;

    PrintFontInfo() :
            FastPrintFontInfo(),
            m_nAscend( 0 ),
            m_nDescend( 0 ),
            m_nLeading( 0 ),
            m_nWidth( 0 )
    {}
};

// the values are per thousand of the font size
// note: width, height contain advances, not bounding box
struct CharacterMetric
{
    short int width, height;

    CharacterMetric() : width( 0 ), height( 0 ) {}
    bool operator==( const CharacterMetric& rOther ) const
    { return rOther.width == width && rOther.height == height; }
    bool operator!=( const CharacterMetric& rOther ) const
    { return rOther.width != width || rOther.height != height; }
};

class FontCache;

// a class to manage printable fonts
// aims are type1 and truetype fonts

class FontCache;

class VCL_PLUGIN_PUBLIC PrintFontManager
{
    struct PrintFont;
    struct TrueTypeFontFile;
    struct Type1FontFile;
    friend struct PrintFont;
    friend struct TrueTypeFontFile;
    friend struct Type1FontFile;
    friend class FontCache;

    struct PrintFontMetrics
    {
        // character metrics are stored by the following keys:
        // lower two bytes contain a sal_Unicode (a UCS2 character)
        // upper byte contains: 0 for horizontal metric
        //                      1 for vertical metric
        // highest byte: 0 for now
        std::unordered_map< int, CharacterMetric >     m_aMetrics;
        // contains the unicode blocks for which metrics were queried
        // this implies that metrics should be queried in terms of
        // unicode blocks. here a unicode block is identified
        // by the upper byte of the UCS2 encoding.
        // note that the corresponding bit should be set even
        // if the font does not support a single character of that page
        // this map shows, which pages were queried already
        // if (like in AFM metrics) all metrics are queried in
        // a single pass, then all bits should be set
        char                                        m_aPages[32];

        std::unordered_map< sal_Unicode, bool >       m_bVerticalSubstitutions;

        PrintFontMetrics() {}

        bool isEmpty() const { return m_aMetrics.empty(); }
    };

    struct PrintFont
    {
        fonttype::type                              m_eType;

        // font attributes
        int                                         m_nFamilyName;  // atom
        std::list< int >                            m_aAliases;
        int                                         m_nPSName;      // atom
        OUString                               m_aStyleName;
        FontItalic                                  m_eItalic;
        FontWidth                                   m_eWidth;
        FontWeight                                  m_eWeight;
        FontPitch                                   m_ePitch;
        rtl_TextEncoding                            m_aEncoding;
        bool                                        m_bFontEncodingOnly; // set if font should be only accessed by builtin encoding
        CharacterMetric                             m_aGlobalMetricX;
        CharacterMetric                             m_aGlobalMetricY;
        PrintFontMetrics*                           m_pMetrics;
        int                                         m_nAscend;
        int                                         m_nDescend;
        int                                         m_nLeading;
        int                                         m_nXMin; // font bounding box
        int                                         m_nYMin;
        int                                         m_nXMax;
        int                                         m_nYMax;
        bool                                        m_bHaveVerticalSubstitutedGlyphs;
        bool                                        m_bUserOverride;

        /// mapping from unicode (well, UCS-2) to font code
        std::map< sal_Unicode, sal_Int32 >          m_aEncodingVector;
        /// HACK for Type 1 fonts: if multiple UCS-2 codes map to the same
        /// font code, this set contains the preferred one, i.e., the one that
        /// is specified explicitly via "C" or "CH" in the AFM file
        std::set<sal_Unicode>                  m_aEncodingVectorPriority;
        std::map< sal_Unicode, OString >       m_aNonEncoded;

        explicit PrintFont( fonttype::type eType );
        virtual ~PrintFont();
        virtual bool queryMetricPage( int nPage, utl::MultiAtomProvider* pProvider ) = 0;

        bool readAfmMetrics( utl::MultiAtomProvider* pProvider, bool bFillEncodingvector, bool bOnlyGlobalAttributes );
    };

    struct Type1FontFile : public PrintFont
    {
        int                 m_nDirectory;       // atom containing system dependent path
        OString      m_aFontFile;        // relative to directory
        OString      m_aMetricFile;      // dito

        /* note: m_aFontFile and Metric file are not atoms
           because they should be fairly unique */

        Type1FontFile() : PrintFont( fonttype::Type1 ), m_nDirectory( 0 ) {}
        virtual ~Type1FontFile();
        virtual bool queryMetricPage( int nPage, utl::MultiAtomProvider* pProvider ) SAL_OVERRIDE;
    };

    struct TrueTypeFontFile : public PrintFont
    {
        int           m_nDirectory;       // atom containing system dependent path
        OString  m_aFontFile;        // relative to directory
        int           m_nCollectionEntry; // 0 for regular fonts, 0 to ... for fonts stemming from collections
        unsigned int  m_nTypeFlags;       // copyright bits and PS-OpenType flag

        TrueTypeFontFile();
        virtual ~TrueTypeFontFile();
        virtual bool queryMetricPage( int nPage, utl::MultiAtomProvider* pProvider ) SAL_OVERRIDE;
    };

    fontID                                      m_nNextFontID;
    std::unordered_map< fontID, PrintFont* >       m_aFonts;
    std::unordered_map< int, FontFamily >        m_aFamilyTypes;
    std::list< OUString >              m_aPrinterDrivers;
    std::list< OString >               m_aFontDirectories;
    std::list< int >                            m_aPrivateFontDirectories;
    utl::MultiAtomProvider*                   m_pAtoms;
    // for speeding up findFontFileID
    std::unordered_map< OString, std::set< fontID >, OStringHash >
                                                m_aFontFileToFontID;

    std::unordered_map< OString, int, OStringHash >
    m_aDirToAtom;
    std::unordered_map< int, OString >     m_aAtomToDir;
    int                                        m_nNextDirAtom;

    std::unordered_multimap< OString, sal_Unicode, OStringHash >
        m_aAdobenameToUnicode;
    std::unordered_multimap< sal_Unicode, OString >
        m_aUnicodeToAdobename;
    std::unordered_multimap< sal_Unicode, sal_uInt8 > m_aUnicodeToAdobecode;
    std::unordered_multimap< sal_uInt8, sal_Unicode > m_aAdobecodeToUnicode;

    mutable FontCache*                                                        m_pFontCache;

    OString getAfmFile( PrintFont* pFont ) const;
    OString getFontFile( PrintFont* pFont ) const;

    bool analyzeFontFile( int nDirID, const OString& rFileName, std::list< PrintFont* >& rNewFonts, const char *pFormat=NULL ) const;
    static OUString convertTrueTypeName( void* pNameRecord ); // actually a NameRecord* formt font subsetting code
    static void analyzeTrueTypeFamilyName( void* pTTFont, std::list< OUString >& rnames ); // actually a TrueTypeFont* from font subsetting code
    bool analyzeTrueTypeFile( PrintFont* pFont ) const;
    // finds the font id for the nFaceIndex face in this font file
    // There may be multiple font ids for TrueType collections
    fontID findFontFileID( int nDirID, const OString& rFile, int nFaceIndex ) const;

    // There may be multiple font ids for TrueType collections
    std::vector<fontID> findFontFileIDs( int nDirID, const OString& rFile ) const;

    static FontFamily matchFamilyName( const OUString& rFamily );

    PrintFont* getFont( fontID nID ) const
    {
        std::unordered_map< fontID, PrintFont* >::const_iterator it;
        it = m_aFonts.find( nID );
        return it == m_aFonts.end() ? NULL : it->second;
    }
    void fillPrintFontInfo( PrintFont* pFont, FastPrintFontInfo& rInfo ) const;
    void fillPrintFontInfo( PrintFont* pFont, PrintFontInfo& rInfo ) const;

    OString getDirectory( int nAtom ) const;
    int getDirectoryAtom( const OString& rDirectory, bool bCreate = false );

    /* try to initialize fonts from libfontconfig

    called from <code>initialize()</code>
    */
    static void initFontconfig();
    void countFontconfigFonts( std::unordered_map<OString, int, OStringHash>& o_rVisitedPaths );
    /* deinitialize fontconfig
     */
    static void deinitFontconfig();

    /* register an application specific font directory for libfontconfig

    since fontconfig is asked for font substitutes before OOo will check for font availability
    and fontconfig will happily substitute fonts it doesn't know (e.g. "Arial Narrow" -> "DejaVu Sans Book"!)
    it becomes necessary to tell the library about all the hidden font treasures

    @returns
    true if libfontconfig accepted the directory
    false else (e.g. no libfontconfig found)
    */
    static bool addFontconfigDir(const OString& rDirectory);

    std::set<OString> m_aPreviousLangSupportRequests;
    std::vector<OString> m_aCurrentRequests;
    Timer m_aFontInstallerTimer;

#if defined(ENABLE_DBUS) && defined(ENABLE_PACKAGEKIT)
    DECL_LINK_TYPED( autoInstallFontLangSupport, Timer*, void );
#endif
    PrintFontManager();
    ~PrintFontManager();
public:
    static PrintFontManager& get(); // one instance only

    // There may be multiple font ids for TrueType collections
    std::vector<fontID> addFontFile( const OString& rFileName );

    void initialize();

    // returns the ids of all managed fonts.
    void getFontList( std::list< fontID >& rFontIDs );

    // get font info for a specific font
    bool getFontInfo( fontID nFontID, PrintFontInfo& rInfo ) const;
    // get fast font info for a specific font
    bool getFontFastInfo( fontID nFontID, FastPrintFontInfo& rInfo ) const;

    // routines to get font info in small pieces

    // get a specific fonts PSName name
    const OUString& getPSName( fontID nFontID ) const;

    // get a specific fonts type
    fonttype::type getFontType( fontID nFontID ) const
    {
        PrintFont* pFont = getFont( nFontID );
        return pFont ? pFont->m_eType : fonttype::Unknown;
    }

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

    // get a specific fonts encoding
    rtl_TextEncoding getFontEncoding( fontID nFontID ) const
    {
        PrintFont* pFont = getFont( nFontID );
        return pFont ? pFont->m_aEncoding : RTL_TEXTENCODING_DONTKNOW;
    }

    // should i only use font's builtin encoding ?
    bool getUseOnlyFontEncoding( fontID nFontID ) const
    {
        PrintFont* pFont = getFont( nFontID );
        return pFont && pFont->m_bFontEncodingOnly;
    }

    // get a specific fonts system dependent filename
    OString getFontFileSysPath( fontID nFontID ) const
    {
        return getFontFile( getFont( nFontID ) );
    }

    // get the ttc face number
    int getFontFaceNumber( fontID nFontID ) const;

    // get a specific fonts ascend
    int getFontAscend( fontID nFontID ) const;

    // get a specific fonts descent
    int getFontDescend( fontID nFontID ) const;

    // get a fonts glyph bounding box
    bool getFontBoundingBox( fontID nFont, int& xMin, int& yMin, int& xMax, int& yMax );

    // info whether an array of glyphs has vertical substitutions
    void hasVerticalSubstitutions( fontID nFontID, const sal_Unicode* pCharacters,
        int nCharacters, bool* pHasSubst ) const;

    // get a specific fonts metrics

    // get metrics for a sal_Unicode range
    // the user is responsible to allocate pArray large enough
    bool getMetrics( fontID nFontID, sal_Unicode minCharacter, sal_Unicode maxCharacter, CharacterMetric* pArray, bool bVertical = false ) const;
    // get metrics for an array of sal_Unicode characters
    // the user is responsible to allocate pArray large enough
    bool getMetrics( fontID nFontID, const sal_Unicode* pString, int nLen, CharacterMetric* pArray, bool bVertical = false ) const;

    // get encoding vector of font, currently only for Type1 fonts
    // returns NULL if encoding vector is empty or font is not type1;
    // if ppNonEncoded is set and non encoded type1 glyphs exist
    // then *ppNonEncoded is set to the mapping for nonencoded glyphs.
    // the encoding vector contains -1 for non encoded glyphs
    const std::map< sal_Unicode, sal_Int32 >* getEncodingMap( fontID nFontID, const std::map< sal_Unicode, OString >** ppNonEncoded, std::set<sal_Unicode> const ** ppPriority ) const;

    // evaluates copyright flags for TrueType fonts for printing/viewing
    // type1 fonts do not have such a feature, so return for them is true
    bool isFontDownloadingAllowedForPrinting( fontID nFont ) const;

    // helper for type 1 fonts
    std::list< OString > getAdobeNameFromUnicode( sal_Unicode aChar ) const;

    std::list< sal_Unicode >  getUnicodeFromAdobeName( const OString& rName ) const;
    std::pair< std::unordered_multimap< sal_uInt8, sal_Unicode >::const_iterator,
                 std::unordered_multimap< sal_uInt8, sal_Unicode >::const_iterator >
    getUnicodeFromAdobeCode( sal_uInt8 aChar ) const
    {
        return m_aAdobecodeToUnicode.equal_range( aChar );
    }

    // creates a new font subset of an existing TrueType font
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
                           int nGlyphs,
                           bool bVertical = false
                           );
    void getGlyphWidths( fontID nFont,
                         bool bVertical,
                         std::vector< sal_Int32 >& rWidths,
                         std::map< sal_Unicode, sal_uInt32 >& rUnicodeEnc );

    // font administration functions

    /*  system dependendent font matching

    <p>
    <code>matchFont</code> matches a pattern of font characteristics
    and returns the closest match if possibe. If a match was found
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

    @returns
    true if a match was found
    false else
     */
    bool matchFont( FastPrintFontInfo& rInfo, const com::sun::star::lang::Locale& rLocale );
    static FontConfigFontOptions* getFontOptions( const FastPrintFontInfo&, int nSize, void (*subcallback)(void*));

    bool Substitute( FontSelectPattern &rPattern, OUString& rMissingCodes );

};

} // namespace

#endif // INCLUDED_VCL_INC_FONTMANAGER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
