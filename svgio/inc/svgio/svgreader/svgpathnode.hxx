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

#ifndef INCLUDED_SVGIO_SVGREADER_SVGPATHNODE_HXX
#define INCLUDED_SVGIO_SVGREADER_SVGPATHNODE_HXX

#include <svgio/svgiodllapi.h>
#include <svgio/svgreader/svgnode.hxx>
#include <svgio/svgreader/svgstyleattributes.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace svgio
{
    namespace svgreader
    {
        class SvgPathNode : public SvgNode
        {
        private:
            /// use styles
            SvgStyleAttributes                  maSvgStyleAttributes;

            /// variable scan values, dependent of given XAttributeList
            basegfx::B2DPolyPolygon*            mpPolyPolygon;
            basegfx::B2DHomMatrix*              mpaTransform;
            SvgNumber                           maPathLength;
            basegfx::tools::PointIndexSet       maHelpPointIndices;

        public:
            SvgPathNode(
                SvgDocument& rDocument,
                SvgNode* pParent);
            virtual ~SvgPathNode();

            virtual const SvgStyleAttributes* getSvgStyleAttributes() const;
            virtual void parseAttribute(const rtl::OUString& rTokenName, SVGToken aSVGToken, const rtl::OUString& aContent);
            virtual void decomposeSvgNode(drawinglayer::primitive2d::Primitive2DSequence& rTarget, bool bReferenced) const;

            /// path content, set if found in current context
            const basegfx::B2DPolyPolygon* getPath() const { return mpPolyPolygon; }
            void setPath(const basegfx::B2DPolyPolygon* pPath = 0) { if(mpPolyPolygon) delete mpPolyPolygon; mpPolyPolygon = 0; if(pPath) mpPolyPolygon = new basegfx::B2DPolyPolygon(*pPath); }

            /// transform content, set if found in current context
            const basegfx::B2DHomMatrix* getTransform() const { return mpaTransform; }
            void setTransform(const basegfx::B2DHomMatrix* pMatrix = 0) { if(mpaTransform) delete mpaTransform; mpaTransform = 0; if(pMatrix) mpaTransform = new basegfx::B2DHomMatrix(*pMatrix); }

            /// PathLength content
            const SvgNumber& getPathLength() const { return maPathLength; }
            void setPathLength(const SvgNumber& rPathLength = SvgNumber()) { maPathLength = rPathLength; }
        };
    } // end of namespace svgreader
} // end of namespace svgio

//////////////////////////////////////////////////////////////////////////////

#endif //INCLUDED_SVGIO_SVGREADER_SVGPATHNODE_HXX

// eof
