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

#include <svgio/svgreader/svgmarkernode.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace svgio
{
    namespace svgreader
    {
        SvgMarkerNode::SvgMarkerNode(
            SvgDocument& rDocument,
            SvgNode* pParent)
        :   SvgNode(SVGTokenMarker, rDocument, pParent),
            aPrimitives(),
            maSvgStyleAttributes(*this),
            mpViewBox(0),
            maSvgAspectRatio(),
            maRefX(0),
            maRefY(0),
            maMarkerUnits(strokeWidth),
            maMarkerWidth(3),
            maMarkerHeight(3),
            mfAngle(0.0),
            mbOrientAuto(false)
        {
        }

        SvgMarkerNode::~SvgMarkerNode()
        {
            if(mpViewBox) delete mpViewBox;
        }

        const SvgStyleAttributes* SvgMarkerNode::getSvgStyleAttributes() const
        {
            static rtl::OUString aClassStr(rtl::OUString::createFromAscii("marker"));
            return checkForCssStyle(aClassStr, maSvgStyleAttributes);
        }

        void SvgMarkerNode::parseAttribute(const OUString& rTokenName, SVGToken aSVGToken, const OUString& aContent)
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
                case SVGTokenViewBox:
                {
                    const basegfx::B2DRange aRange(readViewBox(aContent, *this));

                    if(!aRange.isEmpty())
                    {
                        setViewBox(&aRange);
                    }
                    break;
                }
                case SVGTokenPreserveAspectRatio:
                {
                    setSvgAspectRatio(readSvgAspectRatio(aContent));
                    break;
                }
                case SVGTokenRefX:
                {
                    SvgNumber aNum;

                    if(readSingleNumber(aContent, aNum))
                    {
                        setRefX(aNum);
                    }
                    break;
                }
                case SVGTokenRefY:
                {
                    SvgNumber aNum;

                    if(readSingleNumber(aContent, aNum))
                    {
                        setRefY(aNum);
                    }
                    break;
                }
                case SVGTokenMarkerUnits:
                {
                    if(!aContent.isEmpty())
                    {
                        static OUString aStrStrokeWidth(OUString::createFromAscii("strokeWidth"));

                        if(aContent.match(aStrStrokeWidth, 0))
                        {
                            setMarkerUnits(strokeWidth);
                        }
                        else if(aContent.match(commonStrings::aStrUserSpaceOnUse, 0))
                        {
                            setMarkerUnits(userSpaceOnUse);
                        }
                    }
                    break;
                }
                case SVGTokenMarkerWidth:
                {
                    SvgNumber aNum;

                    if(readSingleNumber(aContent, aNum))
                    {
                        if(aNum.isPositive())
                        {
                            setMarkerWidth(aNum);
                        }
                    }
                    break;
                }
                case SVGTokenMarkerHeight:
                {
                    SvgNumber aNum;

                    if(readSingleNumber(aContent, aNum))
                    {
                        if(aNum.isPositive())
                        {
                            setMarkerHeight(aNum);
                        }
                    }
                    break;
                }
                case SVGTokenOrient:
                {
                    const sal_Int32 nLen(aContent.getLength());

                    if(nLen)
                    {
                        static OUString aStrAuto(OUString::createFromAscii("auto"));

                        if(aContent.match(aStrAuto, 0))
                        {
                            setOrientAuto(true);
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

        const drawinglayer::primitive2d::Primitive2DSequence& SvgMarkerNode::getMarkerPrimitives() const
        {
            if(!aPrimitives.hasElements() && Display_none != getDisplay())
            {
                decomposeSvgNode(const_cast< SvgMarkerNode* >(this)->aPrimitives, true);
            }

            return aPrimitives;
        }

        const basegfx::B2DRange SvgMarkerNode::getCurrentViewPort() const
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

    } // end of namespace svgreader
} // end of namespace svgio

//////////////////////////////////////////////////////////////////////////////
// eof

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
