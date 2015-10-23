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


// - ImplLineInfo -


class SvStream;
namespace basegfx { class B2DPolyPolygon; }

struct ImplLineInfo
{
    sal_uInt32              mnRefCount;
    LineStyle               meStyle;
    long                    mnWidth;
    sal_uInt16              mnDashCount;
    long                    mnDashLen;
    sal_uInt16              mnDotCount;
    long                    mnDotLen;
    long                    mnDistance;

    basegfx::B2DLineJoin    meLineJoin;
    css::drawing::LineCap   meLineCap;

                        ImplLineInfo();
                        ImplLineInfo( const ImplLineInfo& rImplLineInfo );

    bool operator==( const ImplLineInfo& ) const;

    friend SvStream&    ReadImplLineInfo( SvStream& rIStm, ImplLineInfo& rImplLineInfo );
    friend SvStream&    WriteImplLineInfo( SvStream& rOStm, const ImplLineInfo& rImplLineInfo );
};


// - LineInfo -


class VCL_DLLPUBLIC LineInfo
{
private:

    ImplLineInfo*   mpImplLineInfo;

    SAL_DLLPRIVATE void ImplMakeUnique();

public:

                    LineInfo( LineStyle eLineStyle = LINE_SOLID, long nWidth = 0L );
                    LineInfo( const LineInfo& rLineInfo );
                    ~LineInfo();

    LineInfo&       operator=( const LineInfo& rLineInfo );
    bool            operator==( const LineInfo& rLineInfo ) const;
    bool            operator!=( const LineInfo& rLineInfo ) const { return !(LineInfo::operator==( rLineInfo ) ); }

    void            SetStyle( LineStyle eStyle );
    LineStyle       GetStyle() const { return mpImplLineInfo->meStyle; }

    void            SetWidth( long nWidth );
    long            GetWidth() const { return mpImplLineInfo->mnWidth; }

    void            SetDashCount( sal_uInt16 nDashCount );
    sal_uInt16          GetDashCount() const { return mpImplLineInfo->mnDashCount; }

    void            SetDashLen( long nDashLen );
    long            GetDashLen() const { return mpImplLineInfo->mnDashLen; }

    void            SetDotCount( sal_uInt16 nDotCount );
    sal_uInt16          GetDotCount() const { return mpImplLineInfo->mnDotCount; }

    void            SetDotLen( long nDotLen );
    long            GetDotLen() const { return mpImplLineInfo->mnDotLen; }

    void            SetDistance( long nDistance );
    long            GetDistance() const { return mpImplLineInfo->mnDistance; }

    void SetLineJoin(basegfx::B2DLineJoin eLineJoin);
    basegfx::B2DLineJoin GetLineJoin() const { return mpImplLineInfo->meLineJoin; }

    void SetLineCap(css::drawing::LineCap eLineCap);
    css::drawing::LineCap GetLineCap() const { return mpImplLineInfo->meLineCap; }

    bool            IsDefault() const;

    friend VCL_DLLPUBLIC SvStream& ReadLineInfo( SvStream& rIStm, LineInfo& rLineInfo );
    friend VCL_DLLPUBLIC SvStream& WriteLineInfo( SvStream& rOStm, const LineInfo& rLineInfo );

    // helper to get decomposed polygon data with the LineInfo applied. The source
    // hairline polygon is given in io_rLinePolyPolygon. Both given polygons may
    // contain results; e.g. when no fat line but DasDot is defined, the result will
    // be in io_rLinePolyPolygon while o_rFillPolyPolygon will be empty. When fat line
    // is defined, it will be vice-versa. If none is defined, io_rLinePolyPolygon will
    // not be changed (but o_rFillPolyPolygon will be freed)
    void applyToB2DPolyPolygon(
        basegfx::B2DPolyPolygon& io_rLinePolyPolygon,
        basegfx::B2DPolyPolygon& o_rFillPolyPolygon) const;
};

#endif // INCLUDED_VCL_LINEINFO_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
