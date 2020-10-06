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

/**
 * @file sft.hxx
 * @brief Sun Font Tools
 */

/*
 *        Generated fonts contain an XUID entry in the form of:
 *
 *                  103 0 T C1 N C2 C3
 *
 *        103 - Sun's Adobe assigned XUID number. Contact person: Alexander Gelfenbain <gelf@eng.sun.com>
 *
 *        T  - font type. 0: Type 3, 1: Type 42
 *        C1 - CRC-32 of the entire source TrueType font
 *        N  - number of glyphs in the subset
 *        C2 - CRC-32 of the array of glyph IDs used to generate the subset
 *        C3 - CRC-32 of the array of encoding numbers used to generate the subset
 *
 */

#ifndef INCLUDED_VCL_INC_SFT_HXX
#define INCLUDED_VCL_INC_SFT_HXX

#include <vcl/dllapi.h>
#include <vcl/fontcapabilities.hxx>
#include <vcl/fontcharmap.hxx>
#include <i18nlangtag/lang.h>

#include <array>
#include <memory>
#include <vector>
#include <cstdint>

namespace vcl
{

/*@{*/
    typedef sal_Int32       F16Dot16;           /**< fixed: 16.16 */
/*@}*/

/** Return value of OpenTTFont() and CreateT3FromTTGlyphs() */
    enum class SFErrCodes {
        Ok,                              /**< no error                                     */
        BadFile,                         /**< file not found                               */
        FileIo,                          /**< file I/O error                               */
        Memory,                          /**< memory allocation error                      */
        GlyphNum,                        /**< incorrect number of glyphs                   */
        BadArg,                          /**< incorrect arguments                          */
        TtFormat,                        /**< incorrect TrueType font format               */
        FontNo                           /**< incorrect logical font number of a TTC font  */
    };

#ifndef FW_THIN /* WIN32 compilation would conflict */
/** Value of the weight member of the TTGlobalFontInfo struct */
    enum WeightClass {
        FW_THIN = 100,                      /**< Thin                               */
        FW_EXTRALIGHT = 200,                /**< Extra-light (Ultra-light)          */
        FW_LIGHT = 300,                     /**< Light                              */
        FW_NORMAL = 400,                    /**< Normal (Regular)                   */
        FW_MEDIUM = 500,                    /**< Medium                             */
        FW_SEMIBOLD = 600,                  /**< Semi-bold (Demi-bold)              */
        FW_BOLD = 700,                      /**< Bold                               */
        FW_EXTRABOLD = 800,                 /**< Extra-bold (Ultra-bold)            */
        FW_BLACK = 900                      /**< Black (Heavy)                      */
    };
#endif /* FW_THIN */

/** Value of the width member of the TTGlobalFontInfo struct */
    enum WidthClass {
        FWIDTH_ULTRA_CONDENSED = 1,         /**< 50% of normal                      */
        FWIDTH_EXTRA_CONDENSED = 2,         /**< 62.5% of normal                    */
        FWIDTH_CONDENSED = 3,               /**< 75% of normal                      */
        FWIDTH_SEMI_CONDENSED = 4,          /**< 87.5% of normal                    */
        FWIDTH_NORMAL = 5,                  /**< Medium, 100%                       */
        FWIDTH_SEMI_EXPANDED = 6,           /**< 112.5% of normal                   */
        FWIDTH_EXPANDED = 7,                /**< 125% of normal                     */
        FWIDTH_EXTRA_EXPANDED = 8,          /**< 150% of normal                     */
        FWIDTH_ULTRA_EXPANDED = 9           /**< 200% of normal                     */
    };

/** Composite glyph flags definition */
    enum CompositeFlags {
        ARG_1_AND_2_ARE_WORDS     = 1,
        ARGS_ARE_XY_VALUES        = 1<<1,
        ROUND_XY_TO_GRID          = 1<<2,
        WE_HAVE_A_SCALE           = 1<<3,
        MORE_COMPONENTS           = 1<<5,
        WE_HAVE_AN_X_AND_Y_SCALE  = 1<<6,
        WE_HAVE_A_TWO_BY_TWO      = 1<<7,
        WE_HAVE_INSTRUCTIONS      = 1<<8,
        USE_MY_METRICS            = 1<<9,
        OVERLAP_COMPOUND          = 1<<10
    };

/** Structure used by GetTTSimpleCharMetrics() functions */
    typedef struct {
        sal_uInt16 adv;                         /**< advance width or height            */
        sal_Int16 sb;                           /**< left or top sidebearing            */
    } TTSimpleGlyphMetrics;

/** Structure used by the TrueType Creator and GetRawGlyphData() */

