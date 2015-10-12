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

#ifndef INCLUDED_EDITENG_TXTRANGE_HXX
#define INCLUDED_EDITENG_TXTRANGE_HXX

#include <editeng/editengdllapi.h>
#include <tools/gen.hxx>
#include <deque>

namespace tools { class PolyPolygon; }
class Rectangle;

namespace basegfx {
    class B2DPolyPolygon;
}

typedef std::deque<long>* LongDqPtr;

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
    tools::PolyPolygon *mpPolyPolygon; // Surface polygon
    tools::PolyPolygon *mpLinePolyPolygon; // Line polygon
    Rectangle *pBound;  // Comprehensive rectangle
    sal_uInt16 nCacheSize;  // Cache-Size
    sal_uInt16 nRight;      // Distance Contour-Text
    sal_uInt16 nLeft;       // Distance Text-Contour
    sal_uInt16 nUpper;      // Distance Contour-Text
    sal_uInt16 nLower;      // Distance Text-Contour
    sal_uInt32 nPointCount; // Number of polygon points
    bool       bSimple : 1; // Just outside edge
    bool       bInner  : 1; // TRUE: Objekt inline (EditEngine);
                            // FALSE: Objekt flow (StarWriter);
    bool       bVertical :1;// for vertical writing mode

    TextRanger( const TextRanger& ) = delete;
    const Rectangle& _GetBoundRect();
public:
    TextRanger( const basegfx::B2DPolyPolygon& rPolyPolygon,
                const basegfx::B2DPolyPolygon* pLinePolyPolygon,
                sal_uInt16 nCacheSize, sal_uInt16 nLeft, sal_uInt16 nRight,
                bool bSimple, bool bInner, bool bVert = false );
    ~TextRanger();
    LongDqPtr GetTextRanges( const Range& rRange );
    sal_uInt16 GetRight() const { return nRight; }
    sal_uInt16 GetLeft() const { return nLeft; }
    sal_uInt16 GetUpper() const { return nUpper; }
    sal_uInt16 GetLower() const { return nLower; }
    sal_uInt32 GetPointCount() const { return nPointCount; }
    bool IsSimple() const { return bSimple; }
    bool IsInner() const { return bInner; }
    bool IsVertical() const { return bVertical; }
    const tools::PolyPolygon& GetPolyPolygon() const { return *mpPolyPolygon; }
    const Rectangle& GetBoundRect()
        { return pBound ? static_cast< const Rectangle& >(*pBound) : _GetBoundRect(); }
    void SetUpper( sal_uInt16 nNew ){ nUpper = nNew; }
    void SetLower( sal_uInt16 nNew ){ nLower = nNew; }
    void SetVertical( bool bNew );
};

#endif      // _TXTRANGE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
