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

#include <svggradientnode.hxx>
#include <svgdocument.hxx>
#include <svggradientstopnode.hxx>
#include <osl/diagnose.h>

namespace svgio::svgreader
{
        void SvgGradientNode::tryToFindLink()
        {
            if(!mpXLink && !maXLink.isEmpty())
            {
                mpXLink = dynamic_cast< const SvgGradientNode* >(getDocument().findSvgNodeById(maXLink));
            }
        }

        SvgGradientNode::SvgGradientNode(
            SVGToken aType,
            SvgDocument& rDocument,
            SvgNode* pParent)
        :   SvgNode(aType, rDocument, pParent),
            maSvgStyleAttributes(*this),
            maGradientUnits(SvgUnits::objectBoundingBox),
            maSpreadMethod(drawinglayer::primitive2d::SpreadMethod::Pad),
            mbResolvingLink(false),
            mpXLink(nullptr)
        {
            OSL_ENSURE(aType == SVGToken::LinearGradient || aType == SVGToken::RadialGradient, "SvgGradientNode should only be used for Linear and Radial gradient (!)");
        }

        SvgGradientNode::~SvgGradientNode()
        {
            // do NOT delete mpXLink, it's only referenced, not owned
        }

        const SvgStyleAttributes* SvgGradientNode::getSvgStyleAttributes() const
        {
            return checkForCssStyle(
                SVGToken::LinearGradient == getType() ? OUString("linearGradient") : OUString("radialGradient"),
                maSvgStyleAttributes);
        }

