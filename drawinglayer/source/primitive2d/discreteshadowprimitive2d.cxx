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

#include <drawinglayer/primitive2d/discreteshadowprimitive2d.hxx>
#include <drawinglayer/primitive2d/drawinglayer_primitivetypes2d.hxx>
#include <drawinglayer/primitive2d/bitmapprimitive2d.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <drawinglayer/primitive2d/transformprimitive2d.hxx>
#include <drawinglayer/geometry/viewinformation2d.hxx>
#include <osl/diagnose.h>
#include <toolkit/helper/vclunohelper.hxx>


namespace drawinglayer::primitive2d
{
        DiscreteShadow::DiscreteShadow(const BitmapEx& rBitmapEx)
        :   maBitmapEx(rBitmapEx)
        {
            const Size& rBitmapSize = getBitmapEx().GetSizePixel();

            if(rBitmapSize.Width() != rBitmapSize.Height() || rBitmapSize.Width() < 7)
            {
                OSL_ENSURE(false, "DiscreteShadowPrimitive2D: wrong bitmap format (!)");
                maBitmapEx = BitmapEx();
            }
        }

        const BitmapEx& DiscreteShadow::getTopLeft() const
        {
            if(maTopLeft.IsEmpty())
            {
                const sal_Int32 nQuarter((getBitmapEx().GetSizePixel().Width() - 3) >> 2);
                const_cast< DiscreteShadow* >(this)->maTopLeft = getBitmapEx();
                const_cast< DiscreteShadow* >(this)->maTopLeft.Crop(
                    ::tools::Rectangle(Point(0, 0), Size((nQuarter * 2) + 1, (nQuarter * 2) + 1)));
            }

            return maTopLeft;
        }

        const BitmapEx& DiscreteShadow::getTop() const
        {
            if(maTop.IsEmpty())
            {
                const sal_Int32 nQuarter((getBitmapEx().GetSizePixel().Width() - 3) >> 2);
                const_cast< DiscreteShadow* >(this)->maTop = getBitmapEx();
                const_cast< DiscreteShadow* >(this)->maTop.Crop(
                    ::tools::Rectangle(Point((nQuarter * 2) + 1, 0), Size(1, nQuarter)));
            }

            return maTop;
        }

        const BitmapEx& DiscreteShadow::getTopRight() const
        {
            if(maTopRight.IsEmpty())
            {
                const sal_Int32 nQuarter((getBitmapEx().GetSizePixel().Width() - 3) >> 2);
                const_cast< DiscreteShadow* >(this)->maTopRight = getBitmapEx();
                const_cast< DiscreteShadow* >(this)->maTopRight.Crop(
                    ::tools::Rectangle(Point((nQuarter * 2) + 2, 0), Size((nQuarter * 2) + 1, (nQuarter * 2) + 1)));
            }

            return maTopRight;
        }

        const BitmapEx& DiscreteShadow::getRight() const
        {
            if(maRight.IsEmpty())
            {
                const sal_Int32 nQuarter((getBitmapEx().GetSizePixel().Width() - 3) >> 2);
                const_cast< DiscreteShadow* >(this)->maRight = getBitmapEx();
                const_cast< DiscreteShadow* >(this)->maRight.Crop(
                    ::tools::Rectangle(Point((nQuarter * 3) + 3, (nQuarter * 2) + 1), Size(nQuarter, 1)));
            }

            return maRight;
        }

        const BitmapEx& DiscreteShadow::getBottomRight() const
        {
            if(maBottomRight.IsEmpty())
            {
                const sal_Int32 nQuarter((getBitmapEx().GetSizePixel().Width() - 3) >> 2);
                const_cast< DiscreteShadow* >(this)->maBottomRight = getBitmapEx();
                const_cast< DiscreteShadow* >(this)->maBottomRight.Crop(
                    ::tools::Rectangle(Point((nQuarter * 2) + 2, (nQuarter * 2) + 2), Size((nQuarter * 2) + 1, (nQuarter * 2) + 1)));
            }

            return maBottomRight;
        }

