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

#include <assert.h>

#include "ttcr.hxx"
#include "list.h"
#include <string.h>

namespace vcl
{

/*
 * Private Data Types
 */

    struct TrueTypeCreator {
        sal_uInt32 tag;                         /**< TrueType file tag */
        list   tables;                      /**< List of table tags and pointers */
    };

/* These must be #defined so that they can be used in initializers */
#define T_maxp  0x6D617870
#define T_glyf  0x676C7966
#define T_head  0x68656164
#define T_loca  0x6C6F6361
#define T_name  0x6E616D65
#define T_hhea  0x68686561
#define T_hmtx  0x686D7478
#define T_cmap  0x636D6170
#define T_post  0x706F7374

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

static int NameRecordCompareF(const void *l, const void *r)
{
    NameRecord const *ll = static_cast<NameRecord const *>(l);
    NameRecord const *rr = static_cast<NameRecord const *>(r);

    if (ll->platformID != rr->platformID) {
        return (ll->platformID < rr->platformID) ? -1 : 1;
    } else if (ll->encodingID != rr->encodingID) {
        return (ll->encodingID < rr->encodingID) ? -1 : 1;
    } else if (ll->languageID != rr->languageID) {
        return (ll->languageID < rr->languageID) ? -1 : 1;
    } else if (ll->nameID != rr->nameID) {
        return (ll->nameID < rr->nameID) ? -1 : 1;
    }
    return 0;
}

static sal_uInt32 CheckSum(sal_uInt32 *ptr, sal_uInt32 length)
{
    sal_uInt32 sum = 0;
    sal_uInt32 *endptr = ptr + ((length + 3) & sal_uInt32(~3)) / 4;

    while (ptr < endptr) sum += *ptr++;

    return sum;
}

static void *smalloc(sal_uInt32 size)
{
    void *res = malloc(size);
    assert(res != nullptr);
    return res;
}

static void *scalloc(sal_uInt32 n, sal_uInt32 size)
{
    void *res = calloc(n, size);
    assert(res != nullptr);
    return res;
}

/*
 * Public functions
 */

void TrueTypeCreatorNewEmpty(sal_uInt32 tag, TrueTypeCreator **_this)
{
    TrueTypeCreator* ptr = static_cast<TrueTypeCreator*>(smalloc(sizeof(TrueTypeCreator)));

    ptr->tables = listNewEmpty();
    listSetElementDtor(ptr->tables, TrueTypeTableDispose);

    ptr->tag = tag;

    *_this = ptr;
}

void AddTable(TrueTypeCreator *_this, TrueTypeTable *table)
{
    if (table != nullptr) {
        listAppend(_this->tables, table);
    }
}

void RemoveTable(TrueTypeCreator *_this, sal_uInt32 tag)
{
    if (listCount(_this->tables))
    {
        listToFirst(_this->tables);
        int done = 0;
        do {
            if (static_cast<TrueTypeTable *>(listCurrent(_this->tables))->tag == tag)
            {
                listRemove(_this->tables);
            }
            else
            {
                if (listNext(_this->tables))
                {
                    done = 1;
                }
            }
        } while (!done);
    }
}

static void ProcessTables(TrueTypeCreator *);

SFErrCodes StreamToMemory(TrueTypeCreator *_this, sal_uInt8 **ptr, sal_uInt32 *length)
{
    sal_uInt16 searchRange=1, entrySelector=0, rangeShift;
    sal_uInt32 s, offset, checkSumAdjustment = 0;
    sal_uInt32 *p;
    sal_uInt8 *head = nullptr;     /* saved pointer to the head table data for checkSumAdjustment calculation */

    if (listIsEmpty(_this->tables)) return SFErrCodes::TtFormat;

    ProcessTables(_this);

    /* ProcessTables() adds 'loca' and 'hmtx' */

    sal_uInt16 numTables = listCount(_this->tables);

    TableEntry* te = static_cast<TableEntry*>(scalloc(numTables, sizeof(TableEntry)));
    TableEntry* e = te;

    listToFirst(_this->tables);
    do {
        GetRawData(static_cast<TrueTypeTable *>(listCurrent(_this->tables)), &e->data, &e->length, &e->tag);
        ++e;
    } while (listNext(_this->tables));

    qsort(te, numTables, sizeof(TableEntry), TableEntryCompareF);

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

    sal_uInt8* ttf = static_cast<sal_uInt8*>(smalloc(s));

    /* Offset Table */
    PutUInt32(_this->tag, ttf, 0);
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

    free(te);

    p = reinterpret_cast<sal_uInt32 *>(ttf);
    for (int i = 0; i < static_cast<int>(s) / 4; ++i) checkSumAdjustment += p[i];
    PutUInt32(0xB1B0AFBA - checkSumAdjustment, head, 8);

    *ptr = ttf;
    *length = s;

    return SFErrCodes::Ok;
}

SFErrCodes StreamToFile(TrueTypeCreator *_this, const char* fname)
{
    sal_uInt8 *ptr;
    sal_uInt32 length;
    SFErrCodes r;
    FILE* fd;

    if ((r = StreamToMemory(_this, &ptr, &length)) != SFErrCodes::Ok) return r;
    if (fname && (fd = fopen(fname, "wb")) != nullptr)
    {
        if (fwrite(ptr, 1, length, fd) != length) {
            r = SFErrCodes::FileIo;
        } else {
            r = SFErrCodes::Ok;
        }
        fclose(fd);
    }
    else
    {
        r = SFErrCodes::BadFile;
    }
    free(ptr);
    return r;
}

/*
 * TrueTypeTable private methods
 */

#define TABLESIZE_head 54
#define TABLESIZE_hhea 36
#define TABLESIZE_maxp 32

/*    Table         data points to
 * --------------------------------------------
 *    generic       tdata_generic struct
 *    'head'        TABLESIZE_head bytes of memory
 *    'hhea'        TABLESIZE_hhea bytes of memory
 *    'loca'        tdata_loca struct
 *    'maxp'        TABLESIZE_maxp bytes of memory
 *    'glyf'        list of GlyphData structs (defined in sft.h)
 *    'name'        list of NameRecord structs (defined in sft.h)
 *    'post'        tdata_post struct
 *
 */

#define CMAP_SUBTABLE_INIT 10
#define CMAP_SUBTABLE_INCR 10
#define CMAP_PAIR_INIT 500
#define CMAP_PAIR_INCR 500

struct CmapSubTable {
    sal_uInt32  id;                         /* subtable ID (platform/encoding ID)    */
    sal_uInt32  n;                          /* number of used translation pairs      */
    sal_uInt32  m;                          /* number of allocated translation pairs */
    sal_uInt32 *xc;                         /* character array                       */
    sal_uInt32 *xg;                         /* glyph array                           */
};

struct table_cmap {
    sal_uInt32 n;                           /* number of used CMAP sub-tables       */
    sal_uInt32 m;                           /* number of allocated CMAP sub-tables  */
    CmapSubTable *s;                    /* sorted array of sub-tables           */
};

struct tdata_generic {
    sal_uInt32 tag;
    sal_uInt32 nbytes;
    sal_uInt8 *ptr;
};

struct tdata_loca {
    sal_uInt32 nbytes;                      /* number of bytes in loca table */
    sal_uInt8 *ptr;                          /* pointer to the data */
};

struct tdata_post {
    sal_uInt32 format;
    sal_uInt32 italicAngle;
    sal_Int16  underlinePosition;
    sal_Int16  underlineThickness;
    sal_uInt32 isFixedPitch;
    void   *ptr;                        /* format-specific pointer */
};

/* allocate memory for a TT table */
static sal_uInt8 *ttmalloc(sal_uInt32 nbytes)
{
    sal_uInt32 n;

    n = (nbytes + 3) & sal_uInt32(~3);
    sal_uInt8* res = static_cast<sal_uInt8*>(calloc(n, 1));
    assert(res != nullptr);

    return res;
}

static void FreeGlyphData(void *ptr)
{
    GlyphData *p = static_cast<GlyphData *>(ptr);
    if (p->ptr) free(p->ptr);
    free(p);
}

static void TrueTypeTableDispose_generic(TrueTypeTable *_this)
{
    if (_this) {
        if (_this->data) {
            tdata_generic *pdata = static_cast<tdata_generic *>(_this->data);
            if (pdata->nbytes) free(pdata->ptr);
            free(_this->data);
        }
        free(_this);
    }
}

static void TrueTypeTableDispose_head(TrueTypeTable *_this)
{
    if (_this) {
        if (_this->data) free(_this->data);
        free(_this);
    }
}

static void TrueTypeTableDispose_hhea(TrueTypeTable *_this)
{
    if (_this) {
        if (_this->data) free(_this->data);
        free(_this);
    }
}

static void TrueTypeTableDispose_loca(TrueTypeTable *_this)
{
    if (_this) {
        if (_this->data) {
            tdata_loca *p = static_cast<tdata_loca *>(_this->data);
            if (p->ptr) free(p->ptr);
            free(_this->data);
        }
        free(_this);
    }
}

static void TrueTypeTableDispose_maxp(TrueTypeTable *_this)
{
    if (_this) {
        if (_this->data) free(_this->data);
        free(_this);
    }
}

static void TrueTypeTableDispose_glyf(TrueTypeTable *_this)
{
    if (_this) {
        if (_this->data) listDispose(static_cast<list>(_this->data));
        free(_this);
    }
}

static void TrueTypeTableDispose_cmap(TrueTypeTable *_this)
{
    if (_this) {
        table_cmap *t = static_cast<table_cmap *>(_this->data);
        if (t) {
            CmapSubTable *s = t->s;
            if (s) {
                for (sal_uInt32 i = 0; i < t->m; i++) {
                    if (s[i].xc) free(s[i].xc);
                    if (s[i].xg) free(s[i].xg);
                }
                free(s);
            }
            free(t);
        }
        free(_this);
    }
}

static void TrueTypeTableDispose_name(TrueTypeTable *_this)
{
    if (_this) {
        if (_this->data) listDispose(static_cast<list>(_this->data));
        free(_this);
    }
}

static void TrueTypeTableDispose_post(TrueTypeTable *_this)
{
    if (_this) {
        tdata_post *p = static_cast<tdata_post *>(_this->data);
        if (p) {
            if (p->format == 0x00030000) {
                /* do nothing */
            } else {
                fprintf(stderr, "Unsupported format of a 'post' table: %08X.\n", static_cast<int>(p->format));
            }
            free(p);
        }
        free(_this);
    }
}

/* destructor vtable */

static struct {
    sal_uInt32 tag;
    void (*f)(TrueTypeTable *);
} const vtable1[] =
{
    {0,      TrueTypeTableDispose_generic},
    {T_head, TrueTypeTableDispose_head},
    {T_hhea, TrueTypeTableDispose_hhea},
    {T_loca, TrueTypeTableDispose_loca},
    {T_maxp, TrueTypeTableDispose_maxp},
    {T_glyf, TrueTypeTableDispose_glyf},
    {T_cmap, TrueTypeTableDispose_cmap},
    {T_name, TrueTypeTableDispose_name},
    {T_post, TrueTypeTableDispose_post}

};

static int GetRawData_generic(TrueTypeTable *_this, sal_uInt8 **ptr, sal_uInt32 *len, sal_uInt32 *tag)
{
    assert(_this != nullptr);
    assert(_this->data != nullptr);

    *ptr = static_cast<tdata_generic *>(_this->data)->ptr;
    *len = static_cast<tdata_generic *>(_this->data)->nbytes;
    *tag = static_cast<tdata_generic *>(_this->data)->tag;

    return TTCR_OK;
}

static int GetRawData_head(TrueTypeTable *_this, sal_uInt8 **ptr, sal_uInt32 *len, sal_uInt32 *tag)
{
    *len = TABLESIZE_head;
    *ptr = static_cast<sal_uInt8 *>(_this->data);
    *tag = T_head;

    return TTCR_OK;
}

static int GetRawData_hhea(TrueTypeTable *_this, sal_uInt8 **ptr, sal_uInt32 *len, sal_uInt32 *tag)
{
    *len = TABLESIZE_hhea;
    *ptr = static_cast<sal_uInt8 *>(_this->data);
    *tag = T_hhea;

    return TTCR_OK;
}

static int GetRawData_loca(TrueTypeTable *_this, sal_uInt8 **ptr, sal_uInt32 *len, sal_uInt32 *tag)
{
    tdata_loca *p;

    assert(_this->data != nullptr);

    p = static_cast<tdata_loca *>(_this->data);

    if (p->nbytes == 0) return TTCR_ZEROGLYPHS;

    *ptr = p->ptr;
    *len = p->nbytes;
    *tag = T_loca;

    return TTCR_OK;
}

static int GetRawData_maxp(TrueTypeTable *_this, sal_uInt8 **ptr, sal_uInt32 *len, sal_uInt32 *tag)
{
    *len = TABLESIZE_maxp;
    *ptr = static_cast<sal_uInt8 *>(_this->data);
    *tag = T_maxp;

    return TTCR_OK;
}

static int GetRawData_glyf(TrueTypeTable *_this, sal_uInt8 **ptr, sal_uInt32 *len, sal_uInt32 *tag)
{
    sal_uInt32 n, nbytes = 0;
    list l = static_cast<list>(_this->data);
    /* sal_uInt16 curID = 0;    */               /* to check if glyph IDs are sequential and start from zero */
    sal_uInt8 *p;

    *ptr = nullptr;
    *len = 0;
    *tag = 0;

    if (listCount(l) == 0) return TTCR_ZEROGLYPHS;

    listToFirst(l);
    do {
        /* if (((GlyphData *) listCurrent(l))->glyphID != curID++) return TTCR_GLYPHSEQ; */
        nbytes += static_cast<GlyphData *>(listCurrent(l))->nbytes;
    } while (listNext(l));

    p = _this->rawdata = ttmalloc(nbytes);

    listToFirst(l);
    do {
        n = static_cast<GlyphData *>(listCurrent(l))->nbytes;
        if (n != 0) {
            memcpy(p, static_cast<GlyphData *>(listCurrent(l))->ptr, n);
            p += n;
        }
    } while (listNext(l));

    *len = nbytes;
    *ptr = _this->rawdata;
    *tag = T_glyf;

    return TTCR_OK;
}

/* cmap packers */
static sal_uInt8 *PackCmapType0(CmapSubTable const *s, sal_uInt32 *length)
{
    sal_uInt8* ptr = static_cast<sal_uInt8*>(smalloc(262));
    sal_uInt8 *p = ptr + 6;
    sal_uInt32 i, j;
    sal_uInt16 g;

    PutUInt16(0, ptr, 0);
    PutUInt16(262, ptr, 2);
    PutUInt16(0, ptr, 4);

    for (i = 0; i < 256; i++) {
        g = 0;
        for (j = 0; j < s->n; j++) {
            if (s->xc[j] == i) {
                g = static_cast<sal_uInt16>(s->xg[j]);
            }
        }
        p[i] = static_cast<sal_uInt8>(g);
    }
    *length = 262;
    return ptr;
}

static sal_uInt8 *PackCmapType6(CmapSubTable const *s, sal_uInt32 *length)
{
    sal_uInt8* ptr = static_cast<sal_uInt8*>(smalloc(s->n*2 + 10));
    sal_uInt8 *p = ptr + 10;
    sal_uInt32 i, j;
    sal_uInt16 g;

    PutUInt16(6, ptr, 0);
    PutUInt16(static_cast<sal_uInt16>(s->n*2+10), ptr, 2);
    PutUInt16(0, ptr, 4);
    PutUInt16(0, ptr, 6);
    PutUInt16(static_cast<sal_uInt16>(s->n), ptr, 8 );

    for (i = 0; i < s->n; i++) {
        g = 0;
        for (j = 0; j < s->n; j++) {
            if (s->xc[j] == i) {
                g = static_cast<sal_uInt16>(s->xg[j]);
            }
        }
        PutUInt16( g, p, 2*i );
    }
    *length = s->n*2+10;
    return ptr;
}

/* XXX it only handles Format 0 encoding tables */
static sal_uInt8 *PackCmap(CmapSubTable const *s, sal_uInt32 *length)
{
    if( s->xg[s->n-1] > 0xff )
        return PackCmapType6(s, length);
    else
        return PackCmapType0(s, length);
}

static int GetRawData_cmap(TrueTypeTable *_this, sal_uInt8 **ptr, sal_uInt32 *len, sal_uInt32 *tag)
{
    table_cmap *t;
    sal_uInt32 i;
    sal_uInt32 tlen = 0;
    sal_uInt32 l;
    sal_uInt32 cmapsize;
    sal_uInt8 *cmap;
    sal_uInt32 coffset;

    assert(_this != nullptr);
    t = static_cast<table_cmap *>(_this->data);
    assert(t != nullptr);
    assert(t->n != 0);

    sal_uInt8** subtables = static_cast<sal_uInt8**>(scalloc(t->n, sizeof(sal_uInt8 *)));
    sal_uInt32* sizes = static_cast<sal_uInt32*>(scalloc(t->n, sizeof(sal_uInt32)));

    for (i = 0; i < t->n; i++) {
        subtables[i] = PackCmap(t->s+i, &l);
        sizes[i] = l;
        tlen += l;
    }

    cmapsize = tlen + 4 + 8 * t->n;
    _this->rawdata = cmap = ttmalloc(cmapsize);

    PutUInt16(0, cmap, 0);
    PutUInt16(static_cast<sal_uInt16>(t->n), cmap, 2);
    coffset = 4 + t->n * 8;

    for (i = 0; i < t->n; i++) {
        PutUInt16(static_cast<sal_uInt16>(t->s[i].id >> 16), cmap + 4, i * 8);
        PutUInt16(static_cast<sal_uInt16>(t->s[i].id & 0xFF), cmap + 4, 2 + i * 8);
        PutUInt32(coffset, cmap + 4, 4 + i * 8);
        memcpy(cmap + coffset, subtables[i], sizes[i]);
        free(subtables[i]);
        coffset += sizes[i];
    }

    free(subtables);
    free(sizes);

    *ptr = cmap;
    *len = cmapsize;
    *tag = T_cmap;

    return TTCR_OK;
}

static int GetRawData_name(TrueTypeTable *_this, sal_uInt8 **ptr, sal_uInt32 *len, sal_uInt32 *tag)
{
    list l;
    sal_Int16 i=0, n;                          /* number of Name Records */
    int stringLen = 0;
    sal_uInt8 *p1, *p2;

    *ptr = nullptr;
    *len = 0;
    *tag = 0;

    assert(_this != nullptr);
    l = static_cast<list>(_this->data);
    assert(l != nullptr);

    if ((n = static_cast<sal_Int16>(listCount(l))) == 0) return TTCR_NONAMES;

    NameRecord* nr = static_cast<NameRecord*>(scalloc(n, sizeof(NameRecord)));

    listToFirst(l);

    do {
        memcpy(nr+i, listCurrent(l), sizeof(NameRecord));
        stringLen += nr[i].slen;
        i++;
    } while (listNext(l));

    if (stringLen > 65535) {
        free(nr);
        return TTCR_NAMETOOLONG;
    }

    qsort(nr, n, sizeof(NameRecord), NameRecordCompareF);

    int nameLen = stringLen + 12 * n + 6;
    sal_uInt8* name = ttmalloc(nameLen);

    PutUInt16(0, name, 0);
    PutUInt16(n, name, 2);
    PutUInt16(static_cast<sal_uInt16>(6 + 12 * n), name, 4);

    p1 = name + 6;
    p2 = p1 + 12 * n;

    for (i = 0; i < n; i++) {
        PutUInt16(nr[i].platformID, p1, 0);
        PutUInt16(nr[i].encodingID, p1, 2);
        PutUInt16(static_cast<sal_uInt16>(nr[i].languageID), p1, 4);
        PutUInt16(nr[i].nameID, p1, 6);
        PutUInt16(nr[i].slen, p1, 8);
        PutUInt16(static_cast<sal_uInt16>(p2 - (name + 6 + 12 * n)), p1, 10);
        if (nr[i].slen) {
            memcpy(p2, nr[i].sptr, nr[i].slen);
        }
        /* {int j; for(j=0; j<nr[i].slen; j++) printf("%c", nr[i].sptr[j]); printf("\n"); }; */
        p2 += nr[i].slen;
        p1 += 12;
    }

    free(nr);
    _this->rawdata = name;

    *ptr = name;
    *len = static_cast<sal_uInt16>(nameLen);
    *tag = T_name;

    /*{int j; for(j=0; j<nameLen; j++) printf("%c", name[j]); }; */

    return TTCR_OK;
}

static int GetRawData_post(TrueTypeTable *_this, sal_uInt8 **ptr, sal_uInt32 *len, sal_uInt32 *tag)
{
    tdata_post *p = static_cast<tdata_post *>(_this->data);
    sal_uInt8 *post = nullptr;
    sal_uInt32 postLen = 0;
    int ret;

    if (_this->rawdata) free(_this->rawdata);

    if (p->format == 0x00030000) {
        postLen = 32;
        post = ttmalloc(postLen);
        PutUInt32(0x00030000, post, 0);
        PutUInt32(p->italicAngle, post, 4);
        PutUInt16(p->underlinePosition, post, 8);
        PutUInt16(p->underlineThickness, post, 10);
        PutUInt16(static_cast<sal_uInt16>(p->isFixedPitch), post, 12);
        ret = TTCR_OK;
    } else {
        fprintf(stderr, "Unrecognized format of a post table: %08X.\n", static_cast<int>(p->format));
        ret = TTCR_POSTFORMAT;
    }

    *ptr = _this->rawdata = post;
    *len = postLen;
    *tag = T_post;

    return ret;
}

static struct {
    sal_uInt32 tag;
    int (*f)(TrueTypeTable *, sal_uInt8 **, sal_uInt32 *, sal_uInt32 *);
} const vtable2[] =
{
    {0,      GetRawData_generic},
    {T_head, GetRawData_head},
    {T_hhea, GetRawData_hhea},
    {T_loca, GetRawData_loca},
    {T_maxp, GetRawData_maxp},
    {T_glyf, GetRawData_glyf},
    {T_cmap, GetRawData_cmap},
    {T_name, GetRawData_name},
    {T_post, GetRawData_post}

};

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

TrueTypeTable *TrueTypeTableNew(sal_uInt32 tag,
                                sal_uInt32 nbytes,
                                const sal_uInt8* ptr)
{
    TrueTypeTable* table = static_cast<TrueTypeTable*>(smalloc(sizeof(TrueTypeTable)));
    tdata_generic* pdata = static_cast<tdata_generic*>(smalloc(sizeof(tdata_generic)));
    pdata->nbytes = nbytes;
    pdata->tag = tag;
    if (nbytes) {
        pdata->ptr = ttmalloc(nbytes);
        memcpy(pdata->ptr, ptr, nbytes);
    } else {
        pdata->ptr = nullptr;
    }

    table->tag = 0;
    table->data = pdata;
    table->rawdata = nullptr;

    return table;
}

TrueTypeTable *TrueTypeTableNew_head(sal_uInt32 fontRevision,
                                     sal_uInt16 flags,
                                     sal_uInt16 unitsPerEm,
                                     const sal_uInt8* created,
                                     sal_uInt16 macStyle,
                                     sal_uInt16 lowestRecPPEM,
                                     sal_Int16  fontDirectionHint)
{
    assert(created != nullptr);

    TrueTypeTable* table  = static_cast<TrueTypeTable*>(smalloc(sizeof(TrueTypeTable)));
    sal_uInt8* ptr = ttmalloc(TABLESIZE_head);

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

    table->data = static_cast<void *>(ptr);
    table->tag = T_head;
    table->rawdata = nullptr;

    return table;
}

TrueTypeTable *TrueTypeTableNew_hhea(sal_Int16  ascender,
                                     sal_Int16  descender,
                                     sal_Int16  linegap,
                                     sal_Int16  caretSlopeRise,
                                     sal_Int16  caretSlopeRun)
{
    TrueTypeTable* table = static_cast<TrueTypeTable*>(smalloc(sizeof(TrueTypeTable)));
    sal_uInt8* ptr = ttmalloc(TABLESIZE_hhea);

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

    table->data = static_cast<void *>(ptr);
    table->tag = T_hhea;
    table->rawdata = nullptr;

    return table;
}

TrueTypeTable *TrueTypeTableNew_loca()
{
    TrueTypeTable* table = static_cast<TrueTypeTable*>(smalloc(sizeof(TrueTypeTable)));
    table->data = smalloc(sizeof(tdata_loca));

    static_cast<tdata_loca *>(table->data)->nbytes = 0;
    static_cast<tdata_loca *>(table->data)->ptr = nullptr;

    table->tag = T_loca;
    table->rawdata = nullptr;

    return table;
}

TrueTypeTable *TrueTypeTableNew_maxp( const sal_uInt8* maxp, int size)
{
    TrueTypeTable* table = static_cast<TrueTypeTable*>(smalloc(sizeof(TrueTypeTable)));
    table->data = ttmalloc(TABLESIZE_maxp);

    if (maxp && size == TABLESIZE_maxp) {
        memcpy(table->data, maxp, TABLESIZE_maxp);
    }

    table->tag = T_maxp;
    table->rawdata = nullptr;

    return table;
}

TrueTypeTable *TrueTypeTableNew_glyf()
{
    TrueTypeTable* table = static_cast<TrueTypeTable*>(smalloc(sizeof(TrueTypeTable)));
    list l = listNewEmpty();

    assert(l != nullptr);

    listSetElementDtor(l, FreeGlyphData);

    table->data = l;
    table->rawdata = nullptr;
    table->tag = T_glyf;

    return table;
}

TrueTypeTable *TrueTypeTableNew_cmap()
{
    TrueTypeTable* table = static_cast<TrueTypeTable*>(smalloc(sizeof(TrueTypeTable)));
    table_cmap* cmap = static_cast<table_cmap*>(smalloc(sizeof(table_cmap)));

    cmap->n = 0;
    cmap->m = CMAP_SUBTABLE_INIT;
    cmap->s = static_cast<CmapSubTable *>(scalloc(CMAP_SUBTABLE_INIT, sizeof(CmapSubTable)));

    table->data = cmap;

    table->rawdata = nullptr;
    table->tag = T_cmap;

    return table;
}

static void DisposeNameRecord(void *ptr)
{
    if (ptr != nullptr) {
        NameRecord *nr = static_cast<NameRecord *>(ptr);
        if (nr->sptr) free(nr->sptr);
        free(ptr);
    }
}

static NameRecord* NameRecordNewCopy(NameRecord const *nr)
{
    NameRecord* p = static_cast<NameRecord*>(smalloc(sizeof(NameRecord)));

    memcpy(p, nr, sizeof(NameRecord));

    if (p->slen) {
        p->sptr = static_cast<sal_uInt8*>(smalloc(p->slen));
        memcpy(p->sptr, nr->sptr, p->slen);
    }

    return p;
}

TrueTypeTable *TrueTypeTableNew_name(int n, NameRecord const *nr)
{
    TrueTypeTable* table = static_cast<TrueTypeTable*>(smalloc(sizeof(TrueTypeTable)));
    list l = listNewEmpty();

    assert(l != nullptr);

    listSetElementDtor(l, DisposeNameRecord);

    if (n != 0) {
        int i;
        for (i = 0; i < n; i++) {
            listAppend(l, NameRecordNewCopy(nr+i));
        }
    }

    table->data = l;
    table->rawdata = nullptr;
    table->tag = T_name;

    return table;
}

TrueTypeTable *TrueTypeTableNew_post(sal_uInt32 format,
                                     sal_uInt32 italicAngle,
                                     sal_Int16 underlinePosition,
                                     sal_Int16 underlineThickness,
                                     sal_uInt32 isFixedPitch)
{
    assert(format == 0x00030000);                 /* Only format 3.0 is supported at this time */
    TrueTypeTable* table = static_cast<TrueTypeTable*>(smalloc(sizeof(TrueTypeTable)));
    tdata_post* post = static_cast<tdata_post*>(smalloc(sizeof(tdata_post)));

    post->format = format;
    post->italicAngle = italicAngle;
    post->underlinePosition = underlinePosition;
    post->underlineThickness = underlineThickness;
    post->isFixedPitch = isFixedPitch;
    post->ptr = nullptr;

    table->data = post;
    table->rawdata = nullptr;
    table->tag = T_post;

    return table;
}

int GetRawData(TrueTypeTable *_this, sal_uInt8 **ptr, sal_uInt32 *len, sal_uInt32 *tag)
{
    /* XXX do a binary search */
    assert(_this != nullptr);
    assert(ptr != nullptr);
    assert(len != nullptr);
    assert(tag != nullptr);

    *ptr = nullptr; *len = 0; *tag = 0;

    if (_this->rawdata) {
        free(_this->rawdata);
        _this->rawdata = nullptr;
    }

    for(size_t i=0; i < SAL_N_ELEMENTS(vtable2); i++) {
        if (_this->tag == vtable2[i].tag) {
            return vtable2[i].f(_this, ptr, len, tag);
        }
    }

    assert(!"Unknown TrueType table.");
    return TTCR_UNKNOWN;
}

void cmapAdd(TrueTypeTable *table, sal_uInt32 id, sal_uInt32 c, sal_uInt32 g)
{
    sal_uInt32 i, found;
    table_cmap *t;
    CmapSubTable *s;

    assert(table != nullptr);
    assert(table->tag == T_cmap);
    t = static_cast<table_cmap *>(table->data); assert(t != nullptr);
    s = t->s; assert(s != nullptr);

    found = 0;

    for (i = 0; i < t->n; i++) {
        if (s[i].id == id) {
            found = 1;
            break;
        }
    }

    if (!found) {
        if (t->n == t->m) {
            CmapSubTable* tmp = static_cast<CmapSubTable*>(scalloc(t->m + CMAP_SUBTABLE_INCR, sizeof(CmapSubTable)));
            memcpy(tmp, s, sizeof(CmapSubTable) * t->m);
            t->m += CMAP_SUBTABLE_INCR;
            free(s);
            s = tmp;
            t->s = s;
        }

        for (i = 0; i < t->n; i++) {
            if (s[i].id > id) break;
        }

        if (i < t->n) {
            memmove(s+i+1, s+i, t->n-i);
        }

        t->n++;

        s[i].id = id;
        s[i].n = 0;
        s[i].m = CMAP_PAIR_INIT;
        s[i].xc = static_cast<sal_uInt32*>(scalloc(CMAP_PAIR_INIT, sizeof(sal_uInt32)));
        s[i].xg = static_cast<sal_uInt32*>(scalloc(CMAP_PAIR_INIT, sizeof(sal_uInt32)));
    }

    if (s[i].n == s[i].m) {
        sal_uInt32* tmp1 = static_cast<sal_uInt32*>(scalloc(s[i].m + CMAP_PAIR_INCR, sizeof(sal_uInt32)));
        sal_uInt32* tmp2 = static_cast<sal_uInt32*>(scalloc(s[i].m + CMAP_PAIR_INCR, sizeof(sal_uInt32)));
        assert(tmp1 != nullptr);
        assert(tmp2 != nullptr);
        memcpy(tmp1, s[i].xc, sizeof(sal_uInt32) * s[i].m);
        memcpy(tmp2, s[i].xg, sizeof(sal_uInt32) * s[i].m);
        s[i].m += CMAP_PAIR_INCR;
        free(s[i].xc);
        free(s[i].xg);
        s[i].xc = tmp1;
        s[i].xg = tmp2;
    }

    s[i].xc[s[i].n] = c;
    s[i].xg[s[i].n] = g;
    s[i].n++;
}

sal_uInt32 glyfAdd(TrueTypeTable *table, GlyphData *glyphdata, TrueTypeFont *fnt)
{
    list l;
    sal_uInt32 currentID;
    int ret, n, ncomponents;

    assert(table != nullptr);
    assert(table->tag == T_glyf);

    if (!glyphdata) return sal_uInt32(~0);

    std::vector< sal_uInt32 > glyphlist;

    ncomponents = GetTTGlyphComponents(fnt, glyphdata->glyphID, glyphlist);

    l = static_cast<list>(table->data);
    if (listCount(l) > 0) {
        listToLast(l);
        ret = n = static_cast<GlyphData *>(listCurrent(l))->newID + 1;
    } else {
        ret = n = 0;
    }
    glyphdata->newID = n++;
    listAppend(l, glyphdata);

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
            listToFirst(l);
            do {
                if (static_cast<GlyphData *>(listCurrent(l))->glyphID == currentID) {
                    found = 1;
                    break;
                }
            } while (listNext(l));

            if (!found) {
                GlyphData *gd = GetTTRawGlyphData(fnt, currentID);
                gd->newID = n++;
                listAppend(l, gd);
            }
        } while( ++it !=  glyphlist.end() );
    }

    return ret;
}

