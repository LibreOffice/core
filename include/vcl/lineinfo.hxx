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

#pragma once

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
    double                  mnWidth;
    double                  mnDashLen;
    double                  mnDotLen;
    double                  mnDistance;

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
                    LineInfo( LineStyle eLineStyle = LineStyle::Solid, double nWidth = 0 );
                    LineInfo( const LineInfo& rLineInfo );
                    LineInfo( LineInfo&& rLineInfo );
                    ~LineInfo();

    LineInfo&       operator=( const LineInfo& rLineInfo );
    LineInfo&       operator=( LineInfo&& rLineInfo );
    bool            operator==( const LineInfo& rLineInfo ) const;
    bool            operator!=( const LineInfo& rLineInfo ) const { return !(LineInfo::operator==( rLineInfo ) ); }

    void            SetStyle( LineStyle eStyle );
    LineStyle       GetStyle() const { return mpImpl->meStyle; }

    void            SetWidth( double nWidth );
    double          GetWidth() const { return mpImpl->mnWidth; }

    void            SetDashCount( sal_uInt16 nDashCount );
    sal_uInt16      GetDashCount() const { return mpImpl->mnDashCount; }

    void            SetDashLen( double nDashLen );
    double          GetDashLen() const { return mpImpl->mnDashLen; }

    void            SetDotCount( sal_uInt16 nDotCount );
    sal_uInt16      GetDotCount() const { return mpImpl->mnDotCount; }

    void            SetDotLen( double nDotLen );
    double          GetDotLen() const { return mpImpl->mnDotLen; }

    void            SetDistance( double nDistance );
    double          GetDistance() const { return mpImpl->mnDistance; }

    void SetLineJoin(basegfx::B2DLineJoin eLineJoin);
    basegfx::B2DLineJoin GetLineJoin() const { return mpImpl->meLineJoin; }

    void SetLineCap(css::drawing::LineCap eLineCap);
    css::drawing::LineCap GetLineCap() const { return mpImpl->meLineCap; }

    bool            IsDefault() const;

    friend SvStream& ReadLineInfo( SvStream& rIStm, LineInfo& rLineInfo );
    friend SvStream& WriteLineInfo( SvStream& rOStm, const LineInfo& rLineInfo );

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
friend class ::std::optional<LineInfo>;
friend class ::o3tl::cow_optional<LineInfo>;

    LineInfo(std::nullopt_t) noexcept;

    o3tl::cow_wrapper< ImplLineInfo >          mpImpl;
};

namespace std
{
    /** Specialise std::optional template for the case where we are wrapping a o3tl::cow_wrapper
        type, and we can make the pointer inside the cow_wrapper act as an empty value,
        and save ourselves some storage */
    template<>
    class optional<LineInfo> final : public o3tl::cow_optional<LineInfo>
    {
    public:
        using cow_optional::cow_optional; // inherit constructors
        optional(const optional&) = default;
        optional(optional&&) = default;
        optional& operator=(const optional&) = default;
        optional& operator=(optional&&) = default;
        ~optional();
        void reset();
    };
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
