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
 * @author Alexander Gelfenbain
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

#ifdef UNX
#include <sys/types.h>
#include <unistd.h>
#endif

#include <vcl/dllapi.h>
#include <vcl/fontcapabilities.hxx>

#include <vector>
#include <cstdint>

namespace vcl
{

/*@{*/
    typedef sal_Int16       F2Dot14;            /**< fixed: 2.14 */
    typedef sal_Int32       F16Dot16;           /**< fixed: 16.16 */
/*@}*/

/** Return value of OpenTTFont() and CreateT3FromTTGlyphs() */
    enum SFErrCodes {
        SF_OK,                              /**< no error                                     */
        SF_BADFILE,                         /**< file not found                               */
        SF_FILEIO,                          /**< file I/O error                               */
        SF_MEMORY,                          /**< memory allocation error                      */
        SF_GLYPHNUM,                        /**< incorrect number of glyphs                   */
        SF_BADARG,                          /**< incorrect arguments                          */
        SF_TTFORMAT,                        /**< incorrect TrueType font format               */
        SF_TABLEFORMAT,                     /**< incorrect format of a TrueType table         */
        SF_FONTNO                           /**< incorrect logical font number of a TTC font  */
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

/** Type of the 'kern' table, stored in TrueTypeFont::kerntype */
    enum KernType {
        KT_NONE         = 0,                /**< no kern table                      */
        KT_APPLE_NEW    = 1,                /**< new Apple kern table               */
        KT_MICROSOFT    = 2                 /**< Microsoft table                    */
    };

/* Composite glyph flags definition */
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

/** Structure used by GetTTSimpleGlyphMetrics() and GetTTSimpleCharMetrics() functions */
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
        sal_uInt16 languageID;                  /**< Language ID                                            */
        sal_uInt16 nameID;                      /**< Name ID                                                */
        sal_uInt16 slen;                        /**< String length in bytes                                 */
        sal_uInt8  *sptr;                        /**< Pointer to string data (not zero-terminated!)          */
    } NameRecord;

/** Return value of GetTTGlobalFontInfo() */

    typedef struct {
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
        int   vascent;            /**< typographic ascent for vertical writing mode            */
        int   vdescent;           /**< typographic descent for vertical writing mode           */
        int   typoAscender;       /**< OS/2 portable typographic ascender                      */
        int   typoDescender;      /**< OS/2 portable typographic descender                     */
        int   typoLineGap;        /**< OS/2 portable typographic line gap                       */
        int   winAscent;          /**< ascender metric for Windows                             */
        int   winDescent;         /**< descender metric for Windows                            */
        bool  symbolEncoded;      /**< true: MS symbol encoded */
        int   rangeFlag;          /**< if set to 1 Unicode Range flags are applicable          */
        sal_uInt32 ur1;           /**< bits 0 - 31 of Unicode Range flags                      */
        sal_uInt32 ur2;           /**< bits 32 - 63 of Unicode Range flags                     */
        sal_uInt32 ur3;           /**< bits 64 - 95 of Unicode Range flags                     */
        sal_uInt32 ur4;           /**< bits 96 - 127 of Unicode Range flags                    */
        sal_uInt8  panose[10];    /**< PANOSE classification number                            */
        sal_uInt32 typeFlags;     /**< type flags (copyright bits + PS-OpenType flag)       */
        sal_uInt16 fsSelection;   /**< OS/2 fsSelection */
    } TTGlobalFontInfo;

#define TYPEFLAG_INVALID        0x8000000
#define TYPEFLAG_COPYRIGHT_MASK 0x000000E
#define TYPEFLAG_PS_OPENTYPE    0x0010000

/** Structure used by KernGlyphs()      */
    typedef struct {
        int x;                    /**< positive: right, negative: left                        */
        int y;                    /**< positive: up, negative: down                           */
    } KernData;

/** ControlPoint structure used by GetTTGlyphPoints() */
    typedef struct {
        sal_uInt32 flags;             /**< 00000000 00000000 e0000000 bbbbbbbb */
        /**< b - byte flags from the glyf array  */
        /**< e == 0 - regular point              */
        /**< e == 1 - end contour                */
        sal_Int16 x;                  /**< X coordinate in EmSquare units      */
        sal_Int16 y;                  /**< Y coordinate in EmSquare units      */
    } ControlPoint;

