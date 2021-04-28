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

#include <svgmarkernode.hxx>

namespace svgio::svgreader
{
        SvgMarkerNode::SvgMarkerNode(
            SvgDocument& rDocument,
            SvgNode* pParent)
        :   SvgNode(SVGToken::Marker, rDocument, pParent),
            aPrimitives(),
            maSvgStyleAttributes(*this),
            maSvgAspectRatio(),
            maRefX(0),
            maRefY(0),
            maMarkerUnits(MarkerUnits::strokeWidth),
            maMarkerWidth(3),
            maMarkerHeight(3),
            mfAngle(0.0),
            mbOrientAuto(false)
        {
        }

        SvgMarkerNode::~SvgMarkerNode()
        {
        }

        const SvgStyleAttributes* SvgMarkerNode::getSvgStyleAttributes() const
        {
            return checkForCssStyle("marker", maSvgStyleAttributes);
        }

        void SvgMarkerNode::parseAttribute(const OUString& rTokenName, SVGToken aSVGToken, const OUString& aContent)
        {
            // call parent
            SvgNode::parseAttribute(rTokenName, aSVGToken, aContent);

            // read style attributes
            maSvgStyleAttributes.parseStyleAttribute(aSVGToken, aContent, false);

            // parse own
            switch(aSVGToken)
            {
                case SVGToken::Style:
                {
                    readLocalCssStyle(aContent);
                    break;
                }
                case SVGToken::ViewBox:
                {
                    const basegfx::B2DRange aRange(readViewBox(aContent, *this));

                    if(!aRange.isEmpty())
                    {
                        setViewBox(&aRange);
                    }
                    break;
                }
                case SVGToken::PreserveAspectRatio:
                {
                    maSvgAspectRatio = readSvgAspectRatio(aContent);
                    break;
                }
                case SVGToken::RefX:
                {
                    SvgNumber aNum;

                    if(readSingleNumber(aContent, aNum))
                    {
                        maRefX = aNum;
                    }
                    break;
                }
                case SVGToken::RefY:
                {
                    SvgNumber aNum;

                    if(readSingleNumber(aContent, aNum))
                    {
                        maRefY = aNum;
                    }
                    break;
                }
                case SVGToken::MarkerUnits:
                {
                    if(!aContent.isEmpty())
                    {
                        if(aContent.startsWith("strokeWidth"))
                        {
                            setMarkerUnits(MarkerUnits::strokeWidth);
                        }
                        else if(aContent.match(commonStrings::aStrUserSpaceOnUse))
                        {
                            setMarkerUnits(MarkerUnits::userSpaceOnUse);
                        }
                    }
                    break;
                }
                case SVGToken::MarkerWidth:
                {
                    SvgNumber aNum;

                    if(readSingleNumber(aContent, aNum))
                    {
                        if(aNum.isPositive())
                        {
                            maMarkerWidth = aNum;
                        }
                    }
                    break;
                }
                case SVGToken::MarkerHeight:
                {
                    SvgNumber aNum;

                    if(readSingleNumber(aContent, aNum))
                    {
                        if(aNum.isPositive())
                        {
                            maMarkerHeight = aNum;
                        }
                    }
                    break;
                }
                case SVGToken::Orient:
                {
                    const sal_Int32 nLen(aContent.getLength());

                    if(nLen)
                    {
                        if(aContent.startsWith("auto"))
                        {
                            mbOrientAuto = true;
                        }
                        else
                        {
                            sal_Int32 nPos(0);
                            double fAngle(0.0);

                            if(readAngle(aContent, nPos, fAngle, nLen))
                            {
                                setAngle(fAngle);
                            }
                        }
                    }
                    break;
                }
                default:
                {
                    break;
                }
            }
        }

        const drawinglayer::primitive2d::Primitive2DContainer& SvgMarkerNode::getMarkerPrimitives() const
        {
            if(aPrimitives.empty() && Display_none != getDisplay())
            {
                decomposeSvgNode(const_cast< SvgMarkerNode* >(this)->aPrimitives, true);
            }

            return aPrimitives;
        }

        basegfx::B2DRange SvgMarkerNode::getCurrentViewPort() const
        {
            if(getViewBox())
            {
                return *(getViewBox());
            }
            else
            {
                return SvgNode::getCurrentViewPort();
            }
        }

} // end of namespace svgio::svgreader

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
