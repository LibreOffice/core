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


#include <sdr/overlay/overlaytools.hxx>
#include <svx/sdr/primitive2d/svx_primitivetypes2d.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <drawinglayer/primitive2d/bitmapprimitive2d.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <drawinglayer/primitive2d/PolygonMarkerPrimitive2D.hxx>
#include <drawinglayer/primitive2d/PolyPolygonColorPrimitive2D.hxx>
#include <drawinglayer/primitive2d/PolyPolygonStrokePrimitive2D.hxx>
#include <drawinglayer/primitive2d/PolyPolygonHatchPrimitive2D.hxx>
#include <drawinglayer/geometry/viewinformation2d.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <drawinglayer/primitive2d/unifiedtransparenceprimitive2d.hxx>
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>
#include <toolkit/helper/vclunohelper.hxx>


namespace drawinglayer::primitive2d
{

OverlayStaticRectanglePrimitive::OverlayStaticRectanglePrimitive(
                                    const basegfx::B2DPoint& rPosition,
                                    const basegfx::B2DSize& rSize,
                                    const basegfx::BColor& rStrokeColor,
                                    const basegfx::BColor& rFillColor,
                                    double fTransparence,
                                    double fRotation)
    : maPosition(rPosition)
    , maSize(rSize)
    , maStrokeColor(rStrokeColor)
    , maFillColor(rFillColor)
    , mfTransparence(fTransparence)
    , mfRotation(fRotation)
{}

void OverlayStaticRectanglePrimitive::create2DDecomposition(Primitive2DContainer& rContainer, const geometry::ViewInformation2D& /*rViewInformation*/) const
{
    Primitive2DContainer aPrimitive2DSequence;
    const double fHalfWidth = maSize.getX() * getDiscreteUnit() / 2.0;
    const double fHalfHeight = maSize.getY() * getDiscreteUnit() / 2.0;

    basegfx::B2DRange aRange(
        maPosition.getX() - fHalfWidth, maPosition.getY() - fHalfHeight,
        maPosition.getX() + fHalfWidth, maPosition.getY() + fHalfHeight);

    if (basegfx::fTools::more(getDiscreteUnit(), 0.0) && mfTransparence <= 1.0)
    {
        basegfx::B2DPolygon aPolygon(
            basegfx::utils::createPolygonFromRect(aRange));

        // create filled primitive
        basegfx::B2DPolyPolygon aPolyPolygon;
        aPolyPolygon.append(aPolygon);

        const attribute::LineAttribute aLineAttribute(maStrokeColor, 1.0);

        // create data
        const Primitive2DReference aStroke(
            new PolyPolygonStrokePrimitive2D(aPolyPolygon, aLineAttribute));

        // create fill primitive
        const Primitive2DReference aFill(
            new PolyPolygonColorPrimitive2D(aPolyPolygon, maFillColor));

        aPrimitive2DSequence = Primitive2DContainer(2);
        aPrimitive2DSequence[0] = aFill;
        aPrimitive2DSequence[1] = aStroke;

        // embed filled to transparency (if used)
        if (mfTransparence > 0.0)
        {
            const Primitive2DReference aFillTransparent(
                new UnifiedTransparencePrimitive2D(
                    std::move(aPrimitive2DSequence),
                    mfTransparence));

            aPrimitive2DSequence = Primitive2DContainer { aFillTransparent };
        }
    }

    rContainer.insert(rContainer.end(), aPrimitive2DSequence.begin(), aPrimitive2DSequence.end());
}

bool OverlayStaticRectanglePrimitive::operator==(const BasePrimitive2D& rPrimitive) const
{
    if (DiscreteMetricDependentPrimitive2D::operator==(rPrimitive))
    {
        const OverlayStaticRectanglePrimitive& rCompare = static_cast<const OverlayStaticRectanglePrimitive&>(rPrimitive);

        return (maPosition == rCompare.maPosition
            && maSize == rCompare.maSize
            && maStrokeColor == rCompare.maStrokeColor
            && maFillColor == rCompare.maFillColor
            && mfTransparence == rCompare.mfTransparence
            && mfRotation == rCompare.mfRotation);
    }

    return false;
}

sal_uInt32 OverlayStaticRectanglePrimitive::getPrimitive2DID() const
{
    return PRIMITIVE2D_ID_OVERLAYRECTANGLEPRIMITIVE;
}



