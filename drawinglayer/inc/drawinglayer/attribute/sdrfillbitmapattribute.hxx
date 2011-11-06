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



#ifndef INCLUDED_DRAWINGLAYER_ATTRIBUTE_SDRFILLBITMAPATTRIBUTE_HXX
#define INCLUDED_DRAWINGLAYER_ATTRIBUTE_SDRFILLBITMAPATTRIBUTE_HXX

#include <drawinglayer/drawinglayerdllapi.h>
//////////////////////////////////////////////////////////////////////////////
// predefines

class Bitmap;

namespace basegfx {
    class B2DRange;
    class B2DVector;
}

namespace drawinglayer { namespace attribute {
    class FillBitmapAttribute;
    class ImpSdrFillBitmapAttribute;
}}

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace attribute
    {
        class DRAWINGLAYER_DLLPUBLIC SdrFillBitmapAttribute
        {
        private:
            ImpSdrFillBitmapAttribute*          mpSdrFillBitmapAttribute;

        public:
            /// constructors/assignmentoperator/destructor
            SdrFillBitmapAttribute(
                const Bitmap& rBitmap,
                const basegfx::B2DVector& rSize,
                const basegfx::B2DVector& rOffset,
                const basegfx::B2DVector& rOffsetPosition,
                const basegfx::B2DVector& rRectPoint,
                bool bTiling,
                bool bStretch,
                bool bLogSize);
            SdrFillBitmapAttribute();
            SdrFillBitmapAttribute(const SdrFillBitmapAttribute& rCandidate);
            SdrFillBitmapAttribute& operator=(const SdrFillBitmapAttribute& rCandidate);
            ~SdrFillBitmapAttribute();

            // checks if the incarnation is default constructed
            bool isDefault() const;

            // compare operator
            bool operator==(const SdrFillBitmapAttribute& rCandidate) const;

            // data read access
            const Bitmap& getBitmap() const;
            const basegfx::B2DVector& getSize() const;
            const basegfx::B2DVector& getOffset() const;
            const basegfx::B2DVector& getOffsetPosition() const;
            const basegfx::B2DVector& getRectPoint() const;
            bool getTiling() const;
            bool getStretch() const;
            bool getLogSize() const;

            // FillBitmapAttribute generator
            FillBitmapAttribute getFillBitmapAttribute(const basegfx::B2DRange& rRange) const;
        };
    } // end of namespace attribute
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

#endif //INCLUDED_DRAWINGLAYER_ATTRIBUTE_SDRFILLBITMAPATTRIBUTE_HXX

// eof
