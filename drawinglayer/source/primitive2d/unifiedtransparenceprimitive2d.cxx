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

#include <drawinglayer/primitive2d/unifiedtransparenceprimitive2d.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/color/bcolor.hxx>
#include <drawinglayer/primitive2d/polypolygonprimitive2d.hxx>
#include <drawinglayer/primitive2d/transparenceprimitive2d.hxx>
#include <drawinglayer/primitive2d/drawinglayer_primitivetypes2d.hxx>
#include <drawinglayer/primitive2d/polygonprimitive2d.hxx>

//////////////////////////////////////////////////////////////////////////////

using namespace com::sun::star;

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive2d
    {
        UnifiedTransparencePrimitive2D::UnifiedTransparencePrimitive2D(
            const Primitive2DSequence& rChildren,
            double fTransparence)
        :   GroupPrimitive2D(rChildren),
            mfTransparence(fTransparence)
        {
        }

        bool UnifiedTransparencePrimitive2D::operator==(const BasePrimitive2D& rPrimitive) const
        {
            if(GroupPrimitive2D::operator==(rPrimitive))
            {
                const UnifiedTransparencePrimitive2D& rCompare = (UnifiedTransparencePrimitive2D&)rPrimitive;

                return (getTransparence() == rCompare.getTransparence());
            }

            return false;
        }

        basegfx::B2DRange UnifiedTransparencePrimitive2D::getB2DRange(const geometry::ViewInformation2D& rViewInformation) const
        {
            // do not use the fallback to decomposition here since for a correct BoundRect we also
            // need invisible (1.0 == getTransparence()) geometry; these would be deleted in the decomposition
            return getB2DRangeFromPrimitive2DSequence(getChildren(), rViewInformation);
        }

        Primitive2DSequence UnifiedTransparencePrimitive2D::get2DDecomposition(const geometry::ViewInformation2D& rViewInformation) const
        {
            if(0.0 == getTransparence())
            {
                // no transparence used, so just use the content
                return getChildren();
            }
            else if(getTransparence() > 0.0 && getTransparence() < 1.0)
            {
                // The idea is to create a TransparencePrimitive2D with transparent content using a fill color
                // corresponding to the transparence value. Problem is that in most systems, the right
                // and bottom pixel array is not filled when filling polygons, thus this would not
                // always produce a complete transparent bitmap. There are some solutions:
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
                // used when UnifiedTransparencePrimitive2D is not handled directly.
                const basegfx::B2DRange aPolygonRange(getB2DRangeFromPrimitive2DSequence(getChildren(), rViewInformation));
                const basegfx::B2DPolygon aPolygon(basegfx::tools::createPolygonFromRect(aPolygonRange));
                const basegfx::BColor aGray(getTransparence(), getTransparence(), getTransparence());
                Primitive2DSequence aTransparenceContent(2);

                aTransparenceContent[0] = Primitive2DReference(new PolyPolygonColorPrimitive2D(basegfx::B2DPolyPolygon(aPolygon), aGray));
                aTransparenceContent[1] = Primitive2DReference(new PolygonHairlinePrimitive2D(aPolygon, aGray));

                // create sub-transparence group with a gray-colored rectangular fill polygon
                const Primitive2DReference xRefB(new TransparencePrimitive2D(getChildren(), aTransparenceContent));
                return Primitive2DSequence(&xRefB, 1L);
            }
            else
            {
                // completely transparent or invalid definition, add nothing
                return Primitive2DSequence();
            }
        }

        // provide unique ID
        ImplPrimitrive2DIDBlock(UnifiedTransparencePrimitive2D, PRIMITIVE2D_ID_UNIFIEDTRANSPARENCEPRIMITIVE2D)

    } // end of namespace primitive2d
} // end of namespace drawinglayer

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