        OverlayBitmapExPrimitive::OverlayBitmapExPrimitive(
            const BitmapEx& rBitmapEx,
            const basegfx::B2DPoint& rBasePosition,
            sal_uInt16 nCenterX,
            sal_uInt16 nCenterY,
            double fShearX,
            double fRotation)
        :   maBitmapEx(rBitmapEx),
            maBasePosition(rBasePosition),
            mnCenterX(nCenterX),
            mnCenterY(nCenterY),
            mfShearX(fShearX),
            mfRotation(fRotation)
        {}

        void OverlayBitmapExPrimitive::create2DDecomposition(Primitive2DContainer& rContainer, const geometry::ViewInformation2D& /*rViewInformation*/) const
        {
            const Size aBitmapSize(getBitmapEx().GetSizePixel());

            if(!aBitmapSize.Width() || !aBitmapSize.Height() || !basegfx::fTools::more(getDiscreteUnit(), 0.0))
                return;

            // calculate back from internal bitmap's extreme coordinates (the edges)
            // to logical coordinates. Only use a unified scaling value (getDiscreteUnit(),
            // the prepared one which expresses how many logic units form a discrete unit)
            // for this step. This primitive is to be displayed always unscaled (in its pixel size)
            // and unrotated, more like a marker
            const double fLeft((0.0 - getCenterX()) * getDiscreteUnit());
            const double fTop((0.0 - getCenterY()) * getDiscreteUnit());
            const double fRight((aBitmapSize.getWidth() - getCenterX()) * getDiscreteUnit());
            const double fBottom((aBitmapSize.getHeight() - getCenterY()) * getDiscreteUnit());

            // create a BitmapPrimitive2D using those positions
            basegfx::B2DHomMatrix aTransform;

            aTransform.set(0, 0, fRight - fLeft);
            aTransform.set(1, 1, fBottom - fTop);
            aTransform.set(0, 2, fLeft);
            aTransform.set(1, 2, fTop);

            // if shearX is used, apply it, too
            if(!basegfx::fTools::equalZero(getShearX()))
            {
                aTransform.shearX(getShearX());
            }

            // if rotation is used, apply it, too
            if(!basegfx::fTools::equalZero(getRotation()))
            {
                aTransform.rotate(getRotation());
            }

            // add BasePosition
            aTransform.translate(getBasePosition().getX(), getBasePosition().getY());

            rContainer.push_back(
                new BitmapPrimitive2D(
                    VCLUnoHelper::CreateVCLXBitmap(getBitmapEx()),
                    aTransform));
        }

        bool OverlayBitmapExPrimitive::operator==( const BasePrimitive2D& rPrimitive ) const
        {
            if(DiscreteMetricDependentPrimitive2D::operator==(rPrimitive))
            {
                const OverlayBitmapExPrimitive& rCompare = static_cast< const OverlayBitmapExPrimitive& >(rPrimitive);

                return (getBitmapEx() == rCompare.getBitmapEx()
                    && getBasePosition() == rCompare.getBasePosition()
                    && getCenterX() == rCompare.getCenterX()
                    && getCenterY() == rCompare.getCenterY()
                    && getShearX() == rCompare.getShearX()
                    && getRotation() == rCompare.getRotation());
            }

            return false;
        }

        sal_uInt32 OverlayBitmapExPrimitive::getPrimitive2DID() const
        {
            return PRIMITIVE2D_ID_OVERLAYBITMAPEXPRIMITIVE;
        }



        OverlayCrosshairPrimitive::OverlayCrosshairPrimitive(
            const basegfx::B2DPoint& rBasePosition,
            const basegfx::BColor& rRGBColorA,
            const basegfx::BColor& rRGBColorB,
            double fDiscreteDashLength)
        :   maBasePosition(rBasePosition),
            maRGBColorA(rRGBColorA),
            maRGBColorB(rRGBColorB),
            mfDiscreteDashLength(fDiscreteDashLength)
        {}

