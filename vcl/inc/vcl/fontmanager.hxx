/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

#ifndef _PSPRINT_FONTMANAGER_HXX_
#define _PSPRINT_FONTMANAGER_HXX_

#include <boost/unordered_map.hpp>
#include <map>
#include <list>
#include <set>

#include "vcl/dllapi.h"
#include "vcl/helper.hxx"
#include "vcl/timer.hxx"
#include "vcl/vclenum.hxx"
#include "com/sun/star/lang/Locale.hpp"

#include <vector>

#define ATOM_FAMILYNAME                     2
#define ATOM_PSNAME                         3

/*
 *  some words on metrics: every length returned by PrintFontManager and
 *  friends are PostScript afm style, that is they are 1/1000 font height
 */

// forward declarations
namespace utl { class MultiAtomProvider; } // see unotools/atom.hxx
class FontSubsetInfo;
class ImplFontOptions;
class FontSelectPattern;

namespace psp {
class PPDParser; // see ppdparser.hxx

namespace fonttype
{
enum type {
    Unknown = 0,
    Type1 = 1,
    TrueType = 2,
    Builtin = 3
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
    fontID                              m_nID; // FontID
    fonttype::type                      m_eType;

    // font attributes
    rtl::OUString                       m_aFamilyName;
    rtl::OUString                       m_aStyleName;
    std::list< rtl::OUString >          m_aAliases;
    FontFamily                          m_eFamilyStyle;
    FontItalic                          m_eItalic;
    FontWidth                           m_eWidth;
    FontWeight                          m_eWeight;
    FontPitch                           m_ePitch;
    rtl_TextEncoding                    m_aEncoding;
    bool                                m_bSubsettable;
    bool                                m_bEmbeddable;

    FastPrintFontInfo() :
            m_nID( 0 ),
            m_eType( fonttype::Unknown ),
            m_eFamilyStyle( FAMILY_DONTKNOW ),
            m_eItalic( ITALIC_DONTKNOW ),
            m_eWidth( WIDTH_DONTKNOW ),
            m_eWeight( WEIGHT_DONTKNOW ),
            m_ePitch( PITCH_DONTKNOW ),
            m_aEncoding( RTL_TEXTENCODING_DONTKNOW )
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

struct KernPair
{
    sal_Unicode first, second;
    short int kern_x, kern_y;

    KernPair() : first( 0 ), second( 0 ), kern_x( 0 ), kern_y( 0 ) {}
};

class FontCache;

// a class to manage printable fonts
// aims are type1 and truetype fonts

class FontCache;

class VCL_DLLPUBLIC PrintFontManager
{
    struct PrintFont;
    struct TrueTypeFontFile;
    struct Type1FontFile;
    struct BuiltinFont;
    friend struct PrintFont;
    friend struct TrueTypeFontFile;
    friend struct Type1FontFile;
    friend struct BuiltinFont;
    friend class FontCache;

    struct PrintFontMetrics
    {
        // character metrics are stored by the following keys:
        // lower two bytes contain a sal_Unicode (a UCS2 character)
        // upper byte contains: 0 for horizontal metric
        //                      1 for vertical metric
        // highest byte: 0 for now
        boost::unordered_map< int, CharacterMetric >     m_aMetrics;
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

        bool                                        m_bKernPairsQueried;
        std::list< KernPair >                     m_aXKernPairs;
        std::list< KernPair >                     m_aYKernPairs;
        boost::unordered_map< sal_Unicode, bool >       m_bVerticalSubstitutions;

        PrintFontMetrics() : m_bKernPairsQueried( false ) {}

        bool isEmpty() const { return m_aMetrics.empty(); }
    };

    struct PrintFont
    {
        fonttype::type                              m_eType;

        // font attributes
        int                                         m_nFamilyName;  // atom
        std::list< int >                            m_aAliases;
        int                                         m_nPSName;      // atom
        rtl::OUString                               m_aStyleName;
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

        std::map< sal_Unicode, sal_Int32 >          m_aEncodingVector;
        std::map< sal_Unicode, rtl::OString >       m_aNonEncoded;

        PrintFont( fonttype::type eType );
        virtual ~PrintFont();
        virtual bool queryMetricPage( int nPage, utl::MultiAtomProvider* pProvider ) = 0;

