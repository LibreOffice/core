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

#include <limits.h>

#include <rtl/alloc.h>
#include <vcl/bitmapaccess.hxx>

#include <octree.hxx>
#include <impoctree.hxx>

static const sal_uInt8 pImplMask[8] = { 0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01 };

ImpNodeCache::ImpNodeCache( const sal_uLong nInitSize ) :
            pActNode( nullptr )
{
    const sal_uLong nSize = nInitSize + 4;

    for( sal_uLong i = 0; i < nSize; i++ )
    {
        OctreeNode* pNewNode = new NODE;

        pNewNode->pNextInCache = pActNode;
        pActNode = pNewNode;
    }
}

ImpNodeCache::~ImpNodeCache()
{
    while( pActNode )
    {
        OctreeNode* pNode = pActNode;

        pActNode = pNode->pNextInCache;
        delete pNode;
    }
}

Octree::Octree(const BitmapReadAccess& rReadAcc, sal_uLong nColors)
    : nMax(nColors)
    , nLeafCount(0)
    , nLevel(0)
    , pTree(nullptr)
    , pColor(nullptr)
    , pAcc(&rReadAcc)
    , nPalIndex(0)
{
    pNodeCache.reset( new ImpNodeCache( nColors ) );
    memset( pReduce, 0, ( OCTREE_BITS + 1 ) * sizeof( NODE* ) );

    if( !!*pAcc )
    {
        const long      nWidth = pAcc->Width();
        const long      nHeight = pAcc->Height();

        if( pAcc->HasPalette() )
        {
            for( long nY = 0; nY < nHeight; nY++ )
            {
                Scanline pScanline = pAcc->GetScanline( nY );
                for( long nX = 0; nX < nWidth; nX++ )
                {
                    pColor = &pAcc->GetPaletteColor( pAcc->GetIndexFromData( pScanline, nX ) );
                    nLevel = 0;
                    ImplAdd( &pTree );

                    while( nLeafCount > nMax )
                        ImplReduce();
                }
            }
        }
        else
        {
            BitmapColor aColor;

            pColor = &aColor;

            for( long nY = 0; nY < nHeight; nY++ )
            {
                Scanline pScanline = pAcc->GetScanline( nY );
                for( long nX = 0; nX < nWidth; nX++ )
                {
                    aColor = pAcc->GetPixelFromData( pScanline, nX );
                    nLevel = 0;
                    ImplAdd( &pTree );

                    while( nLeafCount > nMax )
                        ImplReduce();
                }
            }
        }
    }
}

Octree::~Octree()
{
    ImplDeleteOctree( &pTree );
    pNodeCache.reset();
}

void Octree::ImplDeleteOctree( NODE** ppNode )
{
    for (OctreeNode* i : (*ppNode)->pChild)
    {
        if ( i )
            ImplDeleteOctree( &i );
    }

    pNodeCache->ImplReleaseNode( *ppNode );
    *ppNode = nullptr;
}

void Octree::ImplAdd( NODE** ppNode )
{
    // possibly generate new nodes
    if( !*ppNode )
    {
        *ppNode = pNodeCache->ImplGetFreeNode();
        (*ppNode)->bLeaf = ( OCTREE_BITS == nLevel );

        if( (*ppNode)->bLeaf )
            nLeafCount++;
        else
        {
            (*ppNode)->pNext = pReduce[ nLevel ];
            pReduce[ nLevel ] = *ppNode;
        }
    }

    if( (*ppNode)->bLeaf )
    {
        (*ppNode)->nCount++;
        (*ppNode)->nRed += pColor->GetRed();
        (*ppNode)->nGreen += pColor->GetGreen();
        (*ppNode)->nBlue += pColor->GetBlue();
    }
    else
    {
        const sal_uLong nShift = 7 - nLevel;
        const sal_uInt8  cMask = pImplMask[ nLevel ];
        const sal_uLong nIndex = ( ( ( pColor->GetRed() & cMask ) >> nShift ) << 2 ) |
                             ( ( ( pColor->GetGreen() & cMask ) >> nShift ) << 1 ) |
                             ( ( pColor->GetBlue() & cMask ) >> nShift );

        nLevel++;
        ImplAdd( &(*ppNode)->pChild[ nIndex ] );
    }
}

