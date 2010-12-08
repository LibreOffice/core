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

#include <hash_map>
#include <map>
#include <list>
#include <set>

#include "vcl/dllapi.h"
#include "vcl/helper.hxx"

#include "com/sun/star/lang/Locale.hpp"

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

namespace psp {
class PPDParser; // see ppdparser.hxx

namespace italic
{
enum type {
    Upright = 0,
    Oblique = 1,
    Italic = 2,
    Unknown = 3
};
}

namespace width
{
enum type {
    Unknown = 0,
    UltraCondensed = 1,
    ExtraCondensed = 2,
    Condensed = 3,
    SemiCondensed = 4,
    Normal = 5,
    SemiExpanded = 6,
    Expanded = 7,
    ExtraExpanded = 8,
    UltraExpanded = 9
};
}

namespace pitch
{
enum type {
    Unknown = 0,
    Fixed = 1,
    Variable = 2
};
}

namespace weight
{
enum type {
    Unknown = 0,
    Thin = 1,
    UltraLight = 2,
    Light = 3,
    SemiLight = 4,
    Normal = 5,
    Medium = 6,
    SemiBold = 7,
    Bold = 8,
    UltraBold = 9,
    Black = 10
};
}

namespace family
{
enum type {
    Unknown = 0,
    Decorative = 1,
    Modern = 2,
    Roman = 3,
    Script = 4,
    Swiss = 5,
    System = 6
};
}

namespace fonttype
{
enum type {
    Unknown = 0,
    Type1 = 1,
    TrueType = 2,
    Builtin = 3
};
}

namespace fcstatus
{
enum type {
    istrue,
    isunset,
    isfalse
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
    family::type                        m_eFamilyStyle;
    italic::type                        m_eItalic;
    width::type                         m_eWidth;
    weight::type                        m_eWeight;
    pitch::type                         m_ePitch;
    rtl_TextEncoding                    m_aEncoding;
    bool                                m_bSubsettable;
    bool                                m_bEmbeddable;

    FastPrintFontInfo() :
            m_nID( 0 ),
            m_eType( fonttype::Unknown ),
            m_eFamilyStyle( family::Unknown ),
            m_eItalic( italic::Unknown ),
            m_eWidth( width::Unknown ),
            m_eWeight( weight::Unknown ),
            m_ePitch( pitch::Unknown ),
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

class VCL_PLUGIN_PUBLIC PrintFontManager
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
        std::hash_map< int, CharacterMetric >     m_aMetrics;
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
        std::hash_map< sal_Unicode, bool >      m_bVerticalSubstitutions;

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
        italic::type                                m_eItalic;
        width::type                                 m_eWidth;
        weight::type                                m_eWeight;
        pitch::type                                 m_ePitch;
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
        rtl::OString      m_aXLFD;            // mainly for administration, contains the XLFD from fonts.dir

        /* note: m_aFontFile and Metric file are not atoms
           because they should be fairly unique */

        Type1FontFile() : PrintFont( fonttype::Type1 ), m_nDirectory( 0 ) {}
        virtual ~Type1FontFile();
        virtual bool queryMetricPage( int nPage, utl::MultiAtomProvider* pProvider );
    };

    struct TrueTypeFontFile : public PrintFont
    {
        int                     m_nDirectory;       // atom containing system dependent path
        rtl::OString          m_aFontFile;        // relative to directory
        rtl::OString          m_aXLFD;            // mainly for administration, contains the XLFD from fonts.dir
        int                     m_nCollectionEntry; // -1 for regular fonts, 0 to ... for fonts stemming from collections
        unsigned int           m_nTypeFlags;        // copyright bits and PS-OpenType flag

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

    struct XLFDEntry
    {
        static const int MaskFoundry    = 1;
        static const int MaskFamily     = 2;
        static const int MaskAddStyle   = 4;
        static const int MaskItalic     = 8;
        static const int MaskWeight     = 16;
        static const int MaskWidth      = 32;
        static const int MaskPitch      = 64;
        static const int MaskEncoding   = 128;

        int                 nMask; // contains a bit set for every valid member

        rtl::OString        aFoundry;
        rtl::OString        aFamily;
        rtl::OString        aAddStyle;
        italic::type        eItalic;
        weight::type        eWeight;
        width::type         eWidth;
        pitch::type         ePitch;
        rtl_TextEncoding    aEncoding;

        XLFDEntry() { nMask = 0; }

        bool operator<(const XLFDEntry& rRight) const;
        bool operator==(const XLFDEntry& rRight) const;
    };

    static rtl::OString s_aEmptyOString;

