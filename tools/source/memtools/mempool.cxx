/*************************************************************************
 *
 *  $RCSfile: mempool.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2004-06-17 13:12:06 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#define private public

#include <limits.h>

#include <debug.hxx>
#include <mempool.hxx>

// -----------------------------------------------------------------------

#if ( SAL_TYPES_ALIGNMENT8 > 4 )
#define MEMPOOL_ALIGNMENT    SAL_TYPES_ALIGNMENT8
#else
#define MEMPOOL_ALIGNMENT    4
#endif

// -----------------
// - FixedMemBlock -
// -----------------

struct FixedMemBlock
{
    /** Representation.
     */
    USHORT          nSize;
    USHORT          nFree;
    USHORT          nFirst;
    USHORT          nDummyAlign1;
    FixedMemBlock*  pNext;
#if (SAL_TYPES_SIZEOFPOINTER == 4) && (SAL_TYPES_ALIGNMENT8 == 8)
    void*           pDummyAlign2;
#endif
    char            aData[1];

    /** Allocation.
     */
    static void* operator new (size_t, USHORT nTypes, USHORT nTypeSize)
    {
        return ::operator new (sizeof(FixedMemBlock) + nTypes * nTypeSize);
    }
    static void  operator delete (void *p, size_t)
    {
        ::operator delete (p);
    }

    /** Construction.
     */
    FixedMemBlock (USHORT nTypes = 1, USHORT nTypeSize = 0);
    ~FixedMemBlock() {}
};

FixedMemBlock::FixedMemBlock (USHORT nTypes, USHORT nTypeSize)
    : nSize  (nTypes * nTypeSize),
      nFree  (nTypes - 1),
      nFirst (1),
      pNext  (0)
{
    char * pData = aData;
    for (USHORT i = 1; i < nTypes; i++)
    {
        *reinterpret_cast<USHORT*>(pData) = i;
        pData += nTypeSize;
    }
}

/*************************************************************************
|*
|*    ImpDbgPoolTest()
|*
|*    Beschreibung      MEMPOOL.SDW
|*    Ersterstellung    TH 30.11.94
|*    Letzte Aenderung  TH 30.11.94
|*
*************************************************************************/

#ifdef DBG_UTIL

static void ImpDbgPoolTest( FixedMemPool* pMemPool )
{
    DbgData* pData = DbgGetData();

    if ( !pData )
        return;

    if ( !(pData->nTestFlags & (DBG_TEST_MEM_OVERWRITE | DBG_TEST_MEM_OVERWRITEFREE)) )
        return;

    FixedMemBlock* pFirst = pMemPool->pFirst;
    FixedMemBlock* pBlock = pFirst;
    while ( pBlock )
    {
        if ( pBlock->nFree )
        {
            USHORT i = pBlock->nFree;
            USHORT n = pBlock->nFirst;
            char*  pData = pBlock->aData;
            while ( i )
            {
                if ( !(n < (pBlock->nSize/pMemPool->nTypeSize)) )
                    DbgError( "MemPool: Memory Overwrite" );

                char* pNext = pData+(n*pMemPool->nTypeSize);
                n = *((USHORT*)pNext);
                i--;
            }
        }

        pBlock = pBlock->pNext;
    }
}

#endif

/*************************************************************************
|*
|*    FixedMemPool::FixedMemPool()
|*
|*    Beschreibung      MEMPOOL.SDW
|*    Ersterstellung    TH 02.09.94
|*    Letzte Aenderung  TH 02.09.94
|*
*************************************************************************/

FixedMemPool::FixedMemPool( USHORT _nTypeSize,
                            USHORT _nInitSize, USHORT _nGrowSize )
{
    pFirst      = NULL;
    nInitSize   = _nInitSize;
    nGrowSize   = _nGrowSize;

    if ( _nTypeSize > 4 )
        nTypeSize = (_nTypeSize + (MEMPOOL_ALIGNMENT-1)) & ~(MEMPOOL_ALIGNMENT-1);
    else if ( _nTypeSize <= 2 )
        nTypeSize = 2;
    else
        nTypeSize = 4;

    DBG_ASSERT( (ULONG)nTypeSize*nInitSize <= USHRT_MAX,
                "FixedMemPool: TypeSize*nInitSize > USHRT_MAX" );
    DBG_ASSERT( (ULONG)nTypeSize*nGrowSize <= USHRT_MAX,
                "FixedMemPool: TypeSize*GrowSize > USHRT_MAX" );
}

