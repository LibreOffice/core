/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
#include "precompiled_vcl.hxx"
#include <limits.h>
#include <vcl/bmpacc.hxx>
#include <vcl/impoct.hxx>
#include <vcl/octree.hxx>

// ---------
// - pMask -
// ---------

static sal_uInt8 pImplMask[8] = { 0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01 };

// -------------
// - NodeCache -
// -------------

ImpNodeCache::ImpNodeCache( const sal_uLong nInitSize ) :
            pActNode( NULL )
{
    const sal_uLong nSize = nInitSize + 4;

    for( sal_uLong i = 0; i < nSize; i++ )
    {
        OctreeNode* pNewNode = new NODE;

        pNewNode->pNextInCache = pActNode;
        pActNode = pNewNode;
    }
}

// ------------------------------------------------------------------------

ImpNodeCache::~ImpNodeCache()
{
    while( pActNode )
    {
        OctreeNode* pNode = pActNode;

        pActNode = pNode->pNextInCache;
        delete pNode;
    }
}

// ----------
// - Octree -
// ----------

Octree::Octree( sal_uLong nColors ) :
            nMax        ( nColors ),
            nLeafCount  ( 0L ),
            pTree       ( NULL ),
            pAcc        ( NULL )
{
    pNodeCache = new ImpNodeCache( nColors );
    memset( pReduce, 0, ( OCTREE_BITS + 1 ) * sizeof( PNODE ) );
}

// ------------------------------------------------------------------------

Octree::Octree( const BitmapReadAccess& rReadAcc, sal_uLong nColors ) :
            nMax        ( nColors ),
            nLeafCount  ( 0L ),
            pTree       ( NULL ),
            pAcc        ( &rReadAcc )
{
    pNodeCache = new ImpNodeCache( nColors );
    memset( pReduce, 0, ( OCTREE_BITS + 1 ) * sizeof( PNODE ) );
    ImplCreateOctree();
}

// ------------------------------------------------------------------------

Octree::~Octree()
{
    ImplDeleteOctree( &pTree );
    delete pNodeCache;
}

// ------------------------------------------------------------------------

void Octree::AddColor( const BitmapColor& rColor )
{
    pColor = &(BitmapColor&) rColor;
    nLevel = 0L;
    ImplAdd( &pTree );

    while( nLeafCount > nMax )
        ImplReduce();
}

// ------------------------------------------------------------------------

