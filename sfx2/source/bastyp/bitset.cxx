/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include <tools/debug.hxx>

#include "bitset.hxx"

#include <string.h>
#include <limits.h>
#include <algorithm>




BitSet BitSet::operator<<( sal_uInt16 nOffset ) const
{
    
    BitSet aSet(*this);
    if ( nOffset == 0 )
        return aSet;

    
    sal_uInt16 nBlockDiff = nOffset / 32;
    sal_uIntPtr nBitValDiff = nOffset % 32;

    
    for ( sal_uInt16 nBlock = 0; nBlock < nBlockDiff; ++nBlock )
        aSet.nCount = aSet.nCount - CountBits( *(aSet.pBitmap+nBlock) );
    aSet.nCount = aSet.nCount -
        CountBits( *(aSet.pBitmap+nBlockDiff) >> (32-nBitValDiff) );

    
    sal_uInt16 nTarget, nSource;
    for ( nTarget = 0, nSource = nBlockDiff;
          (nSource+1) < aSet.nBlocks;
          ++nTarget, ++nSource )
        *(aSet.pBitmap+nTarget) =
            ( *(aSet.pBitmap+nSource) << nBitValDiff ) |
            ( *(aSet.pBitmap+nSource+1) >> (32-nBitValDiff) );

    
    *(aSet.pBitmap+nTarget) = *(aSet.pBitmap+nSource) << nBitValDiff;

    
    while ( *(aSet.pBitmap+nTarget) == 0 )
        --nTarget;

    
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





BitSet BitSet::operator>>( sal_uInt16 ) const
{
    return BitSet();
}





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





BitSet::BitSet()
{
    nCount = 0;
    nBlocks = 0;
    pBitmap = 0;
}





BitSet::BitSet( const BitSet& rOrig )
{
    CopyFrom(rOrig);
}





BitSet::~BitSet()
{
    delete [] pBitmap;
}





BitSet& BitSet::operator=( const BitSet& rOrig )
{
    if ( this != &rOrig )
    {
        delete [] pBitmap;
        CopyFrom(rOrig);
    }
    return *this;
}





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





BitSet& BitSet::operator|=( const BitSet& rSet )
{
    sal_uInt16 nMax = std::min(nBlocks, rSet.nBlocks);

    
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

    
    for ( sal_uInt16 nBlock = 0; nBlock < nMax; ++nBlock )
    {
        
        sal_uIntPtr nDiff = ~*(pBitmap+nBlock) & *(rSet.pBitmap+nBlock);
        nCount = nCount + CountBits(nDiff);

        *(pBitmap+nBlock) |= *(rSet.pBitmap+nBlock);
    }

    return *this;
}





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





sal_Bool BitSet::Contains( sal_uInt16 nBit ) const
{
    sal_uInt16 nBlock = nBit / 32;
    sal_uIntPtr nBitVal = 1L << (nBit % 32);

    if ( nBlock >= nBlocks )
        return sal_False;
    return ( nBitVal & *(pBitmap+nBlock) ) == nBitVal;
}





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
