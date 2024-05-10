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

#include <svx/diagram/IDiagramHelper.hxx>
#include <svx/svdogrp.hxx>
#include <svx/svdhdl.hxx>
#include <svx/svdmrkv.hxx>
#include <svx/svdpagv.hxx>
#include <svx/sdrpagewindow.hxx>
#include <svx/sdrpaintwindow.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <svx/sdr/primitive2d/svx_primitivetypes2d.hxx>
#include <drawinglayer/primitive2d/primitivetools2d.hxx>
#include <svx/sdr/overlay/overlaymanager.hxx>
#include <drawinglayer/attribute/lineattribute.hxx>
#include <drawinglayer/primitive2d/PolyPolygonStrokePrimitive2D.hxx>
#include <drawinglayer/primitive2d/PolygonStrokePrimitive2D.hxx>
#include <drawinglayer/primitive2d/PolyPolygonColorPrimitive2D.hxx>
#include <drawinglayer/primitive2d/groupprimitive2d.hxx>
#include <comphelper/dispatchcommand.hxx>
#include <drawinglayer/primitive2d/textlayoutdevice.hxx>
#include <officecfg/Office/Common.hxx>
#include <svx/strings.hrc>
#include <svx/dialmgr.hxx>

namespace {

// helper to create the geometry for a rounded polygon, maybe
// containing a Lap positioned inside top-left for some text
basegfx::B2DPolygon createRoundedPolygon(
    const basegfx::B2DRange& rRange,
    double fDistance,
    bool bCreateLap,
    double fTextWidth)
{
    basegfx::B2DPolygon aRetval;

    // TopLeft rounded edge
    aRetval.append(
        basegfx::utils::createPolygonFromEllipseSegment(
            basegfx::B2DPoint(rRange.getMinX(), rRange.getMinY()),
            fDistance,
            fDistance,
            M_PI * 1.0,
            M_PI * 1.5));

    // create Lap topLeft inside
    if(bCreateLap)
    {
        const double fLapLeft(rRange.getMinX() + fDistance);
        double fLapRight(rRange.getMinX() + (rRange.getWidth() * 0.5)  - fDistance);
        const double fLapTop(rRange.getMinY() - fDistance);
        const double fLapBottom(fLapTop + (fDistance * 2.0));
        const double fExtendedTextWidth(fTextWidth + (fDistance * 3.0));

        if(0.0 != fExtendedTextWidth && fLapLeft + fExtendedTextWidth < fLapRight)
        {
            fLapRight = fLapLeft + fExtendedTextWidth;
        }

        aRetval.append(basegfx::B2DPoint(fLapLeft, fLapTop));
        aRetval.append(basegfx::B2DPoint(fLapLeft + (fDistance * 0.5), fLapBottom));
        aRetval.append(basegfx::B2DPoint(fLapRight - (fDistance * 0.5), fLapBottom));
        aRetval.append(basegfx::B2DPoint(fLapRight, fLapTop));
    }

    // TopRight rounded edge
    aRetval.append(
        basegfx::utils::createPolygonFromEllipseSegment(
            basegfx::B2DPoint(rRange.getMaxX(), rRange.getMinY()),
            fDistance,
            fDistance,
            M_PI * 1.5,
            M_PI * 0.0));

    // BottomRight rounded edge
    aRetval.append(
        basegfx::utils::createPolygonFromEllipseSegment(
            basegfx::B2DPoint(rRange.getMaxX(), rRange.getMaxY()),
            fDistance,
            fDistance,
            M_PI * 0.0,
            M_PI * 0.5));

    // BottomLeft rounded edge
    aRetval.append(
        basegfx::utils::createPolygonFromEllipseSegment(
            basegfx::B2DPoint(rRange.getMinX(), rRange.getMaxY()),
            fDistance,
            fDistance,
            M_PI * 0.5,
            M_PI * 1.0));

    aRetval.setClosed(true);

    return aRetval;
}

// helper primitive to create/show the overlay geometry for a DynamicDiagram
class OverlayDiagramPrimitive final : public drawinglayer::primitive2d::DiscreteMetricDependentPrimitive2D
{
private:
    basegfx::B2DHomMatrix maTransformation;  // object dimensions
    double mfDiscreteDistance; // distance from object in pixels
    double mfDiscreteGap; // gap/width of visualization in pixels
    Color maColor;  // base color (made lighter/darker as needed, should be system selection color)

