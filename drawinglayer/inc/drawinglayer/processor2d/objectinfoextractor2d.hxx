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

#ifndef INCLUDED_DRAWINGLAYER_PROCESSOR2D_OBJECTINFOEXTRACTOR2D_HXX
#define INCLUDED_DRAWINGLAYER_PROCESSOR2D_OBJECTINFOEXTRACTOR2D_HXX

#include <drawinglayer/drawinglayerdllapi.h>
#include <drawinglayer/processor2d/baseprocessor2d.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer { namespace primitive2d { class ObjectInfoPrimitive2D; }}

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace processor2d
    {
        /** ObjectInfoPrimitiveExtractor2D class

            A processor to find the first incarnation of ObjectInfoPrimitive2D
            in a given hierarchy
         */
        class DRAWINGLAYER_DLLPUBLIC ObjectInfoPrimitiveExtractor2D : public BaseProcessor2D
        {
        private:
            /// the target
            const primitive2d::ObjectInfoPrimitive2D*      mpFound;

            /// tooling methods
            virtual void processBasePrimitive2D(
                const primitive2d::BasePrimitive2D& rCandidate,
                const primitive2d::Primitive2DReference& rUnoCandidate);

        public:
            ObjectInfoPrimitiveExtractor2D(const geometry::ViewInformation2D& rViewInformation);
            virtual ~ObjectInfoPrimitiveExtractor2D();

            const primitive2d::ObjectInfoPrimitive2D* getResult() const { return mpFound; }
        };
    } // end of namespace processor2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

#endif // INCLUDED_DRAWINGLAYER_PROCESSOR2D_OBJECTINFOEXTRACTOR2D_HXX

// eof
