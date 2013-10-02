/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "svl/stringpool.hxx"
#include "unotools/charclass.hxx"

namespace svl {

StringPool::StringPool() : mpCharClass(NULL) {}
StringPool::StringPool( const CharClass* pCharClass ) : mpCharClass(pCharClass) {}

rtl_uString* StringPool::intern( const OUString& rStr )
{
    InsertResultType aRes = findOrInsert(maStrPool, rStr);
    if (aRes.first == maStrPool.end())
        // Insertion failed.
        return NULL;

    rtl_uString* pOrig = aRes.first->pData;

    if (!aRes.second)
        // No new string has been inserted. Return the existing string in the pool.
        return pOrig;

    if (!mpCharClass)
        return pOrig;

    // This is a new string insertion. Establish mapping to upper-case variant.

    OUString aUpper = mpCharClass->uppercase(rStr);
    aRes = findOrInsert(maStrPoolUpper, aUpper);
    if (aRes.first == maStrPoolUpper.end())
        // Failed to insert or fetch upper-case variant. Should never happen.
        return pOrig;

    // Set mapping.
    maToUpperMap.insert(StrIdMapType::value_type(pOrig, *aRes.first));

    return pOrig;
}

StringPool::StrIdType StringPool::getIdentifier( const OUString& rStr ) const
{
    StrHashType::const_iterator it = maStrPool.find(rStr);
    return (it == maStrPool.end()) ? 0 : reinterpret_cast<StrIdType>(it->pData);
}

StringPool::StrIdType StringPool::getIdentifierIgnoreCase( const OUString& rStr ) const
{
    StrHashType::const_iterator itOrig = maStrPool.find(rStr);
    if (itOrig == maStrPool.end())
        // Not in the pool.
        return 0;

    StrIdMapType::const_iterator itUpper = maToUpperMap.find(itOrig->pData);
    if (itUpper == maToUpperMap.end())
        // Passed string is not in the pool.
        return 0;

    const rtl_uString* pUpper = itUpper->second.pData;
    return reinterpret_cast<StrIdType>(pUpper);
}

namespace {

inline sal_Int32 getRefCount( const rtl_uString* p )
{
    return (p->refCount & 0x3FFFFFFF);
}

}

void StringPool::purge()
{
    StrHashType aNewStrPool;
    StrHashType::iterator it = maStrPool.begin(), itEnd = maStrPool.end();
    for (; it != itEnd; ++it)
    {
        const rtl_uString* p = it->pData;
        if (getRefCount(p) == 1)
        {
            // Remove it from the upper string map.  This should unref the
            // upper string linked to this original string.
            maToUpperMap.erase(p);
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

size_t StringPool::getCount() const
{
    return maStrPool.size();
}

size_t StringPool::getCountIgnoreCase() const
{
    return maStrPoolUpper.size();
}

StringPool::InsertResultType StringPool::findOrInsert( StrHashType& rPool, const OUString& rStr ) const
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
