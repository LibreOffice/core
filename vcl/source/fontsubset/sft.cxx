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
#include <unistd.h>
#endif
#include <sft.hxx>
#include <impfontcharmap.hxx>
#include "ttcr.hxx"
#include "xlat.hxx"
#include <rtl/crc.h>
#include <rtl/ustring.hxx>
#include <sal/log.hxx>
#include <o3tl/safeint.hxx>
#include <osl/endian.h>
#include <algorithm>

namespace vcl
{

/*- module identification */

const char * const modname  = "SunTypeTools-TT";
const char * const modver   = "1.0";
const char * const modextra = "gelf";

/*- private functions, constants and data types */

namespace {

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

    explicit PSPathElement( PathSegmentType i_eType ) : type( i_eType ),
                                   x1( 0 ), y1( 0 ),
                                   x2( 0 ), y2( 0 ),
                                   x3( 0 ), y3( 0 )
    {
    }
};

/*- In horizontal writing mode right sidebearing is calculated using this formula
 *- rsb = aw - (lsb + xMax - xMin) -*/
struct TTGlyphMetrics {
    sal_Int16  xMin;
    sal_Int16  yMin;
    sal_Int16  xMax;
    sal_Int16  yMax;
    sal_uInt16 aw;                /*- Advance Width (horizontal writing mode)    */
    sal_Int16  lsb;               /*- Left sidebearing (horizontal writing mode) */
    sal_uInt16 ah;                /*- advance height (vertical writing mode)     */
};

#define HFORMAT_LINELEN 64

struct HexFmt {
    FILE *o;
    char buffer[HFORMAT_LINELEN];
    size_t bufpos;
    int total;
};

struct GlyphOffsets {
    sal_uInt32 nGlyphs;           /* number of glyphs in the font + 1 */
    sal_uInt32 *offs;             /* array of nGlyphs offsets */
};

}

static void *smalloc(size_t size)
{
    void *res = malloc(size);
    assert(res != nullptr);
    return res;
}

static void *scalloc(size_t n, size_t size)
{
    void *res = calloc(n, size);
    assert(res != nullptr);
    return res;
}

/*- Data access methods for data stored in big-endian format */
static sal_Int16 GetInt16(const sal_uInt8 *ptr, size_t offset)
{
    sal_Int16 t;
    assert(ptr != nullptr);

    t = (ptr+offset)[0] << 8 | (ptr+offset)[1];

    return t;
}

static sal_uInt16 GetUInt16(const sal_uInt8 *ptr, size_t offset)
{
    sal_uInt16 t;
    assert(ptr != nullptr);

    t = (ptr+offset)[0] << 8 | (ptr+offset)[1];

    return t;
}

static sal_Int32  GetInt32(const sal_uInt8 *ptr, size_t offset)
{
    sal_Int32 t;
    assert(ptr != nullptr);

    t = (ptr+offset)[0] << 24 | (ptr+offset)[1] << 16 |
        (ptr+offset)[2] << 8  | (ptr+offset)[3];

    return t;
}

static sal_uInt32 GetUInt32(const sal_uInt8 *ptr, size_t offset)
{
    sal_uInt32 t;
    assert(ptr != nullptr);

    t = (ptr+offset)[0] << 24 | (ptr+offset)[1] << 16 |
        (ptr+offset)[2] << 8  | (ptr+offset)[3];

    return t;
}

static F16Dot16 fixedMul(F16Dot16 a, F16Dot16 b)
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

static F16Dot16 fixedDiv(F16Dot16 a, F16Dot16 b)
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
static F16Dot16 fixedMulDiv(F16Dot16 a, F16Dot16 b, F16Dot16 c)
{
    F16Dot16 res;

    res = fixedMul(a, b);
    return fixedDiv(res, c);
}

/*- Translate units from TT to PS (standard 1/1000) -*/
static int XUnits(int unitsPerEm, int n)
{
    return (n * 1000) / unitsPerEm;
}

static char toHex(sal_uInt8 nIndex)
{
    /* Hex Formatter functions */
    static const char HexChars[] = "0123456789ABCDEF";
    assert(nIndex < SAL_N_ELEMENTS(HexChars));
    return HexChars[nIndex];
}

static HexFmt *HexFmtNew(FILE *outf)
{
    HexFmt* res = static_cast<HexFmt*>(smalloc(sizeof(HexFmt)));
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

static void HexFmtOpenString(HexFmt *_this)
{
    fputs("<\n", _this->o);
}

static void HexFmtCloseString(HexFmt *_this)
{
    HexFmtFlush(_this);
    fputs("00\n>\n", _this->o);
}

static void HexFmtDispose(HexFmt *_this)
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
        Ch = static_cast<sal_uInt8 const *>(ptr)[i];
        _this->buffer[_this->bufpos++] = toHex(Ch >> 4);
        _this->buffer[_this->bufpos++] = toHex(Ch & 0xF);
        if (_this->bufpos == HFORMAT_LINELEN) {
            HexFmtFlush(_this);
            fputc('\n', _this->o);
        }

    }
    _this->total += size;
}

/* Outline Extraction functions */

/* fills the aw and lsb entries of the TTGlyphMetrics structure from hmtx table -*/
static void GetMetrics(AbstractTrueTypeFont const *ttf, sal_uInt32 glyphID, TTGlyphMetrics *metrics)
{
    sal_uInt32 nSize;
    const sal_uInt8* table = ttf->table(O_hmtx, nSize);

    metrics->aw = metrics->lsb = metrics->ah = 0;
    if (!table || !ttf->horzMetricCount())
        return;

    if (glyphID < ttf->horzMetricCount())
    {
        metrics->aw = GetUInt16(table, 4 * glyphID);
        metrics->lsb = GetInt16(table, 4 * glyphID + 2);
    }
    else
    {
        metrics->aw = GetUInt16(table, 4 * (ttf->horzMetricCount() - 1));
        metrics->lsb = GetInt16(table + ttf->horzMetricCount() * 4, (glyphID - ttf->horzMetricCount()) * 2);
    }

    table = ttf->table(O_vmtx, nSize);
    if (!table || !ttf->vertMetricCount())
        return;

    if (glyphID < ttf->vertMetricCount())
        metrics->ah = GetUInt16(table, 4 * glyphID);
    else
        metrics->ah = GetUInt16(table, 4 * (ttf->vertMetricCount() - 1));
}

static int GetTTGlyphOutline(AbstractTrueTypeFont *, sal_uInt32 , ControlPoint **, TTGlyphMetrics *, std::vector< sal_uInt32 >* );

/* returns the number of control points, allocates the pointArray */
static int GetSimpleTTOutline(AbstractTrueTypeFont const *ttf, sal_uInt32 glyphID, ControlPoint **pointArray, TTGlyphMetrics *metrics)
{
    sal_uInt32 nTableSize;
    const sal_uInt8* table = ttf->table(O_glyf, nTableSize);
    sal_uInt8 flag, n;
    int i, j, z;

    *pointArray = nullptr;

    if (glyphID >= ttf->glyphCount())
        return 0;

    const sal_uInt8* ptr = table + ttf->glyphOffset(glyphID);
    const sal_Int16 numberOfContours = GetInt16(ptr, GLYF_numberOfContours_offset);
    if( numberOfContours <= 0 )             /*- glyph is not simple */
        return 0;

    if (metrics) {                                                    /*- GetCompoundTTOutline() calls this function with NULL metrics -*/
        metrics->xMin = GetInt16(ptr, GLYF_xMin_offset);
        metrics->yMin = GetInt16(ptr, GLYF_yMin_offset);
        metrics->xMax = GetInt16(ptr, GLYF_xMax_offset);
        metrics->yMax = GetInt16(ptr, GLYF_yMax_offset);
        GetMetrics(ttf, glyphID, metrics);
    }

    /* determine the last point and be extra safe about it. But probably this code is not needed */
    sal_uInt16 lastPoint=0;
    const sal_Int32 nMaxContours = (nTableSize - 10)/2;
    if (numberOfContours > nMaxContours)
        return 0;
    for (i=0; i<numberOfContours; i++)
    {
        const sal_uInt16 t = GetUInt16(ptr, 10+i*2);
        if (t > lastPoint)
            lastPoint = t;
    }

    sal_uInt16 instLen = GetUInt16(ptr, 10 + numberOfContours*2);
    sal_uInt32 nOffset = 10 + 2 * numberOfContours + 2 + instLen;
    if (nOffset > nTableSize)
        return 0;
    const sal_uInt8* p = ptr + nOffset;

    const sal_uInt32 nBytesRemaining = nTableSize - nOffset;
    const sal_uInt16 palen = lastPoint+1;

    //at a minimum its one byte per entry
    if (palen > nBytesRemaining || lastPoint > nBytesRemaining-1)
    {
        SAL_WARN("vcl.fonts", "Font " << OUString::createFromAscii(ttf->fileName()) <<
            "claimed a palen of "
            << palen << " but max bytes remaining is " << nBytesRemaining);
        return 0;
    }

    ControlPoint* pa = static_cast<ControlPoint*>(calloc(palen, sizeof(ControlPoint)));

    i = 0;
    while (i <= lastPoint) {
        flag = *p++;
        pa[i++].flags = static_cast<sal_uInt32>(flag);
        if (flag & 8) {                                     /*- repeat flag */
            n = *p++;
            // coverity[tainted_data : FALSE] - i > lastPoint extra checks the n loop bound
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
                z += static_cast<int>(*p++);
            } else {
                z -= static_cast<int>(*p++);
            }
        } else if ( !(pa[i].flags & 0x10)) {
            z += GetInt16(p, 0);
            p += 2;
        }
        pa[i].x = static_cast<sal_Int16>(z);
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
            z += GetInt16(p, 0);
            p += 2;
        }
        pa[i].y = static_cast<sal_Int16>(z);
    }

    for (i=0; i<numberOfContours; i++) {
        sal_uInt16 offset = GetUInt16(ptr, 10 + i * 2);
        SAL_WARN_IF(offset >= palen, "vcl.fonts", "Font " << OUString::createFromAscii(ttf->fileName()) <<
            " contour " << i << " claimed an illegal offset of "
            << offset << " but max offset is " << palen-1);
        if (offset >= palen)
            continue;
        pa[offset].flags |= 0x00008000;      /*- set the end contour flag */
    }

    *pointArray = pa;
    return lastPoint + 1;
}

