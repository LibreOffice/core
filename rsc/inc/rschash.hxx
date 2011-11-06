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


#ifndef _RSCHASH_HXX
#define _RSCHASH_HXX

#include <sal/types.h>
#include <rtl/string.hxx>
#include <hash_map>

typedef sal_uInt32 Atom;

#define InvalidAtom Atom( ~0 )

class AtomContainer
{
    Atom m_nNextID;
    std::hash_map< rtl::OString, Atom, rtl::OStringHash > m_aStringToID;
    std::hash_map< Atom, rtl::OString > m_aIDToString;

    public:
    AtomContainer();
    ~AtomContainer();

    Atom getID( const rtl::OString& rStr, bool bOnlyIfExists = false );
    const rtl::OString& getString( Atom nAtom );

};

#endif // _RSCHASH_HXX

