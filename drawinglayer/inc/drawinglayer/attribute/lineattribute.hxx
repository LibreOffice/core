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



#ifndef INCLUDED_DRAWINGLAYER_ATTRIBUTE_LINEATTRIBUTE_HXX
#define INCLUDED_DRAWINGLAYER_ATTRIBUTE_LINEATTRIBUTE_HXX

#include <drawinglayer/drawinglayerdllapi.h>
#include <basegfx/vector/b2enums.hxx>

//////////////////////////////////////////////////////////////////////////////
// predefines

namespace basegfx {
    class BColor;
}

namespace drawinglayer { namespace attribute {
    class ImpLineAttribute;
}}

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace attribute
    {
        class DRAWINGLAYER_DLLPUBLIC LineAttribute
        {
        private:
            ImpLineAttribute*                           mpLineAttribute;

        public:
            /// constructors/assignmentoperator/destructor
            LineAttribute(
                const basegfx::BColor& rColor,
                double fWidth = 0.0,
                basegfx::B2DLineJoin aB2DLineJoin = basegfx::B2DLINEJOIN_ROUND);
            LineAttribute();
            LineAttribute(const LineAttribute& rCandidate);
            LineAttribute& operator=(const LineAttribute& rCandidate);
            ~LineAttribute();

            // checks if the incarnation is default constructed
            bool isDefault() const;

            // compare operator
            bool operator==(const LineAttribute& rCandidate) const;

            // data read access
            const basegfx::BColor& getColor() const;
            double getWidth() const;
            basegfx::B2DLineJoin getLineJoin() const;
        };
    } // end of namespace attribute
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

#endif //INCLUDED_DRAWINGLAYER_ATTRIBUTE_LINEATTRIBUTE_HXX

// eof
