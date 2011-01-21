/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sfx2.hxx"
#include <tools/debug.hxx>
#ifndef GCC
#endif

#include "bitset.hxx"

#include <string.h>     // memset(), memcpy()
#include <limits.h>     // USHRT_MAX

//====================================================================
// add nOffset to each bit-value in the set

BitSet BitSet::operator<<( sal_uInt16 nOffset ) const
{
    DBG_MEMTEST();
    // create a work-copy, return it if nothing to shift
    BitSet aSet(*this);
    if ( nOffset == 0 )
        return aSet;

    // compute the shiftment in long-words and bits
    sal_uInt16 nBlockDiff = nOffset / 32;
    sal_uIntPtr nBitValDiff = nOffset % 32;

    // compute the new number of bits
    for ( sal_uInt16 nBlock = 0; nBlock < nBlockDiff; ++nBlock )
        aSet.nCount = aSet.nCount - CountBits( *(aSet.pBitmap+nBlock) );
    aSet.nCount = aSet.nCount -
        CountBits( *(aSet.pBitmap+nBlockDiff) >> (32-nBitValDiff) );

    // shift complete long-words
    sal_uInt16 nTarget, nSource;
    for ( nTarget = 0, nSource = nBlockDiff;
          (nSource+1) < aSet.nBlocks;
          ++nTarget, ++nSource )
        *(aSet.pBitmap+nTarget) =
            ( *(aSet.pBitmap+nSource) << nBitValDiff ) |
            ( *(aSet.pBitmap+nSource+1) >> (32-nBitValDiff) );

    // shift the remainder (if in total minor 32 bits, only this)
    *(aSet.pBitmap+nTarget) = *(aSet.pBitmap+nSource) << nBitValDiff;

    // determine the last used block
    while ( *(aSet.pBitmap+nTarget) == 0 )
        --nTarget;

    // shorten the block-array
    if ( nTarget < aSet.nBlocks )
    {
        sal_uIntPtr* pNewMap = new sal_uIntPtr[nTarget];
        memcpy( pNewMap, aSet.pBitmap, 4 * nTarget );
        delete [] aSet.pBitmap;
        aSet.pBitmap = pNewMap;
        aSet.nBlocks = nTarget;
    }

    return aSet;
}

//--------------------------------------------------------------------

// substracts nOffset from each bit-value in the set

BitSet BitSet::operator>>( sal_uInt16 ) const
{
    DBG_MEMTEST();
    return BitSet();
}

//--------------------------------------------------------------------

// internal code for operator= and copy-ctor

void BitSet::CopyFrom( const BitSet& rSet )
{
    DBG_MEMTEST();
    nCount = rSet.nCount;
    nBlocks = rSet.nBlocks;
    if ( rSet.nBlocks )
    {
        DBG_MEMTEST();
        pBitmap = new sal_uIntPtr[nBlocks];
        memcpy( pBitmap, rSet.pBitmap, 4 * nBlocks );
    }
    else
        pBitmap = 0;
}

//--------------------------------------------------------------------

// creates an empty bitset

BitSet::BitSet()
{
    DBG_MEMTEST();
    nCount = 0;
    nBlocks = 0;
    pBitmap = 0;
}

//--------------------------------------------------------------------

// creates a copy of bitset rOrig

BitSet::BitSet( const BitSet& rOrig )
{
    DBG_MEMTEST();
    CopyFrom(rOrig);
}

//--------------------------------------------------------------------

// creates a bitset from an array

BitSet::BitSet( sal_uInt16* pArray, sal_uInt16 nSize ):
    nCount(nSize)
{
    DBG_MEMTEST();
    // find the highest bit to set
    sal_uInt16 nMax = 0;
    for ( sal_uInt16 n = 0; n < nCount; ++n )
        if ( pArray[n] > nMax )
            nMax = pArray[n];

    // if there are bits at all
    if ( nMax > 0 )
    {
        // allocate memory for all blocks needed
        nBlocks = nMax / 32 + 1;
        pBitmap = new sal_uIntPtr[nBlocks];
        memset( pBitmap, 0, 4 * nBlocks );

        // set all the bits
        for ( sal_uInt16 n = 0; n < nCount; ++n )
        {
            // compute the block no. and bitvalue
            sal_uInt16 nBlock = n / 32;
            sal_uIntPtr nBitVal = 1L << (n % 32);

            // set a single bit
            if ( ( *(pBitmap+nBlock) & nBitVal ) == 0 )
            {
                *(pBitmap+nBlock) |= nBitVal;
                ++nCount;
            }
        }
    }
    else
    {
        // initalize emtpy set
        nBlocks = 0;
        pBitmap = 0;
    }
}

//--------------------------------------------------------------------

// frees the storage

BitSet::~BitSet()
{
    DBG_MEMTEST();
    delete [] pBitmap;
}

//--------------------------------------------------------------------

// creates a bitmap with all bits in rRange set

BitSet::BitSet( const Range& )
{
    DBG_MEMTEST();
}

//--------------------------------------------------------------------

// assignment from another bitset

