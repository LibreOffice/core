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
#ifndef INCLUDED_VCL_INC_REGIONBAND_HXX
#define INCLUDED_VCL_INC_REGIONBAND_HXX

#include <vcl/region.hxx>

#include "regband.hxx"

#ifdef DBG_UTIL
const char* ImplDbgTestRegionBand(const void*);
#endif

class RegionBand
{
private:
    friend const char* ImplDbgTestRegionBand(const void*);

    ImplRegionBand*             mpFirstBand;        // root of the list with y-bands
    ImplRegionBand*             mpLastCheckedBand;

    void implReset();

public:
    RegionBand();
    RegionBand(const RegionBand&);
    RegionBand& operator=(const RegionBand&);
    RegionBand(const tools::Rectangle&);
    ~RegionBand();

    bool operator==( const RegionBand& rRegionBand ) const;

    void load(SvStream& rIStrm);
    void save(SvStream& rIStrm) const;

    bool isSingleRectangle() const;
    ImplRegionBand* ImplGetFirstRegionBand() const { return mpFirstBand; }
    void ImplAddMissingBands(const sal_Int32 nTop, const sal_Int32 nBottom);
    void InsertBand(ImplRegionBand* pPreviousBand, ImplRegionBand* pBandToInsert);
    void processPoints();
    void CreateBandRange(sal_Int32 nYTop, sal_Int32 nYBottom);
    void InsertLine(const Point& rStartPt, const Point& rEndPt, sal_Int32 nLineId);
    void InsertPoint(const Point &rPoint, sal_Int32 nLineID, bool bEndPoint, LineType eLineType);
    bool OptimizeBandList();
    void Move(sal_Int32 nHorzMove, sal_Int32 nVertMove);
    void Scale(double fScaleX, double fScaleY);
    void InsertBands(sal_Int32 nTop, sal_Int32 nBottom);
    static bool InsertSingleBand(ImplRegionBand* pBand, sal_Int32 nYBandPosition);
    void Union(sal_Int32 nLeft, sal_Int32 nTop, sal_Int32 nRight, sal_Int32 nBottom);
    void Intersect(sal_Int32 nLeft, sal_Int32 nTop, sal_Int32 nRight, sal_Int32 nBottom);
    void Union(const RegionBand& rSource);
    void Exclude(sal_Int32 nLeft, sal_Int32 nTop, sal_Int32 nRight, sal_Int32 nBottom);
    void XOr(sal_Int32 nLeft, sal_Int32 nTop, sal_Int32 nRight, sal_Int32 nBottom);
    void Intersect(const RegionBand& rSource);
    bool Exclude(const RegionBand& rSource);
    void XOr(const RegionBand& rSource);
    tools::Rectangle GetBoundRect() const;
    bool IsInside(const Point& rPoint) const;
    sal_uInt32 getRectangleCount() const; // only users are Region::Intersect, Region::IsRectangle and PSWriter::ImplBmp
    void GetRegionRectangles(RectangleVector& rTarget) const;
};

#endif // INCLUDED_VCL_INC_REGIONBAND_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
