/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: impoct.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2007-07-24 10:00:21 $
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

#ifndef _SV_IMPOCT_HXX
#define _SV_IMPOCT_HXX

#ifndef _SV_OCTREE_HXX
#include <vcl/octree.hxx>
#endif

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
