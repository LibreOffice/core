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

#ifndef _SV_LINEINFO_HXX
#define _SV_LINEINFO_HXX

#include <vcl/dllapi.h>
#include <tools/gen.hxx>
#include <vcl/vclenum.hxx>
#include <basegfx/vector/b2enums.hxx>

// ----------------
// - ImplLineInfo -
// ----------------

class SvStream;
namespace basegfx { class B2DPolyPolygon; }

struct ImplLineInfo
{
    sal_uLong                   mnRefCount;
    LineStyle               meStyle;
    long                    mnWidth;
    sal_uInt16                  mnDashCount;
    long                    mnDashLen;
    sal_uInt16                  mnDotCount;
    long                    mnDotLen;
    long                    mnDistance;

    basegfx::B2DLineJoin    meLineJoin;

                        ImplLineInfo();
                        ImplLineInfo( const ImplLineInfo& rImplLineInfo );

    bool operator==( const ImplLineInfo& ) const;

    friend SvStream&    operator>>( SvStream& rIStm, ImplLineInfo& rImplLineInfo );
    friend SvStream&    operator<<( SvStream& rOStm, const ImplLineInfo& rImplLineInfo );
};

// ------------
// - LineInfo -
// ------------

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
    sal_Bool            operator==( const LineInfo& rLineInfo ) const;
    sal_Bool            operator!=( const LineInfo& rLineInfo ) const { return !(LineInfo::operator==( rLineInfo ) ); }
    sal_Bool            IsSameInstance( const LineInfo& rLineInfo ) const { return( mpImplLineInfo == rLineInfo.mpImplLineInfo ); }

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

    sal_Bool            IsDefault() const { return( !mpImplLineInfo->mnWidth && ( LINE_SOLID == mpImplLineInfo->meStyle ) ); }

    friend VCL_DLLPUBLIC SvStream& operator>>( SvStream& rIStm, LineInfo& rLineInfo );
    friend VCL_DLLPUBLIC SvStream& operator<<( SvStream& rOStm, const LineInfo& rLineInfo );

    // helper to get decomposed polygon data with the LineInfo applied. The source
    // hairline polygon is given in io_rLinePolyPolygon. Both given polygons may
    // contain results; e.g. when no fat line but DasDot is defined, the resut will
    // be in io_rLinePolyPolygon while o_rFillPolyPolygon will be empty. When fat line
    // is defined, it will be vice-versa. If none is defined, io_rLinePolyPolygon will
    // not be changed (but o_rFillPolyPolygon will be freed)
    void applyToB2DPolyPolygon(
        basegfx::B2DPolyPolygon& io_rLinePolyPolygon,
        basegfx::B2DPolyPolygon& o_rFillPolyPolygon) const;
};

#endif  // _SV_LINEINFO_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
