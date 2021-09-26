/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <sal/types.h>
#include <tools/solar.h>
#include <tools/color.hxx>
#include <tools/fontenum.hxx>
#include <i18nlangtag/lang.h>

#include <vcl/ClippingInterface.hxx>
#include <vcl/mapmod.hxx>
#include <vcl/metric.hxx>
#include <vcl/mapmod.hxx>
#include <vcl/metric.hxx>
#include <vcl/outdevstate.hxx>
#include <vcl/rendercontext/RasterOp.hxx>
#include <vcl/rendercontext/DrawGridFlags.hxx>
#include <vcl/rendercontext/DrawImageFlags.hxx>
#include <vcl/rendercontext/DrawTextFlags.hxx>
#include <vcl/rendercontext/InvertFlags.hxx>
#include <vcl/rendercontext/SalLayoutFlags.hxx>

#include <com/sun/star/drawing/LineCap.hpp>

enum class B2DLineJoin;

class Animation;
class Bitmap;
class BitmapEx;
class Color;
class GDIMetaFile;
class GfxLink;
class Gradient;
class Hatch;
class Image;
class LineInfo;
class SalLayoutGlyphs;
class Region;
class Wallpaper;

namespace vcl
{
class Font;
class ITextLayout;
class Region;
}

namespace basegfx
{
class B2DHomMatrix;
class B2DPolygon;
}

namespace tools
{
class Polygon;
class PolyPolygon;
class Rectangle;
}

class DrawingInterface : public ClippingInterface
{
public:
    virtual ~DrawingInterface() {}

    virtual FontMetric GetFontMetric() const = 0;

    virtual void SetFont(vcl::Font const& rNewFont) = 0;
    virtual void SetTextColor(Color const& rColor) = 0;
    virtual void SetTextFillColor(Color const& rColor) = 0;
    virtual void SetLineColor(Color const& rColor) = 0;
    virtual void SetFillColor(Color const& rColor) = 0;
    virtual void SetRasterOp(RasterOp eRasterOp) = 0;
    virtual void SetMapMode(MapMode const& rNewMapMode) = 0;
    virtual void SetTextAlign(TextAlign eAlign) = 0;
    virtual void SetLayoutMode(ComplexTextLayoutFlags nTextLayoutMode) = 0;
    virtual void SetDigitLanguage(LanguageType) = 0;
    virtual void Push(PushFlags nFlags = PushFlags::ALL) = 0;
    virtual void Pop() = 0;

    /** @name Pixel functions
     */
    ///@{

    virtual void DrawPixel(Point const& rPt) = 0;
    virtual void DrawPixel(Point const& rPt, Color const& rColor) = 0;
    ///@}

    /** @name Rectangle functions
     */
    ///@{

    virtual void DrawRect(tools::Rectangle const& rRect) = 0;
    virtual void DrawRect(tools::Rectangle const& rRect, sal_uLong nHorzRount, sal_uLong nVertRound)
        = 0;

    virtual void DrawGrid(tools::Rectangle const& rRect, Size const& rDist, DrawGridFlags nFlags)
        = 0;
    virtual void DrawBorder(tools::Rectangle aBorderRect) = 0;

    /// Fill the given rectangle with checkered rectangles of size nLen x nLen using the colors aStart and aEnd
    virtual void DrawCheckered(const Point& rPos, const Size& rSize, sal_uInt32 nLen = 8,
                               Color aStart = COL_WHITE, Color aEnd = COL_BLACK)
        = 0;

    ///@}

    /** @name Line functions
     */
    ///@{

    virtual void DrawLine(Point const& rStartPt, Point const& rEndPt) = 0;
    virtual void DrawLine(Point const& rStartPt, Point const& rEndPt, LineInfo const& rLineInfo)
        = 0;
    virtual void DrawWaveLine(Point const& rStartPos, Point const& rEndPos,
                              tools::Long nLineWidth = 1)
        = 0;

    ///@}

    /** @name Polyline functions
     */
    ///@{

    /** Render the given polygon as a line stroke
        The given polygon is stroked with the current LineColor, start
        and end point are not automatically connected
        @see DrawPolygon
        @see DrawPolyPolygon
     */
    virtual void DrawPolyLine(const tools::Polygon& rPoly) = 0;