sal_uInt32 glyfCount(const TrueTypeTable *table)
{
    assert(table != nullptr);
    assert(table->tag == T_glyf);
    return listCount(static_cast<list>(table->data));
}

static TrueTypeTable *FindTable(TrueTypeCreator *tt, sal_uInt32 tag)
{
    if (listIsEmpty(tt->tables)) return nullptr;

    listToFirst(tt->tables);

    do {
        if (static_cast<TrueTypeTable *>(listCurrent(tt->tables))->tag == tag) {
            return static_cast<TrueTypeTable*>(listCurrent(tt->tables));
        }
    } while (listNext(tt->tables));

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
static void ProcessTables(TrueTypeCreator *tt)
{
    TrueTypeTable *glyf, *loca, *head, *maxp, *hhea;
    list glyphlist;
    sal_uInt32 nGlyphs, locaLen = 0, glyfLen = 0;
    sal_Int16 xMin = 0, yMin = 0, xMax = 0, yMax = 0;
    sal_uInt32 i = 0;
    sal_Int16 indexToLocFormat;
    sal_uInt8 *hmtxPtr, *hheaPtr;
    sal_uInt32 hmtxSize;
    sal_uInt8 *p1, *p2;
    sal_uInt16 maxPoints = 0, maxContours = 0, maxCompositePoints = 0, maxCompositeContours = 0;
    int nlsb = 0;
    sal_uInt32 *gid;                        /* array of old glyphIDs */

    glyf = FindTable(tt, T_glyf);
    glyphlist = static_cast<list>(glyf->data);
    nGlyphs = listCount(glyphlist);
    assert(nGlyphs != 0);
    gid = static_cast<sal_uInt32*>(scalloc(nGlyphs, sizeof(sal_uInt32)));

    RemoveTable(tt, T_loca);
    RemoveTable(tt, T_hmtx);

    /* XXX Need to make sure that composite glyphs do not break during glyph renumbering */

    listToFirst(glyphlist);
    do {
        GlyphData *gd = static_cast<GlyphData *>(listCurrent(glyphlist));
        sal_Int16 z;
        glyfLen += gd->nbytes;
        /* XXX if (gd->nbytes & 1) glyfLen++; */

        assert(gd->newID == i);
        gid[i++] = gd->glyphID;
        /* gd->glyphID = i++; */

        /* printf("IDs: %d %d.\n", gd->glyphID, gd->newID); */

        if (gd->nbytes != 0) {
            z = GetInt16(gd->ptr, 2);
            if (z < xMin) xMin = z;

            z = GetInt16(gd->ptr, 4);
            if (z < yMin) yMin = z;

            z = GetInt16(gd->ptr, 6);
            if (z > xMax) xMax = z;

            z = GetInt16(gd->ptr, 8);
            if (z > yMax) yMax = z;
        }

        if (!gd->compflag) {                                /* non-composite glyph */
            if (gd->npoints > maxPoints) maxPoints = gd->npoints;
            if (gd->ncontours > maxContours) maxContours = gd->ncontours;
        } else {                                            /* composite glyph */
            if (gd->npoints > maxCompositePoints) maxCompositePoints = gd->npoints;
            if (gd->ncontours > maxCompositeContours) maxCompositeContours = gd->ncontours;
        }

    } while (listNext(glyphlist));

    indexToLocFormat = (glyfLen / 2 > 0xFFFF) ? 1 : 0;
    locaLen = indexToLocFormat ?  (nGlyphs + 1) << 2 : (nGlyphs + 1) << 1;

    sal_uInt8* glyfPtr = ttmalloc(glyfLen);
    sal_uInt8* locaPtr = ttmalloc(locaLen);
    TTSimpleGlyphMetrics* met = static_cast<TTSimpleGlyphMetrics*>(scalloc(nGlyphs, sizeof(TTSimpleGlyphMetrics)));
    i = 0;

    listToFirst(glyphlist);
    p1 = glyfPtr;
    p2 = locaPtr;
    do {
        GlyphData *gd = static_cast<GlyphData *>(listCurrent(glyphlist));

        if (gd->compflag) {                       /* re-number all components */
            sal_uInt16 flags, index;
            sal_uInt8 *ptr = gd->ptr + 10;
            do {
                sal_uInt32 j;
                flags = GetUInt16(ptr, 0);
                index = GetUInt16(ptr, 2);
                /* XXX use the sorted array of old to new glyphID mapping and do a binary search */
                for (j = 0; j < nGlyphs; j++) {
                    if (gid[j] == index) {
                        break;
                    }
                }
                /* printf("X: %d -> %d.\n", index, j); */

                PutUInt16(static_cast<sal_uInt16>(j), ptr, 2);

                ptr += 4;

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

        if (gd->nbytes != 0) {
            memcpy(p1, gd->ptr, gd->nbytes);
        }
        if (indexToLocFormat == 1) {
            PutUInt32(p1 - glyfPtr, p2, 0);
            p2 += 4;
        } else {
            PutUInt16(static_cast<sal_uInt16>((p1 - glyfPtr) >> 1), p2, 0);
            p2 += 2;
        }
        p1 += gd->nbytes;

        /* fill the array of metrics */
        met[i].adv = gd->aw;
        met[i].sb  = gd->lsb;
        i++;
    } while (listNext(glyphlist));

    free(gid);

    if (indexToLocFormat == 1) {
        PutUInt32(p1 - glyfPtr, p2, 0);
    } else {
        PutUInt16(static_cast<sal_uInt16>((p1 - glyfPtr) >> 1), p2, 0);
    }

    glyf->rawdata = glyfPtr;

    loca = TrueTypeTableNew_loca(); assert(loca != nullptr);
    static_cast<tdata_loca *>(loca->data)->ptr = locaPtr;
    static_cast<tdata_loca *>(loca->data)->nbytes = locaLen;

    AddTable(tt, loca);

    head = FindTable(tt, T_head);
    sal_uInt8* const pHeadData = static_cast<sal_uInt8*>(head->data);
    PutInt16(xMin, pHeadData, 36);
    PutInt16(yMin, pHeadData, 38);
    PutInt16(xMax, pHeadData, 40);
    PutInt16(yMax, pHeadData, 42);
    PutInt16(indexToLocFormat, pHeadData, 50);

    maxp = FindTable(tt, T_maxp);

    sal_uInt8* const pMaxpData = static_cast<sal_uInt8*>(maxp->data);
    PutUInt16(static_cast<sal_uInt16>(nGlyphs), pMaxpData, 4);
    PutUInt16(maxPoints, pMaxpData, 6);
    PutUInt16(maxContours, pMaxpData, 8);
    PutUInt16(maxCompositePoints, pMaxpData, 10);
    PutUInt16(maxCompositeContours, pMaxpData, 12);

    /*
     * Generate an htmx table and update hhea table
     */
    hhea = FindTable(tt, T_hhea); assert(hhea != nullptr);
    hheaPtr = static_cast<sal_uInt8 *>(hhea->data);
    if (nGlyphs > 2) {
        for (i = nGlyphs - 1; i > 0; i--) {
            if (met[i].adv != met[i-1].adv) break;
        }
        nlsb = nGlyphs - 1 - i;
    }
    hmtxSize = (nGlyphs - nlsb) * 4 + nlsb * 2;
    hmtxPtr = ttmalloc(hmtxSize);
    p1 = hmtxPtr;

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

    AddTable(tt, TrueTypeTableNew(T_hmtx, hmtxSize, hmtxPtr));
    PutUInt16(static_cast<sal_uInt16>(nGlyphs - nlsb), hheaPtr, 34);
    free(hmtxPtr);
    free(met);
}

} // namespace vcl

extern "C"
{
    /**
     * TrueTypeCreator destructor. It calls destructors for all TrueTypeTables added to it.
     */
     void TrueTypeCreatorDispose(vcl::TrueTypeCreator *_this)
    {
        listDispose(_this->tables);
        free(_this);
    }

    /**
     * Destructor for the TrueTypeTable object.
     */
     void TrueTypeTableDispose(void * arg)
    {
        vcl::TrueTypeTable *_this = static_cast<vcl::TrueTypeTable *>(arg);
        /* XXX do a binary search */
        assert(_this != nullptr);

        if (_this->rawdata) free(_this->rawdata);

        for(size_t i=0; i < SAL_N_ELEMENTS(vcl::vtable1); i++) {
            if (_this->tag == vcl::vtable1[i].tag) {
                vcl::vtable1[i].f(_this);
                return;
            }
        }
        assert(!"Unknown TrueType table.");
    }
}

#ifdef TEST_TTCR
static sal_uInt32 mkTag(sal_uInt8 a, sal_uInt8 b, sal_uInt8 c, sal_uInt8 d) {
    return (a << 24) | (b << 16) | (c << 8) | d;
}

int main()
{
    TrueTypeCreator *ttcr;
    sal_uInt8 *t1, *t2, *t3, *t4, *t5, *t6, *t7;

    TrueTypeCreatorNewEmpty(mkTag('t','r','u','e'), &ttcr);

    t1 = malloc(1000); memset(t1, 'a', 1000);
    t2 = malloc(2000); memset(t2, 'b', 2000);
    t3 = malloc(3000); memset(t3, 'c', 3000);
    t4 = malloc(4000); memset(t4, 'd', 4000);
    t5 = malloc(5000); memset(t5, 'e', 5000);
    t6 = malloc(6000); memset(t6, 'f', 6000);
    t7 = malloc(7000); memset(t7, 'g', 7000);

    AddTable(ttcr, TrueTypeTableNew(0x6D617870, 1000, t1));
    AddTable(ttcr, TrueTypeTableNew(0x4F532F32, 2000, t2));
    AddTable(ttcr, TrueTypeTableNew(0x636D6170, 3000, t3));
    AddTable(ttcr, TrueTypeTableNew(0x6C6F6361, 4000, t4));
    AddTable(ttcr, TrueTypeTableNew(0x68686561, 5000, t5));
    AddTable(ttcr, TrueTypeTableNew(0x676C7966, 6000, t6));
    AddTable(ttcr, TrueTypeTableNew(0x6B65726E, 7000, t7));

    free(t1);
    free(t2);
    free(t3);
    free(t4);
    free(t5);
    free(t6);
    free(t7);

    StreamToFile(ttcr, "ttcrout.ttf");

    TrueTypeCreatorDispose(ttcr);
    return 0;
}
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
