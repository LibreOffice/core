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

#include <svgio/svgreader/svggradientnode.hxx>
#include <svgio/svgreader/svgdocument.hxx>
#include <svgio/svgreader/svggradientstopnode.hxx>

namespace svgio
{
    namespace svgreader
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
            maX1(),
            maY1(),
            maX2(),
            maY2(),
            maCx(),
            maCy(),
            maR(),
            maFx(),
            maFy(),
            maGradientUnits(objectBoundingBox),
            maSpreadMethod(drawinglayer::primitive2d::SpreadMethod::Pad),
            mpaGradientTransform(nullptr),
            maXLink(),
            mpXLink(nullptr)
        {
            OSL_ENSURE(aType == SVGTokenLinearGradient || aType == SVGTokenRadialGradient, "SvgGradientNode should ony be used for Linear and Radial gradient (!)");
        }

        SvgGradientNode::~SvgGradientNode()
        {
            delete mpaGradientTransform;
            // do NOT delete mpXLink, it's only referenced, not owned
        }

        const SvgStyleAttributes* SvgGradientNode::getSvgStyleAttributes() const
        {
            OUString aClassStrA("linearGradient");
            OUString aClassStrB("radialGradient");

            return checkForCssStyle(
                SVGTokenLinearGradient == getType() ? aClassStrA : aClassStrB,
                maSvgStyleAttributes);
        }

        void SvgGradientNode::parseAttribute(const OUString& rTokenName, SVGToken aSVGToken, const OUString& aContent)
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
                case SVGTokenX1:
                {
                    SvgNumber aNum;

                    if(readSingleNumber(aContent, aNum))
                    {
                        setX1(aNum);
                    }
                    break;
                }
                case SVGTokenY1:
                {
                    SvgNumber aNum;

                    if(readSingleNumber(aContent, aNum))
                    {
                        setY1(aNum);
                    }
                    break;
                }
                case SVGTokenX2:
                {
                    SvgNumber aNum;

                    if(readSingleNumber(aContent, aNum))
                    {
                        setX2(aNum);
                    }
                    break;
                }
                case SVGTokenY2:
                {
                    SvgNumber aNum;

                    if(readSingleNumber(aContent, aNum))
                    {
                        setY2(aNum);
                    }
                    break;
                }
                case SVGTokenCx:
                {
                    SvgNumber aNum;

                    if(readSingleNumber(aContent, aNum))
                    {
                        setCx(aNum);
                    }
                    break;
                }
                case SVGTokenCy:
                {
                    SvgNumber aNum;

                    if(readSingleNumber(aContent, aNum))
                    {
                        setCy(aNum);
                    }
                    break;
                }
                case SVGTokenFx:
                {
                    SvgNumber aNum;

                    if(readSingleNumber(aContent, aNum))
                    {
                        setFx(aNum);
                    }
                    break;
                }
                case SVGTokenFy:
                {
                    SvgNumber aNum;

                    if(readSingleNumber(aContent, aNum))
                    {
                        setFy(aNum);
                    }
                    break;
                }
                case SVGTokenR:
                {
                    SvgNumber aNum;

                    if(readSingleNumber(aContent, aNum))
                    {
                        if(aNum.isPositive())
                        {
                            setR(aNum);
                        }
                    }
                    break;
                }
                case SVGTokenGradientUnits:
                {
                    if(!aContent.isEmpty())
                    {
                        if(aContent.match(commonStrings::aStrUserSpaceOnUse))
                        {
                            setGradientUnits(userSpaceOnUse);
                        }
                        else if(aContent.match(commonStrings::aStrObjectBoundingBox))
                        {
                            setGradientUnits(objectBoundingBox);
                        }
                    }
                    break;
                }
                case SVGTokenSpreadMethod:
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
                case SVGTokenGradientTransform:
                {
                    const basegfx::B2DHomMatrix aMatrix(readTransform(aContent, *this));

                    if(!aMatrix.isIdentity())
                    {
                        setGradientTransform(&aMatrix);
                    }
                    break;
                }
                case SVGTokenXlinkHref:
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

