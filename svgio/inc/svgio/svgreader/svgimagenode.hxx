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

#ifndef INCLUDED_SVGIO_SVGREADER_SVGIMAGENODE_HXX
#define INCLUDED_SVGIO_SVGREADER_SVGIMAGENODE_HXX

#include <svgio/svgiodllapi.h>
#include <svgio/svgreader/svgnode.hxx>
#include <svgio/svgreader/svgstyleattributes.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace svgio
{
    namespace svgreader
    {
        class SvgImageNode : public SvgNode
        {
        private:
            /// use styles
            SvgStyleAttributes      maSvgStyleAttributes;

            /// variable scan values, dependent of given XAttributeList
            SvgAspectRatio          maSvgAspectRatio;
            basegfx::B2DHomMatrix*  mpaTransform;
            SvgNumber               maX;
            SvgNumber               maY;
            SvgNumber               maWidth;
            SvgNumber               maHeight;

            rtl::OUString           maXLink;        // internal link
            rtl::OUString           maUrl;          // external link

            rtl::OUString           maMimeType;     // mimetype and
            rtl::OUString           maData;         // base64 data

        public:
            SvgImageNode(
                SvgDocument& rDocument,
                SvgNode* pParent);
            virtual ~SvgImageNode();

            virtual const SvgStyleAttributes* getSvgStyleAttributes() const;
            virtual void parseAttribute(const rtl::OUString& rTokenName, SVGToken aSVGToken, const rtl::OUString& aContent);
            virtual void decomposeSvgNode(drawinglayer::primitive2d::Primitive2DSequence& rTarget, bool bReferenced) const;

            /// SvgAspectRatio content
            const SvgAspectRatio& getSvgAspectRatio() const { return maSvgAspectRatio; }
            void setSvgAspectRatio(const SvgAspectRatio& rSvgAspectRatio = SvgAspectRatio()) { maSvgAspectRatio = rSvgAspectRatio; }

            /// transform content, set if found in current context
            const basegfx::B2DHomMatrix* getTransform() const { return mpaTransform; }
            void setTransform(const basegfx::B2DHomMatrix* pMatrix = 0) { if(mpaTransform) delete mpaTransform; mpaTransform = 0; if(pMatrix) mpaTransform = new basegfx::B2DHomMatrix(*pMatrix); }

            /// x content, set if found in current context
            const SvgNumber& getX() const { return maX; }
            void setX(const SvgNumber& rX = SvgNumber()) { maX = rX; }

            /// y content, set if found in current context
            const SvgNumber& getY() const { return maY; }
            void setY(const SvgNumber& rY = SvgNumber()) { maY = rY; }

            /// width content, set if found in current context
            const SvgNumber& getWidth() const { return maWidth; }
            void setWidth(const SvgNumber& rWidth = SvgNumber()) { maWidth = rWidth; }

            /// height content, set if found in current context
            const SvgNumber& getHeight() const { return maHeight; }
            void setHeight(const SvgNumber& rHeight = SvgNumber()) { maHeight = rHeight; }

            /// XLink access
            const rtl::OUString& getXLink() const { return maXLink; }
        };
    } // end of namespace svgreader
} // end of namespace svgio

//////////////////////////////////////////////////////////////////////////////

#endif //INCLUDED_SVGIO_SVGREADER_SVGIMAGENODE_HXX

// eof