void Octree::ImplCreateOctree()
{
    if( !!*pAcc )
    {
        const long      nWidth = pAcc->Width();
        const long      nHeight = pAcc->Height();

        if( pAcc->HasPalette() )
        {
            for( long nY = 0; nY < nHeight; nY++ )
            {
                for( long nX = 0; nX < nWidth; nX++ )
                {
                    pColor = &(BitmapColor&) pAcc->GetPaletteColor( pAcc->GetPixel( nY, nX ) );
                    nLevel = 0L;
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
                for( long nX = 0; nX < nWidth; nX++ )
                {
                    aColor = pAcc->GetPixel( nY, nX );
                    nLevel = 0L;
                    ImplAdd( &pTree );

                    while( nLeafCount > nMax )
                        ImplReduce();
                }
            }
        }
    }
}

// ------------------------------------------------------------------------

void Octree::ImplDeleteOctree( PPNODE ppNode )
{
    for ( sal_uLong i = 0UL; i < 8UL; i++ )
    {
        if ( (*ppNode)->pChild[ i ] )
            ImplDeleteOctree( &(*ppNode)->pChild[ i ] );
    }

    pNodeCache->ImplReleaseNode( *ppNode );
    *ppNode = NULL;
}

// ------------------------------------------------------------------------

void Octree::ImplAdd( PPNODE ppNode )
{
    // ggf. neuen Knoten erzeugen
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

// ------------------------------------------------------------------------

void Octree::ImplReduce()
{
    sal_uLong   i;
    PNODE   pNode;
    sal_uLong   nRedSum = 0L;
    sal_uLong   nGreenSum = 0L;
    sal_uLong   nBlueSum = 0L;
    sal_uLong   nChilds = 0L;

    for ( i = OCTREE_BITS - 1; i && !pReduce[i]; i-- ) {}

    pNode = pReduce[ i ];
    pReduce[ i ] = pNode->pNext;

    for ( i = 0; i < 8; i++ )
    {
        if ( pNode->pChild[ i ] )
        {
            PNODE pChild = pNode->pChild[ i ];

            nRedSum += pChild->nRed;
            nGreenSum += pChild->nGreen;
            nBlueSum += pChild->nBlue;
            pNode->nCount += pChild->nCount;

            pNodeCache->ImplReleaseNode( pNode->pChild[ i ] );
            pNode->pChild[ i ] = NULL;
            nChilds++;
        }
    }

    pNode->bLeaf = sal_True;
    pNode->nRed = nRedSum;
    pNode->nGreen = nGreenSum;
    pNode->nBlue = nBlueSum;
    nLeafCount -= --nChilds;
}

// ------------------------------------------------------------------------

void Octree::CreatePalette( PNODE pNode )
{
    if( pNode->bLeaf )
    {
        pNode->nPalIndex = nPalIndex;
        aPal[ nPalIndex++ ] = BitmapColor( (sal_uInt8) ( (double) pNode->nRed / pNode->nCount ),
                                           (sal_uInt8) ( (double) pNode->nGreen / pNode->nCount ),
                                           (sal_uInt8) ( (double) pNode->nBlue / pNode->nCount ) );
    }
    else for( sal_uLong i = 0UL; i < 8UL; i++ )
        if( pNode->pChild[ i ] )
            CreatePalette( pNode->pChild[ i ] );

}

// ------------------------------------------------------------------------

void Octree::GetPalIndex( PNODE pNode )
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

// -------------------
// - InverseColorMap -
// -------------------

InverseColorMap::InverseColorMap( const BitmapPalette& rPal ) :
            nBits( 8 - OCTREE_BITS )
{
    sal_uLong*          cdp;
    sal_uInt8*           crgbp;
    const sal_uLong     nColorMax = 1 << OCTREE_BITS;
    const sal_uLong     xsqr = 1 << ( nBits << 1 );
    const sal_uLong     xsqr2 = xsqr << 1;
    const sal_uLong     nColors = rPal.GetEntryCount();
    const long      x = 1L << nBits;
    const long      x2 = x >> 1L;
    sal_uLong           r, g, b;
    long            rxx, gxx, bxx;
    long            rdist, gdist, bdist;
    long            crinc, cginc, cbinc;

    ImplCreateBuffers( nColorMax );

    for( sal_uLong nIndex = 0; nIndex < nColors; nIndex++ )
    {
        const BitmapColor&  rColor = rPal[ (sal_uInt16) nIndex ];
        const sal_uInt8         cRed = rColor.GetRed();
        const sal_uInt8         cGreen = rColor.GetGreen();
        const sal_uInt8         cBlue = rColor.GetBlue();

        rdist = cRed - x2;
        gdist = cGreen - x2;
        bdist = cBlue - x2;
        rdist = rdist*rdist + gdist*gdist + bdist*bdist;

        crinc = ( xsqr - ( cRed << nBits ) ) << 1L;
        cginc = ( xsqr - ( cGreen << nBits ) ) << 1L;
        cbinc = ( xsqr - ( cBlue << nBits ) ) << 1L;

        cdp = (sal_uLong*) pBuffer;
        crgbp = pMap;

        for( r = 0, rxx = crinc; r < nColorMax; rdist += rxx, r++, rxx += xsqr2 )
        {
            for( g = 0, gdist = rdist, gxx = cginc; g < nColorMax;  gdist += gxx, g++, gxx += xsqr2 )
            {
                for( b = 0, bdist = gdist, bxx = cbinc; b < nColorMax; bdist += bxx, b++, cdp++, crgbp++, bxx += xsqr2 )
                    if ( !nIndex || ( (long) *cdp ) > bdist )
                    {
                        *cdp = bdist;
                        *crgbp = (sal_uInt8) nIndex;
                    }
            }
        }
    }
}

// ------------------------------------------------------------------------

InverseColorMap::~InverseColorMap()
{
    rtl_freeMemory( pBuffer );
    rtl_freeMemory( pMap );
}

// ------------------------------------------------------------------------

void InverseColorMap::ImplCreateBuffers( const sal_uLong nMax )
{
    const sal_uLong nCount = nMax * nMax * nMax;
    const sal_uLong nSize = nCount * sizeof( sal_uLong );

    pMap = (sal_uInt8*) rtl_allocateMemory( nCount );
    memset( pMap, 0x00, nCount );

    pBuffer = (sal_uInt8*) rtl_allocateMemory( nSize );
    memset( pBuffer, 0xff, nSize );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
