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


#ifndef _SFXIDPOOL_HXX
#define _SFXIDPOOL_HXX

#include <limits.h>     // USHRT_MAX
#include "bitset.hxx"

// class IdPool ----------------------------------------------------------

class IdPool: private BitSet
{
private:
    sal_uInt16 nNextFree;
    sal_uInt16 nRange;
    sal_uInt16 nOffset;
public:
    sal_Bool Lock( const BitSet& rLockSet );
    sal_Bool IsLocked( sal_uInt16 nId ) const;
    IdPool( sal_uInt16 nMin = 1, sal_uInt16 nMax = USHRT_MAX );
    sal_uInt16 Get();
    sal_Bool Put( sal_uInt16 nId );
    sal_Bool Lock( const Range& rRange );
    sal_Bool Lock( sal_uInt16 nId );

};

//------------------------------------------------------------------------

// returns sal_True if the id is locked

inline sal_Bool IdPool::IsLocked( sal_uInt16 nId ) const
{
    return ( this->Contains(nId-nOffset) );
}


#endif

