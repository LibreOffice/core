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


#ifndef _CONTNR_HXX
#define _CONTNR_HXX

#include "tools/toolsdllapi.h"
#include <tools/solar.h>

#include <limits.h>

class CBlock;

// -------------
// - Container -
// -------------

// Maximale Blockgroesse
#define CONTAINER_MAXBLOCKSIZE      ((sal_uInt16)0x3FF0)

#define CONTAINER_APPEND            ULONG_MAX
#define CONTAINER_ENTRY_NOTFOUND    ULONG_MAX

class TOOLS_DLLPUBLIC Container
{
private:
    CBlock*     pFirstBlock;
    CBlock*     pCurBlock;
    CBlock*     pLastBlock;
    sal_uInt16      nCurIndex;
    sal_uInt16      nBlockSize;
    sal_uInt16      nInitSize;
    sal_uInt16      nReSize;
    sal_uIntPtr       nCount;

    TOOLS_DLLPRIVATE void ImpCopyContainer(Container const *);
#if defined DBG_UTIL
    TOOLS_DLLPRIVATE static char const * DbgCheckContainer(void const *);
#endif

protected:
#ifdef _IMPCONT_HXX
    void        ImpInsert( void* p, CBlock* pBlock, sal_uInt16 nIndex );
    void*       ImpRemove( CBlock* pBlock, sal_uInt16 nIndex );
    void*       ImpGetObject( sal_uIntPtr nIndex ) const;
    void**      ImpGetOnlyNodes() const;
#endif
    void**      GetObjectPtr( sal_uIntPtr nIndex );

public:
                Container( sal_uInt16 nBlockSize,
                           sal_uInt16 nInitSize,
                           sal_uInt16 nReSize );
                Container( sal_uIntPtr nSize );
                Container( const Container& rContainer );
                ~Container();

    void        Insert( void* p );
    void        Insert( void* p, sal_uIntPtr nIndex );
    void        Insert( void* pNew, void* pOld );

    void*       Remove();
    void*       Remove( sal_uIntPtr nIndex );
    void*       Remove( void* p )
                    { return Remove( GetPos( p ) ); }

    void*       Replace( void* p );
    void*       Replace( void* p, sal_uIntPtr nIndex );
    void*       Replace( void* pNew, void* pOld )
                    { return Replace( pNew, GetPos( pOld ) ); }

    void        SetSize( sal_uIntPtr nNewSize );
    sal_uIntPtr       GetSize() const { return nCount; }

    sal_uIntPtr       Count() const { return nCount; }
    void        Clear();

    void*       GetCurObject() const;
    sal_uIntPtr       GetCurPos() const;
    void*       GetObject( sal_uIntPtr nIndex ) const;
    sal_uIntPtr       GetPos( const void* p ) const;
    sal_uIntPtr       GetPos( const void* p, sal_uIntPtr nStartIndex,
                        sal_Bool bForward = sal_True ) const;

    void*       Seek( sal_uIntPtr nIndex );
    void*       Seek( void* p ) { return Seek( GetPos( p ) ); }

    void*       First();
    void*       Last();
    void*       Next();
    void*       Prev();

    Container&  operator =( const Container& rContainer );

    sal_Bool        operator ==( const Container& rContainer ) const;
    sal_Bool        operator !=( const Container& rContainer ) const
                    { return !(Container::operator==( rContainer )); }
};

#endif // _CONTNR_HXX
