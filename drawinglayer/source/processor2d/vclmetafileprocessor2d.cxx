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

#include <cmath>
#include <memory>
#include "vclmetafileprocessor2d.hxx"
#include "vclpixelprocessor2d.hxx"
#include <rtl/ustring.hxx>
#include <tools/gen.hxx>
#include <tools/stream.hxx>
#include <comphelper/diagnose_ex.hxx>
#include <comphelper/flagguard.hxx>
#include <comphelper/processfactory.hxx>
#include <config_global.h>
#include <basegfx/polygon/b2dpolygonclipper.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/polygon/b2dlinegeometry.hxx>
#include <basegfx/utils/gradienttools.hxx>
#include <vcl/virdev.hxx>
#include <vcl/gdimtf.hxx>
#include <vcl/gradient.hxx>
#include <vcl/graphictools.hxx>
#include <vcl/metaact.hxx>
#include <vcl/graph.hxx> // for PDFExtOutDevData Graphic support
#include <vcl/formpdfexport.hxx> // for PDFExtOutDevData Graphic support
#include <drawinglayer/primitive2d/drawinglayer_primitivetypes2d.hxx>
#include <drawinglayer/primitive2d/textprimitive2d.hxx>
#include <drawinglayer/primitive2d/PolygonHairlinePrimitive2D.hxx>
#include <drawinglayer/primitive2d/PolygonStrokeArrowPrimitive2D.hxx>
#include <drawinglayer/primitive2d/PolyPolygonColorPrimitive2D.hxx>
#include <drawinglayer/primitive2d/PolyPolygonGradientPrimitive2D.hxx>
#include <drawinglayer/primitive2d/PolyPolygonHatchPrimitive2D.hxx>
#include <drawinglayer/primitive2d/PolyPolygonGraphicPrimitive2D.hxx>
#include <drawinglayer/primitive2d/bitmapprimitive2d.hxx>
#include <drawinglayer/primitive2d/maskprimitive2d.hxx>
#include <drawinglayer/primitive2d/modifiedcolorprimitive2d.hxx>
#include <drawinglayer/primitive2d/unifiedtransparenceprimitive2d.hxx>
#include <drawinglayer/primitive2d/transparenceprimitive2d.hxx>
#include <drawinglayer/primitive2d/fillgradientprimitive2d.hxx>
#include <drawinglayer/primitive2d/transformprimitive2d.hxx>
#include <drawinglayer/primitive2d/markerarrayprimitive2d.hxx>
#include <drawinglayer/primitive2d/pointarrayprimitive2d.hxx>
#include <drawinglayer/primitive2d/texthierarchyprimitive2d.hxx>
#include <drawinglayer/primitive2d/controlprimitive2d.hxx>
#include <drawinglayer/primitive2d/graphicprimitive2d.hxx>
#include <drawinglayer/primitive2d/pagepreviewprimitive2d.hxx>
#include <drawinglayer/primitive2d/epsprimitive2d.hxx>
#include <drawinglayer/primitive2d/structuretagprimitive2d.hxx>
#include <drawinglayer/primitive2d/objectinfoprimitive2d.hxx> // for Title/Description metadata
#include <drawinglayer/converters.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <tools/vcompat.hxx>

#include <com/sun/star/awt/XControl.hpp>
#include <com/sun/star/i18n/BreakIterator.hpp>
#include <com/sun/star/i18n/CharacterIteratorMode.hpp>
#include <com/sun/star/i18n/WordType.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>

using namespace com::sun::star;

// #112245# definition for maximum allowed point count due to Metafile target.
// To be on the safe side with the old tools polygon, use slightly less than
// the theoretical maximum (bad experiences with tools polygon)

#define MAX_POLYGON_POINT_COUNT_METAFILE (0x0000fff0)

namespace
{
// #112245# helper to split line polygon in half
void splitLinePolygon(const basegfx::B2DPolygon& rBasePolygon, basegfx::B2DPolygon& o_aLeft,
                      basegfx::B2DPolygon& o_aRight)
{
    const sal_uInt32 nCount(rBasePolygon.count());

    if (nCount)
    {
        const sal_uInt32 nHalfCount((nCount - 1) >> 1);

        o_aLeft = basegfx::B2DPolygon(rBasePolygon, 0, nHalfCount + 1);
        o_aLeft.setClosed(false);

        o_aRight = basegfx::B2DPolygon(rBasePolygon, nHalfCount, nCount - nHalfCount);
        o_aRight.setClosed(false);

        if (rBasePolygon.isClosed())
        {
            o_aRight.append(rBasePolygon.getB2DPoint(0));

            if (rBasePolygon.areControlPointsUsed())
            {
                o_aRight.setControlPoints(o_aRight.count() - 1, rBasePolygon.getPrevControlPoint(0),
                                          rBasePolygon.getNextControlPoint(0));
            }
        }
    }
    else
    {
        o_aLeft.clear();
        o_aRight.clear();
    }
}

// #112245# helper to evtl. split filled polygons to maximum metafile point count
void fillPolyPolygonNeededToBeSplit(basegfx::B2DPolyPolygon& rPolyPolygon)
{
    const sal_uInt32 nPolyCount(rPolyPolygon.count());

    if (!nPolyCount)
        return;

    basegfx::B2DPolyPolygon aSplitted;

    for (sal_uInt32 a(0); a < nPolyCount; a++)
    {
        const basegfx::B2DPolygon& aCandidate(rPolyPolygon.getB2DPolygon(a));
        const sal_uInt32 nPointCount(aCandidate.count());
        bool bNeedToSplit(false);

        if (aCandidate.areControlPointsUsed())
        {
            // compare with the maximum for bezier curved polygons
            bNeedToSplit = nPointCount > ((MAX_POLYGON_POINT_COUNT_METAFILE / 3L) - 1);
        }
        else
        {
            // compare with the maximum for simple point polygons
            bNeedToSplit = nPointCount > (MAX_POLYGON_POINT_COUNT_METAFILE - 1);
        }

        if (bNeedToSplit)
        {
            // need to split the partial polygon
            const basegfx::B2DRange aRange(aCandidate.getB2DRange());
            const basegfx::B2DPoint aCenter(aRange.getCenter());

            if (aRange.getWidth() > aRange.getHeight())
            {
                // clip in left and right
                const basegfx::B2DPolyPolygon aLeft(basegfx::utils::clipPolygonOnParallelAxis(
                    aCandidate, false, true, aCenter.getX(), false));
                const basegfx::B2DPolyPolygon aRight(basegfx::utils::clipPolygonOnParallelAxis(
                    aCandidate, false, false, aCenter.getX(), false));

                aSplitted.append(aLeft);
                aSplitted.append(aRight);
            }
            else
            {
                // clip in top and bottom
                const basegfx::B2DPolyPolygon aTop(basegfx::utils::clipPolygonOnParallelAxis(
                    aCandidate, true, true, aCenter.getY(), false));
                const basegfx::B2DPolyPolygon aBottom(basegfx::utils::clipPolygonOnParallelAxis(
                    aCandidate, true, false, aCenter.getY(), false));

                aSplitted.append(aTop);
                aSplitted.append(aBottom);
            }
        }
        else
        {
            aSplitted.append(aCandidate);
        }
    }

    if (aSplitted.count() != nPolyCount)
    {
        rPolyPolygon = aSplitted;
    }
}

/** Filter input polypolygon for effectively empty sub-fills

    Needed to fix fdo#37559

    @param rPoly
    tools::PolyPolygon to filter

    @return converted tools PolyPolygon, w/o one-point fills
 */
tools::PolyPolygon getFillPolyPolygon(const ::basegfx::B2DPolyPolygon& rPoly)
{
    // filter input rPoly
    basegfx::B2DPolyPolygon aPoly;
    sal_uInt32 nCount(rPoly.count());
    for (sal_uInt32 i = 0; i < nCount; ++i)
    {
        const basegfx::B2DPolygon& aCandidate(rPoly.getB2DPolygon(i));
        if (!aCandidate.isClosed() || aCandidate.count() > 1)
            aPoly.append(aCandidate);
    }
    return tools::PolyPolygon(aPoly);
}

} // end of anonymous namespace

