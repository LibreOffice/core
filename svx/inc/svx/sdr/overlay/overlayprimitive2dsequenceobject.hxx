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



#ifndef _SDR_OVERLAY_OVERLAYPRIMITIVE2DSEQUENCEOBJECT_HXX
#define _SDR_OVERLAY_OVERLAYPRIMITIVE2DSEQUENCEOBJECT_HXX

#include <svx/sdr/overlay/overlayobject.hxx>
#include <drawinglayer/primitive2d/baseprimitive2d.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace overlay
    {
        class SVX_DLLPUBLIC OverlayPrimitive2DSequenceObject : public OverlayObjectWithBasePosition
        {
        protected:
            // the sequence of primitives to show
            const drawinglayer::primitive2d::Primitive2DSequence    maSequence;

            // geometry creation for OverlayObject
            virtual drawinglayer::primitive2d::Primitive2DSequence createOverlayObjectPrimitive2DSequence();

        private:
            // internal helper to create a drawinglayer::geometry::ViewInformation2D
            // using a OutputDevice and local knowledge
            drawinglayer::geometry::ViewInformation2D impCreateViewInformation2D(OutputDevice& rOutputDevice) const;

        public:
            OverlayPrimitive2DSequenceObject(const drawinglayer::primitive2d::Primitive2DSequence& rSequence);

            virtual ~OverlayPrimitive2DSequenceObject();

            // data read access
            const drawinglayer::primitive2d::Primitive2DSequence& getSequence() const { return maSequence; }
        };
    } // end of namespace overlay
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////

#endif //_SDR_OVERLAY_OVERLAYPRIMITIVE2DSEQUENCEOBJECT_HXX

// eof