    typedef struct {
        sal_uInt32 glyphID;                     /**< glyph ID                           */
        sal_uInt16 nbytes;                      /**< number of bytes in glyph data      */
        sal_uInt8  *ptr;                         /**< pointer to glyph data              */
        sal_uInt16 aw;                          /**< advance width                      */
        sal_Int16  lsb;                         /**< left sidebearing                   */
        bool compflag;                          /**< false- if non-composite */
        sal_uInt16 npoints;                     /**< number of points                   */
        sal_uInt16 ncontours;                   /**< number of contours                 */
        /* */
        sal_uInt32 newID;                       /**< used internally by the TTCR        */
    } GlyphData;

/** Structure used by the TrueType Creator and CreateTTFromTTGlyphs() */
    typedef struct {
        sal_uInt16 platformID;                  /**< Platform ID                                            */
        sal_uInt16 encodingID;                  /**< Platform-specific encoding ID                          */
        LanguageType languageID;                /**< Language ID                                            */
        sal_uInt16 nameID;                      /**< Name ID                                                */
        sal_uInt16 slen;                        /**< String length in bytes                                 */
        sal_uInt8  *sptr;                        /**< Pointer to string data (not zero-terminated!)          */
    } NameRecord;

/** Return value of GetTTGlobalFontInfo() */

