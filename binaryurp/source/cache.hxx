/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#ifndef INCLUDED_BINARYURP_SOURCE_CACHE_HXX
#define INCLUDED_BINARYURP_SOURCE_CACHE_HXX

#include "sal/config.h"

#include <cstddef>
#include <list>
#ifdef USE_UNORDERED_MAP
    #include <unordered_map>
#else
    #include <map>
#endif

#include "boost/noncopyable.hpp"
#include "osl/diagnose.h"
#include "sal/types.h"

namespace binaryurp {

namespace cache {

enum { size = 256, ignore = 0xFFFF };

}

template< typename T > class Cache : private boost::noncopyable {
public:
    typedef sal_uInt16 IdxType;

    explicit Cache(std::size_t size):
        size_(size)
    {
        OSL_ASSERT(size < cache::ignore);
    }

    IdxType add( const T& rContent, bool* pbFound) {
    OSL_ASSERT( pbFound != NULL);
    if( !size_) {
        *pbFound = false;
        return cache::ignore;
    }
    // try to insert into the map
    list_.push_front( rContent); // create a temp entry
    typedef std::pair<typename LruList::iterator, IdxType> MappedType;
    typedef std::pair<typename LruItMap::iterator,bool> MapPair;
    MapPair aMP = map_.insert( MappedType( list_.begin(), 0));
    *pbFound = !aMP.second;

    if( !aMP.second) { // insertion not needed => found the entry
        list_.pop_front(); // remove the temp entry
        list_.splice( list_.begin(), list_, aMP.first->first); // the found entry is moved to front
        return aMP.first->second;
    }

    // test insertion successful => it was new so we keep it
    IdxType n = static_cast<IdxType>( map_.size() - 1);
    if( n >= size_) { // cache full => replace the LRU entry
        // find the least recently used element in the map
        typename LruItMap::iterator it = map_.find( --list_.end());
        n = it->second;
        map_.erase( it); // remove it from the map
        list_.pop_back(); // remove from the list
    }
    aMP.first->second = n;
    return n;
    }

private:
    typedef std::list<T> LruList; // last recently used list
    typedef typename LruList::iterator LruListIt;
#ifdef URPCACHE_USES_UNORDERED_MAP
    struct HashT{ size_t operator()( const LruListIt& rA) const { return hash(*rA;);};
    struct EqualT{ bool operator()( const LruListIt& rA, const LruListIt& rB) const { return *rA==*rB;}};
    typedef ::std::unordered_map< LruListIt, IdxType, HashT, EqualT > LruItMap; // a map into a LruList
#else
    struct CmpT{ bool operator()( const LruListIt& rA, const LruListIt& rB) const { return (*rA<*rB);}};
    typedef ::std::map< LruListIt, IdxType, CmpT > LruItMap; // a map into a LruList
#endif

    std::size_t size_;
    LruItMap map_;
    LruList list_;
};

}

#endif

