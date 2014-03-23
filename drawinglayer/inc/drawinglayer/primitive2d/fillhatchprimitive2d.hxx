/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_FILLHATCHPRIMITIVE2D_HXX
#define INCLUDED_DRAWINGLAYER_PRIMITIVE2D_FILLHATCHPRIMITIVE2D_HXX

#include <drawinglayer/drawinglayerdllapi.h>
#include <drawinglayer/primitive2d/primitivetools2d.hxx>
#include <drawinglayer/attribute/fillhatchattribute.hxx>
#include <basegfx/color/bcolor.hxx>

//////////////////////////////////////////////////////////////////////////////
// FillHatchPrimitive2D class

namespace drawinglayer
{
    namespace primitive2d
    {
        /** FillHatchPrimitive2D class

            This class defines a hatch filling for a rectangular area. The
            Range is defined by the Transformation, the hatch by the FillHatchAttribute.
            If the background is to be filled, a flag in FillHatchAttribute is set and
            the BColor defines the background color.

            #120230# This primitive is now evtl. metric dependent due to the value
            MinimalDiscreteDistance in the FillHatchAttribute if the value is not zero.
            This is used for a more appealing, VCL-like visualisation by not letting the
            distances get too small between lines.

            The decomposition will deliver the hatch lines.
         */
        class DRAWINGLAYER_DLLPUBLIC FillHatchPrimitive2D : public DiscreteMetricDependentPrimitive2D
        {
        private:
            /// the geometrically visible area
            basegfx::B2DRange                       maOutputRange;

            /// the area the gradient definition is based on
            /// in the simplest case identical to OutputRange
            basegfx::B2DRange                       maDefinitionRange;

            /// the hatch definition
            attribute::FillHatchAttribute           maFillHatch;

            /// hatch background color (if used)
            basegfx::BColor                         maBColor;

        protected:
            /// local decomposition.
            virtual Primitive2DSequence create2DDecomposition(const geometry::ViewInformation2D& rViewInformation) const;

        public:
            /// constructors. The one without definition range will use output range as definition range
            FillHatchPrimitive2D(
                const basegfx::B2DRange& rOutputRange,
                const basegfx::BColor& rBColor,
                const attribute::FillHatchAttribute& rFillHatch);
            FillHatchPrimitive2D(
                const basegfx::B2DRange& rOutputRange,
                const basegfx::B2DRange& rDefinitionRange,
                const basegfx::BColor& rBColor,
                const attribute::FillHatchAttribute& rFillHatch);

            /// data read access
            const basegfx::B2DRange& getOutputRange() const { return maOutputRange; }
            const basegfx::B2DRange& getDefinitionRange() const { return maDefinitionRange; }
            const attribute::FillHatchAttribute& getFillHatch() const { return maFillHatch; }
            const basegfx::BColor& getBColor() const { return maBColor; }

            /// get range
            virtual basegfx::B2DRange getB2DRange(const geometry::ViewInformation2D& rViewInformation) const;

            /// get local decomposition. Overloaded since this decomposition is view-dependent
            virtual Primitive2DSequence get2DDecomposition(const geometry::ViewInformation2D& rViewInformation) const;

            /// provide unique ID
            DeclPrimitrive2DIDBlock()
        };
    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

#endif //INCLUDED_DRAWINGLAYER_PRIMITIVE2D_FILLHATCHPRIMITIVE2D_HXX

//////////////////////////////////////////////////////////////////////////////
// eof
