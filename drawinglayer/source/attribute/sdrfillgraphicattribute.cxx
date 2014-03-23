/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_drawinglayer.hxx"

#include <drawinglayer/attribute/sdrfillgraphicattribute.hxx>
#include <drawinglayer/attribute/fillgraphicattribute.hxx>
#include <vcl/graph.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace attribute
    {
        class ImpSdrFillGraphicAttribute
        {
        public:
            // refcounter
            sal_uInt32                              mnRefCount;

            // data definitions
            Graphic                                 maFillGraphic;
            basegfx::B2DVector                      maGraphicLogicSize;
            basegfx::B2DVector                      maSize;
            basegfx::B2DVector                      maOffset;
            basegfx::B2DVector                      maOffsetPosition;
            basegfx::B2DVector                      maRectPoint;

            // bitfield
            unsigned                                mbTiling : 1;
            unsigned                                mbStretch : 1;
            unsigned                                mbLogSize : 1;

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
            :   mnRefCount(0),
                maFillGraphic(rFillGraphic),
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

            // data read access
            const Graphic& getFillGraphic() const { return maFillGraphic; }
            const basegfx::B2DVector& getGraphicLogicSize() const { return maGraphicLogicSize; }
            const basegfx::B2DVector& getSize() const { return maSize; }
            const basegfx::B2DVector& getOffset() const { return maOffset; }
            const basegfx::B2DVector& getOffsetPosition() const { return maOffsetPosition; }
            const basegfx::B2DVector& getRectPoint() const { return maRectPoint; }
            bool getTiling() const { return mbTiling; }
            bool getStretch() const { return mbStretch; }
            bool getLogSize() const { return mbLogSize; }

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
                    && getLogSize() == rCandidate.getLogSize());
            }

            static ImpSdrFillGraphicAttribute* get_global_default()
            {
                static ImpSdrFillGraphicAttribute* pDefault = 0;

                if(!pDefault)
                {
                    pDefault = new ImpSdrFillGraphicAttribute(
                        Graphic(),
                        basegfx::B2DVector(),
                        basegfx::B2DVector(),
                        basegfx::B2DVector(),
                        basegfx::B2DVector(),
                        basegfx::B2DVector(),
                        false,
                        false,
                        false);

                    // never delete; start with RefCount 1, not 0
                    pDefault->mnRefCount++;
                }

                return pDefault;
            }
        };

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
                new ImpSdrFillGraphicAttribute(
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
        :   mpSdrFillGraphicAttribute(ImpSdrFillGraphicAttribute::get_global_default())
        {
            mpSdrFillGraphicAttribute->mnRefCount++;
        }

        SdrFillGraphicAttribute::SdrFillGraphicAttribute(const SdrFillGraphicAttribute& rCandidate)
        :   mpSdrFillGraphicAttribute(rCandidate.mpSdrFillGraphicAttribute)
        {
            mpSdrFillGraphicAttribute->mnRefCount++;
        }

        SdrFillGraphicAttribute::~SdrFillGraphicAttribute()
        {
            if(mpSdrFillGraphicAttribute->mnRefCount)
            {
                mpSdrFillGraphicAttribute->mnRefCount--;
            }
            else
            {
                delete mpSdrFillGraphicAttribute;
            }
        }

        bool SdrFillGraphicAttribute::isDefault() const
        {
            return mpSdrFillGraphicAttribute == ImpSdrFillGraphicAttribute::get_global_default();
        }

        SdrFillGraphicAttribute& SdrFillGraphicAttribute::operator=(const SdrFillGraphicAttribute& rCandidate)
        {
            if(rCandidate.mpSdrFillGraphicAttribute != mpSdrFillGraphicAttribute)
            {
                if(mpSdrFillGraphicAttribute->mnRefCount)
                {
                    mpSdrFillGraphicAttribute->mnRefCount--;
                }
                else
                {
                    delete mpSdrFillGraphicAttribute;
                }

                mpSdrFillGraphicAttribute = rCandidate.mpSdrFillGraphicAttribute;
                mpSdrFillGraphicAttribute->mnRefCount++;
            }

            return *this;
        }

        bool SdrFillGraphicAttribute::operator==(const SdrFillGraphicAttribute& rCandidate) const
        {
            if(rCandidate.mpSdrFillGraphicAttribute == mpSdrFillGraphicAttribute)
            {
                return true;
            }

            if(rCandidate.isDefault() != isDefault())
            {
                return false;
            }

            return (*rCandidate.mpSdrFillGraphicAttribute == *mpSdrFillGraphicAttribute);
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

        bool SdrFillGraphicAttribute::getStretch() const
        {
            return mpSdrFillGraphicAttribute->getStretch();
        }

        bool SdrFillGraphicAttribute::getLogSize() const
        {
            return mpSdrFillGraphicAttribute->getLogSize();
        }

        FillGraphicAttribute SdrFillGraphicAttribute::createFillGraphicAttribute(const basegfx::B2DRange& rRange) const
        {
            // get logical size of bitmap (before expanding eventually)
            Graphic aGraphic(getFillGraphic());

            // init values with defaults for stretched
            basegfx::B2DPoint aBitmapSize(1.0, 1.0);
            basegfx::B2DVector aBitmapTopLeft(0.0, 0.0);

            //UUUU are changes needed? When streched we are already done, all other values will have no influence
            if(getTiling() || !getStretch())
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
                    // #124002# use GraphicLogicSize directly, do not try to use GetPrefSize
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
                    // #124002# use GraphicLogicSize directly, do not try to use GetPrefSize
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
            const double fOffsetX(basegfx::clamp(getOffset().getX() * 0.01, 0.0, 1.0));
            const double fOffsetY(basegfx::clamp(getOffset().getY() * 0.01, 0.0, 1.0));

            // create FillGraphicAttribute
            return FillGraphicAttribute(
                aGraphic,
                basegfx::B2DRange(aBitmapTopLeft, aBitmapTopLeft + aBitmapSize),
                getTiling(),
                fOffsetX,
                fOffsetY);
        }
    } // end of namespace attribute
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////
// eof
