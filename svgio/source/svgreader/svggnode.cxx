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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svgio.hxx"

#include <svgio/svgreader/svggnode.hxx>
#include <drawinglayer/primitive2d/transformprimitive2d.hxx>
#include <drawinglayer/primitive2d/unifiedtransparenceprimitive2d.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace svgio
{
    namespace svgreader
    {
        SvgGNode::SvgGNode(
            SVGToken aType,
            SvgDocument& rDocument,
            SvgNode* pParent)
        :   SvgNode(aType, rDocument, pParent),
            maSvgStyleAttributes(*this),
            mpaTransform(0)
        {
            OSL_ENSURE(aType == SVGTokenDefs || aType == SVGTokenG, "SvgGNode should ony be used for Group and Defs (!)");
        }

        SvgGNode::~SvgGNode()
        {
            if(mpaTransform) delete mpaTransform;
        }

        const SvgStyleAttributes* SvgGNode::getSvgStyleAttributes() const
        {
            if(SVGTokenDefs == getType())
            {
                // #125258# call parent for SVGTokenDefs
                return SvgNode::getSvgStyleAttributes();
            }
            else
            {
                // #125258# for SVGTokenG take CssStyles into account
                static rtl::OUString aClassStr(rtl::OUString::createFromAscii("g"));

                return checkForCssStyle(aClassStr, maSvgStyleAttributes);
            }
        }

        void SvgGNode::parseAttribute(const rtl::OUString& rTokenName, SVGToken aSVGToken, const rtl::OUString& aContent)
        {
            // call parent
            SvgNode::parseAttribute(rTokenName, aSVGToken, aContent);

            // read style attributes
            maSvgStyleAttributes.parseStyleAttribute(rTokenName, aSVGToken, aContent);

            // parse own
            switch(aSVGToken)
            {
                case SVGTokenStyle:
                {
                    readLocalCssStyle(aContent);
                    break;
                }
                case SVGTokenTransform:
                {
                    const basegfx::B2DHomMatrix aMatrix(readTransform(aContent, *this));

                    if(!aMatrix.isIdentity())
                    {
                        setTransform(&aMatrix);
                    }
                    break;
                }
                default:
                {
                    break;
                }
            }
        }

        void SvgGNode::decomposeSvgNode(drawinglayer::primitive2d::Primitive2DSequence& rTarget, bool bReferenced) const
        {
            if(SVGTokenDefs == getType())
            {
                // #125258# no decompose needed for defs element, call parent for SVGTokenDefs
                SvgNode::decomposeSvgNode(rTarget, bReferenced);
            }
            else
            {
                // #125258# for SVGTokenG decompose childs
                const SvgStyleAttributes* pStyle = getSvgStyleAttributes();

                if(pStyle)
                {
                    const double fOpacity(pStyle->getOpacity().getNumber());

                    if(fOpacity > 0.0 && Display_none != getDisplay())
                    {
                        drawinglayer::primitive2d::Primitive2DSequence aContent;

                        // decompose childs
                        SvgNode::decomposeSvgNode(aContent, bReferenced);

                        if(aContent.hasElements())
                        {
                            pStyle->add_postProcess(rTarget, aContent, getTransform());
                        }
                    }
                }
            }
        }
    } // end of namespace svgreader
} // end of namespace svgio

//////////////////////////////////////////////////////////////////////////////
// eof
