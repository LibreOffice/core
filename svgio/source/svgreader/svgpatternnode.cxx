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

#include <svgio/svgreader/svgpatternnode.hxx>
#include <svgio/svgreader/svgdocument.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace svgio
{
    namespace svgreader
    {
        void SvgPatternNode::tryToFindLink()
        {
            if(!mpXLink && !maXLink.isEmpty())
            {
                mpXLink = dynamic_cast< const SvgPatternNode* >(getDocument().findSvgNodeById(maXLink));
            }
        }

        SvgPatternNode::SvgPatternNode(
            SvgDocument& rDocument,
            SvgNode* pParent)
        :   SvgNode(SVGTokenPattern, rDocument, pParent),
            aPrimitives(),
            maSvgStyleAttributes(*this),
            mpViewBox(0),
            maSvgAspectRatio(),
            maX(),
            maY(),
            maWidth(),
            maHeight(),
            mpPatternUnits(0),
            mpPatternContentUnits(0),
            mpaPatternTransform(0),
            maXLink(),
            mpXLink(0)
        {
        }

        SvgPatternNode::~SvgPatternNode()
        {
            if(mpViewBox) delete mpViewBox;
            if(mpaPatternTransform) delete mpaPatternTransform;
            if(mpPatternUnits) delete mpPatternUnits;
            if(mpPatternContentUnits) delete mpPatternContentUnits;
        }

        const SvgStyleAttributes* SvgPatternNode::getSvgStyleAttributes() const
        {
            static rtl::OUString aClassStr(rtl::OUString::createFromAscii("pattern"));
            return checkForCssStyle(aClassStr, maSvgStyleAttributes);
        }

        void SvgPatternNode::parseAttribute(const OUString& rTokenName, SVGToken aSVGToken, const OUString& aContent)
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
                    break;
                }
                case SVGTokenPatternUnits:
                {
                    if(!aContent.isEmpty())
                    {
                        if(aContent.match(commonStrings::aStrUserSpaceOnUse, 0))
                        {
                            setPatternUnits(userSpaceOnUse);
                        }
                        else if(aContent.match(commonStrings::aStrObjectBoundingBox, 0))
                        {
                            setPatternUnits(objectBoundingBox);
                        }
                    }
                    break;
                }
                case SVGTokenPatternContentUnits:
                {
                    if(!aContent.isEmpty())
                    {
                        if(aContent.match(commonStrings::aStrUserSpaceOnUse, 0))
                        {
                            setPatternContentUnits(userSpaceOnUse);
                        }
                        else if(aContent.match(commonStrings::aStrObjectBoundingBox, 0))
                        {
                            setPatternContentUnits(objectBoundingBox);
                        }
                    }
                    break;
                }
                case SVGTokenPatternTransform:
                {
                    const basegfx::B2DHomMatrix aMatrix(readTransform(aContent, *this));

                    if(!aMatrix.isIdentity())
                    {
                        setPatternTransform(&aMatrix);
                    }
                    break;
                }
                case SVGTokenXlinkHref:
                {
                    const sal_Int32 nLen(aContent.getLength());

                    if(nLen && sal_Unicode('#') == aContent[0])
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

        void SvgPatternNode::getValuesRelative(double& rfX, double& rfY, double& rfW, double& rfH, const basegfx::B2DRange& rGeoRange, SvgNode& rUser) const
        {
            double fTargetWidth(rGeoRange.getWidth());
            double fTargetHeight(rGeoRange.getHeight());

            if(fTargetWidth > 0.0 && fTargetHeight > 0.0)
            {
                const SvgUnits aPatternUnits(getPatternUnits() ? *getPatternUnits() : objectBoundingBox);

                if(objectBoundingBox == aPatternUnits)
                {
                    rfW = (getWidth().isSet()) ? getWidth().getNumber() : 0.0;
                    rfH = (getHeight().isSet()) ? getHeight().getNumber() : 0.0;

                    if(Unit_percent == getWidth().getUnit())
                    {
                        rfW *= 0.01;
                    }

                    if(Unit_percent == getHeight().getUnit())
                    {
                        rfH *= 0.01;
                    }
                }
                else
                {
                    rfW = (getWidth().isSet()) ? getWidth().solve(rUser, xcoordinate) : 0.0;
                    rfH = (getHeight().isSet()) ? getHeight().solve(rUser, ycoordinate) : 0.0;

                    // make relative to rGeoRange
                    rfW /= fTargetWidth;
                    rfH /= fTargetHeight;
                }

                if(rfW > 0.0 && rfH > 0.0)
                {
                    if(objectBoundingBox == aPatternUnits)
                    {
                        rfX = (getX().isSet()) ? getX().getNumber() : 0.0;
                        rfY = (getY().isSet()) ? getY().getNumber() : 0.0;

                        if(Unit_percent == getX().getUnit())
                        {
                            rfX *= 0.01;
                        }

                        if(Unit_percent == getY().getUnit())
                        {
                            rfY *= 0.01;
                        }
                    }
                    else
                    {
                        rfX = (getX().isSet()) ? getX().solve(rUser, xcoordinate) : 0.0;
                        rfY = (getY().isSet()) ? getY().solve(rUser, ycoordinate) : 0.0;

                        // make relative to rGeoRange
                        rfX = (rfX - rGeoRange.getMinX()) / fTargetWidth;
                        rfY = (rfY - rGeoRange.getMinY()) / fTargetHeight;
                    }
                }
            }
        }

        const drawinglayer::primitive2d::Primitive2DSequence& SvgPatternNode::getPatternPrimitives() const
        {
            if(!aPrimitives.hasElements() && Display_none != getDisplay())
            {
                decomposeSvgNode(const_cast< SvgPatternNode* >(this)->aPrimitives, true);
            }

            if(!aPrimitives.hasElements() && !maXLink.isEmpty())
            {
                const_cast< SvgPatternNode* >(this)->tryToFindLink();

                if(mpXLink)
                {
                    return mpXLink->getPatternPrimitives();
                }
            }

            return aPrimitives;
        }

        const basegfx::B2DRange SvgPatternNode::getCurrentViewPort() const
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

        const basegfx::B2DRange* SvgPatternNode::getViewBox() const
        {
            if(mpViewBox)
            {
                return mpViewBox;
            }

            const_cast< SvgPatternNode* >(this)->tryToFindLink();

            if(mpXLink)
            {
                return mpXLink->getViewBox();
            }

            return 0;
        }

        const SvgAspectRatio& SvgPatternNode::getSvgAspectRatio() const
        {
            if(maSvgAspectRatio.isSet())
            {
                return maSvgAspectRatio;
            }

            const_cast< SvgPatternNode* >(this)->tryToFindLink();

            if(mpXLink)
            {
                return mpXLink->getSvgAspectRatio();
            }

            return maSvgAspectRatio;
        }

        const SvgNumber& SvgPatternNode::getX() const
        {
            if(maX.isSet())
            {
                return maX;
            }

            const_cast< SvgPatternNode* >(this)->tryToFindLink();

            if(mpXLink)
            {
                return mpXLink->getX();
            }

            return maX;
        }

        const SvgNumber& SvgPatternNode::getY() const
        {
            if(maY.isSet())
            {
                return maY;
            }

            const_cast< SvgPatternNode* >(this)->tryToFindLink();

            if(mpXLink)
            {
                return mpXLink->getY();
            }

            return maY;
        }

        const SvgNumber& SvgPatternNode::getWidth() const
        {
            if(maWidth.isSet())
            {
                return maWidth;
            }

            const_cast< SvgPatternNode* >(this)->tryToFindLink();

            if(mpXLink)
            {
                return mpXLink->getWidth();
            }

            return maWidth;
        }

        const SvgNumber& SvgPatternNode::getHeight() const
        {
            if(maHeight.isSet())
            {
                return maHeight;
            }

            const_cast< SvgPatternNode* >(this)->tryToFindLink();

            if(mpXLink)
            {
                return mpXLink->getHeight();
            }

            return maHeight;
        }

        const SvgUnits* SvgPatternNode::getPatternUnits() const
        {
            if(mpPatternUnits)
            {
                return mpPatternUnits;
            }

            const_cast< SvgPatternNode* >(this)->tryToFindLink();

            if(mpXLink)
            {
                return mpXLink->getPatternUnits();
            }

            return 0;
        }

        const SvgUnits* SvgPatternNode::getPatternContentUnits() const
        {
            if(mpPatternContentUnits)
            {
                return mpPatternContentUnits;
            }

            const_cast< SvgPatternNode* >(this)->tryToFindLink();

            if(mpXLink)
            {
                return mpXLink->getPatternContentUnits();
            }

            return 0;
        }

        const basegfx::B2DHomMatrix* SvgPatternNode::getPatternTransform() const
        {
            if(mpaPatternTransform)
            {
                return mpaPatternTransform;
            }

            const_cast< SvgPatternNode* >(this)->tryToFindLink();

            if(mpXLink)
            {
                return mpXLink->getPatternTransform();
            }

            return 0;
        }

    } // end of namespace svgreader
} // end of namespace svgio

//////////////////////////////////////////////////////////////////////////////
// eof

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