static F16Dot16 fromF2Dot14(sal_Int16 n)
{
    // Avoid undefined shift of negative values prior to C++2a:
    return sal_uInt32(n) << 2;
}

static int GetCompoundTTOutline(AbstractTrueTypeFont *ttf, sal_uInt32 glyphID, ControlPoint **pointArray, TTGlyphMetrics *metrics, std::vector< sal_uInt32 >& glyphlist)
{
    sal_uInt16 flags, index;
    sal_Int16 e, f;
    sal_uInt32 nSize;
    const sal_uInt8* table = ttf->table(O_glyf, nSize);
    std::vector<ControlPoint> myPoints;
    ControlPoint *nextComponent, *pa;
    int i, np;
    F16Dot16 a = 0x10000, b = 0, c = 0, d = 0x10000, m, n, abs1, abs2, abs3;

    *pointArray = nullptr;

    if (glyphID >= ttf->glyphCount())
        return 0;

    const sal_uInt8* ptr = table + ttf->glyphOffset(glyphID);
    if (GetInt16(ptr, GLYF_numberOfContours_offset) != -1)   /* number of contours - glyph is not compound */
        return 0;

    if (metrics) {
        metrics->xMin = GetInt16(ptr, GLYF_xMin_offset);
        metrics->yMin = GetInt16(ptr, GLYF_yMin_offset);
        metrics->xMax = GetInt16(ptr, GLYF_xMax_offset);
        metrics->yMax = GetInt16(ptr, GLYF_yMax_offset);
        GetMetrics(ttf, glyphID, metrics);
    }

    ptr += 10;

    do {
        flags = GetUInt16(ptr, 0);
        /* printf("flags: 0x%X\n", flags); */
        index = GetUInt16(ptr, 2);
        ptr += 4;

        if( std::find( glyphlist.begin(), glyphlist.end(), index ) != glyphlist.end() )
        {
#if OSL_DEBUG_LEVEL > 1
            SAL_INFO("vcl.fonts", "Endless loop found in a compound glyph.");

            std::ostringstream oss;
            oss << index << " -> [";
            for( const auto& rGlyph : glyphlist )
            {
                oss << (int) rGlyph << " ";
            }
            oss << "]";
            SAL_INFO("vcl.fonts", oss.str());
        /**/
#endif
        }

        glyphlist.push_back( index );

        if ((np = GetTTGlyphOutline(ttf, index, &nextComponent, nullptr, &glyphlist)) == 0)
        {
            /* XXX that probably indicates a corrupted font */
#if OSL_DEBUG_LEVEL > 1
            SAL_WARN("vcl.fonts", "An empty compound!");
            /* assert(!"An empty compound"); */
#endif
        }

        if( ! glyphlist.empty() )
            glyphlist.pop_back();

        if ((flags & USE_MY_METRICS) && metrics)
            GetMetrics(ttf, index, metrics);

        if (flags & ARG_1_AND_2_ARE_WORDS) {
            e = GetInt16(ptr, 0);
            f = GetInt16(ptr, 2);
            /* printf("ARG_1_AND_2_ARE_WORDS: %d %d\n", e & 0xFFFF, f & 0xFFFF); */
            ptr += 4;
        } else {
            if (flags & ARGS_ARE_XY_VALUES) {     /* args are signed */
                e = static_cast<sal_Int8>(*ptr++);
                f = static_cast<sal_Int8>(*ptr++);
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
            a = fromF2Dot14(GetInt16(ptr, 0));
            d = a;
            ptr += 2;
        } else if (flags & WE_HAVE_AN_X_AND_Y_SCALE) {
            a = fromF2Dot14(GetInt16(ptr, 0));
            d = fromF2Dot14(GetInt16(ptr, 2));
            ptr += 4;
        } else if (flags & WE_HAVE_A_TWO_BY_TWO) {
            a = fromF2Dot14(GetInt16(ptr, 0));
            b = fromF2Dot14(GetInt16(ptr, 2));
            c = fromF2Dot14(GetInt16(ptr, 4));
            d = fromF2Dot14(GetInt16(ptr, 6));
            ptr += 8;
        }

        abs1 = (a < 0) ? -a : a;
        abs2 = (b < 0) ? -b : b;
        m    = std::max(abs1, abs2);
        abs3 = abs1 - abs2;
        if (abs3 < 0) abs3 = -abs3;
        if (abs3 <= 33) m *= 2;

        abs1 = (c < 0) ? -c : c;
        abs2 = (d < 0) ? -d : d;
        n    = std::max(abs1, abs2);
        abs3 = abs1 - abs2;
        if (abs3 < 0) abs3 = -abs3;
        if (abs3 <= 33) n *= 2;

        SAL_WARN_IF(np && !m, "vcl.fonts", "Parsing error in " << OUString::createFromAscii(ttf->fileName()) <<
                     ": divide by zero");

        if (m != 0) {
            for (i=0; i<np; i++) {
                F16Dot16 t;
                ControlPoint cp;
                cp.flags = nextComponent[i].flags;
                const sal_uInt16 x = nextComponent[i].x;
                const sal_uInt16 y = nextComponent[i].y;
                t = fixedMulDiv(a, x << 16, m) + fixedMulDiv(c, y << 16, m) + sal_Int32(sal_uInt16(e) << 16);
                cp.x = static_cast<sal_Int16>(fixedMul(t, m) >> 16);
                t = fixedMulDiv(b, x << 16, n) + fixedMulDiv(d, y << 16, n) + sal_Int32(sal_uInt16(f) << 16);
                cp.y = static_cast<sal_Int16>(fixedMul(t, n) >> 16);

                myPoints.push_back( cp );
            }
        }

        free(nextComponent);

    } while (flags & MORE_COMPONENTS);

    // #i123417# some fonts like IFAOGrec have no outline points in some compound glyphs
    // so this unlikely but possible scenario should be handled gracefully
    if( myPoints.empty() )
        return 0;

    np = myPoints.size();
    if (np > 0)
    {
        pa = static_cast<ControlPoint*>(calloc(np, sizeof(ControlPoint)));
        assert(pa != nullptr);

        memcpy(pa, myPoints.data(), np * sizeof(ControlPoint));

        *pointArray = pa;
    }
    return np;
}

/* NOTE: GetTTGlyphOutline() returns -1 if the glyphID is incorrect,
 * but Get{Simple|Compound}GlyphOutline returns 0 in such a case.
 *
 * NOTE: glyphlist is the stack of glyphs traversed while constructing
 * a composite glyph. This is a safeguard against endless recursion
 * in corrupted fonts.
 */
static int GetTTGlyphOutline(AbstractTrueTypeFont *ttf, sal_uInt32 glyphID, ControlPoint **pointArray, TTGlyphMetrics *metrics, std::vector< sal_uInt32 >* glyphlist)
{
    sal_uInt32 nSize;
    const sal_uInt8 *table = ttf->table(O_glyf, nSize);
    sal_Int16 numberOfContours;
    int res;
    *pointArray = nullptr;

    if (metrics)
        memset(metrics, 0, sizeof(TTGlyphMetrics));

    if (glyphID >= ttf->glyphCount())
        return -1;

    const sal_uInt8* ptr = table + ttf->glyphOffset(glyphID);
    int length = ttf->glyphOffset(glyphID + 1) - ttf->glyphOffset(glyphID);

    if (length == 0) {                                      /*- empty glyphs still have hmtx and vmtx metrics values */
        if (metrics) GetMetrics(ttf, glyphID, metrics);
        return 0;
    }

    numberOfContours = GetInt16(ptr, 0);

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

/*- returns the number of items in the path -*/

static int BSplineToPSPath(ControlPoint const *srcA, int srcCount, PSPathElement **path)
{
    std::vector< PSPathElement > aPathList;
    int nPathCount = 0;
    PSPathElement p( PS_NOOP );

    int x0 = 0, y0 = 0, x1 = 0, y1 = 0, x2, y2, curx, cury;
    bool lastOff = false;                                   /*- last point was off-contour */
    int scflag = 1;                                         /*- start contour flag */
    bool ecflag = false;                                    /*- end contour flag */
    int cp = 0;                                             /*- current point */
    int StartContour = 0, EndContour = 1;

    *path = nullptr;

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
            lastOff = false;
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
                if (x0 != curx || y0 != cury)
                {                              /* eliminate empty lines */
                    p = PSPathElement(PS_LINETO);
                    p.x1 = curx;
                    p.y1 = cury;
                    aPathList.push_back( p );
                }
            }
            x0 = curx; y0 = cury; lastOff = false;
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
            aPathList.emplace_back(PS_CLOSEPATH );
            scflag = 1;
            ecflag = false;
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

    if( (nPathCount = static_cast<int>(aPathList.size())) > 0)
    {
        *path = static_cast<PSPathElement*>(calloc(nPathCount, sizeof(PSPathElement)));
        assert(*path != nullptr);
        memcpy( *path, aPathList.data(), nPathCount * sizeof(PSPathElement) );
    }

    return nPathCount;
}

/*- Extracts a string from the name table and allocates memory for it -*/

static char *nameExtract( const sal_uInt8* name, int nTableSize, int n, int dbFlag, sal_Unicode** ucs2result )
{
    char *res;
    const sal_uInt8* ptr = name + GetUInt16(name, 4) + GetUInt16(name + 6, 12 * n + 10);
    int len = GetUInt16(name+6, 12 * n + 8);

    // sanity check
    const sal_uInt8* end_table = name+nTableSize;
    const int available_space = ptr > end_table ? 0 : (end_table - ptr);
    if( (len <= 0) || len > available_space)
    {
        if( ucs2result )
            *ucs2result = nullptr;
        return nullptr;
    }

    if( ucs2result )
        *ucs2result = nullptr;
    if (dbFlag) {
        res = static_cast<char*>(malloc(1 + len/2));
        assert(res != nullptr);
        for (int i = 0; i < len/2; i++)
            res[i] = *(ptr + i * 2 + 1);
        res[len/2] = 0;
        if( ucs2result )
        {
            *ucs2result = static_cast<sal_Unicode*>(malloc( len+2 ));
            for (int i = 0; i < len/2; i++ )
                (*ucs2result)[i] = GetUInt16( ptr, 2*i );
            (*ucs2result)[len/2] = 0;
        }
    } else {
        res = static_cast<char*>(malloc(1 + len));
        assert(res != nullptr);
        memcpy(res, ptr, len);
        res[len] = 0;
    }

    return res;
}

static int findname( const sal_uInt8 *name, sal_uInt16 n, sal_uInt16 platformID,
    sal_uInt16 encodingID, sal_uInt16 languageID, sal_uInt16 nameID )
{
    if (n == 0) return -1;

    int l = 0, r = n-1;
    sal_uInt32 t1, t2;
    sal_uInt32 m1, m2;

    m1 = (platformID << 16) | encodingID;
    m2 = (languageID << 16) | nameID;

    do {
        const int i = (l + r) >> 1;
        t1 = GetUInt32(name + 6, i * 12 + 0);
        t2 = GetUInt32(name + 6, i * 12 + 4);

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
 * /d/fonts/fdltest/Korean/h2drrm has unsorted names and an unknown (to me) Mac LanguageID,
 * but (1, 0, 1042) strings usable
 * Fix: change algorithm, and use (1, 0, *) if both standard Mac and MS strings are not found
 */

static void GetNames(TrueTypeFont *t)
{
    sal_uInt32 nTableSize;
    const sal_uInt8* table = t->table(O_name, nTableSize);

    if (nTableSize < 6)
    {
#if OSL_DEBUG_LEVEL > 1
        SAL_WARN("vcl.fonts", "O_name table too small.");
#endif
        return;
    }

    sal_uInt16 n = GetUInt16(table, 2);

    /* simple sanity check for name table entry count */
    const size_t nMinRecordSize = 12;
    const size_t nSpaceAvailable = nTableSize - 6;
    const size_t nMaxRecords = nSpaceAvailable/nMinRecordSize;
    if (n >= nMaxRecords)
        n = 0;

    int i, r;
    bool bPSNameOK = true;

    /* PostScript name: preferred Microsoft */
    t->psname = nullptr;
    if ((r = findname(table, n, 3, 1, 0x0409, 6)) != -1)
        t->psname = nameExtract(table, nTableSize, r, 1, nullptr);
    if ( ! t->psname && (r = findname(table, n, 1, 0, 0, 6)) != -1)
        t->psname = nameExtract(table, nTableSize, r, 0, nullptr);
    if ( ! t->psname && (r = findname(table, n, 3, 0, 0x0409, 6)) != -1)
    {
        // some symbol fonts like Marlett have a 3,0 name!
        t->psname = nameExtract(table, nTableSize, r, 1, nullptr);
    }
    // for embedded font in Ghostscript PDFs
    if ( ! t->psname && (r = findname(table, n, 2, 2, 0, 6)) != -1)
    {
        t->psname = nameExtract(table, nTableSize, r, 0, nullptr);
    }
    if ( ! t->psname )
    {
        if (t->fileName())
        {
            const char* pReverse = t->fileName() + strlen(t->fileName());
            /* take only last token of filename */
            while (pReverse != t->fileName() && *pReverse != '/') pReverse--;
            if(*pReverse == '/') pReverse++;
            t->psname = strdup(pReverse);
            assert(t->psname != nullptr);
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
    t->family = nullptr;
    if ((r = findname(table, n, 0, 0, 0, 1)) != -1)
        t->family = nameExtract(table, nTableSize, r, 1, &t->ufamily);
    if ( ! t->family && (r = findname(table, n, 3, 1, 0x0409, 1)) != -1)
        t->family = nameExtract(table, nTableSize, r, 1, &t->ufamily);
    if ( ! t->family && (r = findname(table, n, 1, 0, 0, 1)) != -1)
        t->family = nameExtract(table, nTableSize, r, 0, nullptr);
    if ( ! t->family && (r = findname(table, n, 3, 1, 0x0411, 1)) != -1)
        t->family = nameExtract(table, nTableSize, r, 1, &t->ufamily);
    if ( ! t->family && (r = findname(table, n, 3, 0, 0x0409, 1)) != -1)
        t->family = nameExtract(table, nTableSize, r, 1, &t->ufamily);
    if ( ! t->family )
    {
        t->family = strdup(t->psname);
        assert(t->family != nullptr);
    }

    t->subfamily = nullptr;
    t->usubfamily = nullptr;
    if ((r = findname(table, n, 1, 0, 0, 2)) != -1)
        t->subfamily = nameExtract(table, nTableSize, r, 0, &t->usubfamily);
    if ( ! t->subfamily && (r = findname(table, n, 3, 1, 0x0409, 2)) != -1)
        t->subfamily = nameExtract(table, nTableSize, r, 1, &t->usubfamily);
    if ( ! t->subfamily )
    {
        t->subfamily = strdup("");
    }

    /* #i60349# sanity check psname
     * psname practically has to be 7bit ASCII and should not contain spaces
     * there is a class of broken fonts which do not fulfill that at all, so let's try
     * if the family name is 7bit ASCII and take it instead if so
     */
    /* check psname */
    for( i = 0; t->psname[i] != 0 && bPSNameOK; i++ )
        if( t->psname[ i ] < 33 || (t->psname[ i ] & 0x80) )
            bPSNameOK = false;
    if( bPSNameOK )
        return;

    /* check if family is a suitable replacement */
    if( !(t->ufamily && t->family) )
        return;

    bool bReplace = true;

    for( i = 0; t->ufamily[ i ] != 0 && bReplace; i++ )
        if( t->ufamily[ i ] < 33 || t->ufamily[ i ] > 127 )
            bReplace = false;
    if( bReplace )
    {
        free( t->psname );
        t->psname = strdup( t->family );
    }
}

/*- Public functions */

int CountTTCFonts(const char* fname)
{
    int nFonts = 0;
    sal_uInt8 buffer[12];
    FILE* fd = fopen(fname, "rb");
    if( fd ) {
        if (fread(buffer, 1, 12, fd) == 12) {
            if(GetUInt32(buffer, 0) == T_ttcf )
                nFonts = GetUInt32(buffer, 8);
        }
        fclose(fd);
    }
    return nFonts;
}

#if !defined(_WIN32)
SFErrCodes OpenTTFontFile(const char* fname, sal_uInt32 facenum, TrueTypeFont** ttf,
                          const FontCharMapRef xCharMap)
{
    SFErrCodes ret;
    int fd = -1;
    struct stat st;

    if (!fname || !*fname) return SFErrCodes::BadFile;

    *ttf = new TrueTypeFont(fname, xCharMap);
    if( ! *ttf )
        return SFErrCodes::Memory;

    if( ! (*ttf)->fileName() )
    {
        ret = SFErrCodes::Memory;
        goto cleanup;
    }

    fd = open(fname, O_RDONLY);

    if (fd == -1) {
        ret = SFErrCodes::BadFile;
        goto cleanup;
    }

    if (fstat(fd, &st) == -1) {
        ret = SFErrCodes::FileIo;
        goto cleanup;
    }

    (*ttf)->fsize = st.st_size;

    /* On Mac OS, most likely will happen if a Mac user renames a font file
     * to be .ttf when it's really a Mac resource-based font.
     * Size will be 0, but fonts smaller than 4 bytes would be broken anyway.
     */
    if ((*ttf)->fsize == 0) {
        ret = SFErrCodes::BadFile;
        goto cleanup;
    }

    if (((*ttf)->ptr = static_cast<sal_uInt8 *>(mmap(nullptr, (*ttf)->fsize, PROT_READ, MAP_SHARED, fd, 0))) == MAP_FAILED) {
        ret = SFErrCodes::Memory;
        goto cleanup;
    }

    ret = (*ttf)->open(facenum);

cleanup:
    if (fd != -1) close(fd);
    if (ret != SFErrCodes::Ok)
    {
        delete *ttf;
        *ttf = nullptr;
    }
    return ret;
}
#endif

SFErrCodes OpenTTFontBuffer(const void* pBuffer, sal_uInt32 nLen, sal_uInt32 facenum, TrueTypeFont** ttf,
                            const FontCharMapRef xCharMap)
{
    *ttf = new TrueTypeFont(nullptr, xCharMap);
    if( *ttf == nullptr )
        return SFErrCodes::Memory;

    (*ttf)->fsize = nLen;
    (*ttf)->ptr   = const_cast<sal_uInt8 *>(static_cast<sal_uInt8 const *>(pBuffer));

    SFErrCodes ret = (*ttf)->open(facenum);
    if (ret != SFErrCodes::Ok)
    {
        delete *ttf;
        *ttf = nullptr;
    }
    return ret;
}

namespace {

bool withinBounds(sal_uInt32 tdoffset, sal_uInt32 moreoffset, sal_uInt32 len, sal_uInt32 available)
{
    sal_uInt32 result;
    if (o3tl::checked_add(tdoffset, moreoffset, result))
        return false;
    if (o3tl::checked_add(result, len, result))
        return false;
    return result <= available;
}
}

AbstractTrueTypeFont::AbstractTrueTypeFont(const char* pFileName, const FontCharMapRef xCharMap)
    : m_pFileName(nullptr)
    , m_nGlyphs(0xFFFFFFFF)
    , m_pGlyphOffsets(nullptr)
    , m_nHorzMetrics(0)
    , m_nVertMetrics(0)
    , m_nUnitsPerEm(0)
    , m_xCharMap(xCharMap)
{
    if (pFileName)
        m_pFileName = strdup(pFileName);
}

AbstractTrueTypeFont::~AbstractTrueTypeFont()
{
    free(m_pFileName);
    free(m_pGlyphOffsets);
}

TrueTypeFont::TrueTypeFont(const char* pFileName, const FontCharMapRef xCharMap)
    : AbstractTrueTypeFont(pFileName, xCharMap)
    , fsize(-1)
    , ptr(nullptr)
    , psname(nullptr)
    , family(nullptr)
    , ufamily(nullptr)
    , subfamily(nullptr)
    , usubfamily(nullptr)
    , ntables(0)
{
}

TrueTypeFont::~TrueTypeFont()
{
#if !defined(_WIN32)
    if (fileName())
        munmap(ptr, fsize);
#endif
    free(psname);
    free(family);
    free(ufamily);
    free(subfamily);
    free(usubfamily);
}

void CloseTTFont(TrueTypeFont* ttf) { delete ttf; }

SFErrCodes AbstractTrueTypeFont::indexGlyphData()
{
    if (!(hasTable(O_maxp) && hasTable(O_head) && hasTable(O_name) && hasTable(O_cmap)))
        return SFErrCodes::TtFormat;

    sal_uInt32 table_size;
    const sal_uInt8* table = this->table(O_maxp, table_size);
    m_nGlyphs = table_size >= 6 ? GetUInt16(table, 4) : 0;

    table = this->table(O_head, table_size);
    if (table_size < HEAD_Length)
        return SFErrCodes::TtFormat;

    m_nUnitsPerEm = GetUInt16(table, HEAD_unitsPerEm_offset);
    int indexfmt = GetInt16(table, HEAD_indexToLocFormat_offset);

    if (((indexfmt != 0) && (indexfmt != 1)) || (m_nUnitsPerEm <= 0))
        return SFErrCodes::TtFormat;

    if (hasTable(O_glyf) && (table = this->table(O_loca, table_size))) /* TTF or TTF-OpenType */
    {
        int k = (table_size / (indexfmt ? 4 : 2)) - 1;
        if (k < static_cast<int>(m_nGlyphs))       /* Hack for broken Chinese fonts */
            m_nGlyphs = k;

        free(m_pGlyphOffsets);
        m_pGlyphOffsets = static_cast<sal_uInt32 *>(calloc(m_nGlyphs + 1, sizeof(sal_uInt32)));
        assert(m_pGlyphOffsets != nullptr);

        for (int i = 0; i <= static_cast<int>(m_nGlyphs); ++i)
            m_pGlyphOffsets[i] = indexfmt ? GetUInt32(table, i << 2) : static_cast<sal_uInt32>(GetUInt16(table, i << 1)) << 1;
    }
    else if (this->table(O_CFF, table_size)) /* PS-OpenType */
    {
        int k = (table_size / 2) - 1; /* set a limit here, presumably much lower than the table size, but establishes some sort of physical bound */
        if (k < static_cast<int>(m_nGlyphs))
            m_nGlyphs = k;

        free(m_pGlyphOffsets);
        m_pGlyphOffsets = static_cast<sal_uInt32 *>(calloc(m_nGlyphs + 1, sizeof(sal_uInt32)));
        /* TODO: implement to get subsetting */
        assert(m_pGlyphOffsets != nullptr);
    }
    else {
        // Bitmap font, accept for now.
        free(m_pGlyphOffsets);
        m_pGlyphOffsets = static_cast<sal_uInt32 *>(calloc(m_nGlyphs + 1, sizeof(sal_uInt32)));
        /* TODO: implement to get subsetting */
        assert(m_pGlyphOffsets != nullptr);
    }

    table = this->table(O_hhea, table_size);
    m_nHorzMetrics = (table && table_size >= 36) ? GetUInt16(table, 34) : 0;

    table = this->table(O_vhea, table_size);
    m_nVertMetrics = (table && table_size >= 36) ? GetUInt16(table, 34) : 0;

    if (!m_xCharMap.is())
    {
        CmapResult aCmapResult;
        table = this->table(O_cmap, table_size);
        if (!ParseCMAP(table, table_size, aCmapResult))
            return SFErrCodes::TtFormat;
        m_xCharMap = new FontCharMap(aCmapResult);
    }

    return SFErrCodes::Ok;
}

SFErrCodes TrueTypeFont::open(sal_uInt32 facenum)
{
    if (fsize < 4)
        return SFErrCodes::TtFormat;

    int i;
    sal_uInt32 length, tag;
    sal_uInt32 tdoffset = 0;        /* offset to TableDirectory in a TTC file. For TTF files is 0 */

    sal_uInt32 TTCTag = GetInt32(ptr, 0);

    if ((TTCTag == 0x00010000) || (TTCTag == T_true)) {
        tdoffset = 0;
    } else if (TTCTag == T_otto) {                         /* PS-OpenType font */
        tdoffset = 0;
    } else if (TTCTag == T_ttcf) {                         /* TrueType collection */
        if (!withinBounds(12, 4 * facenum, sizeof(sal_uInt32), fsize))
            return SFErrCodes::FontNo;
        sal_uInt32 Version = GetUInt32(ptr, 4);
        if (Version != 0x00010000 && Version != 0x00020000) {
            return SFErrCodes::TtFormat;
        }
        if (facenum >= GetUInt32(ptr, 8))
            return SFErrCodes::FontNo;
        tdoffset = GetUInt32(ptr, 12 + 4 * facenum);
    } else {
        return SFErrCodes::TtFormat;
    }

    if (withinBounds(tdoffset, 0, 4 + sizeof(sal_uInt16), fsize))
        ntables = GetUInt16(ptr + tdoffset, 4);

    if (ntables >= 128 || ntables == 0)
        return SFErrCodes::TtFormat;

    /* parse the tables */
    for (i = 0; i < static_cast<int>(ntables); i++)
    {
        int nIndex;
        const sal_uInt32 nStart = tdoffset + 12;
        const sal_uInt32 nOffset = 16 * i;
        if (withinBounds(nStart, nOffset, sizeof(sal_uInt32), fsize))
            tag = GetUInt32(ptr + nStart, nOffset);
        else
            tag = static_cast<sal_uInt32>(-1);
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
            case T_cvt : nIndex = O_cvt;  break;
            case T_prep: nIndex = O_prep; break;
            case T_fpgm: nIndex = O_fpgm; break;
            case T_gsub: nIndex = O_gsub; break;
            case T_CFF:  nIndex = O_CFF; break;
            default: nIndex = -1; break;
        }

        if ((nIndex >= 0) && withinBounds(nStart, nOffset, 12 + sizeof(sal_uInt32), fsize))
        {
            sal_uInt32 nTableOffset = GetUInt32(ptr + nStart, nOffset + 8);
            length = GetUInt32(ptr + nStart, nOffset + 12);
            m_aTableList[nIndex].pData = ptr + nTableOffset;
            m_aTableList[nIndex].nSize = length;
        }
    }

    /* Fixup offsets when only a TTC extract was provided */
    if (facenum == sal_uInt32(~0))
    {
        sal_uInt8* pHead = const_cast<sal_uInt8*>(m_aTableList[O_head].pData);
        if (!pHead)
            return SFErrCodes::TtFormat;

        /* limit Head candidate to TTC extract's limits */
        if (pHead > ptr + (fsize - 54))
            pHead = ptr + (fsize - 54);

        /* TODO: find better method than searching head table's magic */
        sal_uInt8* p = nullptr;
        for (p = pHead + 12; p > ptr; --p)
        {
            if( p[0]==0x5F && p[1]==0x0F && p[2]==0x3C && p[3]==0xF5 ) {
                int nDelta = (pHead + 12) - p;
                if( nDelta )
                    for( int j = 0; j < NUM_TAGS; ++j )
                        if (hasTable(j))
                            m_aTableList[j].pData -= nDelta;
                break;
            }
        }
        if (p <= ptr)
            return SFErrCodes::TtFormat;
    }

    /* Check the table offsets after TTC correction */
    for (i=0; i<NUM_TAGS; i++) {
        /* sanity check: table must lay completely within the file
         * at this point one could check the checksum of all contained
         * tables, but this would be quite time intensive.
         * Try to fix tables, so we can cope with minor problems.
         */

        if (m_aTableList[i].pData < ptr)
        {
#if OSL_DEBUG_LEVEL > 1
            SAL_WARN_IF(m_aTableList[i].pData, "vcl.fonts", "font file " << fileName()
                    << " has bad table offset "
                    << (sal_uInt8*)m_aTableList[i].pData - ptr
                    << "d (tagnum=" << i << ").");
#endif
            m_aTableList[i].nSize = 0;
            m_aTableList[i].pData = nullptr;
        }
        else if (const_cast<sal_uInt8*>(m_aTableList[i].pData) + m_aTableList[i].nSize > ptr + fsize)
        {
            sal_PtrDiff nMaxLen = (ptr + fsize) - m_aTableList[i].pData;
            if( nMaxLen < 0 )
                nMaxLen = 0;
            m_aTableList[i].nSize = nMaxLen;
#if OSL_DEBUG_LEVEL > 1
            SAL_WARN("vcl.fonts", "font file " << fileName()
                    << " has too big table (tagnum=" << i << ").");
#endif
        }
    }

    /* At this point TrueTypeFont is constructed, now need to verify the font format
       and read the basic font properties */

    SFErrCodes ret = indexGlyphData();
    if (ret != SFErrCodes::Ok)
        return ret;

    GetNames(this);

    return SFErrCodes::Ok;
}

int GetTTGlyphPoints(AbstractTrueTypeFont *ttf, sal_uInt32 glyphID, ControlPoint **pointArray)
{
    return GetTTGlyphOutline(ttf, glyphID, pointArray, nullptr, nullptr);
}

int GetTTGlyphComponents(AbstractTrueTypeFont *ttf, sal_uInt32 glyphID, std::vector< sal_uInt32 >& glyphlist)
{
    int n = 1;

    if (glyphID >= ttf->glyphCount())
        return 0;

    sal_uInt32 nSize;
    const sal_uInt8* glyf = ttf->table(O_glyf, nSize);
    const sal_uInt8* ptr = glyf + ttf->glyphOffset(glyphID);
    const sal_uInt8* nptr = glyf + ttf->glyphOffset(glyphID + 1);
    if (nptr <= ptr)
        return 0;

    glyphlist.push_back( glyphID );

    if (GetInt16(ptr, 0) == -1) {
        sal_uInt16 flags, index;
        ptr += 10;
        do {
            flags = GetUInt16(ptr, 0);
            index = GetUInt16(ptr, 2);

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

SFErrCodes CreateT3FromTTGlyphs(TrueTypeFont *ttf, FILE *outf, const char *fname,
                          sal_uInt16 const *glyphArray, sal_uInt8 *encoding, int nGlyphs,
                          int wmode)
{
    ControlPoint *pa;
    PSPathElement *path;
    int i, j, n;
    sal_uInt32 nSize;
    const sal_uInt8* table = ttf->table(O_head, nSize);
    TTGlyphMetrics metrics;
    int UPEm = ttf->unitsPerEm();

    const char * const h01 = "%%!PS-AdobeFont-%d.%d-%d.%d\n";
    const char * const h02 = "%% Creator: %s %s %s\n";
    const char * const h09 = "%% Original font name: %s\n";

    const char * const h10 =
        "30 dict begin\n"
        "/PaintType 0 def\n"
        "/FontType 3 def\n"
        "/StrokeWidth 0 def\n";

    const char * const h11 = "/FontName (%s) cvn def\n";

    /*
      const char *h12 = "%/UniqueID %d def\n";
    */
    const char * const h13 = "/FontMatrix [.001 0 0 .001 0 0] def\n";
    const char * const h14 = "/FontBBox [%d %d %d %d] def\n";

    const char * const h15=
        "/Encoding 256 array def\n"
        "    0 1 255 {Encoding exch /.notdef put} for\n";

    const char * const h16 = "    Encoding %d /glyph%d put\n";
    const char * const h17 = "/XUID [103 0 0 16#%08" SAL_PRIXUINT32 " %d 16#%08" SAL_PRIXUINT32 " 16#%08" SAL_PRIXUINT32 "] def\n";

    const char * const h30 = "/CharProcs %d dict def\n";
    const char * const h31 = "  CharProcs begin\n";
    const char * const h32 = "    /.notdef {} def\n";
    const char * const h33 = "    /glyph%d {\n";
    const char * const h34 = "    } bind def\n";
    const char * const h35 = "  end\n";

    const char * const h40 =
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

    const char * const h41 = "(%s) cvn exch definefont pop\n";

    if ((nGlyphs <= 0) || (nGlyphs > 256)) return SFErrCodes::GlyphNum;
    if (!glyphArray) return SFErrCodes::BadArg;
    if (!fname) fname = ttf->psname;

    fprintf(outf, h01, GetInt16(table, 0), GetUInt16(table, 2), GetInt16(table, 4), GetUInt16(table, 6));
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
    fprintf(outf, h14, XUnits(UPEm, GetInt16(table, 36)), XUnits(UPEm, GetInt16(table, 38)), XUnits(UPEm, GetInt16(table, 40)), XUnits(UPEm, GetInt16(table, 42)));
    fprintf(outf, "%s", h15);

    for (i = 0; i < nGlyphs; i++) {
        fprintf(outf, h16, encoding[i], i);
    }

    fprintf(outf, h30, nGlyphs+1);
    fprintf(outf, "%s", h31);
    fprintf(outf, "%s", h32);

    for (i = 0; i < nGlyphs; i++) {
        fprintf(outf, h33, i);
        int r = GetTTGlyphOutline(ttf, glyphArray[i] < ttf->glyphCount() ? glyphArray[i] : 0, &pa, &metrics, nullptr);

        if (r > 0) {
            n =  BSplineToPSPath(pa, r, &path);
        } else {
            n = 0;                      /* glyph might have zero contours but valid metrics ??? */
            path = nullptr;
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

    return SFErrCodes::Ok;
}

SFErrCodes CreateTTFromTTGlyphs(AbstractTrueTypeFont  *ttf,
                          const char    *fname,
                          sal_uInt16 const *glyphArray,
                          sal_uInt8 const *encoding,
                          int            nGlyphs)
{
    TrueTypeCreator *ttcr;
    TrueTypeTable *head=nullptr, *hhea=nullptr, *maxp=nullptr, *cvt=nullptr, *prep=nullptr, *glyf=nullptr, *fpgm=nullptr, *cmap=nullptr, *name=nullptr, *post = nullptr, *os2 = nullptr;
    int i;
    SFErrCodes res;

    TrueTypeCreatorNewEmpty(T_true, &ttcr);

    /**                       name                         **/

    NameRecord *names;
    int n = GetTTNameRecords(ttf, &names);
    name = TrueTypeTableNew_name(n, names);
    DisposeNameRecords(names, n);

    /**                       maxp                         **/
    sal_uInt32 nTableSize;
    const sal_uInt8* p = ttf->table(O_maxp, nTableSize);
    maxp = TrueTypeTableNew_maxp(p, nTableSize);

    /**                       hhea                         **/
    p = ttf->table(O_hhea, nTableSize);
    if (p)
        hhea = TrueTypeTableNew_hhea(GetInt16(p, HHEA_ascender_offset), GetInt16(p, HHEA_descender_offset), GetInt16(p, HHEA_lineGap_offset), GetInt16(p, HHEA_caretSlopeRise_offset), GetInt16(p, HHEA_caretSlopeRun_offset));
    else
        hhea = TrueTypeTableNew_hhea(0, 0, 0, 0, 0);

    /**                       head                         **/

    p = ttf->table(O_head, nTableSize);
    assert(p != nullptr);
    head = TrueTypeTableNew_head(GetInt32(p, HEAD_fontRevision_offset),
                                 GetUInt16(p, HEAD_flags_offset),
                                 GetUInt16(p, HEAD_unitsPerEm_offset),
                                 p+HEAD_created_offset,
                                 GetUInt16(p, HEAD_macStyle_offset),
                                 GetUInt16(p, HEAD_lowestRecPPEM_offset),
                                 GetInt16(p, HEAD_fontDirectionHint_offset));

    /**                       glyf                          **/

    glyf = TrueTypeTableNew_glyf();
    sal_uInt32* gID = static_cast<sal_uInt32*>(scalloc(nGlyphs, sizeof(sal_uInt32)));

    for (i = 0; i < nGlyphs; i++) {
        gID[i] = glyfAdd(glyf, GetTTRawGlyphData(ttf, glyphArray[i]), ttf);
    }

    /**                       cmap                          **/
    cmap = TrueTypeTableNew_cmap();

    for (i=0; i < nGlyphs; i++) {
        cmapAdd(cmap, 0x010000, encoding[i], gID[i]);
    }

    /**                       cvt                           **/
    if ((p = ttf->table(O_cvt, nTableSize)) != nullptr)
        cvt = TrueTypeTableNew(T_cvt, nTableSize, p);

    /**                       prep                          **/
    if ((p = ttf->table(O_prep, nTableSize)) != nullptr)
        prep = TrueTypeTableNew(T_prep, nTableSize, p);

    /**                       fpgm                          **/
    if ((p = ttf->table(O_fpgm, nTableSize)) != nullptr)
        fpgm = TrueTypeTableNew(T_fpgm, nTableSize, p);

    /**                       post                          **/
    if ((p = ttf->table(O_post, nTableSize)) != nullptr)
        post = TrueTypeTableNew_post(0x00030000,
                                     GetInt32(p, POST_italicAngle_offset),
                                     GetInt16(p, POST_underlinePosition_offset),
                                     GetInt16(p, POST_underlineThickness_offset),
                                     GetUInt32(p, POST_isFixedPitch_offset));
    else
        post = TrueTypeTableNew_post(0x00030000, 0, 0, 0, 0);

    AddTable(ttcr, name); AddTable(ttcr, maxp); AddTable(ttcr, hhea);
    AddTable(ttcr, head); AddTable(ttcr, glyf); AddTable(ttcr, cmap);
    AddTable(ttcr, cvt ); AddTable(ttcr, prep); AddTable(ttcr, fpgm);
    AddTable(ttcr, post); AddTable(ttcr, os2);

    res = StreamToFile(ttcr, fname);
#if OSL_DEBUG_LEVEL > 1
    SAL_WARN_IF(res != SFErrCodes::Ok, "vcl.fonts", "StreamToFile: error code: "
            << (int) res << ".");
#endif

    TrueTypeCreatorDispose(ttcr);
    free(gID);

    return res;
}

static GlyphOffsets *GlyphOffsetsNew(sal_uInt8 *sfntP, sal_uInt32 sfntLen)
{
    GlyphOffsets* res = static_cast<GlyphOffsets*>(smalloc(sizeof(GlyphOffsets)));
    sal_uInt8 *loca = nullptr;
    sal_uInt16 numTables = GetUInt16(sfntP, 4);
    sal_uInt32 locaLen = 0;
    sal_Int16 indexToLocFormat = 0;

    sal_uInt32 nMaxPossibleTables = sfntLen / (3*sizeof(sal_uInt32)); /*the three GetUInt32 calls*/
    if (numTables > nMaxPossibleTables)
    {
        SAL_WARN( "vcl.fonts", "GlyphOffsetsNew claimed to have "
            << numTables  << " tables, but that's impossibly large");
        numTables = nMaxPossibleTables;
    }

    for (sal_uInt16 i = 0; i < numTables; i++) {
        sal_uInt32 nLargestFixedOffsetPos = 12 + 16 * i + 12;
        sal_uInt32 nMinSize = nLargestFixedOffsetPos + sizeof(sal_uInt32);
        if (nMinSize > sfntLen)
        {
            SAL_WARN( "vcl.fonts", "GlyphOffsetsNew claimed to have "
                << numTables  << " tables, but only space for " << i);
            break;
        }

        sal_uInt32 tag = GetUInt32(sfntP, 12 + 16 * i);
        sal_uInt32 off = GetUInt32(sfntP, 12 + 16 * i + 8);
        sal_uInt32 len = GetUInt32(sfntP, nLargestFixedOffsetPos);

        if (tag == T_loca) {
            loca = sfntP + off;
            locaLen = len;
        } else if (tag == T_head) {
            indexToLocFormat = GetInt16(sfntP + off, 50);
        }
    }

    res->nGlyphs = locaLen / ((indexToLocFormat == 1) ? 4 : 2);
    assert(res->nGlyphs != 0);
    res->offs = static_cast<sal_uInt32*>(scalloc(res->nGlyphs, sizeof(sal_uInt32)));

    for (sal_uInt32 i = 0; i < res->nGlyphs; i++) {
        if (indexToLocFormat == 1) {
            res->offs[i] = GetUInt32(loca, i * 4);
        } else {
            res->offs[i] = GetUInt16(loca, i * 2) << 1;
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

static void DumpSfnts(FILE *outf, sal_uInt8 *sfntP, sal_uInt32 sfntLen)
{
    if (sfntLen < 12)
    {
        SAL_WARN( "vcl.fonts", "DumpSfnts sfntLen is too short: "
            << sfntLen << " legal min is: " << 12);
        return;
    }

    const sal_uInt32 nSpaceForTables = sfntLen - 12;
    const sal_uInt32 nTableSize = 16;
    const sal_uInt32 nMaxPossibleTables = nSpaceForTables/nTableSize;

    HexFmt *h = HexFmtNew(outf);
    sal_uInt16 i, numTables = GetUInt16(sfntP, 4);
    GlyphOffsets *go = GlyphOffsetsNew(sfntP, sfntLen);
    sal_uInt8 const pad[] = {0,0,0,0};                     /* zeroes                       */

    if (numTables > nMaxPossibleTables)
    {
        SAL_WARN( "vcl.fonts", "DumpSfnts claimed to have "
            << numTables  << " tables, but only space for " << nMaxPossibleTables);
        numTables = nMaxPossibleTables;
    }

    assert(numTables <= 9);                                 /* Type42 has 9 required tables */

    sal_uInt32* offs = static_cast<sal_uInt32*>(scalloc(numTables, sizeof(sal_uInt32)));

    fputs("/sfnts [", outf);
    HexFmtOpenString(h);
    HexFmtBlockWrite(h, sfntP, 12);                         /* stream out the Offset Table    */
    HexFmtBlockWrite(h, sfntP+12, 16 * numTables);          /* stream out the Table Directory */

    for (i=0; i<numTables; i++)
    {
        sal_uInt32 nLargestFixedOffsetPos = 12 + 16 * i + 12;
        sal_uInt32 nMinSize = nLargestFixedOffsetPos + sizeof(sal_uInt32);
        if (nMinSize > sfntLen)
        {
            SAL_WARN( "vcl.fonts", "DumpSfnts claimed to have "
                << numTables  << " tables, but only space for " << i);
            break;
        }

        sal_uInt32 tag = GetUInt32(sfntP, 12 + 16 * i);
        sal_uInt32 off = GetUInt32(sfntP, 12 + 16 * i + 8);
        if (off > sfntLen)
        {
            SAL_WARN( "vcl.fonts", "DumpSfnts claims offset of "
                << off << " but max possible is " << sfntLen);
            break;
        }
        sal_uInt8 *pRecordStart = sfntP + off;
        sal_uInt32 len = GetUInt32(sfntP, nLargestFixedOffsetPos);
        sal_uInt32 nMaxLenPossible = sfntLen - off;
        if (len > nMaxLenPossible)
        {
            SAL_WARN( "vcl.fonts", "DumpSfnts claims len of "
                << len << " but only space for " << nMaxLenPossible);
            break;
        }

        if (tag != T_glyf)
        {
            HexFmtBlockWrite(h, pRecordStart, len);
        }
        else
        {
            sal_uInt8 *glyf = pRecordStart;
            for (sal_uInt32 j = 0; j < go->nGlyphs - 1; j++)
            {
                sal_uInt32 o = go->offs[j];
                sal_uInt32 l = go->offs[j + 1] - o;
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
}

SFErrCodes CreateT42FromTTGlyphs(TrueTypeFont  *ttf,
                           FILE          *outf,
                           const char    *psname,
                           sal_uInt16 const *glyphArray,
                           sal_uInt8          *encoding,
                           int            nGlyphs)
{
    TrueTypeCreator *ttcr;
    TrueTypeTable *head=nullptr, *hhea=nullptr, *maxp=nullptr, *cvt=nullptr, *prep=nullptr, *glyf=nullptr, *fpgm=nullptr;
    int i;
    SFErrCodes res;

    sal_uInt16 ver;
    sal_Int32 rev;

    sal_uInt8 *sfntP;
    sal_uInt32 sfntLen;
    int UPEm = ttf->unitsPerEm();

    if (nGlyphs >= 256) return SFErrCodes::GlyphNum;

    assert(psname != nullptr);

    TrueTypeCreatorNewEmpty(T_true, &ttcr);

    /*                        head                          */
    sal_uInt32 nTableSize;
    const sal_uInt8* p = ttf->table(O_head, nTableSize);
    const sal_uInt8* headP = p;
    assert(p != nullptr);
    head = TrueTypeTableNew_head(GetInt32(p, HEAD_fontRevision_offset), GetUInt16(p, HEAD_flags_offset), GetUInt16(p, HEAD_unitsPerEm_offset), p+HEAD_created_offset, GetUInt16(p, HEAD_macStyle_offset), GetUInt16(p, HEAD_lowestRecPPEM_offset), GetInt16(p, HEAD_fontDirectionHint_offset));
    ver = GetUInt16(p, HEAD_majorVersion_offset);
    rev = GetInt32(p, HEAD_fontRevision_offset);

    /**                       hhea                         **/
    p = ttf->table(O_hhea, nTableSize);
    if (p)
        hhea = TrueTypeTableNew_hhea(GetInt16(p, HHEA_ascender_offset), GetInt16(p, HHEA_descender_offset), GetInt16(p, HHEA_lineGap_offset), GetInt16(p, HHEA_caretSlopeRise_offset), GetInt16(p, HHEA_caretSlopeRun_offset));
    else
        hhea = TrueTypeTableNew_hhea(0, 0, 0, 0, 0);

    /**                       maxp                         **/
    p = ttf->table(O_maxp, nTableSize);
    maxp = TrueTypeTableNew_maxp(p, nTableSize);

    /**                       cvt                           **/
    if ((p = ttf->table(O_cvt, nTableSize)) != nullptr)
        cvt = TrueTypeTableNew(T_cvt, nTableSize, p);

    /**                       prep                          **/
    if ((p = ttf->table(O_prep, nTableSize)) != nullptr)
        prep = TrueTypeTableNew(T_prep, nTableSize, p);

    /**                       fpgm                          **/
    if ((p = ttf->table(O_fpgm, nTableSize)) != nullptr)
        fpgm = TrueTypeTableNew(T_fpgm, nTableSize, p);

    /**                       glyf                          **/
    glyf = TrueTypeTableNew_glyf();
    sal_uInt16* gID = static_cast<sal_uInt16*>(scalloc(nGlyphs, sizeof(sal_uInt32)));

    for (i = 0; i < nGlyphs; i++) {
        gID[i] = static_cast<sal_uInt16>(glyfAdd(glyf, GetTTRawGlyphData(ttf, glyphArray[i]), ttf));
    }

    AddTable(ttcr, head); AddTable(ttcr, hhea); AddTable(ttcr, maxp); AddTable(ttcr, cvt);
    AddTable(ttcr, prep); AddTable(ttcr, glyf); AddTable(ttcr, fpgm);

    if ((res = StreamToMemory(ttcr, &sfntP, &sfntLen)) != SFErrCodes::Ok) {
        TrueTypeCreatorDispose(ttcr);
        free(gID);
        return res;
    }

    fprintf(outf, "%%!PS-TrueTypeFont-%d.%d-%d.%d\n", static_cast<int>(ver), static_cast<int>(ver & 0xFF), static_cast<int>(rev>>16), static_cast<int>(rev & 0xFFFF));
    fprintf(outf, "%%%%Creator: %s %s %s\n", modname, modver, modextra);
    fprintf(outf, "%%- Font subset generated from a source font file: '%s'\n", ttf->fileName());
    fprintf(outf, "%%- Original font name: %s\n", ttf->psname);
    fprintf(outf, "%%- Original font family: %s\n", ttf->family);
    fprintf(outf, "%%- Original font sub-family: %s\n", ttf->subfamily);
    fprintf(outf, "11 dict begin\n");
    fprintf(outf, "/FontName (%s) cvn def\n", psname);
    fprintf(outf, "/PaintType 0 def\n");
    fprintf(outf, "/FontMatrix [1 0 0 1 0 0] def\n");
    fprintf(outf, "/FontBBox [%d %d %d %d] def\n", XUnits(UPEm, GetInt16(headP, HEAD_xMin_offset)), XUnits(UPEm, GetInt16(headP, HEAD_yMin_offset)), XUnits(UPEm, GetInt16(headP, HEAD_xMax_offset)), XUnits(UPEm, GetInt16(headP, HEAD_yMax_offset)));
    fprintf(outf, "/FontType 42 def\n");
    fprintf(outf, "/Encoding 256 array def\n");
    fprintf(outf, "    0 1 255 {Encoding exch /.notdef put} for\n");

    for (i = 1; i<nGlyphs; i++) {
        fprintf(outf, "Encoding %d /glyph%u put\n", encoding[i], gID[i]);
    }
    fprintf(outf, "/XUID [103 0 1 16#%08X %u 16#%08X 16#%08X] def\n", static_cast<unsigned int>(rtl_crc32(0, ttf->ptr, ttf->fsize)), static_cast<unsigned int>(nGlyphs), static_cast<unsigned int>(rtl_crc32(0, glyphArray, nGlyphs * 2)), static_cast<unsigned int>(rtl_crc32(0, encoding, nGlyphs)));

    DumpSfnts(outf, sfntP, sfntLen);

    /* dump charstrings */
    fprintf(outf, "/CharStrings %d dict dup begin\n", nGlyphs);
    fprintf(outf, "/.notdef 0 def\n");
    for (i = 1; i < static_cast<int>(glyfCount(glyf)); i++) {
        fprintf(outf,"/glyph%d %d def\n", i, i);
    }
    fprintf(outf, "end readonly def\n");

    fprintf(outf, "FontName currentdict end definefont pop\n");
    TrueTypeCreatorDispose(ttcr);
    free(gID);
    free(sfntP);
    return SFErrCodes::Ok;
}

std::unique_ptr<sal_uInt16[]> GetTTSimpleGlyphMetrics(AbstractTrueTypeFont const *ttf, const sal_uInt16 *glyphArray, int nGlyphs, bool vertical)
{
    const sal_uInt8* pTable;
    sal_uInt32 n;
    sal_uInt32 nTableSize;

    if (!vertical)
    {
        n = ttf->horzMetricCount();
        pTable = ttf->table(O_hmtx, nTableSize);
    }
    else
    {
        n = ttf->vertMetricCount();
        pTable = ttf->table(O_vmtx, nTableSize);
    }

    if (!nGlyphs || !glyphArray) return nullptr;        /* invalid parameters */
    if (!n || !pTable) return nullptr;                  /* the font does not contain the requested metrics */

    std::unique_ptr<sal_uInt16[]> res(new sal_uInt16[nGlyphs]);

    const int UPEm = ttf->unitsPerEm();
    for( int i = 0; i < nGlyphs; ++i) {
        sal_uInt32 nAdvOffset;
        sal_uInt16 glyphID = glyphArray[i];

        if (glyphID < n) {
            nAdvOffset = 4 * glyphID;
        } else {
            nAdvOffset = 4 * (n - 1);
        }

        if( nAdvOffset >= nTableSize)
            res[i] = 0; /* better than a crash for buggy fonts */
        else
            res[i] = static_cast<sal_uInt16>(
                XUnits( UPEm, GetUInt16( pTable, nAdvOffset) ) );
    }

    return res;
}

// TODO, clean up table parsing and re-use it elsewhere in this file.
void GetTTFontMetrics(const uint8_t *pHhea, size_t nHhea,
                      const uint8_t *pOs2, size_t nOs2,
                      TTGlobalFontInfo *info)
{
    /* There are 3 different versions of OS/2 table: original (68 bytes long),
     * Microsoft old (78 bytes long) and Microsoft new (86 bytes long,)
     * Apple's documentation recommends looking at the table length.
     */
    if (nOs2 >= OS2_V0_length)
    {
        info->fsSelection   = GetUInt16(pOs2, OS2_fsSelection_offset);
        info->typoAscender  = GetInt16(pOs2, OS2_typoAscender_offset);
        info->typoDescender = GetInt16(pOs2, OS2_typoDescender_offset);
        info->typoLineGap   = GetInt16(pOs2, OS2_typoLineGap_offset);
        info->winAscent     = GetUInt16(pOs2, OS2_winAscent_offset);
        info->winDescent    = GetUInt16(pOs2, OS2_winDescent_offset);
    }

    if (nHhea >= HHEA_lineGap_offset + 2) {
        info->ascender      = GetInt16(pHhea, HHEA_ascender_offset);
        info->descender     = GetInt16(pHhea, HHEA_descender_offset);
        info->linegap       = GetInt16(pHhea, HHEA_lineGap_offset);
    }
}

bool GetTTGlobalFontHeadInfo(const AbstractTrueTypeFont *ttf, int& xMin, int& yMin, int& xMax, int& yMax, sal_uInt16& macStyle)
{
    sal_uInt32 table_size;
    const sal_uInt8* table = ttf->table(O_head, table_size);
    if (table_size < 46)
        return false;

    const int UPEm = ttf->unitsPerEm();
    xMin = XUnits(UPEm, GetInt16(table, HEAD_xMin_offset));
    yMin = XUnits(UPEm, GetInt16(table, HEAD_yMin_offset));
    xMax = XUnits(UPEm, GetInt16(table, HEAD_xMax_offset));
    yMax = XUnits(UPEm, GetInt16(table, HEAD_yMax_offset));
    macStyle = GetUInt16(table, HEAD_macStyle_offset);
    return true;
}

void GetTTGlobalFontInfo(TrueTypeFont *ttf, TTGlobalFontInfo *info)
{
    int UPEm = ttf->unitsPerEm();

    memset(info, 0, sizeof(TTGlobalFontInfo));

    info->family = ttf->family;
    info->ufamily = ttf->ufamily;
    info->subfamily = ttf->subfamily;
    info->usubfamily = ttf->usubfamily;
    info->psname = ttf->psname;
    info->symbolEncoded = ttf->GetCharMap()->isSymbolic();

    sal_uInt32 table_size;
    const sal_uInt8* table = ttf->table(O_OS2, table_size);
    if (table_size >= 42)
    {
        info->weight = GetUInt16(table, OS2_usWeightClass_offset);
        info->width  = GetUInt16(table, OS2_usWidthClass_offset);

        if (table_size >= OS2_V0_length) {
            info->typoAscender = XUnits(UPEm,GetInt16(table, OS2_typoAscender_offset));
            info->typoDescender = XUnits(UPEm, GetInt16(table, OS2_typoDescender_offset));
            info->typoLineGap = XUnits(UPEm, GetInt16(table, OS2_typoLineGap_offset));
            info->winAscent = XUnits(UPEm, GetUInt16(table, OS2_winAscent_offset));
            info->winDescent = XUnits(UPEm, GetUInt16(table, OS2_winDescent_offset));
            /* sanity check; some fonts treat winDescent as signed
           * violating the standard */
            if( info->winDescent > 5*UPEm )
                info->winDescent = XUnits(UPEm, GetInt16(table, OS2_winDescent_offset));
        }
        memcpy(info->panose, table + OS2_panose_offset, OS2_panoseNbBytes_offset);
        info->typeFlags = GetUInt16( table, OS2_fsType_offset );
    }

    table = ttf->table(O_post, table_size);
    if (table_size >= 12 + sizeof(sal_uInt32))
    {
        info->pitch  = GetUInt32(table, POST_isFixedPitch_offset);
        info->italicAngle = GetInt32(table, POST_italicAngle_offset);
    }

    GetTTGlobalFontHeadInfo(ttf, info->xMin, info->yMin, info->xMax, info->yMax, info->macStyle);

    table  = ttf->table(O_hhea, table_size);
    if (table_size >= 10)
    {
        info->ascender  = XUnits(UPEm, GetInt16(table, HHEA_ascender_offset));
        info->descender = XUnits(UPEm, GetInt16(table, HHEA_descender_offset));
        info->linegap   = XUnits(UPEm, GetInt16(table, HHEA_lineGap_offset));
    }
}

GlyphData *GetTTRawGlyphData(AbstractTrueTypeFont *ttf, sal_uInt32 glyphID)
{
    sal_uInt32 length;
    const sal_uInt8* hmtx = ttf->table(O_hmtx, length);
    const sal_uInt8* glyf = ttf->table(O_glyf, length);
    int n;

    if (glyphID >= ttf->glyphCount())
        return nullptr;

    /* #127161# check the glyph offsets */
    if (length < ttf->glyphOffset(glyphID + 1))
        return nullptr;

    length = ttf->glyphOffset(glyphID + 1) - ttf->glyphOffset(glyphID);

    GlyphData* d = static_cast<GlyphData*>(malloc(sizeof(GlyphData))); assert(d != nullptr);

    if (length > 0) {
        const sal_uInt8* srcptr = glyf + ttf->glyphOffset(glyphID);
        const size_t nChunkLen = ((length + 1) & ~1);
        d->ptr = static_cast<sal_uInt8*>(malloc(nChunkLen)); assert(d->ptr != nullptr);
        memcpy(d->ptr, srcptr, length);
        memset(d->ptr + length, 0, nChunkLen - length);
        d->compflag = (GetInt16( srcptr, 0 ) < 0);
    } else {
        d->ptr = nullptr;
        d->compflag = false;
    }

    d->glyphID = glyphID;
    d->nbytes = static_cast<sal_uInt16>((length + 1) & ~1);

    /* now calculate npoints and ncontours */
    ControlPoint *cp;
    n = GetTTGlyphPoints(ttf, glyphID, &cp);
    if (n > 0)
    {
        int m = 0;
        for (int i = 0; i < n; i++)
        {
            if (cp[i].flags & 0x8000)
                m++;
        }
        d->npoints = static_cast<sal_uInt16>(n);
        d->ncontours = static_cast<sal_uInt16>(m);
        free(cp);
    } else {
        d->npoints = 0;
        d->ncontours = 0;
    }

    /* get advance width and left sidebearing */
    if (glyphID < ttf->horzMetricCount()) {
        d->aw = GetUInt16(hmtx, 4 * glyphID);
        d->lsb = GetInt16(hmtx, 4 * glyphID + 2);
    } else {
        d->aw = GetUInt16(hmtx, 4 * (ttf->horzMetricCount() - 1));
        d->lsb  = GetInt16(hmtx + ttf->horzMetricCount() * 4, (glyphID - ttf->horzMetricCount()) * 2);
    }

    return d;
}

int GetTTNameRecords(AbstractTrueTypeFont const *ttf, NameRecord **nr)
{
    sal_uInt32 nTableSize;
    const sal_uInt8* table = ttf->table(O_name, nTableSize);

    if (nTableSize < 6)
    {
#if OSL_DEBUG_LEVEL > 1
        SAL_WARN("vcl.fonts", "O_name table too small.");
#endif
        return 0;
    }

    sal_uInt16 n = GetUInt16(table, 2);
    sal_uInt32 nStrBase = GetUInt16(table, 4);
    int i;

    *nr = nullptr;
    if (n == 0) return 0;

    const sal_uInt32 remaining_table_size = nTableSize-6;
    const sal_uInt32 nMinRecordSize = 12;
    const sal_uInt32 nMaxRecords = remaining_table_size / nMinRecordSize;
    if (n > nMaxRecords)
    {
        SAL_WARN("vcl.fonts", "Parsing error in " << OUString::createFromAscii(ttf->fileName()) <<
                 ": " << nMaxRecords << " max possible entries, but " <<
                 n << " claimed, truncating");
        n = nMaxRecords;
    }

    NameRecord* rec = static_cast<NameRecord*>(calloc(n, sizeof(NameRecord)));
    assert(rec);

    for (i = 0; i < n; i++) {
        sal_uInt32 nLargestFixedOffsetPos = 6 + 10 + 12 * i;
        sal_uInt32 nMinSize = nLargestFixedOffsetPos + sizeof(sal_uInt16);
        if (nMinSize > nTableSize)
        {
            SAL_WARN( "vcl.fonts", "Font " << OUString::createFromAscii(ttf->fileName()) << " claimed to have "
                << n << " name records, but only space for " << i);
            n = i;
            break;
        }

        rec[i].platformID = GetUInt16(table, 6 + 0 + 12 * i);
        rec[i].encodingID = GetUInt16(table, 6 + 2 + 12 * i);
        rec[i].languageID = LanguageType(GetUInt16(table, 6 + 4 + 12 * i));
        rec[i].nameID = GetUInt16(table, 6 + 6 + 12 * i);
        rec[i].slen = GetUInt16(table, 6 + 8 + 12 * i);
        sal_uInt32 nStrOffset = GetUInt16(table, nLargestFixedOffsetPos);
        if (rec[i].slen) {
            if (nStrBase + nStrOffset + rec[i].slen >= nTableSize)
            {
                rec[i].sptr = nullptr;
                rec[i].slen = 0;
                continue;
            }

            const sal_uInt32 rec_string = nStrBase + nStrOffset;
            const size_t available_space = rec_string > nTableSize ? 0 : (nTableSize - rec_string);
            if (rec[i].slen <= available_space)
            {
                rec[i].sptr = static_cast<sal_uInt8 *>(malloc(rec[i].slen)); assert(rec[i].sptr != nullptr);
                memcpy(rec[i].sptr, table + rec_string, rec[i].slen);
            }
            else
            {
                rec[i].sptr = nullptr;
                rec[i].slen = 0;
            }
        } else {
            rec[i].sptr = nullptr;
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

template<size_t N> static void
append(std::bitset<N> & rSet, size_t const nOffset, sal_uInt32 const nValue)
{
    for (size_t i = 0; i < 32; ++i)
    {
        rSet.set(nOffset + i, (nValue & (1U << i)) != 0);
    }
}

bool getTTCoverage(
    std::optional<std::bitset<UnicodeCoverage::MAX_UC_ENUM>> &rUnicodeRange,
    std::optional<std::bitset<CodePageCoverage::MAX_CP_ENUM>> &rCodePageRange,
    const unsigned char* pTable, size_t nLength)
{
    bool bRet = false;
    // parse OS/2 header
    if (nLength >= OS2_Legacy_length)
    {
        rUnicodeRange = std::bitset<UnicodeCoverage::MAX_UC_ENUM>();
        append(*rUnicodeRange,  0, GetUInt32(pTable, OS2_ulUnicodeRange1_offset));
        append(*rUnicodeRange, 32, GetUInt32(pTable, OS2_ulUnicodeRange2_offset));
        append(*rUnicodeRange, 64, GetUInt32(pTable, OS2_ulUnicodeRange3_offset));
        append(*rUnicodeRange, 96, GetUInt32(pTable, OS2_ulUnicodeRange4_offset));
        bRet = true;
        if (nLength >= OS2_V1_length)
        {
            rCodePageRange = std::bitset<CodePageCoverage::MAX_CP_ENUM>();
            append(*rCodePageRange,  0, GetUInt32(pTable, OS2_ulCodePageRange1_offset));
            append(*rCodePageRange, 32, GetUInt32(pTable, OS2_ulCodePageRange2_offset));
        }
    }
    return bRet;
}

} // namespace vcl

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
