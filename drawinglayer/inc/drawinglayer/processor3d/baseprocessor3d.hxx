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



#ifndef INCLUDED_DRAWINGLAYER_PROCESSOR3D_BASEPROCESSOR3D_HXX
#define INCLUDED_DRAWINGLAYER_PROCESSOR3D_BASEPROCESSOR3D_HXX

#include <drawinglayer/drawinglayerdllapi.h>
#include <drawinglayer/primitive3d/baseprimitive3d.hxx>
#include <drawinglayer/geometry/viewinformation3d.hxx>
#include <boost/utility.hpp>

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace processor3d
    {
        /** BaseProcessor3D class

            Baseclass for all C++ implementations of instances which process
            primitives. Is is derived from boost::noncopyable to not copy it by
            accident.

            Please have a look at baseprocessor2d.hxx for more comments.
         */
        class DRAWINGLAYER_DLLPUBLIC BaseProcessor3D
        :   private boost::noncopyable
        {
        private:
            geometry::ViewInformation3D                     maViewInformation3D;

        protected:
            void updateViewInformation(const geometry::ViewInformation3D& rViewInformation3D)
            {
                maViewInformation3D = rViewInformation3D;
            }

            /*  as tooling, the process() implementation takes over API handling and calls this
                virtual render method when the primitive implementation is BasePrimitive3D-based.
                Default implementation does nothing
             */
            virtual void processBasePrimitive3D(const primitive3d::BasePrimitive3D& rCandidate);

        public:
            BaseProcessor3D(const geometry::ViewInformation3D& rViewInformation);
            virtual ~BaseProcessor3D();

            // the central processing method
            virtual void process(const primitive3d::Primitive3DSequence& rSource);

            // data access
            const geometry::ViewInformation3D& getViewInformation3D() const { return maViewInformation3D; }
        };
    } // end of namespace processor3d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace processor3d
    {
        /** CollectingProcessor3D class

            A processor which just collects all primitives given to it in
            process(..) calls to maPrimitive3DSequence. This can e.g. be used to
            hand around as instance over various methods where every called
            method can add graphic content to it.
         */
        class DRAWINGLAYER_DLLPUBLIC CollectingProcessor3D : public BaseProcessor3D
        {
        private:
            primitive3d::Primitive3DSequence                        maPrimitive3DSequence;

        public:
            CollectingProcessor3D(const geometry::ViewInformation3D& rViewInformation);
            virtual ~CollectingProcessor3D();

            /// the central processing method
            virtual void process(const primitive3d::Primitive3DSequence& rSource);

            /// helpers for adding to local sequence
            void appendPrimitive3DReference(const primitive3d::Primitive3DReference& rSource)
            {
                primitive3d::appendPrimitive3DReferenceToPrimitive3DSequence(maPrimitive3DSequence, rSource);
            }

            /// data access and reset
            const primitive3d::Primitive3DSequence& getPrimitive3DSequence() const { return maPrimitive3DSequence; }
            void reset() { maPrimitive3DSequence = primitive3d::Primitive3DSequence(); }
        };
    } // end of namespace processor3d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

#endif //_DRAWINGLAYER_PROCESSOR3D_BASEPROCESSOR3D_HXX

// eof
