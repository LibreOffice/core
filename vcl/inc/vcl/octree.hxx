/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: octree.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 18:01:36 $
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

#ifndef _SV_OCTREE_HXX
#define _SV_OCTREE_HXX

#ifndef _SV_SALBTYPE_HXX
#include <vcl/salbtype.hxx>
#endif

#ifndef _VCL_DLLAPI_H
#include <vcl/dllapi.h>
#endif

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
    ULONG       nCount;
    ULONG       nRed;
    ULONG       nGreen;
    ULONG       nBlue;
    OctreeNode* pChild[ 8 ];
    OctreeNode* pNext;
    OctreeNode* pNextInCache;
    USHORT      nPalIndex;
    BOOL        bLeaf;
} NODE;

typedef NODE*       PNODE;
typedef PNODE*      PPNODE;

// ----------
// - Octree -
// ----------

class ImpNodeCache;
class BitmapReadAccess;

class VCL_DLLPUBLIC Octree
{
private:

    BitmapPalette               aPal;
    ULONG                       nMax;
    ULONG                       nLeafCount;
    ULONG                       nLevel;
    PNODE                       pTree;
    PNODE                       pReduce[ OCTREE_BITS + 1 ];
    BitmapColor*                pColor;
    ImpNodeCache*               pNodeCache;
    const BitmapReadAccess*     pAcc;
    USHORT                      nPalIndex;

                                Octree() {};

    void                        CreatePalette( PNODE pNode );
    void                        GetPalIndex( PNODE pNode );

    SAL_DLLPRIVATE void         ImplCreateOctree();
    SAL_DLLPRIVATE void         ImplDeleteOctree( PPNODE ppNode );
    SAL_DLLPRIVATE void         ImplAdd( PPNODE ppNode );
    SAL_DLLPRIVATE void         ImplReduce();

public:

                                Octree( const BitmapReadAccess& rReadAcc, ULONG nColors );
                                Octree( ULONG nColors );
                                ~Octree();

    void                        AddColor( const BitmapColor& rColor );

    inline const BitmapPalette& GetPalette();
    inline USHORT               GetBestPaletteIndex( const BitmapColor& rColor );
};

// ------------------------------------------------------------------------

inline const BitmapPalette& Octree::GetPalette()
{
    aPal.SetEntryCount( (USHORT) nLeafCount );
    nPalIndex = 0;
    CreatePalette( pTree );
    return aPal;
}

// ------------------------------------------------------------------------

inline USHORT Octree::GetBestPaletteIndex( const BitmapColor& rColor )
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

class VCL_DLLPUBLIC InverseColorMap
{
private:

    BYTE*               pBuffer;
    BYTE*               pMap;
    const ULONG         nBits;

//#if 0 // _SOLAR__PRIVATE

    SAL_DLLPRIVATE void ImplCreateBuffers( const ULONG nMax );

//#endif // __PRIVATE

public:

                        InverseColorMap( const BitmapPalette& rPal );
                        ~InverseColorMap();

    inline USHORT       GetBestPaletteIndex( const BitmapColor& rColor );
};

// ------------------------------------------------------------------------

inline USHORT InverseColorMap::GetBestPaletteIndex( const BitmapColor& rColor )
{
    return pMap[ ( ( (ULONG) rColor.GetRed() >> nBits ) << OCTREE_BITS_1 ) |
                 ( ( (ULONG) rColor.GetGreen() >> nBits ) << OCTREE_BITS ) |
                 ( (ULONG) rColor.GetBlue() >> nBits ) ];
}

#endif // _SV_OCTREE_HXX