    virtual drawinglayer::primitive2d::Primitive2DReference create2DDecomposition(
        const drawinglayer::geometry::ViewInformation2D& rViewInformation) const override;

public:
    OverlayDiagramPrimitive(
        const basegfx::B2DHomMatrix& rTransformation,
        double fDiscreteDistance,
        double fDiscreteGap,
        Color const & rColor);

    virtual sal_uInt32 getPrimitive2DID() const override;
};

drawinglayer::primitive2d::Primitive2DReference OverlayDiagramPrimitive::create2DDecomposition(
    const drawinglayer::geometry::ViewInformation2D& /*rViewInformation*/) const
{
    // get the dimensions. Do *not* take rotation/shear into account,
    // this is intended to be a pure expanded/frame visualization as
    // needed in UI for simplified visualization
    basegfx::B2DRange aRange(0.0, 0.0, 1.0, 1.0);
    aRange.transform(maTransformation);

    basegfx::B2DPolyPolygon aPolyPolygon;
    const double fInnerDistance(mfDiscreteDistance * getDiscreteUnit());
    const double fOuterDistance((mfDiscreteDistance + mfDiscreteGap) * getDiscreteUnit());
    bool bCreateLap(true);
    basegfx::B2DPolyPolygon aTextAsPolyPolygon;
    double fTextWidth(0.0);

    // initially try to create lap
    if(bCreateLap)
    {
        // take a resource text (for now existing one that fits)
        const OUString aName(SvxResId(RID_STR_DATANAV_EDIT_ELEMENT));
        drawinglayer::primitive2d::TextLayouterDevice aTextLayouter;
        basegfx::B2DPolyPolygonVector aTarget;
        std::vector<double> aDXArray;

        // to simplify things for now, do not create a TextSimplePortionPrimitive2D
        // and needed FontAttribute, just get the TextOutlines as geometry
        aTextLayouter.getTextOutlines(
            aTarget,
            aName,
            0,
            aName.getLength(),
            aDXArray,
            {});

        // put into one PolyPolygon (also simplification - overlapping chars
        // may create XOR gaps, so these exist for a reason, but low probability)
        for (auto const& elem : aTarget)
        {
            aTextAsPolyPolygon.append(elem);
        }

        // get text dimensions & transform to destination
        const basegfx::B2DRange aTextRange(aTextAsPolyPolygon.getB2DRange());
        basegfx::B2DHomMatrix aTextTransform;

        aTextTransform.translate(aTextRange.getMinX(), aTextRange.getMinY());
        const double fTargetTextHeight((mfDiscreteDistance + mfDiscreteGap - 2.0) * getDiscreteUnit());
        const double fTextScale(fTargetTextHeight / aTextRange.getHeight());
        aTextTransform.scale(fTextScale, fTextScale);
        aTextTransform.translate(
            aRange.getMinX() + (fInnerDistance * 2.0),
            aRange.getMinY() + fTargetTextHeight + (fOuterDistance - fInnerDistance) - (2.0 * getDiscreteUnit()));
        aTextAsPolyPolygon.transform(aTextTransform);

        // check text size/position
        fTextWidth = aTextRange.getWidth() * fTextScale;
        const double fLapLeft(aRange.getMinX() + fInnerDistance);
        const double fLapRight(aRange.getMinX() + (aRange.getWidth() * 0.5)  - fInnerDistance);

        // if text is too big, do not create a Lap at all
        // to avoid trouble. It is expected that the user keeps
        // the object he works with big enough to do useful actions
        if(fTextWidth + (4.0 * getDiscreteUnit()) > fLapRight - fLapLeft)
            bCreateLap = false;
    }

    // create outer polygon
    aPolyPolygon.append(
        createRoundedPolygon(
            aRange,
            fOuterDistance,
            false,
            0.0));

    // create inner polygon, maybe with Lap
    aPolyPolygon.append(
        createRoundedPolygon(
            aRange,
            fInnerDistance,
            bCreateLap,
            fTextWidth));

    Color aFillColor(maColor);
    Color aLineColor(maColor);

    aFillColor.IncreaseLuminance(10);
    aLineColor.DecreaseLuminance(30);

    const drawinglayer::attribute::LineAttribute aLineAttribute(
        aLineColor.getBColor(),
        1.0 * getDiscreteUnit());

    drawinglayer::primitive2d::Primitive2DContainer aContainer;

    // filled polygon as BG (may get transparence for better look ?)
    aContainer.push_back(
        new drawinglayer::primitive2d::PolyPolygonColorPrimitive2D(
            aPolyPolygon,
            aFillColor.getBColor()));

    // outline polygon for visibility (may be accentuated shaded
    // top/left, would require alternative creation)
    aContainer.push_back(
        new drawinglayer::primitive2d::PolyPolygonStrokePrimitive2D(
            std::move(aPolyPolygon),
            aLineAttribute));

    // top-left line pattern (as grep-here-sign to signal
    // that this construct may be also dragged by the user)
    const double fLapLeft(aRange.getMinX() + fInnerDistance);
    const double fLapRight(aRange.getMinX() + (aRange.getWidth() * 0.5)  - fInnerDistance);
    const double  fLapUp(aRange.getMinY() - ((mfDiscreteDistance + mfDiscreteDistance * 0.666) * getDiscreteUnit()));
    const double  fLapDown(aRange.getMinY() - ((mfDiscreteDistance + mfDiscreteDistance * 0.333) * getDiscreteUnit()));
    basegfx::B2DPolygon aPolygonLapUp;
    aPolygonLapUp.append(basegfx::B2DPoint(fLapLeft, fLapUp));
    aPolygonLapUp.append(basegfx::B2DPoint(fLapRight, fLapUp));
    basegfx::B2DPolygon aPolygonLapDown;
    aPolygonLapDown.append(basegfx::B2DPoint(fLapLeft, fLapDown));
    aPolygonLapDown.append(basegfx::B2DPoint(fLapRight, fLapDown));
    drawinglayer::attribute::StrokeAttribute aStrokeAttribute({ 2.0 * getDiscreteUnit(), 2.0 * getDiscreteUnit() });

    aContainer.push_back(
        new drawinglayer::primitive2d::PolygonStrokePrimitive2D(
            std::move(aPolygonLapUp),
            aLineAttribute,
            aStrokeAttribute));

    aContainer.push_back(
        new drawinglayer::primitive2d::PolygonStrokePrimitive2D(
            std::move(aPolygonLapDown),
            aLineAttribute,
            std::move(aStrokeAttribute)));

    // add text last. May use darker text color, go for same color
    // as accentuation line for now
    if(bCreateLap && 0 != aTextAsPolyPolygon.count())
    {
        aContainer.push_back(
            new drawinglayer::primitive2d::PolyPolygonColorPrimitive2D(
                std::move(aTextAsPolyPolygon),
                aLineColor.getBColor()));
    }
    return new drawinglayer::primitive2d::GroupPrimitive2D(std::move(aContainer));
}

OverlayDiagramPrimitive::OverlayDiagramPrimitive(
    const basegfx::B2DHomMatrix& rTransformation,
    double fDiscreteDistance,
    double fDiscreteGap,
    Color const & rColor)
: drawinglayer::primitive2d::DiscreteMetricDependentPrimitive2D()
, maTransformation(rTransformation)
, mfDiscreteDistance(fDiscreteDistance)
, mfDiscreteGap(fDiscreteGap)
, maColor(rColor)
{
}

sal_uInt32 OverlayDiagramPrimitive::getPrimitive2DID() const
{
    return PRIMITIVE2D_ID_OVERLAYDIAGRAMPRIMITIVE2D;
}

// helper object for DiagramOverlay
class OverlayDiagramFrame final : public sdr::overlay::OverlayObject
{
private:
    basegfx::B2DHomMatrix maTransformation; // object dimensions
    Color maColor; // base color

