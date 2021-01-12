/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#ifndef INCLUDED_O3TL_LRU_MAP_HXX
#define INCLUDED_O3TL_LRU_MAP_HXX

#include <algorithm>
#include <cassert>
#include <list>
#include <unordered_map>
#include <cstddef>

namespace o3tl
{
/** LRU map
 *
 * Similar to unordered_map (it actually uses it) with additionally functionality
 * which removes the entries that have been "least recently used" when the size
 * hits the specified capacity.
 *
 * It only implements the minimal methods needed and the implementation is NOT
 * thread safe.
 *
 * The implementation is as simple as possible but it still uses O(1) complexity
 * for most of the operations with a combination unordered map and linked list.
 *
 **/
template <typename Key, typename Value, class KeyHash = std::hash<Key>,
          class KeyEqual = std::equal_to<Key>>
class lru_map final
{
public:
    typedef typename std::pair<Key, Value> key_value_pair_t;

private:
    typedef std::list<key_value_pair_t> list_t;
    typedef typename list_t::iterator list_iterator_t;
    typedef typename list_t::const_iterator list_const_iterator_t;

    typedef std::unordered_map<Key, list_iterator_t, KeyHash, KeyEqual> map_t;
    typedef typename map_t::iterator map_iterator_t;
    typedef typename map_t::const_iterator map_const_iterator_t;

    list_t mLruList;
    map_t mLruMap;
    const size_t mMaxSize;

    void checkLRU()
    {
        if (mLruMap.size() > mMaxSize)
        {
            // remove from map
            mLruMap.erase(mLruList.back().first);
            // remove from list
            mLruList.pop_back();
        }
    }

public:
    typedef list_iterator_t iterator;
    typedef list_const_iterator_t const_iterator;

    // a size of 0 effectively disables the LRU cleanup code
    lru_map(size_t nMaxSize)
        : mMaxSize(nMaxSize ? nMaxSize : std::min(mLruMap.max_size(), mLruList.max_size()))
    {
    }
    ~lru_map()
    {
        // Some code .e.g. SalBitmap likes to remove itself from a cache during it's destructor, which means we
        // get calls into lru_map while we are in destruction, so use the swap-and-clear idiom to avoid those problems.
        mLruMap.clear();
        list_t aLruListTemp;
        aLruListTemp.swap(mLruList);
    }

    void insert(key_value_pair_t& rPair)
    {
        map_iterator_t i = mLruMap.find(rPair.first);

        if (i == mLruMap.end()) // doesn't exist -> add to queue and map
        {
            // add to front of the list
            mLruList.push_front(rPair);
            // add the list position (iterator) to the map
            auto it = mLruList.begin();
            mLruMap[it->first] = it;
            checkLRU();
        }
        else // already exists -> replace value
        {
            // replace value
            i->second->second = rPair.second;
            // bring to front of the lru list
            mLruList.splice(mLruList.begin(), mLruList, i->second);
        }
    }

    void insert(key_value_pair_t&& rPair)
    {
        map_iterator_t i = mLruMap.find(rPair.first);

        if (i == mLruMap.end()) // doesn't exist -> add to list and map
        {
            // add to front of the list
            mLruList.push_front(std::move(rPair));
            // add the list position (iterator) to the map
            auto it = mLruList.begin();
            mLruMap[it->first] = it;
            checkLRU();
        }
        else // already exists -> replace value
        {
            // replace value
            i->second->second = std::move(rPair.second);
            // push to back of the lru list
            mLruList.splice(mLruList.begin(), mLruList, i->second);
        }
    }

    list_const_iterator_t find(const Key& key)
    {
        const map_iterator_t i = mLruMap.find(key);
        if (i == mLruMap.cend()) // can't find entry for the key
        {
            // return empty iterator
            return mLruList.cend();
        }
        else
        {
            // push to back of the lru list
            mLruList.splice(mLruList.begin(), mLruList, i->second);
            return i->second;
        }
    }

    // reverse-iterates the list removing all items matching the predicate
    template <class UnaryPredicate> void remove_if(UnaryPredicate pred)
    {
        auto it = mLruList.rbegin();
        while (it != mLruList.rend())
        {
            if (pred(*it))
            {
                mLruMap.erase(it->first);
                it = decltype(it){ mLruList.erase(std::next(it).base()) };
            }
            else
                ++it;
        }
    }

    list_const_iterator_t begin() const { return mLruList.cbegin(); }

    list_const_iterator_t end() const { return mLruList.cend(); }

    size_t size() const
    {
        assert(mLruMap.size() == mLruList.size());
        return mLruMap.size();
    }

    void clear()
    {
        mLruMap.clear();
        mLruList.clear();
    }
};
}

#endif /* INCLUDED_O3TL_LRU_MAP_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
