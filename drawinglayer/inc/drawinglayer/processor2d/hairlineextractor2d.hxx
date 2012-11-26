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

#ifndef INCLUDED_DRAWINGLAYER_PROCESSOR2D_HAIRLINEEXTRACTOR2D_HXX
#define INCLUDED_DRAWINGLAYER_PROCESSOR2D_HAIRLINEEXTRACTOR2D_HXX

#include <drawinglayer/processor2d/baseprocessor2d.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace processor2d
    {
        /** HairlineGeometryExtractor2D class

            A processor who extracts the hairlines, even of invisible
            geometries. Text is ignored. This is used for calculating the
            former SnapRect of a SdrObject from it's primitive representation
         */
        class DRAWINGLAYER_DLLPUBLIC HairlineGeometryExtractor2D : public BaseProcessor2D
        {
        private:
            std::vector< basegfx::B2DPolyPolygon >  maExtractedHairlines;

            /// tooling methods
            void processBasePrimitive2D(
                const primitive2d::BasePrimitive2D& rCandidate,
                const primitive2d::Primitive2DReference& rUnoCandidate);
            void addB2DPolygon(const basegfx::B2DPolygon& rPolygon);
            void addB2DPolyPolygon(const basegfx::B2DPolyPolygon& rPolyPolygon);
            void addB2DTransform(const basegfx::B2DHomMatrix& rTransform);

        public:
            HairlineGeometryExtractor2D(const geometry::ViewInformation2D& rViewInformation);
            virtual ~HairlineGeometryExtractor2D();

            const std::vector< basegfx::B2DPolyPolygon >& getExtractedHairlines() const { return maExtractedHairlines; }
        };
    } // end of namespace processor2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

#endif // INCLUDED_DRAWINGLAYER_PROCESSOR2D_HAIRLINEEXTRACTOR2D_HXX

// eof
