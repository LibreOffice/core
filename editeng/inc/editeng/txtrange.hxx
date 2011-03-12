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

#ifndef _MyTXTRANGE_HXX
#define _MyTXTRANGE_HXX

#ifndef _TXTRANGE_HXX
#define _SVSTDARR_sal_BoolS
#define _SVSTDARR_LONGS
#include <svl/svstdarr.hxx>
#endif
#include "editeng/editengdllapi.h"

class PolyPolygon;
class Range;
class Rectangle;

namespace basegfx {
    class B2DPolyPolygon;
}

typedef SvLongs* SvLongsPtr;

/*************************************************************************
|*
|*    class TextRanger
|*
*************************************************************************/
class EDITENG_DLLPUBLIC TextRanger
{
    Range *pRangeArr;
    SvLongsPtr *pCache;
    PolyPolygon *mpPolyPolygon; // Surface polygon
    PolyPolygon *mpLinePolyPolygon; // Line polygon
    Rectangle *pBound;  // Comprehensive rectangle
    sal_uInt16 nCacheSize;  // Cache-Size
    sal_uInt16 nCacheIdx;   // Cache-Index
    sal_uInt16 nRight;      // Distance Contour-Text
    sal_uInt16 nLeft;       // Distance Text-Contour
    sal_uInt16 nUpper;      // Distance Contour-Text
    sal_uInt16 nLower;      // Distance Text-Contour
    sal_uInt32 nPointCount; // Number of polygon points
    sal_Bool bSimple : 1;   // Just outside edge
    sal_Bool bInner  : 1;   // TRUE: Objekt inline (EditEngine);
                        // FALSE: Objekt flow (StarWriter);
    sal_Bool bVertical :1;  // for vertical writing mode
    sal_Bool bFlag3 :1;
    sal_Bool bFlag4 :1;
    sal_Bool bFlag5 :1;
    sal_Bool bFlag6 :1;
    sal_Bool bFlag7 :1;
    TextRanger( const TextRanger& ); // not implemented
    const Rectangle& _GetBoundRect();
public:
    TextRanger( const basegfx::B2DPolyPolygon& rPolyPolygon, const basegfx::B2DPolyPolygon* pLinePolyPolygon,
                sal_uInt16 nCacheSize, sal_uInt16 nLeft, sal_uInt16 nRight,
                sal_Bool bSimple, sal_Bool bInner, sal_Bool bVert = sal_False );
    ~TextRanger();
    SvLongsPtr GetTextRanges( const Range& rRange );
    sal_uInt16 GetRight() const { return nRight; }
    sal_uInt16 GetLeft() const { return nLeft; }
    sal_uInt16 GetUpper() const { return nUpper; }
    sal_uInt16 GetLower() const { return nLower; }
    sal_uInt32 GetPointCount() const { return nPointCount; }
    sal_Bool IsSimple() const { return bSimple; }
    sal_Bool IsInner() const { return bInner; }
    sal_Bool IsVertical() const { return bVertical; }
    sal_Bool HasBorder() const { return nRight || nLeft; }
    const PolyPolygon& GetPolyPolygon() const { return *mpPolyPolygon; }
    const PolyPolygon* GetLinePolygon() const { return mpLinePolyPolygon; }
    const Rectangle& GetBoundRect()
        { return pBound ? static_cast< const Rectangle& >(*pBound) : _GetBoundRect(); }
    void SetUpper( sal_uInt16 nNew ){ nUpper = nNew; }
    void SetLower( sal_uInt16 nNew ){ nLower = nNew; }
    void SetVertical( sal_Bool bNew );
    sal_Bool IsFlag3() const { return bFlag3; }
    void SetFlag3( sal_Bool bNew ) { bFlag3 = bNew; }
    sal_Bool IsFlag4() const { return bFlag4; }
    void SetFlag4( sal_Bool bNew ) { bFlag4 = bNew; }
    sal_Bool IsFlag5() const { return bFlag5; }
    void SetFlag5( sal_Bool bNew ) { bFlag5 = bNew; }
    sal_Bool IsFlag6() const { return bFlag6; }
    void SetFlag6( sal_Bool bNew ) { bFlag6 = bNew; }
    sal_Bool IsFlag7() const { return bFlag7; }
    void SetFlag7( sal_Bool bNew ) { bFlag7 = bNew; }
};



#endif      // _TXTRANGE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