                if(mpXLink)
                {
                    mpXLink->collectGradientEntries(aVector);
                }
            }
            else
            {
                const sal_uInt32 nCount(getChildren().size());

                for(sal_uInt32 a(0); a < nCount; a++)
                {
                    const SvgGradientStopNode* pCandidate = dynamic_cast< const SvgGradientStopNode* >(getChildren()[a]);

                    if(pCandidate)
                    {
                        const SvgStyleAttributes* pStyle = pCandidate->getSvgStyleAttributes();

                        if(pStyle)
                        {
                            const SvgNumber aOffset(pCandidate->getOffset());
                            double fOffset(0.0);

                            if(Unit_percent == aOffset.getUnit())
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

                            aVector.push_back(
                                drawinglayer::primitive2d::SvgGradientEntry(
                                    fOffset,
                                    pStyle->getStopColor(),
                                    pStyle->getStopOpacity().solve(*this)));
                        }
                        else
                        {
                            OSL_ENSURE(false, "OOps, SvgGradientStopNode without Style (!)");
                        }
                    }
                }
            }
        }

        const SvgNumber SvgGradientNode::getX1() const
        {
            if(maX1.isSet())
            {
                return maX1;
            }

            const_cast< SvgGradientNode* >(this)->tryToFindLink();

            if(mpXLink)
            {
                return mpXLink->getX1();
            }

            // default is 0%
            return SvgNumber(0.0, Unit_percent);
        }

        const SvgNumber SvgGradientNode::getY1() const
        {
            if(maY1.isSet())
            {
                return maY1;
            }

            const_cast< SvgGradientNode* >(this)->tryToFindLink();

            if(mpXLink)
            {
                return mpXLink->getY1();
            }

            // default is 0%
            return SvgNumber(0.0, Unit_percent);
        }

        const SvgNumber SvgGradientNode::getX2() const
        {
            if(maX2.isSet())
            {
                return maX2;
            }

            const_cast< SvgGradientNode* >(this)->tryToFindLink();

            if(mpXLink)
            {
                return mpXLink->getX2();
            }

            // default is 100%
            return SvgNumber(100.0, Unit_percent);
        }

        const SvgNumber SvgGradientNode::getY2() const
        {
            if(maY2.isSet())
            {
                return maY2;
            }

            const_cast< SvgGradientNode* >(this)->tryToFindLink();

            if(mpXLink)
            {
                return mpXLink->getY2();
            }

            // default is 0%
            return SvgNumber(0.0, Unit_percent);
        }

        const SvgNumber SvgGradientNode::getCx() const
        {
            if(maCx.isSet())
            {
                return maCx;
            }

            const_cast< SvgGradientNode* >(this)->tryToFindLink();

            if(mpXLink)
            {
                return mpXLink->getCx();
            }

            // default is 50%
            return SvgNumber(50.0, Unit_percent);
        }

        const SvgNumber SvgGradientNode::getCy() const
        {
            if(maCy.isSet())
            {
                return maCy;
            }

            const_cast< SvgGradientNode* >(this)->tryToFindLink();

            if(mpXLink)
            {
                return mpXLink->getCy();
            }

            // default is 50%
            return SvgNumber(50.0, Unit_percent);
        }

        const SvgNumber SvgGradientNode::getR() const
        {
            if(maR.isSet())
            {
                return maR;
            }

            const_cast< SvgGradientNode* >(this)->tryToFindLink();

            if(mpXLink)
            {
                return mpXLink->getR();
            }

            // default is 50%
            return SvgNumber(50.0, Unit_percent);
        }

        const SvgNumber* SvgGradientNode::getFx() const
        {
            if(maFx.isSet())
            {
                return &maFx;
            }

            const_cast< SvgGradientNode* >(this)->tryToFindLink();

            if(mpXLink)
            {
                return mpXLink->getFx();
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

            if(mpXLink)
            {
                return mpXLink->getFy();
            }

            return nullptr;
        }

        const basegfx::B2DHomMatrix* SvgGradientNode::getGradientTransform() const
        {
            if(mpaGradientTransform)
            {
                return mpaGradientTransform;
            }

            const_cast< SvgGradientNode* >(this)->tryToFindLink();

            if(mpXLink)
            {
                return mpXLink->getGradientTransform();
            }

            return nullptr;
        }

        void SvgGradientNode::setGradientTransform(const basegfx::B2DHomMatrix* pMatrix)
        {
            if(mpaGradientTransform)
            {
                delete mpaGradientTransform;
                mpaGradientTransform = nullptr;
            }

            if(pMatrix)
            {
                mpaGradientTransform = new basegfx::B2DHomMatrix(*pMatrix);
            }
        }

    } // end of namespace svgreader
} // end of namespace svgio

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
