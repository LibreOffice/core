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

#include <svgio/svgreader/svgusenode.hxx>
#include <drawinglayer/primitive2d/transformprimitive2d.hxx>
#include <svgio/svgreader/svgdocument.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace svgio
{
    namespace svgreader
    {
        SvgUseNode::SvgUseNode(
            SvgDocument& rDocument,
            SvgNode* pParent)
        :   SvgNode(SVGTokenG, rDocument, pParent),
            maSvgStyleAttributes(*this),
            mpaTransform(0),
            maX(),
            maY(),
            maWidth(),
            maHeight(),
            maXLink()
        {
        }

        SvgUseNode::~SvgUseNode()
        {
            if(mpaTransform) delete mpaTransform;
        }

        const SvgStyleAttributes* SvgUseNode::getSvgStyleAttributes() const
        {
            static rtl::OUString aClassStr(rtl::OUString::createFromAscii("use"));
            return checkForCssStyle(aClassStr, maSvgStyleAttributes);
        }

        void SvgUseNode::parseAttribute(const OUString& rTokenName, SVGToken aSVGToken, const OUString& aContent)
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
                    maSvgStyleAttributes.readStyle(aContent);
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
                case SVGTokenX:
                {
                    SvgNumber aNum;

                    if(readSingleNumber(aContent, aNum))
                    {
                        setX(aNum);
                    }
                    break;
                }
                case SVGTokenY:
                {
                    SvgNumber aNum;

                    if(readSingleNumber(aContent, aNum))
                    {
                        setY(aNum);
                    }
                    break;
                }
                case SVGTokenWidth:
                {
                    SvgNumber aNum;

                    if(readSingleNumber(aContent, aNum))
                    {
                        if(aNum.isPositive())
                        {
                            setWidth(aNum);
                        }
                    }
                    break;
                }
                case SVGTokenHeight:
                {
                    SvgNumber aNum;

                    if(readSingleNumber(aContent, aNum))
                    {
                        if(aNum.isPositive())
                        {
                            setHeight(aNum);
                        }
                    }
                }
                case SVGTokenXlinkHref:
                {
                    const sal_Int32 nLen(aContent.getLength());

                    if(nLen && sal_Unicode('#') == aContent[0])
                    {
                        maXLink = aContent.copy(1);
                    }
                    break;
                }
                default:
                {
                    break;
                }
            }
        }

        void SvgUseNode::decomposeSvgNode(drawinglayer::primitive2d::Primitive2DSequence& rTarget, bool /*bReferenced*/) const
        {
            // try to access link to content
            const SvgNode* mpXLink = getDocument().findSvgNodeById(maXLink);

            if(mpXLink && Display_none != mpXLink->getDisplay())
            {
                // decompose children
                drawinglayer::primitive2d::Primitive2DSequence aNewTarget;

                // todo: in case mpXLink is a SVGTokenSvg or SVGTokenSymbol the
                // SVG docs want the getWidth() and getHeight() from this node
                // to be valid for the subtree.
                const_cast< SvgNode* >(mpXLink)->setAlternativeParent(this);
                mpXLink->decomposeSvgNode(aNewTarget, true);
                const_cast< SvgNode* >(mpXLink)->setAlternativeParent(0);

                if(aNewTarget.hasElements())
                {
                    basegfx::B2DHomMatrix aTransform;

                    if(getX().isSet() || getY().isSet())
                    {
                        aTransform.translate(
                            getX().solve(*this, xcoordinate),
                            getY().solve(*this, ycoordinate));
                    }

                    if(getTransform())
                    {
                        aTransform = *getTransform() * aTransform;
                    }

                    if(!aTransform.isIdentity())
                    {
                        const drawinglayer::primitive2d::Primitive2DReference xRef(
                            new drawinglayer::primitive2d::TransformPrimitive2D(
                                aTransform,
                                aNewTarget));

                        drawinglayer::primitive2d::appendPrimitive2DReferenceToPrimitive2DSequence(rTarget, xRef);
                    }
                    else
                    {
                        drawinglayer::primitive2d::appendPrimitive2DSequenceToPrimitive2DSequence(rTarget, aNewTarget);
                    }
                }
            }
        }

    } // end of namespace svgreader
} // end of namespace svgio

//////////////////////////////////////////////////////////////////////////////
// eof

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