        bool readAfmMetrics( const rtl::OString& rFileName, utl::MultiAtomProvider* pProvider, bool bFillEncodingvector, bool bOnlyGlobalAttributes );
    };

    struct Type1FontFile : public PrintFont
    {
        int                 m_nDirectory;       // atom containing system dependent path
        rtl::OString      m_aFontFile;        // relative to directory
        rtl::OString      m_aMetricFile;      // dito

        /* note: m_aFontFile and Metric file are not atoms
           because they should be fairly unique */

        Type1FontFile() : PrintFont( fonttype::Type1 ), m_nDirectory( 0 ) {}
        virtual ~Type1FontFile();
        virtual bool queryMetricPage( int nPage, utl::MultiAtomProvider* pProvider );
    };

    struct TrueTypeFontFile : public PrintFont
    {
        int           m_nDirectory;       // atom containing system dependent path
        rtl::OString  m_aFontFile;        // relative to directory
        int           m_nCollectionEntry; // 0 for regular fonts, 0 to ... for fonts stemming from collections
        unsigned int  m_nTypeFlags;       // copyright bits and PS-OpenType flag

        TrueTypeFontFile();
        virtual ~TrueTypeFontFile();
        virtual bool queryMetricPage( int nPage, utl::MultiAtomProvider* pProvider );
    };

    struct BuiltinFont : public PrintFont
    {
        int                 m_nDirectory;       // atom containing system dependent path
        rtl::OString      m_aMetricFile;

        BuiltinFont() : PrintFont( fonttype::Builtin ) {}
        virtual ~BuiltinFont();
        virtual bool queryMetricPage( int nPage, utl::MultiAtomProvider* pProvider );
    };

    fontID                                      m_nNextFontID;
    boost::unordered_map< fontID, PrintFont* >       m_aFonts;
    boost::unordered_map< int, FontFamily >        m_aFamilyTypes;
    std::list< rtl::OUString >              m_aPrinterDrivers;
    std::list< rtl::OString >               m_aFontDirectories;
    std::list< int >                            m_aPrivateFontDirectories;
    utl::MultiAtomProvider*                   m_pAtoms;
    // for speeding up findFontFileID
    boost::unordered_map< rtl::OString, std::set< fontID >, rtl::OStringHash >
                                                m_aFontFileToFontID;

    boost::unordered_map< rtl::OString, int, rtl::OStringHash >
    m_aDirToAtom;
    boost::unordered_map< int, rtl::OString >     m_aAtomToDir;
    int                                        m_nNextDirAtom;

    boost::unordered_multimap< rtl::OString, sal_Unicode, rtl::OStringHash >
        m_aAdobenameToUnicode;
    boost::unordered_multimap< sal_Unicode, rtl::OString >
        m_aUnicodeToAdobename;
    boost::unordered_multimap< sal_Unicode, sal_uInt8 > m_aUnicodeToAdobecode;
    boost::unordered_multimap< sal_uInt8, sal_Unicode > m_aAdobecodeToUnicode;

    mutable FontCache*                                                        m_pFontCache;

    mutable std::vector< fontID >               m_aOverrideFonts;

    rtl::OString getAfmFile( PrintFont* pFont ) const;
    rtl::OString getFontFile( PrintFont* pFont ) const;

    bool analyzeFontFile( int nDirID, const rtl::OString& rFileName, std::list< PrintFont* >& rNewFonts, const char *pFormat=NULL ) const;
    rtl::OUString convertTrueTypeName( void* pNameRecord ) const; // actually a NameRecord* formt font subsetting code
    void analyzeTrueTypeFamilyName( void* pTTFont, std::list< rtl::OUString >& rnames ) const; // actually a TrueTypeFont* from font subsetting code
    bool analyzeTrueTypeFile( PrintFont* pFont ) const;
    // finds the font id for the nFaceIndex face in this font file
    // There may be multiple font ids for TrueType collections
    fontID findFontFileID( int nDirID, const rtl::OString& rFile, int nFaceIndex ) const;

    // There may be multiple font ids for TrueType collections
    std::vector<fontID> findFontFileIDs( int nDirID, const rtl::OString& rFile ) const;