BitSet& BitSet::operator=( const BitSet& rOrig )
{
    DBG_MEMTEST();
    if ( this != &rOrig )
    {
        delete [] pBitmap;
        CopyFrom(rOrig);
    }
    return *this;
}

//--------------------------------------------------------------------

// assignment from a single bit

BitSet& BitSet::operator=( sal_uInt16 nBit )
{
    DBG_MEMTEST();
    delete [] pBitmap;

    nBlocks = nBit / 32;
    sal_uIntPtr nBitVal = 1L << (nBit % 32);
    nCount = 1;

    pBitmap = new sal_uIntPtr[nBlocks];
    memset( pBitmap + nBlocks, 0, 4 * nBlocks );

    *(pBitmap+nBlocks) = nBitVal;

    return *this;
}

//--------------------------------------------------------------------

// creates the asymetric difference with another bitset

BitSet& BitSet::operator-=(sal_uInt16 nBit)
{
    DBG_MEMTEST();
    sal_uInt16 nBlock = nBit / 32;
    sal_uIntPtr nBitVal = 1L << (nBit % 32);

    if ( nBlock >= nBlocks )
      return *this;

    if ( (*(pBitmap+nBlock) & nBitVal) )
    {
        *(pBitmap+nBlock) &= ~nBitVal;
        --nCount;
    }

    return *this;
}

//--------------------------------------------------------------------

// unites with the bits of rSet

BitSet& BitSet::operator|=( const BitSet& rSet )
{
    DBG_MEMTEST();
    sal_uInt16 nMax = Min(nBlocks, rSet.nBlocks);

    // expand the bitmap
    if ( nBlocks < rSet.nBlocks )
    {
        sal_uIntPtr *pNewMap = new sal_uIntPtr[rSet.nBlocks];
        memset( pNewMap + nBlocks, 0, 4 * (rSet.nBlocks - nBlocks) );

        if ( pBitmap )
        {
            memcpy( pNewMap, pBitmap, 4 * nBlocks );
            delete [] pBitmap;
        }
        pBitmap = pNewMap;
        nBlocks = rSet.nBlocks;
    }

    // add the bits blocks by block
    for ( sal_uInt16 nBlock = 0; nBlock < nMax; ++nBlock )
    {
        // compute numberof additional bits
        sal_uIntPtr nDiff = ~*(pBitmap+nBlock) & *(rSet.pBitmap+nBlock);
        nCount = nCount + CountBits(nDiff);

        *(pBitmap+nBlock) |= *(rSet.pBitmap+nBlock);
    }

    return *this;
}

//--------------------------------------------------------------------

// unites with a single bit

BitSet& BitSet::operator|=( sal_uInt16 nBit )
{
    DBG_MEMTEST();
    sal_uInt16 nBlock = nBit / 32;
    sal_uIntPtr nBitVal = 1L << (nBit % 32);

    if ( nBlock >= nBlocks )
    {
        sal_uIntPtr *pNewMap = new sal_uIntPtr[nBlock+1];
        memset( pNewMap + nBlocks, 0, 4 * (nBlock - nBlocks + 1) );

        if ( pBitmap )
        {
            memcpy( pNewMap, pBitmap, 4 * nBlocks );
            delete [] pBitmap;
        }
        pBitmap = pNewMap;
        nBlocks = nBlock+1;
    }

    if ( (*(pBitmap+nBlock) & nBitVal) == 0 )
    {
        *(pBitmap+nBlock) |= nBitVal;
        ++nCount;
    }

    return *this;
}

//--------------------------------------------------------------------

// determines if the bit is set (may be the only one)

sal_Bool BitSet::Contains( sal_uInt16 nBit ) const
{
    DBG_MEMTEST();
    sal_uInt16 nBlock = nBit / 32;
    sal_uIntPtr nBitVal = 1L << (nBit % 32);

    if ( nBlock >= nBlocks )
        return sal_False;
    return ( nBitVal & *(pBitmap+nBlock) ) == nBitVal;
}

//--------------------------------------------------------------------

// determines if the bitsets are equal

sal_Bool BitSet::operator==( const BitSet& rSet ) const
{
    DBG_MEMTEST();
    if ( nBlocks != rSet.nBlocks )
        return sal_False;

    sal_uInt16 nBlock = nBlocks;
    while ( nBlock-- > 0 )
        if ( *(pBitmap+nBlock) != *(rSet.pBitmap+nBlock) )
            return sal_False;

    return sal_True;
}

//--------------------------------------------------------------------

// counts the number of 1-bits in the parameter

sal_uInt16 BitSet::CountBits( sal_uIntPtr nBits )
{
    sal_uInt16 nCount = 0;
    int nBit = 32;
    while ( nBit-- && nBits )
    {   if ( ( (long)nBits ) < 0 )
            ++nCount;
        nBits = nBits << 1;
    }
    return nCount;
}

//--------------------------------------------------------------------

sal_uInt16 IndexBitSet::GetFreeIndex()
{
  for(sal_uInt16 i=0;i<USHRT_MAX;i++)
    if(!Contains(i))
      {
        *this|=i;
        return i;
      }
  DBG_ASSERT(sal_False, "IndexBitSet enthaelt mehr als USHRT_MAX Eintraege");
  return 0;
}


