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

#if ! defined INCLUDED_DP_PERSMAP_H
#define INCLUDED_DP_PERSMAP_H

#include "rtl/ustring.hxx"
#include "osl/file.hxx"
#include <hash_map>

namespace dp_misc
{

typedef ::std::hash_map<
    ::rtl::OString, ::rtl::OString, ::rtl::OStringHash > t_string2string_map;

//==============================================================================
class PersistentMap
{
    ::osl::File m_MapFile;
    t_string2string_map m_entries;
    bool m_bReadOnly;
    bool m_bIsOpen;
    bool m_bToBeCreated;
    bool m_bIsDirty;

public:
    ~PersistentMap();
    PersistentMap( ::rtl::OUString const & url, bool readOnly );
    /** in mem db */
    PersistentMap();

    bool has( ::rtl::OString const & key ) const;
    bool get( ::rtl::OString * value, ::rtl::OString const & key ) const;
    t_string2string_map getEntries() const;
    void put( ::rtl::OString const & key, ::rtl::OString const & value );
    bool erase( ::rtl::OString const & key, bool flush_immediately = true );

protected:
    bool open( void);
    bool readAll( void);
    void add( ::rtl::OString const & key, ::rtl::OString const & value );
    void flush( void);
};

}

#endif

