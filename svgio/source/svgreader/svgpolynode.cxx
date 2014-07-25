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

#include <svgio/svgreader/svgpolynode.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace svgio
{
    namespace svgreader
    {
        SvgPolyNode::SvgPolyNode(
            SvgDocument& rDocument,
            SvgNode* pParent,
            bool bIsPolyline)
        :   SvgNode(SVGTokenPolygon, rDocument, pParent),
            maSvgStyleAttributes(*this),
            mpPolygon(0),
            mpaTransform(0),
            mbIsPolyline(bIsPolyline)
        {
        }

        SvgPolyNode::~SvgPolyNode()
        {
            if(mpaTransform) delete mpaTransform;
            if(mpPolygon) delete mpPolygon;
        }

        const SvgStyleAttributes* SvgPolyNode::getSvgStyleAttributes() const
        {
            static rtl::OUString aClassStrA(rtl::OUString::createFromAscii("polygon"));
            static rtl::OUString aClassStrB(rtl::OUString::createFromAscii("polyline"));

            return checkForCssStyle(mbIsPolyline? aClassStrB : aClassStrA, maSvgStyleAttributes);
        }

        void SvgPolyNode::parseAttribute(const rtl::OUString& rTokenName, SVGToken aSVGToken, const rtl::OUString& aContent)
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
                case SVGTokenPoints:
                {
                    basegfx::B2DPolygon aPath;

                    if(basegfx::tools::importFromSvgPoints(aPath, aContent))
                    {
                        if(aPath.count())
                        {
                            if(!isPolyline())
                            {
                                aPath.setClosed(true);
                            }

                            setPolygon(&aPath);
                        }
                    }
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

        void SvgPolyNode::decomposeSvgNode(drawinglayer::primitive2d::Primitive2DSequence& rTarget, bool /*bReferenced*/) const
        {
            const SvgStyleAttributes* pStyle = getSvgStyleAttributes();

            if(pStyle && getPolygon())
            {
                drawinglayer::primitive2d::Primitive2DSequence aNewTarget;

                pStyle->add_path(basegfx::B2DPolyPolygon(*getPolygon()), aNewTarget, 0);

                if(aNewTarget.hasElements())
                {
                    pStyle->add_postProcess(rTarget, aNewTarget, getTransform());
                }
            }
        }
    } // end of namespace svgreader
} // end of namespace svgio

//////////////////////////////////////////////////////////////////////////////
// eof
