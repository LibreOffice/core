/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <drawinglayer/processor2d/cairopixelprocessor2d.hxx>
#include <sal/log.hxx>
#include <vcl/cairo.hxx>
#include <vcl/outdev.hxx>
#include <vcl/svapp.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <drawinglayer/primitive2d/drawinglayer_primitivetypes2d.hxx>
#include <drawinglayer/primitive2d/PolyPolygonColorPrimitive2D.hxx>
#include <drawinglayer/primitive2d/PolygonHairlinePrimitive2D.hxx>
#include <drawinglayer/primitive2d/bitmapprimitive2d.hxx>
#include <drawinglayer/primitive2d/unifiedtransparenceprimitive2d.hxx>
#include <drawinglayer/primitive2d/backgroundcolorprimitive2d.hxx>
#include <drawinglayer/primitive2d/baseprimitive2d.hxx>
#include <drawinglayer/primitive2d/markerarrayprimitive2d.hxx>
#include <drawinglayer/primitive2d/maskprimitive2d.hxx>
#include <drawinglayer/primitive2d/modifiedcolorprimitive2d.hxx>
#include <drawinglayer/primitive2d/pointarrayprimitive2d.hxx>
#include <drawinglayer/primitive2d/PolygonStrokePrimitive2D.hxx>
#include <drawinglayer/primitive2d/Tools.hxx>
#include <drawinglayer/primitive2d/transformprimitive2d.hxx>
#include <drawinglayer/primitive2d/transparenceprimitive2d.hxx>
#include <drawinglayer/converters.hxx>
#include <basegfx/curve/b2dcubicbezier.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <basegfx/utils/systemdependentdata.hxx>
#include <vcl/BitmapReadAccess.hxx>

using namespace com::sun::star;

namespace
{
basegfx::B2DPoint impPixelSnap(const basegfx::B2DPolygon& rPolygon,
                               const drawinglayer::geometry::ViewInformation2D& rViewInformation,
                               sal_uInt32 nIndex)
{
    const sal_uInt32 nCount(rPolygon.count());

    // get the data
    const basegfx::B2ITuple aPrevTuple(
        basegfx::fround(rViewInformation.getObjectToViewTransformation()
                        * rPolygon.getB2DPoint((nIndex + nCount - 1) % nCount)));
    const basegfx::B2DPoint aCurrPoint(rViewInformation.getObjectToViewTransformation()
                                       * rPolygon.getB2DPoint(nIndex));
    const basegfx::B2ITuple aCurrTuple(basegfx::fround(aCurrPoint));
    const basegfx::B2ITuple aNextTuple(
        basegfx::fround(rViewInformation.getObjectToViewTransformation()
                        * rPolygon.getB2DPoint((nIndex + 1) % nCount)));

    // get the states
    const bool bPrevVertical(aPrevTuple.getX() == aCurrTuple.getX());
    const bool bNextVertical(aNextTuple.getX() == aCurrTuple.getX());
    const bool bPrevHorizontal(aPrevTuple.getY() == aCurrTuple.getY());
    const bool bNextHorizontal(aNextTuple.getY() == aCurrTuple.getY());
    const bool bSnapX(bPrevVertical || bNextVertical);
    const bool bSnapY(bPrevHorizontal || bNextHorizontal);

    if (bSnapX || bSnapY)
    {
        basegfx::B2DPoint aSnappedPoint(bSnapX ? aCurrTuple.getX() : aCurrPoint.getX(),
                                        bSnapY ? aCurrTuple.getY() : aCurrPoint.getY());

        aSnappedPoint *= rViewInformation.getInverseObjectToViewTransformation();

        return aSnappedPoint;
    }

    return rPolygon.getB2DPoint(nIndex);
}

void addB2DPolygonToPathGeometry(cairo_t* cr, const basegfx::B2DPolygon& rPolygon,
                                 const drawinglayer::geometry::ViewInformation2D* pViewInformation)
{
    // short circuit if there is nothing to do
    const sal_uInt32 nPointCount(rPolygon.count());

    const bool bHasCurves(rPolygon.areControlPointsUsed());
    const bool bClosePath(rPolygon.isClosed());
    basegfx::B2DPoint aLast;

    for (sal_uInt32 nPointIdx = 0, nPrevIdx = 0;; nPrevIdx = nPointIdx++)
    {
        int nClosedIdx = nPointIdx;
        if (nPointIdx >= nPointCount)
        {
            // prepare to close last curve segment if needed
            if (bClosePath && (nPointIdx == nPointCount))
            {
                nClosedIdx = 0;
            }
            else
            {
                break;
            }
        }

        const basegfx::B2DPoint aPoint(nullptr == pViewInformation
                                           ? rPolygon.getB2DPoint(nClosedIdx)
                                           : impPixelSnap(rPolygon, *pViewInformation, nClosedIdx));

        if (!nPointIdx)
        {
            // first point => just move there
            cairo_move_to(cr, aPoint.getX(), aPoint.getY());
            aLast = aPoint;
            continue;
        }

        bool bPendingCurve(false);

        if (bHasCurves)
        {
            bPendingCurve = rPolygon.isNextControlPointUsed(nPrevIdx);
            bPendingCurve |= rPolygon.isPrevControlPointUsed(nClosedIdx);
        }

        if (!bPendingCurve) // line segment
        {
            cairo_line_to(cr, aPoint.getX(), aPoint.getY());
        }
        else // cubic bezier segment
        {
            basegfx::B2DPoint aCP1 = rPolygon.getNextControlPoint(nPrevIdx);
            basegfx::B2DPoint aCP2 = rPolygon.getPrevControlPoint(nClosedIdx);

            // tdf#99165 if the control points are 'empty', create the mathematical
            // correct replacement ones to avoid problems with the graphical sub-system
            // tdf#101026 The 1st attempt to create a mathematically correct replacement control
            // vector was wrong. Best alternative is one as close as possible which means short.
            if (aCP1.equal(aLast))
            {
                aCP1 = aLast + ((aCP2 - aLast) * 0.0005);
            }

            if (aCP2.equal(aPoint))
            {
                aCP2 = aPoint + ((aCP1 - aPoint) * 0.0005);
            }

            cairo_curve_to(cr, aCP1.getX(), aCP1.getY(), aCP2.getX(), aCP2.getY(), aPoint.getX(),
                           aPoint.getY());
        }

        aLast = aPoint;
    }

    if (bClosePath)
    {
        cairo_close_path(cr);
    }
}
}

