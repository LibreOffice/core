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
#ifdef SYSTEM_LIBFIXMATH
#include <libfixmath/fix16.hpp>
#else
#include <tools/fix16.hxx>
#endif
#include "ttcr.hxx"
#include "xlat.hxx"
#include <rtl/crc.h>
#include <rtl/ustring.hxx>
#include <rtl/ustrbuf.hxx>
#include <sal/log.hxx>
#include <o3tl/safeint.hxx>
#include <osl/endian.h>
#include <osl/thread.h>
#include <unotools/tempfile.hxx>
#include <fontsubset.hxx>
#include <algorithm>
#include <memory>

namespace vcl
{

namespace {

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
    return fix16_mul(a, b);
}

static F16Dot16 fixedDiv(F16Dot16 a, F16Dot16 b)
{
    return fix16_div(a, b);
}

/*- returns a * b / c -*/
/* XXX provide a real implementation that preserves accuracy */
static F16Dot16 fixedMulDiv(F16Dot16 a, F16Dot16 b, F16Dot16 c)
{
    F16Dot16 res = fixedMul(a, b);
    return fixedDiv(res, c);
}

/*- Translate units from TT to PS (standard 1/1000) -*/
static int XUnits(int unitsPerEm, int n)
{
    return (n * 1000) / unitsPerEm;
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

static int GetTTGlyphOutline(AbstractTrueTypeFont *, sal_uInt32 , std::vector<ControlPoint>&, TTGlyphMetrics *, std::vector< sal_uInt32 >* );

/* returns the number of control points, allocates the pointArray */
static int GetSimpleTTOutline(AbstractTrueTypeFont const *ttf, sal_uInt32 glyphID,
                              std::vector<ControlPoint>& pointArray, TTGlyphMetrics *metrics)
{
    sal_uInt32 nTableSize;
    const sal_uInt8* table = ttf->table(O_glyf, nTableSize);
    sal_uInt8 n;
    int i, j, z;

    pointArray.clear();

    if (glyphID >= ttf->glyphCount())
        return 0;

    sal_uInt32 nGlyphOffset = ttf->glyphOffset(glyphID);
    if (nGlyphOffset > nTableSize)
        return 0;

    const sal_uInt8* ptr = table + nGlyphOffset;
    const sal_uInt32 nMaxGlyphSize = nTableSize - nGlyphOffset;
    constexpr sal_uInt32 nContourOffset = 10;
    if (nMaxGlyphSize < nContourOffset)
        return 0;

    const sal_Int16 numberOfContours = GetInt16(ptr, GLYF_numberOfContours_offset);
    if( numberOfContours <= 0 )             /*- glyph is not simple */
        return 0;

    const sal_Int32 nMaxContours = (nMaxGlyphSize - nContourOffset)/2;
    if (numberOfContours > nMaxContours)
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
    for (i=0; i<numberOfContours; i++)
    {
        const sal_uInt16 t = GetUInt16(ptr, nContourOffset + i * 2);
        if (t > lastPoint)
            lastPoint = t;
    }

    sal_uInt32 nInstLenOffset = nContourOffset + numberOfContours * 2;
    if (nInstLenOffset + 2 > nMaxGlyphSize)
        return 0;
    sal_uInt16 instLen = GetUInt16(ptr, nInstLenOffset);

    sal_uInt32 nOffset = nContourOffset + 2 * numberOfContours + 2 + instLen;
    if (nOffset > nMaxGlyphSize)
        return 0;
    const sal_uInt8* p = ptr + nOffset;

    sal_uInt32 nBytesRemaining = nMaxGlyphSize - nOffset;
    const sal_uInt32 palen = lastPoint+1;

    //at a minimum its one byte per entry
    if (palen > nBytesRemaining || lastPoint > nBytesRemaining-1)
    {
        SAL_WARN("vcl.fonts", "Font " << OUString::createFromAscii(ttf->fileName()) <<
            "claimed a palen of "
            << palen << " but max bytes remaining is " << nBytesRemaining);
        return 0;
    }

    std::vector<ControlPoint> pa(palen);

    i = 0;
    while (i <= lastPoint) {
        if (!nBytesRemaining)
        {
            SAL_WARN("vcl.fonts", "short read");
            break;
        }
        sal_uInt8 flag = *p++;
        --nBytesRemaining;
        pa[i++].flags = static_cast<sal_uInt32>(flag);
        if (flag & 8) {                                     /*- repeat flag */
            if (!nBytesRemaining)
            {
                SAL_WARN("vcl.fonts", "short read");
                break;
            }
            n = *p++;
            --nBytesRemaining;
            // coverity[tainted_data : FALSE] - i > lastPoint extra checks the n loop bound
            for (j=0; j<n; j++) {
                if (i > lastPoint) {                        /*- if the font is really broken */
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
            if (!nBytesRemaining)
            {
                SAL_WARN("vcl.fonts", "short read");
                break;
            }
            if (pa[i].flags & 0x10) {
                z += static_cast<int>(*p++);
            } else {
                z -= static_cast<int>(*p++);
            }
            --nBytesRemaining;
        } else if ( !(pa[i].flags & 0x10)) {
            if (nBytesRemaining < 2)
            {
                SAL_WARN("vcl.fonts", "short read");
                break;
            }
            z += GetInt16(p, 0);
            p += 2;
            nBytesRemaining -= 2;
        }
        pa[i].x = static_cast<sal_Int16>(z);
    }

    /*- Process the Y coordinate */
    z = 0;
    for (i = 0; i <= lastPoint; i++) {
        if (pa[i].flags & 0x04) {
            if (!nBytesRemaining)
            {
                SAL_WARN("vcl.fonts", "short read");
                break;
            }
            if (pa[i].flags & 0x20) {
                z += *p++;
            } else {
                z -= *p++;
            }
            --nBytesRemaining;
        } else if ( !(pa[i].flags & 0x20)) {
            if (nBytesRemaining < 2)
            {
                SAL_WARN("vcl.fonts", "short read");
                break;
            }
            z += GetInt16(p, 0);
            p += 2;
            nBytesRemaining -= 2;
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

    pointArray = std::move(pa);
    return lastPoint + 1;
}

static F16Dot16 fromF2Dot14(sal_Int16 n)
{
    // Avoid undefined shift of negative values prior to C++2a:
    return sal_uInt32(n) << 2;
}

static int GetCompoundTTOutline(AbstractTrueTypeFont *ttf, sal_uInt32 glyphID, std::vector<ControlPoint>& pointArray,
                                TTGlyphMetrics *metrics, std::vector<sal_uInt32>& glyphlist)
{
    sal_uInt16 flags, index;
    sal_Int16 e, f;
    sal_uInt32 nTableSize;
    const sal_uInt8* table = ttf->table(O_glyf, nTableSize);
    std::vector<ControlPoint> myPoints;
    std::vector<ControlPoint> nextComponent;
    int i, np;
    F16Dot16 a = 0x10000, b = 0, c = 0, d = 0x10000, m, n, abs1, abs2, abs3;

    pointArray.clear();

    if (glyphID >= ttf->glyphCount())
        return 0;

    sal_uInt32 nGlyphOffset = ttf->glyphOffset(glyphID);
    if (nGlyphOffset > nTableSize)
        return 0;

    const sal_uInt8* ptr = table + nGlyphOffset;
    sal_uInt32 nAvailableBytes = nTableSize - nGlyphOffset;

    if (GLYF_numberOfContours_offset + 2 > nAvailableBytes)
        return 0;

    if (GetInt16(ptr, GLYF_numberOfContours_offset) != -1)   /* number of contours - glyph is not compound */
        return 0;

    if (metrics) {
        metrics->xMin = GetInt16(ptr, GLYF_xMin_offset);
        metrics->yMin = GetInt16(ptr, GLYF_yMin_offset);
        metrics->xMax = GetInt16(ptr, GLYF_xMax_offset);
        metrics->yMax = GetInt16(ptr, GLYF_yMax_offset);
        GetMetrics(ttf, glyphID, metrics);
    }

    if (nAvailableBytes < 10)
    {
        SAL_WARN("vcl.fonts", "short read");
        return 0;
    }

    ptr += 10;
    nAvailableBytes -= 10;

    do {

        if (nAvailableBytes < 4)
        {
            SAL_WARN("vcl.fonts", "short read");
            return 0;
        }
        flags = GetUInt16(ptr, 0);
        /* printf("flags: 0x%X\n", flags); */
        index = GetUInt16(ptr, 2);
        ptr += 4;
        nAvailableBytes -= 4;

        if( std::find( glyphlist.begin(), glyphlist.end(), index ) != glyphlist.end() )
        {
            SAL_WARN("vcl.fonts", "Endless loop found in a compound glyph.");

#if OSL_DEBUG_LEVEL > 1
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
            return 0;
        }

        glyphlist.push_back( index );

        np = GetTTGlyphOutline(ttf, index, nextComponent, nullptr, &glyphlist);

        if( ! glyphlist.empty() )
            glyphlist.pop_back();

        if (np == 0)
        {
            /* XXX that probably indicates a corrupted font */
            SAL_WARN("vcl.fonts", "An empty compound!");
            /* assert(!"An empty compound"); */
            return 0;
        }

        if ((flags & USE_MY_METRICS) && metrics)
            GetMetrics(ttf, index, metrics);

        if (flags & ARG_1_AND_2_ARE_WORDS) {
            if (nAvailableBytes < 4)
            {
                SAL_WARN("vcl.fonts", "short read");
                return 0;
            }
            e = GetInt16(ptr, 0);
            f = GetInt16(ptr, 2);
            /* printf("ARG_1_AND_2_ARE_WORDS: %d %d\n", e & 0xFFFF, f & 0xFFFF); */
            ptr += 4;
            nAvailableBytes -= 4;
        } else {
            if (nAvailableBytes < 2)
            {
                SAL_WARN("vcl.fonts", "short read");
                return 0;
            }
            if (flags & ARGS_ARE_XY_VALUES) {     /* args are signed */
                e = static_cast<sal_Int8>(*ptr++);
                f = static_cast<sal_Int8>(*ptr++);
                /* printf("ARGS_ARE_XY_VALUES: %d %d\n", e & 0xFF, f & 0xFF); */
            } else {                              /* args are unsigned */
                /* printf("!ARGS_ARE_XY_VALUES\n"); */
                e = *ptr++;
                f = *ptr++;
            }
            nAvailableBytes -= 2;
        }

        a = d = 0x10000;
        b = c = 0;

        if (flags & WE_HAVE_A_SCALE) {
            if (nAvailableBytes < 2)
            {
                SAL_WARN("vcl.fonts", "short read");
                return 0;
            }
            a = fromF2Dot14(GetInt16(ptr, 0));
            d = a;
            ptr += 2;
            nAvailableBytes -= 2;
        } else if (flags & WE_HAVE_AN_X_AND_Y_SCALE) {
            if (nAvailableBytes < 4)
            {
                SAL_WARN("vcl.fonts", "short read");
                return 0;
            }
            a = fromF2Dot14(GetInt16(ptr, 0));
            d = fromF2Dot14(GetInt16(ptr, 2));
            ptr += 4;
            nAvailableBytes -= 4;
        } else if (flags & WE_HAVE_A_TWO_BY_TWO) {
            if (nAvailableBytes < 8)
            {
                SAL_WARN("vcl.fonts", "short read");
                return 0;
            }
            a = fromF2Dot14(GetInt16(ptr, 0));
            b = fromF2Dot14(GetInt16(ptr, 2));
            c = fromF2Dot14(GetInt16(ptr, 4));
            d = fromF2Dot14(GetInt16(ptr, 6));
            ptr += 8;
            nAvailableBytes -= 8;
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

        SAL_WARN_IF(np && (!m || !n), "vcl.fonts", "Parsing error in " << OUString::createFromAscii(ttf->fileName()) <<
                     ": divide by zero");

        if (m != 0 && n != 0) {
            for (i=0; i<np; i++) {
                F16Dot16 t;
                ControlPoint cp;
                cp.flags = nextComponent[i].flags;
                const sal_uInt16 x = nextComponent[i].x;
                const sal_uInt16 y = nextComponent[i].y;
                t = o3tl::saturating_add(o3tl::saturating_add(fixedMulDiv(a, x << 16, m), fixedMulDiv(c, y << 16, m)), sal_Int32(sal_uInt16(e) << 16));
                cp.x = static_cast<sal_Int16>(fixedMul(t, m) >> 16);
                t = o3tl::saturating_add(o3tl::saturating_add(fixedMulDiv(b, x << 16, n), fixedMulDiv(d, y << 16, n)), sal_Int32(sal_uInt16(f) << 16));
                cp.y = static_cast<sal_Int16>(fixedMul(t, n) >> 16);

                myPoints.push_back( cp );
            }
        }

        if (myPoints.size() > SAL_MAX_UINT16) {
            SAL_WARN("vcl.fonts", "number of points has to be limited to max value GlyphData::npoints can contain, abandon effort");
            myPoints.clear();
            break;
        }

    } while (flags & MORE_COMPONENTS);

    // #i123417# some fonts like IFAOGrec have no outline points in some compound glyphs
    // so this unlikely but possible scenario should be handled gracefully
    if( myPoints.empty() )
        return 0;

    np = myPoints.size();

    pointArray = std::move(myPoints);

    return np;
}

/* NOTE: GetTTGlyphOutline() returns -1 if the glyphID is incorrect,
 * but Get{Simple|Compound}GlyphOutline returns 0 in such a case.
 *
 * NOTE: glyphlist is the stack of glyphs traversed while constructing
 * a composite glyph. This is a safeguard against endless recursion
 * in corrupted fonts.
 */
static int GetTTGlyphOutline(AbstractTrueTypeFont *ttf, sal_uInt32 glyphID, std::vector<ControlPoint>& pointArray, TTGlyphMetrics *metrics, std::vector< sal_uInt32 >* glyphlist)
{
    sal_uInt32 glyflength;
    const sal_uInt8 *table = ttf->table(O_glyf, glyflength);
    sal_Int16 numberOfContours;
    int res;
    pointArray.clear();

    if (metrics)
        memset(metrics, 0, sizeof(TTGlyphMetrics));

    if (glyphID >= ttf->glyphCount())
        return -1;

    sal_uInt32 nNextOffset = ttf->glyphOffset(glyphID + 1);
    if (nNextOffset > glyflength)
        return -1;

    sal_uInt32 nOffset = ttf->glyphOffset(glyphID);
    if (nOffset > nNextOffset)
        return -1;

    int length = nNextOffset - nOffset;
    if (length == 0) {                                      /*- empty glyphs still have hmtx and vmtx metrics values */
        if (metrics) GetMetrics(ttf, glyphID, metrics);
        return 0;
    }

    const sal_uInt8* ptr = table + nOffset;
    const sal_uInt32 nMaxGlyphSize = glyflength - nOffset;

    if (nMaxGlyphSize < 2)
        return -1;

    numberOfContours = GetInt16(ptr, 0);

    if (numberOfContours >= 0)
    {
        res = GetSimpleTTOutline(ttf, glyphID, pointArray, metrics);
    }
    else
    {
        std::vector< sal_uInt32 > aPrivList { glyphID };
        res = GetCompoundTTOutline(ttf, glyphID, pointArray, metrics, glyphlist ? *glyphlist : aPrivList );
    }

    return res;
}

/*- Extracts a string from the name table and allocates memory for it -*/

static OString nameExtract( const sal_uInt8* name, int nTableSize, int n, int dbFlag, OUString* ucs2result )
{
    OStringBuffer res;
    const sal_uInt8* ptr = name + GetUInt16(name, 4) + GetUInt16(name + 6, 12 * n + 10);
    int len = GetUInt16(name+6, 12 * n + 8);

    // sanity check
    const sal_uInt8* end_table = name+nTableSize;
    const int available_space = ptr > end_table ? 0 : (end_table - ptr);
    if( (len <= 0) || len > available_space)
    {
        if( ucs2result )
            ucs2result->clear();
        return OString();
    }

    if( ucs2result )
        ucs2result->clear();
    if (dbFlag) {
        res.setLength(len/2);
        for (int i = 0; i < len/2; i++)
        {
            res[i] = *(ptr + i * 2 + 1);
            SAL_WARN_IF(res[i] == 0, "vcl.fonts", "font name is bogus");
        }
        if( ucs2result )
        {
            OUStringBuffer buf(len/2);
            buf.setLength(len/2);
            for (int i = 0; i < len/2; i++ )
            {
                buf[i] = GetUInt16( ptr, 2*i );
                SAL_WARN_IF(buf[i] == 0, "vcl.fonts", "font name is bogus");
            }
            *ucs2result = buf.makeStringAndClear();
        }
    } else {
        res.setLength(len);
        memcpy(static_cast<void*>(const_cast<char*>(res.getStr())), ptr, len);
    }

    return res.makeStringAndClear();
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

static void GetNames(AbstractTrueTypeFont *t)
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
    t->psname.clear();
    if ((r = findname(table, n, 3, 1, 0x0409, 6)) != -1)
        t->psname = nameExtract(table, nTableSize, r, 1, nullptr);
    if ( t->psname.isEmpty() && (r = findname(table, n, 1, 0, 0, 6)) != -1)
        t->psname = nameExtract(table, nTableSize, r, 0, nullptr);
    if ( t->psname.isEmpty() && (r = findname(table, n, 3, 0, 0x0409, 6)) != -1)
    {
        // some symbol fonts like Marlett have a 3,0 name!
        t->psname = nameExtract(table, nTableSize, r, 1, nullptr);
    }
    // for embedded font in Ghostscript PDFs
    if ( t->psname.isEmpty() && (r = findname(table, n, 2, 2, 0, 6)) != -1)
    {
        t->psname = nameExtract(table, nTableSize, r, 0, nullptr);
    }
    if ( t->psname.isEmpty() )
    {
        if (!t->fileName().empty())
        {
            const char* pReverse = t->fileName().data() + t->fileName().length();
            /* take only last token of filename */
            while (pReverse != t->fileName().data() && *pReverse != '/') pReverse--;
            if(*pReverse == '/') pReverse++;
            int nReverseLen = strlen(pReverse);
            for (i=nReverseLen - 1; i > 0; i--)
            {
                /*- Remove the suffix  -*/
                if (*(pReverse + i) == '.' ) {
                    nReverseLen = i;
                    break;
                }
            }
            t->psname = OString(std::string_view(pReverse, nReverseLen));
        }
        else
            t->psname = "Unknown"_ostr;
    }

    /* Font family and subfamily names: preferred Apple */
    t->family.clear();
    if ((r = findname(table, n, 0, 0, 0, 1)) != -1)
        t->family = nameExtract(table, nTableSize, r, 1, &t->ufamily);
    if ( t->family.isEmpty() && (r = findname(table, n, 3, 1, 0x0409, 1)) != -1)
        t->family = nameExtract(table, nTableSize, r, 1, &t->ufamily);
    if ( t->family.isEmpty() && (r = findname(table, n, 1, 0, 0, 1)) != -1)
        t->family = nameExtract(table, nTableSize, r, 0, nullptr);
    if ( t->family.isEmpty() && (r = findname(table, n, 3, 1, 0x0411, 1)) != -1)
        t->family = nameExtract(table, nTableSize, r, 1, &t->ufamily);
    if ( t->family.isEmpty() && (r = findname(table, n, 3, 0, 0x0409, 1)) != -1)
        t->family = nameExtract(table, nTableSize, r, 1, &t->ufamily);
    if ( t->family.isEmpty() )
        t->family = t->psname;

    t->subfamily.clear();
    t->usubfamily.clear();
    if ((r = findname(table, n, 1, 0, 0, 2)) != -1)
        t->subfamily = nameExtract(table, nTableSize, r, 0, &t->usubfamily);
    if ( t->subfamily.isEmpty() && (r = findname(table, n, 3, 1, 0x0409, 2)) != -1)
        t->subfamily = nameExtract(table, nTableSize, r, 1, &t->usubfamily);

    /* #i60349# sanity check psname
     * psname practically has to be 7bit ASCII and should not contain spaces
     * there is a class of broken fonts which do not fulfill that at all, so let's try
     * if the family name is 7bit ASCII and take it instead if so
     */
    /* check psname */
    for( i = 0; i < t->psname.getLength() && bPSNameOK; i++ )
        if( t->psname[ i ] < 33 || (t->psname[ i ] & 0x80) )
            bPSNameOK = false;
    if( bPSNameOK )
        return;

    /* check if family is a suitable replacement */
    if( t->ufamily.isEmpty() && t->family.isEmpty() )
        return;

    bool bReplace = true;

    for( i = 0; i < t->ufamily.getLength() && bReplace; i++ )
        if( t->ufamily[ i ] < 33 || t->ufamily[ i ] > 127 )
            bReplace = false;
    if( bReplace )
    {
        t->psname = t->family;
    }
}

/*- Public functions */

int CountTTCFonts(const char* fname)
{
    FILE* fd;
#ifdef LINUX
    int nFD;
    int n;
    if (sscanf(fname, "/:FD:/%d%n", &nFD, &n) == 1 && fname[n] == '\0')
    {
        lseek(nFD, 0, SEEK_SET);
        int nDupFd = dup(nFD);
        fd = nDupFd != -1 ? fdopen(nDupFd, "rb") : nullptr;
    }
    else
#endif
        fd = fopen(fname, "rb");

    if (!fd)
        return 0;

    int nFonts = 0;
    sal_uInt8 buffer[12];
    if (fread(buffer, 1, 12, fd) == 12) {
        if(GetUInt32(buffer, 0) == T_ttcf )
            nFonts = GetUInt32(buffer, 8);
    }

    if (nFonts > 0)
    {
        fseek(fd, 0, SEEK_END);
        sal_uInt64 fileSize = ftell(fd);

        //Feel free to calc the exact max possible number of fonts a file
        //could contain given its physical size. But this will clamp it to
        //a sane starting point
        //http://processingjs.nihongoresources.com/the_smallest_font/
        //https://github.com/grzegorzrolek/null-ttf
        const int nMaxFontsPossible = fileSize / 528;
        if (nFonts > nMaxFontsPossible)
        {
            SAL_WARN("vcl.fonts", "font file " << fname <<" claims to have "
                     << nFonts << " fonts, but only "
                     << nMaxFontsPossible << " are possible");
            nFonts = nMaxFontsPossible;
        }
    }

    fclose(fd);

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

    if( (*ttf)->fileName().empty() )
    {
        ret = SFErrCodes::Memory;
        goto cleanup;
    }

    int nFD;
    int n;
    if (sscanf(fname, "/:FD:/%d%n", &nFD, &n) == 1 && fname[n] == '\0')
    {
        lseek(nFD, 0, SEEK_SET);
        fd = dup(nFD);
    }
    else
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
    : m_nGlyphs(0xFFFFFFFF)
    , m_nHorzMetrics(0)
    , m_nVertMetrics(0)
    , m_nUnitsPerEm(0)
    , m_xCharMap(xCharMap)
    , m_bMicrosoftSymbolEncoded(false)
{
    if (pFileName)
        m_sFileName = pFileName;
}

AbstractTrueTypeFont::~AbstractTrueTypeFont()
{
}

TrueTypeFont::TrueTypeFont(const char* pFileName, const FontCharMapRef xCharMap)
    : AbstractTrueTypeFont(pFileName, xCharMap)
    , fsize(-1)
    , ptr(nullptr)
    , ntables(0)
{
}

TrueTypeFont::~TrueTypeFont()
{
#if !defined(_WIN32)
    if (!fileName().empty())
        munmap(ptr, fsize);
#endif
}

void CloseTTFont(TrueTypeFont* ttf) { delete ttf; }

SFErrCodes AbstractTrueTypeFont::initialize()
{
    SFErrCodes ret = indexGlyphData();
    if (ret != SFErrCodes::Ok)
        return ret;

    GetNames(this);

    return SFErrCodes::Ok;
}

sal_uInt32 AbstractTrueTypeFont::glyphOffset(sal_uInt32 glyphID) const
{
    if (m_aGlyphOffsets.empty()) // the O_CFF and Bitmap cases
        return 0;
    return m_aGlyphOffsets[glyphID];
}

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

        m_aGlyphOffsets.clear();
        m_aGlyphOffsets.reserve(m_nGlyphs + 1);
        for (int i = 0; i <= static_cast<int>(m_nGlyphs); ++i)
            m_aGlyphOffsets.push_back(indexfmt ? GetUInt32(table, i << 2) : static_cast<sal_uInt32>(GetUInt16(table, i << 1)) << 1);
    }
    else if (this->table(O_CFF, table_size)) /* PS-OpenType */
    {
        int k = (table_size / 2) - 1; /* set a limit here, presumably much lower than the table size, but establishes some sort of physical bound */
        if (k < static_cast<int>(m_nGlyphs))
            m_nGlyphs = k;

        m_aGlyphOffsets.clear();
        /* TODO: implement to get subsetting */
    }
    else {
        // Bitmap font, accept for now.
        // TODO: We only need this for fonts with CBDT table since they usually
        // lack glyf or CFF table, the check should be more specific, or better
        // non-subsetting code should not be calling this.
        m_aGlyphOffsets.clear();
    }

    table = this->table(O_hhea, table_size);
    m_nHorzMetrics = (table && table_size >= 36) ? GetUInt16(table, 34) : 0;

    table = this->table(O_vhea, table_size);
    m_nVertMetrics = (table && table_size >= 36) ? GetUInt16(table, 34) : 0;

    if (!m_xCharMap.is())
    {
        table = this->table(O_cmap, table_size);
        m_bMicrosoftSymbolEncoded = HasMicrosoftSymbolCmap(table, table_size);
    }
    else
        m_bMicrosoftSymbolEncoded = m_xCharMap->isMicrosoftSymbolMap();

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

    return AbstractTrueTypeFont::initialize();
}

int GetTTGlyphPoints(AbstractTrueTypeFont *ttf, sal_uInt32 glyphID, std::vector<ControlPoint>& pointArray)
{
    return GetTTGlyphOutline(ttf, glyphID, pointArray, nullptr, nullptr);
}

int GetTTGlyphComponents(AbstractTrueTypeFont *ttf, sal_uInt32 glyphID, std::vector< sal_uInt32 >& glyphlist)
{
    int n = 1;

    if (glyphID >= ttf->glyphCount())
        return 0;

    sal_uInt32 glyflength;
    const sal_uInt8* glyf = ttf->table(O_glyf, glyflength);

    sal_uInt32 nNextOffset = ttf->glyphOffset(glyphID + 1);
    if (nNextOffset > glyflength)
        return 0;

    sal_uInt32 nOffset = ttf->glyphOffset(glyphID);
    if (nOffset > nNextOffset)
        return 0;

    if (std::find(glyphlist.begin(), glyphlist.end(), glyphID) != glyphlist.end())
    {
        SAL_WARN("vcl.fonts", "Endless loop found in a compound glyph.");
        return 0;
    }

    glyphlist.push_back( glyphID );

    // Empty glyph.
    if (nOffset == nNextOffset)
        return n;

    const auto* ptr = glyf + nOffset;
    sal_uInt32 nRemainingData = glyflength - nOffset;

    if (nRemainingData >= 10 && GetInt16(ptr, 0) == -1) {
        sal_uInt16 flags, index;
        ptr += 10;
        nRemainingData -= 10;
        do {
            if (nRemainingData < 4)
            {
                SAL_WARN("vcl.fonts", "short read");
                break;
            }
            flags = GetUInt16(ptr, 0);
            index = GetUInt16(ptr, 2);

            ptr += 4;
            nRemainingData -= 4;
            n += GetTTGlyphComponents(ttf, index, glyphlist);

            sal_uInt32 nAdvance;
            if (flags & ARG_1_AND_2_ARE_WORDS) {
                nAdvance = 4;
            } else {
                nAdvance = 2;
            }

            if (flags & WE_HAVE_A_SCALE) {
                nAdvance += 2;
            } else if (flags & WE_HAVE_AN_X_AND_Y_SCALE) {
                nAdvance += 4;
            } else if (flags & WE_HAVE_A_TWO_BY_TWO) {
                nAdvance += 8;
            }
            if (nRemainingData < nAdvance)
            {
                SAL_WARN("vcl.fonts", "short read");
                break;
            }
            ptr += nAdvance;
            nRemainingData -= nAdvance;
        } while (flags & MORE_COMPONENTS);
    }

    return n;
}

SFErrCodes CreateTTFromTTGlyphs(AbstractTrueTypeFont  *ttf,
                          std::vector<sal_uInt8>& rOutBuffer,
                          sal_uInt16 const *glyphArray,
                          sal_uInt8 const *encoding,
                          int            nGlyphs)
{
    std::unique_ptr<TrueTypeTableGeneric> cvt, prep, fpgm, os2;
    std::unique_ptr<TrueTypeTableName> name;
    std::unique_ptr<TrueTypeTableMaxp> maxp;
    std::unique_ptr<TrueTypeTableHhea> hhea;
    std::unique_ptr<TrueTypeTableHead> head;
    std::unique_ptr<TrueTypeTableGlyf> glyf;
    std::unique_ptr<TrueTypeTableCmap> cmap;
    std::unique_ptr<TrueTypeTablePost> post;
    int i;
    SFErrCodes res;

    TrueTypeCreator ttcr(T_true);

    /**                       name                         **/

    std::vector<NameRecord> names;
    GetTTNameRecords(ttf, names);
    name.reset(new TrueTypeTableName(std::move(names)));

    /**                       maxp                         **/
    sal_uInt32 nTableSize;
    const sal_uInt8* p = ttf->table(O_maxp, nTableSize);
    maxp.reset(new TrueTypeTableMaxp(p, nTableSize));

    /**                       hhea                         **/
    p = ttf->table(O_hhea, nTableSize);
    if (p && nTableSize >= HHEA_caretSlopeRun_offset + 2)
        hhea.reset(new TrueTypeTableHhea(GetInt16(p, HHEA_ascender_offset), GetInt16(p, HHEA_descender_offset), GetInt16(p, HHEA_lineGap_offset), GetInt16(p, HHEA_caretSlopeRise_offset), GetInt16(p, HHEA_caretSlopeRun_offset)));
    else
        hhea.reset(new TrueTypeTableHhea(0, 0, 0, 0, 0));

    /**                       head                         **/

    p = ttf->table(O_head, nTableSize);
    assert(p != nullptr);
    head.reset(new TrueTypeTableHead(GetInt32(p, HEAD_fontRevision_offset),
                                 GetUInt16(p, HEAD_flags_offset),
                                 GetUInt16(p, HEAD_unitsPerEm_offset),
                                 p+HEAD_created_offset,
                                 GetUInt16(p, HEAD_macStyle_offset),
                                 GetUInt16(p, HEAD_lowestRecPPEM_offset),
                                 GetInt16(p, HEAD_fontDirectionHint_offset)));

    /**                       glyf                          **/

    glyf.reset(new TrueTypeTableGlyf());
    std::unique_ptr<sal_uInt32[]> gID(new sal_uInt32[nGlyphs]);

    for (i = 0; i < nGlyphs; i++) {
        gID[i] = glyf->glyfAdd(GetTTRawGlyphData(ttf, glyphArray[i]), ttf);
    }

    /**                       cmap                          **/
    cmap.reset(new TrueTypeTableCmap());

    for (i=0; i < nGlyphs; i++) {
        cmap->cmapAdd(0x010000, encoding[i], gID[i]);
    }

    /**                       cvt                           **/
    if ((p = ttf->table(O_cvt, nTableSize)) != nullptr)
        cvt.reset(new TrueTypeTableGeneric(T_cvt, nTableSize, p));

    /**                       prep                          **/
    if ((p = ttf->table(O_prep, nTableSize)) != nullptr)
        prep.reset(new TrueTypeTableGeneric(T_prep, nTableSize, p));

    /**                       fpgm                          **/
    if ((p = ttf->table(O_fpgm, nTableSize)) != nullptr)
        fpgm.reset(new TrueTypeTableGeneric(T_fpgm, nTableSize, p));

    /**                       post                          **/
    if ((p = ttf->table(O_post, nTableSize)) != nullptr)
    {
        sal_Int32 nItalic = (POST_italicAngle_offset + 4 < nTableSize) ?
            GetInt32(p, POST_italicAngle_offset) : 0;
        sal_Int16 nPosition = (POST_underlinePosition_offset + 2 < nTableSize) ?
            GetInt16(p, POST_underlinePosition_offset) : 0;
        sal_Int16 nThickness = (POST_underlineThickness_offset + 2 < nTableSize) ?
            GetInt16(p, POST_underlineThickness_offset) : 0;
        sal_uInt32 nFixedPitch = (POST_isFixedPitch_offset + 4 < nTableSize) ?
            GetUInt32(p, POST_isFixedPitch_offset) : 0;

        post.reset(new TrueTypeTablePost(0x00030000,
                                     nItalic, nPosition,
                                     nThickness, nFixedPitch));
    }
    else
        post.reset(new TrueTypeTablePost(0x00030000, 0, 0, 0, 0));

    ttcr.AddTable(std::move(name)); ttcr.AddTable(std::move(maxp)); ttcr.AddTable(std::move(hhea));
    ttcr.AddTable(std::move(head)); ttcr.AddTable(std::move(glyf)); ttcr.AddTable(std::move(cmap));
    ttcr.AddTable(std::move(cvt)); ttcr.AddTable(std::move(prep)); ttcr.AddTable(std::move(fpgm));
    ttcr.AddTable(std::move(post)); ttcr.AddTable(std::move(os2));

    res = ttcr.StreamToMemory(rOutBuffer);
#if OSL_DEBUG_LEVEL > 1
    SAL_WARN_IF(res != SFErrCodes::Ok, "vcl.fonts", "StreamToMemory: error code: "
            << (int) res << ".");
#endif

    return res;
}

namespace
{
void FillFontSubsetInfo(AbstractTrueTypeFont* ttf, FontSubsetInfo& rInfo)
{
    TTGlobalFontInfo aTTInfo;
    GetTTGlobalFontInfo(ttf, &aTTInfo);

    rInfo.m_aPSName = OUString::fromUtf8(aTTInfo.psname);
    rInfo.m_nFontType = FontType::SFNT_TTF;
    rInfo.m_aFontBBox
        = tools::Rectangle(Point(aTTInfo.xMin, aTTInfo.yMin), Point(aTTInfo.xMax, aTTInfo.yMax));
    rInfo.m_nCapHeight = aTTInfo.yMax; // Well ...
    rInfo.m_nAscent = aTTInfo.winAscent;
    rInfo.m_nDescent = aTTInfo.winDescent;

    // mac fonts usually do not have an OS2-table
    // => get valid ascent/descent values from other tables
    if (!rInfo.m_nAscent)
        rInfo.m_nAscent = +aTTInfo.typoAscender;
    if (!rInfo.m_nAscent)
        rInfo.m_nAscent = +aTTInfo.ascender;
    if (!rInfo.m_nDescent)
        rInfo.m_nDescent = +aTTInfo.typoDescender;
    if (!rInfo.m_nDescent)
        rInfo.m_nDescent = -aTTInfo.descender;

    rInfo.m_bFilled = true;
}

bool CreateCFFfontSubset(const unsigned char* pFontBytes, int nByteLength,
                         std::vector<sal_uInt8>& rOutBuffer, const sal_GlyphId* pGlyphIds,
                         const sal_uInt8* pEncoding, int nGlyphCount, FontSubsetInfo& rInfo)
{
    utl::TempFileFast aTempFile;
    SvStream* pStream = aTempFile.GetStream(StreamMode::READWRITE);

    rInfo.LoadFont(FontType::CFF_FONT, pFontBytes, nByteLength);
    bool bRet = rInfo.CreateFontSubset(FontType::TYPE1_PFB, pStream, pGlyphIds, pEncoding,
                                       nGlyphCount);

    if (bRet)
    {
        rOutBuffer.resize(pStream->TellEnd());
        pStream->Seek(0);
        auto nRead = pStream->ReadBytes(rOutBuffer.data(), rOutBuffer.size());
        if (nRead != rOutBuffer.size())
        {
            rOutBuffer.clear();
            return false;
        }
    }

    return bRet;
}
}

bool CreateTTFfontSubset(vcl::AbstractTrueTypeFont& rTTF, std::vector<sal_uInt8>& rOutBuffer,
                         const sal_GlyphId* pGlyphIds, const sal_uInt8* pEncoding,
                         const int nOrigGlyphCount, FontSubsetInfo& rInfo)
{
    // Get details about the subset font.
    FillFontSubsetInfo(&rTTF, rInfo);

    // Shortcut for CFF-subsetting.
    sal_uInt32 nCFF;
    const sal_uInt8* pCFF = rTTF.table(O_CFF, nCFF);
    if (nCFF)
        return CreateCFFfontSubset(pCFF, nCFF, rOutBuffer, pGlyphIds, pEncoding,
                                   nOrigGlyphCount, rInfo);

    // Multiple questions:
    // - Why is there a glyph limit?
    //   MacOS used to handle 257 glyphs...
    //   Also the much more complex PrintFontManager variant has this limit.
    //   Also the very first implementation has the limit in
    //   commit 8789ed701e98031f2a1657ea0dfd6f7a0b050992
    // - Why doesn't the PrintFontManager care about the fake glyph? It
    //   is used on all unx platforms to create the subset font.
    // - Should the SAL_WARN actually be asserts, like on MacOS?
    if (nOrigGlyphCount > 256)
    {
        SAL_WARN("vcl.fonts", "too many glyphs for subsetting");
        return false;
    }

    int nGlyphCount = nOrigGlyphCount;
    sal_uInt16 aShortIDs[256];
    sal_uInt8 aTempEncs[256];

    // handle the undefined / first font glyph
    int nNotDef = -1, i;
    for (i = 0; i < nGlyphCount; ++i)
    {
        aTempEncs[i] = pEncoding[i];
        aShortIDs[i] = static_cast<sal_uInt16>(pGlyphIds[i]);
        if (!aShortIDs[i])
            if (nNotDef < 0)
                nNotDef = i;
    }

    // nNotDef glyph must be in pos 0 => swap glyphids
    if (nNotDef != 0)
    {
        if (nNotDef < 0)
        {
            if (nGlyphCount == 256)
            {
                SAL_WARN("vcl.fonts", "too many glyphs for subsetting");
                return false;
            }
            nNotDef = nGlyphCount++;
        }

        aShortIDs[nNotDef] = aShortIDs[0];
        aTempEncs[nNotDef] = aTempEncs[0];
        aShortIDs[0] = 0;
        aTempEncs[0] = 0;
    }

    // write subset into destination file
    return (CreateTTFromTTGlyphs(&rTTF, rOutBuffer, aShortIDs, aTempEncs, nGlyphCount)
            == vcl::SFErrCodes::Ok);
}

bool GetTTGlobalFontHeadInfo(const AbstractTrueTypeFont *ttf, int& xMin, int& yMin, int& xMax, int& yMax, sal_uInt16& macStyle)
{
    sal_uInt32 table_size;
    const sal_uInt8* table = ttf->table(O_head, table_size);
    if (table_size < 46)
        return false;

    const int UPEm = ttf->unitsPerEm();
    if (UPEm == 0)
        return false;
    xMin = XUnits(UPEm, GetInt16(table, HEAD_xMin_offset));
    yMin = XUnits(UPEm, GetInt16(table, HEAD_yMin_offset));
    xMax = XUnits(UPEm, GetInt16(table, HEAD_xMax_offset));
    yMax = XUnits(UPEm, GetInt16(table, HEAD_yMax_offset));
    macStyle = GetUInt16(table, HEAD_macStyle_offset);
    return true;
}

void GetTTGlobalFontInfo(AbstractTrueTypeFont *ttf, TTGlobalFontInfo *info)
{
    int UPEm = ttf->unitsPerEm();

    info->family = ttf->family;
    info->ufamily = ttf->ufamily;
    info->subfamily = ttf->subfamily;
    info->usubfamily = ttf->usubfamily;
    info->psname = ttf->psname;
    info->microsoftSymbolEncoded = ttf->IsMicrosoftSymbolEncoded();

    sal_uInt32 table_size;
    const sal_uInt8* table = ttf->table(O_OS2, table_size);
    if (table_size >= 42)
    {
        info->weight = GetUInt16(table, OS2_usWeightClass_offset);
        info->width  = GetUInt16(table, OS2_usWidthClass_offset);

        if (table_size >= OS2_V0_length && UPEm != 0) {
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
    if (table_size >= 10 && UPEm != 0)
    {
        info->ascender  = XUnits(UPEm, GetInt16(table, HHEA_ascender_offset));
        info->descender = XUnits(UPEm, GetInt16(table, HHEA_descender_offset));
        info->linegap   = XUnits(UPEm, GetInt16(table, HHEA_lineGap_offset));
    }
}

std::unique_ptr<GlyphData> GetTTRawGlyphData(AbstractTrueTypeFont *ttf, sal_uInt32 glyphID)
{
    if (glyphID >= ttf->glyphCount())
        return nullptr;

    sal_uInt32 hmtxlength;
    const sal_uInt8* hmtx = ttf->table(O_hmtx, hmtxlength);

    if (!hmtxlength)
        return nullptr;

    sal_uInt32 glyflength;
    const sal_uInt8* glyf = ttf->table(O_glyf, glyflength);
    int n;

    /* #127161# check the glyph offsets */
    sal_uInt32 nNextOffset = ttf->glyphOffset(glyphID + 1);
    if (nNextOffset > glyflength)
        return nullptr;

    sal_uInt32 nOffset = ttf->glyphOffset(glyphID);
    if (nOffset > nNextOffset)
        return nullptr;

    sal_uInt32 length = nNextOffset - nOffset;

    std::unique_ptr<GlyphData> d(new GlyphData);

    if (length > 0) {
        const sal_uInt8* srcptr = glyf + ttf->glyphOffset(glyphID);
        const size_t nChunkLen = ((length + 1) & ~1);
        d->ptr.reset(new sal_uInt8[nChunkLen]);
        memcpy(d->ptr.get(), srcptr, length);
        memset(d->ptr.get() + length, 0, nChunkLen - length);
        d->compflag = (GetInt16( srcptr, 0 ) < 0);
    } else {
        d->ptr = nullptr;
        d->compflag = false;
    }

    d->glyphID = glyphID;
    d->nbytes = static_cast<sal_uInt16>((length + 1) & ~1);

    /* now calculate npoints and ncontours */
    std::vector<ControlPoint> cp;
    n = GetTTGlyphPoints(ttf, glyphID, cp);
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
    } else {
        d->npoints = 0;
        d->ncontours = 0;
    }

    /* get advance width and left sidebearing */
    sal_uInt32 nAwOffset;
    sal_uInt32 nLsboffset;
    if (glyphID < ttf->horzMetricCount()) {
        nAwOffset = 4 * glyphID;
        nLsboffset = 4 * glyphID + 2;
    } else {
        nAwOffset = 4 * (ttf->horzMetricCount() - 1);
        nLsboffset = (ttf->horzMetricCount() * 4) + ((glyphID - ttf->horzMetricCount()) * 2);
    }

    if (nAwOffset + 2 <= hmtxlength)
        d->aw = GetUInt16(hmtx, nAwOffset);
    else
    {
        SAL_WARN("vcl.fonts", "hmtx offset " << nAwOffset << " not available");
        d->aw = 0;
    }
    if (nLsboffset + 2 <= hmtxlength)
        d->lsb = GetInt16(hmtx, nLsboffset);
    else
    {
        SAL_WARN("vcl.fonts", "hmtx offset " << nLsboffset << " not available");
        d->lsb = 0;
    }

    return d;
}

void GetTTNameRecords(AbstractTrueTypeFont const *ttf, std::vector<NameRecord>& nr)
{
    sal_uInt32 nTableSize;
    const sal_uInt8* table = ttf->table(O_name, nTableSize);

    nr.clear();

    if (nTableSize < 6)
    {
#if OSL_DEBUG_LEVEL > 1
        SAL_WARN("vcl.fonts", "O_name table too small.");
#endif
        return;
    }

    sal_uInt16 n = GetUInt16(table, 2);
    sal_uInt32 nStrBase = GetUInt16(table, 4);
    int i;

    if (n == 0) return;

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

    nr.resize(n);

    for (i = 0; i < n; i++) {
        sal_uInt32 nLargestFixedOffsetPos = 6 + 10 + 12 * i;
        sal_uInt32 nMinSize = nLargestFixedOffsetPos + sizeof(sal_uInt16);
        if (nMinSize > nTableSize)
        {
            SAL_WARN( "vcl.fonts", "Font " << OUString::createFromAscii(ttf->fileName()) << " claimed to have "
                << n << " name records, but only space for " << i);
            break;
        }

        nr[i].platformID = GetUInt16(table, 6 + 0 + 12 * i);
        nr[i].encodingID = GetUInt16(table, 6 + 2 + 12 * i);
        nr[i].languageID = LanguageType(GetUInt16(table, 6 + 4 + 12 * i));
        nr[i].nameID = GetUInt16(table, 6 + 6 + 12 * i);
        sal_uInt16 slen = GetUInt16(table, 6 + 8 + 12 * i);
        sal_uInt32 nStrOffset = GetUInt16(table, nLargestFixedOffsetPos);
        if (slen) {
            if (nStrBase + nStrOffset + slen >= nTableSize)
                continue;

            const sal_uInt32 rec_string = nStrBase + nStrOffset;
            const size_t available_space = rec_string > nTableSize ? 0 : (nTableSize - rec_string);
            if (slen <= available_space)
            {
                nr[i].sptr.resize(slen);
                memcpy(nr[i].sptr.data(), table + rec_string, slen);
            }
        }
        // some fonts have 3.0 names => fix them to 3.1
        if( (nr[i].platformID == 3) && (nr[i].encodingID == 0) )
            nr[i].encodingID = 1;
    }
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

int TestFontSubset(const void* data, sal_uInt32 size)
{
    int nResult = -1;
    vcl::TrueTypeFont* pTTF = nullptr;
    if (OpenTTFontBuffer(data, size, 0, &pTTF) == vcl::SFErrCodes::Ok)
    {
        vcl::TTGlobalFontInfo aInfo;
        GetTTGlobalFontInfo(pTTF, &aInfo);

        sal_uInt16 aGlyphIds[ 256 ] = {};
        sal_uInt8 aEncoding[ 256 ] = {};

        for (sal_uInt16 c = 32; c < 256; ++c)
        {
            aEncoding[c] = c;
            aGlyphIds[c] = c - 31;
        }

        std::vector<sal_uInt8> aBuffer;
        CreateTTFromTTGlyphs(pTTF, aBuffer, aGlyphIds, aEncoding, 256);


        // cleanup
        CloseTTFont( pTTF );
        // success
        nResult = 0;
    }

    return nResult;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
