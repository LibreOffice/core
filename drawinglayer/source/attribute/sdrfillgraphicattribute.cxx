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

#include <sal/config.h>

#include <algorithm>

#include <drawinglayer/attribute/sdrfillgraphicattribute.hxx>
#include <drawinglayer/attribute/fillgraphicattribute.hxx>
#include <drawinglayer/primitive2d/baseprimitive2d.hxx>
#include <vcl/graph.hxx>


namespace drawinglayer::attribute
{
        class ImpSdrFillGraphicAttribute
        {
        public:
            // data definitions
            Graphic                                 maFillGraphic;
            basegfx::B2DVector                      maGraphicLogicSize;
            basegfx::B2DVector                      maSize;
            basegfx::B2DVector                      maOffset;
            basegfx::B2DVector                      maOffsetPosition;
            basegfx::B2DVector                      maRectPoint;

            bool                                    mbTiling : 1;
            bool                                    mbStretch : 1;
            bool                                    mbLogSize : 1;

            ImpSdrFillGraphicAttribute(
                const Graphic& rFillGraphic,
                const basegfx::B2DVector& rGraphicLogicSize,
                const basegfx::B2DVector& rSize,
                const basegfx::B2DVector& rOffset,
                const basegfx::B2DVector& rOffsetPosition,
                const basegfx::B2DVector& rRectPoint,
                bool bTiling,
                bool bStretch,
                bool bLogSize)
            :   maFillGraphic(rFillGraphic),
                maGraphicLogicSize(rGraphicLogicSize),
                maSize(rSize),
                maOffset(rOffset),
                maOffsetPosition(rOffsetPosition),
                maRectPoint(rRectPoint),
                mbTiling(bTiling),
                mbStretch(bStretch),
                mbLogSize(bLogSize)
            {
            }

            ImpSdrFillGraphicAttribute()
            :   mbTiling(false),
                mbStretch(false),
                mbLogSize(false)
            {
            }

            // data read access
            const Graphic& getFillGraphic() const { return maFillGraphic; }
            const basegfx::B2DVector& getGraphicLogicSize() const { return maGraphicLogicSize; }
            const basegfx::B2DVector& getSize() const { return maSize; }
            const basegfx::B2DVector& getOffset() const { return maOffset; }
            const basegfx::B2DVector& getOffsetPosition() const { return maOffsetPosition; }
            const basegfx::B2DVector& getRectPoint() const { return maRectPoint; }
            bool getTiling() const { return mbTiling; }
            bool getStretch() const { return mbStretch; }

            bool operator==(const ImpSdrFillGraphicAttribute& rCandidate) const
            {
                return (getFillGraphic() == rCandidate.getFillGraphic()
                    && getGraphicLogicSize() == rCandidate.getGraphicLogicSize()
                    && getSize() == rCandidate.getSize()
                    && getOffset() == rCandidate.getOffset()
                    && getOffsetPosition() == rCandidate.getOffsetPosition()
                    && getRectPoint() == rCandidate.getRectPoint()
                    && getTiling() == rCandidate.getTiling()
                    && getStretch() == rCandidate.getStretch()
                    && mbLogSize == rCandidate.mbLogSize);
            }
        };

        namespace
        {
            SdrFillGraphicAttribute::ImplType& theGlobalDefault()
            {
                static SdrFillGraphicAttribute::ImplType SINGLETON;
                return SINGLETON;
            }
        }

        SdrFillGraphicAttribute::SdrFillGraphicAttribute(
            const Graphic& rFillGraphic,
            const basegfx::B2DVector& rGraphicLogicSize,
            const basegfx::B2DVector& rSize,
            const basegfx::B2DVector& rOffset,
            const basegfx::B2DVector& rOffsetPosition,
            const basegfx::B2DVector& rRectPoint,
            bool bTiling,
            bool bStretch,
            bool bLogSize)
        :   mpSdrFillGraphicAttribute(
                ImpSdrFillGraphicAttribute(
                    rFillGraphic,
                    rGraphicLogicSize,
                    rSize,
                    rOffset,
                    rOffsetPosition,
                    rRectPoint,
                    bTiling,
                    bStretch,
                    bLogSize))
        {
        }

        SdrFillGraphicAttribute::SdrFillGraphicAttribute()
        :   mpSdrFillGraphicAttribute(theGlobalDefault())
        {
        }

        SdrFillGraphicAttribute::SdrFillGraphicAttribute(const SdrFillGraphicAttribute&) = default;

        SdrFillGraphicAttribute::SdrFillGraphicAttribute(SdrFillGraphicAttribute&&) = default;

        SdrFillGraphicAttribute::~SdrFillGraphicAttribute() = default;

        bool SdrFillGraphicAttribute::isDefault() const
        {
            return mpSdrFillGraphicAttribute.same_object(theGlobalDefault());
        }

        SdrFillGraphicAttribute& SdrFillGraphicAttribute::operator=(const SdrFillGraphicAttribute&) = default;

        SdrFillGraphicAttribute& SdrFillGraphicAttribute::operator=(SdrFillGraphicAttribute&&) = default;

        bool SdrFillGraphicAttribute::operator==(const SdrFillGraphicAttribute& rCandidate) const
        {
            // tdf#87509 default attr is always != non-default attr, even with same values
            if(rCandidate.isDefault() != isDefault())
                return false;

            return rCandidate.mpSdrFillGraphicAttribute == mpSdrFillGraphicAttribute;
        }