    bool knownFontFile( int nDirID, const rtl::OString& rFile ) const
    {
        return findFontFileID(nDirID, rFile, 0) != 0;
    }

    fontID findFontBuiltinID( int nPSNameAtom ) const;

    FontFamily matchFamilyName( const rtl::OUString& rFamily ) const;

    PrintFont* getFont( fontID nID ) const
    {
        boost::unordered_map< fontID, PrintFont* >::const_iterator it;
        it = m_aFonts.find( nID );
        return it == m_aFonts.end() ? NULL : it->second;
    }
    void fillPrintFontInfo( PrintFont* pFont, FastPrintFontInfo& rInfo ) const;
    void fillPrintFontInfo( PrintFont* pFont, PrintFontInfo& rInfo ) const;

    rtl::OString getDirectory( int nAtom ) const;
    int getDirectoryAtom( const rtl::OString& rDirectory, bool bCreate = false );

    void cleanTemporaryFonts();

    /* try to initialize fonts from libfontconfig

    called from <code>initialize()</code>
    */
    void initFontconfig();
    void countFontconfigFonts( boost::unordered_map<rtl::OString, int, rtl::OStringHash>& o_rVisitedPaths );
    /* deinitialize fontconfig
     */
    void deinitFontconfig();

    /* register an application specific font directory for libfontconfig

    since fontconfig is asked for font substitutes before OOo will check for font availability
    and fontconfig will happily substitute fonts it doesn't know (e.g. "Arial Narrow" -> "DejaVu Sans Book"!)
    it becomes necessary to tell the library about all the hidden font treasures

    @returns
    true if libfontconfig accepted the directory
    false else (e.g. no libfontconfig found)
    */
    bool addFontconfigDir(const rtl::OString& rDirectory);

    bool readOverrideMetrics();

    std::set<OString> m_aPreviousLangSupportRequests;
    std::vector<OString> m_aCurrentRequests;
    Timer m_aFontInstallerTimer;

    DECL_LINK( autoInstallFontLangSupport, void* );

    PrintFontManager();
    ~PrintFontManager();
public:
    static PrintFontManager& get(); // one instance only

    // There may be multiple font ids for TrueType collections
    std::vector<fontID> addFontFile( const rtl::OString& rFileName );

    void initialize();

    // returns the number of managed fonts
    int getFontCount() const { return m_aFonts.size(); }

    // caution: the getFontList* methods can change the font list on demand
    // depending on the pParser argument. That is getFontCount() may
    // return a larger value after getFontList()

    // returns the ids of all managed fonts. on pParser != NULL
    // all fonttype::Builtin type fonts are not listed
    // which do not occur in the PPD of pParser
    void getFontList( std::list< fontID >& rFontIDs, const PPDParser* pParser = NULL, bool bUseOverrideMetrics = false );
    // get the font list and fast font info. see getFontList for pParser
    void getFontListWithFastInfo( std::list< FastPrintFontInfo >& rFonts, const PPDParser* pParser = NULL, bool bUseOverrideMetrics = false );

    // get font info for a specific font
    bool getFontInfo( fontID nFontID, PrintFontInfo& rInfo ) const;
    // get fast font info for a specific font
    bool getFontFastInfo( fontID nFontID, FastPrintFontInfo& rInfo ) const;

    // routines to get font info in small pieces

    // get a specific fonts PSName name
    const rtl::OUString& getPSName( fontID nFontID ) const;

    // get a specific fonts family name aliases
    void getFontFamilyAliases( fontID nFontID ) const;

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

    // get a specific fonts width type
    FontWidth getFontWidth( fontID nFontID ) const
    {
        PrintFont* pFont = getFont( nFontID );
        return pFont ? pFont->m_eWidth : WIDTH_DONTKNOW;
    }

    // get a specific fonts weight type
    FontWeight getFontWeight( fontID nFontID ) const
    {
        PrintFont* pFont = getFont( nFontID );
        return pFont ? pFont->m_eWeight : WEIGHT_DONTKNOW;
    }

    // get a specific fonts pitch type
    FontPitch getFontPitch( fontID nFontID ) const
    {
        PrintFont* pFont = getFont( nFontID );
        return pFont ? pFont->m_ePitch : PITCH_DONTKNOW;
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
        return pFont ? pFont->m_bFontEncodingOnly : false;
    }