namespace drawinglayer::processor2d
{
CairoPixelProcessor2D::CairoPixelProcessor2D(const geometry::ViewInformation2D& rViewInformation)
    : BaseProcessor2D(rViewInformation)
    , maBColorModifierStack()
    , mpRT(nullptr)
{
}

CairoPixelProcessor2D::CairoPixelProcessor2D(const geometry::ViewInformation2D& rViewInformation,
                                             cairo_surface_t* pTarget)
    : BaseProcessor2D(rViewInformation)
    , maBColorModifierStack()
    , mpRT(nullptr)
{
    if (pTarget)
    {
        cairo_t* pRT = cairo_create(pTarget);
        cairo_set_antialias(pRT, rViewInformation.getUseAntiAliasing() ? CAIRO_ANTIALIAS_DEFAULT
                                                                       : CAIRO_ANTIALIAS_NONE);
        setRenderTarget(pRT);
    }
}

CairoPixelProcessor2D::~CairoPixelProcessor2D()
{
    if (mpRT)
        cairo_destroy(mpRT);
}

void CairoPixelProcessor2D::processPolygonHairlinePrimitive2D(
    const primitive2d::PolygonHairlinePrimitive2D& rPolygonHairlinePrimitive2D)
{
    const basegfx::B2DPolygon& rPolygon(rPolygonHairlinePrimitive2D.getB2DPolygon());

    if (!rPolygon.count())
        return;

    cairo_save(mpRT);

    cairo_matrix_t aMatrix;
    const double fAAOffset(getViewInformation2D().getUseAntiAliasing() ? 0.5 : 0.0);
    const basegfx::B2DHomMatrix& rObjectToView(
        getViewInformation2D().getObjectToViewTransformation());
    cairo_matrix_init(&aMatrix, rObjectToView.a(), rObjectToView.b(), rObjectToView.c(),
                      rObjectToView.d(), rObjectToView.e() + fAAOffset,
                      rObjectToView.f() + fAAOffset);

    // set linear transformation
    cairo_set_matrix(mpRT, &aMatrix);

    const basegfx::BColor aHairlineColor(
        maBColorModifierStack.getModifiedColor(rPolygonHairlinePrimitive2D.getBColor()));
    cairo_set_source_rgb(mpRT, aHairlineColor.getRed(), aHairlineColor.getGreen(),
                         aHairlineColor.getBlue());

    // TODO: Unfortunately Direct2D paint of one pixel wide lines does not
    // correctly and completely blend 100% over the background. Experimenting
    // shows that a value around/slightly below 2.0 is needed which hints that
    // alpha blending the half-shifted lines (see fAAOffset above) is involved.
    // To get correct blending I try to use just wider hairlines for now. This
    // may need to be improved - or balanced (trying sqrt(2) now...)
    cairo_set_line_width(mpRT, 1.44f);

    addB2DPolygonToPathGeometry(mpRT, rPolygon, &getViewInformation2D());

    cairo_stroke(mpRT);

    cairo_restore(mpRT);
}

void CairoPixelProcessor2D::processPolyPolygonColorPrimitive2D(
    const primitive2d::PolyPolygonColorPrimitive2D& rPolyPolygonColorPrimitive2D)
{
    const basegfx::B2DPolyPolygon& rPolyPolygon(rPolyPolygonColorPrimitive2D.getB2DPolyPolygon());
    const sal_uInt32 nCount(rPolyPolygon.count());

    if (!nCount)
        return;

    cairo_save(mpRT);

    cairo_matrix_t aMatrix;
    const double fAAOffset(getViewInformation2D().getUseAntiAliasing() ? 0.5 : 0.0);
    const basegfx::B2DHomMatrix& rObjectToView(
        getViewInformation2D().getObjectToViewTransformation());
    cairo_matrix_init(&aMatrix, rObjectToView.a(), rObjectToView.b(), rObjectToView.c(),
                      rObjectToView.d(), rObjectToView.e() + fAAOffset,
                      rObjectToView.f() + fAAOffset);

    // set linear transformation
    cairo_set_matrix(mpRT, &aMatrix);

    const basegfx::BColor aFillColor(
        maBColorModifierStack.getModifiedColor(rPolyPolygonColorPrimitive2D.getBColor()));
    cairo_set_source_rgb(mpRT, aFillColor.getRed(), aFillColor.getGreen(), aFillColor.getBlue());

    for (const auto& rPolygon : rPolyPolygon)
        addB2DPolygonToPathGeometry(mpRT, rPolygon, &getViewInformation2D());

    cairo_fill(mpRT);

    cairo_restore(mpRT);
}

#if 0

void CairoPixelProcessor2D::processBitmapPrimitive2D(
    const primitive2d::BitmapPrimitive2D& rBitmapCandidate)
{
    // TODO: All the smarts to get/make a cairo_surface_t from a BitmapEx is internal to vcl at the moment
}

#endif

namespace
{
// This bit-tweaking looping is unpleasant and unfortunate
void LuminanceToAlpha(cairo_surface_t* pMask)
{
    cairo_surface_flush(pMask);

    int nWidth = cairo_image_surface_get_width(pMask);
    int nHeight = cairo_image_surface_get_height(pMask);
    int nStride = cairo_image_surface_get_stride(pMask);
    unsigned char* mask_surface_data = cairo_image_surface_get_data(pMask);

    // include/basegfx/color/bcolormodifier.hxx
    const double nRedMul = 0.2125 / 255.0;
    const double nGreenMul = 0.7154 / 255.0;
    const double nBlueMul = 0.0721 / 255.0;
    for (int y = 0; y < nHeight; ++y)
    {
        unsigned char* pMaskPixelData = mask_surface_data + (nStride * y);
        for (int x = 0; x < nWidth; ++x)
        {
            double fLuminance = pMaskPixelData[SVP_CAIRO_RED] * nRedMul
                                + pMaskPixelData[SVP_CAIRO_GREEN] * nGreenMul
                                + pMaskPixelData[SVP_CAIRO_BLUE] * nBlueMul;
            // Only this alpha channel is taken into account by cairo_mask_surface
            // so reuse this surface for the alpha result
            pMaskPixelData[SVP_CAIRO_ALPHA] = 255.0 * fLuminance;
            pMaskPixelData += 4;
        }
    }

    cairo_surface_mark_dirty(pMask);
}
}

void CairoPixelProcessor2D::processTransparencePrimitive2D(
    const primitive2d::TransparencePrimitive2D& rTransCandidate)
{
    if (rTransCandidate.getChildren().empty())
        return;

    if (rTransCandidate.getTransparence().empty())
        return;

    cairo_surface_t* pTarget = cairo_get_target(mpRT);

    double clip_x1, clip_x2, clip_y1, clip_y2;
    cairo_clip_extents(mpRT, &clip_x1, &clip_y1, &clip_x2, &clip_y2);

    // calculate visible range, create only for that range
    basegfx::B2DRange aDiscreteRange(
        rTransCandidate.getChildren().getB2DRange(getViewInformation2D()));
    aDiscreteRange.transform(getViewInformation2D().getObjectToViewTransformation());
    const basegfx::B2DRange aViewRange(basegfx::B2DPoint(clip_x1, clip_y1),
                                       basegfx::B2DPoint(clip_x2, clip_y2));
    basegfx::B2DRange aVisibleRange(aDiscreteRange);
    aVisibleRange.intersect(aViewRange);

    if (aVisibleRange.isEmpty())
    {
        // not visible, done
        return;
    }

    const basegfx::B2DHomMatrix aEmbedTransform(basegfx::utils::createTranslateB2DHomMatrix(
        -aVisibleRange.getMinX(), -aVisibleRange.getMinY()));
    geometry::ViewInformation2D aViewInformation2D(getViewInformation2D());
    aViewInformation2D.setViewTransformation(aEmbedTransform
                                             * getViewInformation2D().getViewTransformation());
    // draw mask to temporary surface
    cairo_surface_t* pMask = cairo_surface_create_similar_image(pTarget, CAIRO_FORMAT_ARGB32,
                                                                ceil(aVisibleRange.getWidth()),
                                                                ceil(aVisibleRange.getHeight()));
    CairoPixelProcessor2D aMaskRenderer(aViewInformation2D, pMask);
    aMaskRenderer.process(rTransCandidate.getTransparence());

    // convert mask to something cairo can use
    LuminanceToAlpha(pMask);

    // draw content to temporary surface
    cairo_surface_t* pContent = cairo_surface_create_similar(
        pTarget, cairo_surface_get_content(pTarget), ceil(aVisibleRange.getWidth()),
        ceil(aVisibleRange.getHeight()));
    CairoPixelProcessor2D aContent(aViewInformation2D, pContent);
    aContent.process(rTransCandidate.getChildren());

    // munge the temporary surfaces to our target surface
    cairo_set_source_surface(mpRT, pContent, aVisibleRange.getMinX(), aVisibleRange.getMinY());
    cairo_mask_surface(mpRT, pMask, aVisibleRange.getMinX(), aVisibleRange.getMinY());

    cairo_surface_destroy(pContent);
    cairo_surface_destroy(pMask);
}

void CairoPixelProcessor2D::processMaskPrimitive2DPixel(
    const primitive2d::MaskPrimitive2D& rMaskCandidate)
{
    if (rMaskCandidate.getChildren().empty())
        return;

    basegfx::B2DPolyPolygon aMask(rMaskCandidate.getMask());

    if (!aMask.count())
        return;

    double clip_x1, clip_x2, clip_y1, clip_y2;
    cairo_clip_extents(mpRT, &clip_x1, &clip_y1, &clip_x2, &clip_y2);

    basegfx::B2DRange aMaskRange(aMask.getB2DRange());
    aMaskRange.transform(getViewInformation2D().getObjectToViewTransformation());
    const basegfx::B2DRange aViewRange(basegfx::B2DPoint(clip_x1, clip_y1),
                                       basegfx::B2DPoint(clip_x2, clip_y2));

    if (!aViewRange.overlaps(aMaskRange))
        return;

    cairo_save(mpRT);

    cairo_matrix_t aMatrix;
    const basegfx::B2DHomMatrix& rObjectToView(
        getViewInformation2D().getObjectToViewTransformation());
    cairo_matrix_init(&aMatrix, rObjectToView.a(), rObjectToView.b(), rObjectToView.c(),
                      rObjectToView.d(), rObjectToView.e(), rObjectToView.f());

    // set linear transformation
    cairo_set_matrix(mpRT, &aMatrix);

    // put mask as path
    for (const auto& rPolygon : aMask)
        addB2DPolygonToPathGeometry(mpRT, rPolygon, &getViewInformation2D());

    // clip to this mask
    cairo_clip(mpRT);

    process(rMaskCandidate.getChildren());

    cairo_restore(mpRT);
}

void CairoPixelProcessor2D::processPointArrayPrimitive2D(
    const primitive2d::PointArrayPrimitive2D& rPointArrayCandidate)
{
    const std::vector<basegfx::B2DPoint>& rPositions(rPointArrayCandidate.getPositions());
    if (rPositions.empty())
        return;

    const basegfx::BColor aPointColor(
        maBColorModifierStack.getModifiedColor(rPointArrayCandidate.getRGBColor()));
    cairo_set_source_rgb(mpRT, aPointColor.getRed(), aPointColor.getGreen(), aPointColor.getBlue());

    // To really paint a single pixel I found nothing better than
    // switch off AA and draw a pixel-aligned rectangle
    const cairo_antialias_t eOldAAMode(cairo_get_antialias(mpRT));
    cairo_set_antialias(mpRT, CAIRO_ANTIALIAS_NONE);

    for (auto const& pos : rPositions)
    {
        const basegfx::B2DPoint aDiscretePos(getViewInformation2D().getObjectToViewTransformation()
                                             * pos);
        const double fX(ceil(aDiscretePos.getX()));
        const double fY(ceil(aDiscretePos.getY()));

        cairo_rectangle(mpRT, fX, fY, 1, 1);
        cairo_fill(mpRT);
    }

    cairo_set_antialias(mpRT, eOldAAMode);
}

void CairoPixelProcessor2D::processModifiedColorPrimitive2D(
    const primitive2d::ModifiedColorPrimitive2D& rModifiedCandidate)
{
    if (!rModifiedCandidate.getChildren().empty())
    {
        maBColorModifierStack.push(rModifiedCandidate.getColorModifier());
        process(rModifiedCandidate.getChildren());
        maBColorModifierStack.pop();
    }
}

void CairoPixelProcessor2D::processTransformPrimitive2D(
    const primitive2d::TransformPrimitive2D& rTransformCandidate)
{
    // remember current transformation and ViewInformation
    const geometry::ViewInformation2D aLastViewInformation2D(getViewInformation2D());

    // create new transformations for local ViewInformation2D
    geometry::ViewInformation2D aViewInformation2D(getViewInformation2D());
    aViewInformation2D.setObjectTransformation(getViewInformation2D().getObjectTransformation()
                                               * rTransformCandidate.getTransformation());
    updateViewInformation(aViewInformation2D);

    // process content
    process(rTransformCandidate.getChildren());

    // restore transformations
    updateViewInformation(aLastViewInformation2D);
}

void CairoPixelProcessor2D::processPolygonStrokePrimitive2D(
    const primitive2d::PolygonStrokePrimitive2D& rPolygonStrokeCandidate)
{
    const basegfx::B2DPolygon& rPolygon(rPolygonStrokeCandidate.getB2DPolygon());
    const attribute::LineAttribute& rLineAttribute(rPolygonStrokeCandidate.getLineAttribute());

    if (!rPolygon.count() || rLineAttribute.getWidth() < 0.0)
    {
        // no geometry, done
        return;
    }

    // get some values early that might be used for decisions
    const bool bHairline(0.0 == rLineAttribute.getWidth());
    const basegfx::B2DHomMatrix& rObjectToView(
        getViewInformation2D().getObjectToViewTransformation());
    const double fDiscreteLineWidth(
        bHairline
            ? 1.0
            : (rObjectToView * basegfx::B2DVector(rLineAttribute.getWidth(), 0.0)).getLength());

    // Here for every combination which the system-specific implementation is not
    // capable of visualizing, use the (for decomposable Primitives always possible)
    // fallback to the decomposition.
    if (basegfx::B2DLineJoin::NONE == rLineAttribute.getLineJoin() && fDiscreteLineWidth > 1.5)
    {
        // basegfx::B2DLineJoin::NONE is special for our office, no other GraphicSystem
        // knows that (so far), so fallback to decomposition. This is only needed if
        // LineJoin will be used, so also check for discrete LineWidth before falling back
        process(rPolygonStrokeCandidate);
        return;
    }

    // This is a method every system-specific implementation of a decomposable Primitive
    // can use to allow simple optical control of paint implementation:
    // Create a copy, e.g. change color to 'red' as here and paint before the system
    // paints it using the decomposition. That way you can - if active - directly
    // optically compare if the system-specific solution is geometrically identical to
    // the decomposition (which defines our interpretation that we need to visualize).
    // Look below in the impl for bRenderDecomposeForCompareInRed to see that in that case
    // we create a half-transparent paint to better support visual control
    static bool bRenderDecomposeForCompareInRed(false);

    if (bRenderDecomposeForCompareInRed)
    {
        const attribute::LineAttribute aRed(
            basegfx::BColor(1.0, 0.0, 0.0), rLineAttribute.getWidth(), rLineAttribute.getLineJoin(),
            rLineAttribute.getLineCap(), rLineAttribute.getMiterMinimumAngle());
        rtl::Reference<primitive2d::PolygonStrokePrimitive2D> xCopy(
            new primitive2d::PolygonStrokePrimitive2D(
                rPolygonStrokeCandidate.getB2DPolygon(), aRed,
                rPolygonStrokeCandidate.getStrokeAttribute()));
        process(*xCopy);
    }

    cairo_save(mpRT);

    cairo_matrix_t aMatrix;
    const double fAAOffset(getViewInformation2D().getUseAntiAliasing() ? 0.5 : 0.0);
    cairo_matrix_init(&aMatrix, rObjectToView.a(), rObjectToView.b(), rObjectToView.c(),
                      rObjectToView.d(), rObjectToView.e() + fAAOffset,
                      rObjectToView.f() + fAAOffset);

    // set linear transformation
    cairo_set_matrix(mpRT, &aMatrix);

    // setup line attributes
    cairo_line_join_t eCairoLineJoin = CAIRO_LINE_JOIN_MITER;
    switch (rLineAttribute.getLineJoin())
    {
        case basegfx::B2DLineJoin::Bevel:
            eCairoLineJoin = CAIRO_LINE_JOIN_BEVEL;
            break;
        case basegfx::B2DLineJoin::Round:
            eCairoLineJoin = CAIRO_LINE_JOIN_ROUND;
            break;
        case basegfx::B2DLineJoin::NONE:
        case basegfx::B2DLineJoin::Miter:
            eCairoLineJoin = CAIRO_LINE_JOIN_MITER;
            break;
    }

    // convert miter minimum angle to miter limit
    double fMiterLimit
        = 1.0 / sin(std::max(rLineAttribute.getMiterMinimumAngle(), 0.01 * M_PI) / 2.0);

    // setup cap attribute
    cairo_line_cap_t eCairoLineCap(CAIRO_LINE_CAP_BUTT);

    switch (rLineAttribute.getLineCap())
    {
        default: // css::drawing::LineCap_BUTT:
        {
            eCairoLineCap = CAIRO_LINE_CAP_BUTT;
            break;
        }
        case css::drawing::LineCap_ROUND:
        {
            eCairoLineCap = CAIRO_LINE_CAP_ROUND;
            break;
        }
        case css::drawing::LineCap_SQUARE:
        {
            eCairoLineCap = CAIRO_LINE_CAP_SQUARE;
            break;
        }
    }

    basegfx::BColor aLineColor(maBColorModifierStack.getModifiedColor(rLineAttribute.getColor()));
    if (bRenderDecomposeForCompareInRed)
        aLineColor.setRed(0.5);

    cairo_set_source_rgb(mpRT, aLineColor.getRed(), aLineColor.getGreen(), aLineColor.getBlue());

    cairo_set_line_join(mpRT, eCairoLineJoin);
    cairo_set_line_cap(mpRT, eCairoLineCap);

    // TODO: Hairline LineWidth, see comment at processPolygonHairlinePrimitive2D
    cairo_set_line_width(mpRT, bHairline ? 1.44 : fDiscreteLineWidth);
    cairo_set_miter_limit(mpRT, fMiterLimit);

    const attribute::StrokeAttribute& rStrokeAttribute(
        rPolygonStrokeCandidate.getStrokeAttribute());
    const bool bDashUsed(!rStrokeAttribute.isDefault()
                         && !rStrokeAttribute.getDotDashArray().empty()
                         && 0.0 < rStrokeAttribute.getFullDotDashLen());
    if (bDashUsed)
    {
        const std::vector<double>& rStroke = rStrokeAttribute.getDotDashArray();
        cairo_set_dash(mpRT, rStroke.data(), rStroke.size(), 0.0);
    }

    addB2DPolygonToPathGeometry(mpRT, rPolygon, &getViewInformation2D());

    cairo_stroke(mpRT);

    cairo_restore(mpRT);
}

void CairoPixelProcessor2D::processLineRectanglePrimitive2D(
    const primitive2d::LineRectanglePrimitive2D& rLineRectanglePrimitive2D)
{
    if (rLineRectanglePrimitive2D.getB2DRange().isEmpty())
    {
        // no geometry, done
        return;
    }

    cairo_save(mpRT);

    cairo_matrix_t aMatrix;
    const double fAAOffset(getViewInformation2D().getUseAntiAliasing() ? 0.5 : 0.0);
    const basegfx::B2DHomMatrix& rObjectToView(
        getViewInformation2D().getObjectToViewTransformation());
    cairo_matrix_init(&aMatrix, rObjectToView.a(), rObjectToView.b(), rObjectToView.c(),
                      rObjectToView.d(), rObjectToView.e() + fAAOffset,
                      rObjectToView.f() + fAAOffset);

    // set linear transformation
    cairo_set_matrix(mpRT, &aMatrix);

    const basegfx::BColor aHairlineColor(
        maBColorModifierStack.getModifiedColor(rLineRectanglePrimitive2D.getBColor()));
    cairo_set_source_rgb(mpRT, aHairlineColor.getRed(), aHairlineColor.getGreen(),
                         aHairlineColor.getBlue());

    const double fDiscreteLineWidth((getViewInformation2D().getInverseObjectToViewTransformation()
                                     * basegfx::B2DVector(1.44, 0.0))
                                        .getLength());
    cairo_set_line_width(mpRT, fDiscreteLineWidth);

    const basegfx::B2DRange& rRange(rLineRectanglePrimitive2D.getB2DRange());
    cairo_rectangle(mpRT, rRange.getMinX(), rRange.getMinY(), rRange.getWidth(),
                    rRange.getHeight());
    cairo_stroke(mpRT);

    cairo_restore(mpRT);
}

void CairoPixelProcessor2D::processFilledRectanglePrimitive2D(
    const primitive2d::FilledRectanglePrimitive2D& rFilledRectanglePrimitive2D)
{
    if (rFilledRectanglePrimitive2D.getB2DRange().isEmpty())
    {
        // no geometry, done
        return;
    }

    cairo_save(mpRT);

    cairo_matrix_t aMatrix;
    const double fAAOffset(getViewInformation2D().getUseAntiAliasing() ? 0.5 : 0.0);
    const basegfx::B2DHomMatrix& rObjectToView(
        getViewInformation2D().getObjectToViewTransformation());
    cairo_matrix_init(&aMatrix, rObjectToView.a(), rObjectToView.b(), rObjectToView.c(),
                      rObjectToView.d(), rObjectToView.e() + fAAOffset,
                      rObjectToView.f() + fAAOffset);

    // set linear transformation
    cairo_set_matrix(mpRT, &aMatrix);

    const basegfx::BColor aFillColor(
        maBColorModifierStack.getModifiedColor(rFilledRectanglePrimitive2D.getBColor()));
    cairo_set_source_rgb(mpRT, aFillColor.getRed(), aFillColor.getGreen(), aFillColor.getBlue());

    const basegfx::B2DRange& rRange(rFilledRectanglePrimitive2D.getB2DRange());
    cairo_rectangle(mpRT, rRange.getMinX(), rRange.getMinY(), rRange.getWidth(),
                    rRange.getHeight());
    cairo_fill(mpRT);

    cairo_restore(mpRT);
}

void CairoPixelProcessor2D::processSingleLinePrimitive2D(
    const primitive2d::SingleLinePrimitive2D& rSingleLinePrimitive2D)
{
    cairo_save(mpRT);

    const basegfx::BColor aLineColor(
        maBColorModifierStack.getModifiedColor(rSingleLinePrimitive2D.getBColor()));
    cairo_set_source_rgb(mpRT, aLineColor.getRed(), aLineColor.getGreen(), aLineColor.getBlue());

    const double fAAOffset(getViewInformation2D().getUseAntiAliasing() ? 0.5 : 0.0);
    const basegfx::B2DHomMatrix& rObjectToView(
        getViewInformation2D().getObjectToViewTransformation());
    const basegfx::B2DPoint aStart(rObjectToView * rSingleLinePrimitive2D.getStart());
    const basegfx::B2DPoint aEnd(rObjectToView * rSingleLinePrimitive2D.getEnd());

    cairo_set_line_width(mpRT, 1.44f);

    cairo_move_to(mpRT, aStart.getX() + fAAOffset, aStart.getY() + fAAOffset);
    cairo_line_to(mpRT, aEnd.getX() + fAAOffset, aEnd.getY() + fAAOffset);
    cairo_stroke(mpRT);

    cairo_restore(mpRT);
}

void CairoPixelProcessor2D::processBasePrimitive2D(const primitive2d::BasePrimitive2D& rCandidate)
{
    switch (rCandidate.getPrimitive2DID())
    {
#if 0
        // geometry that *has* to be processed
        case PRIMITIVE2D_ID_BITMAPPRIMITIVE2D:
        {
            processBitmapPrimitive2D(
                static_cast<const primitive2d::BitmapPrimitive2D&>(rCandidate));
            break;
        }
#endif
        case PRIMITIVE2D_ID_POINTARRAYPRIMITIVE2D:
        {
            processPointArrayPrimitive2D(
                static_cast<const primitive2d::PointArrayPrimitive2D&>(rCandidate));
            break;
        }
        case PRIMITIVE2D_ID_POLYGONHAIRLINEPRIMITIVE2D:
        {
            processPolygonHairlinePrimitive2D(
                static_cast<const primitive2d::PolygonHairlinePrimitive2D&>(rCandidate));
            break;
        }
        case PRIMITIVE2D_ID_POLYPOLYGONCOLORPRIMITIVE2D:
        {
            processPolyPolygonColorPrimitive2D(
                static_cast<const primitive2d::PolyPolygonColorPrimitive2D&>(rCandidate));
            break;
        }
        // embedding/groups that *have* to be processed
        case PRIMITIVE2D_ID_TRANSPARENCEPRIMITIVE2D:
        {
            processTransparencePrimitive2D(
                static_cast<const primitive2d::TransparencePrimitive2D&>(rCandidate));
            break;
        }
        case PRIMITIVE2D_ID_INVERTPRIMITIVE2D:
        {
            // TODO: fallback is at VclPixelProcessor2D::processInvertPrimitive2D, so
            // not in reach. Ignore for now.
            // processInvertPrimitive2D(rCandidate);
            break;
        }
        case PRIMITIVE2D_ID_MASKPRIMITIVE2D:
        {
            processMaskPrimitive2DPixel(
                static_cast<const primitive2d::MaskPrimitive2D&>(rCandidate));
            break;
        }
        case PRIMITIVE2D_ID_MODIFIEDCOLORPRIMITIVE2D:
        {
            processModifiedColorPrimitive2D(
                static_cast<const primitive2d::ModifiedColorPrimitive2D&>(rCandidate));
            break;
        }
        case PRIMITIVE2D_ID_TRANSFORMPRIMITIVE2D:
        {
            processTransformPrimitive2D(
                static_cast<const primitive2d::TransformPrimitive2D&>(rCandidate));
            break;
        }
#if 0
        // geometry that *may* be processed due to being able to do it better
        // then using the decomposition
        case PRIMITIVE2D_ID_UNIFIEDTRANSPARENCEPRIMITIVE2D:
        {
            processUnifiedTransparencePrimitive2D(
                static_cast<const primitive2d::UnifiedTransparencePrimitive2D&>(rCandidate));
            break;
        }
        case PRIMITIVE2D_ID_MARKERARRAYPRIMITIVE2D:
        {
            processMarkerArrayPrimitive2D(
                static_cast<const primitive2d::MarkerArrayPrimitive2D&>(rCandidate));
            break;
        }
        case PRIMITIVE2D_ID_BACKGROUNDCOLORPRIMITIVE2D:
        {
            processBackgroundColorPrimitive2D(
                static_cast<const primitive2d::BackgroundColorPrimitive2D&>(rCandidate));
            break;
        }
#endif
        case PRIMITIVE2D_ID_POLYGONSTROKEPRIMITIVE2D:
        {
            processPolygonStrokePrimitive2D(
                static_cast<const primitive2d::PolygonStrokePrimitive2D&>(rCandidate));
            break;
        }
        case PRIMITIVE2D_ID_LINERECTANGLEPRIMITIVE2D:
        {
            processLineRectanglePrimitive2D(
                static_cast<const primitive2d::LineRectanglePrimitive2D&>(rCandidate));
            break;
        }
        case PRIMITIVE2D_ID_FILLEDRECTANGLEPRIMITIVE2D:
        {
            processFilledRectanglePrimitive2D(
                static_cast<const primitive2d::FilledRectanglePrimitive2D&>(rCandidate));
            break;
        }
        case PRIMITIVE2D_ID_SINGLELINEPRIMITIVE2D:
        {
            processSingleLinePrimitive2D(
                static_cast<const primitive2d::SingleLinePrimitive2D&>(rCandidate));
            break;
        }

        // continue with decompose
        default:
        {
            SAL_INFO("drawinglayer", "default case for " << drawinglayer::primitive2d::idToString(
                                         rCandidate.getPrimitive2DID()));
            // process recursively
            process(rCandidate);
            break;
        }
    }
}

} // end of namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