    typedef struct TTGlobalFontInfo_ {
        char *family;             /**< family name                                             */
        sal_Unicode *ufamily;     /**< family name UCS2                                         */
        char *subfamily;          /**< subfamily name                                          */
        sal_Unicode *usubfamily;  /**< subfamily name UCS2 */
        char *psname;             /**< PostScript name                                         */
        sal_uInt16 macStyle;      /**< macstyle bits from 'HEAD' table */
        int   weight;             /**< value of WeightClass or 0 if can't be determined        */
        int   width;              /**< value of WidthClass or 0 if can't be determined         */
        int   pitch;              /**< 0: proportional font, otherwise: monospaced             */
        int   italicAngle;        /**< in counter-clockwise degrees * 65536                    */
        int   xMin;               /**< global bounding box: xMin                               */
        int   yMin;               /**< global bounding box: yMin                               */
        int   xMax;               /**< global bounding box: xMax                               */
        int   yMax;               /**< global bounding box: yMax                               */
        int   ascender;           /**< typographic ascent.                                     */
        int   descender;          /**< typographic descent.                                    */
        int   linegap;            /**< typographic line gap.\ Negative values are treated as
                                     zero in Win 3.1, System 6 and System 7.                 */
        int   typoAscender;       /**< OS/2 portable typographic ascender                      */
        int   typoDescender;      /**< OS/2 portable typographic descender                     */
        int   typoLineGap;        /**< OS/2 portable typographic line gap                       */
        int   winAscent;          /**< ascender metric for Windows                             */
        int   winDescent;         /**< descender metric for Windows                            */
        bool  symbolEncoded;      /**< true: MS symbol encoded */
        sal_uInt8  panose[10];    /**< PANOSE classification number                            */
        sal_uInt32 typeFlags;     /**< type flags (copyright bits)                             */
        sal_uInt16 fsSelection;   /**< OS/2 fsSelection */
    } TTGlobalFontInfo;

/** ControlPoint structure used by GetTTGlyphPoints() */
    typedef struct {
        sal_uInt32 flags;             /**< 00000000 00000000 e0000000 bbbbbbbb */
        /**< b - byte flags from the glyf array  */
        /**< e == 0 - regular point              */
        /**< e == 1 - end contour                */
        sal_Int16 x;                  /**< X coordinate in EmSquare units      */
        sal_Int16 y;                  /**< Y coordinate in EmSquare units      */
    } ControlPoint;


/*
  Some table OS/2 consts
  quick history:
  OpenType has been created from TrueType
  - original TrueType had an OS/2 table with a length of 68 bytes
  (cf https://developer.apple.com/fonts/TrueType-Reference-Manual/RM06/Chap6OS2.html)
  - There have been 6 versions (from version 0 to 5)
  (cf https://docs.microsoft.com/en-us/typography/opentype/otspec140/os2ver0)

  For the record:
  // From Initial TrueType version
  TYPE       NAME                       FROM BYTE
  uint16     version                    0
  int16      xAvgCharWidth              2
  uint16     usWeightClass              4
  uint16     usWidthClass               6
  uint16     fsType                     8
  int16      ySubscriptXSize           10
  int16      ySubscriptYSize           12
  int16      ySubscriptXOffset         14
  int16      ySubscriptYOffset         16
  int16      ySuperscriptXSize         18
  int16      ySuperscriptYSize         20
  int16      ySuperscriptXOffset       22
  int16      ySuperscriptYOffset       24
  int16      yStrikeoutSize            26
  int16      yStrikeoutPosition        28
  int16      sFamilyClass              30
  uint8      panose[10]                32
  uint32     ulUnicodeRange1           42
  uint32     ulUnicodeRange2           46
  uint32     ulUnicodeRange3           50
  uint32     ulUnicodeRange4           54
  Tag        achVendID                 58
  uint16     fsSelection               62
  uint16     usFirstCharIndex          64
  uint16     usLastCharIndex           66

  // From Version 0 of OpenType
  int16      sTypoAscender             68
  int16      sTypoDescender            70
  int16      sTypoLineGap              72
  uint16     usWinAscent               74
  uint16     usWinDescent              76

  => length for OpenType version 0 = 78 bytes

  // From Version 1 of OpenType
  uint32     ulCodePageRange1          78
  uint32     ulCodePageRange2          82

  => length for OpenType version 1 = 86 bytes

  // From Version 2 of OpenType
  // (idem for Versions 3 and 4)
  int16      sxHeight                  86
  int16      sCapHeight                88
  uint16     usDefaultChar             90
  uint16     usBreakChar               92
  uint16     usMaxContext              94

  => length for OpenType version 2, 3 and 4 = 96 bytes

  // From Version 5 of OpenType
  uint16     usLowerOpticalPointSize   96
  uint16     usUpperOpticalPointSize   98
  END                                 100

  => length for OS/2 table version 5 = 100 bytes

*/
constexpr int OS2_Legacy_length = 68;
constexpr int OS2_V0_length = 78;
constexpr int OS2_V1_length = 86;

constexpr int OS2_usWeightClass_offset = 4;
constexpr int OS2_usWidthClass_offset = 6;
constexpr int OS2_fsType_offset = 8;
constexpr int OS2_panose_offset = 32;
constexpr int OS2_panoseNbBytes_offset = 10;
constexpr int OS2_ulUnicodeRange1_offset = 42;
constexpr int OS2_ulUnicodeRange2_offset = 46;
constexpr int OS2_ulUnicodeRange3_offset = 50;
constexpr int OS2_ulUnicodeRange4_offset = 54;
constexpr int OS2_fsSelection_offset = 62;
constexpr int OS2_typoAscender_offset = 68;
constexpr int OS2_typoDescender_offset = 70;
constexpr int OS2_typoLineGap_offset = 72;
constexpr int OS2_winAscent_offset = 74;
constexpr int OS2_winDescent_offset = 76;
constexpr int OS2_ulCodePageRange1_offset = 78;
constexpr int OS2_ulCodePageRange2_offset = 82;

/*
  Some table hhea consts
  cf https://docs.microsoft.com/fr-fr/typography/opentype/spec/hhea
  TYPE       NAME                       FROM BYTE
  uint16     majorVersion               0
  uint16     minorVersion               2
  FWORD      ascender                   4
  FWORD      descender                  6
  FWORD      lineGap                    8
  UFWORD     advanceWidthMax           10
  FWORD      minLeftSideBearing        12
  FWORD      minRightSideBearing       14
  FWORD      xMaxExtent                16
  int16      caretSlopeRise            18
  int16      caretSlopeRun             20
  int16      caretOffset               22
  int16      (reserved)                24
  int16      (reserved)                26
  int16      (reserved)                28
  int16      (reserved)                30
  int16      metricDataFormat          32
  uint16     numberOfHMetrics          34
  END                                  36

  => length for hhea table = 36 bytes

*/
constexpr int HHEA_Length = 36;

constexpr int HHEA_ascender_offset = 4;
constexpr int HHEA_descender_offset = 6;
constexpr int HHEA_lineGap_offset = 8;
constexpr int HHEA_caretSlopeRise_offset = 18;
constexpr int HHEA_caretSlopeRun_offset = 20;

/*
  Some table post consts
  cf https://docs.microsoft.com/fr-fr/typography/opentype/spec/post
  TYPE       NAME                       FROM BYTE
  Fixed      version                    0
  Fixed      italicAngle                4
  FWord      underlinePosition          8
  FWord      underlineThickness        10
  uint32     isFixedPitch              12
  ...

*/
constexpr int POST_italicAngle_offset = 4;
constexpr int POST_underlinePosition_offset = 8;
constexpr int POST_underlineThickness_offset = 10;
constexpr int POST_isFixedPitch_offset = 12;

/*
  Some table head consts
  cf https://docs.microsoft.com/fr-fr/typography/opentype/spec/head
  TYPE       NAME                       FROM BYTE
  uit16      majorVersion               0
  uit16      minorVersion               2
  Fixed      fontRevision               4
  uint32     checkSumAdjustment         8
  uint32     magicNumber               12 (= 0x5F0F3CF5)
  uint16     flags                     16
  uint16     unitsPerEm                18
  LONGDATETIME created                 20
  LONGDATETIME modified                28
  int16      xMin                      36
  int16      yMin                      38
  int16      xMax                      40
  int16      yMax                      42
  uint16     macStyle                  44
  uint16     lowestRecPPEM             46
  int16      fontDirectionHint         48
  int16      indexToLocFormat          50
  int16      glyphDataFormat           52

  END                                  54

  => length head table = 54 bytes
*/
constexpr int HEAD_Length = 54;

constexpr int HEAD_majorVersion_offset = 0;
constexpr int HEAD_fontRevision_offset = 4;
constexpr int HEAD_magicNumber_offset = 12;
constexpr int HEAD_flags_offset = 16;
constexpr int HEAD_unitsPerEm_offset = 18;
constexpr int HEAD_created_offset = 20;
constexpr int HEAD_xMin_offset = 36;
constexpr int HEAD_yMin_offset = 38;
constexpr int HEAD_xMax_offset = 40;
constexpr int HEAD_yMax_offset = 42;
constexpr int HEAD_macStyle_offset = 44;
constexpr int HEAD_lowestRecPPEM_offset = 46;
constexpr int HEAD_fontDirectionHint_offset = 48;
constexpr int HEAD_indexToLocFormat_offset = 50;
constexpr int HEAD_glyphDataFormat_offset = 52;

/*
  Some table maxp consts
  cf https://docs.microsoft.com/fr-fr/typography/opentype/spec/maxp
  For 0.5 version
  TYPE       NAME                       FROM BYTE
  Fixed      version                    0
  uint16     numGlyphs                  4

  For 1.0 Version
  Fixed      version                    0
  uint16     numGlyphs                  4
  uint16     maxPoints                  6
  uint16     maxContours                8
  uint16     maxCompositePoints        10
  uint16     maxCompositeContours      12
  ...

*/
constexpr int MAXP_Version1Length = 32;

constexpr int MAXP_numGlyphs_offset = 4;
constexpr int MAXP_maxPoints_offset = 6;
constexpr int MAXP_maxContours_offset = 8;
constexpr int MAXP_maxCompositePoints_offset = 10;
constexpr int MAXP_maxCompositeContours_offset = 12;

/*
  Some table glyf consts
  cf https://docs.microsoft.com/fr-fr/typography/opentype/spec/glyf
  For 0.5 version
  TYPE       NAME                       FROM BYTE
  int16      numberOfContours           0
  int16      xMin                       2
  int16      yMin                       4
  int16      xMax                       6
  int16      yMax                       8

  END                                  10

  => length glyf table = 10 bytes

*/
constexpr int GLYF_Length = 10;

constexpr int GLYF_numberOfContours_offset = 0;
constexpr int GLYF_xMin_offset = 2;
constexpr int GLYF_yMin_offset = 4;
constexpr int GLYF_xMax_offset = 6;
constexpr int GLYF_yMax_offset = 8;

constexpr sal_uInt32 T_true = 0x74727565;        /* 'true' */
constexpr sal_uInt32 T_ttcf = 0x74746366;        /* 'ttcf' */
constexpr sal_uInt32 T_otto = 0x4f54544f;        /* 'OTTO' */

// standard TrueType table tags
constexpr sal_uInt32 T_maxp = 0x6D617870;
constexpr sal_uInt32 T_glyf = 0x676C7966;
constexpr sal_uInt32 T_head = 0x68656164;
constexpr sal_uInt32 T_loca = 0x6C6F6361;
constexpr sal_uInt32 T_name = 0x6E616D65;
constexpr sal_uInt32 T_hhea = 0x68686561;
constexpr sal_uInt32 T_hmtx = 0x686D7478;
constexpr sal_uInt32 T_cmap = 0x636D6170;
constexpr sal_uInt32 T_vhea = 0x76686561;
constexpr sal_uInt32 T_vmtx = 0x766D7478;
constexpr sal_uInt32 T_OS2  = 0x4F532F32;
constexpr sal_uInt32 T_post = 0x706F7374;
constexpr sal_uInt32 T_cvt  = 0x63767420;
constexpr sal_uInt32 T_prep = 0x70726570;
constexpr sal_uInt32 T_fpgm = 0x6670676D;
constexpr sal_uInt32 T_gsub = 0x47535542;
constexpr sal_uInt32 T_CFF  = 0x43464620;

class AbstractTrueTypeFont;
class TrueTypeFont;

/**
 * @defgroup sft Sun Font Tools Exported Functions
 */

/**
 * Get the number of fonts contained in a TrueType collection
 * @param  fname - file name
 * @return number of fonts or zero, if file is not a TTC file.
 * @ingroup sft
 */
    int CountTTCFonts(const char* fname);

/**
 * TrueTypeFont constructor.
 * The font file has to be provided as a memory buffer and length
 * @param  pBuffer - memory buffer
 * @param  nLen    - size of memory buffer
 * @param  facenum - logical font number within a TTC file. This value is ignored
 *                   for TrueType fonts
 * @param  ttf     - returns the opened TrueTypeFont
 * @param  xCharMap  - optional parsed character map
 * @return value of SFErrCodes enum
 * @ingroup sft
 */
    SFErrCodes VCL_DLLPUBLIC OpenTTFontBuffer(const void* pBuffer, sal_uInt32 nLen, sal_uInt32 facenum,
                                              TrueTypeFont** ttf, const FontCharMapRef xCharMap = nullptr);
#if !defined(_WIN32)
/**
 * TrueTypeFont constructor.
 * Reads the font file and allocates the memory for the structure.
 * on WIN32 the font has to be provided as a memory buffer and length
 * @param  fname   - name of TrueType font file
 * @param  facenum - logical font number within a TTC file. This value is ignored
 *                   for TrueType fonts
 * @param  ttf     - returns the opened TrueTypeFont
 * @param  xCharMap  - optional parsed character map
 * @return value of SFErrCodes enum
 * @ingroup sft
 */
    SFErrCodes VCL_DLLPUBLIC OpenTTFontFile(const char *fname, sal_uInt32 facenum, TrueTypeFont** ttf,
                                            const FontCharMapRef xCharMap = nullptr);
#endif

