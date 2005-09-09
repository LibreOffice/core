/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: impvect.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 12:03:06 $
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

#ifndef _SV_IMPVECT_HXX
#define _SV_IMPVECT_HXX

#ifndef _TL_POLY_HXX
#include <tools/poly.hxx>
#endif
#ifndef _SV_GDIMTF_HXX
#include <gdimtf.hxx>
#endif

// --------------
// - Vectorizer -
// --------------

class BitmapReadAccess;
class ImplChain;
class ImplVectMap;

class ImplVectorizer
{
private:

    ImplVectMap*    ImplExpand( BitmapReadAccess* pRAcc, const Color& rColor );
    void            ImplCalculate( ImplVectMap* pMap, PolyPolygon& rPolyPoly, BYTE cReduce, ULONG nFlags );
    BOOL            ImplGetChain( ImplVectMap* pMap, const Point& rStartPt, ImplChain& rChain );
    BOOL            ImplIsUp( ImplVectMap* pMap, long nY, long nX ) const;
    void            ImplLimitPolyPoly( PolyPolygon& rPolyPoly );

public:

                    ImplVectorizer();
                    ~ImplVectorizer();

    BOOL            ImplVectorize( const Bitmap& rColorBmp, GDIMetaFile& rMtf,
                                   BYTE cReduce, ULONG nFlags, const Link* pProgress );
    BOOL            ImplVectorize( const Bitmap& rMonoBmp, PolyPolygon& rPolyPoly,
                                   ULONG nFlags, const Link* pProgress );
};

#endif
