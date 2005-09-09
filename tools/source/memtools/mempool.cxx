/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: mempool.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 14:29:00 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#include "mempool.hxx"

#include "debug.hxx"
#include <limits.h>

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
|*    FixedMemPool_Impl.
|*
*************************************************************************/
struct FixedMemPool_Impl
{
    /** Representation.
     */
    FixedMemBlock*  m_pFirst;
    USHORT          m_nTypeSize;
    USHORT          m_nInitSize;
    USHORT          m_nGrowSize;

    /** Construction.
     */
    FixedMemPool_Impl (USHORT nTypeSize, USHORT nInitSize, USHORT nGrowSize);
    ~FixedMemPool_Impl();

    /** Operation.
     */
    FixedMemBlock * newInitBlock() const
    {
        return new(m_nInitSize, m_nTypeSize) FixedMemBlock(m_nInitSize, m_nTypeSize);
    }
    FixedMemBlock * newGrowBlock() const
    {
        return new(m_nGrowSize, m_nTypeSize) FixedMemBlock(m_nGrowSize, m_nTypeSize);
    }
};

FixedMemPool_Impl::FixedMemPool_Impl (
    USHORT nTypeSize, USHORT nInitSize, USHORT nGrowSize)
    : m_pFirst (0), m_nInitSize (nInitSize), m_nGrowSize (nGrowSize)
{
    if ( nTypeSize > 4 )
        m_nTypeSize = (nTypeSize + (MEMPOOL_ALIGNMENT-1)) & ~(MEMPOOL_ALIGNMENT-1);
    else if ( nTypeSize <= 2 )
        m_nTypeSize = 2;
    else
        m_nTypeSize = 4;

    DBG_ASSERT( (ULONG)nTypeSize*nInitSize <= USHRT_MAX,
                "FixedMemPool: TypeSize*nInitSize > USHRT_MAX" );
    DBG_ASSERT( (ULONG)nTypeSize*nGrowSize <= USHRT_MAX,
                "FixedMemPool: TypeSize*GrowSize > USHRT_MAX" );
}

FixedMemPool_Impl::~FixedMemPool_Impl()
{
    FixedMemBlock* pBlock = m_pFirst;
    while ( pBlock )
    {
        FixedMemBlock* pTemp = pBlock;
        pBlock = pBlock->pNext;
        delete pTemp;
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

static void ImpDbgPoolTest( FixedMemPool_Impl * pMemPool )
{
    DbgData* pData = DbgGetData();

    if ( !pData )
        return;

    if ( !(pData->nTestFlags & (DBG_TEST_MEM_OVERWRITE | DBG_TEST_MEM_OVERWRITEFREE)) )
        return;

    FixedMemBlock* pFirst = pMemPool->m_pFirst;
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
                if ( !(n < (pBlock->nSize/pMemPool->m_nTypeSize)) )
                    DbgError( "MemPool: Memory Overwrite" );

                char* pNext = pData+(n*pMemPool->m_nTypeSize);
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

FixedMemPool::FixedMemPool (
    USHORT _nTypeSize, USHORT _nInitSize, USHORT _nGrowSize )
    : m_pImpl (new FixedMemPool_Impl (_nTypeSize, _nInitSize, _nGrowSize))
{
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
    delete m_pImpl;
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
    ImpDbgPoolTest( m_pImpl );
#endif

    if ( !m_pImpl->m_pFirst )
    {
        m_pImpl->m_pFirst = m_pImpl->newInitBlock();
        if ( !m_pImpl->m_pFirst )
            return NULL;

        return (void*)(m_pImpl->m_pFirst->aData);
    }

    FixedMemBlock* pBlock = m_pImpl->m_pFirst;
    while ( pBlock && !pBlock->nFree )
        pBlock = pBlock->pNext;

    if ( pBlock )
    {
        char* pFree = pBlock->aData+(pBlock->nFirst*m_pImpl->m_nTypeSize);
        pBlock->nFirst = *((USHORT*)pFree); // UMR, wenn letzter freier Block, ist OK
        pBlock->nFree--;
        return (void*)pFree;
    }
    else
    {
        if ( !m_pImpl->m_nGrowSize )
            return NULL;

        pBlock = m_pImpl->newGrowBlock();
        if ( !pBlock )
            return NULL;

        pBlock->pNext = m_pImpl->m_pFirst->pNext;
        m_pImpl->m_pFirst->pNext = pBlock;

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
    ImpDbgPoolTest( m_pImpl );
#endif

    FixedMemBlock* pBlock = m_pImpl->m_pFirst;
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
    pBlock->nFirst = (USHORT)(((ULONG)pFree-(ULONG)(pBlock->aData)) / m_pImpl->m_nTypeSize);

    if ( pPrev && (pBlock->nFree*m_pImpl->m_nTypeSize == pBlock->nSize) )
    {
        pPrev->pNext = pBlock->pNext;
        delete pBlock;
    }
    else
    {
        if ( pPrev )
        {
            pPrev->pNext  = pBlock->pNext;
            pBlock->pNext = m_pImpl->m_pFirst->pNext;
            m_pImpl->m_pFirst->pNext = pBlock;
        }
    }
}