    virtual void DrawPolyLine(const basegfx::B2DPolygon&, double fLineWidth = 0.0,
                              basegfx::B2DLineJoin eLineJoin = basegfx::B2DLineJoin::Round,
                              css::drawing::LineCap eLineCap = css::drawing::LineCap_BUTT,
                              double fMiterMinimumAngle = basegfx::deg2rad(15.0))
        = 0;

    /** Render the given polygon as a line stroke
        The given polygon is stroked with the current LineColor, start
        and end point are not automatically connected. The line is
        rendered according to the specified LineInfo, e.g. supplying a
        dash pattern, or a line thickness.
        @see DrawPolygon
        @see DrawPolyPolygon
     */
    virtual void DrawPolyLine(tools::Polygon const& rPoly, LineInfo const& rLineInfo) = 0;

    ///@}

    /** @name Polygon functions
     */
    ///@{

    virtual void DrawPolygon(tools::Polygon const& rPoly) = 0;
    virtual void DrawPolygon(basegfx::B2DPolygon const& rPoly) = 0;

    /** Render the given poly-polygon
        The given poly-polygon is stroked with the current LineColor,
        and filled with the current FillColor. If one of these colors
        are transparent, the corresponding stroke or fill stays
        invisible. Start and end points of the contained polygons are
        automatically connected.
        @see DrawPolyLine
     */
    virtual void DrawPolyPolygon(tools::PolyPolygon const& rPolyPoly) = 0;
    virtual void DrawPolyPolygon(basegfx::B2DPolyPolygon const& rPolyPoly) = 0;

    ///@}

    /** @name Curved shape functions
     */
    ///@{

    virtual void DrawEllipse(tools::Rectangle const& rRect) = 0;
    virtual void DrawArc(tools::Rectangle const& rRect, Point const& rStartPt, Point const& rEndPt)
        = 0;
    virtual void DrawPie(tools::Rectangle const& rRect, Point const& rStartPt, Point const& rEndPt)
        = 0;
    virtual void DrawChord(tools::Rectangle const& rRect, Point const& rStartPt,
                           Point const& rEndPt)
        = 0;

    ///@}

    /** @name Gradient functions
     */
    ///@{

    virtual void DrawGradient(tools::Rectangle const& rRect, Gradient const& rGradient) = 0;
    virtual void DrawGradient(tools::PolyPolygon const& rPolyPoly, Gradient const& rGradient) = 0;

    ///@}

    /** @name Hatch functions
     */
    ///@{

#ifdef _MSC_VER
    virtual void DrawHatch(tools::PolyPolygon const& rPolyPoly, ::Hatch const& rHatch) = 0;
#else
    virtual void DrawHatch(tools::PolyPolygon const& rPolyPoly, Hatch const& rHatch) = 0;
#endif

    ///@}

    /** @name Wallpaper functions
     */
    ///@{

    virtual void DrawWallpaper(const tools::Rectangle& rRect, const Wallpaper& rWallpaper) = 0;

    virtual void Erase() = 0;
    virtual void Erase(tools::Rectangle const& rRect) = 0;

    virtual void Invert(tools::Rectangle const& rRect, InvertFlags nFlags = InvertFlags::NONE) = 0;
    virtual void Invert(tools::Polygon const& rPoly, InvertFlags nFlags = InvertFlags::NONE) = 0;

    ///@}

    /** @name Text functions
     */
    ///@{

    virtual void DrawText(Point const& rStartPt, OUString const& rStr, sal_Int32 nIndex = 0,
                          sal_Int32 nLen = -1, std::vector<tools::Rectangle>* pVector = nullptr,
                          OUString* pDisplayText = nullptr,
                          SalLayoutGlyphs const* pLayoutCache = nullptr)
        = 0;

    virtual void DrawText(tools::Rectangle const& rRect, OUString const& rStr,
                          DrawTextFlags nStyle = DrawTextFlags::NONE,
                          std::vector<tools::Rectangle>* pVector = nullptr,
                          OUString* pDisplayText = nullptr,
                          vcl::ITextLayout* _pTextLayout = nullptr)
        = 0;

