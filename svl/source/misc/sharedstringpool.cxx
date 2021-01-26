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

/** create a key class that caches the hashcode */
namespace
{
struct StringWithHash
{
    OUString str;
    sal_Int32 hashCode;
    StringWithHash(OUString s)
        : str(s)
        , hashCode(s.hashCode())
    {
    }

    bool operator==(StringWithHash const& rhs) const
    {
        if (hashCode != rhs.hashCode)
            return false;
        return str == rhs.str;
    }
};
}

namespace std
{
template <> struct hash<StringWithHash>
{
    std::size_t operator()(const StringWithHash& k) const { return k.hashCode; }
};
}

namespace svl
{
namespace
{
sal_Int32 getRefCount(const rtl_uString* p) { return (p->refCount & 0x3FFFFFFF); }
}

struct SharedStringPool::Impl
{
    mutable osl::Mutex maMutex;
    // We use this map for two purposes - to store lower->upper case mappings
    // and to retrieve a shared uppercase object, so the management logic
    // is quite complex.
    std::unordered_map<StringWithHash, OUString> maStrMap;
    const CharClass& mrCharClass;

    explicit Impl(const CharClass& rCharClass)
        : mrCharClass(rCharClass)
    {
    }
};

SharedStringPool::SharedStringPool(const CharClass& rCharClass)
    : mpImpl(new Impl(rCharClass))
{
}

SharedStringPool::~SharedStringPool() {}

SharedString SharedStringPool::intern(const OUString& rStr)
{
    StringWithHash aStrWithHash(rStr);
    osl::MutexGuard aGuard(&mpImpl->maMutex);

    auto[mapIt, bInserted] = mpImpl->maStrMap.emplace(aStrWithHash, rStr);
    if (!bInserted)
        // there is already a mapping
        return SharedString(mapIt->first.str.pData, mapIt->second.pData);

    // This is a new string insertion. Establish mapping to upper-case variant.
    OUString aUpper = mpImpl->mrCharClass.uppercase(rStr);
    if (aUpper == rStr)
        // no need to do anything more, because we inserted an upper->upper mapping
        return SharedString(mapIt->first.str.pData, mapIt->second.pData);

    // We need to insert a lower->upper mapping, so also insert
    // an upper->upper mapping, which we can use both for when an upper string
    // is interned, and to look up a shared upper string.
    StringWithHash aUpperWithHash(aUpper);
    auto mapIt2 = mpImpl->maStrMap.find(aUpperWithHash);
    if (mapIt2 != mpImpl->maStrMap.end())
    {
        // there is an already existing upper string
        mapIt->second = mapIt2->first.str;
        return SharedString(mapIt->first.str.pData, mapIt->second.pData);
    }

    // There is no already existing upper string.
    // First, update using the iterator, can't do this later because
    // the iterator will be invalid.
    mapIt->second = aUpper;
    mpImpl->maStrMap.emplace_hint(mapIt2, aUpperWithHash, aUpper);
    return SharedString(rStr.pData, aUpper.pData);
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
        rtl_uString* p1 = it->first.str.pData;
        rtl_uString* p2 = it->second.pData;
        if (p1 != p2)
        {
            // normal case - lowercase mapped to uppercase, which
            // means that the lowercase entry has one ref-counted
            // entry as the key in the map
            if (getRefCount(p1) == 1)
            {
                it = mpImpl->maStrMap.erase(it);
                continue;
            }
        }
        ++it;
    }

    it = mpImpl->maStrMap.begin();
    itEnd = mpImpl->maStrMap.end();
    while (it != itEnd)
    {
        rtl_uString* p1 = it->first.str.pData;
        rtl_uString* p2 = it->second.pData;
        if (p1 == p2)
        {
            // uppercase which is mapped to itself, which means
            // one ref-counted entry as the key in the map, and
            // one ref-counted entry in the value in the map
            if (getRefCount(p1) == 2)
            {
                it = mpImpl->maStrMap.erase(it);
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
    // this is only called from unit tests, so no need to be efficient
    std::unordered_set<OUString> aUpperSet;
    for (auto const& pair : mpImpl->maStrMap)
        aUpperSet.insert(pair.second);
    return aUpperSet.size();
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
