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

#ifndef INCLUDED_SVGIO_SVGREADER_SVGMASKNODE_HXX
#define INCLUDED_SVGIO_SVGREADER_SVGMASKNODE_HXX

#include <svgio/svgiodllapi.h>
#include <svgio/svgreader/svgnode.hxx>
#include <svgio/svgreader/svgstyleattributes.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace svgio
{
    namespace svgreader
    {
        class SvgMaskNode : public SvgNode
        {
        private:
            /// use styles
            SvgStyleAttributes          maSvgStyleAttributes;

            /// variable scan values, dependent of given XAttributeList
            SvgNumber                   maX;
            SvgNumber                   maY;
            SvgNumber                   maWidth;
            SvgNumber                   maHeight;
            basegfx::B2DHomMatrix*      mpaTransform;
            SvgUnits                    maMaskUnits;
            SvgUnits                    maMaskContentUnits;

        public:
            SvgMaskNode(
                SvgDocument& rDocument,
                SvgNode* pParent);
            virtual ~SvgMaskNode();

            virtual const SvgStyleAttributes* getSvgStyleAttributes() const;
            virtual void parseAttribute(const rtl::OUString& rTokenName, SVGToken aSVGToken, const rtl::OUString& aContent);
            virtual void decomposeSvgNode(drawinglayer::primitive2d::Primitive2DSequence& rTarget, bool bReferenced) const;

            /// apply contained clipPath to given geometry #i124852# transform may be needed
            void apply(
                drawinglayer::primitive2d::Primitive2DSequence& rTarget,
                const basegfx::B2DHomMatrix* pTransform) const;

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

            /// transform content
            const basegfx::B2DHomMatrix* getTransform() const { return mpaTransform; }
            void setTransform(const basegfx::B2DHomMatrix* pMatrix = 0) { if(mpaTransform) delete mpaTransform; mpaTransform = 0; if(pMatrix) mpaTransform = new basegfx::B2DHomMatrix(*pMatrix); }

            /// MaskUnits content
            SvgUnits getMaskUnits() const { return maMaskUnits; }
            void setMaskUnits(const SvgUnits aMaskUnits) { maMaskUnits = aMaskUnits; }

            /// MaskContentUnits content
            SvgUnits getMaskContentUnits() const { return maMaskContentUnits; }
            void setMaskContentUnits(const SvgUnits aMaskContentUnits) { maMaskContentUnits = aMaskContentUnits; }
        };
    } // end of namespace svgreader
} // end of namespace svgio

//////////////////////////////////////////////////////////////////////////////

#endif //INCLUDED_SVGIO_SVGREADER_SVGMASKNODE_HXX

// eof