    struct TrueTypeFont;

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
 * @param  ttf     - array of TrueTypeFonts
 * @return value of SFErrCodes enum
 * @ingroup sft
 */
    int VCL_DLLPUBLIC OpenTTFontBuffer(const void* pBuffer, sal_uInt32 nLen, sal_uInt32 facenum, TrueTypeFont** ttf);
#if !defined(_WIN32)
/**
 * TrueTypeFont constructor.
 * Reads the font file and allocates the memory for the structure.
 * on WIN32 the font has to be provided as a memory buffer and length
 * @param  fname   - name of TrueType font file
 * @param  facenum - logical font number within a TTC file. This value is ignored
 *                   for TrueType fonts
 * @param  ttf     - array of TrueTypeFonts
 * @return value of SFErrCodes enum
 * @ingroup sft
 */
    int VCL_DLLPUBLIC OpenTTFontFile(const char *fname, sal_uInt32 facenum, TrueTypeFont** ttf);
#endif

    bool getTTCoverage(
        boost::optional<std::bitset<UnicodeCoverage::MAX_UC_ENUM>> & rUnicodeCoverage,
        boost::optional<std::bitset<CodePageCoverage::MAX_CP_ENUM>> & rCodePageCoverage,
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
    int GetTTGlyphPoints(TrueTypeFont *ttf, sal_uInt32 glyphID, ControlPoint **pointArray);

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
    GlyphData *GetTTRawGlyphData(TrueTypeFont *ttf, sal_uInt32 glyphID);

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
    int GetTTGlyphComponents(TrueTypeFont *ttf, sal_uInt32 glyphID, std::vector< sal_uInt32 >& glyphlist);

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

    int GetTTNameRecords(TrueTypeFont *ttf, NameRecord **nr);

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
    int  CreateT3FromTTGlyphs(TrueTypeFont *ttf, FILE *outf, const char *fname, sal_uInt16 *glyphArray, sal_uInt8 *encoding, int nGlyphs, int wmode);

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
 * @param nNameRecs   number of NameRecords for the font, if 0 the name table from the
 *                    original font will be used
 * @param nr          array of NameRecords
 * @param flags       or'ed TTCreationFlags
 * @return            return the value of SFErrCodes enum
 * @see               SFErrCodes
 * @ingroup sft
 *
 */
    int  CreateTTFromTTGlyphs(TrueTypeFont  *ttf,
                              const char    *fname,
                              sal_uInt16    *glyphArray,
                              sal_uInt8     *encoding,
                              int            nGlyphs,
                              int            nNameRecs,
                              NameRecord    *nr);

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
 * @return            SF_OK - no errors
 *                    SF_GLYPHNUM - too many glyphs (> 255)
 *                    SF_TTFORMAT - corrupted TrueType fonts
 *
 * @see               SFErrCodes
 * @ingroup sft
 *
 */
    int  CreateT42FromTTGlyphs(TrueTypeFont  *ttf,
                               FILE          *outf,
                               const char    *psname,
                               sal_uInt16        *glyphArray,
                               sal_uInt8          *encoding,
                               int            nGlyphs);

/**
 * Queries glyph metrics. Allocates an array of TTSimpleGlyphMetrics structs and returns it.
 *
 * @param ttf         pointer to the TrueTypeFont structure
 * @param glyphArray  pointer to an array of glyphs that are to be extracted from ttf
 * @param nGlyphs     number of glyph IDs in glyphArray and encoding values in encoding
 * @param vertical    writing mode: false - horizontal, true - vertical
 * @ingroup sft
 *
 */
    TTSimpleGlyphMetrics *GetTTSimpleGlyphMetrics(TrueTypeFont *ttf, sal_uInt16 *glyphArray, int nGlyphs, bool vertical);

#if defined(_WIN32) || defined(MACOSX) || defined(IOS)
/**
 * Maps a Unicode (UCS-2) character to a glyph ID and returns it. Missing glyph has
 * a glyphID of 0 so this function can be used to test if a character is encoded in the font.
 *
 * @param ttf         pointer to the TrueTypeFont structure
 * @param ch          Unicode (UCS-2) character
 * @return glyph ID, if the character is missing in the font, the return value is 0.
 * @ingroup sft
 */
    sal_uInt16 MapChar(TrueTypeFont *ttf, sal_uInt16 ch);
#endif

/**
 * Returns global font information about the TrueType font.
 * @see TTGlobalFontInfo
 *
 * @param ttf         pointer to a TrueTypeFont structure
 * @param info        pointer to a TTGlobalFontInfo structure
 * @ingroup sft
 *
 */
    void GetTTGlobalFontInfo(TrueTypeFont *ttf, TTGlobalFontInfo *info);

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
 void GetTTFontMterics(const std::vector<uint8_t>& hhea,
                       const std::vector<uint8_t>& os2,
                       TTGlobalFontInfo *info);

/**
 * returns the number of glyphs in a font
 */
 int GetTTGlyphCount( TrueTypeFont* ttf );

/**
 * provide access to the raw data of a SFNT-container's subtable
 */
 bool GetSfntTable( TrueTypeFont* ttf, int nSubtableIndex,
     const sal_uInt8** ppRawBytes, int* pRawLength );

/*- private definitions */

    struct TrueTypeFont {
        sal_uInt32 tag;

        char        *fname;
        sal_Int32   fsize;
        sal_uInt8   *ptr;

        char        *psname;
        char        *family;
        sal_Unicode *ufamily;
        char        *subfamily;
        sal_Unicode *usubfamily;

        sal_uInt32  ntables;
        sal_uInt32  *goffsets;
        sal_uInt32  nglyphs;
        sal_uInt32  unitsPerEm;
        sal_uInt32  numberOfHMetrics;
        sal_uInt32  numOfLongVerMetrics;                   /* if this number is not 0, font has vertical metrics information */
        const sal_uInt8* cmap;
        int         cmapType;
        sal_uInt32 (*mapper)(const sal_uInt8 *, sal_uInt32, sal_uInt32); /* character to glyphID translation function                          */
        const sal_uInt8   **tables;                        /* array of pointers to raw subtables in SFNT file                    */
        sal_uInt32  *tlens;                                /* array of table lengths                                             */
        int         kerntype;                              /* Defined in the KernType enum                                       */
        sal_uInt32  nkern;                                 /* number of kern subtables                                           */
        const sal_uInt8** kerntables;                      /* array of pointers to kern subtables                                */
        void        *pGSubstitution;                       /* info provided by GSUB for UseGSUB()                                */
    };

/* indexes into TrueTypeFont::tables[] and TrueTypeFont::tlens[] */
#define O_maxp 0     /* 'maxp' */
#define O_glyf 1     /* 'glyf' */
#define O_head 2     /* 'head' */
#define O_loca 3     /* 'loca' */
#define O_name 4     /* 'name' */
#define O_hhea 5     /* 'hhea' */
#define O_hmtx 6     /* 'hmtx' */
#define O_cmap 7     /* 'cmap' */
#define O_vhea 8     /* 'vhea' */
#define O_vmtx 9     /* 'vmtx' */
#define O_OS2  10    /* 'OS/2' */
#define O_post 11    /* 'post' */
#define O_kern 12    /* 'kern' */
#define O_cvt  13    /* 'cvt_' - only used in TT->TT generation */
#define O_prep 14    /* 'prep' - only used in TT->TT generation */
#define O_fpgm 15    /* 'fpgm' - only used in TT->TT generation */
#define O_gsub 16    /* 'GSUB' */
#define O_CFF  17    /* 'CFF' */
#define NUM_TAGS 18

} // namespace vcl

#endif // INCLUDED_VCL_INC_SFT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
