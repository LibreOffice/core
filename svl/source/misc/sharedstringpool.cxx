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

sal_Int32 getRefCount( const rtl_uString* p )
{
    return (p->refCount & 0x3FFFFFFF);
}

}

struct SharedStringPool::Impl
{
    mutable osl::Mutex maMutex;
    // set of upper-case, so we can share these as the value in the maStrMap
    std::unordered_set<OUString> maStrPoolUpper;
    // map with rtl_uString* as key so we can avoid some ref-counting
    // (but it still is ref-counted to know when to remove from uppercase pool)
    std::unordered_map<OUString,rtl_uString*> maStrMap;
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

    auto [mapIt,bInserted] = mpImpl->maStrMap.emplace(rStr, rStr.pData);
    if (bInserted)
    {
        // This is a new string insertion. Establish mapping to upper-case variant.
        OUString aUpper = mpImpl->mrCharClass.uppercase(rStr);
        if (aUpper == rStr)
        {
            auto insertResult = mpImpl->maStrPoolUpper.insert(rStr);
            mapIt->second = insertResult.first->pData;
            // refcount the shared uppercase (unless already refcounted by the OUString)
            if (mapIt->second != mapIt->first.pData)
                rtl_uString_acquire(mapIt->second);
        }
        else
        {
            auto insertResult = mpImpl->maStrPoolUpper.insert(aUpper);
            mapIt->second = insertResult.first->pData;
            rtl_uString_acquire(mapIt->second);
        }
    }
    return SharedString(mapIt->first.pData, mapIt->second);
}

void SharedStringPool::purge()
{
    osl::MutexGuard aGuard(&mpImpl->maMutex);

    {
        auto it = mpImpl->maStrMap.begin(), itEnd = mpImpl->maStrMap.end();
        while (it != itEnd)
        {
            const rtl_uString* p = it->first.pData;
            // If the string itself is uppercase, it is refcounted
            // both by the item and the uppercase pool.
            const int unusedRefCount = (p == it->second) ? 2 : 1;
            if (getRefCount(p) == unusedRefCount)
            {
                if (p != it->second)
                    rtl_uString_release(it->second);
                assert(getRefCount(it->second) >= unusedRefCount);
                if (getRefCount(it->second) == unusedRefCount)
                    mpImpl->maStrPoolUpper.erase(it->second);
                it = mpImpl->maStrMap.erase(it);
            }
            else
            {
                // Still referenced outside the pool. Keep it.
                ++it;
            }
        }
    }
}

size_t SharedStringPool::getCount() const
{
    osl::MutexGuard aGuard(&mpImpl->maMutex);
    return mpImpl->maStrMap.size();
}

size_t SharedStringPool::getCountIgnoreCase() const
{
    osl::MutexGuard aGuard(&mpImpl->maMutex);
    return mpImpl->maStrPoolUpper.size();
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