    virtual drawinglayer::primitive2d::Primitive2DContainer createOverlayObjectPrimitive2DSequence() override;

public:
    explicit OverlayDiagramFrame(
        const basegfx::B2DHomMatrix& rTransformation,
        Color const & rColor);
};

OverlayDiagramFrame::OverlayDiagramFrame(
    const basegfx::B2DHomMatrix& rTransformation,
    const Color& rColor)
: sdr::overlay::OverlayObject(rColor)
, maTransformation(rTransformation)
, maColor(rColor)
{
}

drawinglayer::primitive2d::Primitive2DContainer OverlayDiagramFrame::createOverlayObjectPrimitive2DSequence()
{
    drawinglayer::primitive2d::Primitive2DContainer aReturnContainer;

    if ( !officecfg::Office::Common::Misc::ExperimentalMode::get() )
        return aReturnContainer;

    if (getOverlayManager())
    {
        aReturnContainer = drawinglayer::primitive2d::Primitive2DContainer {
            new OverlayDiagramPrimitive(
                maTransformation,
                8.0, // distance from geometry in pixels
                8.0, // gap/width of visualization in pixels
                maColor) };
    }

    return aReturnContainer;
}

} // end of anonymous namespace

namespace svx { namespace diagram {

void DiagramFrameHdl::clicked(const Point& /*rPnt*/)
{
    // this may check for a direct hit at the text later
    // and only then take action. That would require
    // to evaluate & keep that (maybe during creation).
    // For now, just trigger to open the Dialog
    comphelper::dispatchCommand(u".uno:EditDiagram"_ustr, {});
}

void DiagramFrameHdl::CreateB2dIAObject()
{
    // first throw away old one
    GetRidOfIAObject();

    SdrMarkView* pView = m_pHdlList->GetView();

    if(!pView || pView->areMarkHandlesHidden())
        return;

    SdrPageView* pPageView = pView->GetSdrPageView();

    if(!pPageView)
        return;

    for(sal_uInt32 b(0); b < pPageView->PageWindowCount(); b++)
    {
        const SdrPageWindow& rPageWindow = *pPageView->GetPageWindow(b);

        if(rPageWindow.GetPaintWindow().OutputToWindow())
        {
            const rtl::Reference< sdr::overlay::OverlayManager >& xManager = rPageWindow.GetOverlayManager();
            if (xManager.is())
            {
                OutputDevice& rOutDev(rPageWindow.GetPaintWindow().GetOutputDevice());
                const StyleSettings& rStyles(rOutDev.GetSettings().GetStyleSettings());
                Color aFillColor(rStyles.GetHighlightColor());
                std::unique_ptr<sdr::overlay::OverlayObject> pNewOverlayObject(
                    new OverlayDiagramFrame(
                        maTransformation,
                        aFillColor));

                // OVERLAYMANAGER
                insertNewlyCreatedOverlayObjectForSdrHdl(
                    std::move(pNewOverlayObject),
                    rPageWindow.GetObjectContact(),
                    *xManager);
            }
        }
    }
}

DiagramFrameHdl::DiagramFrameHdl(const basegfx::B2DHomMatrix& rTransformation)
: SdrHdl(Point(), SdrHdlKind::Move)
, maTransformation(rTransformation)
{
}

IDiagramHelper::IDiagramHelper()
: mbUseDiagramThemeData(false)
, mbUseDiagramModelData(true)
, mbForceThemePtrRecreation(false)
{
}

IDiagramHelper::~IDiagramHelper() {}

void IDiagramHelper::anchorToSdrObjGroup(SdrObjGroup& rTarget)
{
    rTarget.mp_DiagramHelper.reset(this);
}

void IDiagramHelper::AddAdditionalVisualization(const SdrObjGroup& rTarget, SdrHdlList& rHdlList)
{
    // create an extra frame visualization here
    basegfx::B2DHomMatrix aTransformation;
    basegfx::B2DPolyPolygon aPolyPolygon;
    rTarget.TRGetBaseGeometry(aTransformation, aPolyPolygon);

    std::unique_ptr<SdrHdl> pHdl(new DiagramFrameHdl(aTransformation));
    rHdlList.AddHdl(std::move(pHdl));
}

}} // end of namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