        const BitmapEx& DiscreteShadow::getBottom() const
        {
            if(maBottom.IsEmpty())
            {
                const sal_Int32 nQuarter((getBitmapEx().GetSizePixel().Width() - 3) >> 2);
                const_cast< DiscreteShadow* >(this)->maBottom = getBitmapEx();
                const_cast< DiscreteShadow* >(this)->maBottom.Crop(
                    ::tools::Rectangle(Point((nQuarter * 2) + 1, (nQuarter * 3) + 3), Size(1, nQuarter)));
            }

            return maBottom;
        }

        const BitmapEx& DiscreteShadow::getBottomLeft() const
        {
            if(maBottomLeft.IsEmpty())
            {
                const sal_Int32 nQuarter((getBitmapEx().GetSizePixel().Width() - 3) >> 2);
                const_cast< DiscreteShadow* >(this)->maBottomLeft = getBitmapEx();
                const_cast< DiscreteShadow* >(this)->maBottomLeft.Crop(
                    ::tools::Rectangle(Point(0, (nQuarter * 2) + 2), Size((nQuarter * 2) + 1, (nQuarter * 2) + 1)));
            }

            return maBottomLeft;
        }

        const BitmapEx& DiscreteShadow::getLeft() const
        {
            if(maLeft.IsEmpty())
            {
                const sal_Int32 nQuarter((getBitmapEx().GetSizePixel().Width() - 3) >> 2);
                const_cast< DiscreteShadow* >(this)->maLeft = getBitmapEx();
                const_cast< DiscreteShadow* >(this)->maLeft.Crop(
                    ::tools::Rectangle(Point(0, (nQuarter * 2) + 1), Size(nQuarter, 1)));
            }

            return maLeft;
        }

} // end of namespace


namespace drawinglayer::primitive2d
{
        void DiscreteShadowPrimitive2D::create2DDecomposition(Primitive2DContainer& rContainer, const geometry::ViewInformation2D& /*rViewInformation*/) const
        {
            Primitive2DContainer xRetval;

            if(getDiscreteShadow().getBitmapEx().IsEmpty())
                return;

            const sal_Int32 nQuarter((getDiscreteShadow().getBitmapEx().GetSizePixel().Width() - 3) >> 2);
            const basegfx::B2DVector aScale(getTransform() * basegfx::B2DVector(1.0, 1.0));
            const double fSingleX(getDiscreteUnit() / aScale.getX());
            const double fSingleY(getDiscreteUnit() / aScale.getY());
            const double fBorderX(fSingleX * nQuarter);
            const double fBorderY(fSingleY * nQuarter);
            const double fBigLenX((fBorderX * 2.0) + fSingleX);
            const double fBigLenY((fBorderY * 2.0) + fSingleY);

            xRetval.resize(8);

            // TopLeft
            xRetval[0] = Primitive2DReference(
                new BitmapPrimitive2D(
                    VCLUnoHelper::CreateVCLXBitmap(getDiscreteShadow().getTopLeft()),
                    basegfx::utils::createScaleTranslateB2DHomMatrix(
                        fBigLenX,
                        fBigLenY,
                        -fBorderX,
                        -fBorderY)));

            // Top
            xRetval[1] = Primitive2DReference(
                new BitmapPrimitive2D(
                    VCLUnoHelper::CreateVCLXBitmap(getDiscreteShadow().getTop()),
                    basegfx::utils::createScaleTranslateB2DHomMatrix(
                        1.0 - (2.0 * (fBorderX + fSingleX)) + fSingleX,
                        fBorderY,
                        fBorderX + fSingleX,
                        -fBorderY)));

            // TopRight
            xRetval[2] = Primitive2DReference(
                new BitmapPrimitive2D(
                    VCLUnoHelper::CreateVCLXBitmap(getDiscreteShadow().getTopRight()),
                    basegfx::utils::createScaleTranslateB2DHomMatrix(
                        fBigLenX,
                        fBigLenY,
                        1.0 - fBorderX,
                        -fBorderY)));

            // Right
            xRetval[3] = Primitive2DReference(
                new BitmapPrimitive2D(
                    VCLUnoHelper::CreateVCLXBitmap(getDiscreteShadow().getRight()),
                    basegfx::utils::createScaleTranslateB2DHomMatrix(
                        fBorderX,
                        1.0 - (2.0 * (fBorderY + fSingleY)) + fSingleY,
                        1.0 + fSingleX,
                        fBorderY + fSingleY)));

            // BottomRight
            xRetval[4] = Primitive2DReference(
                new BitmapPrimitive2D(
                    VCLUnoHelper::CreateVCLXBitmap(getDiscreteShadow().getBottomRight()),
                    basegfx::utils::createScaleTranslateB2DHomMatrix(
                        fBigLenX,
                        fBigLenY,
                        1.0 - (fBorderX + fSingleX) + fSingleX,
                        1.0 - (fBorderY + fSingleY) + fSingleY)));

            // Bottom
            xRetval[5] = Primitive2DReference(
                new BitmapPrimitive2D(
                    VCLUnoHelper::CreateVCLXBitmap(getDiscreteShadow().getBottom()),
                    basegfx::utils::createScaleTranslateB2DHomMatrix(
                        1.0 - (2.0 * (fBorderX + fSingleX)) + fSingleX,
                        fBorderY,
                        fBorderX + fSingleX,
                        1.0 + fSingleY)));

            // BottomLeft
            xRetval[6] = Primitive2DReference(
                new BitmapPrimitive2D(
                    VCLUnoHelper::CreateVCLXBitmap(getDiscreteShadow().getBottomLeft()),
                    basegfx::utils::createScaleTranslateB2DHomMatrix(
                        fBigLenX,
                        fBigLenY,
                        -fBorderX,
                        1.0 - fBorderY)));

            // Left
            xRetval[7] = Primitive2DReference(
                new BitmapPrimitive2D(
                    VCLUnoHelper::CreateVCLXBitmap(getDiscreteShadow().getLeft()),
                    basegfx::utils::createScaleTranslateB2DHomMatrix(
                        fBorderX,
                        1.0 - (2.0 * (fBorderY + fSingleY)) + fSingleY,
                        -fBorderX,
                        fBorderY + fSingleY)));

            // put all in object transformation to get to target positions
            rContainer.push_back(
                new TransformPrimitive2D(
                    getTransform(),
                    std::move(xRetval)));
        }

