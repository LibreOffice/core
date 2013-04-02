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

#include <drawinglayer/attribute/sdrfillbitmapattribute.hxx>
#include <drawinglayer/attribute/fillbitmapattribute.hxx>
#include <rtl/instance.hxx>
#include <vcl/bitmapex.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace attribute
    {
        class ImpSdrFillBitmapAttribute
        {
        public:
            // data definitions
            BitmapEx                                maBitmapEx;
            basegfx::B2DVector                      maSize;
            basegfx::B2DVector                      maOffset;
            basegfx::B2DVector                      maOffsetPosition;
            basegfx::B2DVector                      maRectPoint;

            // bitfield
            unsigned                                mbTiling : 1;
            unsigned                                mbStretch : 1;
            unsigned                                mbLogSize : 1;

            ImpSdrFillBitmapAttribute(
                const BitmapEx& rBitmapEx,
                const basegfx::B2DVector& rSize,
                const basegfx::B2DVector& rOffset,
                const basegfx::B2DVector& rOffsetPosition,
                const basegfx::B2DVector& rRectPoint,
                bool bTiling,
                bool bStretch,
                bool bLogSize)
            :   maBitmapEx(rBitmapEx),
                maSize(rSize),
                maOffset(rOffset),
                maOffsetPosition(rOffsetPosition),
                maRectPoint(rRectPoint),
                mbTiling(bTiling),
                mbStretch(bStretch),
                mbLogSize(bLogSize)
            {
            }

            ImpSdrFillBitmapAttribute()
            :   maBitmapEx(BitmapEx()),
                maSize(basegfx::B2DVector()),
                maOffset(basegfx::B2DVector()),
                maOffsetPosition(basegfx::B2DVector()),
                maRectPoint(basegfx::B2DVector()),
                mbTiling(false),
                mbStretch(false),
                mbLogSize(false)
            {
            }

            // data read access
            const BitmapEx& getBitmapEx() const { return maBitmapEx; }
            const basegfx::B2DVector& getSize() const { return maSize; }
            const basegfx::B2DVector& getOffset() const { return maOffset; }
            const basegfx::B2DVector& getOffsetPosition() const { return maOffsetPosition; }
            const basegfx::B2DVector& getRectPoint() const { return maRectPoint; }
            bool getTiling() const { return mbTiling; }
            bool getStretch() const { return mbStretch; }
            bool getLogSize() const { return mbLogSize; }

            bool operator==(const ImpSdrFillBitmapAttribute& rCandidate) const
            {
                return (getBitmapEx() == rCandidate.getBitmapEx()
                    && getSize() == rCandidate.getSize()
                    && getOffset() == rCandidate.getOffset()
                    && getOffsetPosition() == rCandidate.getOffsetPosition()
                    && getRectPoint() == rCandidate.getRectPoint()
                    && getTiling() == rCandidate.getTiling()
                    && getStretch() == rCandidate.getStretch()
                    && getLogSize() == rCandidate.getLogSize());
            }
        };

        namespace
        {
            struct theGlobalDefault :
                public rtl::Static< SdrFillBitmapAttribute::ImplType, theGlobalDefault > {};
        }

        SdrFillBitmapAttribute::SdrFillBitmapAttribute(
            const BitmapEx& rBitmapEx,
            const basegfx::B2DVector& rSize,
            const basegfx::B2DVector& rOffset,
            const basegfx::B2DVector& rOffsetPosition,
            const basegfx::B2DVector& rRectPoint,
            bool bTiling,
            bool bStretch,
            bool bLogSize)
        :   mpSdrFillBitmapAttribute(
                ImpSdrFillBitmapAttribute(
                    rBitmapEx,
                    rSize,
                    rOffset,
                    rOffsetPosition,
                    rRectPoint,
                    bTiling,
                    bStretch,
                    bLogSize))
        {
        }

        SdrFillBitmapAttribute::SdrFillBitmapAttribute()
        :   mpSdrFillBitmapAttribute(theGlobalDefault::get())
        {
        }

        SdrFillBitmapAttribute::SdrFillBitmapAttribute(const SdrFillBitmapAttribute& rCandidate)
        :   mpSdrFillBitmapAttribute(rCandidate.mpSdrFillBitmapAttribute)
        {
        }

        SdrFillBitmapAttribute::~SdrFillBitmapAttribute()
        {
        }

        bool SdrFillBitmapAttribute::isDefault() const
        {
            return mpSdrFillBitmapAttribute.same_object(theGlobalDefault::get());
        }

        SdrFillBitmapAttribute& SdrFillBitmapAttribute::operator=(const SdrFillBitmapAttribute& rCandidate)
        {
            mpSdrFillBitmapAttribute = rCandidate.mpSdrFillBitmapAttribute;
            return *this;
        }

        bool SdrFillBitmapAttribute::operator==(const SdrFillBitmapAttribute& rCandidate) const
        {
            return rCandidate.mpSdrFillBitmapAttribute == mpSdrFillBitmapAttribute;
        }

        const BitmapEx& SdrFillBitmapAttribute::getBitmapEx() const
        {
            return mpSdrFillBitmapAttribute->getBitmapEx();
        }

        const basegfx::B2DVector& SdrFillBitmapAttribute::getSize() const
        {
            return mpSdrFillBitmapAttribute->getSize();
        }

        const basegfx::B2DVector& SdrFillBitmapAttribute::getOffset() const
        {
            return mpSdrFillBitmapAttribute->getOffset();
        }

        const basegfx::B2DVector& SdrFillBitmapAttribute::getOffsetPosition() const
        {
            return mpSdrFillBitmapAttribute->getOffsetPosition();
        }

        const basegfx::B2DVector& SdrFillBitmapAttribute::getRectPoint() const
        {
            return mpSdrFillBitmapAttribute->getRectPoint();
        }

        bool SdrFillBitmapAttribute::getTiling() const
        {
            return mpSdrFillBitmapAttribute->getTiling();
        }

        bool SdrFillBitmapAttribute::getStretch() const
        {
            return mpSdrFillBitmapAttribute->getStretch();
        }

        FillBitmapAttribute SdrFillBitmapAttribute::getFillBitmapAttribute(const basegfx::B2DRange& rRange) const
        {
            // get logical size of bitmap (before expanding eventually)
            BitmapEx aBitmapEx(getBitmapEx());
            const basegfx::B2DVector aLogicalSize(aBitmapEx.GetPrefSize().getWidth(), aBitmapEx.GetPrefSize().getHeight());

            // get hor/ver shiftings and apply them eventually to the bitmap, but only
            // when tiling is on
            bool bExpandWidth(false);
            bool bExpandHeight(false);

            if(getTiling())
            {
                if(0.0 != getOffset().getX() || 0.0 != getOffset().getY())
                {
                    const sal_uInt32 nWidth(aBitmapEx.GetSizePixel().getWidth());
                    const sal_uInt32 nHeight(aBitmapEx.GetSizePixel().getHeight());

                    if(0.0 != getOffset().getX())
                    {
                        bExpandHeight = true;
                        const sal_uInt32 nOffset(basegfx::fround(((double)nWidth * getOffset().getX()) / 100.0));
                        aBitmapEx.Expand(0L, nHeight);

                        const Size aSizeA(nOffset, nHeight);
                        const Rectangle aDstA(Point(0L, nHeight), aSizeA);
                        const Rectangle aSrcA(Point(nWidth - nOffset, 0L), aSizeA);
                        aBitmapEx.CopyPixel(aDstA, aSrcA);

                        const Size aSizeB(nWidth - nOffset, nHeight);
                        const Rectangle aDstB(Point(nOffset, nHeight), aSizeB);
                        const Rectangle aSrcB(Point(0L, 0L), aSizeB);
                        aBitmapEx.CopyPixel(aDstB, aSrcB);
                    }
                    else
                    {
                        bExpandWidth = true;
                        const sal_uInt32 nOffset(basegfx::fround(((double)nHeight * getOffset().getY()) / 100.0));
                        aBitmapEx.Expand(nWidth, 0L);

                        const Size aSize(nWidth, nHeight);
                        const Rectangle aDst(Point(nWidth, 0L), aSize);
                        const Rectangle aSrc(Point(0L, 0L), aSize);
                        aBitmapEx.CopyPixel(aDst, aSrc);

                        const Size aSizeA(nWidth, nOffset);
                        const Rectangle aDstA(Point(0L, 0L), aSizeA);
                        const Rectangle aSrcA(Point(nWidth, nHeight - nOffset), aSizeA);
                        aBitmapEx.CopyPixel(aDstA, aSrcA);

                        const Size aSizeB(nWidth, nHeight - nOffset);
                        const Rectangle aDstB(Point(0L, nOffset), aSizeB);
                        const Rectangle aSrcB(Point(nWidth, 0L), aSizeB);
                        aBitmapEx.CopyPixel(aDstB, aSrcB);
                    }
                }
            }

            // init values with defaults
            basegfx::B2DPoint aBitmapSize(1.0, 1.0);
            basegfx::B2DVector aBitmapTopLeft(0.0, 0.0);

            // are canges needed?
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
                    aBitmapSize.setX(aLogicalSize.getX());
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
                    aBitmapSize.setY(aLogicalSize.getY());
                }

                // get values, force to centered if necessary
                const basegfx::B2DVector aRectPoint(getTiling() ? getRectPoint() : basegfx::B2DVector(0.0, 0.0));

                // position changes X
                if(0.0 == aRectPoint.getX())
                {
                    aBitmapTopLeft.setX((fRangeWidth - aBitmapSize.getX()) * 0.5);
                }
                else if(1.0 == aRectPoint.getX())
                {
                    aBitmapTopLeft.setX(fRangeWidth - aBitmapSize.getX());
                }

                if(getTiling() && 0.0 != getOffsetPosition().getX())
                {
                    aBitmapTopLeft.setX(aBitmapTopLeft.getX() + (aBitmapSize.getX() * (getOffsetPosition().getX() * 0.01)));
                }

                // position changes Y
                if(0.0 == aRectPoint.getY())
                {
                    aBitmapTopLeft.setY((fRangeHeight - aBitmapSize.getY()) * 0.5);
                }
                else if(1.0 == aRectPoint.getY())
                {
                    aBitmapTopLeft.setY(fRangeHeight - aBitmapSize.getY());
                }

                if(getTiling() && 0.0 != getOffsetPosition().getY())
                {
                    aBitmapTopLeft.setY(aBitmapTopLeft.getY() + (aBitmapSize.getY() * (getOffsetPosition().getY() * 0.01)));
                }

                // apply expand
                if(bExpandWidth)
                {
                    aBitmapSize.setX(aBitmapSize.getX() * 2.0);
                }

                if(bExpandHeight)
                {
                    aBitmapSize.setY(aBitmapSize.getY() * 2.0);
                }

                // apply bitmap size scaling to unit rectangle
                aBitmapTopLeft.setX(aBitmapTopLeft.getX() / fRangeWidth);
                aBitmapTopLeft.setY(aBitmapTopLeft.getY() / fRangeHeight);
                aBitmapSize.setX(aBitmapSize.getX() / fRangeWidth);
                aBitmapSize.setY(aBitmapSize.getY() / fRangeHeight);
            }

            return FillBitmapAttribute(aBitmapEx, aBitmapTopLeft, aBitmapSize, getTiling());
        }
    } // end of namespace attribute
} // end of namespace drawinglayer

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
