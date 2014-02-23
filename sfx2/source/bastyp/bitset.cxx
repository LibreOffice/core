/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <tools/debug.hxx>

#include "bitset.hxx"

#include <string.h>
#include <limits.h>
#include <algorithm>


// add nOffset to each bit-value in the set

BitSet BitSet::operator<<( sal_uInt16 nOffset ) const
{
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



// substracts nOffset from each bit-value in the set

BitSet BitSet::operator>>( sal_uInt16 ) const
{
    return BitSet();
}



// internal code for operator= and copy-ctor

void BitSet::CopyFrom( const BitSet& rSet )
{
    nCount = rSet.nCount;
    nBlocks = rSet.nBlocks;
    if ( rSet.nBlocks )
    {
        pBitmap = new sal_uIntPtr[nBlocks];
        memcpy( pBitmap, rSet.pBitmap, 4 * nBlocks );
    }
    else
        pBitmap = 0;
}



// creates an empty bitset

BitSet::BitSet()
{
    nCount = 0;
    nBlocks = 0;
    pBitmap = 0;
}



// creates a copy of bitset rOrig

BitSet::BitSet( const BitSet& rOrig )
{
    CopyFrom(rOrig);
}



// frees the storage

BitSet::~BitSet()
{
    delete [] pBitmap;
}



// assignment from another bitset

BitSet& BitSet::operator=( const BitSet& rOrig )
{
    if ( this != &rOrig )
    {
        delete [] pBitmap;
        CopyFrom(rOrig);
    }
    return *this;
}



// assignment from a single bit

BitSet& BitSet::operator=( sal_uInt16 nBit )
{
    delete [] pBitmap;

    nBlocks = nBit / 32;
    sal_uIntPtr nBitVal = 1L << (nBit % 32);
    nCount = 1;

    pBitmap = new sal_uIntPtr[nBlocks + 1];
    memset( pBitmap, 0, 4 * (nBlocks + 1) );

    *(pBitmap+nBlocks) = nBitVal;

    return *this;
}



// creates the asymetric difference with another bitset

BitSet& BitSet::operator-=(sal_uInt16 nBit)
{
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



// unites with the bits of rSet

BitSet& BitSet::operator|=( const BitSet& rSet )
{
    sal_uInt16 nMax = std::min(nBlocks, rSet.nBlocks);

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



// unites with a single bit

BitSet& BitSet::operator|=( sal_uInt16 nBit )
{
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



// determines if the bit is set (may be the only one)

sal_Bool BitSet::Contains( sal_uInt16 nBit ) const
{
    sal_uInt16 nBlock = nBit / 32;
    sal_uIntPtr nBitVal = 1L << (nBit % 32);

    if ( nBlock >= nBlocks )
        return sal_False;
    return ( nBitVal & *(pBitmap+nBlock) ) == nBitVal;
}



// determines if the bitsets are equal

sal_Bool BitSet::operator==( const BitSet& rSet ) const
{
    if ( nBlocks != rSet.nBlocks )
        return sal_False;

    sal_uInt16 nBlock = nBlocks;
    while ( nBlock-- > 0 )
        if ( *(pBitmap+nBlock) != *(rSet.pBitmap+nBlock) )
            return sal_False;

    return sal_True;
}



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



sal_uInt16 IndexBitSet::GetFreeIndex()
{
  for(sal_uInt16 i=0;i<USHRT_MAX;i++)
    if(!Contains(i))
      {
        *this|=i;
        return i;
      }
  DBG_ASSERT(false, "IndexBitSet enthaelt mehr als USHRT_MAX Eintraege");
  return 0;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
