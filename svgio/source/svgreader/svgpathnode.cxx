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

#include <svgio/svgreader/svgpathnode.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace svgio
{
    namespace svgreader
    {
        SvgPathNode::SvgPathNode(
            SvgDocument& rDocument,
            SvgNode* pParent)
        :   SvgNode(SVGTokenPath, rDocument, pParent),
            maSvgStyleAttributes(*this),
            mpPolyPolygon(0),
            mpaTransform(0),
            maPathLength()
        {
        }

        SvgPathNode::~SvgPathNode()
        {
            if(mpPolyPolygon) delete mpPolyPolygon;
            if(mpaTransform) delete mpaTransform;
        }

        const SvgStyleAttributes* SvgPathNode::getSvgStyleAttributes() const
        {
            static rtl::OUString aClassStr(rtl::OUString::createFromAscii("path"));

            return checkForCssStyle(aClassStr, maSvgStyleAttributes);
        }

        void SvgPathNode::parseAttribute(const rtl::OUString& rTokenName, SVGToken aSVGToken, const rtl::OUString& aContent)
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
                case SVGTokenD:
                {
                    basegfx::B2DPolyPolygon aPath;

                    if(basegfx::tools::importFromSvgD(aPath, aContent, false, &maHelpPointIndices))
                    {
                        if(aPath.count())
                        {
                            setPath(&aPath);
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
                case SVGTokenPathLength:
                {
                    SvgNumber aNum;

                    if(readSingleNumber(aContent, aNum))
                    {
                        setPathLength(aNum);
                    }
                    break;
                }
                default:
                {
                    break;
                }
            }
        }

        void SvgPathNode::decomposeSvgNode(drawinglayer::primitive2d::Primitive2DSequence& rTarget, bool /*bReferenced*/) const
        {
            // fill and/or stroke needed, also a path
            const SvgStyleAttributes* pStyle = getSvgStyleAttributes();

            if(pStyle && getPath())
            {
                drawinglayer::primitive2d::Primitive2DSequence aNewTarget;

                pStyle->add_path(*getPath(), aNewTarget, &maHelpPointIndices);

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
