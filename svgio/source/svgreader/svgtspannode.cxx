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

#include <svgtspannode.hxx>
#include <o3tl/string_view.hxx>

namespace svgio::svgreader
{
        SvgTspanNode::SvgTspanNode(
            SVGToken aType,
            SvgDocument& rDocument,
            SvgNode* pParent)
        :   SvgNode(aType, rDocument, pParent),
            maSvgStyleAttributes(*this),
            mbLengthAdjust(true),
            mnTextLineWidth(0.0)
        {
        }

        SvgTspanNode::~SvgTspanNode()
        {
        }

        const SvgStyleAttributes* SvgTspanNode::getSvgStyleAttributes() const
        {
            // #i125293# Need to support CssStyles in tspan text sections
            return checkForCssStyle(maSvgStyleAttributes);
        }

        void SvgTspanNode::parseAttribute(SVGToken aSVGToken, const OUString& aContent)
        {
            // call parent
            SvgNode::parseAttribute(aSVGToken, aContent);

            // read style attributes
            maSvgStyleAttributes.parseStyleAttribute(aSVGToken, aContent);

            // parse own
            switch(aSVGToken)
            {
                case SVGToken::Style:
                {
                    readLocalCssStyle(aContent);
                    break;
                }
                case SVGToken::X:
                {
                    SvgNumberVector aVector;

                    if(readSvgNumberVector(aContent, aVector))
                    {
                        setX(std::move(aVector));
                    }
                    break;
                }
                case SVGToken::Y:
                {
                    SvgNumberVector aVector;

                    if(readSvgNumberVector(aContent, aVector))
                    {
                        setY(std::move(aVector));
                    }
                    break;
                }
                case SVGToken::Dx:
                {
                    SvgNumberVector aVector;

                    if(readSvgNumberVector(aContent, aVector))
                    {
                        setDx(std::move(aVector));
                    }
                    break;
                }
                case SVGToken::Dy:
                {
                    SvgNumberVector aVector;

                    if(readSvgNumberVector(aContent, aVector))
                    {
                        setDy(std::move(aVector));
                    }
                    break;
                }
                case SVGToken::Rotate:
                {
                    SvgNumberVector aVector;

                    if(readSvgNumberVector(aContent, aVector))
                    {
                        setRotate(std::move(aVector));
                    }
                    break;
                }
                case SVGToken::TextLength:
                {
                    SvgNumber aNum;

                    if(readSingleNumber(aContent, aNum))
                    {
                        if(aNum.isPositive())
                        {
                            setTextLength(aNum);
                        }
                    }
                    break;
                }
                case SVGToken::LengthAdjust:
                {
                    if(o3tl::equalsIgnoreAsciiCase(o3tl::trim(aContent), u"spacing"))
                    {
                        setLengthAdjust(true);
                    }
                    else if(o3tl::equalsIgnoreAsciiCase(o3tl::trim(aContent), u"spacingAndGlyphs"))
                    {
                        setLengthAdjust(false);
                    }
                    break;
                }
                default:
                {
                    break;
                }
            }
        }

} // end of namespace svgio::svgreader

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
