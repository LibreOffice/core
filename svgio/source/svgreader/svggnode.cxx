/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <svgio/svgreader/svggnode.hxx>
#include <drawinglayer/primitive2d/transformprimitive2d.hxx>
#include <drawinglayer/primitive2d/unifiedtransparenceprimitive2d.hxx>

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
            mpaTransform(nullptr)
        {
            OSL_ENSURE(aType == SVGTokenDefs || aType == SVGTokenG, "SvgGNode should ony be used for Group and Defs (!)");
        }

        SvgGNode::~SvgGNode()
        {
            delete mpaTransform;
        }

        const SvgStyleAttributes* SvgGNode::getSvgStyleAttributes() const
        {
            if (SVGTokenDefs == getType())
            {
                // #i125258# call parent for SVGTokenDefs
                return SvgNode::getSvgStyleAttributes();
            }
            else
            {
                // #i125258# for SVGTokenG take CssStyles into account
                return checkForCssStyle("g", maSvgStyleAttributes);
            }
        }

        void SvgGNode::parseAttribute(const OUString& rTokenName, SVGToken aSVGToken, const OUString& aContent)
        {
            // call parent
            SvgNode::parseAttribute(rTokenName, aSVGToken, aContent);

            // read style attributes
            maSvgStyleAttributes.parseStyleAttribute(rTokenName, aSVGToken, aContent, false);

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

        void SvgGNode::decomposeSvgNode(drawinglayer::primitive2d::Primitive2DContainer& rTarget, bool bReferenced) const
        {
            if(SVGTokenDefs == getType())
            {
                // #i125258# no decompose needed for defs element, call parent for SVGTokenDefs
                SvgNode::decomposeSvgNode(rTarget, bReferenced);
            }
            else
            {
                // #i125258# for SVGTokenG decompose children
                const SvgStyleAttributes* pStyle = getSvgStyleAttributes();

                if(pStyle)
                {
                    const double fOpacity(pStyle->getOpacity().getNumber());

                    if(fOpacity > 0.0 && Display_none != getDisplay())
                    {
                        drawinglayer::primitive2d::Primitive2DContainer aContent;

                        // decompose children
                        SvgNode::decomposeSvgNode(aContent, bReferenced);

                        if(!aContent.empty())
                        {
                            pStyle->add_postProcess(rTarget, aContent, getTransform());
                        }
                    }
                }
            }
        }
    } // end of namespace svgreader
} // end of namespace svgio

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
