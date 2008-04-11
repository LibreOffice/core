/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: impoct.hxx,v $
 * $Revision: 1.3 $
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

#ifndef _SV_IMPOCT_HXX
#define _SV_IMPOCT_HXX

#include <vcl/octree.hxx>

// ----------------
// - ImpErrorQuad -
// ----------------

class ImpErrorQuad
{
    long                    nRed;
    long                    nGreen;
    long                    nBlue;
    long                    nReserved;

public:

    inline                  ImpErrorQuad() {}
    inline                  ImpErrorQuad( const BitmapColor& rColor ) :
                                nRed    ( (long) rColor.GetRed() << 5L ),
                                nGreen  ( (long) rColor.GetGreen() << 5L ),
                                nBlue   ( (long) rColor.GetBlue() << 5L ) {}

    inline void             operator=( const BitmapColor& rColor );
    inline ImpErrorQuad&    operator-=( const BitmapColor& rColor );

    inline void             ImplAddColorError1( const ImpErrorQuad& rErrQuad );
    inline void             ImplAddColorError3( const ImpErrorQuad& rErrQuad );
    inline void             ImplAddColorError5( const ImpErrorQuad& rErrQuad );
    inline void             ImplAddColorError7( const ImpErrorQuad& rErrQuad );

    inline BitmapColor      ImplGetColor();
};

// ------------------------------------------------------------------------

inline void ImpErrorQuad::operator=( const BitmapColor& rColor )
{
    nRed = (long) rColor.GetRed() << 5L;
    nGreen = (long) rColor.GetGreen() << 5L;
    nBlue = (long) rColor.GetBlue() << 5L;
}

// ------------------------------------------------------------------------

inline ImpErrorQuad& ImpErrorQuad::operator-=( const BitmapColor& rColor )
{
    nRed -= ( (long) rColor.GetRed() << 5L );
    nGreen -= ( (long) rColor.GetGreen() << 5L );
    nBlue -= ( (long) rColor.GetBlue() << 5L );

    return *this;
}

// ------------------------------------------------------------------------

inline void ImpErrorQuad::ImplAddColorError1( const ImpErrorQuad& rErrQuad )
{
    nRed += ( rErrQuad.nRed >> 4L );
    nGreen += ( rErrQuad.nGreen >> 4L );
    nBlue += ( rErrQuad.nBlue >> 4L );
}

// ------------------------------------------------------------------------

inline void ImpErrorQuad::ImplAddColorError3( const ImpErrorQuad& rErrQuad )
{
    nRed += ( rErrQuad.nRed * 3L >> 4L );
    nGreen += ( rErrQuad.nGreen * 3L >> 4L );
    nBlue += ( rErrQuad.nBlue * 3L >> 4L );
}

// ------------------------------------------------------------------------

inline void ImpErrorQuad::ImplAddColorError5( const ImpErrorQuad& rErrQuad )
{
    nRed += ( rErrQuad.nRed * 5L >> 4L );
    nGreen += ( rErrQuad.nGreen * 5L >> 4L );
    nBlue += ( rErrQuad.nBlue * 5L >> 4L );
}

// ------------------------------------------------------------------------

inline void ImpErrorQuad::ImplAddColorError7( const ImpErrorQuad& rErrQuad )
{
    nRed += ( rErrQuad.nRed * 7L >> 4L );
    nGreen += ( rErrQuad.nGreen * 7L >> 4L );
    nBlue += ( rErrQuad.nBlue *7L >> 4L );
}

// ------------------------------------------------------------------------

inline BitmapColor ImpErrorQuad::ImplGetColor()
{
    return BitmapColor( (BYTE) ( ( nRed < 0L ? 0L : nRed > 8160L ? 8160L : nRed ) >> 5L ),
                        (BYTE) ( ( nGreen < 0L ? 0L : nGreen > 8160L ? 8160L : nGreen ) >> 5L ),
                        (BYTE) ( ( nBlue < 0L ? 0L : nBlue > 8160L ? 8160L : nBlue ) >> 5L ) );
}

// -------------
// - NodeCache -
// -------------

class ImpNodeCache
{
    OctreeNode*         pActNode;
    ULONG               nNew;
    ULONG               nDelete;
    ULONG               nGet;
    ULONG               nRelease;

public:

                        ImpNodeCache( const ULONG nInitSize );
                        ~ImpNodeCache();

    inline OctreeNode*  ImplGetFreeNode();
    inline void         ImplReleaseNode( OctreeNode* pNode );
};

// ------------------------------------------------------------------------

inline OctreeNode* ImpNodeCache::ImplGetFreeNode()
{
    OctreeNode* pNode;

    if ( !pActNode )
    {
        pActNode = new NODE;
        pActNode->pNextInCache = NULL;
    }

    pNode = pActNode;
    pActNode = pNode->pNextInCache;
    memset( pNode, 0, sizeof( NODE ) );

    return pNode;
}

// ------------------------------------------------------------------------

inline void ImpNodeCache::ImplReleaseNode( OctreeNode* pNode )
{
    pNode->pNextInCache = pActNode;
    pActNode = pNode;
}

#endif // _SV_IMPOCT_HXX
