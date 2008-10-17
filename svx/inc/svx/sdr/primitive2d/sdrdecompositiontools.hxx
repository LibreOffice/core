/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: sdrdecompositiontools.hxx,v $
 *
 * $Revision: 1.2.18.1 $
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

#ifndef INCLUDED_SDR_PRIMITIVE2D_SDRDECOMPOSITIONTOOLS_HXX
#define INCLUDED_SDR_PRIMITIVE2D_SDRDECOMPOSITIONTOOLS_HXX

#include <drawinglayer/primitive2d/baseprimitive2d.hxx>

//////////////////////////////////////////////////////////////////////////////
// predefines
namespace basegfx {
    class B2DPolygon;
    class B2DPolyPolygon;
    class B2DHomMatrix;
}

namespace drawinglayer { namespace attribute {
    class SdrFillAttribute;
    class SdrLineAttribute;
    class FillGradientAttribute;
    class SdrShadowAttribute;
    class SdrLineStartEndAttribute;
    class SdrTextAttribute;
}}

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive2d
    {
        Primitive2DReference createPolyPolygonFillPrimitive(
            const ::basegfx::B2DPolyPolygon& rUnitPolyPolygon,
            const ::basegfx::B2DHomMatrix& rObjectTransform,
            const attribute::SdrFillAttribute& rFill,
            const attribute::FillGradientAttribute* pFillGradient = 0L);

        Primitive2DReference createPolygonLinePrimitive(
            const ::basegfx::B2DPolygon& rUnitPolygon,
            const ::basegfx::B2DHomMatrix& rObjectTransform,
            const attribute::SdrLineAttribute& rLine,
            const attribute::SdrLineStartEndAttribute* pStroke = 0L);

        Primitive2DReference createTextPrimitive(
            const ::basegfx::B2DPolyPolygon& rUnitPolyPolygon,
            const ::basegfx::B2DHomMatrix& rObjectTransform,
            const attribute::SdrTextAttribute& rText,
            const attribute::SdrLineAttribute* pStroke,
            bool bCellText,
            bool bWordWrap);

        Primitive2DReference createShadowPrimitive(
            const Primitive2DSequence& rSource,
            const attribute::SdrShadowAttribute& rShadow);

    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

#endif //INCLUDED_SDR_PRIMITIVE2D_SDRDECOMPOSITIONTOOLS_HXX

// eof
