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



#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_GROUPPRIMITIVE2D_HXX
#define INCLUDED_DRAWINGLAYER_PRIMITIVE2D_GROUPPRIMITIVE2D_HXX

#include <drawinglayer/drawinglayerdllapi.h>
#include <drawinglayer/primitive2d/baseprimitive2d.hxx>

//////////////////////////////////////////////////////////////////////////////
// GroupPrimitive2D class

namespace drawinglayer
{
    namespace primitive2d
    {
        /** GroupPrimitive2D class

            Baseclass for all grouping 2D primitives

            The grouping primitive in it's basic form is capable of holding
            a child primitive content and returns it on decomposition on default.
            It is used for two main purposes, but more may apply:

            - to transport extended information, e.g. for text classification,
              see e.g. TextHierarchy*Primitive2D implementations. Since they
              decompose to their child content, renderers not aware/interested
              in that extra information will just ignore these primitives

            - to encapsulate common geometry, e.g. the ShadowPrimitive2D implements
              applying a generic shadow to a child sequence by adding the needed
              offset and color stuff in the decomposition

            In most cases the decomposition is straightforward, so by default
            this primitive will not buffer the result and is not derived from
            BufferedDecompositionPrimitive2D, but from BasePrimitive2D.

            A renderer has to take GroupPrimitive2D derivations into account which
            are used to hold a state.

            Current Basic 2D StatePrimitives are:

            - TransparencePrimitive2D (objects with freely defined transparence)
            - InvertPrimitive2D (for XOR)
            - MaskPrimitive2D (for masking)
            - ModifiedColorPrimitive2D (for a stack of color modifications)
            - TransformPrimitive2D (for a transformation stack)
         */
        class DRAWINGLAYER_DLLPUBLIC GroupPrimitive2D : public BasePrimitive2D
        {
        private:
            /// the children. Declared private since this shall never be changed at all after construction
            Primitive2DSequence                             maChildren;

        public:
            /// constructor
            GroupPrimitive2D(const Primitive2DSequence& rChildren);

            /// data read access
            const Primitive2DSequence& getChildren() const { return maChildren; }

            /// local decomposition. Implementation will just return children
            virtual Primitive2DSequence get2DDecomposition(const geometry::ViewInformation2D& rViewInformation) const;

            /// provide unique ID
            DeclPrimitrive2DIDBlock()
        };
    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

#endif //INCLUDED_DRAWINGLAYER_PRIMITIVE2D_GROUPPRIMITIVE2D_HXX

//////////////////////////////////////////////////////////////////////////////
// eof
