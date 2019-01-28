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

#include <svganode.hxx>

namespace svgio
{
    namespace svgreader
    {
        SvgANode::SvgANode(
            SvgDocument& rDocument,
            SvgNode* pParent)
        :   SvgNode(SVGTokenA, rDocument, pParent),
            maSvgStyleAttributes(*this)
        {
        }

        SvgANode::~SvgANode()
        {
        }

        const SvgStyleAttributes* SvgANode::getSvgStyleAttributes() const
        {
            return checkForCssStyle("a", maSvgStyleAttributes);
        }

        void SvgANode::parseAttribute(const OUString& rTokenName, SVGToken aSVGToken, const OUString& aContent)
        {
            // call parent
            SvgNode::parseAttribute(rTokenName, aSVGToken, aContent);

            // read style attributes
            maSvgStyleAttributes.parseStyleAttribute(aSVGToken, aContent, false);

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
                case SVGTokenXlinkHref:
                    //TODO: add support for xlink:href
                    break;
                default:
                {
                    break;
                }
            }
        }

        void SvgANode::decomposeSvgNode(drawinglayer::primitive2d::Primitive2DContainer& rTarget, bool bReferenced) const
        {
            // #i125258# for SVGTokenA decompose children
            const SvgStyleAttributes* pStyle = getSvgStyleAttributes();

            if(!pStyle)
                return;

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
    } // end of namespace svgreader
} // end of namespace svgio

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
