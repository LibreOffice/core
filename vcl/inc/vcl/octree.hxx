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



#ifndef _SV_OCTREE_HXX
#define _SV_OCTREE_HXX

#include <vcl/salbtype.hxx>
#include <vcl/dllapi.h>

// -----------
// - Defines -
// -----------

#define OCTREE_BITS     5
#define OCTREE_BITS_1   10

// --------------
// - OctreeNode -
// --------------

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
    sal_Bool        bLeaf;
} NODE;

typedef NODE*       PNODE;
typedef PNODE*      PPNODE;

// ----------
// - Octree -
// ----------

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

                                Octree() {};

    void                        CreatePalette( PNODE pNode );
    void                        GetPalIndex( PNODE pNode );

    SAL_DLLPRIVATE void         ImplCreateOctree();
    SAL_DLLPRIVATE void         ImplDeleteOctree( PPNODE ppNode );
    SAL_DLLPRIVATE void         ImplAdd( PPNODE ppNode );
    SAL_DLLPRIVATE void         ImplReduce();

public:

                                Octree( const BitmapReadAccess& rReadAcc, sal_uLong nColors );
                                Octree( sal_uLong nColors );
                                ~Octree();

    void                        AddColor( const BitmapColor& rColor );

    inline const BitmapPalette& GetPalette();
    inline sal_uInt16               GetBestPaletteIndex( const BitmapColor& rColor );
};

// ------------------------------------------------------------------------

inline const BitmapPalette& Octree::GetPalette()
{
    aPal.SetEntryCount( (sal_uInt16) nLeafCount );
    nPalIndex = 0;
    CreatePalette( pTree );
    return aPal;
}

// ------------------------------------------------------------------------

inline sal_uInt16 Octree::GetBestPaletteIndex( const BitmapColor& rColor )
{
    pColor = &(BitmapColor&) rColor;
    nPalIndex = 65535;
    nLevel = 0L;
    GetPalIndex( pTree );
    return nPalIndex;
}

// -------------------
// - InverseColorMap -
// -------------------

class VCL_PLUGIN_PUBLIC InverseColorMap
{
private:

    sal_uInt8*              pBuffer;
    sal_uInt8*              pMap;
    const sal_uLong         nBits;

//#if 0 // _SOLAR__PRIVATE

    SAL_DLLPRIVATE void ImplCreateBuffers( const sal_uLong nMax );

//#endif // __PRIVATE

public:

    explicit            InverseColorMap( const BitmapPalette& rPal );
                        ~InverseColorMap();

    inline sal_uInt16       GetBestPaletteIndex( const BitmapColor& rColor );
};

// ------------------------------------------------------------------------

inline sal_uInt16 InverseColorMap::GetBestPaletteIndex( const BitmapColor& rColor )
{
    return pMap[ ( ( (sal_uLong) rColor.GetRed() >> nBits ) << OCTREE_BITS_1 ) |
                 ( ( (sal_uLong) rColor.GetGreen() >> nBits ) << OCTREE_BITS ) |
                 ( (sal_uLong) rColor.GetBlue() >> nBits ) ];
}

#endif // _SV_OCTREE_HXX
