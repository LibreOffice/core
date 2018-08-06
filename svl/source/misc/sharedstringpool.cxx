/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <svl/sharedstringpool.hxx>
#include <svl/sharedstring.hxx>
#include <unotools/charclass.hxx>
#include <osl/mutex.hxx>

#include <unordered_map>
#include <unordered_set>

namespace svl {

namespace {

inline sal_Int32 getRefCount( const rtl_uString* p )
{
    return (p->refCount & 0x3FFFFFFF);
}

typedef std::unordered_set<OUString> StrHashType;
typedef std::pair<StrHashType::iterator, bool> InsertResultType;
typedef std::unordered_map<const rtl_uString*, OUString> StrStoreType;

InsertResultType findOrInsert( StrHashType& rPool, const OUString& rStr )
{
    StrHashType::iterator it = rPool.find(rStr);
    bool bInserted = false;
    if (it == rPool.end())
    {
        // Not yet in the pool.
        std::pair<StrHashType::iterator, bool> r = rPool.insert(rStr);
        assert(r.second);
        it = r.first;
        bInserted = true;
    }

    return InsertResultType(it, bInserted);
}

}

struct SharedStringPool::Impl
{
    mutable osl::Mutex maMutex;
    StrHashType maStrPool;
    StrHashType maStrPoolUpper;
    StrStoreType maStrStore;
    const CharClass& mrCharClass;

    explicit Impl( const CharClass& rCharClass ) : mrCharClass(rCharClass) {}
};

SharedStringPool::SharedStringPool( const CharClass& rCharClass ) :
    mpImpl(new Impl(rCharClass)) {}

SharedStringPool::~SharedStringPool()
{
}

SharedString SharedStringPool::intern( const OUString& rStr )
{
    osl::MutexGuard aGuard(&mpImpl->maMutex);

    InsertResultType aRes = findOrInsert(mpImpl->maStrPool, rStr);

    rtl_uString* pOrig = aRes.first->pData;

    if (!aRes.second)
    {
        // No new string has been inserted. Return the existing string in the pool.
        StrStoreType::const_iterator it = mpImpl->maStrStore.find(pOrig);
        assert(it != mpImpl->maStrStore.end());

        rtl_uString* pUpper = it->second.pData;
        return SharedString(pOrig, pUpper);
    }

    // This is a new string insertion. Establish mapping to upper-case variant.

    OUString aUpper = mpImpl->mrCharClass.uppercase(rStr);
    aRes = findOrInsert(mpImpl->maStrPoolUpper, aUpper);
    assert(aRes.first != mpImpl->maStrPoolUpper.end());

    mpImpl->maStrStore.emplace(pOrig, *aRes.first);

    return SharedString(pOrig, aRes.first->pData);
}

void SharedStringPool::purge()
{
    osl::MutexGuard aGuard(&mpImpl->maMutex);

    StrHashType aNewStrPool;
    StrHashType::iterator it = mpImpl->maStrPool.begin(), itEnd = mpImpl->maStrPool.end();
    for (; it != itEnd; ++it)
    {
        const rtl_uString* p = it->pData;
        if (getRefCount(p) == 1)
        {
            // Remove it from the upper string map.  This should unref the
            // upper string linked to this original string.
            mpImpl->maStrStore.erase(p);
        }
        else
            // Still referenced outside the pool. Keep it.
            aNewStrPool.insert(*it);
    }

    mpImpl->maStrPool.swap(aNewStrPool);

    aNewStrPool.clear(); // for re-use.

    // Purge the upper string pool as well.
    it = mpImpl->maStrPoolUpper.begin();
    itEnd = mpImpl->maStrPoolUpper.end();
    for (; it != itEnd; ++it)
    {
        const rtl_uString* p = it->pData;
        if (getRefCount(p) > 1)
            aNewStrPool.insert(*it);
    }

    mpImpl->maStrPoolUpper.swap(aNewStrPool);
}

size_t SharedStringPool::getCount() const
{
    osl::MutexGuard aGuard(&mpImpl->maMutex);
    return mpImpl->maStrPool.size();
}

size_t SharedStringPool::getCountIgnoreCase() const
{
    osl::MutexGuard aGuard(&mpImpl->maMutex);
    return mpImpl->maStrPoolUpper.size();
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
