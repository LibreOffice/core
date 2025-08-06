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

#include <drawinglayer/primitive2d/markerarrayprimitive2d.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <drawinglayer/geometry/viewinformation2d.hxx>
#include <drawinglayer/primitive2d/drawinglayer_primitivetypes2d.hxx>
#include <drawinglayer/primitive2d/bitmapprimitive2d.hxx>
#include <drawinglayer/primitive2d/groupprimitive2d.hxx>
#include <toolkit/helper/vclunohelper.hxx>


using namespace com::sun::star;


namespace drawinglayer::primitive2d
{
        Primitive2DReference MarkerArrayPrimitive2D::create2DDecomposition(const geometry::ViewInformation2D& rViewInformation) const
        {
            const std::vector< basegfx::B2DPoint >& rPositions = getPositions();
            const sal_uInt32 nMarkerCount(rPositions.size());

            if(!nMarkerCount || getMarker().IsEmpty())
                return nullptr;

            // get pixel size
            Size aBitmapSize(getMarker().GetSizePixel());

            if(!(aBitmapSize.Width() && aBitmapSize.Height()))
                return nullptr;

            // get logic half pixel size
            basegfx::B2DVector aLogicHalfSize(rViewInformation.getInverseObjectToViewTransformation() *
                basegfx::B2DVector(aBitmapSize.getWidth() - 1.0, aBitmapSize.getHeight() - 1.0));

            // use half size for expand
            aLogicHalfSize *= 0.5;

            Primitive2DContainer aContainer;
            for(const auto& rPosition : rPositions)
            {
                const basegfx::B2DRange aRange(rPosition - aLogicHalfSize, rPosition + aLogicHalfSize);
                basegfx::B2DHomMatrix aTransform;

                aTransform.set(0, 0, aRange.getWidth());
                aTransform.set(1, 1, aRange.getHeight());
                aTransform.set(0, 2, aRange.getMinX());
                aTransform.set(1, 2, aRange.getMinY());

                aContainer.push_back(
                    new BitmapPrimitive2D(
                        BitmapEx(getMarker()),
                        aTransform));
            }
            return new GroupPrimitive2D(std::move(aContainer));
        }

        MarkerArrayPrimitive2D::MarkerArrayPrimitive2D(
            std::vector< basegfx::B2DPoint >&& rPositions,
            const Bitmap& rMarker)
        :   maPositions(std::move(rPositions)),
            maMarker(rMarker)
        {
        }

        bool MarkerArrayPrimitive2D::operator==(const BasePrimitive2D& rPrimitive) const
        {
            if(BufferedDecompositionPrimitive2D::operator==(rPrimitive))
            {
                const MarkerArrayPrimitive2D& rCompare = static_cast<const MarkerArrayPrimitive2D&>(rPrimitive);

                return (getPositions() == rCompare.getPositions()
                    && getMarker() == rCompare.getMarker());
            }

            return false;
        }

        basegfx::B2DRange MarkerArrayPrimitive2D::getB2DRange(const geometry::ViewInformation2D& rViewInformation) const
        {
            basegfx::B2DRange aRetval;

            if(!getPositions().empty())
            {
                // get the basic range from the position vector
                for (auto const& pos : getPositions())
                {
                    aRetval.expand(pos);
                }

                if(!getMarker().IsEmpty())
                {
                    // get pixel size
                    const Size aBitmapSize(getMarker().GetSizePixel());

                    if(aBitmapSize.Width() && aBitmapSize.Height())
                    {
                        // get logic half size
                        basegfx::B2DVector aLogicHalfSize(rViewInformation.getInverseObjectToViewTransformation() *
                            basegfx::B2DVector(aBitmapSize.getWidth(), aBitmapSize.getHeight()));

                        // use half size for expand
                        aLogicHalfSize *= 0.5;

                        // apply aLogicHalfSize
                        aRetval.expand(aRetval.getMinimum() - aLogicHalfSize);
                        aRetval.expand(aRetval.getMaximum() + aLogicHalfSize);
                    }
                }
            }

            return aRetval;
        }

        // provide unique ID
        sal_uInt32 MarkerArrayPrimitive2D::getPrimitive2DID() const
        {
            return PRIMITIVE2D_ID_MARKERARRAYPRIMITIVE2D;
        }

} // end of namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