        void OverlayCrosshairPrimitive::create2DDecomposition(Primitive2DContainer& rContainer, const geometry::ViewInformation2D& /*rViewInformation*/) const
        {
            // use the prepared Viewport information accessible using getViewport()

            if(getViewport().isEmpty())
                return;

            basegfx::B2DPolygon aPolygon;

            aPolygon.append(basegfx::B2DPoint(getViewport().getMinX(), getBasePosition().getY()));
            aPolygon.append(basegfx::B2DPoint(getViewport().getMaxX(), getBasePosition().getY()));

            rContainer.push_back(
                new PolygonMarkerPrimitive2D(
                    aPolygon,
                    getRGBColorA(),
                    getRGBColorB(),
                    getDiscreteDashLength()));

            aPolygon.clear();
            aPolygon.append(basegfx::B2DPoint(getBasePosition().getX(), getViewport().getMinY()));
            aPolygon.append(basegfx::B2DPoint(getBasePosition().getX(), getViewport().getMaxY()));

            rContainer.push_back(
                new PolygonMarkerPrimitive2D(
                    aPolygon,
                    getRGBColorA(),
                    getRGBColorB(),
                    getDiscreteDashLength()));
        }

        bool OverlayCrosshairPrimitive::operator==( const BasePrimitive2D& rPrimitive ) const
        {
            if(ViewportDependentPrimitive2D::operator==(rPrimitive))
            {
                const OverlayCrosshairPrimitive& rCompare = static_cast< const OverlayCrosshairPrimitive& >(rPrimitive);

                return (getBasePosition() == rCompare.getBasePosition()
                    && getRGBColorA() == rCompare.getRGBColorA()
                    && getRGBColorB() == rCompare.getRGBColorB()
                    && getDiscreteDashLength() == rCompare.getDiscreteDashLength());
            }

            return false;
        }

        sal_uInt32 OverlayCrosshairPrimitive::getPrimitive2DID() const
        {
            return PRIMITIVE2D_ID_OVERLAYCROSSHAIRPRIMITIVE;
        }



        OverlayRectanglePrimitive::OverlayRectanglePrimitive(
            const basegfx::B2DRange& rObjectRange,
            const basegfx::BColor& rColor,
            double fTransparence,
            double fDiscreteGrow,
            double fDiscreteShrink,
            double fRotation)
        :   maObjectRange(rObjectRange),
            maColor(rColor),
            mfTransparence(fTransparence),
            mfDiscreteGrow(fDiscreteGrow),
            mfDiscreteShrink(fDiscreteShrink),
            mfRotation(fRotation)
        {}

