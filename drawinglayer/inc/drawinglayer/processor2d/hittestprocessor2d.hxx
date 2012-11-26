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



#ifndef INCLUDED_DRAWINGLAYER_PROCESSOR2D_HITTESTPROCESSOR2D_HXX
#define INCLUDED_DRAWINGLAYER_PROCESSOR2D_HITTESTPROCESSOR2D_HXX

#include <drawinglayer/drawinglayerdllapi.h>
#include <drawinglayer/processor2d/baseprocessor2d.hxx>

//////////////////////////////////////////////////////////////////////////////
// predeclarations

namespace basegfx { class B2DPolygon; }
namespace basegfx { class B2DPolyPolygon; }
namespace drawinglayer { namespace primitive2d { class ScenePrimitive2D; }}

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace processor2d
    {
        /** HitTestProcessor2D class

            This processor implements a HitTest with the feeded primitives,
            given tolerance and extras
         */
        class DRAWINGLAYER_DLLPUBLIC HitTestProcessor2D : public BaseProcessor2D
        {
        private:
            /// discrete HitTest position
            basegfx::B2DPoint           maDiscreteHitPosition;

            /// discrete HitTolerance
            double                      mfDiscreteHitTolerance;

            /// space to keep hit TextHierarchyFieldPrimitive2Ds
            primitive2d::Primitive2DSequence*   mpRecordFields;

            /// bitfield
            bool                        mbHit : 1;
            bool                        mbHitToleranceUsed : 1;

            /*  this flag decides if primitives which are embedded to an
                UnifiedTransparencePrimitive2D and are invisible will be taken into account for
                HitTesting or not. Those primitives are created for objects which are else
                completely invisible and normally their content exists of hairline
                primitives describing the object's contour
             */
            bool                        mbUseInvisiblePrimitiveContent : 1;

            /// flag to concentrate on text hits only
            bool                        mbHitTextOnly : 1;

            /// tooling methods
            void processBasePrimitive2D(
                const primitive2d::BasePrimitive2D& rCandidate,
                const primitive2d::Primitive2DReference& rUnoCandidate);
            bool checkHairlineHitWithTolerance(
                const basegfx::B2DPolygon& rPolygon,
                double fDiscreteHitTolerance);
            bool checkFillHitWithTolerance(
                const basegfx::B2DPolyPolygon& rPolyPolygon,
                double fDiscreteHitTolerance);
            void check3DHit(const primitive2d::ScenePrimitive2D& rCandidate);

        public:
            HitTestProcessor2D(
                const geometry::ViewInformation2D& rViewInformation,
                const basegfx::B2DPoint& rLogicHitPosition,
                double fLogicHitTolerance,
                bool bHitTextOnly,
                primitive2d::Primitive2DSequence* pRecordFields);
            virtual ~HitTestProcessor2D();

            /// data write access
            void setUseInvisiblePrimitiveContent(bool bNew)
            {
                if(mbUseInvisiblePrimitiveContent != bNew) mbUseInvisiblePrimitiveContent = bNew;
            }

            /// data read access
            const basegfx::B2DPoint& getDiscreteHitPosition() const { return maDiscreteHitPosition; }
            double getDiscreteHitTolerance() const { return mfDiscreteHitTolerance; }
            bool getHit() const { return mbHit; }
            bool getHitToleranceUsed() const { return mbHitToleranceUsed; }
            bool getUseInvisiblePrimitiveContent() const { return mbUseInvisiblePrimitiveContent;}
            bool getHitTextOnly() const { return mbHitTextOnly; }
        };
    } // end of namespace processor2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

#endif // INCLUDED_DRAWINGLAYER_PROCESSOR2D_HITTESTPROCESSOR2D_HXX

// eof
