/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: implncvt.hxx,v $
 * $Revision: 1.5 $
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

#ifndef _SV_LINECONV_HXX
#define _SV_LINECONV_HXX

#include <tools/poly.hxx>
#include <vcl/lineinfo.hxx>

// --------------------
// - ImplLineConverter
// --------------------

struct ImplFloatPoint;

class ImplLineConverter
{
    BOOL            mbClosed;
    BOOL            mbRefPoint;
    INT32           mnRefDistance;

    double          mfWidthHalf;
    LineInfo        maLineInfo;

    double          mfDashDotLenght;
    double          mfDistanceLenght;

    UINT32          mnDashCount;
    UINT32          mnDotCount;

    Polygon         maPolygon;
    UINT32          mnFloat0Points;
    ImplFloatPoint* mpFloat0;
    UINT32          mnFloat1Points;
    ImplFloatPoint* mpFloat1;

    UINT32          mnLinesAvailable;
    UINT32          mnLines;

    ImplFloatPoint* mpFloatPoint;

    public:

                    ImplLineConverter( const Polygon& rPoly, const LineInfo& rLineInfo, const Point* pRefPoint );
                    ~ImplLineConverter();

    const Polygon*  ImplGetFirst();
    const Polygon*  ImplGetNext();
};

#endif
