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
 * TrueTypeCreator method implementation
 */

#include <iomanip>
#include <assert.h>

#include <sal/log.hxx>

#include "ttcr.hxx"
#include <string.h>

namespace vcl
{

/*
 * Private Data Types
 */

struct TableEntry {
    sal_uInt32 tag;
    sal_uInt32 length;
    sal_uInt8  *data;
};

/*- Data access macros for data stored in big-endian or little-endian format */
static sal_Int16 GetInt16( const sal_uInt8* ptr, sal_uInt32 offset)
{
    assert(ptr != nullptr);
    sal_Int16 t = (ptr+offset)[0] << 8 | (ptr+offset)[1];
    return t;
}

static sal_uInt16 GetUInt16( const sal_uInt8* ptr, sal_uInt32 offset)
{
    assert(ptr != nullptr);
    sal_uInt16 t = (ptr+offset)[0] << 8 | (ptr+offset)[1];
    return t;
}

static void PutInt16(sal_Int16 val, sal_uInt8 *ptr, sal_uInt32 offset)
{
    assert(ptr != nullptr);

    ptr[offset] = static_cast<sal_uInt8>((val >> 8) & 0xFF);
    ptr[offset+1] = static_cast<sal_uInt8>(val & 0xFF);
}

static void PutUInt16(sal_uInt16 val, sal_uInt8 *ptr, sal_uInt32 offset)
{
    assert(ptr != nullptr);

    ptr[offset] = static_cast<sal_uInt8>((val >> 8) & 0xFF);
    ptr[offset+1] = static_cast<sal_uInt8>(val & 0xFF);
}

static void PutUInt32(sal_uInt32 val, sal_uInt8 *ptr, sal_uInt32 offset)
{
    assert(ptr != nullptr);

    ptr[offset]   = static_cast<sal_uInt8>((val >> 24) & 0xFF);
    ptr[offset+1] = static_cast<sal_uInt8>((val >> 16) & 0xFF);
    ptr[offset+2] = static_cast<sal_uInt8>((val >> 8) & 0xFF);
    ptr[offset+3] = static_cast<sal_uInt8>(val & 0xFF);
}

static int TableEntryCompareF(const void *l, const void *r)
{
    sal_uInt32 const ltag(static_cast<TableEntry const*>(l)->tag);
    sal_uInt32 const rtag(static_cast<TableEntry const*>(r)->tag);
    return (ltag == rtag) ? 0 : (ltag < rtag) ? -1 : 1;
}

namespace {
struct NameRecordCompareF
{
    bool operator()(const NameRecord& l, const NameRecord& r) const
    {
        if (l.platformID != r.platformID) {
            return l.platformID < r.platformID;
        } else if (l.encodingID != r.encodingID) {
            return l.encodingID < r.encodingID;
        } else if (l.languageID != r.languageID) {
            return l.languageID < r.languageID;
        } else if (l.nameID != r.nameID) {
            return l.nameID < r.nameID;
        }
        return false;
    }
};
}

static sal_uInt32 CheckSum(sal_uInt32 *ptr, sal_uInt32 length)
{
    sal_uInt32 sum = 0;
    sal_uInt32 *endptr = ptr + ((length + 3) & sal_uInt32(~3)) / 4;

    while (ptr < endptr) sum += *ptr++;

    return sum;
}

/*
 * Public functions
 */

TrueTypeCreator::TrueTypeCreator(sal_uInt32 _tag)
{
    this->m_tag = _tag;
}

void TrueTypeCreator::AddTable(std::unique_ptr<TrueTypeTable> table)
{
    if (table != nullptr) {
        this->m_tables.push_back(std::move(table));
    }
}

void TrueTypeCreator::RemoveTable(sal_uInt32 tableTag)
{
    for (auto it = this->m_tables.begin(); it != this->m_tables.end(); )
    {
        if ((*it)->m_tag == tableTag)
        {
            it = this->m_tables.erase(it);
        }
        else
            ++it;
    }
}

SFErrCodes TrueTypeCreator::StreamToMemory(std::vector<sal_uInt8>& rOutBuffer)
{
    sal_uInt16 searchRange=1, entrySelector=0, rangeShift;
    sal_uInt32 s, offset, checkSumAdjustment = 0;
    sal_uInt32 *p;
    sal_uInt8 *head = nullptr;     /* saved pointer to the head table data for checkSumAdjustment calculation */

    if (this->m_tables.empty())
        return SFErrCodes::TtFormat;

    ProcessTables();

    /* ProcessTables() adds 'loca' and 'hmtx' */

    sal_uInt16 numTables = this->m_tables.size();

    std::unique_ptr<TableEntry[]> te(new TableEntry[numTables]);

    int teIdx = 0;
    for (auto const & e : this->m_tables)
    {
        e->GetRawData(&te[teIdx]);
        ++teIdx;
    }

    qsort(te.get(), numTables, sizeof(TableEntry), TableEntryCompareF);

    do {
        searchRange *= 2;
        entrySelector++;
    } while (searchRange <= numTables);

    searchRange *= 8;
    entrySelector--;
    rangeShift = numTables * 16 - searchRange;

    s = offset = 12 + 16 * numTables;

    for (int i = 0; i < numTables; ++i) {
        s += (te[i].length + 3) & sal_uInt32(~3);
        /* if ((te[i].length & 3) != 0) s += (4 - (te[i].length & 3)) & 3; */
    }

    rOutBuffer.resize(s);
    sal_uInt8* ttf = rOutBuffer.data();

    /* Offset Table */
    PutUInt32(this->m_tag, ttf, 0);
    PutUInt16(numTables, ttf, 4);
    PutUInt16(searchRange, ttf, 6);
    PutUInt16(entrySelector, ttf, 8);
    PutUInt16(rangeShift, ttf, 10);

    /* Table Directory */
    for (int i = 0; i < numTables; ++i) {
        PutUInt32(te[i].tag, ttf + 12, 16 * i);
        PutUInt32(CheckSum(reinterpret_cast<sal_uInt32 *>(te[i].data), te[i].length), ttf + 12, 16 * i + 4);
        PutUInt32(offset, ttf + 12, 16 * i + 8);
        PutUInt32(te[i].length, ttf + 12, 16 * i + 12);

        if (te[i].tag == T_head) {
            head = ttf + offset;
        }

        memcpy(ttf+offset, te[i].data, (te[i].length + 3) & sal_uInt32(~3) );
        offset += (te[i].length + 3) & sal_uInt32(~3);
        /* if ((te[i].length & 3) != 0) offset += (4 - (te[i].length & 3)) & 3; */
    }

    te.reset();

    p = reinterpret_cast<sal_uInt32 *>(ttf);
    for (int i = 0; i < static_cast<int>(s) / 4; ++i) checkSumAdjustment += p[i];
    PutUInt32(0xB1B0AFBA - checkSumAdjustment, head, 8);

    return SFErrCodes::Ok;
}

/*
 * TrueTypeTable private methods
 */

/*    Table         data points to
 * --------------------------------------------
 *    generic       tdata_generic struct
 *    'head'        HEAD_Length bytes of memory
 *    'hhea'        HHEA_Length bytes of memory
 *    'loca'        tdata_loca struct
 *    'maxp'        MAXP_Version1Length bytes of memory
 *    'glyf'        list of GlyphData structs (defined in sft.h)
 *    'name'        list of NameRecord structs (defined in sft.h)
 *    'post'        tdata_post struct
 *
 */

#define CMAP_SUBTABLE_INIT 10
#define CMAP_SUBTABLE_INCR 10

namespace {

struct CmapSubTable {
    sal_uInt32  id;                         /* subtable ID (platform/encoding ID)    */
    std::vector<std::pair<sal_uInt32, sal_uInt32>> mappings;  /* character to glyph mapping array */
};

}

struct table_cmap {
    sal_uInt32 n;                           /* number of used CMAP sub-tables       */
    sal_uInt32 m;                           /* number of allocated CMAP sub-tables  */
    std::unique_ptr<CmapSubTable[]> s;      /* sorted array of sub-tables           */
};

struct tdata_loca {
    sal_uInt32 nbytes;                      /* number of bytes in loca table */
    std::unique_ptr<sal_uInt8[]> ptr;       /* pointer to the data */
};

/* allocate memory for a TT table */
static std::unique_ptr<sal_uInt8[]> ttmalloc(sal_uInt32 nbytes)
{
    sal_uInt32 n = (nbytes + 3) & sal_uInt32(~3);
    return std::make_unique<sal_uInt8[]>(n);
}

TrueTypeTable::~TrueTypeTable() {}

TrueTypeTableGeneric::~TrueTypeTableGeneric()
{
}

TrueTypeTableHead::~TrueTypeTableHead()
{
}

TrueTypeTableHhea::~TrueTypeTableHhea()
{
}

TrueTypeTableLoca::~TrueTypeTableLoca()
{
}

TrueTypeTableMaxp::~TrueTypeTableMaxp()
{
}

TrueTypeTableGlyf::~TrueTypeTableGlyf()
{
}

TrueTypeTableCmap::~TrueTypeTableCmap()
{
}

TrueTypeTableName::~TrueTypeTableName()
{
}

TrueTypeTablePost::~TrueTypeTablePost()
{
    if (m_format == 0x00030000) {
        /* do nothing */
    } else {
        SAL_WARN("vcl.fonts", "Unsupported format of a 'post' table: "
                << std::setfill('0')
                << std::setw(8)
                << std::hex
                << std::uppercase
                << static_cast<int>(m_format) << ".");
    }
}


int TrueTypeTableGeneric::GetRawData(TableEntry* te)
{
    te->data = this->m_ptr.get();
    te->length = this->m_nbytes;
    te->tag = this->m_tag;

    return TTCR_OK;
}

int TrueTypeTableHead::GetRawData(TableEntry* te)
{
    te->length = HEAD_Length;
    te->data = this->m_head.get();
    te->tag = T_head;

    return TTCR_OK;
}

int TrueTypeTableHhea::GetRawData(TableEntry* te)
{
    te->length = HHEA_Length;
    te->data = this->m_hhea.get();
    te->tag = T_hhea;

    return TTCR_OK;
}

int TrueTypeTableLoca::GetRawData(TableEntry* te)
{
    assert(this->m_loca != nullptr);

    if (m_loca->nbytes == 0) return TTCR_ZEROGLYPHS;

    te->data = m_loca->ptr.get();
    te->length = m_loca->nbytes;
    te->tag = T_loca;

    return TTCR_OK;
}

int TrueTypeTableMaxp::GetRawData(TableEntry* te)
{
    te->length = MAXP_Version1Length;
    te->data = this->m_maxp.get();
    te->tag = T_maxp;

    return TTCR_OK;
}

int TrueTypeTableGlyf::GetRawData(TableEntry* te)
{
    sal_uInt32 n, nbytes = 0;
    /* sal_uInt16 curID = 0;    */               /* to check if glyph IDs are sequential and start from zero */

    te->data = nullptr;
    te->length = 0;
    te->tag = 0;

    if (m_list.empty()) return TTCR_ZEROGLYPHS;

    for (const std::unique_ptr<GlyphData>& pGlyph : m_list)
    {
        /* if (((GlyphData *) listCurrent(l))->glyphID != curID++) return TTCR_GLYPHSEQ; */
        nbytes += pGlyph->nbytes;
    }

    m_rawdata = ttmalloc(nbytes);

    auto p = m_rawdata.get();
    for (const std::unique_ptr<GlyphData>& pGlyph : m_list)
    {
        n = pGlyph->nbytes;
        if (n != 0) {
            memcpy(p, pGlyph->ptr.get(), n);
            p += n;
        }
    }

    te->length = nbytes;
    te->data = m_rawdata.get();
    te->tag = T_glyf;

    return TTCR_OK;
}

/* cmap packers */
static std::unique_ptr<sal_uInt8[]> PackCmapType0(CmapSubTable const *s, sal_uInt32 *length)
{
    std::unique_ptr<sal_uInt8[]> ptr(new sal_uInt8[262]);
    sal_uInt8 *p = ptr.get() + 6;

    PutUInt16(0, ptr.get(), 0);
    PutUInt16(262, ptr.get(), 2);
    PutUInt16(0, ptr.get(), 4);

    for (sal_uInt32 i = 0; i < 256; i++) {
        sal_uInt16 g = 0;
        for (const auto& [ch, glyph] : s->mappings) {
            if (ch == i) {
                g = static_cast<sal_uInt16>(glyph);
            }
        }
        p[i] = static_cast<sal_uInt8>(g);
    }
    *length = 262;
    return ptr;
}

static std::unique_ptr<sal_uInt8[]> PackCmapType6(CmapSubTable const *s, sal_uInt32 *length)
{
    std::unique_ptr<sal_uInt8[]> ptr(new sal_uInt8[s->mappings.size()*2 + 10]);
    sal_uInt8 *p = ptr.get() + 10;

    PutUInt16(6, ptr.get(), 0);
    PutUInt16(static_cast<sal_uInt16>(s->mappings.size()*2+10), ptr.get(), 2);
    PutUInt16(0, ptr.get(), 4);
    PutUInt16(0, ptr.get(), 6);
    PutUInt16(static_cast<sal_uInt16>(s->mappings.size()), ptr.get(), 8 );

    for (size_t i = 0; i < s->mappings.size(); i++) {
        sal_uInt16 g = 0;
        for (const auto& [ch, glyph] : s->mappings) {
            if (ch == i) {
                g = static_cast<sal_uInt16>(glyph);
            }
        }
        PutUInt16( g, p, 2*i );
    }
    *length = s->mappings.size()*2+10;
    return ptr;
}

/* XXX it only handles Format 0 encoding tables */
static std::unique_ptr<sal_uInt8[]> PackCmap(CmapSubTable const *s, sal_uInt32 *length)
{
    if (s->mappings.back().second > 0xff)
        return PackCmapType6(s, length);
    else
        return PackCmapType0(s, length);
}

int TrueTypeTableCmap::GetRawData(TableEntry* te)
{
    sal_uInt32 i;
    sal_uInt32 tlen = 0;
    sal_uInt32 l;
    sal_uInt32 cmapsize;
    sal_uInt8 *cmap;
    sal_uInt32 coffset;

    assert(m_cmap);
    assert(m_cmap->n != 0);

    std::unique_ptr<std::unique_ptr<sal_uInt8[]>[]> subtables(new std::unique_ptr<sal_uInt8[]>[m_cmap->n]);
    std::unique_ptr<sal_uInt32[]> sizes(new sal_uInt32[m_cmap->n]);

    for (i = 0; i < m_cmap->n; i++) {
        subtables[i] = PackCmap(m_cmap->s.get()+i, &l);
        sizes[i] = l;
        tlen += l;
    }

    cmapsize = tlen + 4 + 8 * m_cmap->n;
    this->m_rawdata = ttmalloc(cmapsize);
    cmap = this->m_rawdata.get();

    PutUInt16(0, cmap, 0);
    PutUInt16(static_cast<sal_uInt16>(m_cmap->n), cmap, 2);
    coffset = 4 + m_cmap->n * 8;

    for (i = 0; i < m_cmap->n; i++) {
        PutUInt16(static_cast<sal_uInt16>(m_cmap->s[i].id >> 16), cmap + 4, i * 8);
        PutUInt16(static_cast<sal_uInt16>(m_cmap->s[i].id & 0xFF), cmap + 4, 2 + i * 8);
        PutUInt32(coffset, cmap + 4, 4 + i * 8);
        memcpy(cmap + coffset, subtables[i].get(), sizes[i]);
        subtables[i].reset();
        coffset += sizes[i];
    }

    subtables.reset();
    sizes.reset();

    te->data = cmap;
    te->length = cmapsize;
    te->tag = T_cmap;

    return TTCR_OK;
}

int TrueTypeTableName::GetRawData(TableEntry* te)
{
    sal_Int16 i=0, n;                          /* number of Name Records */
    int stringLen = 0;
    sal_uInt8 *p1, *p2;

    te->data = nullptr;
    te->length = 0;
    te->tag = 0;

    if ((n = static_cast<sal_Int16>(m_list.size())) == 0) return TTCR_NONAMES;

    std::vector<NameRecord> nr = m_list;

    for (const NameRecord & rName : m_list)
        stringLen += rName.sptr.size();

    if (stringLen > 65535) {
        return TTCR_NAMETOOLONG;
    }

    std::sort(nr.begin(), nr.end(), NameRecordCompareF());

    int nameLen = stringLen + 12 * n + 6;
    std::unique_ptr<sal_uInt8[]> name = ttmalloc(nameLen);

    PutUInt16(0, name.get(), 0);
    PutUInt16(n, name.get(), 2);
    PutUInt16(static_cast<sal_uInt16>(6 + 12 * n), name.get(), 4);

    p1 = name.get() + 6;
    p2 = p1 + 12 * n;

    for (i = 0; i < n; i++) {
        PutUInt16(nr[i].platformID, p1, 0);
        PutUInt16(nr[i].encodingID, p1, 2);
        PutUInt16(static_cast<sal_uInt16>(nr[i].languageID), p1, 4);
        PutUInt16(nr[i].nameID, p1, 6);
        PutUInt16(nr[i].sptr.size(), p1, 8);
        PutUInt16(static_cast<sal_uInt16>(p2 - (name.get() + 6 + 12 * n)), p1, 10);
        if (nr[i].sptr.size()) {
            memcpy(p2, nr[i].sptr.data(), nr[i].sptr.size());
        }
        /* {int j; for(j=0; j<nr[i].slen; j++) printf("%c", nr[i].sptr[j]); printf("\n"); }; */
        p2 += nr[i].sptr.size();
        p1 += 12;
    }

    nr.clear();
    this->m_rawdata = std::move(name);

    te->data = this->m_rawdata.get();
    te->length = static_cast<sal_uInt16>(nameLen);
    te->tag = T_name;

    /*{int j; for(j=0; j<nameLen; j++) printf("%c", name[j]); }; */

    return TTCR_OK;
}

int TrueTypeTablePost::GetRawData(TableEntry* te)
{
    std::unique_ptr<sal_uInt8[]> post;
    sal_uInt32 postLen = 0;
    int ret;

    this->m_rawdata.reset();

    if (m_format == 0x00030000) {
        postLen = 32;
        post = ttmalloc(postLen);
        PutUInt32(0x00030000, post.get(), 0);
        PutUInt32(m_italicAngle, post.get(), 4);
        PutUInt16(m_underlinePosition, post.get(), 8);
        PutUInt16(m_underlineThickness, post.get(), 10);
        PutUInt16(static_cast<sal_uInt16>(m_isFixedPitch), post.get(), 12);
        ret = TTCR_OK;
    } else {
        SAL_WARN("vcl.fonts", "Unrecognized format of a post table: "
                << std::setfill('0')
                << std::setw(8)
                << std::hex
                << std::uppercase
                << static_cast<int>(m_format) << ".");
        ret = TTCR_POSTFORMAT;
    }

    this->m_rawdata = std::move(post);
    te->data = this->m_rawdata.get();
    te->length = postLen;
    te->tag = T_post;

    return ret;
}

/*
 * TrueTypeTable public methods
 */

/* Note: Type42 fonts only need these tables:
 *        head, hhea, loca, maxp, cvt, prep, glyf, hmtx, fpgm
 *
 * Microsoft required tables
 *        cmap, glyf, head, hhea, hmtx, loca, maxp, name, post, OS/2
 *
 * Apple required tables
 *        cmap, glyf, head, hhea, hmtx, loca, maxp, name, post
 *
 */

TrueTypeTableGeneric::TrueTypeTableGeneric(sal_uInt32 tag,
                                sal_uInt32 nbytes,
                                const sal_uInt8* ptr)
    : TrueTypeTable(tag),
    m_nbytes(nbytes)
{
    if (nbytes) {
        m_ptr = ttmalloc(nbytes);
        memcpy(m_ptr.get(), ptr, nbytes);
    }
}

TrueTypeTableGeneric::TrueTypeTableGeneric(sal_uInt32 tag,
                                sal_uInt32 nbytes,
                                std::unique_ptr<sal_uInt8[]> ptr)
    : TrueTypeTable(tag),
    m_nbytes(nbytes)
{
    if (nbytes) {
        m_ptr = std::move(ptr);
    }
}

TrueTypeTableHead::TrueTypeTableHead(sal_uInt32 fontRevision,
                                     sal_uInt16 flags,
                                     sal_uInt16 unitsPerEm,
                                     const sal_uInt8* created,
                                     sal_uInt16 macStyle,
                                     sal_uInt16 lowestRecPPEM,
                                     sal_Int16  fontDirectionHint)
    : TrueTypeTable(T_head)
    , m_head(ttmalloc(HEAD_Length))
{
    assert(created != nullptr);

    sal_uInt8* ptr = m_head.get();

    PutUInt32(0x00010000, ptr, 0);             /* version */
    PutUInt32(fontRevision, ptr, 4);
    PutUInt32(0x5F0F3CF5, ptr, 12);            /* magic number */
    PutUInt16(flags, ptr, 16);
    PutUInt16(unitsPerEm, ptr, 18);
    memcpy(ptr+20, created, 8);                   /* Created Long Date */
    memset(ptr+28, 0, 8);                         /* Modified Long Date */
    PutUInt16(macStyle, ptr, 44);
    PutUInt16(lowestRecPPEM, ptr, 46);
    PutUInt16(fontDirectionHint, ptr, 48);
    PutUInt16(0, ptr, 52);                     /* glyph data format: 0 */
}

TrueTypeTableHhea::TrueTypeTableHhea(sal_Int16  ascender,
                                     sal_Int16  descender,
                                     sal_Int16  linegap,
                                     sal_Int16  caretSlopeRise,
                                     sal_Int16  caretSlopeRun)
    : TrueTypeTable(T_hhea),
    m_hhea(ttmalloc(HHEA_Length))
{
    sal_uInt8* ptr = m_hhea.get();

    PutUInt32(0x00010000, ptr, 0);             /* version */
    PutUInt16(ascender, ptr, 4);
    PutUInt16(descender, ptr, 6);
    PutUInt16(linegap, ptr, 8);
    PutUInt16(caretSlopeRise, ptr, 18);
    PutUInt16(caretSlopeRun, ptr, 20);
    PutUInt16(0, ptr, 22);                     /* reserved 1 */
    PutUInt16(0, ptr, 24);                     /* reserved 2 */
    PutUInt16(0, ptr, 26);                     /* reserved 3 */
    PutUInt16(0, ptr, 28);                     /* reserved 4 */
    PutUInt16(0, ptr, 30);                     /* reserved 5 */
    PutUInt16(0, ptr, 32);                     /* metricDataFormat */
}

TrueTypeTableLoca::TrueTypeTableLoca()
    : TrueTypeTable(T_loca),
    m_loca(new tdata_loca)
{
    this->m_loca->nbytes = 0;
    this->m_loca->ptr = nullptr;
}

TrueTypeTableMaxp::TrueTypeTableMaxp( const sal_uInt8* maxp, int size)
    : TrueTypeTable(T_maxp)
{
    this->m_maxp = ttmalloc(MAXP_Version1Length);

    if (maxp && size == MAXP_Version1Length) {
        memcpy(this->m_maxp.get(), maxp, MAXP_Version1Length);
    }
}

TrueTypeTableGlyf::TrueTypeTableGlyf()
    : TrueTypeTable(T_glyf)
{
}

TrueTypeTableCmap::TrueTypeTableCmap()
    : TrueTypeTable(T_cmap)
    , m_cmap(new table_cmap)
{
    m_cmap->n = 0;
    m_cmap->m = CMAP_SUBTABLE_INIT;
    m_cmap->s.reset(new CmapSubTable[CMAP_SUBTABLE_INIT]);
}

TrueTypeTableName::TrueTypeTableName(std::vector<NameRecord> nr)
    : TrueTypeTable(T_name)
    , m_list(std::move(nr))
{
}

TrueTypeTablePost::TrueTypeTablePost(sal_Int32 format,
                                     sal_Int32 italicAngle,
                                     sal_Int16 underlinePosition,
                                     sal_Int16 underlineThickness,
                                     sal_uInt32 isFixedPitch)
    : TrueTypeTable(T_post)
{
    assert(format == 0x00030000);                 /* Only format 3.0 is supported at this time */

    m_format = format;
    m_italicAngle = italicAngle;
    m_underlinePosition = underlinePosition;
    m_underlineThickness = underlineThickness;
    m_isFixedPitch = isFixedPitch;
}

void TrueTypeTableCmap::cmapAdd(sal_uInt32 id, sal_uInt32 c, sal_uInt32 g)
{
    sal_uInt32 i, found;
    CmapSubTable *s;

    assert(m_cmap);
    s = m_cmap->s.get(); assert(s != nullptr);

    found = 0;

    for (i = 0; i < m_cmap->n; i++) {
        if (s[i].id == id) {
            found = 1;
            break;
        }
    }

    if (!found) {
        if (m_cmap->n == m_cmap->m) {
            std::unique_ptr<CmapSubTable[]> tmp(new CmapSubTable[m_cmap->m + CMAP_SUBTABLE_INCR]);
            for (sal_uInt32 j = 0; j != m_cmap->m; ++j) {
                tmp[j] = std::move(s[j]);
            }
            m_cmap->m += CMAP_SUBTABLE_INCR;
            s = tmp.get();
            m_cmap->s = std::move(tmp);
        }

        for (i = 0; i < m_cmap->n; i++) {
            if (s[i].id > id) break;
        }

        if (i < m_cmap->n) {
            for (sal_uInt32 j = m_cmap->n; j != i; --j) {
                s[j + 1] = std::move(s[j]);
            }
        }

        m_cmap->n++;

        s[i].id = id;
    }

    s[i].mappings.emplace_back(c, g);
}

sal_uInt32 TrueTypeTableGlyf::glyfAdd(std::unique_ptr<GlyphData> glyphdata, AbstractTrueTypeFont *fnt)
{
    sal_uInt32 currentID;
    int ret, n, ncomponents;

    if (!glyphdata) return sal_uInt32(~0);

    std::vector<sal_uInt32> glyphlist;
    std::vector<sal_uInt32> currentGlyphStack;

    ncomponents = GetTTGlyphComponents(fnt, glyphdata->glyphID, glyphlist, currentGlyphStack);

    if (m_list.size() > 0) {
        ret = n = m_list.back()->newID + 1;
    } else {
        ret = n = 0;
    }
    glyphdata->newID = n++;
    m_list.push_back(std::move(glyphdata));

    if (ncomponents > 1 && glyphlist.size() > 1 )
    {
        std::vector< sal_uInt32 >::const_iterator it = glyphlist.begin();
        ++it;
        /* glyphData->glyphID is always the first glyph on the list */
        do
        {
            int found = 0;
            currentID = *it;
            /* XXX expensive! should be rewritten with sorted arrays! */
            for (const std::unique_ptr<GlyphData>& pGlyph : m_list)
            {
                if (pGlyph->glyphID == currentID) {
                    found = 1;
                    break;
                }
            }

            if (!found) {
                std::unique_ptr<GlyphData> gd = GetTTRawGlyphData(fnt, currentID);
                gd->newID = n++;
                m_list.push_back(std::move(gd));
            }
        } while( ++it != glyphlist.end() );
    }

    return ret;
}

TrueTypeTable *TrueTypeCreator::FindTable(sal_uInt32 tableTag)
{
    for (const std::unique_ptr<TrueTypeTable>& p : this->m_tables)
        if (p->m_tag == tableTag) {
            return p.get();
        }

    return nullptr;
}

/* This function processes all the tables and synchronizes them before creating
 * the output TrueType stream.
 *
 * *** It adds two TrueType tables to the font: 'loca' and 'hmtx' ***
 *
 * It does:
 *
 * - Re-numbers glyph IDs and creates 'glyf', 'loca', and 'hmtx' tables.
 * - Calculates xMin, yMin, xMax, and yMax and stores values in 'head' table.
 * - Stores indexToLocFormat in 'head'
 * - updates 'maxp' table
 * - Calculates advanceWidthMax, minLSB, minRSB, xMaxExtent and numberOfHMetrics
 *   in 'hhea' table
 *
 */
void TrueTypeCreator::ProcessTables()
{
    TrueTypeTableHhea *hhea = nullptr;
    TrueTypeTableMaxp *maxp = nullptr;
    TrueTypeTableHead *head = nullptr;
    std::unique_ptr<TrueTypeTableLoca> loca;
    TrueTypeTableGlyf *glyf = nullptr;
    sal_uInt32 nGlyphs, locaLen = 0, glyfLen = 0;
    sal_Int16 xMin = 0, yMin = 0, xMax = 0, yMax = 0;
    sal_uInt32 i = 0;
    sal_Int16 indexToLocFormat;
    std::unique_ptr<sal_uInt8[]> hmtxPtr;
    sal_uInt8 *hheaPtr;
    sal_uInt32 hmtxSize;
    sal_uInt8 *p1, *p2;
    sal_uInt16 maxPoints = 0, maxContours = 0, maxCompositePoints = 0, maxCompositeContours = 0;
    int nlsb = 0;
    std::unique_ptr<sal_uInt32[]> gid;   /* array of old glyphIDs */

    glyf = static_cast<TrueTypeTableGlyf*>(FindTable(T_glyf));
    std::vector<std::unique_ptr<GlyphData>>& glyphlist = glyf->m_list;
    nGlyphs = glyphlist.size();
    if (!nGlyphs)
    {
        SAL_WARN("vcl.fonts", "no glyphs found in ProcessTables");
        return;
    }
    gid.reset(new sal_uInt32[nGlyphs]);

    RemoveTable(T_loca);
    RemoveTable(T_hmtx);

    /* XXX Need to make sure that composite glyphs do not break during glyph renumbering */

    for (const std::unique_ptr<GlyphData>& gd : glyphlist)
    {
        glyfLen += gd->nbytes;
        /* XXX if (gd->nbytes & 1) glyfLen++; */

        assert(gd->newID == i);
        gid[i++] = gd->glyphID;
        /* gd->glyphID = i++; */

        /* printf("IDs: %d %d.\n", gd->glyphID, gd->newID); */

        if (gd->nbytes >= 10) {
            sal_Int16 z = GetInt16(gd->ptr.get(), 2);
            if (z < xMin) xMin = z;

            z = GetInt16(gd->ptr.get(), 4);
            if (z < yMin) yMin = z;

            z = GetInt16(gd->ptr.get(), 6);
            if (z > xMax) xMax = z;

            z = GetInt16(gd->ptr.get(), 8);
            if (z > yMax) yMax = z;
        }

        if (!gd->compflag) {                                /* non-composite glyph */
            if (gd->npoints > maxPoints) maxPoints = gd->npoints;
            if (gd->ncontours > maxContours) maxContours = gd->ncontours;
        } else {                                            /* composite glyph */
            if (gd->npoints > maxCompositePoints) maxCompositePoints = gd->npoints;
            if (gd->ncontours > maxCompositeContours) maxCompositeContours = gd->ncontours;
        }

    }

    indexToLocFormat = (glyfLen / 2 > 0xFFFF) ? 1 : 0;
    locaLen = indexToLocFormat ?  (nGlyphs + 1) << 2 : (nGlyphs + 1) << 1;

    std::unique_ptr<sal_uInt8[]> glyfPtr = ttmalloc(glyfLen);
    std::unique_ptr<sal_uInt8[]> locaPtr = ttmalloc(locaLen);
    std::unique_ptr<TTSimpleGlyphMetrics[]> met(new TTSimpleGlyphMetrics[nGlyphs]);
    i = 0;

    p1 = glyfPtr.get();
    p2 = locaPtr.get();
    for (const std::unique_ptr<GlyphData>& gd : glyphlist)
    {
        if (gd->compflag && gd->nbytes > 10) {    /* re-number all components */
            sal_uInt16 flags, index;
            sal_uInt8 *ptr = gd->ptr.get() + 10;
            size_t nRemaining = gd->nbytes - 10;
            do {
                if (nRemaining < 4)
                {
                    SAL_WARN("vcl.fonts", "truncated font");
                    break;
                }
                flags = GetUInt16(ptr, 0);
                index = GetUInt16(ptr, 2);

                /* XXX use the sorted array of old to new glyphID mapping and do a binary search */
                sal_uInt32 j;
                for (j = 0; j < nGlyphs; j++) {
                    if (gid[j] == index) {
                        break;
                    }
                }
                /* printf("X: %d -> %d.\n", index, j); */

                PutUInt16(static_cast<sal_uInt16>(j), ptr, 2);

                ptr += 4;
                nRemaining -= 4;

                sal_uInt32 nAdvance = 0;
                if (flags & ARG_1_AND_2_ARE_WORDS) {
                    nAdvance += 4;
                } else {
                    nAdvance += 2;
                }

                if (flags & WE_HAVE_A_SCALE) {
                    nAdvance += 2;
                } else if (flags & WE_HAVE_AN_X_AND_Y_SCALE) {
                    nAdvance += 4;
                } else if (flags & WE_HAVE_A_TWO_BY_TWO) {
                    nAdvance += 8;
                }

                if (nRemaining < nAdvance)
                {
                    SAL_WARN("vcl.fonts", "truncated font");
                    break;
                }

                ptr += nAdvance;
                nRemaining -= nAdvance;

            } while (flags & MORE_COMPONENTS);
        }

        if (gd->nbytes != 0) {
            memcpy(p1, gd->ptr.get(), gd->nbytes);
        }
        if (indexToLocFormat == 1) {
            PutUInt32(p1 - glyfPtr.get(), p2, 0);
            p2 += 4;
        } else {
            PutUInt16(static_cast<sal_uInt16>((p1 - glyfPtr.get()) >> 1), p2, 0);
            p2 += 2;
        }
        p1 += gd->nbytes;

        /* fill the array of metrics */
        met[i].adv = gd->aw;
        met[i].sb  = gd->lsb;
        i++;
    }

    gid.reset();

    if (indexToLocFormat == 1) {
        PutUInt32(p1 - glyfPtr.get(), p2, 0);
    } else {
        PutUInt16(static_cast<sal_uInt16>((p1 - glyfPtr.get()) >> 1), p2, 0);
    }

    glyf->m_rawdata = std::move(glyfPtr);

    loca.reset(new TrueTypeTableLoca());
    loca->m_loca->ptr = std::move(locaPtr);
    loca->m_loca->nbytes = locaLen;

    AddTable(std::move(loca));

    head = static_cast<TrueTypeTableHead*>(FindTable(T_head));
    sal_uInt8* const pHeadData = head->m_head.get();
    PutInt16(xMin, pHeadData, HEAD_xMin_offset);
    PutInt16(yMin, pHeadData, HEAD_yMin_offset);
    PutInt16(xMax, pHeadData, HEAD_xMax_offset);
    PutInt16(yMax, pHeadData, HEAD_yMax_offset);
    PutInt16(indexToLocFormat, pHeadData, HEAD_indexToLocFormat_offset);

    maxp = static_cast<TrueTypeTableMaxp*>(FindTable(T_maxp));

    sal_uInt8* const pMaxpData = maxp->m_maxp.get();
    PutUInt16(static_cast<sal_uInt16>(nGlyphs), pMaxpData, MAXP_numGlyphs_offset);
    PutUInt16(maxPoints, pMaxpData, MAXP_maxPoints_offset);
    PutUInt16(maxContours, pMaxpData, MAXP_maxContours_offset);
    PutUInt16(maxCompositePoints, pMaxpData, MAXP_maxCompositePoints_offset);
    PutUInt16(maxCompositeContours, pMaxpData, MAXP_maxCompositeContours_offset);

    /*
     * Generate an htmx table and update hhea table
     */
    hhea = static_cast<TrueTypeTableHhea*>(FindTable(T_hhea)); assert(hhea != nullptr);
    hheaPtr = hhea->m_hhea.get();
    if (nGlyphs > 2) {
        for (i = nGlyphs - 1; i > 0; i--) {
            if (met[i].adv != met[i-1].adv) break;
        }
        nlsb = nGlyphs - 1 - i;
    }
    hmtxSize = (nGlyphs - nlsb) * 4 + nlsb * 2;
    hmtxPtr = ttmalloc(hmtxSize);
    p1 = hmtxPtr.get();

    for (i = 0; i < nGlyphs; i++) {
        if (i < nGlyphs - nlsb) {
            PutUInt16(met[i].adv, p1, 0);
            PutUInt16(met[i].sb, p1, 2);
            p1 += 4;
        } else {
            PutUInt16(met[i].sb, p1, 0);
            p1 += 2;
        }
    }

    AddTable(std::make_unique<TrueTypeTableGeneric>(T_hmtx, hmtxSize, std::move(hmtxPtr)));
    PutUInt16(static_cast<sal_uInt16>(nGlyphs - nlsb), hheaPtr, 34);
}

/**
 * TrueTypeCreator destructor. It calls destructors for all TrueTypeTables added to it.
 */
TrueTypeCreator::~TrueTypeCreator()
{
}

} // namespace vcl