        void OverlayRectanglePrimitive::create2DDecomposition(Primitive2DContainer& rContainer, const geometry::ViewInformation2D& /*rViewInformation*/) const
        {
            Primitive2DContainer aRetval;
            basegfx::B2DRange aInnerRange(getObjectRange());

            if(!aInnerRange.isEmpty() && basegfx::fTools::more(getDiscreteUnit(), 0.0) && getTransparence() <= 1.0)
            {
                basegfx::B2DRange aOuterRange(getObjectRange());

                // grow/shrink inner/outer polygons
                aOuterRange.grow(getDiscreteUnit() * getDiscreteGrow());
                aInnerRange.grow(getDiscreteUnit() * -getDiscreteShrink());

                // convert to polygons
                const double fFullGrow(getDiscreteGrow() + getDiscreteShrink());
                const double fRelativeRadiusX(fFullGrow / aOuterRange.getWidth());
                const double fRelativeRadiusY(fFullGrow / aOuterRange.getHeight());
                basegfx::B2DPolygon aOuterPolygon(
                    basegfx::utils::createPolygonFromRect(
                        aOuterRange,
                        fRelativeRadiusX,
                        fRelativeRadiusY));
                basegfx::B2DPolygon aInnerPolygon(
                    basegfx::utils::createPolygonFromRect(
                        aInnerRange));

                // apply evtl. existing rotation
                if(!basegfx::fTools::equalZero(getRotation()))
                {
                    const basegfx::B2DHomMatrix aTransform(basegfx::utils::createRotateAroundPoint(
                        getObjectRange().getMinX(), getObjectRange().getMinY(), getRotation()));

                    aOuterPolygon.transform(aTransform);
                    aInnerPolygon.transform(aTransform);
                }

                // create filled primitive
                basegfx::B2DPolyPolygon aPolyPolygon;

                aPolyPolygon.append(aOuterPolygon);
                aPolyPolygon.append(aInnerPolygon);

                if(Application::GetSettings().GetStyleSettings().GetHighContrastMode())
                {
                    // for high contrast, use hatch
                    const basegfx::BColor aHighContrastLineColor(Application::GetSettings().GetStyleSettings().GetFontColor().getBColor());
                    const basegfx::BColor aEmptyColor(0.0, 0.0, 0.0);
                    const double fHatchRotation(basegfx::deg2rad(45));
                    const double fDiscreteHatchDistance(3.0);
                    const drawinglayer::attribute::FillHatchAttribute aFillHatchAttribute(
                        drawinglayer::attribute::HatchStyle::Single,
                        fDiscreteHatchDistance * getDiscreteUnit(),
                        fHatchRotation - getRotation(),
                        aHighContrastLineColor,
                        3, // same default as VCL, a minimum of three discrete units (pixels) offset
                        false);
                    const Primitive2DReference aHatch(
                        new PolyPolygonHatchPrimitive2D(
                            aPolyPolygon,
                            aEmptyColor,
                            aFillHatchAttribute));

                    aRetval = Primitive2DContainer { aHatch };
                }
                else
                {
                    // create fill primitive
                    const Primitive2DReference aFill(
                        new PolyPolygonColorPrimitive2D(
                            aPolyPolygon,
                            getColor()));

                    aRetval = Primitive2DContainer { aFill };

                    // embed filled to transparency (if used)
                    if(getTransparence() > 0.0)
                    {
                        Primitive2DReference aFillTransparent(
                            new UnifiedTransparencePrimitive2D(
                                std::move(aRetval),
                                getTransparence()));

                        aRetval = Primitive2DContainer { aFillTransparent };
                    }
                }
            }

            rContainer.insert(rContainer.end(), aRetval.begin(), aRetval.end());
        }

        bool OverlayRectanglePrimitive::operator==( const BasePrimitive2D& rPrimitive ) const
        {
            if(DiscreteMetricDependentPrimitive2D::operator==(rPrimitive))
            {
                const OverlayRectanglePrimitive& rCompare = static_cast< const OverlayRectanglePrimitive& >(rPrimitive);

                return (getObjectRange() == rCompare.getObjectRange()
                    && getColor() == rCompare.getColor()
                    && getTransparence() == rCompare.getTransparence()
                    && getDiscreteGrow() == rCompare.getDiscreteGrow()
                    && getDiscreteShrink() == rCompare.getDiscreteShrink()
                    && getRotation() == rCompare.getRotation());
            }

            return false;
        }

        sal_uInt32 OverlayRectanglePrimitive::getPrimitive2DID() const
        {
            return PRIMITIVE2D_ID_OVERLAYRECTANGLEPRIMITIVE;
        }



        OverlayHelplineStripedPrimitive::OverlayHelplineStripedPrimitive(
            const basegfx::B2DPoint& rBasePosition,
            HelplineStyle eStyle,
            const basegfx::BColor& rRGBColorA,
            const basegfx::BColor& rRGBColorB,
            double fDiscreteDashLength)
        :   maBasePosition(rBasePosition),
            meStyle(eStyle),
            maRGBColorA(rRGBColorA),
            maRGBColorB(rRGBColorB),
            mfDiscreteDashLength(fDiscreteDashLength)
        {}