    virtual void DrawCtrlText(Point const& rPos, OUString const& rStr, sal_Int32 nIndex = 0,
                              sal_Int32 nLen = -1, DrawTextFlags nStyle = DrawTextFlags::Mnemonic,
                              std::vector<tools::Rectangle>* pVector = nullptr,
                              OUString* pDisplayText = nullptr,
                              SalLayoutGlyphs const* pGlyphs = nullptr)
        = 0;

    virtual void DrawTextLine(const Point& rPos, tools::Long nWidth, FontStrikeout eStrikeout,
                              FontLineStyle eUnderline, FontLineStyle eOverline,
                              bool bUnderlineAbove = false)
        = 0;

    virtual void DrawTextArray(Point const& rStartPt, OUString const& rStr,
                               tools::Long const* pDXAry, sal_Int32 nIndex = 0, sal_Int32 nLen = -1,
                               SalLayoutFlags flags = SalLayoutFlags::NONE,
                               SalLayoutGlyphs const* pLayoutCache = nullptr)
        = 0;

    virtual void DrawStretchText(Point const& rStartPt, sal_uLong nWidth, OUString const& rStr,
                                 sal_Int32 nIndex = 0, sal_Int32 nLen = -1)
        = 0;
    ///@}

    /** @name Bitmap functions
     */
    ///@{

    virtual void DrawBitmap(Point const& rDestPt, Bitmap const& rBitmap) = 0;
    virtual void DrawBitmap(Point const& rDestPt, Size const& rDestSize, Bitmap const& rBitmap) = 0;
    virtual void DrawBitmap(Point const& rDestPt, Size const& rDestSize, Point const& rSrcPtPixel,
                            Size const& rSrcSizePixel, Bitmap const& rBitmap)
        = 0;
    ///@}

    /** @name BitmapEx functions
     */
    ///@{

    virtual void DrawBitmapEx(const Point& rDestPt, const BitmapEx& rBitmapEx) = 0;
    virtual void DrawBitmapEx(const Point& rDestPt, const Size& rDestSize,
                              const BitmapEx& rBitmapEx)
        = 0;
    virtual void DrawBitmapEx(const Point& rDestPt, const Size& rDestSize, const Point& rSrcPtPixel,
                              const Size& rSrcSizePixel, const BitmapEx& rBitmapEx)
        = 0;

    virtual void DrawTransformedBitmapEx(const basegfx::B2DHomMatrix& rTransformation,
                                         const BitmapEx& rBitmapEx, double fAlpha = 1.0)
        = 0;

    ///@}

    /** @name Image functions
     */
    ///@{

    virtual void DrawImage(Point const& rPos, Image const& rImage,
                           DrawImageFlags nStyle = DrawImageFlags::NONE)
        = 0;
    virtual void DrawImage(Point const& rPos, Size const& rSize, Image const& rImage,
                           DrawImageFlags nStyle = DrawImageFlags::NONE)
        = 0;

    ///@}

    /** @name Transparency functions
     */
    ///@{
    virtual void DrawTransparent(tools::PolyPolygon const& rPolyPoly,
                                 sal_uInt16 nTransparencePercent)
        = 0;

    virtual void DrawTransparent(basegfx::B2DHomMatrix const& rObjectTransform,
                                 basegfx::B2DPolyPolygon const& rB2DPolyPoly, double fTransparency)
        = 0;
    ///@}

    /** @name Mask functions
     */
    ///@{

    virtual void DrawMask(Point const& rDestPt, Bitmap const& rBitmap, Color const& rMaskColor) = 0;
    virtual void DrawMask(Point const& rDestPt, Size const& rDestSize, Bitmap const& rBitmap,
                          Color const& rMaskColor)
        = 0;
    virtual void DrawMask(Point const& rDestPt, Size const& rDestSize, Point const& rSrcPtPixel,
                          Size const& rSrcSizePixel, Bitmap const& rBitmap, Color const& rMaskColor)
        = 0;
    ///@}

    /** @name EPS functions
     */
    ///@{

    /** @returns boolean value to see if EPS could be painted directly.
        Theoretically, handing over a matrix would be needed to handle
        painting rotated EPS files (e.g. contained in Metafiles). This
        would then need to be supported for Mac and PS printers, but
        that's too much for now, wrote \#i107046# for this */
    virtual bool DrawEPS(const Point& rPt, const Size& rSz, const GfxLink& rGfxLink,
                         GDIMetaFile* pSubst = nullptr)
        = 0;
    ///@}
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
