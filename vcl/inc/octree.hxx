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

#ifndef INCLUDED_VCL_INC_OCTREE_HXX
#define INCLUDED_VCL_INC_OCTREE_HXX

#include <vcl/salbtype.hxx>
#include <vcl/dllapi.h>

#define OCTREE_BITS     5
#define OCTREE_BITS_1   10

typedef struct OctreeNode
{
    sal_uLong       nCount;
    sal_uLong       nRed;
    sal_uLong       nGreen;
    sal_uLong       nBlue;
    OctreeNode* pChild[ 8 ];
    OctreeNode* pNext;
    OctreeNode* pNextInCache;
    sal_uInt16      nPalIndex;
    bool        bLeaf;
} NODE;

typedef NODE*       PNODE;
typedef PNODE*      PPNODE;

class ImpNodeCache;
class BitmapReadAccess;

class VCL_PLUGIN_PUBLIC Octree
{
private:

    BitmapPalette               aPal;
    sal_uLong                       nMax;
    sal_uLong                       nLeafCount;
    sal_uLong                       nLevel;
    PNODE                       pTree;
    PNODE                       pReduce[ OCTREE_BITS + 1 ];
    BitmapColor*                pColor;
    ImpNodeCache*               pNodeCache;
    const BitmapReadAccess*     pAcc;
    sal_uInt16                      nPalIndex;

                                Octree() {}

    void                        CreatePalette( PNODE pNode );
    void                        GetPalIndex( PNODE pNode );

    SAL_DLLPRIVATE void         ImplCreateOctree();
    SAL_DLLPRIVATE void         ImplDeleteOctree( PPNODE ppNode );
    SAL_DLLPRIVATE void         ImplAdd( PPNODE ppNode );
    SAL_DLLPRIVATE void         ImplReduce();

public:

                                Octree( const BitmapReadAccess& rReadAcc, sal_uLong nColors );
                                ~Octree();

    inline const BitmapPalette& GetPalette();
    inline sal_uInt16               GetBestPaletteIndex( const BitmapColor& rColor );
};

inline const BitmapPalette& Octree::GetPalette()
{
    aPal.SetEntryCount( (sal_uInt16) nLeafCount );
    nPalIndex = 0;
    CreatePalette( pTree );
    return aPal;
}

inline sal_uInt16 Octree::GetBestPaletteIndex( const BitmapColor& rColor )
{
    pColor = &(BitmapColor&) rColor;
    nPalIndex = 65535;
    nLevel = 0L;
    GetPalIndex( pTree );
    return nPalIndex;
}

class VCL_PLUGIN_PUBLIC InverseColorMap
{
private:

    sal_uInt8*              pBuffer;
    sal_uInt8*              pMap;
    const sal_uLong         nBits;


    SAL_DLLPRIVATE void ImplCreateBuffers( const sal_uLong nMax );

public:

    explicit            InverseColorMap( const BitmapPalette& rPal );
                        ~InverseColorMap();

    inline sal_uInt16       GetBestPaletteIndex( const BitmapColor& rColor );
};

inline sal_uInt16 InverseColorMap::GetBestPaletteIndex( const BitmapColor& rColor )
{
    return pMap[ ( ( (sal_uLong) rColor.GetRed() >> nBits ) << OCTREE_BITS_1 ) |
                 ( ( (sal_uLong) rColor.GetGreen() >> nBits ) << OCTREE_BITS ) |
                 ( (sal_uLong) rColor.GetBlue() >> nBits ) ];
}

#endif // INCLUDED_VCL_INC_OCTREE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