        void OverlayHelplineStripedPrimitive::create2DDecomposition(Primitive2DContainer& rContainer, const geometry::ViewInformation2D& rViewInformation) const
        {
            // use the prepared Viewport information accessible using getViewport()

            if(getViewport().isEmpty())
                return;

            switch(getStyle())
            {
                case HELPLINESTYLE_VERTICAL :
                {
                    basegfx::B2DPolygon aLine;

                    aLine.append(basegfx::B2DPoint(getBasePosition().getX(), getViewport().getMinY()));
                    aLine.append(basegfx::B2DPoint(getBasePosition().getX(), getViewport().getMaxY()));

                    rContainer.push_back(
                        new PolygonMarkerPrimitive2D(
                            aLine,
                            getRGBColorA(),
                            getRGBColorB(),
                            getDiscreteDashLength()));
                    break;
                }

                case HELPLINESTYLE_HORIZONTAL :
                {
                    basegfx::B2DPolygon aLine;

                    aLine.append(basegfx::B2DPoint(getViewport().getMinX(), getBasePosition().getY()));
                    aLine.append(basegfx::B2DPoint(getViewport().getMaxX(), getBasePosition().getY()));

                    rContainer.push_back(
                        new PolygonMarkerPrimitive2D(
                            aLine,
                            getRGBColorA(),
                            getRGBColorB(),
                            getDiscreteDashLength()));
                    break;
                }

                default: // case HELPLINESTYLE_POINT :
                {
                    const double fDiscreteUnit((rViewInformation.getInverseObjectToViewTransformation() * basegfx::B2DVector(1.0, 0.0)).getLength());
                    basegfx::B2DPolygon aLineA, aLineB;

                    aLineA.append(basegfx::B2DPoint(getBasePosition().getX(), getBasePosition().getY() - fDiscreteUnit));
                    aLineA.append(basegfx::B2DPoint(getBasePosition().getX(), getBasePosition().getY() + fDiscreteUnit));

                    rContainer.push_back(
                        new PolygonMarkerPrimitive2D(
                            aLineA,
                            getRGBColorA(),
                            getRGBColorB(),
                            getDiscreteDashLength()));

                    aLineB.append(basegfx::B2DPoint(getBasePosition().getX() - fDiscreteUnit, getBasePosition().getY()));
                    aLineB.append(basegfx::B2DPoint(getBasePosition().getX() + fDiscreteUnit, getBasePosition().getY()));

                    rContainer.push_back(
                        new PolygonMarkerPrimitive2D(
                            aLineB,
                            getRGBColorA(),
                            getRGBColorB(),
                            getDiscreteDashLength()));

                    break;
                }
            }
        }

        bool OverlayHelplineStripedPrimitive::operator==( const BasePrimitive2D& rPrimitive ) const
        {
            if(ViewportDependentPrimitive2D::operator==(rPrimitive))
            {
                const OverlayHelplineStripedPrimitive& rCompare = static_cast< const OverlayHelplineStripedPrimitive& >(rPrimitive);

                return (getBasePosition() == rCompare.getBasePosition()
                    && getStyle() == rCompare.getStyle()
                    && getRGBColorA() == rCompare.getRGBColorA()
                    && getRGBColorB() == rCompare.getRGBColorB()
                    && getDiscreteDashLength() == rCompare.getDiscreteDashLength());
            }

            return false;
        }

        sal_uInt32 OverlayHelplineStripedPrimitive::getPrimitive2DID() const
        {
            return PRIMITIVE2D_ID_OVERLAYHELPLINESTRIPEDPRIMITIVE;
        }



        OverlayRollingRectanglePrimitive::OverlayRollingRectanglePrimitive(
            const basegfx::B2DRange& aRollingRectangle,
            const basegfx::BColor& rRGBColorA,
            const basegfx::BColor& rRGBColorB,
            double fDiscreteDashLength)
        :   maRollingRectangle(aRollingRectangle),
            maRGBColorA(rRGBColorA),
            maRGBColorB(rRGBColorB),
            mfDiscreteDashLength(fDiscreteDashLength)
        {}