    bool VCL_DLLPUBLIC getTTCoverage(
        std::optional<std::bitset<UnicodeCoverage::MAX_UC_ENUM>> & rUnicodeCoverage,
        std::optional<std::bitset<CodePageCoverage::MAX_CP_ENUM>> & rCodePageCoverage,
        const unsigned char* pTable, size_t nLength);

/**
 * TrueTypeFont destructor. Deallocates the memory.
 * @ingroup sft
 */
    void VCL_DLLPUBLIC CloseTTFont(TrueTypeFont *);

/**
 * Extracts TrueType control points, and stores them in an allocated array pointed to
 * by *pointArray. This function returns the number of extracted points.
 *
 * @param ttf         pointer to the TrueTypeFont structure
 * @param glyphID     Glyph ID
 * @param pointArray  Return value - address of the pointer to the first element of the array
 *                    of points allocated by the function
 * @return            Returns the number of points in *pointArray or -1 if glyphID is
 *                    invalid.
 * @ingroup sft
 *
 */
    int GetTTGlyphPoints(AbstractTrueTypeFont *ttf, sal_uInt32 glyphID, ControlPoint **pointArray);

/**
 * Extracts raw glyph data from the 'glyf' table and returns it in an allocated
 * GlyphData structure.
 *
 * @param ttf         pointer to the TrueTypeFont structure
 * @param glyphID     Glyph ID
 *
 * @return            pointer to an allocated GlyphData structure or NULL if
 *                    glyphID is not present in the font
 * @ingroup sft
 *
 */
    GlyphData *GetTTRawGlyphData(AbstractTrueTypeFont *ttf, sal_uInt32 glyphID);

/**
 * For a specified glyph adds all component glyphs IDs to the list and
 * return their number. If the glyph is a single glyph it has one component
 * glyph (which is added to the list) and the function returns 1.
 * For a composite glyphs it returns the number of component glyphs
 * and adds all of them to the list.
 *
 * @param ttf         pointer to the TrueTypeFont structure
 * @param glyphID     Glyph ID
 * @param glyphlist   list of glyphs
 *
 * @return            number of component glyphs
 * @ingroup sft
 *
 */
    int GetTTGlyphComponents(AbstractTrueTypeFont *ttf, sal_uInt32 glyphID, std::vector< sal_uInt32 >& glyphlist);

/**
 * Extracts all Name Records from the font and stores them in an allocated
 * array of NameRecord structs
 *
 * @param ttf       pointer to the TrueTypeFont struct
 * @param nr        pointer to the array of NameRecord structs
 *
 * @return          number of NameRecord structs
 * @ingroup sft
 */

