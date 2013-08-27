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


/*
 * Sun Font Tools
 *
 * Author: Alexander Gelfenbain
 *
 */

#include <assert.h>

#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#ifdef UNX
#include <sys/mman.h>
#include <sys/stat.h>
#endif
#include "sft.hxx"
#include "gsub.h"
#if ! (defined(NO_TTCR) && defined(NO_TYPE42))
#include "ttcr.hxx"
#endif
#ifndef NO_MAPPERS            /* include MapChar() and MapString() */
#include "xlat.hxx"
#endif
#ifndef NO_TYPE3              /* include CreateT3FromTTGlyphs() */
#include <rtl/crc.h>
#endif

#include <osl/endian.h>
#include <algorithm>

namespace vcl
{

/*- module identification */

static const char *modname  = "SunTypeTools-TT";
static const char *modver   = "1.0";
static const char *modextra = "gelf";

/*- private functions, constants and data types */ /*FOLD00*/

enum PathSegmentType {
    PS_NOOP      = 0,
    PS_MOVETO    = 1,
    PS_LINETO    = 2,
    PS_CURVETO   = 3,
    PS_CLOSEPATH = 4
};

struct PSPathElement
{
    PathSegmentType type;
    int x1, y1;
    int x2, y2;
    int x3, y3;

