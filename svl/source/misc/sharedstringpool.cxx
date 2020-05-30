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
    // map with rtl_uString* as value so we can avoid some ref-counting
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
            // need to use the same underlying rtl_uString object so the
            // upper->upper detection in purge() works
            auto pData = insertResult.first->pData;
            mpImpl->maStrMap.insert_or_assign(mapIt, pData, pData);
        }
        else
        {
            auto insertResult = mpImpl->maStrPoolUpper.insert(aUpper);
            mapIt->second = insertResult.first->pData;
        }
    }
    return SharedString(mapIt->first.pData, mapIt->second);
}

void SharedStringPool::purge()
{
    osl::MutexGuard aGuard(&mpImpl->maMutex);

    // Because we can have an uppercase entry mapped to itself,
    // and then a bunch of lowercase entries mapped to that same
    // upper-case entry, we need to scan the map twice - the first
    // time to remove lowercase entries, and then only can we
    // check for unused uppercase entries.

    auto it = mpImpl->maStrMap.begin();
    auto itEnd = mpImpl->maStrMap.end();
    while (it != itEnd)
    {
        rtl_uString* p1 = it->first.pData;
        rtl_uString* p2 = it->second;
        if (p1 != p2)
        {
            // normal case - lowercase mapped to uppercase, which
            // means that the lowercase entry has one ref-counted
            // entry as the key in the map
            if (getRefCount(p1) == 1)
            {
                it = mpImpl->maStrMap.erase(it);
                // except that the uppercase entry may be mapped to
                // by other lower-case entries
                if (getRefCount(p2) == 1)
                    mpImpl->maStrPoolUpper.erase(OUString::unacquired(&p2));
                continue;
            }
        }
        ++it;
    }

    it = mpImpl->maStrMap.begin();
    itEnd = mpImpl->maStrMap.end();
    while (it != itEnd)
    {
        rtl_uString* p1 = it->first.pData;
        rtl_uString* p2 = it->second;
        if (p1 == p2)
        {
            // uppercase which is mapped to itself, which means
            // one ref-counted entry as the key in the map, and
            // one ref-counted entry in the set
            if (getRefCount(p1) == 2)
            {
                it = mpImpl->maStrMap.erase(it);
                mpImpl->maStrPoolUpper.erase(OUString::unacquired(&p1));
                continue;
            }
        }
        ++it;
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