    fontID                                      m_nNextFontID;
    std::hash_map< fontID, PrintFont* >       m_aFonts;
    std::hash_map< int, family::type >        m_aFamilyTypes;
    std::list< rtl::OUString >              m_aPrinterDrivers;
    std::list< rtl::OString >               m_aFontDirectories;
    std::list< int >                            m_aPrivateFontDirectories;
    std::map< struct XLFDEntry, std::list< struct XLFDEntry > >
    m_aXLFD_Aliases;
    utl::MultiAtomProvider*                   m_pAtoms;
    // for speeding up findFontFileID
    std::hash_map< rtl::OString, std::set< fontID >, rtl::OStringHash >
                                                m_aFontFileToFontID;

    std::hash_map< rtl::OString, int, rtl::OStringHash >
    m_aDirToAtom;
    std::hash_map< int, rtl::OString >     m_aAtomToDir;
    int                                        m_nNextDirAtom;

    std::hash_multimap< rtl::OString, sal_Unicode, rtl::OStringHash >
        m_aAdobenameToUnicode;
    std::hash_multimap< sal_Unicode, rtl::OString >
        m_aUnicodeToAdobename;
    std::hash_multimap< sal_Unicode, sal_uInt8 >    m_aUnicodeToAdobecode;
    std::hash_multimap< sal_uInt8, sal_Unicode >    m_aAdobecodeToUnicode;

    mutable FontCache*                                                        m_pFontCache;
    bool m_bFontconfigSuccess;

    mutable std::vector< fontID >               m_aOverrideFonts;

    rtl::OString getAfmFile( PrintFont* pFont ) const;
    rtl::OString getFontFile( PrintFont* pFont ) const;

    void getFontAttributesFromXLFD( PrintFont* pFont, const std::list< rtl::OString >& rXLFDs ) const;

    bool analyzeFontFile( int nDirID, const rtl::OString& rFileName, const std::list< rtl::OString >& rXLFDs, std::list< PrintFont* >& rNewFonts ) const;
    rtl::OUString convertTrueTypeName( void* pNameRecord ) const; // actually a NameRecord* formt font subsetting code
    void analyzeTrueTypeFamilyName( void* pTTFont, std::list< rtl::OUString >& rnames ) const; // actually a TrueTypeFont* from font subsetting code
    bool analyzeTrueTypeFile( PrintFont* pFont ) const;
    // finds the FIRST id for this font file; there may be more
    // for TrueType collections
    fontID findFontFileID( int nDirID, const rtl::OString& rFile ) const;
    fontID findFontBuiltinID( int nPSNameAtom ) const;

    family::type matchFamilyName( const rtl::OUString& rFamily ) const;

    PrintFont* getFont( fontID nID ) const
    {
        std::hash_map< fontID, PrintFont* >::const_iterator it;
        it = m_aFonts.find( nID );
        return it == m_aFonts.end() ? NULL : it->second;
    }
    rtl::OString getXLFD( PrintFont* pFont ) const;
    void fillPrintFontInfo( PrintFont* pFont, FastPrintFontInfo& rInfo ) const;
    void fillPrintFontInfo( PrintFont* pFont, PrintFontInfo& rInfo ) const;

    const rtl::OString& getDirectory( int nAtom ) const;
    int getDirectoryAtom( const rtl::OString& rDirectory, bool bCreate = false );

    /* try to initialize fonts from libfontconfig

    called from <code>initialize()</code>

    @returns
    true if at least one font was added by libfontconfig
    false else (e.g. no libfontconfig found)
    */
    bool initFontconfig();
    int  countFontconfigFonts( std::hash_map<rtl::OString, int, rtl::OStringHash>& o_rVisitedPaths );
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

    static bool parseXLFD( const rtl::OString& rXLFD, XLFDEntry& rEntry );
    void parseXLFD_appendAliases( const std::list< rtl::OString >& rXLFDs, std::list< XLFDEntry >& rEntries ) const;
    void initFontsAlias();

    bool readOverrideMetrics();

    PrintFontManager();
    ~PrintFontManager();
public:
    static PrintFontManager& get(); // one instance only

    int addFontFile( const rtl::OString& rFileName, int nFaceNum );

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
    // get the font list and detailed font info. see getFontList for pParser
    void getFontListWithInfo( std::list< PrintFontInfo >& rFonts, const PPDParser* pParser = NULL, bool bUseOverrideMetrics = false );
    // get the font list and fast font info. see getFontList for pParser
    void getFontListWithFastInfo( std::list< FastPrintFontInfo >& rFonts, const PPDParser* pParser = NULL, bool bUseOverrideMetrics = false );

    // get font info for a specific font
    bool getFontInfo( fontID nFontID, PrintFontInfo& rInfo ) const;
    // get fast font info for a specific font
    bool getFontFastInfo( fontID nFontID, FastPrintFontInfo& rInfo ) const;

    // routines to get font info in small pieces

    // get a specific fonts family name
    const rtl::OUString& getFontFamily( fontID nFontID ) const;
    // get a specific fonts PSName name
    const rtl::OUString& getPSName( fontID nFontID ) const;

