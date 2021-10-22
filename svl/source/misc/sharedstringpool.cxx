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

#include <mutex>
#include <unordered_map>
#include <unordered_set>

#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wshadow"
#endif
#if defined __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-parameter"
#endif
#if defined _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4324) // structure was padded due to alignment specifier
#endif
#include <libcuckoo/cuckoohash_map.hh>
#if defined _MSC_VER
#pragma warning(pop)
#endif
#if defined __clang__
#pragma clang diagnostic pop
#endif
#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif

namespace svl
{
namespace
{
sal_Int32 getRefCount(const rtl_uString* p) { return (p->refCount & 0x3FFFFFFF); }

// we store the key twice, because the concurrent hashtable we are using does not provide any way to return the key in use
typedef std::pair<OUString, OUString> Mapped;

struct HashFunction
{
    size_t operator()(rtl_uString* const key) const
    {
        return rtl_ustr_hashCode_WithLength(key->buffer, key->length);
    }
};

struct EqualsFunction
{
    bool operator()(rtl_uString* const lhs, rtl_uString* const rhs) const
    {
        return OUString::unacquired(&lhs) == OUString::unacquired(&rhs);
    }
};
}

typedef libcuckoo::cuckoohash_map<rtl_uString*, Mapped, HashFunction, EqualsFunction>
    SharedStringMap;

struct SharedStringPool::Impl
{
    // We use this map for two purposes - to store lower->upper case mappings
    // and to store an upper->upper mapping.
    // The second mapping is used so that we can
    // share the same rtl_uString object between different keys which map to the same uppercase string to save memory.
    //
    // Docs for this concurrent hashtable here: http://efficient.github.io/libcuckoo/classlibcuckoo_1_1cuckoohash__map.html
    SharedStringMap maStrMap;
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
    auto& rMap = mpImpl->maStrMap;

    rtl_uString *pResultLower, *pResultUpper;
    if (rMap.find_fn(rStr.pData, [&](const Mapped& rMapped) {
            pResultLower = rMapped.first.pData;
            pResultUpper = rMapped.second.pData;
        }))
        // there is already a mapping
        return SharedString(pResultLower, pResultUpper);

    // This is a new string insertion. Establish mapping to upper-case variant.
    OUString aUpper = mpImpl->mrCharClass.uppercase(rStr);

    // either insert a new upper->upper mapping, or write the existing mapping into aUpper
    mpImpl->maStrMap.uprase_fn(aUpper.pData,
                               [&](Mapped& mapped) -> bool {
                                   aUpper = mapped.second;
                                   return false;
                               },
                               aUpper, aUpper);

    if (aUpper == rStr)
        // no need to do anything more, because the key is already uppercase
        return SharedString(aUpper.pData, aUpper.pData);

    // either insert a new lower->upper mapping, or write the existing mapping into aLower
    if (mpImpl->maStrMap.uprase_fn(rStr.pData,
                                   [&](Mapped& mapped) -> bool {
                                       pResultLower = mapped.first.pData;
                                       pResultUpper = mapped.second.pData;
                                       return false;
                                   },
                                   rStr, aUpper))
    {
        pResultLower = rStr.pData;
        pResultUpper = aUpper.pData;
    }

    return SharedString(pResultLower, pResultUpper);
}

void SharedStringPool::purge()
{
    SharedStringMap::locked_table locked_table = mpImpl->maStrMap.lock_table();

    // Because we can have an uppercase entry mapped to itself,
    // and then a bunch of lowercase entries mapped to that same
    // upper-case entry, we need to scan the map twice - the first
    // time to remove lowercase entries, and then only can we
    // check for unused uppercase entries.

    auto it = locked_table.begin();
    auto itEnd = locked_table.end();
    while (it != itEnd)
    {
        rtl_uString* p1 = it->second.first.pData;
        rtl_uString* p2 = it->second.second.pData;
        if (p1 != p2)
        {
            // normal case - lowercase mapped to uppercase, which
            // means that the lowercase entry has one ref-counted
            // entry as the key in the map
            if (getRefCount(p1) == 1)
            {
                it = locked_table.erase(it);
                continue;
            }
        }
        ++it;
    }

    it = locked_table.begin();
    itEnd = locked_table.end();
    while (it != itEnd)
    {
        rtl_uString* p1 = it->second.first.pData;
        rtl_uString* p2 = it->second.second.pData;
        if (p1 == p2)
        {
            // uppercase which is mapped to itself, which means
            // one ref-counted entry as the key in the map, and
            // one ref-counted entry in the value in the map
            if (getRefCount(p1) == 2)
            {
                it = locked_table.erase(it);
                continue;
            }
        }
        ++it;
    }
}

size_t SharedStringPool::getCount() const { return mpImpl->maStrMap.size(); }

size_t SharedStringPool::getCountIgnoreCase() const
{
    // this is only called from unit tests, so no need to be efficient
    std::unordered_set<OUString> aUpperSet;
    SharedStringMap::locked_table locked_table = mpImpl->maStrMap.lock_table();
    for (auto const& pair : locked_table)
        aUpperSet.insert(pair.second.second);
    return aUpperSet.size();
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