#ifdef TEST_TTCR
static sal_uInt32 mkTag(sal_uInt8 a, sal_uInt8 b, sal_uInt8 c, sal_uInt8 d) {
    return (a << 24) | (b << 16) | (c << 8) | d;
}

int main()
{
    TrueTypeCreator *ttcr;
    sal_uInt8 *t1, *t2, *t3, *t4, *t5, *t6;

    TrueTypeCreatorNewEmpty(mkTag('t','r','u','e'), &ttcr);

    t1 = malloc(1000); memset(t1, 'a', 1000);
    t2 = malloc(2000); memset(t2, 'b', 2000);
    t3 = malloc(3000); memset(t3, 'c', 3000);
    t4 = malloc(4000); memset(t4, 'd', 4000);
    t5 = malloc(5000); memset(t5, 'e', 5000);
    t6 = malloc(6000); memset(t6, 'f', 6000);

    AddTable(ttcr, TrueTypeTableNew(T_maxp, 1000, t1));
    AddTable(ttcr, TrueTypeTableNew(T_OS2, 2000, t2));
    AddTable(ttcr, TrueTypeTableNew(T_cmap, 3000, t3));
    AddTable(ttcr, TrueTypeTableNew(T_loca, 4000, t4));
    AddTable(ttcr, TrueTypeTableNew(T_hhea, 5000, t5));
    AddTable(ttcr, TrueTypeTableNew(T_glyf, 6000, t6));

    free(t1);
    free(t2);
    free(t3);
    free(t4);
    free(t5);
    free(t6);

    StreamToFile(ttcr, "ttcrout.ttf");

    TrueTypeCreatorDispose(ttcr);
    return 0;
}
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
