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

#ifndef _SV_IMPVECT_HXX
#define _SV_IMPVECT_HXX

#include <tools/poly.hxx>
#include <vcl/gdimtf.hxx>

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
    void            ImplCalculate( ImplVectMap* pMap, PolyPolygon& rPolyPoly, sal_uInt8 cReduce, sal_uLong nFlags );
    sal_Bool            ImplGetChain( ImplVectMap* pMap, const Point& rStartPt, ImplChain& rChain );
    sal_Bool            ImplIsUp( ImplVectMap* pMap, long nY, long nX ) const;
    void            ImplLimitPolyPoly( PolyPolygon& rPolyPoly );

public:

                    ImplVectorizer();
                    ~ImplVectorizer();

    sal_Bool            ImplVectorize( const Bitmap& rColorBmp, GDIMetaFile& rMtf,
                                   sal_uInt8 cReduce, sal_uLong nFlags, const Link* pProgress );
    sal_Bool            ImplVectorize( const Bitmap& rMonoBmp, PolyPolygon& rPolyPoly,
                                   sal_uLong nFlags, const Link* pProgress );
};

#endif