        const Graphic& SdrFillGraphicAttribute::getFillGraphic() const
        {
            return mpSdrFillGraphicAttribute->getFillGraphic();
        }

        const basegfx::B2DVector& SdrFillGraphicAttribute::getGraphicLogicSize() const
        {
            return mpSdrFillGraphicAttribute->getGraphicLogicSize();
        }

        const basegfx::B2DVector& SdrFillGraphicAttribute::getSize() const
        {
            return mpSdrFillGraphicAttribute->getSize();
        }

        const basegfx::B2DVector& SdrFillGraphicAttribute::getOffset() const
        {
            return mpSdrFillGraphicAttribute->getOffset();
        }

        const basegfx::B2DVector& SdrFillGraphicAttribute::getOffsetPosition() const
        {
            return mpSdrFillGraphicAttribute->getOffsetPosition();
        }

        const basegfx::B2DVector& SdrFillGraphicAttribute::getRectPoint() const
        {
            return mpSdrFillGraphicAttribute->getRectPoint();
        }

        bool SdrFillGraphicAttribute::getTiling() const
        {
            return mpSdrFillGraphicAttribute->getTiling();
        }

        FillGraphicAttribute SdrFillGraphicAttribute::createFillGraphicAttribute(const basegfx::B2DRange& rRange) const
        {
            // get logical size of bitmap (before possibly expanding it)
            Graphic aGraphic(getFillGraphic());

            // init values with defaults for stretched
            basegfx::B2DPoint aBitmapSize(1.0, 1.0);
            basegfx::B2DVector aBitmapTopLeft(0.0, 0.0);

            // are changes needed? When stretched we are already done, all other values will have no influence
            if(getTiling() || !mpSdrFillGraphicAttribute->getStretch())
            {
                // init values with range sizes
                const double fRangeWidth(0.0 != rRange.getWidth() ? rRange.getWidth() : 1.0);
                const double fRangeHeight(0.0 != rRange.getHeight() ? rRange.getHeight() : 1.0);
                aBitmapSize = basegfx::B2DPoint(fRangeWidth, fRangeHeight);

                // size changes
                if(0.0 != getSize().getX())
                {
                    if(getSize().getX() < 0.0)
                    {
                        aBitmapSize.setX(aBitmapSize.getX() * (getSize().getX() * -0.01));
                    }
                    else
                    {
                        aBitmapSize.setX(getSize().getX());
                    }
                }
                else
                {
                    // #i124002# use GraphicLogicSize directly, do not try to use GetPrefSize
                    // of the graphic, that may not be adapted to the MapMode of the target
                    aBitmapSize.setX(getGraphicLogicSize().getX());
                }

                if(0.0 != getSize().getY())
                {
                    if(getSize().getY() < 0.0)
                    {
                        aBitmapSize.setY(aBitmapSize.getY() * (getSize().getY() * -0.01));
                    }
                    else
                    {
                        aBitmapSize.setY(getSize().getY());
                    }
                }
                else
                {
                    // #i124002# use GraphicLogicSize directly, do not try to use GetPrefSize
                    // of the graphic, that may not be adapted to the MapMode of the target
                    aBitmapSize.setY(getGraphicLogicSize().getY());
                }

                // position changes X
                if(0.0 == getRectPoint().getX())
                {
                    aBitmapTopLeft.setX((fRangeWidth - aBitmapSize.getX()) * 0.5);
                }
                else if(1.0 == getRectPoint().getX())
                {
                    aBitmapTopLeft.setX(fRangeWidth - aBitmapSize.getX());
                }

                // offset positions are only meaningful when tiled
                if(getTiling() && 0.0 != getOffsetPosition().getX())
                {
                    aBitmapTopLeft.setX(aBitmapTopLeft.getX() + (aBitmapSize.getX() * (getOffsetPosition().getX() * 0.01)));
                }

                // position changes Y
                if(0.0 == getRectPoint().getY())
                {
                    aBitmapTopLeft.setY((fRangeHeight - aBitmapSize.getY()) * 0.5);
                }
                else if(1.0 == getRectPoint().getY())
                {
                    aBitmapTopLeft.setY(fRangeHeight - aBitmapSize.getY());
                }

                // offset positions are only meaningful when tiled
                if(getTiling() && 0.0 != getOffsetPosition().getY())
                {
                    aBitmapTopLeft.setY(aBitmapTopLeft.getY() + (aBitmapSize.getY() * (getOffsetPosition().getY() * 0.01)));
                }

                // apply bitmap size scaling to unit rectangle
                aBitmapTopLeft.setX(aBitmapTopLeft.getX() / fRangeWidth);
                aBitmapTopLeft.setY(aBitmapTopLeft.getY() / fRangeHeight);
                aBitmapSize.setX(aBitmapSize.getX() / fRangeWidth);
                aBitmapSize.setY(aBitmapSize.getY() / fRangeHeight);
            }

            // get offset in percent
            const double fOffsetX(std::clamp(getOffset().getX() * 0.01, 0.0, 1.0));
            const double fOffsetY(std::clamp(getOffset().getY() * 0.01, 0.0, 1.0));

            // create FillGraphicAttribute
            return FillGraphicAttribute(
                aGraphic,
                basegfx::B2DRange(aBitmapTopLeft, aBitmapTopLeft + aBitmapSize),
                getTiling(),
                fOffsetX,
                fOffsetY);
        }

} // end of namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