    int GetTTNameRecords(AbstractTrueTypeFont const *ttf, NameRecord **nr);

/**
 * Deallocates previously allocated array of NameRecords.
 *
 * @param nr        array of NameRecord structs
 * @param n         number of elements in the array
 *
 * @ingroup sft
 */
    void DisposeNameRecords(NameRecord* nr, int n);

/**
 * Generates a new PostScript Type 3 font and dumps it to <b>outf</b> file.
 * This function substitutes glyph 0 for all glyphIDs that are not found in the font.
 * @param ttf         pointer to the TrueTypeFont structure
 * @param outf        the resulting font is written to this stream
 * @param fname       font name for the new font. If it is NULL the PostScript name of the
 *                    original font will be used
 * @param glyphArray  pointer to an array of glyphs that are to be extracted from ttf
 * @param encoding    array of encoding values. encoding[i] specifies the position of the glyph
 *                    glyphArray[i] in the encoding vector of the resulting Type3 font
 * @param nGlyphs     number of glyph IDs in glyphArray and encoding values in encoding
 * @param wmode       writing mode for the output file: 0 - horizontal, 1 - vertical
 * @return            return the value of SFErrCodes enum
 * @see               SFErrCodes
 * @ingroup sft
 *
 */
    SFErrCodes CreateT3FromTTGlyphs(TrueTypeFont *ttf, FILE *outf, const char *fname, sal_uInt16 const *glyphArray, sal_uInt8 *encoding, int nGlyphs, int wmode);

/**
 * Generates a new TrueType font and dumps it to <b>outf</b> file.
 * This function substitutes glyph 0 for all glyphIDs that are not found in the font.
 * @param ttf         pointer to the TrueTypeFont structure
 * @param fname       file name for the output TrueType font file
 * @param glyphArray  pointer to an array of glyphs that are to be extracted from ttf. The first
 *                    element of this array has to be glyph 0 (default glyph)
 * @param encoding    array of encoding values. encoding[i] specifies character code for
 *                    the glyphID glyphArray[i]. Character code 0 usually points to a default
 *                    glyph (glyphID 0)
 * @param nGlyphs     number of glyph IDs in glyphArray and encoding values in encoding
 * @param flags       or'ed TTCreationFlags
 * @return            return the value of SFErrCodes enum
 * @see               SFErrCodes
 * @ingroup sft
 *
 */
    VCL_DLLPUBLIC SFErrCodes CreateTTFromTTGlyphs(AbstractTrueTypeFont  *ttf,
                              const char    *fname,
                              sal_uInt16 const *glyphArray,
                              sal_uInt8 const *encoding,
                              int            nGlyphs);

/**
 * Generates a new PostScript Type42 font and dumps it to <b>outf</b> file.
 * This function substitutes glyph 0 for all glyphIDs that are not found in the font.
 * @param ttf         pointer to the TrueTypeFont structure
 * @param outf        output stream for a resulting font
 * @param psname      PostScript name of the resulting font
 * @param glyphArray  pointer to an array of glyphs that are to be extracted from ttf. The first
 *                    element of this array has to be glyph 0 (default glyph)
 * @param encoding    array of encoding values. encoding[i] specifies character code for
 *                    the glyphID glyphArray[i]. Character code 0 usually points to a default
 *                    glyph (glyphID 0)
 * @param nGlyphs     number of glyph IDs in glyphArray and encoding values in encoding
 * @return            SFErrCodes::Ok - no errors
 *                    SFErrCodes::GlyphNum - too many glyphs (> 255)
 *                    SFErrCodes::TtFormat - corrupted TrueType fonts
 *
 * @see               SFErrCodes
 * @ingroup sft
 *
 */
    SFErrCodes CreateT42FromTTGlyphs(TrueTypeFont  *ttf,
                               FILE          *outf,
                               const char    *psname,
                               sal_uInt16 const *glyphArray,
                               sal_uInt8          *encoding,
                               int            nGlyphs);

/**
 * Queries glyph metrics. Allocates an array of advance width/height values and returns it.
 *
 * @param ttf         pointer to the TrueTypeFont structure
 * @param glyphArray  pointer to an array of glyphs that are to be extracted from ttf
 * @param nGlyphs     number of glyph IDs in glyphArray and encoding values in encoding
 * @param vertical    writing mode: false - horizontal, true - vertical
 * @ingroup sft
 *
 */
    VCL_DLLPUBLIC std::unique_ptr<sal_uInt16[]> GetTTSimpleGlyphMetrics(AbstractTrueTypeFont const *ttf, const sal_uInt16 *glyphArray, int nGlyphs, bool vertical);

/**
 * Returns global font information about the TrueType font.
 * @see TTGlobalFontInfo
 *
 * @param ttf         pointer to a TrueTypeFont structure
 * @param info        pointer to a TTGlobalFontInfo structure
 * @ingroup sft
 *
 */
    VCL_DLLPUBLIC void GetTTGlobalFontInfo(TrueTypeFont *ttf, TTGlobalFontInfo *info);

/**
 * Returns part of the head table info, normally collected by GetTTGlobalFontInfo.
 *
 * Just implemented separate, because this info not available via Qt API.
 *
 * @param ttf         pointer to a AbstractTrueTypeFont structure
 * @param xMin        global glyph bounding box min X
 * @param yMin        global glyph bounding box min Y
 * @param xMax        global glyph bounding box max X
 * @param yMax        global glyph bounding box max Y
 * @param macStyle    encoded Mac style flags of the font
 * @return            true, if table data could be decoded
 * @ingroup sft
 */
    VCL_DLLPUBLIC bool GetTTGlobalFontHeadInfo(const AbstractTrueTypeFont *ttf, int& xMin, int& yMin, int& xMax, int& yMax, sal_uInt16& macStyle);

/**
 * Returns fonts metrics.
 * @see TTGlobalFontInfo
 *
 * @param hhea        hhea table data
 * @param os2         OS/2 table data
 * @param info        pointer to a TTGlobalFontInfo structure
 * @ingroup sft
 *
 */
 void GetTTFontMetrics(const uint8_t *pHhea, size_t nHhea,
                       const uint8_t *pOs2, size_t nOs2,
                       TTGlobalFontInfo *info);

/*- private definitions */

/* indexes into TrueTypeFont::tables[] and TrueTypeFont::tlens[] */
constexpr int O_maxp = 0;
constexpr int O_glyf = 1;    /* 'glyf' */
constexpr int O_head = 2;    /* 'head' */
constexpr int O_loca = 3;    /* 'loca' */
constexpr int O_name = 4;    /* 'name' */
constexpr int O_hhea = 5;    /* 'hhea' */
constexpr int O_hmtx = 6;    /* 'hmtx' */
constexpr int O_cmap = 7;    /* 'cmap' */
constexpr int O_vhea = 8;    /* 'vhea' */
constexpr int O_vmtx = 9;    /* 'vmtx' */
constexpr int O_OS2  = 10;   /* 'OS/2' */
constexpr int O_post = 11;   /* 'post' */
constexpr int O_cvt  = 12;   /* 'cvt_' - only used in TT->TT generation */
constexpr int O_prep = 13;   /* 'prep' - only used in TT->TT generation */
constexpr int O_fpgm = 14;   /* 'fpgm' - only used in TT->TT generation */
constexpr int O_gsub = 15;   /* 'GSUB' */
constexpr int O_CFF = 16;   /* 'CFF' */
constexpr int NUM_TAGS = 17;

class VCL_DLLPUBLIC AbstractTrueTypeFont
{
    char* m_pFileName;
    sal_uInt32 m_nGlyphs;
    sal_uInt32* m_pGlyphOffsets;
    sal_uInt32 m_nHorzMetrics;
    sal_uInt32 m_nVertMetrics; /* if not 0 => font has vertical metrics information */
    sal_uInt32 m_nUnitsPerEm;
    FontCharMapRef m_xCharMap;

protected:
    SFErrCodes indexGlyphData();

public:
    AbstractTrueTypeFont(const char* fileName = nullptr, const FontCharMapRef xCharMap = nullptr);
    virtual ~AbstractTrueTypeFont();

