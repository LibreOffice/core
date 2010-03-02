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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_drawinglayer.hxx"

#include <drawinglayer/primitive2d/unifiedalphaprimitive2d.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/color/bcolor.hxx>
#include <drawinglayer/primitive2d/polypolygonprimitive2d.hxx>
#include <drawinglayer/primitive2d/alphaprimitive2d.hxx>
#include <drawinglayer/primitive2d/drawinglayer_primitivetypes2d.hxx>
#include <drawinglayer/primitive2d/polygonprimitive2d.hxx>

//////////////////////////////////////////////////////////////////////////////

using namespace com::sun::star;

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive2d
    {
        UnifiedAlphaPrimitive2D::UnifiedAlphaPrimitive2D(
            const Primitive2DSequence& rChildren,
            double fAlpha)
        :   GroupPrimitive2D(rChildren),
            mfAlpha(fAlpha)
        {
        }

        bool UnifiedAlphaPrimitive2D::operator==(const BasePrimitive2D& rPrimitive) const
        {
            if(GroupPrimitive2D::operator==(rPrimitive))
            {
                const UnifiedAlphaPrimitive2D& rCompare = (UnifiedAlphaPrimitive2D&)rPrimitive;

                return (getAlpha() == rCompare.getAlpha());
            }

            return false;
        }

        Primitive2DSequence UnifiedAlphaPrimitive2D::get2DDecomposition(const geometry::ViewInformation2D& rViewInformation) const
        {
            if(0.0 == getAlpha())
            {
                // no transparence used, so just use the content
                return getChildren();
            }
            else if(getAlpha() > 0.0 && getAlpha() < 1.0)
            {
                // The idea is to create a AlphaPrimitive2D with alpha content using a fill color
                // corresponding to the alpha value. Problem is that in most systems, the right
                // and bottom pixel array is not filled when filling polygons, thus this would not
                // always produce a complete alpha bitmap. There are some solutions:
                //
                // - Grow the used polygon range by one discrete unit in X and Y. This
                // will make the decomposition view-dependent.
                //
                // - For all filled polygon renderings, dra wthe polygon outline extra. This
                // would lead to unwanted side effects when using concatenated polygons.
                //
                // - At this decomposition, add a filled polygon and a hairline polygon. This
                // solution stays view-independent.
                //
                // I will take the last one here. The small overhead of two primitives will only be
                // used when UnifiedAlphaPrimitive2D is not handled directly.
                const basegfx::B2DRange aPolygonRange(getB2DRangeFromPrimitive2DSequence(getChildren(), rViewInformation));
                const basegfx::B2DPolygon aPolygon(basegfx::tools::createPolygonFromRect(aPolygonRange));
                const basegfx::BColor aGray(getAlpha(), getAlpha(), getAlpha());
                Primitive2DSequence aAlphaContent(2);

                aAlphaContent[0] = Primitive2DReference(new PolyPolygonColorPrimitive2D(basegfx::B2DPolyPolygon(aPolygon), aGray));
                aAlphaContent[1] = Primitive2DReference(new PolygonHairlinePrimitive2D(aPolygon, aGray));

                // create sub-transparence group with a gray-colored rectangular fill polygon
                const Primitive2DReference xRefB(new AlphaPrimitive2D(getChildren(), aAlphaContent));
                return Primitive2DSequence(&xRefB, 1L);
            }
            else
            {
                // completely transparent or invalid definition, add nothing
                return Primitive2DSequence();
            }
        }

        // provide unique ID
        ImplPrimitrive2DIDBlock(UnifiedAlphaPrimitive2D, PRIMITIVE2D_ID_UNIFIEDALPHAPRIMITIVE2D)

    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////
// eof
