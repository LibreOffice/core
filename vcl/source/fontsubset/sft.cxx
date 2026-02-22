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
#include <sft.hxx>
#include <font/TTFStructure.hxx>
#include <impfontcharmap.hxx>
#ifdef SYSTEM_LIBFIXMATH
#include <libfixmath/fix16.hpp>
#else
#include <tools/fix16.hxx>
#endif
#include <i18nlangtag/applelangid.hxx>
#include <i18nlangtag/languagetag.hxx>
#include <rtl/crc.h>
#include <rtl/ustring.hxx>
#include <rtl/ustrbuf.hxx>
#include <sal/log.hxx>
#include <tools/stream.hxx>
#include <o3tl/safeint.hxx>
#include <o3tl/string_view.hxx>
#include <osl/endian.h>
#include <osl/thread.h>
#include <tools/UnixWrappers.h>
#include <unotools/tempfile.hxx>
#include <fontsubset.hxx>

namespace vcl
{

namespace {

/*- In horizontal writing mode right sidebearing is calculated using this formula
 *- rsb = aw - (lsb + xMax - xMin) -*/

}

/*- Data access methods for data stored in big-endian format */
static sal_uInt16 GetUInt16(const sal_uInt8 *ptr, size_t offset)
{
    sal_uInt16 t;
    assert(ptr != nullptr);

    t = (ptr+offset)[0] << 8 | (ptr+offset)[1];

    return t;
}

static sal_Int16 GetInt16(const sal_uInt8* ptr, size_t offset)
{
    return static_cast<sal_Int16>(GetUInt16(ptr, offset));
}

static sal_uInt32 GetUInt32(const sal_uInt8 *ptr, size_t offset)
{
    sal_uInt32 t;
    assert(ptr != nullptr);

    t = (ptr+offset)[0] << 24 | (ptr+offset)[1] << 16 |
        (ptr+offset)[2] << 8  | (ptr+offset)[3];

    return t;
}

static sal_Int32 GetInt32(const sal_uInt8* ptr, size_t offset)
{
    return static_cast<sal_Int32>(GetUInt32(ptr, offset));
}


/*- Extracts a string from the name table and allocates memory for it -*/

static OString nameExtract( const sal_uInt8* name, int nTableSize, int n, int dbFlag, OUString* ucs2result )
{
    if( ucs2result )
        ucs2result->clear();

    const sal_uInt8* ptr = name + GetUInt16(name, 4) + GetUInt16(name + 6, 12 * n + 10);
    int len = GetUInt16(name+6, 12 * n + 8);

    // sanity check
    const sal_uInt8* end_table = name+nTableSize;
    const int available_space = ptr > end_table ? 0 : (end_table - ptr);
    if( (len <= 0) || len > available_space)
    {
        return OString();
    }

    OStringBuffer res;
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
        memcpy(res.appendUninitialized(len), ptr, len);
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
        fd = wrap_fopen(fname, "rb");

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

#if !defined(_WIN32) || defined(DO_USE_TTF_ON_WIN32)
SFErrCodes OpenTTFontFile(const char* fname, sal_uInt32 facenum, TrueTypeFont** ttf)
{
    SFErrCodes ret;
    int fd = -1;
    struct stat st;

    if (!fname || !*fname) return SFErrCodes::BadFile;

    *ttf = new TrueTypeFont(fname);
    if( ! *ttf )
        return SFErrCodes::Memory;

    if( (*ttf)->fileName().empty() )
    {
        ret = SFErrCodes::Memory;
        goto cleanup;
    }

#ifdef LINUX
    int nFD;
    int n;
    if (sscanf(fname, "/:FD:/%d%n", &nFD, &n) == 1 && fname[n] == '\0')
    {
        lseek(nFD, 0, SEEK_SET);
        fd = dup(nFD);
    }
    else
#endif
        fd = wrap_open(fname, O_RDONLY, 0);

    if (fd == -1) {
        ret = SFErrCodes::BadFile;
        goto cleanup;
    }

    if (wrap_fstat(fd, &st) == -1) {
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

    if (((*ttf)->ptr = static_cast<sal_uInt8 *>(wrap_mmap((*ttf)->fsize, fd, &(*ttf)->mmhandle))) == MAP_FAILED) {
        ret = SFErrCodes::Memory;
        goto cleanup;
    }

    ret = (*ttf)->open(facenum);

cleanup:
    if (fd != -1) wrap_close(fd);
    if (ret != SFErrCodes::Ok)
    {
        delete *ttf;
        *ttf = nullptr;
    }
    return ret;
}
#endif

SFErrCodes OpenTTFontBuffer(const void* pBuffer, sal_uInt32 nLen, sal_uInt32 facenum, TrueTypeFont** ttf)
{
    *ttf = new TrueTypeFont(nullptr);
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

AbstractTrueTypeFont::AbstractTrueTypeFont(const char* pFileName)
    : m_nGlyphs(0xFFFFFFFF)
    , m_bMicrosoftSymbolEncoded(false)
{
    if (pFileName)
        m_sFileName = pFileName;
}

AbstractTrueTypeFont::~AbstractTrueTypeFont()
{
}

TrueTypeFont::TrueTypeFont(const char* pFileName)
    : AbstractTrueTypeFont(pFileName)
    , fsize(-1)
    , mmhandle(0)
    , ptr(nullptr)
    , ntables(0)
{
}

TrueTypeFont::~TrueTypeFont()
{
#if !defined(_WIN32) || defined(DO_USE_TTF_ON_WIN32)
    if (!fileName().empty())
        wrap_munmap(ptr, fsize, mmhandle);
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

    sal_uInt32 unitsPerEm = GetUInt16(table, HEAD_unitsPerEm_offset);
    int indexfmt = GetInt16(table, HEAD_indexToLocFormat_offset);

    if (((indexfmt != 0) && (indexfmt != 1)) || (unitsPerEm <= 0))
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

    table = this->table(O_cmap, table_size);
    m_bMicrosoftSymbolEncoded = HasMicrosoftSymbolCmap(table, table_size);

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
            case T_cmap: nIndex = O_cmap; break;
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

void GetTTGlobalFontInfo(const AbstractTrueTypeFont *ttf, TTGlobalFontInfo *info)
{
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
        info->typeFlags = GetUInt16( table, OS2_fsType_offset );
    }

    table = ttf->table(O_post, table_size);
    if (table_size >= 12 + sizeof(sal_uInt32))
    {
        info->pitch  = GetUInt32(table, POST_isFixedPitch_offset);
        info->italicAngle = GetInt32(table, POST_italicAngle_offset);
    }

    table = ttf->table(O_head, table_size);
    if (table_size >= 46)
        info->macStyle = GetUInt16(table, HEAD_macStyle_offset);
}



/**
 * Extracts all Name Records from the font and stores them in an allocated
 * array of NameRecord structs
 */
static void GetTTNameRecords(AbstractTrueTypeFont const *ttf, std::vector<NameRecord>& nr)
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

static FontWeight ImplWeightToSal( int nWeight )
{
    if ( nWeight <= FW_THIN )
        return WEIGHT_THIN;
    else if ( nWeight <= FW_EXTRALIGHT )
        return WEIGHT_ULTRALIGHT;
    else if ( nWeight <= FW_LIGHT )
        return WEIGHT_LIGHT;
    else if ( nWeight < FW_MEDIUM )
        return WEIGHT_NORMAL;
    else if ( nWeight == FW_MEDIUM )
        return WEIGHT_MEDIUM;
    else if ( nWeight <= FW_SEMIBOLD )
        return WEIGHT_SEMIBOLD;
    else if ( nWeight <= FW_BOLD )
        return WEIGHT_BOLD;
    else if ( nWeight <= FW_EXTRABOLD )
        return WEIGHT_ULTRABOLD;
    else
        return WEIGHT_BLACK;
}

/*
 *  static helpers
 */
static sal_uInt16 getUInt16BE( const sal_uInt8*& pBuffer )
{
    sal_uInt16 nRet = static_cast<sal_uInt16>(pBuffer[1]) |
        (static_cast<sal_uInt16>(pBuffer[0]) << 8);
    pBuffer+=2;
    return nRet;
}

OUString convertSfntName( const NameRecord& rNameRecord )
{
    OUString aValue;
    if(
       ( rNameRecord.platformID == 3 && ( rNameRecord.encodingID == 0 || rNameRecord.encodingID == 1 ) )  // MS, Unicode
       ||
       ( rNameRecord.platformID == 0 ) // Apple, Unicode
       )
    {
        OUStringBuffer aName( rNameRecord.sptr.size()/2 );
        const sal_uInt8* pNameBuffer = rNameRecord.sptr.data();
        for(size_t n = 0; n < rNameRecord.sptr.size()/2; n++ )
            aName.append( static_cast<sal_Unicode>(getUInt16BE( pNameBuffer )) );
        aValue = aName.makeStringAndClear();
    }
    else if( rNameRecord.platformID == 3 )
    {
        if( rNameRecord.encodingID >= 2 && rNameRecord.encodingID <= 6 )
        {
            /*
             *  and now for a special kind of madness:
             *  some fonts encode their byte value string as BE uint16
             *  (leading to stray zero bytes in the string)
             *  while others code two bytes as a uint16 and swap to BE
             */
            OStringBuffer aName;
            const sal_uInt8* pNameBuffer = rNameRecord.sptr.data();
            for(size_t n = 0; n < rNameRecord.sptr.size()/2; n++ )
            {
                sal_Unicode aCode = static_cast<sal_Unicode>(getUInt16BE( pNameBuffer ));
                char aChar = aCode >> 8;
                if( aChar )
                    aName.append( aChar );
                aChar = aCode & 0x00ff;
                if( aChar )
                    aName.append( aChar );
            }
            switch( rNameRecord.encodingID )
            {
                case 2:
                    aValue = OStringToOUString( aName, RTL_TEXTENCODING_MS_932 );
                    break;
                case 3:
                    aValue = OStringToOUString( aName, RTL_TEXTENCODING_MS_936 );
                    break;
                case 4:
                    aValue = OStringToOUString( aName, RTL_TEXTENCODING_MS_950 );
                    break;
                case 5:
                    aValue = OStringToOUString( aName, RTL_TEXTENCODING_MS_949 );
                    break;
                case 6:
                    aValue = OStringToOUString( aName, RTL_TEXTENCODING_MS_1361 );
                    break;
            }
        }
    }
    else if( rNameRecord.platformID == 1 )
    {
        std::string_view aName(reinterpret_cast<const char*>(rNameRecord.sptr.data()), rNameRecord.sptr.size());
        rtl_TextEncoding eEncoding = RTL_TEXTENCODING_DONTKNOW;
        switch (rNameRecord.encodingID)
        {
            case 0:
                eEncoding = RTL_TEXTENCODING_APPLE_ROMAN;
                break;
            case 1:
                eEncoding = RTL_TEXTENCODING_APPLE_JAPANESE;
                break;
            case 2:
                eEncoding = RTL_TEXTENCODING_APPLE_CHINTRAD;
                break;
            case 3:
                eEncoding = RTL_TEXTENCODING_APPLE_KOREAN;
                break;
            case 4:
                eEncoding = RTL_TEXTENCODING_APPLE_ARABIC;
                break;
            case 5:
                eEncoding = RTL_TEXTENCODING_APPLE_HEBREW;
                break;
            case 6:
                eEncoding = RTL_TEXTENCODING_APPLE_GREEK;
                break;
            case 7:
                eEncoding = RTL_TEXTENCODING_APPLE_CYRILLIC;
                break;
            case 9:
                eEncoding = RTL_TEXTENCODING_APPLE_DEVANAGARI;
                break;
            case 10:
                eEncoding = RTL_TEXTENCODING_APPLE_GURMUKHI;
                break;
            case 11:
                eEncoding = RTL_TEXTENCODING_APPLE_GUJARATI;
                break;
            case 21:
                eEncoding = RTL_TEXTENCODING_APPLE_THAI;
                break;
            case 25:
                eEncoding = RTL_TEXTENCODING_APPLE_CHINSIMP;
                break;
            case 29:
                eEncoding = RTL_TEXTENCODING_APPLE_CENTEURO;
                break;
            case 32:    //Uninterpreted
                eEncoding = RTL_TEXTENCODING_UTF8;
                break;
            default:
                if (o3tl::starts_with(aName, "Khmer OS") || // encoding '20' (Khmer) isn't implemented
                    o3tl::starts_with(aName, "YoavKtav")) // tdf#152278
                {
                    eEncoding = RTL_TEXTENCODING_UTF8;
                }
                SAL_WARN_IF(eEncoding == RTL_TEXTENCODING_DONTKNOW, "vcl.fonts", "mac encoding " <<
                            rNameRecord.encodingID << " in font '" << aName << "'" <<
                            (rNameRecord.encodingID > 32 ? " is invalid" : " has unimplemented conversion"));
                break;
        }
        if (eEncoding != RTL_TEXTENCODING_DONTKNOW)
            aValue = OStringToOUString(aName, eEncoding);
    }

    return aValue;
}

OUString analyzeSfntName(const TrueTypeFont* pTTFont, sal_uInt16 nameId, const LanguageTag& rPrefLang)
{
    OUString aResult;

    std::vector<NameRecord> aNameRecords;
    GetTTNameRecords(pTTFont, aNameRecords);
    if( !aNameRecords.empty() )
    {
        LanguageType eLang = rPrefLang.getLanguageType();
        int nLastMatch = -1;
        for( size_t i = 0; i < aNameRecords.size(); i++ )
        {
            if( aNameRecords[i].nameID != nameId || aNameRecords[i].sptr.empty() )
                continue;
            int nMatch = -1;
            if( aNameRecords[i].platformID == 0 ) // Unicode
                nMatch = 4000;
            else if( aNameRecords[i].platformID == 3 )
            {
                // this bases on the LanguageType actually being a Win LCID
                if (aNameRecords[i].languageID == eLang)
                    nMatch = 8000;
                else if( aNameRecords[i].languageID == LANGUAGE_ENGLISH_US )
                    nMatch = 2000;
                else if( aNameRecords[i].languageID == LANGUAGE_ENGLISH ||
                         aNameRecords[i].languageID == LANGUAGE_ENGLISH_UK )
                    nMatch = 1500;
                else
                    nMatch = 1000;
            }
            else if (aNameRecords[i].platformID == 1)
            {
                AppleLanguageId aAppleId = static_cast<AppleLanguageId>(static_cast<sal_uInt16>(aNameRecords[i].languageID));
                LanguageTag aApple(makeLanguageTagFromAppleLanguageId(aAppleId));
                if (aApple == rPrefLang)
                    nMatch = 8000;
                else if (aAppleId == AppleLanguageId::ENGLISH)
                    nMatch = 2000;
                else
                    nMatch = 1000;
            }
            OUString aName = convertSfntName( aNameRecords[i] );
            if (!(aName.isEmpty()) && nMatch >= nLastMatch)
            {
                nLastMatch = nMatch;
                aResult = aName;
            }
        }
    }

    return aResult;
}

FontWeight AnalyzeTTFWeight(const TrueTypeFont* ttf)
{
    sal_uInt32 table_size;
    const sal_uInt8* table = ttf->table(O_OS2, table_size);
    if (table_size >= 42)
    {
        sal_uInt16 weightOS2 = GetUInt16(table, OS2_usWeightClass_offset);
        return ImplWeightToSal(weightOS2);
    }

    // Fallback to inferring from the style name (name ID 2).
    OUString sStyle = analyzeSfntName(ttf, 2, LanguageTag(LANGUAGE_ENGLISH_US));

    bool bBold(false), bItalic(false);
    if (o3tl::equalsIgnoreAsciiCase(sStyle, u"Regular"))
    {
        bBold = false;
        bItalic = false;
    }
    else if (o3tl::equalsIgnoreAsciiCase(sStyle, u"Bold"))
        bBold = true;
    else if (o3tl::equalsIgnoreAsciiCase(sStyle, u"Bold Italic"))
    {
        bBold = true;
        bItalic = true;
    }
    else if (o3tl::equalsIgnoreAsciiCase(sStyle, u"Italic"))
    {
        bItalic = true;
    }
    else
    {
        SAL_WARN("vcl.fonts", "Unhandled font style: " << sStyle);
    }

    (void)bItalic; // we might need to use this in a similar scenario where
                   // italic cannot be found

    return bBold ? WEIGHT_BOLD : WEIGHT_NORMAL;
}

} // namespace vcl


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