    PSPathElement( PathSegmentType i_eType ) : type( i_eType ),
                                   x1( 0 ), y1( 0 ),
                                   x2( 0 ), y2( 0 ),
                                   x3( 0 ), y3( 0 )
    {
    }
};

/*- In horizontal writing mode right sidebearing is calculated using this formula
 *- rsb = aw - (lsb + xMax - xMin) -*/
typedef struct {
    sal_Int16  xMin;
    sal_Int16  yMin;
    sal_Int16  xMax;
    sal_Int16  yMax;
    sal_uInt16 aw;                /*- Advance Width (horizontal writing mode)    */
    sal_Int16  lsb;               /*- Left sidebearing (horizontal writing mode) */
    sal_uInt16 ah;                /*- advance height (vertical writing mode)     */
    sal_Int16  tsb;               /*- top sidebearing (vertical writing mode)    */
} TTGlyphMetrics;

#define HFORMAT_LINELEN 64

typedef struct {
    FILE *o;
    char buffer[HFORMAT_LINELEN];
    size_t bufpos;
    int total;
} HexFmt;

typedef struct {
    sal_uInt32 nGlyphs;           /* number of glyphs in the font + 1 */
    sal_uInt32 *offs;             /* array of nGlyphs offsets */
} GlyphOffsets;

/* private tags */
static const sal_uInt32 TTFontClassTag = 0x74746663;  /* 'ttfc' */

static const sal_uInt32 T_true = 0x74727565;        /* 'true' */
static const sal_uInt32 T_ttcf = 0x74746366;        /* 'ttcf' */
static const sal_uInt32 T_otto = 0x4f54544f;        /* 'OTTO' */

/* standard TrueType table tags */
#define T_maxp 0x6D617870
#define T_glyf 0x676C7966
#define T_head 0x68656164
#define T_loca 0x6C6F6361
#define T_name 0x6E616D65
#define T_hhea 0x68686561
#define T_hmtx 0x686D7478
#define T_cmap 0x636D6170
#define T_vhea 0x76686561
#define T_vmtx 0x766D7478
#define T_OS2  0x4F532F32
#define T_post 0x706F7374
#define T_kern 0x6B65726E
#define T_cvt  0x63767420
#define T_prep 0x70726570
#define T_fpgm 0x6670676D
#define T_gsub 0x47535542
#define T_CFF  0x43464620

#define LAST_URANGE_BIT 69
const char *ulcodes[LAST_URANGE_BIT+2] = {
    /*  0   */  "Basic Latin",
    /*  1   */  "Latin-1 Supplement",
    /*  2   */  "Latin Extended-A",
    /*  3   */  "Latin Extended-B",
    /*  4   */  "IPA Extensions",
    /*  5   */  "Spacing Modifier Letters",
    /*  6   */  "Combining Diacritical Marks",
    /*  7   */  "Basic Greek",
    /*  8   */  "Greek Symbols And Coptic",
    /*  9   */  "Cyrillic",
    /*  10  */  "Armenian",
    /*  11  */  "Basic Hebrew",
    /*  12  */  "Hebrew Extended (A and B blocks combined)",
    /*  13  */  "Basic Arabic",
    /*  14  */  "Arabic Extended",
    /*  15  */  "Devanagari",
    /*  16  */  "Bengali",
    /*  17  */  "Gurmukhi",
    /*  18  */  "Gujarati",
    /*  19  */  "Oriya",
    /*  20  */  "Tamil",
    /*  21  */  "Telugu",
    /*  22  */  "Kannada",
    /*  23  */  "Malayalam",
    /*  24  */  "Thai",
    /*  25  */  "Lao",
    /*  26  */  "Basic Georgian",
    /*  27  */  "Georgian Extended",
    /*  28  */  "Hangul Jamo",
    /*  29  */  "Latin Extended Additional",
    /*  30  */  "Greek Extended",
    /*  31  */  "General Punctuation",
    /*  32  */  "Superscripts And Subscripts",
    /*  33  */  "Currency Symbols",
    /*  34  */  "Combining Diacritical Marks For Symbols",
    /*  35  */  "Letterlike Symbols",
    /*  36  */  "Number Forms",
    /*  37  */  "Arrows",
    /*  38  */  "Mathematical Operators",
    /*  39  */  "Miscellaneous Technical",
    /*  40  */  "Control Pictures",
    /*  41  */  "Optical Character Recognition",
    /*  42  */  "Enclosed Alphanumerics",
    /*  43  */  "Box Drawing",
    /*  44  */  "Block Elements",
    /*  45  */  "Geometric Shapes",
    /*  46  */  "Miscellaneous Symbols",
    /*  47  */  "Dingbats",
    /*  48  */  "CJK Symbols And Punctuation",
    /*  49  */  "Hiragana",
    /*  50  */  "Katakana",
    /*  51  */  "Bopomofo",
    /*  52  */  "Hangul Compatibility Jamo",
    /*  53  */  "CJK Miscellaneous",
    /*  54  */  "Enclosed CJK Letters And Months",
    /*  55  */  "CJK Compatibility",
    /*  56  */  "Hangul",
    /*  57  */  "Reserved for Unicode SubRanges",
    /*  58  */  "Reserved for Unicode SubRanges",
    /*  59  */  "CJK Unified Ideographs",
    /*  60  */  "Private Use Area",
    /*  61  */  "CJK Compatibility Ideographs",
    /*  62  */  "Alphabetic Presentation Forms",
    /*  63  */  "Arabic Presentation Forms-A",
    /*  64  */  "Combining Half Marks",
    /*  65  */  "CJK Compatibility Forms",
    /*  66  */  "Small Form Variants",
    /*  67  */  "Arabic Presentation Forms-B",
    /*  68  */  "Halfwidth And Fullwidth Forms",
    /*  69  */  "Specials",
    /*70-127*/  "Reserved for Unicode SubRanges"
};



/*- inline functions */ /*FOLD01*/
#ifdef __GNUC__
#define _inline static __inline__
#else
#define _inline static
#endif

_inline void *smalloc(size_t size)
{
    void *res = malloc(size);
    assert(res != 0);
    return res;
}

_inline void *scalloc(size_t n, size_t size)
{
    void *res = calloc(n, size);
    assert(res != 0);
    return res;
}

_inline sal_uInt32 mkTag(sal_uInt8 a, sal_uInt8 b, sal_uInt8 c, sal_uInt8 d) {
    return (a << 24) | (b << 16) | (c << 8) | d;
}

/*- Data access macros for data stored in big-endian or little-endian format */
_inline sal_Int16 GetInt16(const sal_uInt8 *ptr, size_t offset, int bigendian)
{
    sal_Int16 t;
    assert(ptr != 0);

    if (bigendian) {
        t = (ptr+offset)[0] << 8 | (ptr+offset)[1];
    } else {
        t = (ptr+offset)[1] << 8 | (ptr+offset)[0];
    }

    return t;
}

_inline sal_uInt16 GetUInt16(const sal_uInt8 *ptr, size_t offset, int bigendian)
{
    sal_uInt16 t;
    assert(ptr != 0);

    if (bigendian) {
        t = (ptr+offset)[0] << 8 | (ptr+offset)[1];
    } else {
        t = (ptr+offset)[1] << 8 | (ptr+offset)[0];
    }

    return t;
}

_inline sal_Int32  GetInt32(const sal_uInt8 *ptr, size_t offset, int bigendian)
{
    sal_Int32 t;
    assert(ptr != 0);

    if (bigendian) {
        t = (ptr+offset)[0] << 24 | (ptr+offset)[1] << 16 |
            (ptr+offset)[2] << 8  | (ptr+offset)[3];
    } else {
        t = (ptr+offset)[3] << 24 | (ptr+offset)[2] << 16 |
            (ptr+offset)[1] << 8  | (ptr+offset)[0];
    }

    return t;
}

_inline sal_uInt32 GetUInt32(const sal_uInt8 *ptr, size_t offset, int bigendian)
{
    sal_uInt32 t;
    assert(ptr != 0);


    if (bigendian) {
        t = (ptr+offset)[0] << 24 | (ptr+offset)[1] << 16 |
            (ptr+offset)[2] << 8  | (ptr+offset)[3];
    } else {
        t = (ptr+offset)[3] << 24 | (ptr+offset)[2] << 16 |
            (ptr+offset)[1] << 8  | (ptr+offset)[0];
    }

    return t;
}

_inline void PutInt16(sal_Int16 val, sal_uInt8 *ptr, size_t offset, int bigendian)
{
    assert(ptr != 0);

    if (bigendian) {
        ptr[offset] = (sal_uInt8)((val >> 8) & 0xFF);
        ptr[offset+1] = (sal_uInt8)(val & 0xFF);
    } else {
        ptr[offset+1] = (sal_uInt8)((val >> 8) & 0xFF);
        ptr[offset] = (sal_uInt8)(val & 0xFF);
    }

}

#if defined(OSL_BIGENDIAN)
#define Int16FromMOTA(a) (a)
#define Int32FromMOTA(a) (a)
#else
static sal_uInt16 Int16FromMOTA(sal_uInt16 a) {
  return (sal_uInt16) (((sal_uInt8)((a) >> 8)) | ((sal_uInt8)(a) << 8));
}
static sal_uInt32 Int32FromMOTA(sal_uInt32 a) {
  return ((a>>24)&0xFF) | (((a>>8)&0xFF00) | ((a&0xFF00)<<8) | ((a&0xFF)<<24));
}
#endif

_inline F16Dot16 fixedMul(F16Dot16 a, F16Dot16 b)
{
    unsigned int a1, b1;
    unsigned int a2, b2;
    F16Dot16 res;
    int sign;

    sign = (a & 0x80000000) ^ (b & 0x80000000);
    if (a < 0) a = -a;
    if (b < 0) b = -b;

    a1 = a >> 16;
    b1 = a & 0xFFFF;
    a2 = b >> 16;
    b2 = b & 0xFFFF;

    res = a1 * a2;

    /* if (res  > 0x7FFF) assert(!"fixedMul: F16Dot16 overflow"); */

    res <<= 16;
    res += a1 * b2 + b1 * a2 + ((b1 * b2) >> 16);

    return sign ? -res : res;
}


_inline F16Dot16 fixedDiv(F16Dot16 a, F16Dot16 b)
{
    unsigned int f, r;
    F16Dot16 res;
    int sign;

    sign = (a & 0x80000000) ^ (b & 0x80000000);
    if (a < 0) a = -a;
    if (b < 0) b = -b;

    f = a / b;
    r = a % b;

    /* if (f > 0x7FFFF) assert(!"fixedDiv: F16Dot16 overflow"); */

    while (r > 0xFFFF) {
        r >>= 1;
        b >>= 1;
    }

    res = (f << 16) + (r << 16) / b;

    return sign ? -res : res;
}

/*- returns a * b / c -*/
/* XXX provide a real implementation that preserves accuracy */
_inline F16Dot16 fixedMulDiv(F16Dot16 a, F16Dot16 b, F16Dot16 c)
{
    F16Dot16 res;

    res = fixedMul(a, b);
    return fixedDiv(res, c);
}

/*- Translate units from TT to PS (standard 1/1000) -*/
_inline int XUnits(int unitsPerEm, int n)
{
    return (n * 1000) / unitsPerEm;
}

_inline const char *UnicodeRangeName(sal_uInt16 bit)
{
  if (bit > LAST_URANGE_BIT) bit = LAST_URANGE_BIT+1;

  return ulcodes[bit];
}

_inline const sal_uInt8* getTable( TrueTypeFont *ttf, sal_uInt32 ord)
{
    return (sal_uInt8*)ttf->tables[ord];
}

_inline sal_uInt32 getTableSize(TrueTypeFont *ttf, sal_uInt32 ord)
{
    return ttf->tlens[ord];
}

#ifndef NO_TYPE42
/* Hex Formatter functions */
static const char HexChars[] = "0123456789ABCDEF";

static HexFmt *HexFmtNew(FILE *outf)
{
    HexFmt* res = (HexFmt*)smalloc(sizeof(HexFmt));
    res->bufpos = res->total = 0;
    res->o = outf;
    return res;
}

static bool HexFmtFlush(HexFmt *_this)
{
    bool bRet = true;
    if (_this->bufpos) {
        size_t nWritten = fwrite(_this->buffer, 1, _this->bufpos, _this->o);
        bRet = nWritten == _this->bufpos;
        _this->bufpos = 0;
    }
    return bRet;
}

_inline void HexFmtOpenString(HexFmt *_this)
{
    fputs("<\n", _this->o);
}

_inline void HexFmtCloseString(HexFmt *_this)
{
    HexFmtFlush(_this);
    fputs("00\n>\n", _this->o);
}

_inline void HexFmtDispose(HexFmt *_this)
{
    HexFmtFlush(_this);
    free(_this);
}

static void HexFmtBlockWrite(HexFmt *_this, const void *ptr, sal_uInt32 size)
{
    sal_uInt8 Ch;
    sal_uInt32 i;

    if (_this->total + size > 65534) {
        HexFmtFlush(_this);
        HexFmtCloseString(_this);
        _this->total = 0;
        HexFmtOpenString(_this);
    }
    for (i=0; i<size; i++) {
        Ch = ((sal_uInt8 *) ptr)[i];
        _this->buffer[_this->bufpos++] = HexChars[Ch >> 4];
        _this->buffer[_this->bufpos++] = HexChars[Ch & 0xF];
        if (_this->bufpos == HFORMAT_LINELEN) {
            HexFmtFlush(_this);
            fputc('\n', _this->o);
        }

    }
    _this->total += size;
}
#endif



/* Outline Extraction functions */ /*FOLD01*/

/* fills the aw and lsb entries of the TTGlyphMetrics structure from hmtx table -*/
static void GetMetrics(TrueTypeFont *ttf, sal_uInt32 glyphID, TTGlyphMetrics *metrics)
{
    const sal_uInt8* table = getTable( ttf, O_hmtx );

    metrics->aw = metrics->lsb = metrics->ah = metrics->tsb = 0;
    if (!table || !ttf->numberOfHMetrics) return;

    if (glyphID < ttf->numberOfHMetrics) {
        metrics->aw  = GetUInt16(table, 4 * glyphID, 1);
        metrics->lsb = GetInt16(table, 4 * glyphID + 2, 1);
    } else {
        metrics->aw  = GetUInt16(table, 4 * (ttf->numberOfHMetrics - 1), 1);
        metrics->lsb = GetInt16(table + ttf->numberOfHMetrics * 4, (glyphID - ttf->numberOfHMetrics) * 2, 1);
    }

    table = getTable(ttf, O_vmtx);
    if( !table || !ttf->numOfLongVerMetrics )
        return;

    if (glyphID < ttf->numOfLongVerMetrics) {
        metrics->ah  = GetUInt16(table, 4 * glyphID, 1);
        metrics->tsb = GetInt16(table, 4 * glyphID + 2, 1);
    } else {
        metrics->ah  = GetUInt16(table, 4 * (ttf->numOfLongVerMetrics - 1), 1);
        metrics->tsb = GetInt16(table + ttf->numOfLongVerMetrics * 4, (glyphID - ttf->numOfLongVerMetrics) * 2, 1);
    }
}

static int GetTTGlyphOutline(TrueTypeFont *, sal_uInt32 , ControlPoint **, TTGlyphMetrics *, std::vector< sal_uInt32 >* );

/* returns the number of control points, allocates the pointArray */
static int GetSimpleTTOutline(TrueTypeFont *ttf, sal_uInt32 glyphID, ControlPoint **pointArray, TTGlyphMetrics *metrics) /*FOLD02*/
{
    const sal_uInt8* table = getTable( ttf, O_glyf );
    sal_uInt8 flag, n;
    sal_uInt16 t, lastPoint=0;
    int i, j, z;

    *pointArray = 0;

    /* printf("GetSimpleTTOutline(%d)\n", glyphID); */

    if( glyphID >= ttf->nglyphs )           /*- glyph is not present in the font */
        return 0;
    const sal_uInt8* ptr = table + ttf->goffsets[glyphID];
    const sal_Int16 numberOfContours = GetInt16(ptr, 0, 1);
    if( numberOfContours <= 0 )             /*- glyph is not simple */
        return 0;

    if (metrics) {                                                    /*- GetCompoundTTOutline() calls this function with NULL metrics -*/
        metrics->xMin = GetInt16(ptr, 2, 1);
        metrics->yMin = GetInt16(ptr, 4, 1);
        metrics->xMax = GetInt16(ptr, 6, 1);
        metrics->yMax = GetInt16(ptr, 8, 1);
        GetMetrics(ttf, glyphID, metrics);
    }

    /* determine the last point and be extra safe about it. But probably this code is not needed */

    for (i=0; i<numberOfContours; i++) {
        if ((t = GetUInt16(ptr, 10+i*2, 1)) > lastPoint) lastPoint = t;
    }

    sal_uInt16 instLen = GetUInt16(ptr, 10 + numberOfContours*2, 1);
    const sal_uInt8* p = ptr + 10 + 2 * numberOfContours + 2 + instLen;
    ControlPoint* pa = (ControlPoint*)calloc(lastPoint+1, sizeof(ControlPoint));

    i = 0;
    while (i <= lastPoint) {
        pa[i++].flags = (sal_uInt32) (flag = *p++);
        if (flag & 8) {                                     /*- repeat flag */
            n = *p++;
            for (j=0; j<n; j++) {
                if (i > lastPoint) {                        /*- if the font is really broken */
                    free(pa);
                    return 0;
                }
                pa[i++].flags = flag;
            }
        }
    }

    /*- Process the X coordinate */
    z = 0;
    for (i = 0; i <= lastPoint; i++) {
        if (pa[i].flags & 0x02) {
            if (pa[i].flags & 0x10) {
                z += (int) (*p++);
            } else {
                z -= (int) (*p++);
            }
        } else if ( !(pa[i].flags & 0x10)) {
            z += GetInt16(p, 0, 1);
            p += 2;
        }
        pa[i].x = (sal_Int16)z;
    }

    /*- Process the Y coordinate */
    z = 0;
    for (i = 0; i <= lastPoint; i++) {
        if (pa[i].flags & 0x04) {
            if (pa[i].flags & 0x20) {
                z += *p++;
            } else {
                z -= *p++;
            }
        } else if ( !(pa[i].flags & 0x20)) {
            z += GetInt16(p, 0, 1);
            p += 2;
        }
        pa[i].y = (sal_Int16)z;
    }

    for (i=0; i<numberOfContours; i++) {
        pa[GetUInt16(ptr, 10 + i * 2, 1)].flags |= 0x00008000;      /*- set the end contour flag */
    }

    *pointArray = pa;
    return lastPoint + 1;
}

static int GetCompoundTTOutline(TrueTypeFont *ttf, sal_uInt32 glyphID, ControlPoint **pointArray, TTGlyphMetrics *metrics, std::vector< sal_uInt32 >& glyphlist) /*FOLD02*/
{
    sal_uInt16 flags, index;
    sal_Int16 e, f, numberOfContours;
    const sal_uInt8* table = getTable( ttf, O_glyf );
    std::vector<ControlPoint> myPoints;
    ControlPoint *nextComponent, *pa;
    int i, np;
    F16Dot16 a = 0x10000, b = 0, c = 0, d = 0x10000, m, n, abs1, abs2, abs3;

    *pointArray = 0;
    /* printf("GetCompoundTTOutline(%d)\n", glyphID); */

    if (glyphID >= ttf->nglyphs)                          /*- incorrect glyphID */
        return 0;

    const sal_uInt8* ptr = table + ttf->goffsets[glyphID];
    if ((numberOfContours = GetInt16(ptr, 0, 1)) != -1)   /*- glyph is not compound */
        return 0;

    if (metrics) {
        metrics->xMin = GetInt16(ptr, 2, 1);
        metrics->yMin = GetInt16(ptr, 4, 1);
        metrics->xMax = GetInt16(ptr, 6, 1);
        metrics->yMax = GetInt16(ptr, 8, 1);
        GetMetrics(ttf, glyphID, metrics);
    }

    ptr += 10;

    do {
        flags = GetUInt16(ptr, 0, 1);
        /* printf("flags: 0x%X\n", flags); */
        index = GetUInt16(ptr, 2, 1);
        ptr += 4;

        if( std::find( glyphlist.begin(), glyphlist.end(), index ) != glyphlist.end() )
        {
#if OSL_DEBUG_LEVEL > 1
            fprintf(stderr, "Endless loop found in a compound glyph.\n");
            fprintf(stderr, "%d -> ", index);
            fprintf(stderr," [");
            for( std::vector< sal_uInt32 >::const_iterator it = glyphlist.begin();
                 it != glyphlist.end(); ++it )
            {
                fprintf( stderr,"%d ", (int) *it );
            }
            fprintf(stderr,"]\n");
        /**/
#endif
        }

        glyphlist.push_back( index );

        if ((np = GetTTGlyphOutline(ttf, index, &nextComponent, 0, &glyphlist)) == 0)
        {
            /* XXX that probably indicates a corrupted font */
#if OSL_DEBUG_LEVEL > 1
            fprintf(stderr, "An empty compound!\n");
            /* assert(!"An empty compound"); */
#endif
        }

        if( ! glyphlist.empty() )
            glyphlist.pop_back();

        if (flags & USE_MY_METRICS) {
            if (metrics) GetMetrics(ttf, index, metrics);
        }

        if (flags & ARG_1_AND_2_ARE_WORDS) {
            e = GetInt16(ptr, 0, 1);
            f = GetInt16(ptr, 2, 1);
            /* printf("ARG_1_AND_2_ARE_WORDS: %d %d\n", e & 0xFFFF, f & 0xFFFF); */
            ptr += 4;
        } else {
            if (flags & ARGS_ARE_XY_VALUES) {     /* args are signed */
                e = (sal_Int8) *ptr++;
                f = (sal_Int8) *ptr++;
                /* printf("ARGS_ARE_XY_VALUES: %d %d\n", e & 0xFF, f & 0xFF); */
            } else {                              /* args are unsigned */
                /* printf("!ARGS_ARE_XY_VALUES\n"); */
                e = *ptr++;
                f = *ptr++;
            }

        }

        a = d = 0x10000;
        b = c = 0;

        if (flags & WE_HAVE_A_SCALE) {
            a = GetInt16(ptr, 0, 1) << 2;
            d = a;
            ptr += 2;
        } else if (flags & WE_HAVE_AN_X_AND_Y_SCALE) {
            a = GetInt16(ptr, 0, 1) << 2;
            d = GetInt16(ptr, 2, 1) << 2;
            ptr += 4;
        } else if (flags & WE_HAVE_A_TWO_BY_TWO) {
            a = GetInt16(ptr, 0, 1) << 2;
            b = GetInt16(ptr, 2, 1) << 2;
            c = GetInt16(ptr, 4, 1) << 2;
            d = GetInt16(ptr, 6, 1) << 2;
            ptr += 8;
        }

        abs1 = (a < 0) ? -a : a;
        abs2 = (b < 0) ? -b : b;
        m    = (abs1 > abs2) ? abs1 : abs2;
        abs3 = abs1 - abs2;
        if (abs3 < 0) abs3 = -abs3;
        if (abs3 <= 33) m *= 2;

        abs1 = (c < 0) ? -c : c;
        abs2 = (d < 0) ? -d : d;
        n    = (abs1 > abs2) ? abs1 : abs2;
        abs3 = abs1 - abs2;
        if (abs3 < 0) abs3 = -abs3;
        if (abs3 <= 33) n *= 2;

        if (!ARGS_ARE_XY_VALUES) {      /* match the points */
            assert(!"ARGS_ARE_XY_VALUES is not implemented!!!\n");
        }

        for (i=0; i<np; i++) {
            F16Dot16 t;
            ControlPoint cp;
            cp.flags = nextComponent[i].flags;
            t = fixedMulDiv(a, nextComponent[i].x << 16, m) + fixedMulDiv(c, nextComponent[i].y << 16, m) + (e << 16);
            cp.x = (sal_Int16)(fixedMul(t, m) >> 16);
            t = fixedMulDiv(b, nextComponent[i].x << 16, n) + fixedMulDiv(d, nextComponent[i].y << 16, n) + (f << 16);
            cp.y = (sal_Int16)(fixedMul(t, n) >> 16);

            myPoints.push_back( cp );
        }

        free(nextComponent);

    } while (flags & MORE_COMPONENTS);



    np = myPoints.size();

    pa = (ControlPoint*)calloc(np, sizeof(ControlPoint));
    assert(pa != 0);

    if (np > 0)
        memcpy( pa, &myPoints[0], np*sizeof(ControlPoint) );

    *pointArray = pa;
    return np;
}

/* NOTE: GetTTGlyphOutline() returns -1 if the glyphID is incorrect,
 * but Get{Simple|Compound}GlyphOutline returns 0 in such a case.
 *
 * NOTE: glyphlist is the stack of glyphs traversed while constructing
 * a composite glyph. This is a safequard against endless recursion
 * in corrupted fonts.
 */
static int GetTTGlyphOutline(TrueTypeFont *ttf, sal_uInt32 glyphID, ControlPoint **pointArray, TTGlyphMetrics *metrics, std::vector< sal_uInt32 >* glyphlist)
{
    const sal_uInt8 *table = getTable( ttf, O_glyf );
    sal_Int16 numberOfContours;
    int res;
    *pointArray = 0;

    if (metrics) {
        memset(metrics, 0, sizeof(TTGlyphMetrics));         /*- metrics is initialized to all zeroes */
    }

    if (glyphID >= ttf->nglyphs) return -1;                 /**/

    const sal_uInt8* ptr = table + ttf->goffsets[glyphID];
    int length = ttf->goffsets[glyphID+1] - ttf->goffsets[glyphID];

    if (length == 0) {                                      /*- empty glyphs still have hmtx and vmtx metrics values */
        if (metrics) GetMetrics(ttf, glyphID, metrics);
        return 0;
    }

    numberOfContours = GetInt16(ptr, 0, 1);

    if (numberOfContours >= 0)
    {
        res=GetSimpleTTOutline(ttf, glyphID, pointArray, metrics);
    }
    else
    {
        std::vector< sal_uInt32 > aPrivList;
        aPrivList.push_back( glyphID );
        res = GetCompoundTTOutline(ttf, glyphID, pointArray, metrics, glyphlist ? *glyphlist : aPrivList );
    }

    return res;
}

#ifndef NO_TYPE3

/*- returns the number of items in the path -*/

static int BSplineToPSPath(ControlPoint *srcA, int srcCount, PSPathElement **path)
{
    std::vector< PSPathElement > aPathList;
    int nPathCount = 0;
    PSPathElement p( PS_NOOP );

    int x0 = 0, y0 = 0, x1 = 0, y1 = 0, x2, y2, curx, cury;
    int lastOff = 0;                                        /*- last point was off-contour */
    int scflag = 1;                                         /*- start contour flag */
    int ecflag = 0;                                         /*- end contour flag */
    int cp = 0;                                             /*- current point */
    int StartContour = 0, EndContour = 1;

    *path = 0;

    /* if (srcCount > 0) for(;;) */
    while (srcCount > 0) {                                  /*- srcCount does not get changed inside the loop. */
        if (scflag) {
            int l = cp;
            StartContour = cp;
            while (!(srcA[l].flags & 0x8000)) l++;
            EndContour = l;
            if (StartContour == EndContour) {
                if (cp + 1 < srcCount) {
                    cp++;
                    continue;
                } else {
                    break;
                }
            }
            p = PSPathElement(PS_MOVETO);
            if (!(srcA[cp].flags & 1)) {
                if (!(srcA[EndContour].flags & 1)) {
                    p.x1 = x0 = (srcA[cp].x + srcA[EndContour].x + 1) / 2;
                    p.y1 = y0 = (srcA[cp].y + srcA[EndContour].y + 1) / 2;
                } else {
                    p.x1 = x0 = srcA[EndContour].x;
                    p.y1 = y0 = srcA[EndContour].y;
                }
            } else {
                p.x1 = x0 = srcA[cp].x;
                p.y1 = y0 = srcA[cp].y;
                cp++;
            }
            aPathList.push_back( p );
            lastOff = 0;
            scflag = 0;
        }

        curx = srcA[cp].x;
        cury = srcA[cp].y;

        if (srcA[cp].flags & 1)
        {
            if (lastOff)
            {
                p = PSPathElement(PS_CURVETO);
                p.x1 = x0 + (2 * (x1 - x0) + 1) / 3;
                p.y1 = y0 + (2 * (y1 - y0) + 1) / 3;
                p.x2 = x1 + (curx - x1 + 1) / 3;
                p.y2 = y1 + (cury - y1 + 1) / 3;
                p.x3 = curx;
                p.y3 = cury;
                aPathList.push_back( p );
            }
            else
            {
                if (!(x0 == curx && y0 == cury))
                {                              /* eliminate empty lines */
                    p = PSPathElement(PS_LINETO);
                    p.x1 = curx;
                    p.y1 = cury;
                    aPathList.push_back( p );
                }
            }
            x0 = curx; y0 = cury; lastOff = 0;
        }
        else
        {
            if (lastOff)
            {
                x2 = (x1 + curx + 1) / 2;
                y2 = (y1 + cury + 1) / 2;
                p = PSPathElement(PS_CURVETO);
                p.x1 = x0 + (2 * (x1 - x0) + 1) / 3;
                p.y1 = y0 + (2 * (y1 - y0) + 1) / 3;
                p.x2 = x1 + (x2 - x1 + 1) / 3;
                p.y2 = y1 + (y2 - y1 + 1) / 3;
                p.x3 = x2;
                p.y3 = y2;
                aPathList.push_back( p );
                x0 = x2; y0 = y2;
                x1 = curx; y1 = cury;
            } else {
                x1 = curx; y1 = cury;
            }
            lastOff = true;
        }

        if (ecflag) {
            aPathList.push_back( PSPathElement(PS_CLOSEPATH) );
            scflag = 1;
            ecflag = 0;
            cp = EndContour + 1;
            if (cp >= srcCount) break;
            continue;
        }


        if (cp == EndContour) {
            cp = StartContour;
            ecflag = true;
        } else {
            cp++;
        }
    }

    if( (nPathCount = (int)aPathList.size()) > 0)
    {
        *path = (PSPathElement*)calloc(nPathCount, sizeof(PSPathElement));
        assert(*path != 0);
        memcpy( *path, &aPathList[0], nPathCount * sizeof(PSPathElement) );
    }

    return nPathCount;
}

#endif

/*- Extracts a string from the name table and allocates memory for it -*/

static char *nameExtract( const sal_uInt8* name, int nTableSize, int n, int dbFlag, sal_uInt16** ucs2result )
{
    char *res;
    const sal_uInt8* ptr = name + GetUInt16(name, 4, 1) + GetUInt16(name + 6, 12 * n + 10, 1);
    int len = GetUInt16(name+6, 12 * n + 8, 1);

    // sanity check
    if( (len <= 0) || ((ptr+len) > (name+nTableSize)) )
    {
        if( ucs2result )
            *ucs2result = NULL;
        return NULL;
    }

    if( ucs2result )
        *ucs2result = NULL;
    if (dbFlag) {
        res = (char*)malloc(1 + len/2);
        assert(res != 0);
        for (int i = 0; i < len/2; i++)
            res[i] = *(ptr + i * 2 + 1);
        res[len/2] = 0;
        if( ucs2result )
        {
            *ucs2result = (sal_uInt16*)malloc( len+2 );
            for (int i = 0; i < len/2; i++ )
                (*ucs2result)[i] = GetUInt16( ptr, 2*i, 1 );
            (*ucs2result)[len/2] = 0;
        }
    } else {
        res = (char*)malloc(1 + len);
        assert(res != 0);
        memcpy(res, ptr, len);
        res[len] = 0;
    }

    return res;
}

static int findname( const sal_uInt8 *name, sal_uInt16 n, sal_uInt16 platformID,
    sal_uInt16 encodingID, sal_uInt16 languageID, sal_uInt16 nameID )
{
    int l = 0, r = n-1, i;
    sal_uInt32 t1, t2;
    sal_uInt32 m1, m2;

    if (n == 0) return -1;

    m1 = (platformID << 16) | encodingID;
    m2 = (languageID << 16) | nameID;

    do {
        i = (l + r) >> 1;
        t1 = GetUInt32(name + 6, i * 12 + 0, 1);
        t2 = GetUInt32(name + 6, i * 12 + 4, 1);

        if (! ((m1 < t1) || ((m1 == t1) && (m2 < t2)))) l = i + 1;
        if (! ((m1 > t1) || ((m1 == t1) && (m2 > t2)))) r = i - 1;
    } while (l <= r);

    if (l - r == 2) {
        return l - 1;
    }

    return -1;
}

/* XXX marlett.ttf uses (3, 0, 1033) instead of (3, 1, 1033) and does not have any Apple tables.
 * Fix: if (3, 1, 1033) is not found - need to check for (3, 0, 1033)
 *
 * /d/fonts/ttzh_tw/Big5/Hanyi/ma6b5p uses (1, 0, 19) for English strings, instead of (1, 0, 0)
 * and does not have (3, 1, 1033)
 * Fix: if (1, 0, 0) and (3, 1, 1033) are not found need to look for (1, 0, *) - that will
 * require a change in algorithm
 *
 * /d/fonts/fdltest/Korean/h2drrm has unsorted names and a an unknown (to me) Mac LanguageID,
 * but (1, 0, 1042) strings usable
 * Fix: change algorithm, and use (1, 0, *) if both standard Mac and MS strings are not found
 */

static void GetNames(TrueTypeFont *t)
{
    const sal_uInt8* table = getTable( t, O_name );
    int nTableSize = getTableSize(t, O_name);

    if (nTableSize < 4)
    {
#if OSL_DEBUG_LEVEL > 1
        fprintf(stderr, "O_name table too small\n");
#endif
        return;
    }

    sal_uInt16 n = GetUInt16(table, 2, 1);
    int i, r;
    bool bPSNameOK = true;

    /* #129743# simple sanity check for name table entry count */
    if( nTableSize <= n * 12 + 6 )
        n = 0;

    /* PostScript name: preferred Microsoft */
    t->psname = NULL;
    if ((r = findname(table, n, 3, 1, 0x0409, 6)) != -1)
        t->psname = nameExtract(table, nTableSize, r, 1, NULL);
    if ( ! t->psname && (r = findname(table, n, 1, 0, 0, 6)) != -1)
        t->psname = nameExtract(table, nTableSize, r, 0, NULL);
    if ( ! t->psname && (r = findname(table, n, 3, 0, 0x0409, 6)) != -1)
    {
        // some symbol fonts like Marlett have a 3,0 name!
        t->psname = nameExtract(table, nTableSize, r, 1, NULL);
    }
    // for embedded font in Ghostscript PDFs
    if ( ! t->psname && (r = findname(table, n, 2, 2, 0, 6)) != -1)
    {
        t->psname = nameExtract(table, nTableSize, r, 0, NULL);
    }
    if ( ! t->psname )
    {
        if ( t->fname )
        {
            char* pReverse = t->fname + strlen(t->fname);
            /* take only last token of filename */
            while(pReverse != t->fname && *pReverse != '/') pReverse--;
            if(*pReverse == '/') pReverse++;
            t->psname = strdup(pReverse);
            assert(t->psname != 0);
            for (i=strlen(t->psname) - 1; i > 0; i--)
            {
                /*- Remove the suffix  -*/
                if (t->psname[i] == '.' ) {
                    t->psname[i] = 0;
                    break;
                }
            }
        }
        else
            t->psname = strdup( "Unknown" );
    }

    /* Font family and subfamily names: preferred Apple */
    t->family = NULL;
    if ((r = findname(table, n, 0, 0, 0, 1)) != -1)
        t->family = nameExtract(table, nTableSize, r, 1, &t->ufamily);
    if ( ! t->family && (r = findname(table, n, 3, 1, 0x0409, 1)) != -1)
        t->family = nameExtract(table, nTableSize, r, 1, &t->ufamily);
    if ( ! t->family && (r = findname(table, n, 1, 0, 0, 1)) != -1)
        t->family = nameExtract(table, nTableSize, r, 0, NULL);
    if ( ! t->family && (r = findname(table, n, 3, 1, 0x0411, 1)) != -1)
        t->family = nameExtract(table, nTableSize, r, 1, &t->ufamily);
    if ( ! t->family && (r = findname(table, n, 3, 0, 0x0409, 1)) != -1)
        t->family = nameExtract(table, nTableSize, r, 1, &t->ufamily);
    if ( ! t->family )
    {
        t->family = strdup(t->psname);
        assert(t->family != 0);
    }

    t->subfamily = NULL;
    t->usubfamily = NULL;
    if ((r = findname(table, n, 1, 0, 0, 2)) != -1)
        t->subfamily = nameExtract(table, nTableSize, r, 0, &t->usubfamily);
    if ( ! t->subfamily && (r = findname(table, n, 3, 1, 0x0409, 2)) != -1)
        t->subfamily = nameExtract(table, nTableSize, r, 1, &t->usubfamily);
    if ( ! t->subfamily )
    {
        t->subfamily = strdup("");
    }

    /* #i60349# sanity check psname
     * psname parctically has to be 7bit ascii and should not contains spaces
     * there is a class of broken fonts which do not fullfill that at all, so let's try
     * if the family name is 7bit ascii and take it instead if so
     */
    /* check psname */
    for( i = 0; t->psname[i] != 0 && bPSNameOK; i++ )
        if( t->psname[ i ] < 33 || (t->psname[ i ] & 0x80) )
            bPSNameOK = false;
    if( !bPSNameOK )
    {
        bool bReplace = true;
        /* check if family is a suitable replacement */
        if( t->ufamily && t->family )
        {
            for( i = 0; t->ufamily[ i ] != 0 && bReplace; i++ )
                if( t->ufamily[ i ] < 33 || t->ufamily[ i ] > 127 )
                    bReplace = false;
            if( bReplace )
            {
                free( t->psname );
                t->psname = strdup( t->family );
            }
        }
    }
}

enum cmapType {
    CMAP_NOT_USABLE           = -1,
    CMAP_MS_Symbol            = 10,
    CMAP_MS_Unicode           = 11,
    CMAP_MS_ShiftJIS          = 12,
    CMAP_MS_Big5              = 13,
    CMAP_MS_PRC               = 14,
    CMAP_MS_Wansung           = 15,
    CMAP_MS_Johab             = 16
};

#define MISSING_GLYPH_INDEX 0

/*
 * getGlyph[0246]() functions and friends are implemented by:
 * @author Manpreet Singh
 * getGlyph12() function and friends by:
 * @author HDU
 */
static sal_uInt32 getGlyph0(const sal_uInt8* cmap, sal_uInt32 c) {
    if (c <= 255) {
        return *(cmap + 6 + c);
    } else {
        return MISSING_GLYPH_INDEX;
    }
}

typedef struct _subHeader2 {
    sal_uInt16 firstCode;
    sal_uInt16 entryCount;
    sal_uInt16 idDelta;
    sal_uInt16 idRangeOffset;
} subHeader2;

static sal_uInt32 getGlyph2(const sal_uInt8 *cmap, sal_uInt32 c) {
    sal_uInt16 *CMAP2 = (sal_uInt16 *) cmap;
    sal_uInt8 theHighByte;

    sal_uInt8 theLowByte;
    subHeader2* subHeader2s;
    sal_uInt16* subHeader2Keys;
    sal_uInt16 firstCode;
    int k;
    sal_uInt32 ToReturn;

    theHighByte = (sal_uInt8)((c >> 8) & 0x00ff);
    theLowByte = (sal_uInt8)(c & 0x00ff);
    subHeader2Keys = CMAP2 + 3;
    subHeader2s = (subHeader2 *)(subHeader2Keys + 256);
    k = Int16FromMOTA(subHeader2Keys[theHighByte]) / 8;

    if(k == 0) {
        firstCode = Int16FromMOTA(subHeader2s[k].firstCode);
        if(theLowByte >= firstCode && theLowByte < (firstCode + Int16FromMOTA(subHeader2s[k].entryCount))) {
            return *((&(subHeader2s[0].idRangeOffset))
                     + (Int16FromMOTA(subHeader2s[0].idRangeOffset)/2)             /* + offset        */
                     + theLowByte                                                  /* + to_look       */
                     - Int16FromMOTA(subHeader2s[0].firstCode)
                     );
        } else {
            return MISSING_GLYPH_INDEX;
        }
    } else if (k > 0) {
        firstCode = Int16FromMOTA(subHeader2s[k].firstCode);
        if(theLowByte >= firstCode && theLowByte < (firstCode + Int16FromMOTA(subHeader2s[k].entryCount))) {
            ToReturn = *((&(subHeader2s[k].idRangeOffset))
                         + (Int16FromMOTA(subHeader2s[k].idRangeOffset)/2)
                         + theLowByte - firstCode);
            if(ToReturn == 0) {
                return MISSING_GLYPH_INDEX;
            } else {
                ToReturn += Int16FromMOTA(subHeader2s[k].idDelta);
                return (ToReturn & 0xFFFF);
            }
        } else {
            return MISSING_GLYPH_INDEX;
        }
    } else {
        return MISSING_GLYPH_INDEX;
    }
}

static sal_uInt32 getGlyph6(const sal_uInt8 *cmap, sal_uInt32 c) {
    sal_uInt16 firstCode, lastCode, count;
    sal_uInt16 *CMAP6 = (sal_uInt16 *) cmap;

    firstCode = Int16FromMOTA(*(CMAP6 + 3));
    count = Int16FromMOTA(*(CMAP6 + 4));
    lastCode = firstCode + count - 1;
    if (c < firstCode || c > lastCode) {
        return MISSING_GLYPH_INDEX;
    } else {
        return *((CMAP6 + 5)/*glyphIdArray*/ + (c - firstCode));
    }
}

static sal_uInt16 GEbinsearch(sal_uInt16 *ar, sal_uInt16 length, sal_uInt16 toSearch) {
    signed int low, mid, high, lastfound = 0xffff;
    sal_uInt16 res;
    if(length == (sal_uInt16)0 || length == (sal_uInt16)0xFFFF) {
        return (sal_uInt16)0xFFFF;
    }
    low = 0;
    high = length - 1;
    while(high >= low) {
        mid = (high + low)/2;
        res = Int16FromMOTA(*(ar+mid));
        if(res >= toSearch) {
            lastfound = mid;
            high = --mid;
        } else {
            low = ++mid;
        }
    }
    return (sal_uInt16)lastfound;
}


static sal_uInt32 getGlyph4(const sal_uInt8 *cmap, sal_uInt32 c) {
    sal_uInt16  i;
    int ToReturn;
    sal_uInt16  segCount;
    sal_uInt16 * startCode;
    sal_uInt16 * endCode;
    sal_uInt16 * idDelta;
    /* sal_uInt16 * glyphIdArray; */
    sal_uInt16 * idRangeOffset;
    /*sal_uInt16 * glyphIndexArray;*/
    sal_uInt16  *CMAP4 = (sal_uInt16 *) cmap;
    /* sal_uInt16  GEbinsearch(sal_uInt16 *ar, sal_uInt16 length, sal_uInt16 toSearch); */

    segCount = Int16FromMOTA(*(CMAP4 + 3))/2;
    endCode = CMAP4 + 7;
    i = GEbinsearch(endCode, segCount, (sal_uInt16)c);

    if (i == (sal_uInt16) 0xFFFF) {
        return MISSING_GLYPH_INDEX;
    }
    startCode = endCode + segCount + 1;

    if(Int16FromMOTA(startCode[i]) > c) {
        return MISSING_GLYPH_INDEX;
    }
    idDelta = startCode + segCount;
    idRangeOffset = idDelta + segCount;
    /*glyphIndexArray = idRangeOffset + segCount;*/

    if(Int16FromMOTA(idRangeOffset[i]) != 0) {
        c = Int16FromMOTA(*(&(idRangeOffset[i]) + (Int16FromMOTA(idRangeOffset[i])/2 + (c - Int16FromMOTA(startCode[i])))));
    }

    ToReturn = (Int16FromMOTA(idDelta[i]) + c) & 0xFFFF;
    return ToReturn;
}

static sal_uInt32 getGlyph12(const sal_uInt8 *pCmap, sal_uInt32 cChar) {
    const sal_uInt32* pCMAP12 = (const sal_uInt32*)pCmap;
    int nLength = Int32FromMOTA( pCMAP12[1] );
    int nGroups = Int32FromMOTA( pCMAP12[3] );
    int nLower = 0;
    int nUpper = nGroups;

    if( nUpper > (nLength-16)/12 )
        nUpper = (nLength-16)/12;

    /* binary search in "segmented coverage" subtable */
    while( nLower < nUpper ) {
        int nIndex = (nLower + nUpper) / 2;
        const sal_uInt32* pEntry = &pCMAP12[ 4 + 3*nIndex ];
        sal_uInt32 cStart = Int32FromMOTA( pEntry[0] );
        sal_uInt32 cLast  = Int32FromMOTA( pEntry[1] );
        if( cChar < cStart )
            nUpper = nIndex;
        else if( cChar > cLast )
            nLower = nIndex + 1;
        else { /* found matching entry! */
            sal_uInt32 nGlyph  = Int32FromMOTA( pEntry[2] );
            nGlyph += cChar - cStart;
            return nGlyph;
        }
    }

    return MISSING_GLYPH_INDEX;
}


static void FindCmap(TrueTypeFont *ttf)
{
    const sal_uInt8* table = getTable(ttf, O_cmap);
    sal_uInt32 table_size = getTableSize(ttf, O_cmap);
    sal_uInt16 ncmaps = GetUInt16(table, 2, 1);
    unsigned int i;
    sal_uInt32 AppleUni   = 0;              // Apple Unicode
    sal_uInt32 ThreeZero  = 0;              /* MS Symbol            */
    sal_uInt32 ThreeOne   = 0;              /* MS UCS-2             */
    sal_uInt32 ThreeTwo   = 0;              /* MS ShiftJIS          */
    sal_uInt32 ThreeThree = 0;              /* MS Big5              */
    sal_uInt32 ThreeFour  = 0;              /* MS PRC               */
    sal_uInt32 ThreeFive  = 0;              /* MS Wansung           */
    sal_uInt32 ThreeSix   = 0;              /* MS Johab             */

    for (i = 0; i < ncmaps; i++) {
        sal_uInt32 offset;
        sal_uInt16 pID, eID;

        /* sanity check, cmap entry must lie within table */
        if( i*8+4 > table_size )
            break;

        pID = GetUInt16(table, 4 + i * 8, 1);
        eID = GetUInt16(table, 6 + i * 8, 1);
        offset = GetUInt32(table, 8 + i * 8, 1);

         /* sanity check, cmap must lie within file */
        if( (table - ttf->ptr) + offset > (sal_uInt32)ttf->fsize )
            continue;

        /* Unicode tables in Apple fonts */
        if (pID == 0) {
            AppleUni = offset;
        }

        if (pID == 3) {
            switch (eID) {
                case 0: ThreeZero  = offset; break;
                case 10: // UCS-4
                case 1: ThreeOne   = offset; break;
                case 2: ThreeTwo   = offset; break;
                case 3: ThreeThree = offset; break;
                case 4: ThreeFour  = offset; break;
                case 5: ThreeFive  = offset; break;
                case 6: ThreeSix   = offset; break;
            }
        }
    }

    // fall back to AppleUnicode if there are no ThreeOne/Threezero tables
    if( AppleUni && !ThreeZero && !ThreeOne)
        ThreeOne = AppleUni;

    if (ThreeOne) {
        ttf->cmapType = CMAP_MS_Unicode;
        ttf->cmap = table + ThreeOne;
    } else if (ThreeTwo) {
        ttf->cmapType = CMAP_MS_ShiftJIS;
        ttf->cmap = table + ThreeTwo;
    } else if (ThreeThree) {
        ttf->cmapType = CMAP_MS_Big5;
        ttf->cmap = table + ThreeThree;
    } else if (ThreeFour) {
        ttf->cmapType = CMAP_MS_PRC;
        ttf->cmap = table + ThreeFour;
    } else if (ThreeFive) {
        ttf->cmapType = CMAP_MS_Wansung;
        ttf->cmap = table + ThreeFive;
    } else if (ThreeSix) {
        ttf->cmapType = CMAP_MS_Johab;
        ttf->cmap = table + ThreeSix;
    } else if (ThreeZero) {
        ttf->cmapType = CMAP_MS_Symbol;
        ttf->cmap = table + ThreeZero;
    } else {
        ttf->cmapType = CMAP_NOT_USABLE;
        ttf->cmap = 0;
    }

    if (ttf->cmapType != CMAP_NOT_USABLE) {
        switch (GetUInt16(ttf->cmap, 0, 1)) {
            case 0: ttf->mapper = getGlyph0; break;
            case 2: ttf->mapper = getGlyph2; break;
            case 4: ttf->mapper = getGlyph4; break;
            case 6: ttf->mapper = getGlyph6; break;
            case 12: ttf->mapper= getGlyph12; break;
            default:
#if OSL_DEBUG_LEVEL > 1
                /*- if the cmap table is really broken */
                printf("%s: %d is not a recognized cmap format.\n", ttf->fname, GetUInt16(ttf->cmap, 0, 1));
#endif
                ttf->cmapType = CMAP_NOT_USABLE;
                ttf->cmap = 0;
                ttf->mapper = 0;
        }
    }
}

static void GetKern(TrueTypeFont *ttf)
{
    const sal_uInt8* table = getTable(ttf, O_kern);
    const sal_uInt8 *ptr;

    if( !table )
        goto badtable;

    if (GetUInt16(table, 0, 1) == 0) {                                /* Traditional Microsoft style table with sal_uInt16 version and nTables fields */
        ttf->nkern = GetUInt16(table, 2, 1);
        ttf->kerntables = (const sal_uInt8**)calloc(ttf->nkern, sizeof(sal_uInt8 *));
        assert(ttf->kerntables != 0);
        ttf->kerntype = KT_MICROSOFT;
        ptr = table + 4;
        for( unsigned i = 0; i < ttf->nkern; ++i) {
            ttf->kerntables[i] = ptr;
            ptr += GetUInt16(ptr, 2, 1);
            /* sanity check */
            if( ptr > ttf->ptr+ttf->fsize )
            {
                free( ttf->kerntables );
                goto badtable;
            }
        }
        return;
    }

    if (GetUInt32(table, 0, 1) == 0x00010000) {                       /* MacOS style kern tables: fixed32 version and sal_uInt32 nTables fields */
        ttf->nkern = GetUInt32(table, 4, 1);
        ttf->kerntables = (const sal_uInt8**)calloc(ttf->nkern, sizeof(sal_uInt8 *));
        assert(ttf->kerntables != 0);
        ttf->kerntype = KT_APPLE_NEW;
        ptr = table + 8;
        for( unsigned i = 0; i < ttf->nkern; ++i) {
            ttf->kerntables[i] = ptr;
            ptr += GetUInt32(ptr, 0, 1);
            /* sanity check; there are some fonts that are broken in this regard */
            if( ptr > ttf->ptr+ttf->fsize )
            {
                free( ttf->kerntables );
                goto badtable;
            }
        }
        return;
    }

  badtable:
    ttf->kerntype = KT_NONE;
    ttf->kerntables = 0;

    return;
}

/*- Public functions */ /*FOLD00*/

int CountTTCFonts(const char* fname)
{
    int nFonts = 0;
    sal_uInt8 buffer[12];
    FILE* fd = fopen(fname, "rb");
    if( fd ) {
        if (fread(buffer, 1, 12, fd) == 12) {
            if(GetUInt32(buffer, 0, 1) == T_ttcf )
                nFonts = GetUInt32(buffer, 8, 1);
        }
        fclose(fd);
    }
    return nFonts;
}

static void allocTrueTypeFont( TrueTypeFont** ttf )
{
    *ttf = (TrueTypeFont*)calloc(1,sizeof(TrueTypeFont));
    if( *ttf != NULL )
    {
        (*ttf)->tag = 0;
        (*ttf)->fname = 0;
        (*ttf)->fsize = -1;
        (*ttf)->ptr = 0;
        (*ttf)->nglyphs = 0xFFFFFFFF;
        (*ttf)->pGSubstitution = 0;
    }
}

/* forward declariotn for the two entry points to use*/
static int doOpenTTFont( sal_uInt32 facenum, TrueTypeFont* t );

#if !defined(WIN32)
int OpenTTFontFile( const char* fname, sal_uInt32 facenum, TrueTypeFont** ttf )
{
    int ret, fd = -1;
    struct stat st;

    if (!fname || !*fname) return SF_BADFILE;

    allocTrueTypeFont( ttf );
    if( ! *ttf )
        return SF_MEMORY;

    (*ttf)->fname = strdup(fname);
    if( ! (*ttf)->fname )
    {
        ret = SF_MEMORY;
        goto cleanup;
    }

    fd = open(fname, O_RDONLY);

    if (fd == -1) {
        ret = SF_BADFILE;
        goto cleanup;
    }

    if (fstat(fd, &st) == -1) {
        ret = SF_FILEIO;
        goto cleanup;
    }

    (*ttf)->fsize = st.st_size;

    /* On Mac OS, most likely will happen if a Mac user renames a font file
     * to be .ttf when its really a Mac resource-based font.
     * Size will be 0, but fonts smaller than 4 bytes would be broken anyway.
     */
    if ((*ttf)->fsize == 0) {
        ret = SF_BADFILE;
        goto cleanup;
    }

    if (((*ttf)->ptr = (sal_uInt8 *) mmap(0, (*ttf)->fsize, PROT_READ, MAP_SHARED, fd, 0)) == MAP_FAILED) {
        ret = SF_MEMORY;
        goto cleanup;
    }
    close(fd);

    return doOpenTTFont( facenum, *ttf );

cleanup:
    if (fd != -1) close(fd);
    /*- t and t->fname have been allocated! */
    free((*ttf)->fname);
    free(*ttf);
    *ttf = NULL;
    return ret;
}
#endif

int OpenTTFontBuffer(const void* pBuffer, sal_uInt32 nLen, sal_uInt32 facenum, TrueTypeFont** ttf)
{
    allocTrueTypeFont( ttf );
    if( *ttf == NULL )
        return SF_MEMORY;

    (*ttf)->fname = NULL;
    (*ttf)->fsize = nLen;
    (*ttf)->ptr   = (sal_uInt8*)pBuffer;

    return doOpenTTFont( facenum, *ttf );
}

static int doOpenTTFont( sal_uInt32 facenum, TrueTypeFont* t )
{
    int i;
    sal_uInt32 length, tag;
    sal_uInt32 tdoffset = 0;        /* offset to TableDirectory in a TTC file. For TTF files is 0 */
    int indexfmt;

    sal_uInt32 version = GetInt32(t->ptr, 0, 1);

    if ((version == 0x00010000) || (version == T_true)) {
        tdoffset = 0;
    } else if (version == T_otto) {                         /* PS-OpenType font */
        tdoffset = 0;
    } else if (version == T_ttcf) {                         /* TrueType collection */
        if (GetUInt32(t->ptr, 4, 1) != 0x00010000) {
            CloseTTFont(t);
            return SF_TTFORMAT;
        }
        if (facenum >= GetUInt32(t->ptr, 8, 1)) {
            CloseTTFont(t);
            return SF_FONTNO;
        }
        tdoffset = GetUInt32(t->ptr, 12 + 4 * facenum, 1);
    } else {
        CloseTTFont(t);
        return SF_TTFORMAT;
    }

    /* magic number */
    t->tag = TTFontClassTag;

    t->ntables = GetUInt16(t->ptr + tdoffset, 4, 1);
    if( t->ntables >= 128 )
        return SF_TTFORMAT;

    t->tables = (const sal_uInt8**)calloc(NUM_TAGS, sizeof(sal_uInt8 *));
    assert(t->tables != 0);
    t->tlens = (sal_uInt32*)calloc(NUM_TAGS, sizeof(sal_uInt32));
    assert(t->tlens != 0);

    /* parse the tables */
    for (i=0; i<(int)t->ntables; i++) {
        int nIndex;
        tag = GetUInt32(t->ptr + tdoffset + 12, 16 * i, 1);
        switch( tag ) {
            case T_maxp: nIndex = O_maxp; break;
            case T_glyf: nIndex = O_glyf; break;
            case T_head: nIndex = O_head; break;
            case T_loca: nIndex = O_loca; break;
            case T_name: nIndex = O_name; break;
            case T_hhea: nIndex = O_hhea; break;
            case T_hmtx: nIndex = O_hmtx; break;
            case T_cmap: nIndex = O_cmap; break;
            case T_vhea: nIndex = O_vhea; break;
            case T_vmtx: nIndex = O_vmtx; break;
            case T_OS2 : nIndex = O_OS2;  break;
            case T_post: nIndex = O_post; break;
            case T_kern: nIndex = O_kern; break;
            case T_cvt : nIndex = O_cvt;  break;
            case T_prep: nIndex = O_prep; break;
            case T_fpgm: nIndex = O_fpgm; break;
            case T_gsub: nIndex = O_gsub; break;
            case T_CFF:  nIndex = O_CFF; break;
            default: nIndex = -1; break;
        }
        if( nIndex >= 0 ) {
            sal_uInt32 nTableOffset = GetUInt32(t->ptr + tdoffset + 12, 16 * i + 8, 1);
            length = GetUInt32(t->ptr + tdoffset + 12, 16 * i + 12, 1);
            t->tables[nIndex] = t->ptr + nTableOffset;
            t->tlens[nIndex] = length;
        }
    }

    /* Fixup offsets when only a TTC extract was provided */
    if( facenum == (sal_uInt32)~0 ) {
        sal_uInt8* pHead = (sal_uInt8*)t->tables[O_head];
        if( !pHead )
            return SF_TTFORMAT;
        /* limit Head candidate to TTC extract's limits */
        if( pHead > t->ptr + (t->fsize - 54) )
            pHead = t->ptr + (t->fsize - 54);
        /* TODO: find better method than searching head table's magic */
        sal_uInt8* p = NULL;
        for( p = pHead + 12; p > t->ptr; --p ) {
            if( p[0]==0x5F && p[1]==0x0F && p[2]==0x3C && p[3]==0xF5 ) {
                int nDelta = (pHead + 12) - p;
                if( nDelta )
                    for( int j = 0; j < NUM_TAGS; ++j )
                        if( t->tables[j] )
                            *(char**)&t->tables[j] -= nDelta;
                break;
            }
        }
        if( p <= t->ptr )
            return SF_TTFORMAT;
    }

    /* Check the table offsets after TTC correction */
    for (i=0; i<NUM_TAGS; i++) {
        /* sanity check: table must lay completely within the file
         * at this point one could check the checksum of all contained
         * tables, but this would be quite time intensive.
         * Try to fix tables, so we can cope with minor problems.
         */

        if( (sal_uInt8*)t->tables[i] < t->ptr )
        {
#if OSL_DEBUG_LEVEL > 1
            if( t->tables[i] )
                fprintf( stderr, "font file %s has bad table offset %" SAL_PRI_PTRDIFFT "d (tagnum=%d)\n", t->fname, (sal_uInt8*)t->tables[i]-t->ptr, i );
#endif
            t->tlens[i] = 0;
            t->tables[i] = NULL;
        }
        else if( (sal_uInt8*)t->tables[i] + t->tlens[i] > t->ptr + t->fsize )
        {
            int nMaxLen = (t->ptr + t->fsize) - (sal_uInt8*)t->tables[i];
            if( nMaxLen < 0 )
                nMaxLen = 0;
            t->tlens[i] = nMaxLen;
#if OSL_DEBUG_LEVEL > 1
            fprintf( stderr, "font file %s has too big table (tagnum=%d)\n", t->fname, i );
#endif
        }
    }

    /* At this point TrueTypeFont is constructed, now need to verify the font format
       and read the basic font properties */

    /* The following tables are absolutely required:
     * maxp, head, name, cmap
     */

    if( !(getTable(t, O_maxp) && getTable(t, O_head) && getTable(t, O_name) && getTable(t, O_cmap)) ) {
        CloseTTFont(t);
        return SF_TTFORMAT;
    }

    const sal_uInt8* table = getTable(t, O_maxp);
    t->nglyphs = GetUInt16(table, 4, 1);

    table = getTable(t, O_head);
    t->unitsPerEm = GetUInt16(table, 18, 1);
    indexfmt = GetInt16(table, 50, 1);

    if( ((indexfmt != 0) && (indexfmt != 1)) || (t->unitsPerEm <= 0) ) {
        CloseTTFont(t);
        return SF_TTFORMAT;
    }

    if( getTable(t, O_glyf) && getTable(t, O_loca) ) /* TTF or TTF-OpenType */
    {
        int k = (getTableSize(t, O_loca) / (indexfmt ? 4 : 2)) - 1;
        if( k < (int)t->nglyphs )       /* Hack for broken Chinese fonts */
            t->nglyphs = k;

        table = getTable(t, O_loca);
        t->goffsets = (sal_uInt32 *) calloc(1+t->nglyphs, sizeof(sal_uInt32));
        assert(t->goffsets != 0);

        for( i = 0; i <= (int)t->nglyphs; ++i )
            t->goffsets[i] = indexfmt ? GetUInt32(table, i << 2, 1) : (sal_uInt32)GetUInt16(table, i << 1, 1) << 1;
    } else if( getTable(t, O_CFF) ) {           /* PS-OpenType */
        t->goffsets = (sal_uInt32 *) calloc(1+t->nglyphs, sizeof(sal_uInt32));
        /* TODO: implement to get subsetting */
        assert(t->goffsets != 0);
    } else {
        CloseTTFont(t);
        return SF_TTFORMAT;
    }

    table = getTable(t, O_hhea);
    t->numberOfHMetrics = (table != 0) ? GetUInt16(table, 34, 1) : 0;

    table = getTable(t, O_vhea);
    t->numOfLongVerMetrics = (table != 0) ? GetUInt16(table, 34, 1) : 0;

    GetNames(t);
    FindCmap(t);
    GetKern(t);
    ReadGSUB( t, 0, 0 );

    return SF_OK;
}

void CloseTTFont(TrueTypeFont *ttf) /*FOLD01*/
{
#if !defined(WIN32)
    if( ttf->fname )
        munmap((char *) ttf->ptr, ttf->fsize);
#endif
    free(ttf->fname);
    free(ttf->goffsets);
    free(ttf->psname);
    free(ttf->family);
    if( ttf->ufamily )
        free( ttf->ufamily );
    free(ttf->subfamily);
    if( ttf->usubfamily )
        free( ttf->usubfamily );
    free(ttf->tables);
    free(ttf->tlens);
    free(ttf->kerntables);

    ReleaseGSUB(ttf);

    free(ttf);
    return;
}

int GetTTGlyphPoints(TrueTypeFont *ttf, sal_uInt32 glyphID, ControlPoint **pointArray)
{
    return GetTTGlyphOutline(ttf, glyphID, pointArray, 0, 0);
}

int GetTTGlyphComponents(TrueTypeFont *ttf, sal_uInt32 glyphID, std::vector< sal_uInt32 >& glyphlist)
{
    int n = 1;

    if( glyphID >= ttf->nglyphs )
        return 0;

    const sal_uInt8* glyf = getTable(ttf, O_glyf);
    const sal_uInt8* ptr = glyf + ttf->goffsets[glyphID];

    glyphlist.push_back( glyphID );

    if (GetInt16(ptr, 0, 1) == -1) {
        sal_uInt16 flags, index;
        ptr += 10;
        do {
            flags = GetUInt16(ptr, 0, 1);
            index = GetUInt16(ptr, 2, 1);

            ptr += 4;
            n += GetTTGlyphComponents(ttf, index, glyphlist);

            if (flags & ARG_1_AND_2_ARE_WORDS) {
                ptr += 4;
            } else {
                ptr += 2;
            }

            if (flags & WE_HAVE_A_SCALE) {
                ptr += 2;
            } else if (flags & WE_HAVE_AN_X_AND_Y_SCALE) {
                ptr += 4;
            } else if (flags & WE_HAVE_A_TWO_BY_TWO) {
                ptr += 8;
            }
        } while (flags & MORE_COMPONENTS);
    }

    return n;
}

#ifndef NO_TYPE3
int  CreateT3FromTTGlyphs(TrueTypeFont *ttf, FILE *outf, const char *fname, /*FOLD00*/
                          sal_uInt16 *glyphArray, sal_uInt8 *encoding, int nGlyphs,
                          int wmode)
{
    ControlPoint *pa;
    PSPathElement *path;
    int i, j, r, n;
    const sal_uInt8* table = getTable(ttf, O_head);
    TTGlyphMetrics metrics;
    int UPEm = ttf->unitsPerEm;

    const char *h01 = "%%!PS-AdobeFont-%d.%d-%d.%d\n";
    const char *h02 = "%% Creator: %s %s %s\n";
    const char *h09 = "%% Original font name: %s\n";

    const char *h10 =
        "30 dict begin\n"
        "/PaintType 0 def\n"
        "/FontType 3 def\n"
        "/StrokeWidth 0 def\n";

    const char *h11 = "/FontName (%s) cvn def\n";

    /*
      const char *h12 = "%/UniqueID %d def\n";
    */
    const char *h13 = "/FontMatrix [.001 0 0 .001 0 0] def\n";
    const char *h14 = "/FontBBox [%d %d %d %d] def\n";

    const char *h15=
        "/Encoding 256 array def\n"
        "    0 1 255 {Encoding exch /.notdef put} for\n";

    const char *h16 = "    Encoding %d /glyph%d put\n";
    const char *h17 = "/XUID [103 0 0 16#%08X %d 16#%08X 16#%08X] def\n";

    const char *h30 = "/CharProcs %d dict def\n";
    const char *h31 = "  CharProcs begin\n";
    const char *h32 = "    /.notdef {} def\n";
    const char *h33 = "    /glyph%d {\n";
    const char *h34 = "    } bind def\n";
    const char *h35 = "  end\n";

    const char *h40 =
        "/BuildGlyph {\n"
        "  exch /CharProcs get exch\n"
        "  2 copy known not\n"
        "    {pop /.notdef} if\n"
        "  get exec\n"
        "} bind def\n"
        "/BuildChar {\n"
        "  1 index /Encoding get exch get\n"
        "  1 index /BuildGlyph get exec\n"
        "} bind def\n"
        "currentdict end\n";

    const char *h41 = "(%s) cvn exch definefont pop\n";


    if (!((nGlyphs > 0) && (nGlyphs <= 256))) return SF_GLYPHNUM;
    if (!glyphArray) return SF_BADARG;
    if (!fname) fname = ttf->psname;

    fprintf(outf, h01, GetInt16(table, 0, 1), GetUInt16(table, 2, 1), GetInt16(table, 4, 1), GetUInt16(table, 6, 1));
    fprintf(outf, h02, modname, modver, modextra);
    fprintf(outf, h09, ttf->psname);

    fprintf(outf, "%s", h10);
    fprintf(outf, h11, fname);
/*    fprintf(outf, h12, 4000000); */

    /* XUID generation:
     * 103 0 0 C1 C2 C3 C4
     * C1 - CRC-32 of the entire source TrueType font
     * C2 - number of glyphs in the subset
     * C3 - CRC-32 of the glyph array
     * C4 - CRC-32 of the encoding array
     *
     * All CRC-32 numbers are presented as hexadecimal numbers
     */

    fprintf(outf, h17, rtl_crc32(0, ttf->ptr, ttf->fsize), nGlyphs, rtl_crc32(0, glyphArray, nGlyphs * 2), rtl_crc32(0, encoding, nGlyphs));
    fprintf(outf, "%s", h13);
    fprintf(outf, h14, XUnits(UPEm, GetInt16(table, 36, 1)), XUnits(UPEm, GetInt16(table, 38, 1)), XUnits(UPEm, GetInt16(table, 40, 1)), XUnits(UPEm, GetInt16(table, 42, 1)));
    fprintf(outf, "%s", h15);

    for (i = 0; i < nGlyphs; i++) {
        fprintf(outf, h16, encoding[i], i);
    }

    fprintf(outf, h30, nGlyphs+1);
    fprintf(outf, "%s", h31);
    fprintf(outf, "%s", h32);

    for (i = 0; i < nGlyphs; i++) {
        fprintf(outf, h33, i);
        r = GetTTGlyphOutline(ttf, glyphArray[i] < ttf->nglyphs ? glyphArray[i] : 0, &pa, &metrics, 0);

        if (r > 0) {
            n =  BSplineToPSPath(pa, r, &path);
        } else {
            n = 0;                      /* glyph might have zero contours but valid metrics ??? */
            path = 0;
            if (r < 0) {                /* glyph is not present in the font - pa array was not allocated, so no need to free it */
                continue;
            }
        }
        fprintf(outf, "\t%d %d %d %d %d %d setcachedevice\n",
                wmode == 0 ? XUnits(UPEm, metrics.aw) : 0,
                wmode == 0 ? 0 : -XUnits(UPEm, metrics.ah),
                XUnits(UPEm, metrics.xMin),
                XUnits(UPEm, metrics.yMin),
                XUnits(UPEm, metrics.xMax),
                XUnits(UPEm, metrics.yMax));

        for (j = 0; j < n; j++)
        {
            switch (path[j].type)
            {
                case PS_MOVETO:
                    fprintf(outf, "\t%d %d moveto\n", XUnits(UPEm, path[j].x1), XUnits(UPEm, path[j].y1));
                    break;

                case PS_LINETO:
                    fprintf(outf, "\t%d %d lineto\n", XUnits(UPEm, path[j].x1), XUnits(UPEm, path[j].y1));
                    break;

                case PS_CURVETO:
                    fprintf(outf, "\t%d %d %d %d %d %d curveto\n", XUnits(UPEm, path[j].x1), XUnits(UPEm, path[j].y1), XUnits(UPEm, path[j].x2), XUnits(UPEm, path[j].y2), XUnits(UPEm, path[j].x3), XUnits(UPEm, path[j].y3));
                    break;

                case PS_CLOSEPATH:
                    fprintf(outf, "\tclosepath\n");
                    break;
                case PS_NOOP:
                    break;
            }
        }
        if (n > 0) fprintf(outf, "\tfill\n");     /* if glyph is not a whitespace character */

        fprintf(outf, "%s", h34);

        free(pa);
        free(path);
    }
    fprintf(outf, "%s", h35);

    fprintf(outf, "%s", h40);
    fprintf(outf, h41, fname);

    return SF_OK;
}
#endif

#ifndef NO_TTCR
int  CreateTTFromTTGlyphs(TrueTypeFont  *ttf,
                          const char    *fname,
                          sal_uInt16        *glyphArray,
                          sal_uInt8          *encoding,
                          int            nGlyphs,
                          int            nNameRecs,
                          NameRecord    *nr,
                          sal_uInt32        flags)
{
    TrueTypeCreator *ttcr;
    TrueTypeTable *head=0, *hhea=0, *maxp=0, *cvt=0, *prep=0, *glyf=0, *fpgm=0, *cmap=0, *name=0, *post = 0, *os2 = 0;
    int i;
    int res;

    TrueTypeCreatorNewEmpty(T_true, &ttcr);

    /**                       name                         **/

    if (flags & TTCF_AutoName) {
        /* not implemented yet
           NameRecord *names;
           NameRecord newname;
           int n = GetTTNameRecords(ttf, &names);
           int n1 = 0, n2 = 0, n3 = 0, n4 = 0, n5 = 0, n6 = 0;
           sal_uInt8 *cp1;
           sal_uInt8 suffix[32];
           sal_uInt32 c1 = crc32(glyphArray, nGlyphs * 2);
           sal_uInt32 c2 = crc32(encoding, nGlyphs);
           int len;
           snprintf(suffix, 31, "S%08X%08X-%d", c1, c2, nGlyphs);

           name = TrueTypeTableNew_name(0, 0);
           for (i = 0; i < n; i++) {
           if (names[i].platformID == 1 && names[i].encodingID == 0 && names[i].languageID == 0 && names[i].nameID == 1) {

           memcpy(newname, names+i, sizeof(NameRecord));
           newname.slen = name[i].slen + strlen(suffix);
        */
        const sal_uInt8 ptr[] = {0,'T',0,'r',0,'u',0,'e',0,'T',0,'y',0,'p',0,'e',0,'S',0,'u',0,'b',0,'s',0,'e',0,'t'};
        NameRecord n1 = {1, 0, 0, 6, 14, (sal_uInt8*)"TrueTypeSubset"};
        NameRecord n2 = {3, 1, 1033, 6, 28, 0};
        n2.sptr = (sal_uInt8 *) ptr;
        name = TrueTypeTableNew_name(0, 0);
        nameAdd(name, &n1);
        nameAdd(name, &n2);
    } else {
        if (nNameRecs == 0) {
            NameRecord *names;
            int n = GetTTNameRecords(ttf, &names);
            name = TrueTypeTableNew_name(n, names);
            DisposeNameRecords(names, n);
        } else {
            name = TrueTypeTableNew_name(nNameRecs, nr);
        }
    }

    /**                       maxp                         **/
    maxp = TrueTypeTableNew_maxp(getTable(ttf, O_maxp), getTableSize(ttf, O_maxp));

    /**                       hhea                         **/
    const sal_uInt8* p = getTable(ttf, O_hhea);
    if (p) {
        hhea = TrueTypeTableNew_hhea(GetUInt16(p, 4, 1), GetUInt16(p, 6, 1), GetUInt16(p, 8, 1), GetUInt16(p, 18, 1), GetUInt16(p, 20, 1));
    } else {
        hhea = TrueTypeTableNew_hhea(0, 0, 0, 0, 0);
    }

    /**                       head                         **/

    p = getTable(ttf, O_head);
    assert(p != 0);
    head = TrueTypeTableNew_head(GetUInt32(p, 4, 1),
                                 GetUInt16(p, 16, 1),
                                 GetUInt16(p, 18, 1),
                                 p+20,
                                 GetUInt16(p, 44, 1),
                                 GetUInt16(p, 46, 1),
                                 GetInt16(p, 48, 1));


    /**                       glyf                          **/

    glyf = TrueTypeTableNew_glyf();
    sal_uInt32* gID = (sal_uInt32*)scalloc(nGlyphs, sizeof(sal_uInt32));

    for (i = 0; i < nGlyphs; i++) {
        gID[i] = glyfAdd(glyf, GetTTRawGlyphData(ttf, glyphArray[i]), ttf);
    }

    /**                       cmap                          **/
    cmap = TrueTypeTableNew_cmap();

    for (i=0; i < nGlyphs; i++) {
        cmapAdd(cmap, 0x010000, encoding[i], gID[i]);
    }

    /**                       cvt                           **/
    if ((p = getTable(ttf, O_cvt)) != 0) {
        cvt = TrueTypeTableNew(T_cvt, getTableSize(ttf, O_cvt), p);
    }

    /**                       prep                          **/
    if ((p = getTable(ttf, O_prep)) != 0) {
        prep = TrueTypeTableNew(T_prep, getTableSize(ttf, O_prep), p);
    }

    /**                       fpgm                          **/
    if ((p = getTable(ttf, O_fpgm)) != 0) {
        fpgm = TrueTypeTableNew(T_fpgm, getTableSize(ttf, O_fpgm), p);
    }

    /**                       post                          **/
    if ((p = getTable(ttf, O_post)) != 0) {
        post = TrueTypeTableNew_post(0x00030000,
                                     GetUInt32(p, 4, 1),
                                     GetUInt16(p, 8, 1),
                                     GetUInt16(p, 10, 1),
                                     GetUInt16(p, 12, 1));
    } else {
        post = TrueTypeTableNew_post(0x00030000, 0, 0, 0, 0);
    }

    if (flags & TTCF_IncludeOS2) {
        if ((p = getTable(ttf, O_OS2)) != 0) {
            os2 = TrueTypeTableNew(T_OS2, getTableSize(ttf, O_OS2), p);
        }
    }

    AddTable(ttcr, name); AddTable(ttcr, maxp); AddTable(ttcr, hhea);
    AddTable(ttcr, head); AddTable(ttcr, glyf); AddTable(ttcr, cmap);
    AddTable(ttcr, cvt ); AddTable(ttcr, prep); AddTable(ttcr, fpgm);
    AddTable(ttcr, post); AddTable(ttcr, os2);

    if ((res = StreamToFile(ttcr, fname)) != SF_OK) {
#if OSL_DEBUG_LEVEL > 1
        fprintf(stderr, "StreamToFile: error code: %d.\n", res);
#endif
    }

    TrueTypeCreatorDispose(ttcr);
    free(gID);

    return res;
}
#endif


#ifndef NO_TYPE42
static GlyphOffsets *GlyphOffsetsNew(sal_uInt8 *sfntP)
{
    GlyphOffsets* res = (GlyphOffsets*)smalloc(sizeof(GlyphOffsets));
    sal_uInt8 *loca = NULL;
    sal_uInt16 i, numTables = GetUInt16(sfntP, 4, 1);
    sal_uInt32 locaLen = 0;
    sal_Int16 indexToLocFormat = 0;

    for (i = 0; i < numTables; i++) {
        sal_uInt32 tag = GetUInt32(sfntP + 12, 16 * i, 1);
        sal_uInt32 off = GetUInt32(sfntP + 12, 16 * i + 8, 1);
        sal_uInt32 len = GetUInt32(sfntP + 12, 16 * i + 12, 1);

        if (tag == T_loca) {
            loca = sfntP + off;
            locaLen = len;
        } else if (tag == T_head) {
            indexToLocFormat = GetInt16(sfntP + off, 50, 1);
        }
    }

    res->nGlyphs = locaLen / ((indexToLocFormat == 1) ? 4 : 2);
    assert(res->nGlyphs != 0);
    res->offs = (sal_uInt32*)scalloc(res->nGlyphs, sizeof(sal_uInt32));

    for (i = 0; i < res->nGlyphs; i++) {
        if (indexToLocFormat == 1) {
            res->offs[i] = GetUInt32(loca, i * 4, 1);
        } else {
            res->offs[i] = GetUInt16(loca, i * 2, 1) << 1;
        }
    }
    return res;
}

static void GlyphOffsetsDispose(GlyphOffsets *_this)
{
    if (_this) {
        free(_this->offs);
        free(_this);
    }
}

static void DumpSfnts(FILE *outf, sal_uInt8 *sfntP)
{
    HexFmt *h = HexFmtNew(outf);
    sal_uInt16 i, numTables = GetUInt16(sfntP, 4, 1);
    GlyphOffsets *go = GlyphOffsetsNew(sfntP);
    sal_uInt8 pad[] = {0,0,0,0};                     /* zeroes                       */

    assert(numTables <= 9);                                 /* Type42 has 9 required tables */

    sal_uInt32* offs = (sal_uInt32*)scalloc(numTables, sizeof(sal_uInt32));
//    sal_uInt32* lens = (sal_uInt32*)scalloc(numTables, sizeof(sal_uInt32));

    fputs("/sfnts [", outf);
    HexFmtOpenString(h);
    HexFmtBlockWrite(h, sfntP, 12);                         /* stream out the Offset Table    */
    HexFmtBlockWrite(h, sfntP+12, 16 * numTables);          /* stream out the Table Directory */

    for (i=0; i<numTables; i++) {
        sal_uInt32 tag = GetUInt32(sfntP + 12, 16 * i, 1);
        sal_uInt32 off = GetUInt32(sfntP + 12, 16 * i + 8, 1);
        sal_uInt32 len = GetUInt32(sfntP + 12, 16 * i + 12, 1);

        if (tag != T_glyf) {
            HexFmtBlockWrite(h, sfntP + off, len);
        } else {
            sal_uInt8 *glyf = sfntP + off;
            sal_uInt32 o, l, j;
            for (j = 0; j < go->nGlyphs - 1; j++) {
                o = go->offs[j];
                l = go->offs[j + 1] - o;
                HexFmtBlockWrite(h, glyf + o, l);
            }
        }
        HexFmtBlockWrite(h, pad, (4 - (len & 3)) & 3);
    }
    HexFmtCloseString(h);
    fputs("] def\n", outf);
    GlyphOffsetsDispose(go);
    HexFmtDispose(h);
    free(offs);
//    free(lens);
}

int  CreateT42FromTTGlyphs(TrueTypeFont  *ttf,
                           FILE          *outf,
                           const char    *psname,
                           sal_uInt16        *glyphArray,
                           sal_uInt8          *encoding,
                           int            nGlyphs)
{
    TrueTypeCreator *ttcr;
    TrueTypeTable *head=0, *hhea=0, *maxp=0, *cvt=0, *prep=0, *glyf=0, *fpgm=0;
    int i;
    int res;

    sal_uInt32 ver, rev;

    sal_uInt8 *sfntP;
    sal_uInt32 sfntLen;
    int UPEm = ttf->unitsPerEm;

    if (nGlyphs >= 256) return SF_GLYPHNUM;

    assert(psname != 0);

    TrueTypeCreatorNewEmpty(T_true, &ttcr);

    /*                        head                          */
    const sal_uInt8* p = getTable(ttf, O_head);
    const sal_uInt8* headP = p;
    assert(p != 0);
    head = TrueTypeTableNew_head(GetUInt32(p, 4, 1), GetUInt16(p, 16, 1), GetUInt16(p, 18, 1), p+20, GetUInt16(p, 44, 1), GetUInt16(p, 46, 1), GetInt16(p, 48, 1));
    ver = GetUInt32(p, 0, 1);
    rev = GetUInt32(p, 4, 1);

    /**                       hhea                         **/
    p = getTable(ttf, O_hhea);
    if (p) {
        hhea = TrueTypeTableNew_hhea(GetUInt16(p, 4, 1), GetUInt16(p, 6, 1), GetUInt16(p, 8, 1), GetUInt16(p, 18, 1), GetUInt16(p, 20, 1));
    } else {
        hhea = TrueTypeTableNew_hhea(0, 0, 0, 0, 0);
    }

    /**                       maxp                         **/
    maxp = TrueTypeTableNew_maxp(getTable(ttf, O_maxp), getTableSize(ttf, O_maxp));

    /**                       cvt                           **/
    if ((p = getTable(ttf, O_cvt)) != 0) {
        cvt = TrueTypeTableNew(T_cvt, getTableSize(ttf, O_cvt), p);
    }

    /**                       prep                          **/
    if ((p = getTable(ttf, O_prep)) != 0) {
        prep = TrueTypeTableNew(T_prep, getTableSize(ttf, O_prep), p);
    }

    /**                       fpgm                          **/
    if ((p = getTable(ttf, O_fpgm)) != 0) {
        fpgm = TrueTypeTableNew(T_fpgm, getTableSize(ttf, O_fpgm), p);
    }

    /**                       glyf                          **/
    glyf = TrueTypeTableNew_glyf();
    sal_uInt16* gID = (sal_uInt16*)scalloc(nGlyphs, sizeof(sal_uInt32));

    for (i = 0; i < nGlyphs; i++) {
        gID[i] = (sal_uInt16)glyfAdd(glyf, GetTTRawGlyphData(ttf, glyphArray[i]), ttf);
    }

    AddTable(ttcr, head); AddTable(ttcr, hhea); AddTable(ttcr, maxp); AddTable(ttcr, cvt);
    AddTable(ttcr, prep); AddTable(ttcr, glyf); AddTable(ttcr, fpgm);

    if ((res = StreamToMemory(ttcr, &sfntP, &sfntLen)) != SF_OK) {
        TrueTypeCreatorDispose(ttcr);
        free(gID);
        return res;
    }

    fprintf(outf, "%%!PS-TrueTypeFont-%d.%d-%d.%d\n", (int)(ver>>16), (int)(ver & 0xFFFF), (int)(rev>>16), (int)(rev & 0xFFFF));
    fprintf(outf, "%%%%Creator: %s %s %s\n", modname, modver, modextra);
    fprintf(outf, "%%- Font subset generated from a source font file: '%s'\n", ttf->fname);
    fprintf(outf, "%%- Original font name: %s\n", ttf->psname);
    fprintf(outf, "%%- Original font family: %s\n", ttf->family);
    fprintf(outf, "%%- Original font sub-family: %s\n", ttf->subfamily);
    fprintf(outf, "11 dict begin\n");
    fprintf(outf, "/FontName (%s) cvn def\n", psname);
    fprintf(outf, "/PaintType 0 def\n");
    fprintf(outf, "/FontMatrix [1 0 0 1 0 0] def\n");
    fprintf(outf, "/FontBBox [%d %d %d %d] def\n", XUnits(UPEm, GetInt16(headP, 36, 1)), XUnits(UPEm, GetInt16(headP, 38, 1)), XUnits(UPEm, GetInt16(headP, 40, 1)), XUnits(UPEm, GetInt16(headP, 42, 1)));
    fprintf(outf, "/FontType 42 def\n");
    fprintf(outf, "/Encoding 256 array def\n");
    fprintf(outf, "    0 1 255 {Encoding exch /.notdef put} for\n");

    for (i = 1; i<nGlyphs; i++) {
        fprintf(outf, "Encoding %d /glyph%d put\n", encoding[i], gID[i]);
    }
    fprintf(outf, "/XUID [103 0 1 16#%08X %d 16#%08X 16#%08X] def\n", (unsigned int)rtl_crc32(0, ttf->ptr, ttf->fsize), (unsigned int)nGlyphs, (unsigned int)rtl_crc32(0, glyphArray, nGlyphs * 2), (unsigned int)rtl_crc32(0, encoding, nGlyphs));

    DumpSfnts(outf, sfntP);

    /* dump charstrings */
    fprintf(outf, "/CharStrings %d dict dup begin\n", nGlyphs);
    fprintf(outf, "/.notdef 0 def\n");
    for (i = 1; i < (int)glyfCount(glyf); i++) {
        fprintf(outf,"/glyph%d %d def\n", i, i);
    }
    fprintf(outf, "end readonly def\n");

    fprintf(outf, "FontName currentdict end definefont pop\n");
    TrueTypeCreatorDispose(ttcr);
    free(gID);
    free(sfntP);
    return SF_OK;
}
#endif


#ifndef NO_MAPPERS
int MapString(TrueTypeFont *ttf, sal_uInt16 *str, int nchars, sal_uInt16 *glyphArray, int bvertical)
{
    int i;
    sal_uInt16 *cp;

    if (ttf->cmapType == CMAP_NOT_USABLE ) return -1;
    if (!nchars) return 0;

    if (glyphArray == 0) {
        cp = str;
    } else {
        cp = glyphArray;
    }

    switch (ttf->cmapType) {
        case CMAP_MS_Symbol:
            if( ttf->mapper == getGlyph0 ) {
                sal_uInt16 aChar;
                for( i = 0; i < nchars; i++ ) {
                    aChar = str[i];
                    if( ( aChar & 0xf000 ) == 0xf000 )
                        aChar &= 0x00ff;
                    cp[i] = aChar;
                }
            }
            else if( glyphArray )
                memcpy(glyphArray, str, nchars * 2);
            break;

        case CMAP_MS_Unicode:
            if (glyphArray != 0) {
                memcpy(glyphArray, str, nchars * 2);
            }
            break;

        case CMAP_MS_ShiftJIS:  TranslateString12(str, cp, nchars); break;
        case CMAP_MS_Big5:      TranslateString13(str, cp, nchars); break;
        case CMAP_MS_PRC:       TranslateString14(str, cp, nchars); break;
        case CMAP_MS_Wansung:   TranslateString15(str, cp, nchars); break;
        case CMAP_MS_Johab:     TranslateString16(str, cp, nchars); break;
    }

    for (i = 0; i < nchars; i++) {
        cp[i] = (sal_uInt16)ttf->mapper(ttf->cmap, cp[i]);
        if (cp[i]!=0 && bvertical!=0)
            cp[i] = (sal_uInt16)UseGSUB(ttf,cp[i],bvertical);
    }
    return nchars;
}

sal_uInt16 MapChar(TrueTypeFont *ttf, sal_uInt16 ch, int bvertical)
{
    switch (ttf->cmapType) {
        case CMAP_MS_Symbol:

            if( ttf->mapper == getGlyph0 && ( ch & 0xf000 ) == 0xf000 )
                ch &= 0x00ff;
            return (sal_uInt16)ttf->mapper(ttf->cmap, ch );

        case CMAP_MS_Unicode:   break;
        case CMAP_MS_ShiftJIS:  ch = TranslateChar12(ch); break;
        case CMAP_MS_Big5:      ch = TranslateChar13(ch); break;
        case CMAP_MS_PRC:       ch = TranslateChar14(ch); break;
        case CMAP_MS_Wansung:   ch = TranslateChar15(ch); break;
        case CMAP_MS_Johab:     ch = TranslateChar16(ch); break;
        default:                return 0;
    }
    ch = (sal_uInt16)ttf->mapper(ttf->cmap, ch);
    if (ch!=0 && bvertical!=0)
        ch = (sal_uInt16)UseGSUB(ttf,ch,bvertical);
    return ch;
}

int DoesVerticalSubstitution( TrueTypeFont *ttf, int bvertical)
{
    int nRet = 0;
    if( bvertical)
        nRet = HasVerticalGSUB( ttf);
    return nRet;
}

#endif

int GetTTGlyphCount( TrueTypeFont* ttf )
{
    return ttf->nglyphs;
}

bool GetSfntTable( TrueTypeFont* ttf, int nSubtableIndex,
    const sal_uInt8** ppRawBytes, int* pRawLength )
{
    if( (nSubtableIndex < 0) || (nSubtableIndex >= NUM_TAGS) )
        return false;
    *pRawLength = ttf->tlens[ nSubtableIndex ];
    *ppRawBytes = ttf->tables[ nSubtableIndex ];
    bool bOk = (*pRawLength > 0) && (ppRawBytes != NULL);
    return bOk;
}

TTSimpleGlyphMetrics *GetTTSimpleGlyphMetrics(TrueTypeFont *ttf, sal_uInt16 *glyphArray, int nGlyphs, int mode)
{
    const sal_uInt8* pTable;
    sal_uInt32 n;
    int nTableSize;

    if (mode == 0) {
        n = ttf->numberOfHMetrics;
        pTable = getTable( ttf, O_hmtx );
        nTableSize = getTableSize( ttf, O_hmtx );
    } else {
        n = ttf->numOfLongVerMetrics;
        pTable = getTable( ttf, O_vmtx );
        nTableSize = getTableSize( ttf, O_vmtx );
    }

    if (!nGlyphs || !glyphArray) return 0;        /* invalid parameters */
    if (!n || !pTable) return 0;                  /* the font does not contain the requested metrics */

    TTSimpleGlyphMetrics* res = (TTSimpleGlyphMetrics*)calloc(nGlyphs, sizeof(TTSimpleGlyphMetrics));
    assert(res != 0);

    const int UPEm = ttf->unitsPerEm;
    for( int i = 0; i < nGlyphs; ++i) {
        int nAdvOffset, nLsbOffset;
        sal_uInt16 glyphID = glyphArray[i];

        if (glyphID < n) {
            nAdvOffset = 4 * glyphID;
            nLsbOffset = nAdvOffset + 2;
        } else {
            nAdvOffset = 4 * (n - 1);
            if( glyphID < ttf->nglyphs )
                nLsbOffset = 4 * n + 2 * (glyphID - n);
            else /* font is broken -> use lsb of last hmetrics */
                nLsbOffset = nAdvOffset + 2;
        }

        if( nAdvOffset >= nTableSize)
            res[i].adv = 0; /* better than a crash for buggy fonts */
        else
            res[i].adv = static_cast<sal_uInt16>(
                XUnits( UPEm, GetUInt16( pTable, nAdvOffset, 1) ) );

        if( nLsbOffset >= nTableSize)
            res[i].sb  = 0; /* better than a crash for buggy fonts */
        else
            res[i].sb  = static_cast<sal_Int16>(
                XUnits( UPEm, GetInt16( pTable, nLsbOffset, 1) ) );
    }

    return res;
}

#ifndef NO_MAPPERS
TTSimpleGlyphMetrics *GetTTSimpleCharMetrics(TrueTypeFont * ttf, sal_uInt16 firstChar, int nChars, int mode)
{
    TTSimpleGlyphMetrics *res = 0;
    int i, n;

    sal_uInt16* str = (sal_uInt16*)malloc(nChars * 2);
    assert(str != 0);

    for (i=0; i<nChars; i++) str[i] = (sal_uInt16)(firstChar + i);
    if ((n = MapString(ttf, str, nChars, 0, mode)) != -1) {
        res = GetTTSimpleGlyphMetrics(ttf, str, n, mode);
    }

    free(str);

    return res;
}
#endif

void GetTTGlobalFontInfo(TrueTypeFont *ttf, TTGlobalFontInfo *info)
{
    int UPEm = ttf->unitsPerEm;

    memset(info, 0, sizeof(TTGlobalFontInfo));

    info->family = ttf->family;
    info->ufamily = ttf->ufamily;
    info->subfamily = ttf->subfamily;
    info->usubfamily = ttf->usubfamily;
    info->psname = ttf->psname;
    info->symbolEncoded = (ttf->cmapType == CMAP_MS_Symbol);

    const sal_uInt8* table = getTable(ttf, O_OS2);
    if (table) {
        info->weight = GetUInt16(table, 4, 1);
        info->width  = GetUInt16(table, 6, 1);

        /* There are 3 different versions of OS/2 table: original (68 bytes long),
         * Microsoft old (78 bytes long) and Microsoft new (86 bytes long,)
         * Apple's documentation recommends looking at the table length.
         */
        if (getTableSize(ttf, O_OS2) > 68) {
            info->typoAscender = XUnits(UPEm,GetInt16(table, 68, 1));
            info->typoDescender = XUnits(UPEm, GetInt16(table, 70, 1));
            info->typoLineGap = XUnits(UPEm, GetInt16(table, 72, 1));
            info->winAscent = XUnits(UPEm, GetUInt16(table, 74, 1));
            info->winDescent = XUnits(UPEm, GetUInt16(table, 76, 1));
            /* sanity check; some fonts treat winDescent as signed
           * violating the standard */
            if( info->winDescent > 5*UPEm )
                info->winDescent = XUnits(UPEm, GetInt16(table, 76,1));
        }
        if (ttf->cmapType == CMAP_MS_Unicode) {
            info->rangeFlag = 1;
            info->ur1 = GetUInt32(table, 42, 1);
            info->ur2 = GetUInt32(table, 46, 1);
            info->ur3 = GetUInt32(table, 50, 1);
            info->ur4 = GetUInt32(table, 54, 1);
        }
        memcpy(info->panose, table + 32, 10);
        info->typeFlags = GetUInt16( table, 8, 1 );
        if( getTable(ttf, O_CFF) )
            info->typeFlags |= TYPEFLAG_PS_OPENTYPE;
    }

    table = getTable(ttf, O_post);
    if (table && getTableSize(ttf, O_post) >= 12+sizeof(sal_uInt32)) {
        info->pitch  = GetUInt32(table, 12, 1);
        info->italicAngle = GetInt32(table, 4, 1);
    }

    table = getTable(ttf, O_head);      /* 'head' tables is always there */
    info->xMin = XUnits(UPEm, GetInt16(table, 36, 1));
    info->yMin = XUnits(UPEm, GetInt16(table, 38, 1));
    info->xMax = XUnits(UPEm, GetInt16(table, 40, 1));
    info->yMax = XUnits(UPEm, GetInt16(table, 42, 1));
    info->macStyle = GetInt16(table, 44, 1);

    table = getTable(ttf, O_hhea);
    if (table) {
        info->ascender  = XUnits(UPEm, GetInt16(table, 4, 1));
        info->descender = XUnits(UPEm, GetInt16(table, 6, 1));
        info->linegap   = XUnits(UPEm, GetInt16(table, 8, 1));
    }

    table = getTable(ttf, O_vhea);
    if (table) {
        info->vascent  = XUnits(UPEm, GetInt16(table, 4, 1));
        info->vdescent = XUnits(UPEm, GetInt16(table, 6, 1));
    }
}

GlyphData *GetTTRawGlyphData(TrueTypeFont *ttf, sal_uInt32 glyphID)
{
    const sal_uInt8* glyf = getTable(ttf, O_glyf);
    const sal_uInt8* hmtx = getTable(ttf, O_hmtx);
    int n;

    if( glyphID >= ttf->nglyphs )
        return 0;

    /* #127161# check the glyph offsets */
    sal_uInt32 length = getTableSize( ttf, O_glyf );
    if( length < ttf->goffsets[ glyphID+1 ] )
        return 0;

    length = ttf->goffsets[glyphID+1] - ttf->goffsets[glyphID];

    GlyphData* d = (GlyphData*)malloc(sizeof(GlyphData)); assert(d != 0);

    if (length > 0) {
        const sal_uInt8* srcptr = glyf + ttf->goffsets[glyphID];
        d->ptr = (sal_uInt8*)malloc((length + 1) & ~1); assert(d->ptr != 0);
        memcpy( d->ptr, srcptr, length );
        d->compflag = (GetInt16( srcptr, 0, 1 ) < 0);
    } else {
        d->ptr = 0;
        d->compflag = 0;
    }

    d->glyphID = glyphID;
    d->nbytes = (sal_uInt16)((length + 1) & ~1);

    /* now calculate npoints and ncontours */
    ControlPoint *cp;
    n = GetTTGlyphPoints(ttf, glyphID, &cp);
    if (n != -1)
    {
        int m = 0;
        for (int i = 0; i < n; i++)
        {
            if (cp[i].flags & 0x8000)
                m++;
        }
        d->npoints = (sal_uInt16)n;
        d->ncontours = (sal_uInt16)m;
        free(cp);
    } else {
        d->npoints = 0;
        d->ncontours = 0;
    }

    /* get advance width and left sidebearing */
    if (glyphID < ttf->numberOfHMetrics) {
        d->aw = GetUInt16(hmtx, 4 * glyphID, 1);
        d->lsb = GetInt16(hmtx, 4 * glyphID + 2, 1);
    } else {
        d->aw = GetUInt16(hmtx, 4 * (ttf->numberOfHMetrics - 1), 1);
        d->lsb  = GetInt16(hmtx + ttf->numberOfHMetrics * 4, (glyphID - ttf->numberOfHMetrics) * 2, 1);
    }

    return d;
}

int GetTTNameRecords(TrueTypeFont *ttf, NameRecord **nr)
{
    const sal_uInt8* table = getTable(ttf, O_name);
    int nTableSize = getTableSize(ttf, O_name );

    if (nTableSize < 6)
    {
#if OSL_DEBUG_LEVEL > 1
        fprintf(stderr, "O_name table too small\n");
#endif
        return 0;
    }

    sal_uInt16 n = GetUInt16(table, 2, 1);
    int nStrBase = GetUInt16(table, 4, 1);
    int i;

    *nr = 0;
    if (n == 0) return 0;

    NameRecord* rec = (NameRecord*)calloc(n, sizeof(NameRecord));

    for (i = 0; i < n; i++) {
        int nStrOffset = GetUInt16(table + 6, 10 + 12 * i, 1);
        rec[i].platformID = GetUInt16(table + 6, 12 * i, 1);
        rec[i].encodingID = GetUInt16(table + 6, 2 + 12 * i, 1);
        rec[i].languageID = GetUInt16(table + 6, 4 + 12 * i, 1);
        rec[i].nameID = GetUInt16(table + 6, 6 + 12 * i, 1);
        rec[i].slen = GetUInt16(table + 6, 8 + 12 * i, 1);
        if (rec[i].slen) {
            if( nStrBase+nStrOffset+rec[i].slen >= nTableSize ) {
                rec[i].sptr = 0;
                rec[i].slen = 0;
                continue;
            }

            const  sal_uInt8* rec_string = table + nStrBase + nStrOffset;
            // sanity check
            if( rec_string > (sal_uInt8*)ttf->ptr && rec_string < ((sal_uInt8*)ttf->ptr + ttf->fsize - rec[i].slen ) )
            {
                rec[i].sptr = (sal_uInt8 *) malloc(rec[i].slen); assert(rec[i].sptr != 0);
                memcpy(rec[i].sptr, rec_string, rec[i].slen);
            }
            else
            {
                rec[i].sptr = 0;
                rec[i].slen = 0;
            }
        } else {
            rec[i].sptr = 0;
        }
        // some fonts have 3.0 names => fix them to 3.1
        if( (rec[i].platformID == 3) && (rec[i].encodingID == 0) )
            rec[i].encodingID = 1;
    }

    *nr = rec;
    return n;
}

void DisposeNameRecords(NameRecord* nr, int n)
{
    int i;
    for (i = 0; i < n; i++) {
        if (nr[i].sptr) free(nr[i].sptr);
    }
    free(nr);
}

bool getTTCoverage(
    boost::dynamic_bitset<sal_uInt32> &rUnicodeRange,
    boost::dynamic_bitset<sal_uInt32> &rCodePageRange,
    const unsigned char* pTable, size_t nLength)
{
    bool bRet = false;
    sal_uInt16 nVersion = GetUInt16(pTable, 0, 1);
    // parse OS/2 header
    if ( nVersion >= 0x0001 && nLength >= 58 )
    {
        rUnicodeRange.append(GetUInt32(pTable, 42, 1));
        rUnicodeRange.append(GetUInt32(pTable, 46, 1));
        rUnicodeRange.append(GetUInt32(pTable, 50, 1));
        rUnicodeRange.append(GetUInt32(pTable, 54, 1));
        bRet = true;
        if (nLength >= 86)
        {
            rCodePageRange.append(GetUInt32(pTable, 78, 1));
            rCodePageRange.append(GetUInt32(pTable, 82, 1));
        }
    }
    return bRet;
}

void getTTScripts(std::vector< sal_uInt32 > &rScriptTags, const unsigned char* pTable, size_t nLength)
{
    if (nLength < 6)
        return;

    // parse GSUB/GPOS header
    const sal_uInt16 nOfsScriptList = GetUInt16(pTable, 4, 1);

    // parse Script Table
    const sal_uInt16 nCntScript = GetUInt16(pTable, nOfsScriptList, 1);
    sal_uInt32 nCurrentPos = nOfsScriptList+2;
    for( sal_uInt16 nScriptIndex = 0;
         nScriptIndex < nCntScript && nLength >= 6; ++nScriptIndex,
         nLength-=6 )
    {
        sal_uInt32 nTag = GetUInt32(pTable, nCurrentPos, 1);
        nCurrentPos+=6;
        rScriptTags.push_back(nTag); // e.g. hani/arab/kana/hang
    }

    std::sort(rScriptTags.begin(), rScriptTags.end());
    rScriptTags.erase(std::unique(rScriptTags.begin(), rScriptTags.end()), rScriptTags.end());
}

} // namespace vcl

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