        void OverlayRollingRectanglePrimitive::create2DDecomposition(Primitive2DContainer& rContainer, const geometry::ViewInformation2D& /*rViewInformation*/) const
        {
            // use the prepared Viewport information accessible using getViewport()

            if(getViewport().isEmpty())
                return;

            basegfx::B2DPolygon aLine;

            // Left lines
            aLine.append(basegfx::B2DPoint(getViewport().getMinX(), getRollingRectangle().getMinY()));
            aLine.append(basegfx::B2DPoint(getRollingRectangle().getMinX(), getRollingRectangle().getMinY()));
            rContainer.push_back(new PolygonMarkerPrimitive2D(aLine, getRGBColorA(), getRGBColorB(), getDiscreteDashLength()));

            aLine.clear();
            aLine.append(basegfx::B2DPoint(getViewport().getMinX(), getRollingRectangle().getMaxY()));
            aLine.append(basegfx::B2DPoint(getRollingRectangle().getMinX(), getRollingRectangle().getMaxY()));
            rContainer.push_back(new PolygonMarkerPrimitive2D(aLine, getRGBColorA(), getRGBColorB(), getDiscreteDashLength()));

            // Right lines
            aLine.clear();
            aLine.append(basegfx::B2DPoint(getRollingRectangle().getMaxX(), getRollingRectangle().getMinY()));
            aLine.append(basegfx::B2DPoint(getViewport().getMaxX(), getRollingRectangle().getMinY()));
            rContainer.push_back(new PolygonMarkerPrimitive2D(aLine, getRGBColorA(), getRGBColorB(), getDiscreteDashLength()));

            aLine.clear();
            aLine.append(basegfx::B2DPoint(getRollingRectangle().getMaxX(), getRollingRectangle().getMaxY()));
            aLine.append(basegfx::B2DPoint(getViewport().getMaxX(), getRollingRectangle().getMaxY()));
            rContainer.push_back(new PolygonMarkerPrimitive2D(aLine, getRGBColorA(), getRGBColorB(), getDiscreteDashLength()));

            // Top lines
            aLine.clear();
            aLine.append(basegfx::B2DPoint(getRollingRectangle().getMinX(), getViewport().getMinY()));
            aLine.append(basegfx::B2DPoint(getRollingRectangle().getMinX(), getRollingRectangle().getMinY()));
            rContainer.push_back(new PolygonMarkerPrimitive2D(aLine, getRGBColorA(), getRGBColorB(), getDiscreteDashLength()));

            aLine.clear();
            aLine.append(basegfx::B2DPoint(getRollingRectangle().getMaxX(), getViewport().getMinY()));
            aLine.append(basegfx::B2DPoint(getRollingRectangle().getMaxX(), getRollingRectangle().getMinY()));
            rContainer.push_back(new PolygonMarkerPrimitive2D(aLine, getRGBColorA(), getRGBColorB(), getDiscreteDashLength()));

            // Bottom lines
            aLine.clear();
            aLine.append(basegfx::B2DPoint(getRollingRectangle().getMinX(), getRollingRectangle().getMaxY()));
            aLine.append(basegfx::B2DPoint(getRollingRectangle().getMinX(), getViewport().getMaxY()));
            rContainer.push_back(new PolygonMarkerPrimitive2D(aLine, getRGBColorA(), getRGBColorB(), getDiscreteDashLength()));

            aLine.clear();
            aLine.append(basegfx::B2DPoint(getRollingRectangle().getMaxX(), getRollingRectangle().getMaxY()));
            aLine.append(basegfx::B2DPoint(getRollingRectangle().getMaxX(), getViewport().getMaxY()));
            rContainer.push_back(new PolygonMarkerPrimitive2D(aLine, getRGBColorA(), getRGBColorB(), getDiscreteDashLength()));

        }

        bool OverlayRollingRectanglePrimitive::operator==( const BasePrimitive2D& rPrimitive ) const
        {
            if(ViewportDependentPrimitive2D::operator==(rPrimitive))
            {
                const OverlayRollingRectanglePrimitive& rCompare = static_cast< const OverlayRollingRectanglePrimitive& >(rPrimitive);

                return (getRollingRectangle() == rCompare.getRollingRectangle()
                    && getRGBColorA() == rCompare.getRGBColorA()
                    && getRGBColorB() == rCompare.getRGBColorB()
                    && getDiscreteDashLength() == rCompare.getDiscreteDashLength());
            }

            return false;
        }

        sal_uInt32 OverlayRollingRectanglePrimitive::getPrimitive2DID() const
        {
            return PRIMITIVE2D_ID_OVERLAYROLLINGRECTANGLEPRIMITIVE;
        }

} // end of namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
