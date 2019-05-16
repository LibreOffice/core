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

    std::unordered_set<OUString> aNewStrPoolUpper;
    {
        auto it = mpImpl->maStrMap.begin(), itEnd = mpImpl->maStrMap.end();
        while (it != itEnd)
        {
            const rtl_uString* p = it->first.pData;
            if (getRefCount(p) == 1)
                it = mpImpl->maStrMap.erase(it);
            else
            {
                // Still referenced outside the pool. Keep it.
                aNewStrPoolUpper.insert(it->second);
                ++it;
            }
        }
    }
    mpImpl->maStrPoolUpper = std::move(aNewStrPoolUpper);
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
