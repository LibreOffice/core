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

#include <drawinglayer/drawinglayerdllapi.h>

#include <drawinglayer/primitive2d/BufferedDecompositionPrimitive2D.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <drawinglayer/attribute/fillgraphicattribute.hxx>
#include <vcl/bitmapex.hxx>

namespace drawinglayer::primitive2d
{
class FillGraphicPrimitive2D;
}

namespace drawinglayer::processor2d
{
// define a simple accessor which can be used as friend. That method exists
// only locally at SDPRProcessor2dTools.cxx and is thus only usable/callable
// from there
void setOffsetXYCreatedBitmap(
    drawinglayer::primitive2d::FillGraphicPrimitive2D&,
    const Bitmap&);
}

// FillbitmapPrimitive2D class
namespace drawinglayer::primitive2d
{
        /** FillGraphicPrimitive2D class

            This class defines a bitmap filling for a rectangular area. The
            Range is defined by the Transformation, the fill by the FillGraphicAttribute.
            There, the fill consists of a Bitmap (not transparent) defining the fill data
            and a Point/Vector pair defining the relative position/size [0.0 .. 1.0]
            inside the area where the bitmap is positioned. A flag defines then if this
            is tiled or not.

            Renderers should handle this primitive; it has a geometrically correct
            decomposition, but on pixel outputs the areas where the tiled pieces are
            aligned tend to show up (one overlapping or empty pixel)

            SDPR: support alpha directly now. If a primitive processor
            cannot deal with it, use it's decomposition. The decomposition
            uses create2DDecompositionOfGraphic, there all paths are now
            capable of handling a given alpha, including metafile, SVG and
            animated graphics
         */
        class DRAWINGLAYER_DLLPUBLIC FillGraphicPrimitive2D final : public BufferedDecompositionPrimitive2D
        {
        private:
            /// the geometric definition
            basegfx::B2DHomMatrix                       maTransformation;

            /// the fill attributes
            attribute::FillGraphicAttribute             maFillGraphic;

            /// the evtl. buffered OffsetXYCreatedBitmap
            Bitmap                                      maOffsetXYCreatedBitmap;

            /// the transparency in range [0.0 .. 1.0]
            double mfTransparency;

            /// local decomposition.
            virtual Primitive2DReference create2DDecomposition(const geometry::ViewInformation2D& rViewInformation) const override;

            // allow this single accessor to change it to set buggered data
            friend void drawinglayer::processor2d::setOffsetXYCreatedBitmap(
                drawinglayer::primitive2d::FillGraphicPrimitive2D&,
                const Bitmap&);

            // private tooling method to be called by setOffsetXYCreatedBitmap
            void impSetOffsetXYCreatedBitmap(const Bitmap& rBitmap)
            {
                maOffsetXYCreatedBitmap = rBitmap;
            }

        public:
            /// constructor
            FillGraphicPrimitive2D(
                basegfx::B2DHomMatrix aTransformation,
                const attribute::FillGraphicAttribute& rFillGraphic,
                double fTransparency = 0.0);

            /// data read access
            const basegfx::B2DHomMatrix& getTransformation() const { return maTransformation; }
            const attribute::FillGraphicAttribute& getFillGraphic() const { return maFillGraphic; }
            const Bitmap& getOffsetXYCreatedBitmap() const { return maOffsetXYCreatedBitmap; }
            double getTransparency() const { return mfTransparency; }
            bool hasTransparency() const { return !basegfx::fTools::equalZero(mfTransparency); }

            /// compare operator
            virtual bool operator==( const BasePrimitive2D& rPrimitive ) const override;

            /// get range
            virtual basegfx::B2DRange getB2DRange(const geometry::ViewInformation2D& rViewInformation) const override;

            /// provide unique ID
            virtual sal_uInt32 getPrimitive2DID() const override;
        };
} // end of namespace drawinglayer::primitive2d


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