        DiscreteShadowPrimitive2D::DiscreteShadowPrimitive2D(
            const basegfx::B2DHomMatrix& rTransform,
            const DiscreteShadow& rDiscreteShadow)
        :   maTransform(rTransform),
            maDiscreteShadow(rDiscreteShadow)
        {
        }

        bool DiscreteShadowPrimitive2D::operator==(const BasePrimitive2D& rPrimitive) const
        {
            if(DiscreteMetricDependentPrimitive2D::operator==(rPrimitive))
            {
                const DiscreteShadowPrimitive2D& rCompare = static_cast<const DiscreteShadowPrimitive2D&>(rPrimitive);

                return (getTransform() == rCompare.getTransform()
                    && getDiscreteShadow() == rCompare.getDiscreteShadow());
            }

            return false;
        }

        basegfx::B2DRange DiscreteShadowPrimitive2D::getB2DRange(const geometry::ViewInformation2D& rViewInformation) const
        {
            if(getDiscreteShadow().getBitmapEx().IsEmpty())
            {
                // no graphics without valid bitmap definition
                return basegfx::B2DRange();
            }
            else
            {
                // prepare normal objectrange
                basegfx::B2DRange aRetval(0.0, 0.0, 1.0, 1.0);
                aRetval.transform(getTransform());

                // extract discrete shadow size and grow
                const basegfx::B2DVector aScale(rViewInformation.getViewTransformation() * basegfx::B2DVector(1.0, 1.0));
                const sal_Int32 nQuarter((getDiscreteShadow().getBitmapEx().GetSizePixel().Width() - 3) >> 2);
                const double fGrowX((1.0 / aScale.getX()) * nQuarter);
                const double fGrowY((1.0 / aScale.getY()) * nQuarter);
                aRetval.grow(std::max(fGrowX, fGrowY));

                return aRetval;
            }
        }

        // provide unique ID
        sal_uInt32 DiscreteShadowPrimitive2D::getPrimitive2DID() const
        {
            return PRIMITIVE2D_ID_DISCRETESHADOWPRIMITIVE2D;
        }

} // end of namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