    const char* fileName() const { return m_pFileName; }
    sal_uInt32 glyphCount() const { return m_nGlyphs; }
    sal_uInt32 glyphOffset(sal_uInt32 glyphID) const { return m_pGlyphOffsets[glyphID]; }
    sal_uInt32 horzMetricCount() const { return m_nHorzMetrics; }
    sal_uInt32 vertMetricCount() const { return m_nVertMetrics; }
    sal_uInt32 unitsPerEm() const { return m_nUnitsPerEm; }
    FontCharMapRef GetCharMap() const { return m_xCharMap; }

    virtual bool hasTable(sal_uInt32 ord) const = 0;
    virtual const sal_uInt8* table(sal_uInt32 ord, sal_uInt32& size) const = 0;
};

class TrueTypeFont final : public AbstractTrueTypeFont
{
    struct TTFontTable_
    {
        const sal_uInt8* pData = nullptr; /* pointer to a raw subtable in the SFNT file */
        sal_uInt32 nSize = 0; /* table size */
    };

    std::array<struct TTFontTable_, NUM_TAGS> m_aTableList;

public:
        sal_Int32   fsize;
        sal_uInt8   *ptr;

        char        *psname;
        char        *family;
        sal_Unicode *ufamily;
        char        *subfamily;
        sal_Unicode *usubfamily;

        sal_uInt32  ntables;

    TrueTypeFont(const char* pFileName = nullptr, const FontCharMapRef xCharMap = nullptr);
    ~TrueTypeFont() override;

    SFErrCodes open(sal_uInt32 facenum);

    bool hasTable(sal_uInt32 ord) const override { return m_aTableList[ord].pData != nullptr; }
    inline const sal_uInt8* table(sal_uInt32 ord, sal_uInt32& size) const override;
};

const sal_uInt8* TrueTypeFont::table(sal_uInt32 ord, sal_uInt32& size) const
{
    if (ord >= NUM_TAGS)
    {
        size = 0;
        return nullptr;
    }

    auto& rTable = m_aTableList[ord];
    size = rTable.nSize;
    return rTable.pData;
}

} // namespace vcl

#endif // INCLUDED_VCL_INC_SFT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
