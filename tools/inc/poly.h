/*************************************************************************
 *
 *  $RCSfile: poly.h,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-27 17:03:25 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _POLY_H
#define _POLY_H

#ifndef _GEN_HXX
#include <gen.hxx>
#endif

#define MAX_64KPOINTS       ((((USHORT)0xFFFF)-32)/sizeof(Point))

// -------------------
// - ImplPolygonData -
// -------------------

class ImplPolygonData
{
public:
#ifdef WIN
    Point huge*     mpPointAry;
    BYTE*           mpFlagAry;
    GLOBALHANDLE    mhPoints;
#else
    Point*          mpPointAry;
    BYTE*           mpFlagAry;
#endif

    USHORT          mnPoints;
    USHORT          mnRefCount;
};

// ---------------
// - ImplPolygon -
// ---------------

class ImplPolygon  : public ImplPolygonData
{
public:
                    ImplPolygon( USHORT nInitSize, BOOL bFlags = FALSE );
                    ImplPolygon( USHORT nPoints, const Point* pPtAry, const BYTE* pInitFlags = NULL );
                    ImplPolygon( const ImplPolygon& rImplPoly );
                    ~ImplPolygon();

    void            ImplSetSize( USHORT nSize, BOOL bResize = TRUE );
    void            ImplCreateFlagArray();
    void            ImplSplit( USHORT nPos, USHORT nSpace, ImplPolygon* pInitPoly = NULL );
    void            ImplRemove( USHORT nPos, USHORT nCount );
};

// -------------------
// - ImplPolyPolygon -
// -------------------

#define MAX_POLYGONS        ((USHORT)0x3FF0)

class Polygon;
typedef Polygon* SVPPOLYGON;

class ImplPolyPolygon
{
public:
    SVPPOLYGON*     mpPolyAry;
    USHORT          mnCount;
    USHORT          mnRefCount;
    USHORT          mnSize;
    USHORT          mnResize;

                    ImplPolyPolygon( USHORT nInitSize, USHORT nResize )
                        { mpPolyAry = NULL; mnCount = 0; mnRefCount = 1;
                          mnSize = nInitSize; mnResize = nResize; }
                    ImplPolyPolygon( USHORT nInitSize );
                    ImplPolyPolygon( const ImplPolyPolygon& rImplPolyPoly );
                    ~ImplPolyPolygon();
};

inline long MinMax( long nVal, long nMin, long nMax )
{
    return( nVal >= nMin ? ( nVal <= nMax ? nVal : nMax ) : nMin );
}

// ------------------------------------------------------------------

inline long FRound( double fVal )
{
    return( fVal > 0.0 ? (long) ( fVal + 0.5 ) : -(long) ( -fVal + 0.5 ) );
}


#endif // _SV_POLY_H
