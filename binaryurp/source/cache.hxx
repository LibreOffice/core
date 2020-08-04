/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#pragma once

#include <sal/config.h>

#include <cassert>
#include <cstddef>
#include <map>
#include <list>

#include <sal/types.h>

namespace binaryurp {

namespace cache {

enum { size = 256, ignore = 0xFFFF };

}

template< typename T > class Cache {
public:
    typedef sal_uInt16 IdxType;

    explicit Cache(std::size_t size):
        size_(size)
    {
        assert(size < cache::ignore);
    }

    IdxType add( const T& rContent, bool* pbFound) {
        assert( pbFound != nullptr);
        if( !size_) {
            *pbFound = false;
            return cache::ignore;
        }
        // try to insert into the map
        list_.push_front( rContent); // create a temp entry
        auto const [it, inserted] = map_.emplace( list_.begin(), 0 );
        *pbFound = !inserted;

        if( !inserted) { // insertion not needed => found the entry
            list_.pop_front(); // remove the temp entry
            list_.splice( list_.begin(), list_, it->first); // the found entry is moved to front
            return it->second;
        }

        // test insertion successful => it was new so we keep it
        IdxType n = static_cast<IdxType>( map_.size() - 1);
        if( n >= size_) { // cache full => replace the LRU entry
            // find the least recently used element in the map
            typename LruItMap::iterator lru = map_.find( --list_.end());
            n = lru->second;
            map_.erase( lru); // remove it from the map
            list_.pop_back(); // remove from the list
        }
        it->second = n;
        return n;
    }

private:
    Cache(const Cache&) = delete;
    Cache& operator=(const Cache&) = delete;

    typedef std::list<T> LruList; // last recently used list
    typedef typename LruList::iterator LruListIt;
    struct CmpT{ bool operator()( const LruListIt& rA, const LruListIt& rB) const { return (*rA<*rB);}};
    typedef std::map< LruListIt, IdxType, CmpT > LruItMap; // a map into a LruList

    std::size_t size_;
    LruItMap map_;
    LruList list_;
};

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
