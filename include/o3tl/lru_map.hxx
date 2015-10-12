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

#include <list>
#include <unordered_map>

namespace o3tl
{

/** LRU map
 *
 * Similar to unordered_map (it actually uses it) with additionaly functionality
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
template<typename Key, typename Value, class KeyHash = std::hash<Key>>
class lru_map final
{
private:
    typedef typename std::pair<Key, Value> key_value_pair_t;
    typedef std::list<key_value_pair_t> list_t;
    typedef typename list_t::iterator list_iterator_t;
    typedef typename list_t::const_iterator list_const_iterator_t;

    typedef std::unordered_map<Key, list_iterator_t, KeyHash> map_t;
    typedef typename map_t::iterator map_iterator_t;
    typedef typename map_t::const_iterator map_const_iterator_t;

    list_t mLruList;
    map_t mLruMap;
    const size_t mMaxSize;

    inline void checkLRU()
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

    lru_map(size_t nMaxSize)
        : mMaxSize(nMaxSize)
    {}

    void insert(key_value_pair_t& rPair)
    {
        map_iterator_t iterator = mLruMap.find(rPair.first);

        if (iterator == mLruMap.end()) // doesn't exist -> add to queue and map
        {
            // add to front of the list
            mLruList.push_front(rPair);
            // add the list position (iterator) to the map
            mLruMap[rPair.first] = mLruList.begin();
            checkLRU();
        }
        else // already exists -> replace value
        {
            // replace value
            iterator->second->second = rPair.second;
            // bring to front of the lru list
            mLruList.splice(mLruList.begin(), mLruList, iterator->second);
        }
    }

    void insert(key_value_pair_t&& rPair)
    {
        map_iterator_t iterator = mLruMap.find(rPair.first);

        if (iterator == mLruMap.end()) // doesn't exist -> add to list and map
        {
            // add to front of the list
            mLruList.push_front(std::move(rPair));
            // add the list position (iterator) to the map
            mLruMap[rPair.first] = mLruList.begin();
            checkLRU();
        }
        else // already exists -> replace value
        {
            // replace value
            iterator->second->second = std::move(rPair.second);
            // push to back of the lru list
            mLruList.splice(mLruList.begin(), mLruList, iterator->second);
        }
    }

    const list_const_iterator_t find(const Key& key)
    {
        const map_iterator_t iterator = mLruMap.find(key);
        if (iterator == mLruMap.cend()) // can't find entry for the key
        {
            // return empty iterator
            return mLruList.cend();
        }
        else
        {
            // push to back of the lru list
            mLruList.splice(mLruList.begin(), mLruList, iterator->second);
            return iterator->second;
        }
    }

    const list_const_iterator_t end() const
    {
        return mLruList.end();
    }

    size_t size() const
    {
        return mLruList.size();
    }
};

}

#endif /* INCLUDED_O3TL_LRU_MAP_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
