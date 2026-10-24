/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * Copyright the Collabora Office contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <CellMarkPrimitive2D.hxx>
#include <sc_primitivetypes2d.hxx>

#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <drawinglayer/primitive2d/PolyPolygonColorPrimitive2D.hxx>
#include <drawinglayer/primitive2d/PolygonHairlinePrimitive2D.hxx>
#include <drawinglayer/primitive2d/groupprimitive2d.hxx>
#include <fillinfo.hxx>
#include <o3tl/unit_conversion.hxx>
#include <svtools/colorcfg.hxx>
#include <vcl/settings.hxx>
#include <vcl/svapp.hxx>

#include <algorithm>
#include <cmath>

namespace
{
/// The size of the formula indicator in pixels.
constexpr double constFormulaMarkPixels = 10.0;
/// The size of the formula error indicator in pixels at 100% zoom.
constexpr double constFormulaErrorMarkPixels = 7.0;
/// Twips per pixel at 100% zoom.
constexpr double constPixelTwips = o3tl::convert(1.0, o3tl::Length::px, o3tl::Length::twip);
/// The largest side of a drop-down button in pixels.
constexpr double constButtonPixels = 17.0;

basegfx::B2DPolygon createTriangle(const basegfx::B2DPoint& rA, const basegfx::B2DPoint& rB,
                                   const basegfx::B2DPoint& rC)
{
    basegfx::B2DPolygon aTriangle;
    aTriangle.append(rA);
    aTriangle.append(rB);
    aTriangle.append(rC);
    aTriangle.setClosed(true);
    return aTriangle;
}

drawinglayer::primitive2d::Primitive2DReference createFill(const basegfx::B2DPolygon& rPolygon,
                                                           const Color& rColor)
{
    return new drawinglayer::primitive2d::PolyPolygonColorPrimitive2D(
        basegfx::B2DPolyPolygon(rPolygon), rColor.getBColor());
}
}

