/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#include <drawinglayer/attribute/sdrfillbitmapattribute.hxx>
#include <drawinglayer/attribute/fillbitmapattribute.hxx>
#include <vcl/bitmapex.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace attribute
    {
        class ImpSdrFillBitmapAttribute
        {
        public:
            // refcounter
            sal_uInt32                              mnRefCount;

            // data definitions
            Bitmap                                  maBitmap;
            basegfx::B2DVector                      maSize;
            basegfx::B2DVector                      maOffset;
            basegfx::B2DVector                      maOffsetPosition;
            basegfx::B2DVector                      maRectPoint;

            // bitfield
            unsigned                                mbTiling : 1;
            unsigned                                mbStretch : 1;
            unsigned                                mbLogSize : 1;

            ImpSdrFillBitmapAttribute(
                const Bitmap& rBitmap,
                const basegfx::B2DVector& rSize,
                const basegfx::B2DVector& rOffset,
                const basegfx::B2DVector& rOffsetPosition,
                const basegfx::B2DVector& rRectPoint,
                bool bTiling,
                bool bStretch,
                bool bLogSize)
            :   mnRefCount(0),
                maBitmap(rBitmap),
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
            const Bitmap& getBitmap() const { return maBitmap; }
            const basegfx::B2DVector& getSize() const { return maSize; }
            const basegfx::B2DVector& getOffset() const { return maOffset; }
            const basegfx::B2DVector& getOffsetPosition() const { return maOffsetPosition; }
            const basegfx::B2DVector& getRectPoint() const { return maRectPoint; }
            bool getTiling() const { return mbTiling; }
            bool getStretch() const { return mbStretch; }
            bool getLogSize() const { return mbLogSize; }

            bool operator==(const ImpSdrFillBitmapAttribute& rCandidate) const
            {
                return (getBitmap() == rCandidate.getBitmap()
                    && getSize() == rCandidate.getSize()
                    && getOffset() == rCandidate.getOffset()
                    && getOffsetPosition() == rCandidate.getOffsetPosition()
                    && getRectPoint() == rCandidate.getRectPoint()
                    && getTiling() == rCandidate.getTiling()
                    && getStretch() == rCandidate.getStretch()
                    && getLogSize() == rCandidate.getLogSize());
            }

            static ImpSdrFillBitmapAttribute* get_global_default()
            {
                static ImpSdrFillBitmapAttribute* pDefault = 0;

                if(!pDefault)
                {
                    pDefault = new ImpSdrFillBitmapAttribute(
                        Bitmap(),
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

        SdrFillBitmapAttribute::SdrFillBitmapAttribute(
            const Bitmap& rBitmap,
            const basegfx::B2DVector& rSize,
            const basegfx::B2DVector& rOffset,
            const basegfx::B2DVector& rOffsetPosition,
            const basegfx::B2DVector& rRectPoint,
            bool bTiling,
            bool bStretch,
            bool bLogSize)
        :   mpSdrFillBitmapAttribute(new ImpSdrFillBitmapAttribute(
                rBitmap, rSize, rOffset, rOffsetPosition, rRectPoint, bTiling,  bStretch, bLogSize))
        {
        }

        SdrFillBitmapAttribute::SdrFillBitmapAttribute()
        :   mpSdrFillBitmapAttribute(ImpSdrFillBitmapAttribute::get_global_default())
        {
            mpSdrFillBitmapAttribute->mnRefCount++;
        }

        SdrFillBitmapAttribute::SdrFillBitmapAttribute(const SdrFillBitmapAttribute& rCandidate)
        :   mpSdrFillBitmapAttribute(rCandidate.mpSdrFillBitmapAttribute)
        {
            mpSdrFillBitmapAttribute->mnRefCount++;
        }

        SdrFillBitmapAttribute::~SdrFillBitmapAttribute()
        {
            if(mpSdrFillBitmapAttribute->mnRefCount)
            {
                mpSdrFillBitmapAttribute->mnRefCount--;
            }
            else
            {
                delete mpSdrFillBitmapAttribute;
            }
        }

        bool SdrFillBitmapAttribute::isDefault() const
        {
            return mpSdrFillBitmapAttribute == ImpSdrFillBitmapAttribute::get_global_default();
        }

        SdrFillBitmapAttribute& SdrFillBitmapAttribute::operator=(const SdrFillBitmapAttribute& rCandidate)
        {
            if(rCandidate.mpSdrFillBitmapAttribute != mpSdrFillBitmapAttribute)
            {
                if(mpSdrFillBitmapAttribute->mnRefCount)
                {
                    mpSdrFillBitmapAttribute->mnRefCount--;
                }
                else
                {
                    delete mpSdrFillBitmapAttribute;
                }

                mpSdrFillBitmapAttribute = rCandidate.mpSdrFillBitmapAttribute;
                mpSdrFillBitmapAttribute->mnRefCount++;
            }

            return *this;
        }

        bool SdrFillBitmapAttribute::operator==(const SdrFillBitmapAttribute& rCandidate) const
        {
            if(rCandidate.mpSdrFillBitmapAttribute == mpSdrFillBitmapAttribute)
            {
                return true;
            }

            if(rCandidate.isDefault() != isDefault())
            {
                return false;
            }

            return (*rCandidate.mpSdrFillBitmapAttribute == *mpSdrFillBitmapAttribute);
        }

        const Bitmap& SdrFillBitmapAttribute::getBitmap() const
        {
            return mpSdrFillBitmapAttribute->getBitmap();
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
            Bitmap aBitmap(getBitmap());
            const basegfx::B2DVector aLogicalSize(aBitmap.GetPrefSize().getWidth(), aBitmap.GetPrefSize().getHeight());

            // get hor/ver shiftings and apply them eventually to the bitmap, but only
            // when tiling is on
            bool bExpandWidth(false);
            bool bExpandHeight(false);

            if(getTiling())
            {
                if(0.0 != getOffset().getX() || 0.0 != getOffset().getY())
                {
                    const sal_uInt32 nWidth(aBitmap.GetSizePixel().getWidth());
                    const sal_uInt32 nHeight(aBitmap.GetSizePixel().getHeight());

                    if(0.0 != getOffset().getX())
                    {
                        bExpandHeight = true;
                        const sal_uInt32 nOffset(basegfx::fround(((double)nWidth * getOffset().getX()) / 100.0));
                        aBitmap.Expand(0L, nHeight);

                        const Size aSizeA(nOffset, nHeight);
                        const Rectangle aDstA(Point(0L, nHeight), aSizeA);
                        const Rectangle aSrcA(Point(nWidth - nOffset, 0L), aSizeA);
                        aBitmap.CopyPixel(aDstA, aSrcA);

                        const Size aSizeB(nWidth - nOffset, nHeight);
                        const Rectangle aDstB(Point(nOffset, nHeight), aSizeB);
                        const Rectangle aSrcB(Point(0L, 0L), aSizeB);
                        aBitmap.CopyPixel(aDstB, aSrcB);
                    }
                    else
                    {
                        bExpandWidth = true;
                        const sal_uInt32 nOffset(basegfx::fround(((double)nHeight * getOffset().getY()) / 100.0));
                        aBitmap.Expand(nWidth, 0L);

                        const Size aSize(nWidth, nHeight);
                        const Rectangle aDst(Point(nWidth, 0L), aSize);
                        const Rectangle aSrc(Point(0L, 0L), aSize);
                        aBitmap.CopyPixel(aDst, aSrc);

                        const Size aSizeA(nWidth, nOffset);
                        const Rectangle aDstA(Point(0L, 0L), aSizeA);
                        const Rectangle aSrcA(Point(nWidth, nHeight - nOffset), aSizeA);
                        aBitmap.CopyPixel(aDstA, aSrcA);

                        const Size aSizeB(nWidth, nHeight - nOffset);
                        const Rectangle aDstB(Point(0L, nOffset), aSizeB);
                        const Rectangle aSrcB(Point(nWidth, 0L), aSizeB);
                        aBitmap.CopyPixel(aDstB, aSrcB);
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

            return FillBitmapAttribute(BitmapEx(aBitmap), aBitmapTopLeft, aBitmapSize, getTiling());
        }
    } // end of namespace attribute
} // end of namespace drawinglayer

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
