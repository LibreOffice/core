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



#ifndef SC_AUTONAMECACHE_HXX
#define SC_AUTONAMECACHE_HXX

#include <vector>
#include <hash_map>
#include "address.hxx"
#include "global.hxx"

typedef ::std::vector< ScAddress > ScAutoNameAddresses;
typedef ::std::hash_map< String, ScAutoNameAddresses, ScStringHashCode, ::std::equal_to< String > > ScAutoNameHashMap;

//
//  Cache for faster lookup of automatic names during CompileXML
//  (during CompileXML, no document content is changed)
//

class ScAutoNameCache
{
    ScAutoNameHashMap   aNames;
    ScDocument*         pDoc;
    SCTAB               nCurrentTab;

public:
            ScAutoNameCache( ScDocument* pD );
            ~ScAutoNameCache();

    const ScAutoNameAddresses& GetNameOccurences( const String& rName, SCTAB nTab );
};

#endif