namespace sc
{
CellMarkPrimitive2D::CellMarkPrimitive2D(CellMarkKind eKind, SCCOL nColumn, SCROW nRow,
                                         const basegfx::B2DRange& rCellRange, const Color& rColor,
                                         double fPixelSize, bool bActive)
    : meKind(eKind)
    , mnColumn(nColumn)
    , mnRow(nRow)
    , maCellRange(rCellRange)
    , maColor(rColor)
    , mfPixelSize(fPixelSize)
    , mbActive(bActive)
    , maButtonFace(COL_TRANSPARENT)
    , maButtonOutline(COL_TRANSPARENT)
    , maButtonArrow(COL_TRANSPARENT)
{
    if (meKind != CellMarkKind::AutoFilter && meKind != CellMarkKind::PivotButton)
        return;

    // A button takes the colors of the style settings like a field button in the window, and the
    // highlight colors when it is active.
    const StyleSettings& rStyle = Application::GetSettings().GetStyleSettings();
    const Color aFace = rStyle.GetFaceColor();
    maButtonFace = mbActive ? rStyle.GetHighlightColor() : aFace;
    // A dark face on a light document is outlined in the shadow color.
    const Color aDocument = svtools::ColorConfig().GetColorValue(svtools::DOCCOLOR).nColor;
    maButtonOutline = aDocument.IsBright() && aFace.IsDark() ? rStyle.GetShadowColor()
                                                             : rStyle.GetLabelTextColor();
    maButtonArrow = mbActive ? rStyle.GetHighlightTextColor() : rStyle.GetButtonTextColor();
}

bool CellMarkPrimitive2D::operator==(
    const drawinglayer::primitive2d::BasePrimitive2D& rPrimitive) const
{
    if (!BufferedDecompositionPrimitive2D::operator==(rPrimitive))
        return false;

    const auto& rOther = static_cast<const CellMarkPrimitive2D&>(rPrimitive);
    return meKind == rOther.meKind && mnColumn == rOther.mnColumn && mnRow == rOther.mnRow
           && maCellRange == rOther.maCellRange && maColor == rOther.maColor
           && mfPixelSize == rOther.mfPixelSize && mbActive == rOther.mbActive
           && maButtonFace == rOther.maButtonFace && maButtonOutline == rOther.maButtonOutline
           && maButtonArrow == rOther.maButtonArrow;
}

sal_uInt32 CellMarkPrimitive2D::getPrimitive2DID() const
{
    return PRIMITIVE2D_ID_SC_CELLMARKPRIMITIVE2D;
}

drawinglayer::primitive2d::Primitive2DReference CellMarkPrimitive2D::create2DDecomposition(
    const drawinglayer::geometry::ViewInformation2D& /*rViewInformation*/) const
{
    if (maCellRange.isEmpty())
        return nullptr;

    const double fLeft = maCellRange.getMinX();
    const double fTop = maCellRange.getMinY();
    const double fRight = maCellRange.getMaxX();
    const double fBottom = maCellRange.getMaxY();

    switch (meKind)
    {
        case CellMarkKind::Formula:
        {
            // The mark sits two pixels above the bottom edge, clear of the grid line.
            const double fSize = constFormulaMarkPixels * mfPixelSize;
            const double fBase = fBottom - 2 * mfPixelSize;
            return createFill(createTriangle({ fLeft, fBase }, { fLeft + fSize, fBase },
                                             { fLeft, fBase - fSize }),
                              maColor);
        }
        case CellMarkKind::FormulaError:
        {
            // The mark is the size of a comment mark: one pixel larger for each zoom step of 1.2
            // above 100%, and one pixel smaller for each step below.
            const double fZoom = std::max(0.1, constPixelTwips / mfPixelSize);
            const double fPixels = std::max(
                0.0, std::round(constFormulaErrorMarkPixels + std::log(fZoom) / std::log(1.2)));
            const double fSize = fPixels * mfPixelSize;
            return createFill(
                createTriangle({ fLeft, fTop }, { fLeft + fSize, fTop }, { fLeft, fTop + fSize }),
                maColor);
        }
        case CellMarkKind::ClipLeft:
        case CellMarkKind::ClipRight:
        {
            // The arrow takes the width of a clip mark and at most half the height of the cell.
            const double fWidth = std::min<double>(SC_CLIPMARK_SIZE, maCellRange.getWidth());
            const double fHeight = std::min(fWidth, maCellRange.getHeight() / 2);
            const double fCenterY = maCellRange.getCenterY();
            if (meKind == CellMarkKind::ClipRight)
            {
                const double fTip = fRight - fWidth * 0.2;
                const double fBack = fRight - fWidth * 0.8;
                return createFill(createTriangle({ fBack, fCenterY - fHeight / 2 },
                                                 { fTip, fCenterY },
                                                 { fBack, fCenterY + fHeight / 2 }),
                                  maColor);
            }
            const double fTip = fLeft + fWidth * 0.2;
            const double fBack = fLeft + fWidth * 0.8;
            return createFill(createTriangle({ fBack, fCenterY - fHeight / 2 }, { fTip, fCenterY },
                                             { fBack, fCenterY + fHeight / 2 }),
                              maColor);
        }
        case CellMarkKind::AutoFilter:
        case CellMarkKind::PivotButton:
        {
            // A square button at the right edge of the cell, as high as the cell allows.
            const double fSide = std::min({ constButtonPixels * mfPixelSize,
                                            maCellRange.getHeight(), maCellRange.getWidth() });
            const basegfx::B2DRange aButton(fRight - fSide, fBottom - fSide, fRight, fBottom);
            const basegfx::B2DPolygon aOutline(basegfx::utils::createPolygonFromRect(aButton));

            const double fArrowHalf = fSide / 4;
            const basegfx::B2DPoint aCenter(aButton.getCenter());

            drawinglayer::primitive2d::Primitive2DContainer aButtonParts{
                createFill(aOutline, maButtonFace),
                new drawinglayer::primitive2d::PolygonHairlinePrimitive2D(
                    aOutline, maButtonOutline.getBColor()),
                createFill(
                    createTriangle({ aCenter.getX() - fArrowHalf, aCenter.getY() - fArrowHalf / 2 },
                                   { aCenter.getX() + fArrowHalf, aCenter.getY() - fArrowHalf / 2 },
                                   { aCenter.getX(), aCenter.getY() + fArrowHalf / 2 }),
                    maButtonArrow)
            };
            // An active button has a small square of three pixels in its bottom-right corner.
            if (mbActive)
            {
                const basegfx::B2DRange aBox(fRight - 5 * mfPixelSize, fBottom - 5 * mfPixelSize,
                                             fRight - 2 * mfPixelSize, fBottom - 2 * mfPixelSize);
                aButtonParts.push_back(
                    createFill(basegfx::utils::createPolygonFromRect(aBox), maButtonArrow));
            }
            return new drawinglayer::primitive2d::GroupPrimitive2D(std::move(aButtonParts));
        }
    }
    return nullptr;
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
