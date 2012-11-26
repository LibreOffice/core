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



#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_BORDERLINEPRIMITIVE2D_HXX
#define INCLUDED_DRAWINGLAYER_PRIMITIVE2D_BORDERLINEPRIMITIVE2D_HXX

#include <drawinglayer/drawinglayerdllapi.h>
#include <drawinglayer/primitive2d/baseprimitive2d.hxx>
#include <basegfx/color/bcolor.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive2d
    {
        /** BorderLinePrimitive2D class

            This is the basic primitive to build frames around objects, e.g. tables.
            It defines a single or double line from Start to nd using the LeftWidth,
            Distance and RightWidth definitions.
            The LineStart/End overlap is defined by the Extend(Inner|Outer)(Start|End)
            definitions.
         */
        class DRAWINGLAYER_DLLPUBLIC BorderLinePrimitive2D : public BufferedDecompositionPrimitive2D
        {
        private:
            /// the line definition
            basegfx::B2DPoint                               maStart;
            basegfx::B2DPoint                               maEnd;

            /// the widths of single/double line
            double                                          mfLeftWidth;
            double                                          mfDistance;
            double                                          mfRightWidth;

            /// edge overlap sizes
            double                                          mfExtendInnerStart;
            double                                          mfExtendInnerEnd;
            double                                          mfExtendOuterStart;
            double                                          mfExtendOuterEnd;

            /// the line color
            basegfx::BColor                                 maRGBColor;

            /// bitfield
            /// flags to influence inside/outside creation
            bool                                            mbCreateInside : 1;
            bool                                            mbCreateOutside : 1;

            /// local helpers
            double getCorrectedLeftWidth() const
            {
                return basegfx::fTools::equal(1.0, mfLeftWidth) ? 0.0 : mfLeftWidth;
            }

            double getCorrectedDistance() const
            {
                return basegfx::fTools::equal(1.0, mfDistance) ? 0.0 : mfDistance;
            }

            double getCorrectedRightWidth() const
            {
                return basegfx::fTools::equal(1.0, mfRightWidth) ? 0.0 : mfRightWidth;
            }

            double getWidth() const
            {
                return getCorrectedLeftWidth() + getCorrectedDistance() + getCorrectedRightWidth();
            }

            bool leftIsHairline() const
            {
                return basegfx::fTools::equal(1.0, mfLeftWidth);
            }

            bool rightIsHairline() const
            {
                return basegfx::fTools::equal(1.0, mfRightWidth);
            }

            bool isInsideUsed() const
            {
                return !basegfx::fTools::equalZero(mfLeftWidth);
            }

            bool isOutsideUsed() const
            {
                return !basegfx::fTools::equalZero(mfRightWidth);
            }

        protected:
            /// create local decomposition
            virtual Primitive2DSequence create2DDecomposition(const geometry::ViewInformation2D& rViewInformation) const;

        public:
            /// constructor
            BorderLinePrimitive2D(
                const basegfx::B2DPoint& rStart,
                const basegfx::B2DPoint& rEnd,
                double fLeftWidth,
                double fDistance,
                double fRightWidth,
                double fExtendInnerStart,
                double fExtendInnerEnd,
                double fExtendOuterStart,
                double fExtendOuterEnd,
                bool bCreateInside,
                bool bCreateOutside,
                const basegfx::BColor& rRGBColor);

            /// data read access
            const basegfx::B2DPoint& getStart() const { return maStart; }
            const basegfx::B2DPoint& getEnd() const { return maEnd; }
            double getLeftWidth() const { return mfLeftWidth; }
            double getDistance() const { return mfDistance; }
            double getRightWidth() const { return mfRightWidth; }
            double getExtendInnerStart() const { return mfExtendInnerStart; }
            double getExtendInnerEnd() const { return mfExtendInnerEnd; }
            double getExtendOuterStart() const { return mfExtendOuterStart; }
            double getExtendOuterEnd() const { return mfExtendOuterEnd; }
            bool getCreateInside() const { return mbCreateInside; }
            bool getCreateOutside() const { return mbCreateOutside; }
            const basegfx::BColor& getRGBColor() const { return maRGBColor; }

            /// provide unique ID
            DeclPrimitrive2DIDBlock()
        };
    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

#endif //INCLUDED_DRAWINGLAYER_PRIMITIVE2D_BORDERLINEPRIMITIVE2D_HXX

//////////////////////////////////////////////////////////////////////////////
// eof