        void SvgGradientNode::parseAttribute(const OUString& rTokenName, SVGToken aSVGToken, const OUString& aContent)
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
                case SVGToken::X1:
                {
                    SvgNumber aNum;

                    if(readSingleNumber(aContent, aNum))
                    {
                        maX1 = aNum;
                    }
                    break;
                }
                case SVGToken::Y1:
                {
                    SvgNumber aNum;

                    if(readSingleNumber(aContent, aNum))
                    {
                        maY1 = aNum;
                    }
                    break;
                }
                case SVGToken::X2:
                {
                    SvgNumber aNum;

                    if(readSingleNumber(aContent, aNum))
                    {
                        maX2 = aNum;
                    }
                    break;
                }
                case SVGToken::Y2:
                {
                    SvgNumber aNum;

                    if(readSingleNumber(aContent, aNum))
                    {
                        maY2 = aNum;
                    }
                    break;
                }
                case SVGToken::Cx:
                {
                    SvgNumber aNum;

                    if(readSingleNumber(aContent, aNum))
                    {
                        maCx = aNum;
                    }
                    break;
                }
                case SVGToken::Cy:
                {
                    SvgNumber aNum;

                    if(readSingleNumber(aContent, aNum))
                    {
                        maCy = aNum;
                    }
                    break;
                }
                case SVGToken::Fx:
                {
                    SvgNumber aNum;

                    if(readSingleNumber(aContent, aNum))
                    {
                        maFx = aNum;
                    }
                    break;
                }
                case SVGToken::Fy:
                {
                    SvgNumber aNum;

                    if(readSingleNumber(aContent, aNum))
                    {
                        maFy = aNum;
                    }
                    break;
                }
                case SVGToken::R:
                {
                    SvgNumber aNum;

                    if(readSingleNumber(aContent, aNum))
                    {
                        if(aNum.isPositive())
                        {
                            maR = aNum;
                        }
                    }
                    break;
                }
                case SVGToken::GradientUnits:
                {
                    if(!aContent.isEmpty())
                    {
                        if(aContent.match(commonStrings::aStrUserSpaceOnUse))
                        {
                            setGradientUnits(SvgUnits::userSpaceOnUse);
                        }
                        else if(aContent.match(commonStrings::aStrObjectBoundingBox))
                        {
                            setGradientUnits(SvgUnits::objectBoundingBox);
                        }
                    }
                    break;
                }
                case SVGToken::SpreadMethod:
                {
                    if(!aContent.isEmpty())
                    {
                        if(aContent.startsWith("pad"))
                        {
                            setSpreadMethod(drawinglayer::primitive2d::SpreadMethod::Pad);
                        }
                        else if(aContent.startsWith("reflect"))
                        {
                            setSpreadMethod(drawinglayer::primitive2d::SpreadMethod::Reflect);
                        }
                        else if(aContent.startsWith("repeat"))
                        {
                            setSpreadMethod(drawinglayer::primitive2d::SpreadMethod::Repeat);
                        }
                    }
                    break;
                }
                case SVGToken::GradientTransform:
                {
                    const basegfx::B2DHomMatrix aMatrix(readTransform(aContent, *this));

                    if(!aMatrix.isIdentity())
                    {
                        setGradientTransform(aMatrix);
                    }
                    break;
                }
                case SVGToken::XlinkHref:
                {
                    const sal_Int32 nLen(aContent.getLength());

                    if(nLen && '#' == aContent[0])
                    {
                        maXLink = aContent.copy(1);
                        tryToFindLink();
                    }
                    break;
                }
                default:
                {
                    break;
                }
            }
        }

        void SvgGradientNode::collectGradientEntries(drawinglayer::primitive2d::SvgGradientEntryVector& aVector) const
        {
            if(getChildren().empty())
            {
                const_cast< SvgGradientNode* >(this)->tryToFindLink();

                if (mpXLink && !mbResolvingLink)
                {
                    mbResolvingLink = true;
                    mpXLink->collectGradientEntries(aVector);
                    mbResolvingLink = false;
                }
            }
            else
            {
                const sal_uInt32 nCount(getChildren().size());

                for(sal_uInt32 a(0); a < nCount; a++)
                {
                    const SvgGradientStopNode* pCandidate = dynamic_cast< const SvgGradientStopNode* >(getChildren()[a].get());

                    if(pCandidate)
                    {
                        const SvgStyleAttributes* pStyle = pCandidate->getSvgStyleAttributes();

                        if(pStyle)
                        {
                            const SvgNumber aOffset(pCandidate->getOffset());
                            double fOffset(0.0);

                            if(SvgUnit::percent == aOffset.getUnit())
                            {
                                // percent is not relative to distances in ColorStop context, solve locally
                                fOffset = aOffset.getNumber() * 0.01;
                            }
                            else
                            {
                                fOffset = aOffset.solve(*this);
                            }

                            if(fOffset < 0.0)
                            {
                                OSL_ENSURE(false, "OOps, SvgGradientStopNode with offset out of range (!)");
                                fOffset = 0.0;
                            }
                            else if(fOffset > 1.0)
                            {
                                OSL_ENSURE(false, "OOps, SvgGradientStopNode with offset out of range (!)");
                                fOffset = 1.0;
                            }

                            aVector.emplace_back(
                                    fOffset,
                                    pStyle->getStopColor(),
                                    pStyle->getStopOpacity().solve(*this));
                        }
                        else
                        {
                            OSL_ENSURE(false, "OOps, SvgGradientStopNode without Style (!)");
                        }
                    }
                }
            }
        }

        SvgNumber SvgGradientNode::getX1() const
        {
            if(maX1.isSet())
            {
                return maX1;
            }

            const_cast< SvgGradientNode* >(this)->tryToFindLink();

            if (mpXLink && !mbResolvingLink)
            {
                mbResolvingLink = true;
                auto ret = mpXLink->getX1();
                mbResolvingLink = false;
                return ret;
            }

            // default is 0%
            return SvgNumber(0.0, SvgUnit::percent);
        }

        SvgNumber SvgGradientNode::getY1() const
        {
            if(maY1.isSet())
            {
                return maY1;
            }

            const_cast< SvgGradientNode* >(this)->tryToFindLink();

            if (mpXLink && !mbResolvingLink)
            {
                mbResolvingLink = true;
                auto ret = mpXLink->getY1();
                mbResolvingLink = false;
                return ret;
            }

            // default is 0%
            return SvgNumber(0.0, SvgUnit::percent);
        }

        SvgNumber SvgGradientNode::getX2() const
        {
            if(maX2.isSet())
            {
                return maX2;
            }

            const_cast< SvgGradientNode* >(this)->tryToFindLink();

            if (mpXLink && !mbResolvingLink)
            {
                mbResolvingLink = true;
                auto ret = mpXLink->getX2();
                mbResolvingLink = false;
                return ret;
            }

            // default is 100%
            return SvgNumber(100.0, SvgUnit::percent);
        }

        SvgNumber SvgGradientNode::getY2() const
        {
            if(maY2.isSet())
            {
                return maY2;
            }

            const_cast< SvgGradientNode* >(this)->tryToFindLink();

            if (mpXLink && !mbResolvingLink)
            {
                mbResolvingLink = true;
                auto ret = mpXLink->getY2();
                mbResolvingLink = false;
                return ret;
            }

            // default is 0%
            return SvgNumber(0.0, SvgUnit::percent);
        }

        SvgNumber SvgGradientNode::getCx() const
        {
            if(maCx.isSet())
            {
                return maCx;
            }

            const_cast< SvgGradientNode* >(this)->tryToFindLink();

            if (mpXLink && !mbResolvingLink)
            {
                mbResolvingLink = true;
                auto ret = mpXLink->getCx();
                mbResolvingLink = false;
                return ret;
            }

            // default is 50%
            return SvgNumber(50.0, SvgUnit::percent);
        }

        SvgNumber SvgGradientNode::getCy() const
        {
            if(maCy.isSet())
            {
                return maCy;
            }

            const_cast< SvgGradientNode* >(this)->tryToFindLink();

            if (mpXLink && !mbResolvingLink)
            {
                mbResolvingLink = true;
                auto ret = mpXLink->getCy();
                mbResolvingLink = false;
                return ret;
            }

            // default is 50%
            return SvgNumber(50.0, SvgUnit::percent);
        }

        SvgNumber SvgGradientNode::getR() const
        {
            if(maR.isSet())
            {
                return maR;
            }

            const_cast< SvgGradientNode* >(this)->tryToFindLink();

            if (mpXLink && !mbResolvingLink)
            {
                mbResolvingLink = true;
                auto ret = mpXLink->getR();
                mbResolvingLink = false;
                return ret;
            }

            // default is 50%
            return SvgNumber(50.0, SvgUnit::percent);
        }

        const SvgNumber* SvgGradientNode::getFx() const
        {
            if(maFx.isSet())
            {
                return &maFx;
            }

            const_cast< SvgGradientNode* >(this)->tryToFindLink();

            if (mpXLink && !mbResolvingLink)
            {
                mbResolvingLink = true;
                auto ret = mpXLink->getFx();
                mbResolvingLink = false;
                return ret;
            }

            return nullptr;
        }

        const SvgNumber* SvgGradientNode::getFy() const
        {
            if(maFy.isSet())
            {
                return &maFy;
            }

            const_cast< SvgGradientNode* >(this)->tryToFindLink();

            if (mpXLink && !mbResolvingLink)
            {
                mbResolvingLink = true;
                auto ret = mpXLink->getFy();
                mbResolvingLink = false;
                return ret;
            }

            return nullptr;
        }

        std::optional<basegfx::B2DHomMatrix> SvgGradientNode::getGradientTransform() const
        {
            if(mpaGradientTransform)
            {
                return mpaGradientTransform;
            }

            const_cast< SvgGradientNode* >(this)->tryToFindLink();

            if (mpXLink && !mbResolvingLink)
            {
                mbResolvingLink = true;
                auto ret = mpXLink->getGradientTransform();
                mbResolvingLink = false;
                return ret;
            }

            return std::nullopt;
        }

} // end of namespace svgio::svgreader

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