    // get a specific fonts system dependent filename
    rtl::OString getFontFileSysPath( fontID nFontID ) const
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

    // get encoding vector of font, currently only for Type1 and Builtin fonts
    // returns NULL if encoding vector is empty or font is neither type1 or
    // builtin; if ppNonEncoded is set and non encoded type1 glyphs exist
    // then *ppNonEncoded is set to the mapping for nonencoded glyphs.
    // the encoding vector contains -1 for non encoded glyphs
    const std::map< sal_Unicode, sal_Int32 >* getEncodingMap( fontID nFontID, const std::map< sal_Unicode, rtl::OString >** ppNonEncoded ) const;

    // to get font substitution transparently use the
    // getKernPairs method of PrinterGfx
    const std::list< KernPair >& getKernPairs( fontID nFontID, bool bVertical = false ) const;

    // evaluates copyright flags for TrueType fonts
    // type1 fonts do not have such a feature, so return for them is true
    // returns true for builtin fonts (surprise!)
    bool isFontDownloadingAllowed( fontID nFont ) const;

    // helper for type 1 fonts
    std::list< rtl::OString > getAdobeNameFromUnicode( sal_Unicode aChar ) const;

    std::pair< boost::unordered_multimap< sal_Unicode, sal_uInt8 >::const_iterator,
               boost::unordered_multimap< sal_Unicode, sal_uInt8 >::const_iterator >
    getAdobeCodeFromUnicode( sal_Unicode aChar ) const
    {
        return m_aUnicodeToAdobecode.equal_range( aChar );
    }
    std::list< sal_Unicode >  getUnicodeFromAdobeName( const rtl::OString& rName ) const;
    std::pair< boost::unordered_multimap< sal_uInt8, sal_Unicode >::const_iterator,
                 boost::unordered_multimap< sal_uInt8, sal_Unicode >::const_iterator >
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
                           const rtl::OUString& rOutFile,
                           sal_Int32* pGlyphIDs,
                           sal_uInt8* pNewEncoding,
                           sal_Int32* pWidths,
                           int nGlyphs,
                           bool bVertical = false
                           );
    void getGlyphWidths( fontID nFont,
                         bool bVertical,
                         std::vector< sal_Int32 >& rWidths,
                         std::map< sal_Unicode, sal_uInt32 >& rUnicodeEnc );


    // font administration functions

    // for importFonts to provide the user feedback
    class ImportFontCallback
    {
    public:
        enum FailCondition { NoWritableDirectory, NoAfmMetric, AfmCopyFailed, FontCopyFailed };
        virtual void importFontsFailed( FailCondition eReason ) = 0;
        virtual void progress( const rtl::OUString& rFile ) = 0;
        virtual bool queryOverwriteFile( const rtl::OUString& rFile ) = 0;
        virtual void importFontFailed( const rtl::OUString& rFile, FailCondition ) = 0;
        virtual bool isCanceled() = 0;

    protected:
        ~ImportFontCallback() {}
    };

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
    ImplFontOptions* getFontOptions( const FastPrintFontInfo&, int nSize, void (*subcallback)(void*)) const;

    bool Substitute( FontSelectPattern &rPattern, rtl::OUString& rMissingCodes );

    int FreeTypeCharIndex( void *pFace, sal_uInt32 aChar );

    /**
      Returns an URL for a file where to store contents of a temporary font, or an empty string
      if this font is already known. The file will be cleaned up automatically as appropriate.
      Use activateTemporaryFont() to actually enable usage of the font.

      @param fontName name of the font (e.g. 'Times New Roman')
      @param fontStyle font style, "" for regular, "bi" for bold italic, etc.
      @since 3.7
    */
    OUString fileUrlForTemporaryFont( const OUString& fontName, const char* fontStyle );

    /**
      Adds the given font to the list of known fonts. The font is used only until application
      exit.

      @param fontName name of the font (e.g. 'Times New Roman')
      @param fileUrl URL of the font file
      @since 3.7
    */
    void activateTemporaryFont( const OUString& fontName, const OUString& fileUrl );
};

} // namespace

#endif // _PSPRINT_FONTMANAGER_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
