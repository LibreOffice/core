/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#pragma once

#include <cassert>
#include <list>
#include <version>
#if defined __cpp_lib_memory_resource
#include <memory_resource>
#endif
#include <unordered_map>
#include <cstddef>

namespace o3tl
{
namespace detail
{
// Helper base class to keep total cost for lru_map with custom item size.
// Custom size is specified by the ValueSize functor, the default of each
// item counting as 1 is specified using the void type.
template <class ValueSize> class lru_map_base
{
public:
    // Returns total of ValueSize for all items.
    size_t total_size() const { return mCurrentSize; }

protected:
    size_t mCurrentSize = 0; // sum of ValueSize for all items
};

// By default cost of each item is 1, so it doesn't need to be tracked.
template <> class lru_map_base<void>
{
};
} // namespace

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
 * It is optionally possible to specify a function for ValueSize template
 * argument (that can be called as 'size_t func(Value)') that will return
 * a size (cost) for an item instead of the default size of 1 for each item.
 * The size of an item must not change for an item (if needed, re-insert
 * the item). A newly inserted item is guaranteed to be in the container,
 * even if its size exceeds the maximum size.
 *
 **/
template <typename Key, typename Value, class KeyHash = std::hash<Key>,
          class KeyEqual = std::equal_to<Key>, class ValueSize = void>
class lru_map final : public detail::lru_map_base<ValueSize>
{
public:
    typedef typename std::pair<Key, Value> key_value_pair_t;

private:
#if defined __cpp_lib_memory_resource
    typedef std::pmr::list<key_value_pair_t> list_t;
#else
    typedef std::list<key_value_pair_t> list_t;
#endif
    typedef typename list_t::iterator list_iterator_t;
    typedef typename list_t::const_iterator list_const_iterator_t;

#if defined __cpp_lib_memory_resource
    typedef std::pmr::unordered_map<Key, list_iterator_t, KeyHash, KeyEqual> map_t;
#else
    typedef std::unordered_map<Key, list_iterator_t, KeyHash, KeyEqual> map_t;
#endif
    typedef typename map_t::iterator map_iterator_t;
    typedef typename map_t::const_iterator map_const_iterator_t;

    list_t mLruList;
    map_t mLruMap;
    size_t mMaxSize;

    void addSize(const Value& value)
    {
        // by default total size is equal to number of items
        if constexpr (!std::is_void_v<ValueSize>)
            this->mCurrentSize += ValueSize()(value);
    }

    void removeSize(const Value& value)
    {
        // by default total size is equal to number of items
        if constexpr (!std::is_void_v<ValueSize>)
        {
            size_t itemSize = ValueSize()(value);
            assert(itemSize <= this->mCurrentSize);
            this->mCurrentSize -= itemSize;
        }
    }

    void removeOldestItem()
    {
        removeSize(mLruList.back().second);
        // remove from map
        mLruMap.erase(mLruList.back().first);
        // remove from list
        mLruList.pop_back();
    }

    void checkLRUItemInsert()
    {
        if constexpr (std::is_void_v<ValueSize>)
        { // One added, so it's enough to remove one, if needed.
            if (mLruMap.size() > mMaxSize)
                removeOldestItem();
        }
        else
        {
            // This must leave at least one item (it's called from insert).
            while (this->mCurrentSize > mMaxSize && mLruMap.size() > 1)
                removeOldestItem();
        }
    }

    void checkLRUItemUpdate()
    {
        // Item update does not change total size by default.
        if constexpr (!std::is_void_v<ValueSize>)
        {
            // This must leave at least one item (it's called from insert).
            while (this->mCurrentSize > mMaxSize && mLruMap.size() > 1)
                removeOldestItem();
        }
    }

    void checkLRUMaxSize()
    {
        if constexpr (std::is_void_v<ValueSize>)
        {
            while (mLruMap.size() > mMaxSize)
                removeOldestItem();
        }
        else
        {
            while (this->mCurrentSize > mMaxSize)
                removeOldestItem();
        }
    }

    void clearSize()
    {
        if constexpr (!std::is_void_v<ValueSize>)
        {
#ifdef DBG_UTIL
            for (const key_value_pair_t& item : mLruList)
                removeSize(item.second);
            assert(this->mCurrentSize == 0);
#else
            this->mCurrentSize = 0;
#endif
        }
    }

public:
    typedef list_iterator_t iterator;
    typedef list_const_iterator_t const_iterator;

    lru_map(size_t nMaxSize)
        : mMaxSize(nMaxSize)
    {
        assert(mMaxSize > 0);
    }
#if defined __cpp_lib_memory_resource
    lru_map(size_t nMaxSize, std::pmr::memory_resource* r)
        : mLruList(r)
        , mLruMap(r)
        , mMaxSize(nMaxSize)
    {
        assert(mMaxSize > 0);
    }
#endif
    ~lru_map()
    {
        clearSize();
        // Some code .e.g. SalBitmap likes to remove itself from a cache during it's destructor, which means we
        // get calls into lru_map while we are in destruction, so use the swap-and-clear idiom to avoid those problems.
        mLruMap.clear();
        list_t(mLruList.get_allocator()).swap(mLruList);
    }

    void setMaxSize(size_t nMaxSize)
    {
        mMaxSize = nMaxSize;
        assert(mMaxSize > 0);
        checkLRUMaxSize();
    }

    void insert(key_value_pair_t& rPair)
    {
        map_iterator_t i = mLruMap.find(rPair.first);

        if (i == mLruMap.end()) // doesn't exist -> add to queue and map
        {
            addSize(rPair.second);
            // add to front of the list
            mLruList.push_front(rPair);
            // add the list position (iterator) to the map
            auto it = mLruList.begin();
            mLruMap[it->first] = it;
            checkLRUItemInsert();
        }
        else // already exists -> replace value
        {
            // update total cost
            removeSize(i->second->second);
            addSize(rPair.second);
            // replace value
            i->second->second = rPair.second;
            // bring to front of the lru list
            mLruList.splice(mLruList.begin(), mLruList, i->second);
            checkLRUItemUpdate();
        }
    }

    void insert(key_value_pair_t&& rPair)
    {
        map_iterator_t i = mLruMap.find(rPair.first);

        if (i == mLruMap.end()) // doesn't exist -> add to list and map
        {
            addSize(rPair.second);
            // add to front of the list
            mLruList.push_front(std::move(rPair));
            // add the list position (iterator) to the map
            auto it = mLruList.begin();
            mLruMap[it->first] = it;
            checkLRUItemInsert();
        }
        else // already exists -> replace value
        {
            removeSize(i->second->second);
            addSize(rPair.second);
            // replace value
            i->second->second = std::move(rPair.second);
            // push to back of the lru list
            mLruList.splice(mLruList.begin(), mLruList, i->second);
            checkLRUItemUpdate();
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
                removeSize(it->second);
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

    // size_t total_size() const; - only if custom ValueSize

    void clear()
    {
        clearSize();
        map_t(mLruMap.get_allocator()).swap(mLruMap);
        list_t(mLruList.get_allocator()).swap(mLruList);
    }
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
