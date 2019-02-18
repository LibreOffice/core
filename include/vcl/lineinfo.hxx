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

#ifndef INCLUDED_VCL_LINEINFO_HXX
#define INCLUDED_VCL_LINEINFO_HXX

#include <sal/types.h>
#include <vcl/dllapi.h>
#include <vcl/vclenum.hxx>
#include <basegfx/vector/b2enums.hxx>
#include <com/sun/star/drawing/LineCap.hpp>
#include <o3tl/cow_wrapper.hxx>

class SvStream;
namespace basegfx { class B2DPolyPolygon; }

struct ImplLineInfo
{
    sal_Int32               mnWidth;
    sal_Int32               mnDashLen;
    sal_Int32               mnDotLen;
    sal_Int32               mnDistance;

    basegfx::B2DLineJoin    meLineJoin;
    css::drawing::LineCap   meLineCap;
    LineStyle               meStyle;

    sal_uInt16              mnDashCount;
    sal_uInt16              mnDotCount;

    ImplLineInfo();

    bool operator==( const ImplLineInfo& ) const;
};


class VCL_DLLPUBLIC LineInfo
{
public:
                    LineInfo( LineStyle eLineStyle = LineStyle::Solid, sal_Int32 nWidth = 0 );
                    LineInfo( const LineInfo& rLineInfo );
                    LineInfo( LineInfo&& rLineInfo );
                    ~LineInfo();

    LineInfo&       operator=( const LineInfo& rLineInfo );
    LineInfo&       operator=( LineInfo&& rLineInfo );
    bool            operator==( const LineInfo& rLineInfo ) const;
    bool            operator!=( const LineInfo& rLineInfo ) const { return !(LineInfo::operator==( rLineInfo ) ); }

    void            SetStyle( LineStyle eStyle );
    LineStyle       GetStyle() const { return mpImplLineInfo->meStyle; }

    void            SetWidth( sal_Int32 nWidth );
    sal_Int32       GetWidth() const { return mpImplLineInfo->mnWidth; }

    void            SetDashCount( sal_uInt16 nDashCount );
    sal_uInt16      GetDashCount() const { return mpImplLineInfo->mnDashCount; }

    void            SetDashLen( sal_Int32 nDashLen );
    sal_Int32       GetDashLen() const { return mpImplLineInfo->mnDashLen; }

    void            SetDotCount( sal_uInt16 nDotCount );
    sal_uInt16      GetDotCount() const { return mpImplLineInfo->mnDotCount; }

    void            SetDotLen( sal_Int32 nDotLen );
    sal_Int32       GetDotLen() const { return mpImplLineInfo->mnDotLen; }

    void            SetDistance( sal_Int32 nDistance );
    sal_Int32       GetDistance() const { return mpImplLineInfo->mnDistance; }

    void SetLineJoin(basegfx::B2DLineJoin eLineJoin);
    basegfx::B2DLineJoin GetLineJoin() const { return mpImplLineInfo->meLineJoin; }

    void SetLineCap(css::drawing::LineCap eLineCap);
    css::drawing::LineCap GetLineCap() const { return mpImplLineInfo->meLineCap; }

    bool            IsDefault() const;

    friend VCL_DLLPUBLIC SvStream& ReadLineInfo( SvStream& rIStm, LineInfo& rLineInfo );
    friend VCL_DLLPUBLIC SvStream& WriteLineInfo( SvStream& rOStm, const LineInfo& rLineInfo );

    // helper to get decomposed polygon data with the LineInfo applied. The source
    // hairline polygon is given in io_rLinePolyPolygon. Both given polygons may
    // contain results; e.g. when no fat line but DashDot is defined, the result will
    // be in io_rLinePolyPolygon while o_rFillPolyPolygon will be empty. When fat line
    // is defined, it will be vice-versa. If none is defined, io_rLinePolyPolygon will
    // not be changed (but o_rFillPolyPolygon will be freed)
    void applyToB2DPolyPolygon(
        basegfx::B2DPolyPolygon& io_rLinePolyPolygon,
        basegfx::B2DPolyPolygon& o_rFillPolyPolygon) const;

private:
    o3tl::cow_wrapper< ImplLineInfo >          mpImplLineInfo;
};

#endif // INCLUDED_VCL_LINEINFO_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