void Octree::ImplReduce()
{
    sal_uLong   i;
    NODE*       pNode;
    sal_uLong   nRedSum = 0;
    sal_uLong   nGreenSum = 0;
    sal_uLong   nBlueSum = 0;
    sal_uLong   nChildren = 0;

    for ( i = OCTREE_BITS - 1; i && !pReduce[i]; i-- ) {}

    pNode = pReduce[ i ];
    pReduce[ i ] = pNode->pNext;

    for ( i = 0; i < 8; i++ )
    {
        if ( pNode->pChild[ i ] )
        {
            NODE* pChild = pNode->pChild[ i ];

            nRedSum += pChild->nRed;
            nGreenSum += pChild->nGreen;
            nBlueSum += pChild->nBlue;
            pNode->nCount += pChild->nCount;

            pNodeCache->ImplReleaseNode( pNode->pChild[ i ] );
            pNode->pChild[ i ] = nullptr;
            nChildren++;
        }
    }

    pNode->bLeaf = true;
    pNode->nRed = nRedSum;
    pNode->nGreen = nGreenSum;
    pNode->nBlue = nBlueSum;
    nLeafCount -= --nChildren;
}

void Octree::CreatePalette( NODE* pNode )
{
    if( pNode->bLeaf )
    {
        pNode->nPalIndex = nPalIndex;
        aPal[ nPalIndex++ ] = BitmapColor( static_cast<sal_uInt8>( static_cast<double>(pNode->nRed) / pNode->nCount ),
                                           static_cast<sal_uInt8>( static_cast<double>(pNode->nGreen) / pNode->nCount ),
                                           static_cast<sal_uInt8>( static_cast<double>(pNode->nBlue) / pNode->nCount ) );
    }
    else for(OctreeNode* i : pNode->pChild)
        if( i )
            CreatePalette( i );

}

void Octree::GetPalIndex( NODE* pNode )
{
    if ( pNode->bLeaf )
        nPalIndex = pNode->nPalIndex;
    else
    {
        const sal_uLong nShift = 7 - nLevel;
        const sal_uInt8  cMask = pImplMask[ nLevel++ ];
        const sal_uLong nIndex = ( ( ( pColor->GetRed() & cMask ) >> nShift ) << 2 ) |
                             ( ( ( pColor->GetGreen() & cMask ) >> nShift ) << 1 ) |
                             ( ( pColor->GetBlue() & cMask ) >> nShift );

        GetPalIndex( pNode->pChild[ nIndex ] );
    }
}

InverseColorMap::InverseColorMap( const BitmapPalette& rPal ) :
            nBits( 8 - OCTREE_BITS )
{
    const int     nColorMax = 1 << OCTREE_BITS;
    const unsigned long xsqr = 1 << ( nBits << 1 );
    const unsigned long xsqr2 = xsqr << 1;
    const int     nColors = rPal.GetEntryCount();
    const long      x = 1 << nBits;
    const long      x2 = x >> 1;
    sal_uLong           r, g, b;
    long            rxx, gxx, bxx;

    ImplCreateBuffers( nColorMax );

    for( int nIndex = 0; nIndex < nColors; nIndex++ )
    {
        const BitmapColor&  rColor = rPal[ static_cast<sal_uInt16>(nIndex) ];
        const long          cRed = rColor.GetRed();
        const long          cGreen = rColor.GetGreen();
        const long          cBlue = rColor.GetBlue();

        long rdist = cRed - x2;
        long gdist = cGreen - x2;
        long bdist = cBlue - x2;
        rdist = rdist*rdist + gdist*gdist + bdist*bdist;

        const long crinc = ( xsqr - ( cRed << nBits ) ) << 1;
        const long cginc = ( xsqr - ( cGreen << nBits ) ) << 1;
        const long cbinc = ( xsqr - ( cBlue << nBits ) ) << 1;

        sal_uLong* cdp = reinterpret_cast<sal_uLong*>(pBuffer);
        sal_uInt8* crgbp = pMap;

        for( r = 0, rxx = crinc; r < nColorMax; rdist += rxx, r++, rxx += xsqr2 )
        {
            for( g = 0, gdist = rdist, gxx = cginc; g < nColorMax;  gdist += gxx, g++, gxx += xsqr2 )
            {
                for( b = 0, bdist = gdist, bxx = cbinc; b < nColorMax; bdist += bxx, b++, cdp++, crgbp++, bxx += xsqr2 )
                    if ( !nIndex || static_cast<long>(*cdp) > bdist )
                    {
                        *cdp = bdist;
                        *crgbp = static_cast<sal_uInt8>(nIndex);
                    }
            }
        }
    }
}

InverseColorMap::~InverseColorMap()
{
    rtl_freeMemory( pBuffer );
    rtl_freeMemory( pMap );
}

void InverseColorMap::ImplCreateBuffers( const sal_uLong nMax )
{
    const sal_uLong nCount = nMax * nMax * nMax;
    const sal_uLong nSize = nCount * sizeof( sal_uLong );

    pMap = static_cast<sal_uInt8*>(rtl_allocateMemory( nCount ));
    memset( pMap, 0x00, nCount );

    pBuffer = static_cast<sal_uInt8*>(rtl_allocateMemory( nSize ));
    memset( pBuffer, 0xff, nSize );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
