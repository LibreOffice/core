/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "svl/sharedstringpool.hxx"
#include "unotools/charclass.hxx"

namespace svl {

SharedStringPool::SharedStringPool( const CharClass* pCharClass ) : mpCharClass(pCharClass) {}

SharedString SharedStringPool::intern( const OUString& rStr )
{
    osl::MutexGuard aGuard(&maMutex);

    InsertResultType aRes = findOrInsert(maStrPool, rStr);
    if (aRes.first == maStrPool.end())
        // Insertion failed.
        return SharedString();

    rtl_uString* pOrig = aRes.first->pData;

    if (!mpCharClass)
        // We don't track case insensitive strings.
        return SharedString(pOrig, NULL);

    if (!aRes.second)
    {
        // No new string has been inserted. Return the existing string in the pool.
        StrStoreType::iterator it = maStrStore.find(pOrig);
        if (it == maStrStore.end())
            return SharedString();

        rtl_uString* pUpper = it->second.pData;
        return SharedString(pOrig, pUpper);
    }

    // This is a new string insertion. Establish mapping to upper-case variant.

    OUString aUpper = mpCharClass->uppercase(rStr);
    aRes = findOrInsert(maStrPoolUpper, aUpper);
    if (aRes.first == maStrPoolUpper.end())
        // Failed to insert or fetch upper-case variant. Should never happen.
        return SharedString();

    maStrStore.insert(StrStoreType::value_type(pOrig, *aRes.first));

    return SharedString(pOrig, aRes.first->pData);
}

namespace {

inline sal_Int32 getRefCount( const rtl_uString* p )
{
    return (p->refCount & 0x3FFFFFFF);
}

}

void SharedStringPool::purge()
{
    osl::MutexGuard aGuard(&maMutex);

    StrHashType aNewStrPool;
    StrHashType::iterator it = maStrPool.begin(), itEnd = maStrPool.end();
    for (; it != itEnd; ++it)
    {
        const rtl_uString* p = it->pData;
        if (getRefCount(p) == 1)
        {
            // Remove it from the upper string map.  This should unref the
            // upper string linked to this original string.
            maStrStore.erase(p);
        }
        else
            // Still referenced outside the pool. Keep it.
            aNewStrPool.insert(*it);
    }

    maStrPool.swap(aNewStrPool);

    aNewStrPool.clear(); // for re-use.

    // Purge the upper string pool as well.
    it = maStrPoolUpper.begin();
    itEnd = maStrPoolUpper.end();
    for (; it != itEnd; ++it)
    {
        const rtl_uString* p = it->pData;
        if (getRefCount(p) > 1)
            aNewStrPool.insert(*it);
    }

    maStrPoolUpper.swap(aNewStrPool);
}

size_t SharedStringPool::getCount() const
{
    osl::MutexGuard aGuard(&maMutex);
    return maStrPool.size();
}

size_t SharedStringPool::getCountIgnoreCase() const
{
    osl::MutexGuard aGuard(&maMutex);
    return maStrPoolUpper.size();
}

SharedStringPool::InsertResultType SharedStringPool::findOrInsert( StrHashType& rPool, const OUString& rStr ) const
{
    StrHashType::iterator it = rPool.find(rStr);
    bool bInserted = false;
    if (it == rPool.end())
    {
        // Not yet in the pool.
        std::pair<StrHashType::iterator, bool> r = rPool.insert(rStr);
        if (!r.second)
            // Insertion failed.
            return InsertResultType(rPool.end(), false);

        it = r.first;
        bInserted = true;
    }

    return InsertResultType(it, bInserted);
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