    // get a specific fonts style family
    family::type getFontFamilyType( fontID nFontID ) const;

    // get a specific fonts family name aliases
    void getFontFamilyAliases( fontID nFontID ) const;

    // get a specific fonts type
    fonttype::type getFontType( fontID nFontID ) const
    {
        PrintFont* pFont = getFont( nFontID );
        return pFont ? pFont->m_eType : fonttype::Unknown;
    }

    // get a specific fonts italic type
    italic::type getFontItalic( fontID nFontID ) const
    {
        PrintFont* pFont = getFont( nFontID );
        return pFont ? pFont->m_eItalic : italic::Unknown;
    }

    // get a specific fonts width type
    width::type getFontWidth( fontID nFontID ) const
    {
        PrintFont* pFont = getFont( nFontID );
        return pFont ? pFont->m_eWidth : width::Unknown;
    }

    // get a specific fonts weight type
    weight::type getFontWeight( fontID nFontID ) const
    {
        PrintFont* pFont = getFont( nFontID );
        return pFont ? pFont->m_eWeight : weight::Unknown;
    }

    // get a specific fonts pitch type
    pitch::type getFontPitch( fontID nFontID ) const
    {
        PrintFont* pFont = getFont( nFontID );
        return pFont ? pFont->m_ePitch : pitch::Unknown;
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

    // get a specific fonts global metrics
    const CharacterMetric& getGlobalFontMetric( fontID nFontID, bool bHorizontal ) const;

    // get a specific fonts ascend
    int getFontAscend( fontID nFontID ) const;

    // get a specific fonts descent
    int getFontDescend( fontID nFontID ) const;

    // get a specific fonts leading
    int getFontLeading( fontID nFontID ) const;

    // get a fonts glyph bounding box
    bool getFontBoundingBox( fontID nFont, int& xMin, int& yMin, int& xMax, int& yMax );

    // info whether there are vertical substitutions
    bool hasVerticalSubstitutions( fontID nFontID ) const;

    // info whether an array of glyphs has vertical substitutions
    void hasVerticalSubstitutions( fontID nFontID, const sal_Unicode* pCharacters,
        int nCharacters, bool* pHasSubst ) const;

    // get the XLFD for a font that originated from the X fontpath
    // note: this may not be the original line that was in the fonts.dir
    // returns a string for every font, but only TrueType and Type1
    // fonts originated from the X font path, so check for the font type
    rtl::OUString getFontXLFD( fontID nFontID ) const;

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

    std::pair< std::hash_multimap< sal_Unicode, sal_uInt8 >::const_iterator,
               std::hash_multimap< sal_Unicode, sal_uInt8 >::const_iterator >
    getAdobeCodeFromUnicode( sal_Unicode aChar ) const
    {
        return m_aUnicodeToAdobecode.equal_range( aChar );
    }
    std::list< sal_Unicode >  getUnicodeFromAdobeName( const rtl::OString& rName ) const;
    std::pair< std::hash_multimap< sal_uInt8, sal_Unicode >::const_iterator,
                 std::hash_multimap< sal_uInt8, sal_Unicode >::const_iterator >
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
    };

    // checks wether font import would fail due to no writeable directory
    bool checkImportPossible() const;
    // expects system paths not UNC paths
    // returns the number of fonts successfully imported
    int importFonts( const std::list< rtl::OString >& rFiles, bool bLinkOnly = false, ImportFontCallback* pCallback = NULL );

    // check wether changeFontProperties would fail due to not writable fonts.dir
    bool checkChangeFontPropertiesPossible( fontID nFont ) const;
    // change fonts.dir entry for font
    bool changeFontProperties( fontID nFont, const rtl::OUString& rXLFD );

    // get properties of a not imported font file
    bool getImportableFontProperties( const rtl::OString& rFile, std::list< FastPrintFontInfo >& rFontProps );

    // get fonts that come from the same font file
    bool getFileDuplicates( fontID nFont, std::list< fontID >& rFonts ) const;
    // remove font files
    bool removeFonts( const std::list< fontID >& rFonts );

    bool isPrivateFontFile( fontID ) const;

    // returns false if there were not any
    bool getAlternativeFamilyNames( fontID nFont, std::list< rtl::OUString >& rNames ) const;

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
    bool getFontOptions( const FastPrintFontInfo&, int nSize, void (*subcallback)(void*), ImplFontOptions& rResult ) const;

    rtl::OUString Substitute( const rtl::OUString& rFontName, rtl::OUString& rMissingCodes,
        const rtl::OString& rLangAttrib, italic::type& rItalic, weight::type& rWeight,
        width::type& rWidth, pitch::type& rPitch) const;
    bool hasFontconfig() const { return m_bFontconfigSuccess; }

    int FreeTypeCharIndex( void *pFace, sal_uInt32 aChar );
};

} // namespace

#endif // _PSPRINT_FONTMANAGER_HXX_
