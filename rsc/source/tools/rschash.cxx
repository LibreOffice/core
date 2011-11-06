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



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_rsc.hxx"
#include <rschash.hxx>

using namespace rtl;

AtomContainer::AtomContainer()
{
    m_aStringToID[ OString() ] = 0;
    m_aIDToString[ 0 ] = OString();
    m_nNextID = 1;
}

AtomContainer::~AtomContainer()
{
}

Atom AtomContainer::getID( const OString& rStr, bool bOnlyIfExists )
{
    OString aKey = rStr.toAsciiLowerCase();
    std::hash_map< OString, Atom, OStringHash >::const_iterator it =
        m_aStringToID.find( aKey );
    if( it != m_aStringToID.end() )
        return it->second;

    if( bOnlyIfExists )
        return InvalidAtom;

    Atom aRet = m_nNextID;
    m_aStringToID[ aKey ] = m_nNextID;
    m_aIDToString[ m_nNextID ] = rStr;
    m_nNextID++;
    return aRet;
}

const OString& AtomContainer::getString( Atom nAtom )
{
    std::hash_map< Atom, OString >::const_iterator it =
        m_aIDToString.find( nAtom );
    return (it != m_aIDToString.end()) ? it->second : m_aIDToString[0];
}
