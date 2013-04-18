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

#ifndef _MyTXTRANGE_HXX
#define _MyTXTRANGE_HXX

#include "editeng/editengdllapi.h"
#include "tools/solar.h"
#include "tools/gen.hxx"

#include <deque>

class PolyPolygon;
class Rectangle;

namespace basegfx {
    class B2DPolyPolygon;
}

typedef std::deque<long>* LongDqPtr;

/*************************************************************************
|*
|*    class TextRanger
|*
*************************************************************************/
class EDITENG_DLLPUBLIC TextRanger
{
    //! The RangeCache class is used to cache the result of a single range calculation.
    struct RangeCache
    {
        Range range;        //!< Range for which we calculated results.
        std::deque<long> results;  //!< Calculated results for the range.
        RangeCache(const Range& rng) : range(rng) {};
    };
    std::deque<RangeCache> mRangeCache; //!< Cached range calculations.
    PolyPolygon *mpPolyPolygon; // Surface polygon
    PolyPolygon *mpLinePolyPolygon; // Line polygon
    Rectangle *pBound;  // Comprehensive rectangle
    sal_uInt16 nCacheSize;  // Cache-Size
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
    TextRanger( const basegfx::B2DPolyPolygon& rPolyPolygon,
                const basegfx::B2DPolyPolygon* pLinePolyPolygon,
                sal_uInt16 nCacheSize, sal_uInt16 nLeft, sal_uInt16 nRight,
                sal_Bool bSimple, sal_Bool bInner, sal_Bool bVert = sal_False );
    ~TextRanger();
    LongDqPtr GetTextRanges( const Range& rRange );
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