/*************************************************************************
|*
|*    FixedMemPool::~FixedMemPool()
|*
|*    Beschreibung      MEMPOOL.SDW
|*    Ersterstellung    TH 02.09.94
|*    Letzte Aenderung  TH 02.09.94
|*
*************************************************************************/

FixedMemPool::~FixedMemPool()
{
    FixedMemBlock* pBlock = pFirst;
    while ( pBlock )
    {
        FixedMemBlock* pTemp = pBlock;
        pBlock = pBlock->pNext;
        delete pTemp;
    }
}

/*************************************************************************
|*
|*    FixedMemPool::Alloc()
|*
|*    Beschreibung      MEMPOOL.SDW
|*    Ersterstellung    TH 02.09.94
|*    Letzte Aenderung  TH 02.09.94
|*
*************************************************************************/

void* FixedMemPool::Alloc()
{
#ifdef DBG_UTIL
    ImpDbgPoolTest( this );
#endif

    if ( !pFirst )
    {
        pFirst = new(nInitSize, nTypeSize) FixedMemBlock(nInitSize, nTypeSize);
        if ( !pFirst )
            return NULL;

        return (void*)(pFirst->aData);
    }

    FixedMemBlock* pBlock = pFirst;
    while ( pBlock && !pBlock->nFree )
        pBlock = pBlock->pNext;

    if ( pBlock )
    {
        char* pFree = pBlock->aData+(pBlock->nFirst*nTypeSize);
        pBlock->nFirst = *((USHORT*)pFree); // UMR, wenn letzter freier Block, ist OK
        pBlock->nFree--;
        return (void*)pFree;
    }
    else
    {
        if ( !nGrowSize )
            return NULL;

        pBlock = new(nGrowSize, nTypeSize) FixedMemBlock(nGrowSize, nTypeSize);
        if ( !pBlock )
            return NULL;

        pBlock->pNext = pFirst->pNext;
        pFirst->pNext = pBlock;

        return (void*)(pBlock->aData);
    }
}

/*************************************************************************
|*
|*    FixedMemPool::Free()
|*
|*    Beschreibung      MEMPOOL.SDW
|*    Ersterstellung    TH 02.09.94
|*    Letzte Aenderung  TH 02.09.94
|*
*************************************************************************/

void FixedMemPool::Free( void* pFree )
{
    if ( !pFree )
        return;

#ifdef DBG_UTIL
    ImpDbgPoolTest( this );
#endif

    FixedMemBlock* pBlock = pFirst;
    FixedMemBlock* pPrev  = NULL;
    while ( ((ULONG)pBlock->aData > (ULONG)pFree) ||
            ((ULONG)pFree >= ((ULONG)pBlock->aData+pBlock->nSize)) )
    {
        pPrev = pBlock;
        pBlock = pBlock->pNext;

#ifdef DBG_UTIL
        DBG_ASSERT( pBlock, "FixedMemPool - Delete: Wrong Pointer" );
#endif
    }

    pBlock->nFree++;
    *((USHORT*)pFree) = pBlock->nFirst;
    pBlock->nFirst = (USHORT)(((ULONG)pFree-(ULONG)(pBlock->aData)) / nTypeSize);

    if ( pPrev && (pBlock->nFree*nTypeSize == pBlock->nSize) )
    {
        pPrev->pNext = pBlock->pNext;
        delete pBlock;
    }
    else
    {
        if ( pPrev )
        {
            pPrev->pNext  = pBlock->pNext;
            pBlock->pNext = pFirst->pNext;
            pFirst->pNext = pBlock;
        }
    }
}