namespace drawinglayer::processor2d
{
tools::Rectangle
VclMetafileProcessor2D::impDumpToMetaFile(const primitive2d::Primitive2DContainer& rContent,
                                          GDIMetaFile& o_rContentMetafile)
{
    // Prepare VDev, MetaFile and connections
    OutputDevice* pLastOutputDevice = mpOutputDevice;
    GDIMetaFile* pLastMetafile = mpMetaFile;
    basegfx::B2DRange aPrimitiveRange(rContent.getB2DRange(getViewInformation2D()));

    // transform primitive range with current transformation (e.g shadow offset)
    aPrimitiveRange.transform(maCurrentTransformation);

    const tools::Rectangle aPrimitiveRectangle(
        basegfx::fround(aPrimitiveRange.getMinX()), basegfx::fround(aPrimitiveRange.getMinY()),
        basegfx::fround(aPrimitiveRange.getMaxX()), basegfx::fround(aPrimitiveRange.getMaxY()));
    ScopedVclPtrInstance<VirtualDevice> aContentVDev;
    MapMode aNewMapMode(pLastOutputDevice->GetMapMode());

    mpOutputDevice = aContentVDev.get();
    mpMetaFile = &o_rContentMetafile;
    aContentVDev->EnableOutput(false);
    aContentVDev->SetMapMode(pLastOutputDevice->GetMapMode());
    o_rContentMetafile.Record(aContentVDev.get());
    aContentVDev->SetLineColor(pLastOutputDevice->GetLineColor());
    aContentVDev->SetFillColor(pLastOutputDevice->GetFillColor());
    aContentVDev->SetFont(pLastOutputDevice->GetFont());
    aContentVDev->SetDrawMode(pLastOutputDevice->GetDrawMode());
    aContentVDev->SetSettings(pLastOutputDevice->GetSettings());
    aContentVDev->SetRefPoint(pLastOutputDevice->GetRefPoint());

    // dump to MetaFile
    process(rContent);

    // cleanups
    o_rContentMetafile.Stop();
    o_rContentMetafile.WindStart();
    aNewMapMode.SetOrigin(aPrimitiveRectangle.TopLeft());
    o_rContentMetafile.SetPrefMapMode(aNewMapMode);
    o_rContentMetafile.SetPrefSize(aPrimitiveRectangle.GetSize());
    mpOutputDevice = pLastOutputDevice;
    mpMetaFile = pLastMetafile;

    return aPrimitiveRectangle;
}

void VclMetafileProcessor2D::impConvertFillGradientAttributeToVCLGradient(
    Gradient& o_rVCLGradient, const attribute::FillGradientAttribute& rFiGrAtt,
    bool bIsTransparenceGradient) const
{
    const basegfx::BColor aStartColor(rFiGrAtt.getColorStops().front().getStopColor());
    const basegfx::BColor aEndColor(rFiGrAtt.getColorStops().back().getStopColor());

    if (bIsTransparenceGradient)
    {
        // it's about transparence channel intensities (black/white), do not use color modifier
        o_rVCLGradient.SetStartColor(Color(aStartColor));
        o_rVCLGradient.SetEndColor(Color(aEndColor));
    }
    else
    {
        // use color modifier to influence start/end color of gradient
        o_rVCLGradient.SetStartColor(Color(maBColorModifierStack.getModifiedColor(aStartColor)));
        o_rVCLGradient.SetEndColor(Color(maBColorModifierStack.getModifiedColor(aEndColor)));
    }

    o_rVCLGradient.SetAngle(
        Degree10(static_cast<sal_uInt32>(basegfx::rad2deg<10>(rFiGrAtt.getAngle()))));
    o_rVCLGradient.SetBorder(static_cast<sal_uInt16>(rFiGrAtt.getBorder() * 100.0));
    o_rVCLGradient.SetOfsX(static_cast<sal_uInt16>(rFiGrAtt.getOffsetX() * 100.0));
    o_rVCLGradient.SetOfsY(static_cast<sal_uInt16>(rFiGrAtt.getOffsetY() * 100.0));
    o_rVCLGradient.SetSteps(rFiGrAtt.getSteps());

    // defaults for intensity; those were computed into the start/end colors already
    o_rVCLGradient.SetStartIntensity(100);
    o_rVCLGradient.SetEndIntensity(100);
    o_rVCLGradient.SetStyle(rFiGrAtt.getStyle());
}

void VclMetafileProcessor2D::impStartSvtGraphicFill(SvtGraphicFill const* pSvtGraphicFill)
{
    if (pSvtGraphicFill && !mnSvtGraphicFillCount)
    {
        SvMemoryStream aMemStm;

        WriteSvtGraphicFill(aMemStm, *pSvtGraphicFill);
        mpMetaFile->AddAction(new MetaCommentAction(
            "XPATHFILL_SEQ_BEGIN"_ostr, 0, static_cast<const sal_uInt8*>(aMemStm.GetData()),
            aMemStm.TellEnd()));
        mnSvtGraphicFillCount++;
    }
}

void VclMetafileProcessor2D::impEndSvtGraphicFill(SvtGraphicFill const* pSvtGraphicFill)
{
    if (pSvtGraphicFill && mnSvtGraphicFillCount)
    {
        mnSvtGraphicFillCount--;
        mpMetaFile->AddAction(new MetaCommentAction("XPATHFILL_SEQ_END"_ostr));
    }
}

double VclMetafileProcessor2D::getTransformedLineWidth(double fWidth) const
{
    // #i113922# the LineWidth is duplicated in the MetaPolylineAction,
    // and also inside the SvtGraphicStroke and needs transforming into
    // the same space as its coordinates here cf. fdo#61789
    // This is a partial fix. When an object transformation is used which
    // e.g. contains a scaleX != scaleY, an unproportional scaling will happen.
    const basegfx::B2DVector aDiscreteUnit(maCurrentTransformation
                                           * basegfx::B2DVector(fWidth, 0.0));

    return aDiscreteUnit.getLength();
}

std::unique_ptr<SvtGraphicStroke> VclMetafileProcessor2D::impTryToCreateSvtGraphicStroke(
    const basegfx::B2DPolygon& rB2DPolygon, const basegfx::BColor* pColor,
    const attribute::LineAttribute* pLineAttribute,
    const attribute::StrokeAttribute* pStrokeAttribute,
    const attribute::LineStartEndAttribute* pStart, const attribute::LineStartEndAttribute* pEnd)
{
    std::unique_ptr<SvtGraphicStroke> pRetval;

    if (rB2DPolygon.count() && !mnSvtGraphicStrokeCount)
    {
        basegfx::B2DPolygon aLocalPolygon(rB2DPolygon);
        basegfx::BColor aStrokeColor;
        basegfx::B2DPolyPolygon aStartArrow;
        basegfx::B2DPolyPolygon aEndArrow;

        if (pColor)
        {
            aStrokeColor = *pColor;
        }
        else if (pLineAttribute)
        {
            aStrokeColor = maBColorModifierStack.getModifiedColor(pLineAttribute->getColor());
        }

        // It IS needed to record the stroke color at all in the metafile,
        // SvtGraphicStroke has NO entry for stroke color(!)
        mpOutputDevice->SetLineColor(Color(aStrokeColor));

        if (!aLocalPolygon.isClosed())
        {
            double fPolyLength(0.0);
            double fStart(0.0);
            double fEnd(0.0);

            if (pStart && pStart->isActive())
            {
                fPolyLength = basegfx::utils::getLength(aLocalPolygon);

                aStartArrow = basegfx::utils::createAreaGeometryForLineStartEnd(
                    aLocalPolygon, pStart->getB2DPolyPolygon(), true, pStart->getWidth(),
                    fPolyLength, pStart->isCentered() ? 0.5 : 0.0, &fStart);
            }

            if (pEnd && pEnd->isActive())
            {
                if (basegfx::fTools::equalZero(fPolyLength))
                {
                    fPolyLength = basegfx::utils::getLength(aLocalPolygon);
                }

                aEndArrow = basegfx::utils::createAreaGeometryForLineStartEnd(
                    aLocalPolygon, pEnd->getB2DPolyPolygon(), false, pEnd->getWidth(), fPolyLength,
                    pEnd->isCentered() ? 0.5 : 0.0, &fEnd);
            }

            if (0.0 != fStart || 0.0 != fEnd)
            {
                // build new poly, consume something from old poly
                aLocalPolygon = basegfx::utils::getSnippetAbsolute(aLocalPolygon, fStart,
                                                                   fPolyLength - fEnd, fPolyLength);
            }
        }

        SvtGraphicStroke::JoinType eJoin(SvtGraphicStroke::joinNone);
        SvtGraphicStroke::CapType eCap(SvtGraphicStroke::capButt);
        double fLineWidth(0.0);
        double fMiterLength(0.0);
        SvtGraphicStroke::DashArray aDashArray;

        if (pLineAttribute)
        {
            fLineWidth = fMiterLength = getTransformedLineWidth(pLineAttribute->getWidth());

            // get Join
            switch (pLineAttribute->getLineJoin())
            {
                case basegfx::B2DLineJoin::NONE:
                {
                    eJoin = SvtGraphicStroke::joinNone;
                    break;
                }
                case basegfx::B2DLineJoin::Bevel:
                {
                    eJoin = SvtGraphicStroke::joinBevel;
                    break;
                }
                case basegfx::B2DLineJoin::Miter:
                {
                    eJoin = SvtGraphicStroke::joinMiter;
                    // ATM 15 degrees is assumed
                    // TODO wait for P1383R0 and C++20's std::numbers::pi
                    fMiterLength /= std::sin(M_PI / 12);
                    break;
                }
                case basegfx::B2DLineJoin::Round:
                {
                    eJoin = SvtGraphicStroke::joinRound;
                    break;
                }
            }

            // get stroke
            switch (pLineAttribute->getLineCap())
            {
                default: /* css::drawing::LineCap_BUTT */
                {
                    eCap = SvtGraphicStroke::capButt;
                    break;
                }
                case css::drawing::LineCap_ROUND:
                {
                    eCap = SvtGraphicStroke::capRound;
                    break;
                }
                case css::drawing::LineCap_SQUARE:
                {
                    eCap = SvtGraphicStroke::capSquare;
                    break;
                }
            }
        }

        if (pStrokeAttribute)
        {
            // copy dash array
            aDashArray = pStrokeAttribute->getDotDashArray();
        }

        // #i101734# apply current object transformation to created geometry.
        // This is a partial fix. When an object transformation is used which
        // e.g. contains a scaleX != scaleY, an unproportional scaling would
        // have to be applied to the evtl. existing fat line. The current
        // concept of PDF export and SvtGraphicStroke usage does simply not
        // allow handling such definitions. The only clean way would be to
        // add the transformation to SvtGraphicStroke and to handle it there
        aLocalPolygon.transform(maCurrentTransformation);
        aStartArrow.transform(maCurrentTransformation);
        aEndArrow.transform(maCurrentTransformation);

        pRetval.reset(
            new SvtGraphicStroke(tools::Polygon(aLocalPolygon), tools::PolyPolygon(aStartArrow),
                                 tools::PolyPolygon(aEndArrow), mfCurrentUnifiedTransparence,
                                 fLineWidth, eCap, eJoin, fMiterLength, std::move(aDashArray)));
    }

    return pRetval;
}

void VclMetafileProcessor2D::impStartSvtGraphicStroke(SvtGraphicStroke const* pSvtGraphicStroke)
{
    if (pSvtGraphicStroke && !mnSvtGraphicStrokeCount)
    {
        SvMemoryStream aMemStm;

        WriteSvtGraphicStroke(aMemStm, *pSvtGraphicStroke);
        mpMetaFile->AddAction(new MetaCommentAction(
            "XPATHSTROKE_SEQ_BEGIN"_ostr, 0, static_cast<const sal_uInt8*>(aMemStm.GetData()),
            aMemStm.TellEnd()));
        mnSvtGraphicStrokeCount++;
    }
}

void VclMetafileProcessor2D::impEndSvtGraphicStroke(SvtGraphicStroke const* pSvtGraphicStroke)
{
    if (pSvtGraphicStroke && mnSvtGraphicStrokeCount)
    {
        mnSvtGraphicStrokeCount--;
        mpMetaFile->AddAction(new MetaCommentAction("XPATHSTROKE_SEQ_END"_ostr));
    }
}

void VclMetafileProcessor2D::popStructureElement(vcl::PDFWriter::StructElement eElem)
{
    if (!maListElements.empty() && maListElements.top() == eElem)
    {
        maListElements.pop();
        mpPDFExtOutDevData->EndStructureElement();
    }
}

void VclMetafileProcessor2D::popListItem()
{
    popStructureElement(vcl::PDFWriter::LIBody);
    popStructureElement(vcl::PDFWriter::ListItem);
}

void VclMetafileProcessor2D::popList()
{
    popListItem();
    popStructureElement(vcl::PDFWriter::List);
}

// init static break iterator
vcl::DeleteOnDeinit<uno::Reference<css::i18n::XBreakIterator>>
    VclMetafileProcessor2D::mxBreakIterator;

VclMetafileProcessor2D::VclMetafileProcessor2D(const geometry::ViewInformation2D& rViewInformation,
                                               OutputDevice& rOutDev)
    : VclProcessor2D(rViewInformation, rOutDev)
    , mpMetaFile(rOutDev.GetConnectMetaFile())
    , mnSvtGraphicFillCount(0)
    , mnSvtGraphicStrokeCount(0)
    , mfCurrentUnifiedTransparence(0.0)
    , mpPDFExtOutDevData(dynamic_cast<vcl::PDFExtOutDevData*>(rOutDev.GetExtOutDevData()))
    , mnCurrentOutlineLevel(-1)
    , mbInListItem(false)
    , mbBulletPresent(false)
{
    OSL_ENSURE(rOutDev.GetConnectMetaFile(),
               "VclMetafileProcessor2D: Used on OutDev which has no MetaFile Target (!)");
    // draw to logic coordinates, do not initialize maCurrentTransformation to viewTransformation
    // but only to ObjectTransformation. Do not change MapMode of destination.
    maCurrentTransformation = rViewInformation.getObjectTransformation();
}

VclMetafileProcessor2D::~VclMetafileProcessor2D()
{
    // MapMode was not changed, no restore necessary
}

/***********************************************************************************************

    Support of MetaCommentActions in the VclMetafileProcessor2D
    Found MetaCommentActions and how they are supported:

    XGRAD_SEQ_BEGIN, XGRAD_SEQ_END:

    Used inside OutputDevice::DrawGradient to mark the start and end of a MetaGradientEx action.
    It is used in various exporters/importers to have direct access to the gradient before it
    is rendered by VCL (and thus fragmented to polygon color actions and others). On that base, e.g.
    the Metafile to SdrObject import creates its gradient objects.
    Best (and safest) way to support it here is to use PRIMITIVE2D_ID_POLYPOLYGONGRADIENTPRIMITIVE2D,
    map it back to the corresponding tools tools::PolyPolygon and the Gradient and just call
    OutputDevice::DrawGradient which creates the necessary compatible actions.

    XPATHFILL_SEQ_BEGIN, XPATHFILL_SEQ_END:

    Two producers, one is vcl/source/gdi/gdimtf.cxx, line 1273. There, it is transformed
    inside GDIMetaFile::Rotate, nothing to take care of here.
    The second producer is in graphics/svx/source/svdraw/impgrfll.cxx, line 374. This is used
    with each incarnation of Imp_GraphicFill when a metafile is recorded, fillstyle is not
    XFILL_NONE and not completely transparent. It creates a SvtGraphicFill and streams it
    to the comment action. A closing end token is created in the destructor.
    Usages of Imp_GraphicFill are in Do_Paint_Object-methods of SdrCircObj, SdrPathObj and
    SdrRectObj.
    The token users pick various actions from SvtGraphicFill, so it may need to be added for all kind
    of filled objects, even simple colored polygons. It is added as extra information; the
    Metafile actions between the two tokens are interpreted as output generated from those
    fills. Thus, users have the choice to use the SvtGraphicFill info or the created output
    actions.
    Even for XFillTransparenceItem it is used, thus it may need to be supported in
    UnifiedTransparencePrimitive2D, too, when interpreted as normally filled PolyPolygon.
    Implemented for:
        PRIMITIVE2D_ID_POLYPOLYGONGRAPHICPRIMITIVE2D,
        PRIMITIVE2D_ID_POLYPOLYGONHATCHPRIMITIVE2D,
        PRIMITIVE2D_ID_POLYPOLYGONGRADIENTPRIMITIVE2D,
        PRIMITIVE2D_ID_POLYPOLYGONCOLORPRIMITIVE2D,
        and for PRIMITIVE2D_ID_UNIFIEDTRANSPARENCEPRIMITIVE2D when detected unified transparence

    XPATHSTROKE_SEQ_BEGIN, XPATHSTROKE_SEQ_END:

    Similar to pathfill, but using SvtGraphicStroke instead. It also has two producers where one
    is also the GDIMetaFile::Rotate. Another user is MetaCommentAction::Move which modifies the
    contained path accordingly.
    The other one is SdrObject::Imp_DrawLineGeometry. It's done when MetaFile is set at OutDev and
    only when geometry is a single polygon (!). I see no reason for that; in the PS exporter this
    would hinder to make use of tools::PolyPolygon strokes. I will need to add support at:
        PRIMITIVE2D_ID_POLYGONHAIRLINEPRIMITIVE2D
        PRIMITIVE2D_ID_POLYGONSTROKEPRIMITIVE2D
        PRIMITIVE2D_ID_POLYGONSTROKEARROWPRIMITIVE2D
    This can be done hierarchical, too.
    Okay, base implementation done based on those three primitives.

    FIELD_SEQ_BEGIN, FIELD_SEQ_END

    Used from slideshow for URLs, created from diverse SvxField implementations inside
    createBeginComment()/createEndComment(). createBeginComment() is used from editeng\impedit3.cxx
    inside ImpEditEngine::Paint.
    Created TextHierarchyFieldPrimitive2D and added needed infos there; it is a group primitive and wraps
    text primitives (but is not limited to that). It contains the field type if special actions for the
    support of FIELD_SEQ_BEGIN/END are needed; this is the case for Page and URL fields. If more is
    needed, it may be supported there.
    FIELD_SEQ_BEGIN;PageField
    FIELD_SEQ_END
    Okay, these are now completely supported by TextHierarchyFieldPrimitive2D. URL works, too.

    XTEXT

    XTEXT_EOC(i) end of character
    XTEXT_EOW(i) end of word
    XTEXT_EOS(i) end of sentence

    this three are with index and are created with the help of an i18n::XBreakIterator in
    ImplDrawWithComments. Simplifying, moving out text painting, reworking to create some
    data structure for holding those TEXT infos.
    Supported directly by TextSimplePortionPrimitive2D with adding a Locale to the basic text
    primitive. In the MetaFileRenderer, the creation is now done (see below). This has the advantage
    that this creations do not need to be done for all paints all the time. This would be
    expensive since the BreakIterator and it's usage is expensive and for each paint also the
    whole character stops would need to be created.
    Created only for TextDecoratedPortionPrimitive2D due to XTEXT_EOL and XTEXT_EOP (see below)

    XTEXT_EOL() end of line
    XTEXT_EOP() end of paragraph

    First try with boolean marks at TextDecoratedPortionPrimitive2D did not work too well,
    i decided to solve it with structure. I added the TextHierarchyPrimitives for this,
    namely:
    - TextHierarchyLinePrimitive2D: Encapsulates single line
    - TextHierarchyParagraphPrimitive2D: Encapsulates single paragraph
    - TextHierarchyBlockPrimitive2D: encapsulates object texts (only one ATM)
    Those are now supported in hierarchy. This means the MetaFile renderer will support them
    by using them, recursively using their content and adding MetaFile comments as needed.
    This also means that when another text layouter will be used it will be necessary to
    create/support the same HierarchyPrimitives to support users.
    To transport the information using this hierarchy is best suited to all future needs;
    the slideshow will be able to profit from it directly when using primitives; all other
    renderers not interested in the text structure will just ignore the encapsulations.

    XTEXT_PAINTSHAPE_BEGIN, XTEXT_PAINTSHAPE_END
    Supported now by the TextHierarchyBlockPrimitive2D.

    EPSReplacementGraphic:
    Only used in goodies\source\filter.vcl\ieps\ieps.cxx and svx\source\xml\xmlgrhlp.cxx to
    hold the original EPS which was imported in the same MetaFile as first 2 entries. Only
    used to export the original again (if exists).
    Not necessary to support with MetaFileRenderer.

    XTEXT_SCROLLRECT, XTEXT_PAINTRECT
    Currently used to get extra MetaFile infos using GraphicExporter which again uses
    SdrTextObj::GetTextScrollMetaFileAndRectangle(). ATM works with primitives since
    the rectangle data is added directly by the GraphicsExporter as comment. Does not need
    to be adapted at once.
    When adapting later, the only user - the diashow - should directly use the provided
    Animation infos in the appropriate primitives (e.g. AnimatedSwitchPrimitive2D)

    PRNSPOOL_TRANSPARENTBITMAP_BEGIN, PRNSPOOL_TRANSPARENTBITMAP_END
    VCL usage when printing PL -> THB. Okay, THB confirms that it is only used as
    a fix (hack) while VCL printing. It is needed to not downscale a bitmap which
    was explicitly created for the printer already again to some default maximum
    bitmap sizes.
    Nothing to do here for the primitive renderer.

    Support for vcl::PDFExtOutDevData:
    PL knows that SJ did that stuff, it's used to hold a pointer to PDFExtOutDevData at
    the OutDev. When set, some extra data is written there. Trying simple PDF export and
    watching if I get those infos.
    Well, a PDF export does not use e.g. ImpEditEngine::Paint since the PdfFilter uses
    the SdXImpressDocument::render and thus uses the VclMetafileProcessor2D. I will check
    if I get a PDFExtOutDevData at the target output device.
    Indeed, I get one. Checking what all may be done when that extra-device-info is there.

    All in all I have to talk to SJ. I will need to emulate some of those actions, but
    i need to discuss which ones.
    In the future, all those infos would be taken from the primitive sequence anyways,
    thus these extensions would potentially be temporary, too.
    Discussed with SJ, added the necessary support and tested it. Details follow.

    - In ImpEditEngine::Paint, paragraph infos and URL stuff is added.
      Added in primitive MetaFile renderer.
      Checking URL: Indeed, current version exports it, but it is missing in primitive
      CWS version. Adding support.
      Okay, URLs work. Checked, Done.

    - UnoControlPDFExportContact is only created when PDFExtOutDevData is used at the
      target and uno control data is created in UnoControlPDFExportContact::do_PaintObject.
      This was added in primitive MetaFile renderer.
      Checked form control export, it works well. Done.

    - In goodies, in GraphicObject::Draw, when the used Graphic is linked, infos are
      generated. I will need to check what happens here with primitives.
      To support, use of GraphicPrimitive2D (PRIMITIVE2D_ID_GRAPHICPRIMITIVE2D) may be needed.
      Added support, but feature is broken in main version, so i cannot test at all.
      Writing a bug to CL (or SJ) and seeing what happens (#i80380#).
      SJ took a look and we got it working. Tested VCL MetaFile Renderer based export,
      as intended, the original file is exported. Works, Done.


    To be done:

    - Maybe there are more places to take care of for vcl::PDFExtOutDevData!


****************************************************************************************************/

void VclMetafileProcessor2D::processBasePrimitive2D(const primitive2d::BasePrimitive2D& rCandidate)
{
    switch (rCandidate.getPrimitive2DID())
    {
        case PRIMITIVE2D_ID_WRONGSPELLPRIMITIVE2D:
        {
            // directdraw of wrong spell primitive
            // Ignore for VclMetafileProcessor2D, this is for printing and MetaFile recording only
            break;
        }
        case PRIMITIVE2D_ID_GRAPHICPRIMITIVE2D:
        {
            processGraphicPrimitive2D(
                static_cast<const primitive2d::GraphicPrimitive2D&>(rCandidate));
            break;
        }
        case PRIMITIVE2D_ID_CONTROLPRIMITIVE2D:
        {
            processControlPrimitive2D(
                static_cast<const primitive2d::ControlPrimitive2D&>(rCandidate));
            break;
        }
        case PRIMITIVE2D_ID_TEXTHIERARCHYFIELDPRIMITIVE2D:
        {
            processTextHierarchyFieldPrimitive2D(
                static_cast<const primitive2d::TextHierarchyFieldPrimitive2D&>(rCandidate));
            break;
        }
        case PRIMITIVE2D_ID_TEXTHIERARCHYLINEPRIMITIVE2D:
        {
            processTextHierarchyLinePrimitive2D(
                static_cast<const primitive2d::TextHierarchyLinePrimitive2D&>(rCandidate));
            break;
        }
        case PRIMITIVE2D_ID_TEXTHIERARCHYBULLETPRIMITIVE2D:
        {
            processTextHierarchyBulletPrimitive2D(
                static_cast<const primitive2d::TextHierarchyBulletPrimitive2D&>(rCandidate));
            break;
        }
        case PRIMITIVE2D_ID_TEXTHIERARCHYPARAGRAPHPRIMITIVE2D:
        {
            processTextHierarchyParagraphPrimitive2D(
                static_cast<const primitive2d::TextHierarchyParagraphPrimitive2D&>(rCandidate));
            break;
        }
        case PRIMITIVE2D_ID_TEXTHIERARCHYBLOCKPRIMITIVE2D:
        {
            processTextHierarchyBlockPrimitive2D(
                static_cast<const primitive2d::TextHierarchyBlockPrimitive2D&>(rCandidate));
            break;
        }
        case PRIMITIVE2D_ID_TEXTSIMPLEPORTIONPRIMITIVE2D:
        case PRIMITIVE2D_ID_TEXTDECORATEDPORTIONPRIMITIVE2D:
        {
            // for supporting TEXT_ MetaFile actions there is more to do here; get the candidate
            processTextSimplePortionPrimitive2D(
                static_cast<const primitive2d::TextSimplePortionPrimitive2D&>(rCandidate));
            break;
        }
        case PRIMITIVE2D_ID_POLYGONHAIRLINEPRIMITIVE2D:
        {
            processPolygonHairlinePrimitive2D(
                static_cast<const primitive2d::PolygonHairlinePrimitive2D&>(rCandidate));
            break;
        }
        case PRIMITIVE2D_ID_POLYGONSTROKEPRIMITIVE2D:
        {
            processPolygonStrokePrimitive2D(
                static_cast<const primitive2d::PolygonStrokePrimitive2D&>(rCandidate));
            break;
        }
        case PRIMITIVE2D_ID_POLYGONSTROKEARROWPRIMITIVE2D:
        {
            processPolygonStrokeArrowPrimitive2D(
                static_cast<const primitive2d::PolygonStrokeArrowPrimitive2D&>(rCandidate));
            break;
        }
        case PRIMITIVE2D_ID_BITMAPPRIMITIVE2D:
        {
            // direct draw of transformed BitmapEx primitive; use default processing, but without
            // former testing if graphic content is inside discrete local viewport; this is not
            // setup for metafile targets (metafile renderer tries to render in logic coordinates,
            // the mapping is kept to the OutputDevice for better Metafile recording)
            RenderBitmapPrimitive2D(static_cast<const primitive2d::BitmapPrimitive2D&>(rCandidate));
            break;
        }
        case PRIMITIVE2D_ID_POLYPOLYGONGRAPHICPRIMITIVE2D:
        {
            if (maBColorModifierStack.count())
            {
                // tdf#151104 unfortunately processPolyPolygonGraphicPrimitive2D below
                // does not support an active BColorModifierStack, so use the default
                process(rCandidate);
            }
            else
            {
                processPolyPolygonGraphicPrimitive2D(
                    static_cast<const primitive2d::PolyPolygonGraphicPrimitive2D&>(rCandidate));
            }
            break;
        }
        case PRIMITIVE2D_ID_POLYPOLYGONHATCHPRIMITIVE2D:
        {
            processPolyPolygonHatchPrimitive2D(
                static_cast<const primitive2d::PolyPolygonHatchPrimitive2D&>(rCandidate));
            break;
        }
        case PRIMITIVE2D_ID_POLYPOLYGONGRADIENTPRIMITIVE2D:
        {
            processPolyPolygonGradientPrimitive2D(
                static_cast<const primitive2d::PolyPolygonGradientPrimitive2D&>(rCandidate));
            break;
        }
        case PRIMITIVE2D_ID_POLYPOLYGONCOLORPRIMITIVE2D:
        {
            processPolyPolygonColorPrimitive2D(
                static_cast<const primitive2d::PolyPolygonColorPrimitive2D&>(rCandidate));
            break;
        }
        case PRIMITIVE2D_ID_MASKPRIMITIVE2D:
        {
            processMaskPrimitive2D(static_cast<const primitive2d::MaskPrimitive2D&>(rCandidate));
            break;
        }
        case PRIMITIVE2D_ID_MODIFIEDCOLORPRIMITIVE2D:
        {
            // modified color group. Force output to unified color. Use default processing.
            RenderModifiedColorPrimitive2D(
                static_cast<const primitive2d::ModifiedColorPrimitive2D&>(rCandidate));
            break;
        }
        case PRIMITIVE2D_ID_UNIFIEDTRANSPARENCEPRIMITIVE2D:
        {
            processUnifiedTransparencePrimitive2D(
                static_cast<const primitive2d::UnifiedTransparencePrimitive2D&>(rCandidate));
            break;
        }
        case PRIMITIVE2D_ID_TRANSPARENCEPRIMITIVE2D:
        {
            processTransparencePrimitive2D(
                static_cast<const primitive2d::TransparencePrimitive2D&>(rCandidate));
            break;
        }
        case PRIMITIVE2D_ID_TRANSFORMPRIMITIVE2D:
        {
            // use default transform group processing
            RenderTransformPrimitive2D(
                static_cast<const primitive2d::TransformPrimitive2D&>(rCandidate));
            break;
        }
        case PRIMITIVE2D_ID_PAGEPREVIEWPRIMITIVE2D:
        {
            // new XDrawPage for ViewInformation2D
            RenderPagePreviewPrimitive2D(
                static_cast<const primitive2d::PagePreviewPrimitive2D&>(rCandidate));
            break;
        }
        case PRIMITIVE2D_ID_MARKERARRAYPRIMITIVE2D:
        {
            // use default marker array processing
            RenderMarkerArrayPrimitive2D(
                static_cast<const primitive2d::MarkerArrayPrimitive2D&>(rCandidate));
            break;
        }
        case PRIMITIVE2D_ID_POINTARRAYPRIMITIVE2D:
        {
            // use default point array processing
            RenderPointArrayPrimitive2D(
                static_cast<const primitive2d::PointArrayPrimitive2D&>(rCandidate));
            break;
        }
        case PRIMITIVE2D_ID_STRUCTURETAGPRIMITIVE2D:
        {
            processStructureTagPrimitive2D(
                static_cast<const primitive2d::StructureTagPrimitive2D&>(rCandidate));
            break;
        }
        case PRIMITIVE2D_ID_TEXTHIERARCHYEDITPRIMITIVE2D:
        {
            // This primitive is created if a text edit is active and contains it's
            // current content, not from model data itself.
            // Pixel renderers need to suppress that content, it gets displayed by the active
            // TextEdit in the EditView. Suppression is done by decomposing to nothing.
            // MetaFile renderers have to show it, so that the edited text is part of the
            // MetaFile, e.g. needed for presentation previews and exports.
            // So take action here and process it's content:
            // Note: Former error was #i97628#
            process(static_cast<const primitive2d::TextHierarchyEditPrimitive2D&>(rCandidate)
                        .getContent());
            break;
        }
        case PRIMITIVE2D_ID_EPSPRIMITIVE2D:
        {
            RenderEpsPrimitive2D(static_cast<const primitive2d::EpsPrimitive2D&>(rCandidate));
            break;
        }
        case PRIMITIVE2D_ID_OBJECTINFOPRIMITIVE2D:
        {
            processObjectInfoPrimitive2D(
                static_cast<const primitive2d::ObjectInfoPrimitive2D&>(rCandidate));
            break;
        }
        default:
        {
            // process recursively
            process(rCandidate);
            break;
        }
    }
}

void VclMetafileProcessor2D::processObjectInfoPrimitive2D(
    primitive2d::ObjectInfoPrimitive2D const& rObjectInfoPrimitive2D)
{
    // tdf#154982 process content first, so this object overrides any nested one
    process(rObjectInfoPrimitive2D.getChildren());

    // currently StructureTagPrimitive2D is only used for SdrObjects - have to
    // avoid adding Alt text if the SdrObject is not actually tagged, as it
    // would then end up on an unrelated structure element.
    if (mpCurrentStructureTag && mpCurrentStructureTag->isTaggedSdrObject())
    {
        // Create image alternative description from ObjectInfoPrimitive2D info
        // for PDF export, for the currently active SdrObject's structure element
        if (mpPDFExtOutDevData->GetIsExportTaggedPDF())
        {
            OUString aAlternateDescription;

            if (!rObjectInfoPrimitive2D.getTitle().isEmpty())
            {
                aAlternateDescription += rObjectInfoPrimitive2D.getTitle();
            }

            if (!rObjectInfoPrimitive2D.getDesc().isEmpty())
            {
                if (!aAlternateDescription.isEmpty())
                {
                    aAlternateDescription += " - ";
                }

                aAlternateDescription += rObjectInfoPrimitive2D.getDesc();
            }

            // Use SetAlternateText to set it. This will work as long as some
            // structure is used (see PDFWriterImpl::setAlternateText and
            // m_nCurrentStructElement - tagged PDF export works with this in
            // Draw/Impress/Writer, but not in Calc due to too less structure...)
            //Z maybe add structure to Calc PDF export, may need some BeginGroup/EndGroup stuff ..?
            if (!aAlternateDescription.isEmpty())
            {
                mpPDFExtOutDevData->SetAlternateText(aAlternateDescription);
            }
        }
    }
}

void VclMetafileProcessor2D::processGraphicPrimitive2D(
    const primitive2d::GraphicPrimitive2D& rGraphicPrimitive)
{
    bool bUsingPDFExtOutDevData(false);
    basegfx::B2DVector aTranslate, aScale;
    static bool bSuppressPDFExtOutDevDataSupport(false); // loplugin:constvars:ignore

    if (mpPDFExtOutDevData && !bSuppressPDFExtOutDevDataSupport)
    {
        // emulate data handling from UnoControlPDFExportContact, original see
        // svtools/source/graphic/grfmgr.cxx
        const Graphic& rGraphic = rGraphicPrimitive.getGraphicObject().GetGraphic();

        if (rGraphic.IsGfxLink())
        {
            const GraphicAttr& rAttr = rGraphicPrimitive.getGraphicAttr();

            if (!rAttr.IsSpecialDrawMode() && !rAttr.IsAdjusted())
            {
                const basegfx::B2DHomMatrix& rTransform = rGraphicPrimitive.getTransform();
                double fRotate, fShearX;
                rTransform.decompose(aScale, aTranslate, fRotate, fShearX);

                if (basegfx::fTools::equalZero(fRotate) && (aScale.getX() > 0.0)
                    && (aScale.getY() > 0.0))
                {
                    bUsingPDFExtOutDevData = true;
                    mpPDFExtOutDevData->BeginGroup();
                }
            }
        }
    }

    // process recursively and add MetaFile comment
    process(rGraphicPrimitive);

    if (!bUsingPDFExtOutDevData)
        return;

    // emulate data handling from UnoControlPDFExportContact, original see
    // svtools/source/graphic/grfmgr.cxx
    const basegfx::B2DRange aCurrentRange(aTranslate.getX(), aTranslate.getY(),
                                          aTranslate.getX() + aScale.getX(),
                                          aTranslate.getY() + aScale.getY());
    const tools::Rectangle aCurrentRect(
        sal_Int32(floor(aCurrentRange.getMinX())), sal_Int32(floor(aCurrentRange.getMinY())),
        sal_Int32(ceil(aCurrentRange.getMaxX())), sal_Int32(ceil(aCurrentRange.getMaxY())));
    const GraphicAttr& rAttr = rGraphicPrimitive.getGraphicAttr();
    // fdo#72530 don't pass empty Rectangle to EndGroup
    tools::Rectangle aCropRect(aCurrentRect);

    if (rAttr.IsCropped())
    {
        // calculate scalings between real image size and logic object size. This
        // is necessary since the crop values are relative to original bitmap size
        double fFactorX(1.0);
        double fFactorY(1.0);

        {
            const MapMode aMapMode100thmm(MapUnit::Map100thMM);
            const Size aBitmapSize(OutputDevice::LogicToLogic(
                rGraphicPrimitive.getGraphicObject().GetPrefSize(),
                rGraphicPrimitive.getGraphicObject().GetPrefMapMode(), aMapMode100thmm));
            const double fDivX(aBitmapSize.Width() - rAttr.GetLeftCrop() - rAttr.GetRightCrop());
            const double fDivY(aBitmapSize.Height() - rAttr.GetTopCrop() - rAttr.GetBottomCrop());

            if (!basegfx::fTools::equalZero(fDivX))
            {
                fFactorX = aScale.getX() / fDivX;
            }

            if (!basegfx::fTools::equalZero(fDivY))
            {
                fFactorY = aScale.getY() / fDivY;
            }
        }

        // calculate crop range and rect
        basegfx::B2DRange aCropRange;
        aCropRange.expand(
            aCurrentRange.getMinimum()
            - basegfx::B2DPoint(rAttr.GetLeftCrop() * fFactorX, rAttr.GetTopCrop() * fFactorY));
        aCropRange.expand(
            aCurrentRange.getMaximum()
            + basegfx::B2DPoint(rAttr.GetRightCrop() * fFactorX, rAttr.GetBottomCrop() * fFactorY));

        aCropRect = tools::Rectangle(
            sal_Int32(floor(aCropRange.getMinX())), sal_Int32(floor(aCropRange.getMinY())),
            sal_Int32(ceil(aCropRange.getMaxX())), sal_Int32(ceil(aCropRange.getMaxY())));
    }

    // #i123295# 3rd param is uncropped rect, 4th is cropped. The primitive has the cropped
    // object transformation, thus aCurrentRect *is* the clip region while aCropRect is the expanded,
    // uncropped region. Thus, correct order is aCropRect, aCurrentRect
    mpPDFExtOutDevData->EndGroup(rGraphicPrimitive.getGraphicObject().GetGraphic(),
                                 255 - rAttr.GetAlpha(), aCropRect, aCurrentRect);
}

void VclMetafileProcessor2D::processControlPrimitive2D(
    const primitive2d::ControlPrimitive2D& rControlPrimitive)
{
    const uno::Reference<awt::XControl>& rXControl(rControlPrimitive.getXControl());
    bool bIsPrintableControl(false);

    // find out if control is printable
    if (rXControl.is())
    {
        try
        {
            uno::Reference<beans::XPropertySet> xModelProperties(rXControl->getModel(),
                                                                 uno::UNO_QUERY);
            uno::Reference<beans::XPropertySetInfo> xPropertyInfo(
                xModelProperties.is() ? xModelProperties->getPropertySetInfo()
                                      : uno::Reference<beans::XPropertySetInfo>());
            static constexpr OUString sPrintablePropertyName(u"Printable"_ustr);

            if (xPropertyInfo.is() && xPropertyInfo->hasPropertyByName(sPrintablePropertyName))
            {
                OSL_VERIFY(xModelProperties->getPropertyValue(sPrintablePropertyName)
                           >>= bIsPrintableControl);
            }
        }
        catch (const uno::Exception&)
        {
            TOOLS_WARN_EXCEPTION("drawinglayer",
                                 "VclMetafileProcessor2D: No access to printable flag of Control");
        }
    }

    // PDF export and printing only for printable controls
    if (!bIsPrintableControl)
        return;

    ::std::optional<sal_Int32> oAnchorParent;
    if (mpPDFExtOutDevData)
    {
        if (rControlPrimitive.GetAnchorStructureElementKey())
        {
            sal_Int32 const id = mpPDFExtOutDevData->EnsureStructureElement(
                rControlPrimitive.GetAnchorStructureElementKey());
            oAnchorParent.emplace(mpPDFExtOutDevData->GetCurrentStructureElement());
            mpPDFExtOutDevData->SetCurrentStructureElement(id);
        }
    }

    const bool bPDFExport(mpPDFExtOutDevData && mpPDFExtOutDevData->GetIsExportFormFields());
    bool bDoProcessRecursively(true);

    if (bPDFExport)
    {
        // PDF export. Emulate data handling from UnoControlPDFExportContact
        std::unique_ptr<vcl::PDFWriter::AnyWidget> pPDFControl(
            ::toolkitform::describePDFControl(rXControl, *mpPDFExtOutDevData));

        if (pPDFControl)
        {
            // still need to fill in the location (is a class Rectangle)
            const basegfx::B2DRange aRangeLogic(
                rControlPrimitive.getB2DRange(getViewInformation2D()));
            const tools::Rectangle aRectLogic(static_cast<sal_Int32>(floor(aRangeLogic.getMinX())),
                                              static_cast<sal_Int32>(floor(aRangeLogic.getMinY())),
                                              static_cast<sal_Int32>(ceil(aRangeLogic.getMaxX())),
                                              static_cast<sal_Int32>(ceil(aRangeLogic.getMaxY())));
            pPDFControl->Location = aRectLogic;

            Size aFontSize(pPDFControl->TextFont.GetFontSize());
            aFontSize = OutputDevice::LogicToLogic(aFontSize, MapMode(MapUnit::MapPoint),
                                                   mpOutputDevice->GetMapMode());
            pPDFControl->TextFont.SetFontSize(aFontSize);

            mpPDFExtOutDevData->WrapBeginStructureElement(vcl::PDFWriter::Form);
            vcl::PDFWriter::StructAttributeValue role;
            switch (pPDFControl->Type)
            {
                case vcl::PDFWriter::PushButton:
                    role = vcl::PDFWriter::Pb;
                    break;
                case vcl::PDFWriter::RadioButton:
                    role = vcl::PDFWriter::Rb;
                    break;
                case vcl::PDFWriter::CheckBox:
                    role = vcl::PDFWriter::Cb;
                    break;
                default: // there is a paucity of roles, tv is the catch-all one
                    role = vcl::PDFWriter::Tv;
                    break;
            }
            // ISO 14289-1:2014, Clause: 7.18.4
            mpPDFExtOutDevData->SetStructureAttribute(vcl::PDFWriter::Role, role);
            // ISO 14289-1:2014, Clause: 7.18.1
            OUString const& rAltText(rControlPrimitive.GetAltText());
            if (!rAltText.isEmpty())
            {
                mpPDFExtOutDevData->SetAlternateText(rAltText);
            }
            mpPDFExtOutDevData->CreateControl(*pPDFControl);
            mpPDFExtOutDevData->EndStructureElement();
            if (oAnchorParent)
            {
                mpPDFExtOutDevData->SetCurrentStructureElement(*oAnchorParent);
            }

            // no normal paint needed (see original UnoControlPDFExportContact::do_PaintObject);
            // do not process recursively
            bDoProcessRecursively = false;
        }
        else
        {
            // PDF export did not work, try simple output.
            // Fallback to printer output by not setting bDoProcessRecursively
            // to false.
        }
    }

    if (!bDoProcessRecursively)
    {
        return;
    }

    if (mpPDFExtOutDevData)
    { // no corresponding PDF Form, use Figure instead
        mpPDFExtOutDevData->WrapBeginStructureElement(vcl::PDFWriter::Figure);
        mpPDFExtOutDevData->SetStructureAttribute(vcl::PDFWriter::Placement, vcl::PDFWriter::Block);
        auto const range(rControlPrimitive.getB2DRange(getViewInformation2D()));
        tools::Rectangle const aLogicRect(
            basegfx::fround(range.getMinX()), basegfx::fround(range.getMinY()),
            basegfx::fround(range.getMaxX()), basegfx::fround(range.getMaxY()));
        mpPDFExtOutDevData->SetStructureBoundingBox(aLogicRect);
        OUString const& rAltText(rControlPrimitive.GetAltText());
        if (!rAltText.isEmpty())
        {
            mpPDFExtOutDevData->SetAlternateText(rAltText);
        }
    }

    // #i93169# used flag the wrong way; true means that nothing was done yet
    if (bDoProcessRecursively)
    {
        // printer output
        try
        {
            // remember old graphics and create new
            uno::Reference<awt::XView> xControlView(rXControl, uno::UNO_QUERY_THROW);
            const uno::Reference<awt::XGraphics> xOriginalGraphics(xControlView->getGraphics());
            const uno::Reference<awt::XGraphics> xNewGraphics(mpOutputDevice->CreateUnoGraphics());

            if (xNewGraphics.is())
            {
                // link graphics and view
                xControlView->setGraphics(xNewGraphics);

                // get position
                const basegfx::B2DHomMatrix aObjectToDiscrete(
                    getViewInformation2D().getObjectToViewTransformation()
                    * rControlPrimitive.getTransform());
                const basegfx::B2DPoint aTopLeftDiscrete(aObjectToDiscrete
                                                         * basegfx::B2DPoint(0.0, 0.0));

                // draw it
                xControlView->draw(basegfx::fround(aTopLeftDiscrete.getX()),
                                   basegfx::fround(aTopLeftDiscrete.getY()));
                bDoProcessRecursively = false;

                // restore original graphics
                xControlView->setGraphics(xOriginalGraphics);
            }
        }
        catch (const uno::Exception&)
        {
            TOOLS_WARN_EXCEPTION("drawinglayer",
                                 "VclMetafileProcessor2D: Printing of Control failed");
        }
    }

    // process recursively if not done yet to export as decomposition (bitmap)
    if (bDoProcessRecursively)
    {
        process(rControlPrimitive);
    }

    if (mpPDFExtOutDevData)
    {
        mpPDFExtOutDevData->EndStructureElement();
        if (oAnchorParent)
        {
            mpPDFExtOutDevData->SetCurrentStructureElement(*oAnchorParent);
        }
    }
}

void VclMetafileProcessor2D::processTextHierarchyFieldPrimitive2D(
    const primitive2d::TextHierarchyFieldPrimitive2D& rFieldPrimitive)
{
    // support for FIELD_SEQ_BEGIN, FIELD_SEQ_END and URL. It wraps text primitives (but is not limited to)
    // thus do the MetafileAction embedding stuff but just handle recursively.
    static constexpr OString aCommentStringCommon("FIELD_SEQ_BEGIN"_ostr);
    OUString aURL;

    switch (rFieldPrimitive.getType())
    {
        default: // case drawinglayer::primitive2d::FIELD_TYPE_COMMON :
        {
            mpMetaFile->AddAction(new MetaCommentAction(aCommentStringCommon));
            break;
        }
        case drawinglayer::primitive2d::FIELD_TYPE_PAGE:
        {
            mpMetaFile->AddAction(new MetaCommentAction("FIELD_SEQ_BEGIN;PageField"_ostr));
            break;
        }
        case drawinglayer::primitive2d::FIELD_TYPE_URL:
        {
            aURL = rFieldPrimitive.getValue("URL");

            if (!aURL.isEmpty())
            {
                mpMetaFile->AddAction(new MetaCommentAction(
                    aCommentStringCommon, 0, reinterpret_cast<const sal_uInt8*>(aURL.getStr()),
                    2 * aURL.getLength()));
            }

            break;
        }
    }

    // process recursively
    primitive2d::Primitive2DContainer rContent;
    rFieldPrimitive.get2DDecomposition(rContent, getViewInformation2D());
    process(rContent);

    // for the end comment the type is not relevant yet, they are all the same. Just add.
    mpMetaFile->AddAction(new MetaCommentAction("FIELD_SEQ_END"_ostr));

    if (!(mpPDFExtOutDevData
          && drawinglayer::primitive2d::FIELD_TYPE_URL == rFieldPrimitive.getType()))
        return;

    // emulate data handling from ImpEditEngine::Paint
    const basegfx::B2DRange aViewRange(rContent.getB2DRange(getViewInformation2D()));
    const tools::Rectangle aRectLogic(static_cast<sal_Int32>(floor(aViewRange.getMinX())),
                                      static_cast<sal_Int32>(floor(aViewRange.getMinY())),
                                      static_cast<sal_Int32>(ceil(aViewRange.getMaxX())),
                                      static_cast<sal_Int32>(ceil(aViewRange.getMaxY())));
    vcl::PDFExtOutDevBookmarkEntry aBookmark;
    OUString const content(rFieldPrimitive.getValue("Representation"));
    aBookmark.nLinkId = mpPDFExtOutDevData->CreateLink(aRectLogic, content);
    aBookmark.aBookmark = aURL;
    std::vector<vcl::PDFExtOutDevBookmarkEntry>& rBookmarks = mpPDFExtOutDevData->GetBookmarks();
    rBookmarks.push_back(aBookmark);
}

void VclMetafileProcessor2D::processTextHierarchyLinePrimitive2D(
    const primitive2d::TextHierarchyLinePrimitive2D& rLinePrimitive)
{
    // process recursively and add MetaFile comment
    process(rLinePrimitive);
    mpMetaFile->AddAction(new MetaCommentAction("XTEXT_EOL"_ostr));
}

void VclMetafileProcessor2D::processTextHierarchyBulletPrimitive2D(
    const primitive2d::TextHierarchyBulletPrimitive2D& rBulletPrimitive)
{
    // this is a part of list item, start LILabel ( = bullet)
    if (mbInListItem)
    {
        maListElements.push(vcl::PDFWriter::LILabel);
        mpPDFExtOutDevData->WrapBeginStructureElement(vcl::PDFWriter::LILabel);
    }

    // process recursively and add MetaFile comment
    process(rBulletPrimitive);
    // in Outliner::PaintBullet(), a MetafileComment for bullets is added, too. The
    // "XTEXT_EOC" is used, use here, too.
    mpMetaFile->AddAction(new MetaCommentAction("XTEXT_EOC"_ostr));

    if (mbInListItem)
    {
        if (maListElements.top() == vcl::PDFWriter::LILabel)
        {
            maListElements.pop();
            mpPDFExtOutDevData->EndStructureElement(); // end LILabel
            mbBulletPresent = true;
        }
    }
}

void VclMetafileProcessor2D::processTextHierarchyParagraphPrimitive2D(
    const primitive2d::TextHierarchyParagraphPrimitive2D& rParagraphPrimitive)
{
    static constexpr OString aCommentString("XTEXT_EOP"_ostr);
    static bool bSuppressPDFExtOutDevDataSupport(false); // loplugin:constvars:ignore

    if (nullptr == mpPDFExtOutDevData || bSuppressPDFExtOutDevDataSupport)
    {
        // Non-PDF export behaviour (metafile only).
        // Process recursively and add MetaFile comment.
        process(rParagraphPrimitive);
        mpMetaFile->AddAction(new MetaCommentAction(aCommentString));
        return;
    }

    if (!mpPDFExtOutDevData->GetIsExportTaggedPDF())
    {
        // No Tagged PDF -> Dump as Paragraph
        // Emulate data handling from old ImpEditEngine::Paint
        mpPDFExtOutDevData->WrapBeginStructureElement(vcl::PDFWriter::Paragraph);

        // Process recursively and add MetaFile comment
        process(rParagraphPrimitive);
        mpMetaFile->AddAction(new MetaCommentAction(aCommentString));

        // Emulate data handling from ImpEditEngine::Paint
        mpPDFExtOutDevData->EndStructureElement();
        return;
    }

    // Create Tagged PDF -> deeper tagged data using StructureElements.
    // Use OutlineLevel from ParagraphPrimitive, ensure not below -1 what
    // means 'not active'
    const sal_Int16 nNewOutlineLevel(
        std::max(static_cast<sal_Int16>(-1), rParagraphPrimitive.getOutlineLevel()));

    // Do we have a change in OutlineLevel compared to the current one?
    if (nNewOutlineLevel != mnCurrentOutlineLevel)
    {
        if (nNewOutlineLevel > mnCurrentOutlineLevel)
        {
            // increase List level
            for (sal_Int16 a(mnCurrentOutlineLevel); a != nNewOutlineLevel; ++a)
            {
                maListElements.push(vcl::PDFWriter::List);
                mpPDFExtOutDevData->WrapBeginStructureElement(vcl::PDFWriter::List);
            }
        }
        else // if(nNewOutlineLevel < mnCurrentOutlineLevel)
        {
            // close list levels below nNewOutlineLevel completely by removing
            // list items as well as list tag itself
            for (sal_Int16 a(nNewOutlineLevel); a < mnCurrentOutlineLevel; ++a)
            {
                popList(); // end LBody LI and L
            }

            // on nNewOutlineLevel close the previous list item (LBody and LI)
            popListItem();
        }

        // Remember new current OutlineLevel
        mnCurrentOutlineLevel = nNewOutlineLevel;
    }
    else // the same list level
    {
        // close the previous list item (LBody and LI)
        popListItem();
    }

    const bool bDumpAsListItem(-1 != mnCurrentOutlineLevel);

    if (bDumpAsListItem)
    {
        // Dump as ListItem
        maListElements.push(vcl::PDFWriter::ListItem);
        mpPDFExtOutDevData->WrapBeginStructureElement(vcl::PDFWriter::ListItem);
        mbInListItem = true;
    }
    else
    {
        // Dump as Paragraph
        mpPDFExtOutDevData->WrapBeginStructureElement(vcl::PDFWriter::Paragraph);
    }

    // Process recursively and add MetaFile comment
    process(rParagraphPrimitive);
    mpMetaFile->AddAction(new MetaCommentAction(aCommentString));

    if (bDumpAsListItem)
        mbInListItem = false;
    else
        mpPDFExtOutDevData->EndStructureElement(); // end Paragraph
}

void VclMetafileProcessor2D::processTextHierarchyBlockPrimitive2D(
    const primitive2d::TextHierarchyBlockPrimitive2D& rBlockPrimitive)
{
    // add MetaFile comment, process recursively and add MetaFile comment
    mpMetaFile->AddAction(new MetaCommentAction("XTEXT_PAINTSHAPE_BEGIN"_ostr));
    process(rBlockPrimitive);

    if (mnCurrentOutlineLevel >= 0)
    {
        // end any opened List structure elements (LBody, LI, L)
        for (sal_Int16 a(0); a <= mnCurrentOutlineLevel; ++a)
        {
            popList();
        }
    }

    mpMetaFile->AddAction(new MetaCommentAction("XTEXT_PAINTSHAPE_END"_ostr));
}

void VclMetafileProcessor2D::processTextSimplePortionPrimitive2D(
    const primitive2d::TextSimplePortionPrimitive2D& rTextCandidate)
{
    // Adapt evtl. used special DrawMode
    const DrawModeFlags nOriginalDrawMode(mpOutputDevice->GetDrawMode());
    adaptTextToFillDrawMode();

    // this is a 2nd portion of list item
    // bullet has been already processed, start LIBody
    if (mbInListItem && mbBulletPresent)
    {
        maListElements.push(vcl::PDFWriter::LIBody);
        mpPDFExtOutDevData->WrapBeginStructureElement(vcl::PDFWriter::LIBody);
    }

    // directdraw of text simple portion; use default processing
    RenderTextSimpleOrDecoratedPortionPrimitive2D(rTextCandidate);

    if (mbInListItem && mbBulletPresent)
        mbBulletPresent = false;

    // restore DrawMode
    mpOutputDevice->SetDrawMode(nOriginalDrawMode);

    // #i101169# if(pTextDecoratedCandidate)
    {
        // support for TEXT_ MetaFile actions only for decorated texts
        if (!mxBreakIterator.get() || !mxBreakIterator.get()->get())
        {
            uno::Reference<uno::XComponentContext> xContext(
                ::comphelper::getProcessComponentContext());
            mxBreakIterator.set(i18n::BreakIterator::create(xContext));
        }
        auto& rBreakIterator = *mxBreakIterator.get()->get();

        const OUString& rTxt = rTextCandidate.getText();
        const sal_Int32 nTextLength(rTextCandidate.getTextLength()); // rTxt.getLength());

        if (nTextLength)
        {
            const css::lang::Locale& rLocale = rTextCandidate.getLocale();
            const sal_Int32 nTextPosition(rTextCandidate.getTextPosition());

            sal_Int32 nDone;
            sal_Int32 nNextCellBreak(rBreakIterator.nextCharacters(
                rTxt, nTextPosition, rLocale, css::i18n::CharacterIteratorMode::SKIPCELL, 0,
                nDone));
            css::i18n::Boundary nNextWordBoundary(rBreakIterator.getWordBoundary(
                rTxt, nTextPosition, rLocale, css::i18n::WordType::ANY_WORD, true));
            sal_Int32 nNextSentenceBreak(
                rBreakIterator.endOfSentence(rTxt, nTextPosition, rLocale));
            static constexpr OStringLiteral aCommentStringA("XTEXT_EOC");
            static constexpr OStringLiteral aCommentStringB("XTEXT_EOW");
            static constexpr OStringLiteral aCommentStringC("XTEXT_EOS");

            for (sal_Int32 i(nTextPosition); i < nTextPosition + nTextLength; i++)
            {
                // create the entries for the respective break positions
                if (i == nNextCellBreak)
                {
                    mpMetaFile->AddAction(
                        new MetaCommentAction(aCommentStringA, i - nTextPosition));
                    nNextCellBreak = rBreakIterator.nextCharacters(
                        rTxt, i, rLocale, css::i18n::CharacterIteratorMode::SKIPCELL, 1, nDone);
                }
                if (i == nNextWordBoundary.endPos)
                {
                    mpMetaFile->AddAction(
                        new MetaCommentAction(aCommentStringB, i - nTextPosition));
                    nNextWordBoundary = rBreakIterator.getWordBoundary(
                        rTxt, i + 1, rLocale, css::i18n::WordType::ANY_WORD, true);
                }
                if (i == nNextSentenceBreak)
                {
                    mpMetaFile->AddAction(
                        new MetaCommentAction(aCommentStringC, i - nTextPosition));
                    nNextSentenceBreak = rBreakIterator.endOfSentence(rTxt, i + 1, rLocale);
                }
            }
        }
    }
}

void VclMetafileProcessor2D::processPolygonHairlinePrimitive2D(
    const primitive2d::PolygonHairlinePrimitive2D& rHairlinePrimitive)
{
    const basegfx::B2DPolygon& rBasePolygon = rHairlinePrimitive.getB2DPolygon();

    if (rBasePolygon.count() > (MAX_POLYGON_POINT_COUNT_METAFILE - 1))
    {
        // #i112245# Metafiles use tools Polygon and are not able to have more than 65535 points
        // per polygon. If there are more, split the polygon in half and call recursively
        basegfx::B2DPolygon aLeft, aRight;
        splitLinePolygon(rBasePolygon, aLeft, aRight);
        rtl::Reference<primitive2d::PolygonHairlinePrimitive2D> xPLeft(
            new primitive2d::PolygonHairlinePrimitive2D(std::move(aLeft),
                                                        rHairlinePrimitive.getBColor()));
        rtl::Reference<primitive2d::PolygonHairlinePrimitive2D> xPRight(
            new primitive2d::PolygonHairlinePrimitive2D(std::move(aRight),
                                                        rHairlinePrimitive.getBColor()));

        processBasePrimitive2D(*xPLeft);
        processBasePrimitive2D(*xPRight);
    }
    else
    {
        // direct draw of hairline; use default processing
        // support SvtGraphicStroke MetaCommentAction
        const basegfx::BColor aLineColor(
            maBColorModifierStack.getModifiedColor(rHairlinePrimitive.getBColor()));
        std::unique_ptr<SvtGraphicStroke> pSvtGraphicStroke;

        // #i121267# Not needed, does not give better quality compared with
        // the MetaActionType::POLYPOLYGON written by RenderPolygonHairlinePrimitive2D
        // below
        const bool bSupportSvtGraphicStroke(false);

        if (bSupportSvtGraphicStroke)
        {
            pSvtGraphicStroke
                = impTryToCreateSvtGraphicStroke(rHairlinePrimitive.getB2DPolygon(), &aLineColor,
                                                 nullptr, nullptr, nullptr, nullptr);

            impStartSvtGraphicStroke(pSvtGraphicStroke.get());
        }

        RenderPolygonHairlinePrimitive2D(rHairlinePrimitive, false);

        if (bSupportSvtGraphicStroke)
        {
            impEndSvtGraphicStroke(pSvtGraphicStroke.get());
        }
    }
}

void VclMetafileProcessor2D::processPolygonStrokePrimitive2D(
    const primitive2d::PolygonStrokePrimitive2D& rStrokePrimitive)
{
    const basegfx::B2DPolygon& rBasePolygon = rStrokePrimitive.getB2DPolygon();

    if (rBasePolygon.count() > (MAX_POLYGON_POINT_COUNT_METAFILE - 1))
    {
        // #i112245# Metafiles use tools Polygon and are not able to have more than 65535 points
        // per polygon. If there are more, split the polygon in half and call recursively
        basegfx::B2DPolygon aLeft, aRight;
        splitLinePolygon(rBasePolygon, aLeft, aRight);
        rtl::Reference<primitive2d::PolygonStrokePrimitive2D> xPLeft(
            new primitive2d::PolygonStrokePrimitive2D(std::move(aLeft),
                                                      rStrokePrimitive.getLineAttribute(),
                                                      rStrokePrimitive.getStrokeAttribute()));
        rtl::Reference<primitive2d::PolygonStrokePrimitive2D> xPRight(
            new primitive2d::PolygonStrokePrimitive2D(std::move(aRight),
                                                      rStrokePrimitive.getLineAttribute(),
                                                      rStrokePrimitive.getStrokeAttribute()));

        processBasePrimitive2D(*xPLeft);
        processBasePrimitive2D(*xPRight);
    }
    else
    {
        mpOutputDevice->Push(vcl::PushFlags::LINECOLOR | vcl::PushFlags::FILLCOLOR);

        // support SvtGraphicStroke MetaCommentAction
        std::unique_ptr<SvtGraphicStroke> pSvtGraphicStroke = impTryToCreateSvtGraphicStroke(
            rBasePolygon, nullptr, &rStrokePrimitive.getLineAttribute(),
            &rStrokePrimitive.getStrokeAttribute(), nullptr, nullptr);

        impStartSvtGraphicStroke(pSvtGraphicStroke.get());
        const attribute::LineAttribute& rLine = rStrokePrimitive.getLineAttribute();

        // create MetaPolyLineActions, but without LineStyle::Dash
        if (basegfx::fTools::more(rLine.getWidth(), 0.0))
        {
            const attribute::StrokeAttribute& rStroke = rStrokePrimitive.getStrokeAttribute();

            const basegfx::BColor aHairlineColor(
                maBColorModifierStack.getModifiedColor(rLine.getColor()));
            mpOutputDevice->SetLineColor(Color(aHairlineColor));
            mpOutputDevice->SetFillColor();

            // use the transformed line width
            LineInfo aLineInfo(LineStyle::Solid,
                               basegfx::fround(getTransformedLineWidth(rLine.getWidth())));
            aLineInfo.SetLineJoin(rLine.getLineJoin());
            aLineInfo.SetLineCap(rLine.getLineCap());

            basegfx::B2DPolyPolygon aHairLinePolyPolygon;
            if (0.0 == rStroke.getFullDotDashLen())
            {
                aHairLinePolyPolygon.append(rBasePolygon);
            }
            else
            {
                bool done = false;
                const std::vector<double>& array = rStroke.getDotDashArray();
                // The dotdash array should generally have the form
                // (<dashLen> <distance>)+ (<dotLen> <distance>)*
                // (where (,),+ and * have their regex meaning).
                // Find out what the lengths and their counts are.
                if (!array.empty() && array.size() % 2 == 0)
                {
                    double dashLen = array[0];
                    double distance = array[1];
                    int dashCount = 1;
                    double dotLen = 0;
                    int dotCount = 0;
                    size_t pos = 2;
                    while (pos + 2 <= array.size())
                    {
                        if (array[pos] != dashLen || array[pos + 1] != distance)
                            break;
                        ++dashCount;
                        pos += 2;
                    }
                    if (pos + 2 <= array.size() && array[pos + 1] == distance)
                    {
                        dotLen = array[pos];
                        ++dotCount;
                        pos += 2;
                        while (pos + 2 <= array.size())
                        {
                            if (array[pos] != dotLen || array[pos + 1] != distance)
                                break;
                            ++dotCount;
                            pos += 2;
                        }
                    }
                    if (array.size() == pos)
                    {
                        aHairLinePolyPolygon.append(rBasePolygon);
                        // This will be used by setupStrokeAttributes() in cppcanvas.
                        aLineInfo.SetStyle(LineStyle::Dash);
                        aLineInfo.SetDashCount(dashCount);
                        aLineInfo.SetDashLen(getTransformedLineWidth(dashLen));
                        aLineInfo.SetDistance(getTransformedLineWidth(distance));
                        if (dotCount != 0)
                        {
                            aLineInfo.SetDotCount(dotCount);
                            aLineInfo.SetDotLen(getTransformedLineWidth(dotLen));
                        }
                        done = true;
                    }
                }
                if (!done)
                {
                    // LineInfo can hold only limited info about dashing, apply dashing manually
                    // if LineInfo cannot describe it. That should not happen though.
                    SAL_WARN("drawinglayer", "dotdash array cannot be converted to LineInfo");
                    basegfx::utils::applyLineDashing(rBasePolygon, rStroke.getDotDashArray(),
                                                     &aHairLinePolyPolygon, nullptr,
                                                     rStroke.getFullDotDashLen());
                }
            }
            aHairLinePolyPolygon.transform(maCurrentTransformation);

            for (sal_uInt32 a(0); a < aHairLinePolyPolygon.count(); a++)
            {
                const basegfx::B2DPolygon& aCandidate(aHairLinePolyPolygon.getB2DPolygon(a));

                if (aCandidate.count() > 1)
                {
                    const tools::Polygon aToolsPolygon(aCandidate);

                    mpMetaFile->AddAction(new MetaPolyLineAction(aToolsPolygon, aLineInfo));
                }
            }
        }
        else
        {
            process(rStrokePrimitive);
        }

        impEndSvtGraphicStroke(pSvtGraphicStroke.get());

        mpOutputDevice->Pop();
    }
}

void VclMetafileProcessor2D::processPolygonStrokeArrowPrimitive2D(
    const primitive2d::PolygonStrokeArrowPrimitive2D& rStrokeArrowPrimitive)
{
    const basegfx::B2DPolygon& rBasePolygon = rStrokeArrowPrimitive.getB2DPolygon();

    if (rBasePolygon.count() > (MAX_POLYGON_POINT_COUNT_METAFILE - 1))
    {
        // #i112245# Metafiles use tools Polygon and are not able to have more than 65535 points
        // per polygon. If there are more, split the polygon in half and call recursively
        basegfx::B2DPolygon aLeft, aRight;
        splitLinePolygon(rBasePolygon, aLeft, aRight);
        const attribute::LineStartEndAttribute aEmpty;
        rtl::Reference<primitive2d::PolygonStrokeArrowPrimitive2D> xPLeft(
            new primitive2d::PolygonStrokeArrowPrimitive2D(
                aLeft, rStrokeArrowPrimitive.getLineAttribute(),
                rStrokeArrowPrimitive.getStrokeAttribute(), rStrokeArrowPrimitive.getStart(),
                aEmpty));
        rtl::Reference<primitive2d::PolygonStrokeArrowPrimitive2D> xPRight(
            new primitive2d::PolygonStrokeArrowPrimitive2D(
                aRight, rStrokeArrowPrimitive.getLineAttribute(),
                rStrokeArrowPrimitive.getStrokeAttribute(), aEmpty,
                rStrokeArrowPrimitive.getEnd()));

        processBasePrimitive2D(*xPLeft);
        processBasePrimitive2D(*xPRight);
    }
    else
    {
        // support SvtGraphicStroke MetaCommentAction
        std::unique_ptr<SvtGraphicStroke> pSvtGraphicStroke = impTryToCreateSvtGraphicStroke(
            rBasePolygon, nullptr, &rStrokeArrowPrimitive.getLineAttribute(),
            &rStrokeArrowPrimitive.getStrokeAttribute(), &rStrokeArrowPrimitive.getStart(),
            &rStrokeArrowPrimitive.getEnd());

        // write LineGeometry start marker
        impStartSvtGraphicStroke(pSvtGraphicStroke.get());

        // #i116162# When B&W is set as DrawMode, DrawModeFlags::WhiteFill is used
        // to let all fills be just white; for lines DrawModeFlags::BlackLine is used
        // so all line geometry is supposed to get black. Since in the in-between
        // stages of line geometry drawing filled polygons are used (e.g. line
        // start/ends) it is necessary to change these drawmodes to preserve
        // that lines shall be black; thus change DrawModeFlags::WhiteFill to
        // DrawModeFlags::BlackFill during line geometry processing to have line geometry
        // parts filled black.
        const DrawModeFlags nOldDrawMode(mpOutputDevice->GetDrawMode());
        const bool bDrawmodeChange(nOldDrawMode & DrawModeFlags::WhiteFill
                                   && mnSvtGraphicStrokeCount);

        if (bDrawmodeChange)
        {
            mpOutputDevice->SetDrawMode((nOldDrawMode & ~DrawModeFlags::WhiteFill)
                                        | DrawModeFlags::BlackFill);
        }

        // process sub-line geometry (evtl. filled PolyPolygons)
        process(rStrokeArrowPrimitive);

        if (bDrawmodeChange)
        {
            mpOutputDevice->SetDrawMode(nOldDrawMode);
        }

        // write LineGeometry end marker
        impEndSvtGraphicStroke(pSvtGraphicStroke.get());
    }
}

void VclMetafileProcessor2D::processPolyPolygonGraphicPrimitive2D(
    const primitive2d::PolyPolygonGraphicPrimitive2D& rBitmapCandidate)
{
    // need to handle PolyPolygonGraphicPrimitive2D here to support XPATHFILL_SEQ_BEGIN/XPATHFILL_SEQ_END
    basegfx::B2DPolyPolygon aLocalPolyPolygon(rBitmapCandidate.getB2DPolyPolygon());

    if (!rBitmapCandidate.getDefinitionRange().isEmpty()
        && aLocalPolyPolygon.getB2DRange() != rBitmapCandidate.getDefinitionRange())
    {
        // The range which defines the bitmap fill is defined and different from the
        // range of the defining geometry (e.g. used for FillStyle UseSlideBackground).
        // This cannot be done calling vcl, thus use decomposition here directly
        process(rBitmapCandidate);
        return;
    }

    fillPolyPolygonNeededToBeSplit(aLocalPolyPolygon);

    std::unique_ptr<SvtGraphicFill> pSvtGraphicFill;

    if (!mnSvtGraphicFillCount && aLocalPolyPolygon.count())
    {
        // #121194# Changed implementation and checked usages of convert to metafile,
        // presentation start (uses SvtGraphicFill) and printing.

        // calculate transformation. Get real object size, all values in FillGraphicAttribute
        // are relative to the unified object
        aLocalPolyPolygon.transform(maCurrentTransformation);
        const basegfx::B2DVector aOutlineSize(aLocalPolyPolygon.getB2DRange().getRange());

        // the scaling needs scale from pixel to logic coordinate system
        const attribute::FillGraphicAttribute& rFillGraphicAttribute
            = rBitmapCandidate.getFillGraphic();
        const Size aBmpSizePixel(rFillGraphicAttribute.getGraphic().GetSizePixel());

        // setup transformation like in impgrfll. Multiply with aOutlineSize
        // to get from unit coordinates in rFillGraphicAttribute.getGraphicRange()
        // to object coordinates with object's top left being at (0,0). Divide
        // by pixel size so that scale from pixel to logic will work in SvtGraphicFill.
        const basegfx::B2DVector aTransformScale(
            rFillGraphicAttribute.getGraphicRange().getRange()
            / basegfx::B2DVector(std::max(1.0, double(aBmpSizePixel.Width())),
                                 std::max(1.0, double(aBmpSizePixel.Height())))
            * aOutlineSize);
        const basegfx::B2DPoint aTransformPosition(
            rFillGraphicAttribute.getGraphicRange().getMinimum() * aOutlineSize);

        // setup transformation like in impgrfll
        SvtGraphicFill::Transform aTransform;

        // scale values are divided by bitmap pixel sizes
        aTransform.matrix[0] = aTransformScale.getX();
        aTransform.matrix[4] = aTransformScale.getY();

        // translates are absolute
        aTransform.matrix[2] = aTransformPosition.getX();
        aTransform.matrix[5] = aTransformPosition.getY();

        pSvtGraphicFill.reset(new SvtGraphicFill(
            getFillPolyPolygon(aLocalPolyPolygon), Color(), 0.0, SvtGraphicFill::fillEvenOdd,
            SvtGraphicFill::fillTexture, aTransform, rFillGraphicAttribute.getTiling(),
            SvtGraphicFill::hatchSingle, Color(), SvtGraphicFill::GradientType::Linear, Color(),
            Color(), 0, rFillGraphicAttribute.getGraphic()));
    }

    // Do use decomposition; encapsulate with SvtGraphicFill
    impStartSvtGraphicFill(pSvtGraphicFill.get());
    process(rBitmapCandidate);
    impEndSvtGraphicFill(pSvtGraphicFill.get());
}

void VclMetafileProcessor2D::processPolyPolygonHatchPrimitive2D(
    const primitive2d::PolyPolygonHatchPrimitive2D& rHatchCandidate)
{
    // need to handle PolyPolygonHatchPrimitive2D here to support XPATHFILL_SEQ_BEGIN/XPATHFILL_SEQ_END
    const attribute::FillHatchAttribute& rFillHatchAttribute = rHatchCandidate.getFillHatch();
    basegfx::B2DPolyPolygon aLocalPolyPolygon(rHatchCandidate.getB2DPolyPolygon());

    if (aLocalPolyPolygon.getB2DRange() != rHatchCandidate.getDefinitionRange())
    {
        // the range which defines the hatch is different from the range of the
        // geometry (used for writer frames). This cannot be done calling vcl, thus use
        // decomposition here
        process(rHatchCandidate);
        return;
    }

    // #i112245# Metafiles use tools Polygon and are not able to have more than 65535 points
    // per polygon. Split polygon until there are less than that
    fillPolyPolygonNeededToBeSplit(aLocalPolyPolygon);

    if (rFillHatchAttribute.isFillBackground())
    {
        // with fixing #i111954# (see below) the possible background
        // fill of a hatched object was lost.Generate a background fill
        // primitive and render it
        const primitive2d::Primitive2DReference xBackground(
            new primitive2d::PolyPolygonColorPrimitive2D(aLocalPolyPolygon,
                                                         rHatchCandidate.getBackgroundColor()));

        process(primitive2d::Primitive2DContainer{ xBackground });
    }

    std::unique_ptr<SvtGraphicFill> pSvtGraphicFill;
    aLocalPolyPolygon.transform(maCurrentTransformation);

    if (!mnSvtGraphicFillCount && aLocalPolyPolygon.count())
    {
        // re-create a VCL hatch as base data
        SvtGraphicFill::HatchType eHatch(SvtGraphicFill::hatchSingle);

        switch (rFillHatchAttribute.getStyle())
        {
            default: // attribute::HatchStyle::Single :
            {
                eHatch = SvtGraphicFill::hatchSingle;
                break;
            }
            case attribute::HatchStyle::Double:
            {
                eHatch = SvtGraphicFill::hatchDouble;
                break;
            }
            case attribute::HatchStyle::Triple:
            {
                eHatch = SvtGraphicFill::hatchTriple;
                break;
            }
        }

        SvtGraphicFill::Transform aTransform;

        // scale
        aTransform.matrix[0] *= rFillHatchAttribute.getDistance();
        aTransform.matrix[4] *= rFillHatchAttribute.getDistance();

        // rotate (was never correct in impgrfll anyways, use correct angle now)
        aTransform.matrix[0] *= cos(rFillHatchAttribute.getAngle());
        aTransform.matrix[1] *= -sin(rFillHatchAttribute.getAngle());
        aTransform.matrix[3] *= sin(rFillHatchAttribute.getAngle());
        aTransform.matrix[4] *= cos(rFillHatchAttribute.getAngle());

        pSvtGraphicFill.reset(new SvtGraphicFill(
            getFillPolyPolygon(aLocalPolyPolygon), Color(), 0.0, SvtGraphicFill::fillEvenOdd,
            SvtGraphicFill::fillHatch, aTransform, false, eHatch,
            Color(maBColorModifierStack.getModifiedColor(rFillHatchAttribute.getColor())),
            SvtGraphicFill::GradientType::Linear, Color(), Color(), 0, Graphic()));
    }

    // Do use decomposition; encapsulate with SvtGraphicFill
    impStartSvtGraphicFill(pSvtGraphicFill.get());

    // #i111954# do NOT use decomposition, but use direct VCL-command
    // process(rCandidate.get2DDecomposition(getViewInformation2D()));
    const tools::PolyPolygon aToolsPolyPolygon(
        basegfx::utils::adaptiveSubdivideByAngle(aLocalPolyPolygon));
    const HatchStyle aHatchStyle(
        attribute::HatchStyle::Single == rFillHatchAttribute.getStyle()
            ? HatchStyle::Single
            : attribute::HatchStyle::Double == rFillHatchAttribute.getStyle() ? HatchStyle::Double
                                                                              : HatchStyle::Triple);

    mpOutputDevice->DrawHatch(
        aToolsPolyPolygon,
        Hatch(aHatchStyle,
              Color(maBColorModifierStack.getModifiedColor(rFillHatchAttribute.getColor())),
              basegfx::fround(rFillHatchAttribute.getDistance()),
              Degree10(basegfx::fround(basegfx::rad2deg<10>(rFillHatchAttribute.getAngle())))));

    impEndSvtGraphicFill(pSvtGraphicFill.get());
}

void VclMetafileProcessor2D::processPolyPolygonGradientPrimitive2D(
    const primitive2d::PolyPolygonGradientPrimitive2D& rGradientCandidate)
{
    bool useDecompose(false);

    if (!useDecompose)
    {
        basegfx::B2DVector aScale, aTranslate;
        double fRotate, fShearX;

        maCurrentTransformation.decompose(aScale, aTranslate, fRotate, fShearX);

        // detect if transformation is rotated, sheared or mirrored in X and/or Y
        if (!basegfx::fTools::equalZero(fRotate) || !basegfx::fTools::equalZero(fShearX)
            || aScale.getX() < 0.0 || aScale.getY() < 0.0)
        {
            // #i121185# When rotation or shear is used, a VCL Gradient cannot be used directly.
            // This is because VCL Gradient mechanism does *not* support to rotate the gradient
            // with objects and this case is not expressible in a Metafile (and cannot be added
            // since the FileFormats used, e.g. *.wmf, do not support it either).
            // Such cases happen when a graphic object uses a Metafile as graphic information or
            // a fill style definition uses a Metafile. In this cases the graphic content is
            // rotated with the graphic or filled object; this is not supported by the target
            // format of this conversion renderer - Metafiles.
            // To solve this, not a Gradient is written, but the decomposition of this object
            // is written to the Metafile. This is the PolyPolygons building the gradient fill.
            // These will need more space and time, but the result will be as if the Gradient
            // was rotated with the object.
            // This mechanism is used by all exporters still not using Primitives (e.g. Print,
            // Slideshow, Export rto PDF, export to Picture, ...) but relying on Metafile
            // transfers. One more reason to *change* these to primitives.
            // BTW: One more example how useful the principles of primitives are; the decomposition
            // is by definition a simpler, maybe more expensive representation of the same content.
            useDecompose = true;
        }
    }

    // tdf#150551 for PDF export, use the decomposition for better gradient visualization
    if (!useDecompose && nullptr != mpPDFExtOutDevData)
    {
        useDecompose = true;
    }

    basegfx::B2DPolyPolygon aLocalPolyPolygon(rGradientCandidate.getB2DPolyPolygon());

    if (!useDecompose && aLocalPolyPolygon.getB2DRange() != rGradientCandidate.getDefinitionRange())
    {
        // the range which defines the gradient is different from the range of the
        // geometry (used for writer frames). This cannot be done calling vcl, thus use
        // decomposition here
        useDecompose = true;
    }

    const attribute::FillGradientAttribute& rFillGradient(rGradientCandidate.getFillGradient());

    if (!useDecompose && rFillGradient.cannotBeHandledByVCL())
    {
        // MCGR: if we have ColorStops, do not try to fallback to old VCL-Gradient,
        // that will *not* be capable of representing this properly. Use the
        // correct decomposition instead
        useDecompose = true;
    }

    if (useDecompose)
    {
        GDIMetaFile* pMetaFile(mpOutputDevice->GetConnectMetaFile());

        // tdf#155479 only add 'BGRAD_SEQ_BEGIN' if SVG export
        if (nullptr != pMetaFile && pMetaFile->getSVG())
        {
            // write the color stops to a memory stream
            SvMemoryStream aMemStm;
            VersionCompatWrite aCompat(aMemStm, 1);

            const basegfx::BColorStops& rColorStops(rFillGradient.getColorStops());
            sal_uInt16 nTmp(sal::static_int_cast<sal_uInt16>(rColorStops.size()));
            aMemStm.WriteUInt16(nTmp);

            for (auto const& rCand : rColorStops)
            {
                aMemStm.WriteDouble(rCand.getStopOffset());
                const basegfx::BColor& rColor(rCand.getStopColor());
                aMemStm.WriteDouble(rColor.getRed());
                aMemStm.WriteDouble(rColor.getGreen());
                aMemStm.WriteDouble(rColor.getBlue());
            }

            // Add a new MetaCommentAction section of type 'BGRAD_SEQ_BEGIN/BGRAD_SEQ_END'
            // that is capable of holding the new color step information, plus the
            // already used MetaActionType::GRADIENTEX.
            // With that combination only places that know about that new BGRAD_SEQ_* will
            // use it while all others will work on the created decomposition of the
            // gradient for compatibility - which are single-color filled polygons
            pMetaFile->AddAction(new MetaCommentAction(
                "BGRAD_SEQ_BEGIN"_ostr, 0, static_cast<const sal_uInt8*>(aMemStm.GetData()),
                aMemStm.TellEnd()));

            // create MetaActionType::GRADIENTEX
            // NOTE: with the new BGRAD_SEQ_* we could use basegfx::B2DPolygon and
            // basegfx::BGradient here directly, but may have to add streaming OPs
            // for these, so for now just go with what we use all the time. The real
            // work for improvement should not go to this 'compromize' but to a real
            // re-work of the SVG export (or/and others) to no longer work on metafiles
            // but on UNO API or primitives (whatever fits best to the specific export)
            fillPolyPolygonNeededToBeSplit(aLocalPolyPolygon);
            Gradient aVCLGradient;
            impConvertFillGradientAttributeToVCLGradient(aVCLGradient, rFillGradient, false);
            aLocalPolyPolygon.transform(maCurrentTransformation);
            const tools::PolyPolygon aToolsPolyPolygon(
                getFillPolyPolygon(basegfx::utils::adaptiveSubdivideByAngle(aLocalPolyPolygon)));
            mpOutputDevice->DrawGradient(aToolsPolyPolygon, aVCLGradient);
        }

        // use decompose to draw, will create PolyPolygon ColorFill actions
        process(rGradientCandidate);

        // tdf#155479 only add 'BGRAD_SEQ_END' if SVG export
        if (nullptr != pMetaFile && pMetaFile->getSVG())
        {
            // close the BGRAD_SEQ_* actions range
            pMetaFile->AddAction(new MetaCommentAction("BGRAD_SEQ_END"_ostr));
        }

        return;
    }

    // #i112245# Metafiles use tools Polygon and are not able to have more than 65535 points
    // per polygon. Split polygon until there are less than that
    fillPolyPolygonNeededToBeSplit(aLocalPolyPolygon);

    // for support of MetaCommentActions of the form XGRAD_SEQ_BEGIN, XGRAD_SEQ_END
    // it is safest to use the VCL OutputDevice::DrawGradient method which creates those.
    // re-create a VCL-gradient from FillGradientPrimitive2D and the needed tools PolyPolygon
    Gradient aVCLGradient;
    impConvertFillGradientAttributeToVCLGradient(aVCLGradient, rFillGradient, false);
    aLocalPolyPolygon.transform(maCurrentTransformation);

    // #i82145# ATM VCL printing of gradients using curved shapes does not work,
    // i submitted the bug with the given ID to THB. When that task is fixed it is
    // necessary to again remove this subdivision since it decreases possible
    // printing quality (not even resolution-dependent for now). THB will tell
    // me when that task is fixed in the master
    const tools::PolyPolygon aToolsPolyPolygon(
        getFillPolyPolygon(basegfx::utils::adaptiveSubdivideByAngle(aLocalPolyPolygon)));

    // XPATHFILL_SEQ_BEGIN/XPATHFILL_SEQ_END support
    std::unique_ptr<SvtGraphicFill> pSvtGraphicFill;

    if (!mnSvtGraphicFillCount && aLocalPolyPolygon.count())
    {
        // setup gradient stuff like in impgrfll
        SvtGraphicFill::GradientType eGrad(SvtGraphicFill::GradientType::Linear);

        switch (aVCLGradient.GetStyle())
        {
            default: // css::awt::GradientStyle_LINEAR:
            case css::awt::GradientStyle_AXIAL:
                eGrad = SvtGraphicFill::GradientType::Linear;
                break;
            case css::awt::GradientStyle_RADIAL:
            case css::awt::GradientStyle_ELLIPTICAL:
                eGrad = SvtGraphicFill::GradientType::Radial;
                break;
            case css::awt::GradientStyle_SQUARE:
            case css::awt::GradientStyle_RECT:
                eGrad = SvtGraphicFill::GradientType::Rectangular;
                break;
        }

        pSvtGraphicFill.reset(new SvtGraphicFill(
            aToolsPolyPolygon, Color(), 0.0, SvtGraphicFill::fillEvenOdd,
            SvtGraphicFill::fillGradient, SvtGraphicFill::Transform(), false,
            SvtGraphicFill::hatchSingle, Color(), eGrad, aVCLGradient.GetStartColor(),
            aVCLGradient.GetEndColor(), aVCLGradient.GetSteps(), Graphic()));
    }

    // call VCL directly; encapsulate with SvtGraphicFill
    impStartSvtGraphicFill(pSvtGraphicFill.get());
    mpOutputDevice->DrawGradient(aToolsPolyPolygon, aVCLGradient);
    impEndSvtGraphicFill(pSvtGraphicFill.get());
}

void VclMetafileProcessor2D::processPolyPolygonColorPrimitive2D(
    const primitive2d::PolyPolygonColorPrimitive2D& rPolygonCandidate)
{
    mpOutputDevice->Push(vcl::PushFlags::LINECOLOR | vcl::PushFlags::FILLCOLOR);
    basegfx::B2DPolyPolygon aLocalPolyPolygon(rPolygonCandidate.getB2DPolyPolygon());

    // #i112245# Metafiles use tools Polygon and are not able to have more than 65535 points
    // per polygon. Split polygon until there are less than that
    fillPolyPolygonNeededToBeSplit(aLocalPolyPolygon);

    const basegfx::BColor aPolygonColor(
        maBColorModifierStack.getModifiedColor(rPolygonCandidate.getBColor()));
    aLocalPolyPolygon.transform(maCurrentTransformation);

    // set line and fill color
    mpOutputDevice->SetFillColor(Color(aPolygonColor));
    mpOutputDevice->SetLineColor();

    mpOutputDevice->DrawPolyPolygon(aLocalPolyPolygon);

    mpOutputDevice->Pop();
}

void VclMetafileProcessor2D::processMaskPrimitive2D(
    const primitive2d::MaskPrimitive2D& rMaskCandidate)
{
    // mask group. Special handling for MetaFiles.
    if (rMaskCandidate.getChildren().empty())
        return;

    basegfx::B2DPolyPolygon aMask(rMaskCandidate.getMask());

    if (aMask.count())
    {
        // prepare new mask polygon and rescue current one
        aMask.transform(maCurrentTransformation);
        const basegfx::B2DPolyPolygon aLastClipPolyPolygon(maClipPolyPolygon);

        if (maClipPolyPolygon.count())
        {
            // there is already a clip polygon set; build clipped union of
            // current mask polygon and new one
            maClipPolyPolygon = basegfx::utils::clipPolyPolygonOnPolyPolygon(
                aMask, maClipPolyPolygon,
                true, // #i106516# we want the inside of aMask, not the outside
                false);
        }
        else
        {
            // use mask directly
            maClipPolyPolygon = aMask;
        }

        if (maClipPolyPolygon.count())
        {
            // set VCL clip region; subdivide before conversion to tools polygon. Subdivision necessary (!)
            // Removed subdivision and fixed in vcl::Region::ImplPolyPolyRegionToBandRegionFunc() in VCL where
            // the ClipRegion is built from the Polygon. An AdaptiveSubdivide on the source polygon was missing there
            mpOutputDevice->Push(vcl::PushFlags::CLIPREGION);
            mpOutputDevice->SetClipRegion(vcl::Region(maClipPolyPolygon));

            // recursively paint content
            // #i121267# Only need to process sub-content when clip polygon is *not* empty.
            // If it is empty, the clip is empty and there can be nothing inside.
            process(rMaskCandidate.getChildren());

            // restore VCL clip region
            mpOutputDevice->Pop();
        }

        // restore to rescued clip polygon
        maClipPolyPolygon = aLastClipPolyPolygon;
    }
    else
    {
        // no mask, no clipping. recursively paint content
        process(rMaskCandidate.getChildren());
    }
}

void VclMetafileProcessor2D::processUnifiedTransparencePrimitive2D(
    const primitive2d::UnifiedTransparencePrimitive2D& rUniTransparenceCandidate)
{
    mpOutputDevice->Push(vcl::PushFlags::LINECOLOR | vcl::PushFlags::FILLCOLOR);
    // for metafile: Need to examine what the pure vcl version is doing here actually
    // - uses DrawTransparent with metafile for content and a gradient
    // - uses DrawTransparent for single PolyPolygons directly. Can be detected by
    //   checking the content for single PolyPolygonColorPrimitive2D
    const primitive2d::Primitive2DContainer& rContent = rUniTransparenceCandidate.getChildren();

    if (!rContent.empty())
    {
        if (0.0 == rUniTransparenceCandidate.getTransparence())
        {
            // not transparent at all, use content
            process(rUniTransparenceCandidate.getChildren());
        }
        else if (rUniTransparenceCandidate.getTransparence() > 0.0
                 && rUniTransparenceCandidate.getTransparence() < 1.0)
        {
            // try to identify a single PolyPolygonColorPrimitive2D in the
            // content part of the transparence primitive
            const primitive2d::PolyPolygonColorPrimitive2D* pPoPoColor = nullptr;
            static bool bForceToMetafile(false); // loplugin:constvars:ignore

            if (!bForceToMetafile && 1 == rContent.size())
            {
                const primitive2d::Primitive2DReference xReference(rContent[0]);
                pPoPoColor = dynamic_cast<const primitive2d::PolyPolygonColorPrimitive2D*>(
                    xReference.get());
            }

            // PolyPolygonGradientPrimitive2D, PolyPolygonHatchPrimitive2D and
            // PolyPolygonGraphicPrimitive2D are derived from PolyPolygonColorPrimitive2D.
            // Check also for correct ID to exclude derived implementations
            if (pPoPoColor
                && PRIMITIVE2D_ID_POLYPOLYGONCOLORPRIMITIVE2D == pPoPoColor->getPrimitive2DID())
            {
                // single transparent tools::PolyPolygon identified, use directly
                const basegfx::BColor aPolygonColor(
                    maBColorModifierStack.getModifiedColor(pPoPoColor->getBColor()));
                basegfx::B2DPolyPolygon aLocalPolyPolygon(pPoPoColor->getB2DPolyPolygon());

                // #i112245# Metafiles use tools Polygon and are not able to have more than 65535 points
                // per polygon. Split polygon until there are less than that
                fillPolyPolygonNeededToBeSplit(aLocalPolyPolygon);

                // now transform
                aLocalPolyPolygon.transform(maCurrentTransformation);

                // set line and fill color
                const sal_uInt16 nTransPercentVcl(static_cast<sal_uInt16>(
                    basegfx::fround(rUniTransparenceCandidate.getTransparence() * 100.0)));
                mpOutputDevice->SetFillColor(Color(aPolygonColor));
                mpOutputDevice->SetLineColor();

                mpOutputDevice->DrawTransparent(tools::PolyPolygon(aLocalPolyPolygon),
                                                nTransPercentVcl);
            }
            else
            {
                // save old mfCurrentUnifiedTransparence and set new one
                // so that contained SvtGraphicStroke may use the current one
                const double fLastCurrentUnifiedTransparence(mfCurrentUnifiedTransparence);
                // #i105377# paint the content metafile opaque as the transparency gets
                // split of into the gradient below
                // mfCurrentUnifiedTransparence = rUniTransparenceCandidate.getTransparence();
                mfCurrentUnifiedTransparence = 0;

                // various content, create content-metafile
                GDIMetaFile aContentMetafile;

                // tdf#155479 always forward propagate SVG flag for sub-content,
                // it may contain cannotBeHandledByVCL gradients or transparencyGradients
                aContentMetafile.setSVG(mpOutputDevice->GetConnectMetaFile()->getSVG());

                const tools::Rectangle aPrimitiveRectangle(
                    impDumpToMetaFile(rContent, aContentMetafile));

                // restore mfCurrentUnifiedTransparence; it may have been used
                // while processing the sub-content in impDumpToMetaFile
                mfCurrentUnifiedTransparence = fLastCurrentUnifiedTransparence;

                // create uniform VCL gradient for uniform transparency
                Gradient aVCLGradient;
                const sal_uInt8 nTransPercentVcl(static_cast<sal_uInt8>(
                    basegfx::fround(rUniTransparenceCandidate.getTransparence() * 255.0)));
                const Color aTransColor(nTransPercentVcl, nTransPercentVcl, nTransPercentVcl);

                aVCLGradient.SetStyle(css::awt::GradientStyle_LINEAR);
                aVCLGradient.SetStartColor(aTransColor);
                aVCLGradient.SetEndColor(aTransColor);
                aVCLGradient.SetAngle(0_deg10);
                aVCLGradient.SetBorder(0);
                aVCLGradient.SetOfsX(0);
                aVCLGradient.SetOfsY(0);
                aVCLGradient.SetStartIntensity(100);
                aVCLGradient.SetEndIntensity(100);
                aVCLGradient.SetSteps(2);

                // render it to VCL
                mpOutputDevice->DrawTransparent(aContentMetafile, aPrimitiveRectangle.TopLeft(),
                                                aPrimitiveRectangle.GetSize(), aVCLGradient);
            }
        }
    }

    mpOutputDevice->Pop();
}

void VclMetafileProcessor2D::processTransparencePrimitive2D(
    const primitive2d::TransparencePrimitive2D& rTransparenceCandidate)
{
    // for metafile: Need to examine what the pure vcl version is doing here actually
    // - uses DrawTransparent with metafile for content and a gradient
    // i can detect this here with checking the gradient part for a single
    // FillGradientPrimitive2D and reconstruct the gradient.
    // If that detection goes wrong, I have to create a transparence-blended bitmap. Eventually
    // do that in stripes, else RenderTransparencePrimitive2D may just be used
    const primitive2d::Primitive2DContainer& rContent(rTransparenceCandidate.getChildren());
    const primitive2d::Primitive2DContainer& rTransparence(
        rTransparenceCandidate.getTransparence());

    if (rContent.empty() || rTransparence.empty())
        return;

    // try to identify a single FillGradientPrimitive2D in the
    // transparence part of the primitive. The hope is to handle
    // the more specific case in a better way than the general
    // TransparencePrimitive2D which has strongly separated
    // definitions for transparency and content, both completely
    // free definable by primitives
    const primitive2d::FillGradientPrimitive2D* pFiGradient(nullptr);
    static bool bForceToBigTransparentVDev(false); // loplugin:constvars:ignore

    // check for single FillGradientPrimitive2D
    if (!bForceToBigTransparentVDev && 1 == rTransparence.size())
    {
        pFiGradient
            = dynamic_cast<const primitive2d::FillGradientPrimitive2D*>(rTransparence[0].get());

        // check also for correct ID to exclude derived implementations
        if (pFiGradient
            && PRIMITIVE2D_ID_FILLGRADIENTPRIMITIVE2D != pFiGradient->getPrimitive2DID())
            pFiGradient = nullptr;
    }

    // tdf#155479 preps for holding extra-MCGR infos
    bool bSVGTransparencyColorStops(false);
    basegfx::BColorStops aSVGTransparencyColorStops;

    // MCGR: tdf#155437 If we have identified a transparency gradient,
    // check if VCL is able to handle it at all
    if (nullptr != pFiGradient && pFiGradient->getFillGradient().cannotBeHandledByVCL())
    {
        // If not, reset the pointer and do not make use of this special case.
        // Adding a gradient in incomplete state that can not be handled by vcl
        // makes no sense and will knowingly lead to errors, especially with
        // MCGR extended possibilities. I checked what happens with the
        // MetaFloatTransparentAction added by OutputDevice::DrawTransparent, but
        // in most cases it gets converted to bitmap or even ignored, see e.g.
        // - vcl/source/gdi/pdfwriter_impl2.cxx for PDF export
        // - vcl/source/filter/wmf/wmfwr.cxx -> does ignore TransparenceGradient completely
        //   - vcl/source/filter/wmf/emfwr.cxx -> same
        //   - vcl/source/filter/eps/eps.cxx -> same
        // NOTE: Theoretically it would be possible to make the new extended Gradient data
        // available in metafiles, with the known limitations (not backward comp, all
        // places using it would need adaption, ...), but combined with knowing that nearly
        // all usages ignore or render it locally anyways makes that a non-option.

        // tdf#155479 Yepp, as already mentioned above we need to add
        // some MCGR infos in case of SVG export, prepare that here
        if (mpOutputDevice->GetConnectMetaFile()->getSVG())
        {
            // for SVG, do not use decompose & prep extra data
            bSVGTransparencyColorStops = true;
            aSVGTransparencyColorStops = pFiGradient->getFillGradient().getColorStops();
        }
        else
        {
            // use decomposition
            pFiGradient = nullptr;
        }
    }

    if (nullptr != pFiGradient)
    {
        // this combination of Gradient can be expressed/handled by
        // vcl/metafile, so add it directly. various content, create content-metafile
        GDIMetaFile aContentMetafile;

        // tdf#155479 always forward propagate SVG flag for sub-content,
        // it may contain cannotBeHandledByVCL gradients or transparencyGradients
        aContentMetafile.setSVG(mpOutputDevice->GetConnectMetaFile()->getSVG());

        const tools::Rectangle aPrimitiveRectangle(impDumpToMetaFile(rContent, aContentMetafile));

        // re-create a VCL-gradient from FillGradientPrimitive2D
        Gradient aVCLGradient;
        impConvertFillGradientAttributeToVCLGradient(aVCLGradient, pFiGradient->getFillGradient(),
                                                     true);

        if (bSVGTransparencyColorStops)
        {
            // tdf#155479 create action directly & add extra
            // MCGR infos to the metafile, do that by adding - ONLY in
            // case of SVG export - to the MetaFileAction. For that
            // reason, do what OutputDevice::DrawTransparent will do,
            // but locally.
            // NOTE: That would be good for this whole
            // VclMetafileProcessor2D anyways to allow to get it
            // completely independent from OutputDevice in the long run
            GDIMetaFile* pMetaFile(mpOutputDevice->GetConnectMetaFile());
            rtl::Reference<::MetaFloatTransparentAction> pAction(
                new MetaFloatTransparentAction(aContentMetafile, aPrimitiveRectangle.TopLeft(),
                                               aPrimitiveRectangle.GetSize(), aVCLGradient));

            pAction->addSVGTransparencyColorStops(aSVGTransparencyColorStops);
            pMetaFile->AddAction(pAction);
        }
        else
        {
            // render it to VCL (creates MetaFloatTransparentAction)
            mpOutputDevice->DrawTransparent(aContentMetafile, aPrimitiveRectangle.TopLeft(),
                                            aPrimitiveRectangle.GetSize(), aVCLGradient);
        }
        return;
    }

    // Here we need to create a correct replacement visualization for the
    // TransparencePrimitive2D for the target metafile.
    // I replaced the n'th iteration to convert-to-bitmap which was
    // used here by using the existing tooling. The orig here was also producing
    // transparency errors with test-file from tdf#155437 on the right part of the
    // image.
    // Just rely on existing tooling doing the right thing in one place, so also
    // corrections/optimizations can be in one single place

    // Start by getting logic range of content, transform object-to-world, then world-to-view
    // to get to discrete values ('pixels'). Matrix multiplication is right-to-left (and not
    // commutative)
    basegfx::B2DRange aLogicRange(rTransparenceCandidate.getB2DRange(getViewInformation2D()));
    aLogicRange.transform(mpOutputDevice->GetViewTransformation() * maCurrentTransformation);

    // expand in discrete coordinates to next-bigger 'pixel' boundaries and remember
    // created discrete range
    aLogicRange.expand(
        basegfx::B2DPoint(floor(aLogicRange.getMinX()), floor(aLogicRange.getMinY())));
    aLogicRange.expand(basegfx::B2DPoint(ceil(aLogicRange.getMaxX()), ceil(aLogicRange.getMaxY())));
    const basegfx::B2DRange aDiscreteRange(aLogicRange);

    // transform back from discrete to world coordinates: this creates the
    // pixel-boundaries extended logic range we need to cover all content
    // reliably
    aLogicRange.transform(mpOutputDevice->GetInverseViewTransformation());

    // create transform embedding for renderer. Goal is to translate what we
    // want to paint to top/left 0/0 and the calculated discrete size
    basegfx::B2DHomMatrix aEmbedding(basegfx::utils::createTranslateB2DHomMatrix(
        -aLogicRange.getMinX(), -aLogicRange.getMinY()));
    const double fLogicWidth(
        basegfx::fTools::equalZero(aLogicRange.getWidth()) ? 1.0 : aLogicRange.getWidth());
    const double fLogicHeight(
        basegfx::fTools::equalZero(aLogicRange.getHeight()) ? 1.0 : aLogicRange.getHeight());
    aEmbedding.scale(aDiscreteRange.getWidth() / fLogicWidth,
                     aDiscreteRange.getHeight() / fLogicHeight);

    // use the whole TransparencePrimitive2D as input (no need to create a new
    // one with the sub-contents, these are ref-counted) and add to embedding
    // primitive2d::TransparencePrimitive2D& rTrCand();
    primitive2d::Primitive2DContainer xEmbedSeq{ &const_cast<primitive2d::TransparencePrimitive2D&>(
        rTransparenceCandidate) };

    // tdf#158743 when embedding, do not forget to 1st apply the evtl. used
    // CurrentTransformation (right-to-left, apply that 1st)
    xEmbedSeq = primitive2d::Primitive2DContainer{ new primitive2d::TransformPrimitive2D(
        aEmbedding * maCurrentTransformation, std::move(xEmbedSeq)) };

    // use empty ViewInformation & a useful MaximumQuadraticPixels
    // limitation to paint the content
    const auto aViewInformation2D(geometry::createViewInformation2D({}));
    const sal_uInt32 nMaximumQuadraticPixels(500000);
    const BitmapEx aBitmapEx(convertToBitmapEx(
        std::move(xEmbedSeq), aViewInformation2D, basegfx::fround(aDiscreteRange.getWidth()),
        basegfx::fround(aDiscreteRange.getHeight()), nMaximumQuadraticPixels));

    // add to target metafile (will create MetaFloatTransparentAction)
    mpOutputDevice->DrawBitmapEx(
        Point(basegfx::fround(aLogicRange.getMinX()), basegfx::fround(aLogicRange.getMinY())),
        Size(basegfx::fround(aLogicRange.getWidth()), basegfx::fround(aLogicRange.getHeight())),
        aBitmapEx);
}

void VclMetafileProcessor2D::processStructureTagPrimitive2D(
    const primitive2d::StructureTagPrimitive2D& rStructureTagCandidate)
{
    ::comphelper::ValueRestorationGuard const g(mpCurrentStructureTag, &rStructureTagCandidate);

    // structured tag primitive
    const vcl::PDFWriter::StructElement& rTagElement(rStructureTagCandidate.getStructureElement());
    bool bTagUsed((vcl::PDFWriter::NonStructElement != rTagElement));
    ::std::optional<sal_Int32> oAnchorParent;

    if (!rStructureTagCandidate.isTaggedSdrObject())
    {
        bTagUsed = false;
    }

    if (mpPDFExtOutDevData && bTagUsed)
    {
        // foreground object: tag as regular structure element
        if (!rStructureTagCandidate.isBackground())
        {
            if (rStructureTagCandidate.GetAnchorStructureElementKey() != nullptr)
            {
                sal_Int32 const id = mpPDFExtOutDevData->EnsureStructureElement(
                    rStructureTagCandidate.GetAnchorStructureElementKey());
                oAnchorParent.emplace(mpPDFExtOutDevData->GetCurrentStructureElement());
                mpPDFExtOutDevData->SetCurrentStructureElement(id);
            }
            mpPDFExtOutDevData->WrapBeginStructureElement(rTagElement);
            switch (rTagElement)
            {
                case vcl::PDFWriter::H1:
                case vcl::PDFWriter::H2:
                case vcl::PDFWriter::H3:
                case vcl::PDFWriter::H4:
                case vcl::PDFWriter::H5:
                case vcl::PDFWriter::H6:
                case vcl::PDFWriter::Paragraph:
                case vcl::PDFWriter::Heading:
                case vcl::PDFWriter::Caption:
                case vcl::PDFWriter::BlockQuote:
                case vcl::PDFWriter::Table:
                case vcl::PDFWriter::TableRow:
                case vcl::PDFWriter::Formula:
                case vcl::PDFWriter::Figure:
                case vcl::PDFWriter::Annot:
                    mpPDFExtOutDevData->SetStructureAttribute(vcl::PDFWriter::Placement,
                                                              vcl::PDFWriter::Block);
                    break;
                case vcl::PDFWriter::TableData:
                case vcl::PDFWriter::TableHeader:
                    mpPDFExtOutDevData->SetStructureAttribute(vcl::PDFWriter::Placement,
                                                              vcl::PDFWriter::Inline);
                    break;
                default:
                    break;
            }
            switch (rTagElement)
            {
                case vcl::PDFWriter::Table:
                case vcl::PDFWriter::Formula:
                case vcl::PDFWriter::Figure:
                case vcl::PDFWriter::Annot:
                {
                    auto const range(rStructureTagCandidate.getB2DRange(getViewInformation2D()));
                    tools::Rectangle const aLogicRect(
                        basegfx::fround(range.getMinX()), basegfx::fround(range.getMinY()),
                        basegfx::fround(range.getMaxX()), basegfx::fround(range.getMaxY()));
                    mpPDFExtOutDevData->SetStructureBoundingBox(aLogicRect);
                    break;
                }
                default:
                    break;
            }
            if (rTagElement == vcl::PDFWriter::Annot)
            {
                mpPDFExtOutDevData->SetStructureAnnotIds(rStructureTagCandidate.GetAnnotIds());
            }
            if (rTagElement == vcl::PDFWriter::TableHeader)
            {
                mpPDFExtOutDevData->SetStructureAttribute(vcl::PDFWriter::Scope,
                                                          vcl::PDFWriter::Column);
            }
        }
        // background object
        else
        {
            // background image: tag as artifact
            if (rStructureTagCandidate.isImage())
                mpPDFExtOutDevData->WrapBeginStructureElement(vcl::PDFWriter::NonStructElement);
            // any other background object: do not tag
            else
                assert(false);
        }
    }

    // process children normally
    process(rStructureTagCandidate.getChildren());

    if (mpPDFExtOutDevData && bTagUsed)
    {
        // write end tag
        mpPDFExtOutDevData->EndStructureElement();
        if (oAnchorParent)
        {
            mpPDFExtOutDevData->SetCurrentStructureElement(*oAnchorParent);
        }
    }
}

} // end of namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
