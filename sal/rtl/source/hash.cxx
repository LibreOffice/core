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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sal.hxx"

#include "hash.h"
#include "strimp.h"
#include <osl/diagnose.h>

struct StringHashTableImpl {
    sal_uInt32    nEntries;
    sal_uInt32    nSize;
    rtl_uString **pData;
};

typedef StringHashTableImpl StringHashTable;

// Only for use in the implementation
static StringHashTable *rtl_str_hash_new (sal_uInt32 nSize);
static void rtl_str_hash_free (StringHashTable *pHash);

StringHashTable *
getHashTable ()
{
    static StringHashTable *pInternPool = NULL;
    if (pInternPool == NULL) {
        static StringHashTable* pHash = rtl_str_hash_new(1024);
        pInternPool = pHash;
    }
    return pInternPool;
}

// Better / smaller / faster hash set ....

// TODO: add bottom bit-set list terminator to string list

static sal_uInt32
getNextSize (sal_uInt32 nSize)
{
    // Sedgewick - Algorithms in C P577.
    static const sal_uInt32 nPrimes[] = { 1021, 2039, 4093, 8191, 16381, 32749,
                                          65521, 131071,262139, 524287, 1048573,
                                          2097143, 4194301, 8388593, 16777213,
                                          33554393, 67108859, 134217689 };
    #define NUM_PRIMES (sizeof (nPrimes)/ sizeof (nPrimes[0]))
    for (sal_uInt32 i = 0; i < NUM_PRIMES; i++)
    {
        if (nPrimes[i] > nSize)
            return nPrimes[i];
    }
    return nSize * 2;
}

static sal_uInt32
hashString (rtl_uString *pString)
{
    return (sal_uInt32) rtl_ustr_hashCode_WithLength (pString->buffer,
                                                      pString->length);
}

static StringHashTable *
rtl_str_hash_new (sal_uInt32 nSize)
{
    StringHashTable *pHash = (StringHashTable *)malloc (sizeof (StringHashTable));

    pHash->nEntries = 0;
    pHash->nSize = getNextSize (nSize);
    pHash->pData = (rtl_uString **) calloc (sizeof (rtl_uString *), pHash->nSize);

    return pHash;
}

static void
rtl_str_hash_free (StringHashTable *pHash)
{
    if (!pHash)
        return;
    if (pHash->pData)
        free (pHash->pData);
    free (pHash);
}

static void
rtl_str_hash_insert_nonequal (StringHashTable   *pHash,
                              rtl_uString       *pString)
{
    sal_uInt32  nHash = hashString (pString);
    sal_uInt32  n;
    rtl_uString *pHashStr;

    n = nHash % pHash->nSize;
    while ((pHashStr = pHash->pData[n]) != NULL) {
        n++;
        if (n >= pHash->nSize)
            n = 0;
    }
    pHash->pData[n] = pString;
}

static void
rtl_str_hash_resize (sal_uInt32        nNewSize)
{
    sal_uInt32 i;
    StringHashTable *pNewHash;
    StringHashTable *pHash = getHashTable();

    OSL_ASSERT (nNewSize > pHash->nEntries);

    pNewHash = rtl_str_hash_new (nNewSize);

    for (i = 0; i < pHash->nSize; i++)
    {
        if (pHash->pData[i] != NULL)
            rtl_str_hash_insert_nonequal (pNewHash, pHash->pData[i]);
    }
    pNewHash->nEntries = pHash->nEntries;
    free (pHash->pData);
    *pHash = *pNewHash;
    pNewHash->pData = NULL;
    rtl_str_hash_free (pNewHash);
}

static int
compareEqual (rtl_uString *pStringA, rtl_uString *pStringB)
{
    if (pStringA == pStringB)
        return 1;
    if (pStringA->length != pStringB->length)
        return 0;
    return !rtl_ustr_compare_WithLength( pStringA->buffer, pStringA->length,
                                         pStringB->buffer, pStringB->length);
}


rtl_uString *
rtl_str_hash_intern (rtl_uString       *pString,
                     int                can_return)
{
    sal_uInt32  nHash = hashString (pString);
    sal_uInt32  n;
    rtl_uString *pHashStr;

    StringHashTable *pHash = getHashTable();

    // Should we resize ?
    if (pHash->nEntries >= pHash->nSize/2)
        rtl_str_hash_resize (getNextSize(pHash->nSize));

    n = nHash % pHash->nSize;
    while ((pHashStr = pHash->pData[n]) != NULL) {
        if (compareEqual (pHashStr, pString))
        {
            rtl_uString_acquire (pHashStr);
            return pHashStr;
        }
        n++;
        if (n >= pHash->nSize)
            n = 0;
    }

    if (!can_return)
    {
        rtl_uString *pCopy = NULL;
        rtl_uString_newFromString( &pCopy, pString );
        pString = pCopy;
        if (!pString)
            return NULL;
    }

    if (!SAL_STRING_IS_STATIC (pString))
        pString->refCount |= SAL_STRING_INTERN_FLAG;
    pHash->pData[n] = pString;
    pHash->nEntries++;

    return pString;
}

void
rtl_str_hash_remove (rtl_uString       *pString)
{
    sal_uInt32   n;
    sal_uInt32   nHash = hashString (pString);
    rtl_uString *pHashStr;

    StringHashTable *pHash = getHashTable();

    n = nHash % pHash->nSize;
    while ((pHashStr = pHash->pData[n]) != NULL) {
        if (compareEqual (pHashStr, pString))
            break;
        n++;
        if (n >= pHash->nSize)
            n = 0;
    }
    OSL_ASSERT (pHash->pData[n] != 0);
    if (pHash->pData[n] == NULL)
        return;

    pHash->pData[n++] = NULL;
    pHash->nEntries--;

    if (n >= pHash->nSize)
        n = 0;

    while ((pHashStr = pHash->pData[n]) != NULL) {
        pHash->pData[n] = NULL;
        // FIXME: rather unsophisticated and N^2 in chain-length, but robust.
        rtl_str_hash_insert_nonequal (pHash, pHashStr);
        n++;
        if (n >= pHash->nSize)
            n = 0;
    }
    // FIXME: Should we down-size ?
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
