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

#include "hash.hxx"
#include "strimp.hxx"
#include <osl/diagnose.h>
#include <sal/macros.h>

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
    static StringHashTable *pInternPool = nullptr;
    if (pInternPool == nullptr) {
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

    for (sal_uInt32 nPrime : nPrimes)
    {
        if (nPrime > nSize)
            return nPrime;
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
    StringHashTable *pHash = static_cast<StringHashTable *>(malloc (sizeof (StringHashTable)));

    pHash->nEntries = 0;
    pHash->nSize = getNextSize (nSize);
    pHash->pData = static_cast< rtl_uString ** >(calloc(pHash->nSize, sizeof(rtl_uString *)));

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

    n = nHash % pHash->nSize;
    while (pHash->pData[n] != nullptr) {
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
        if (pHash->pData[i] != nullptr)
            rtl_str_hash_insert_nonequal (pNewHash, pHash->pData[i]);
    }
    pNewHash->nEntries = pHash->nEntries;
    free (pHash->pData);
    *pHash = *pNewHash;
    pNewHash->pData = nullptr;
    rtl_str_hash_free (pNewHash);
}

static bool
compareEqual (rtl_uString *pStringA, rtl_uString *pStringB)
{
    if (pStringA == pStringB)
        return true;
    if (pStringA->length != pStringB->length)
        return false;
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
    while ((pHashStr = pHash->pData[n]) != nullptr) {
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
        rtl_uString *pCopy = nullptr;
        rtl_uString_newFromString( &pCopy, pString );
        pString = pCopy;
        if (!pString)
            return nullptr;
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
    while ((pHashStr = pHash->pData[n]) != nullptr) {
        if (compareEqual (pHashStr, pString))
            break;
        n++;
        if (n >= pHash->nSize)
            n = 0;
    }
    OSL_ASSERT (pHash->pData[n] != nullptr);
    if (pHash->pData[n] == nullptr)
        return;

    pHash->pData[n++] = nullptr;
    pHash->nEntries--;

    if (n >= pHash->nSize)
        n = 0;

    while ((pHashStr = pHash->pData[n]) != nullptr) {
        pHash->pData[n] = nullptr;
        // FIXME: rather unsophisticated and N^2 in chain-length, but robust.
        rtl_str_hash_insert_nonequal (pHash, pHashStr);
        n++;
        if (n >= pHash->nSize)
            n = 0;
    }
    // FIXME: Should we down-size ?
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
