/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sdr/contact/extrafooterpainter.hxx>

#include <svx/svdocapt.hxx>
#include <svx/xfillit0.hxx>
#include <svx/xflclit.hxx>
#include <svl/itemset.hxx>

#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/vector/b2dvector.hxx>

#include <drawinglayer/attribute/fontattribute.hxx>
#include <drawinglayer/primitive2d/maskprimitive2d.hxx>
#include <drawinglayer/primitive2d/PolygonHairlinePrimitive2D.hxx>
#include <drawinglayer/primitive2d/textprimitive2d.hxx>
#include <drawinglayer/primitive2d/textlayoutdevice.hxx>

#include <i18nutil/scripttypedetector.hxx>
#include <com/sun/star/i18n/ScriptDirection.hpp>

#include <vcl/font.hxx>
#include <vcl/settings.hxx>
#include <vcl/svapp.hxx>

namespace
{
/// Horizontal padding inside the footer band, in 1/100 mm.
constexpr sal_Int32 EXTRA_FOOTER_PADDING_X = 100;
/// Vertical padding inside the footer band, in 1/100 mm.
constexpr sal_Int32 EXTRA_FOOTER_PADDING_Y = 50;
/// Footer font height in 1/100 mm. ~6 pt at 100% zoom.
constexpr double EXTRA_FOOTER_FONT_HEIGHT = 212.0;

drawinglayer::attribute::FontAttribute
lcl_GetExtraFooterFontAttribute(basegfx::B2DVector& o_rFontSize, bool bBold, bool bRTL)
{
    vcl::Font aFont = Application::GetSettings().GetStyleSettings().GetAppFont();
    aFont.SetFontHeight(EXTRA_FOOTER_FONT_HEIGHT);
    if (bBold)
        aFont.SetWeight(WEIGHT_BOLD);
    return drawinglayer::primitive2d::getFontAttributeFromVclFont(o_rFontSize, aFont, bRTL,
                                                                  /*bBiDiStrong*/ false);
}

/// Truncate with a trailing ellipsis so it fits within fAvailWidth.
OUString lcl_TruncateWithEllipsis(const OUString& rText,
                                  const drawinglayer::attribute::FontAttribute& rFontAttr,
                                  const basegfx::B2DVector& rFontSize, double fAvailWidth)
{
    drawinglayer::primitive2d::TextLayouterDevice aLayouter;
    aLayouter.setFontAttribute(rFontAttr, rFontSize.getX(), rFontSize.getY(), css::lang::Locale());
    if (aLayouter.getTextWidth(rText, 0, rText.getLength()) <= fAvailWidth)
        return rText;

    const OUString sEllipsis(u"…"_ustr);
    const double fEllipsisW = aLayouter.getTextWidth(sEllipsis, 0, sEllipsis.getLength());
    if (fEllipsisW >= fAvailWidth)
        return OUString();

    sal_Int32 nLo = 0, nHi = rText.getLength();
    while (nLo < nHi)
    {
        const sal_Int32 nMid = (nLo + nHi + 1) / 2;
        if (aLayouter.getTextWidth(rText, 0, nMid) + fEllipsisW <= fAvailWidth)
            nLo = nMid;
        else
            nHi = nMid - 1;
    }
    return OUString::Concat(rText.subView(0, nLo)) + sEllipsis;
}

/// True if any character has RTL script direction.
bool lcl_HasRTLChars(std::u16string_view rText)
{
    for (sal_Int32 i = 0; i < static_cast<sal_Int32>(rText.size()); ++i)
    {
        if (ScriptTypeDetector::getScriptDirection(rText, i,
                                                   css::i18n::ScriptDirection::LEFT_TO_RIGHT)
            == css::i18n::ScriptDirection::RIGHT_TO_LEFT)
            return true;
    }
    return false;
}

void lcl_AppendExtraFooterText(drawinglayer::primitive2d::Primitive2DContainer& rContainer,
                               const OUString& rText, const basegfx::B2DVector& rFontSize,
                               bool bBold, double fPosX, double fPosY, double fAvailWidth,
                               const basegfx::BColor& rColor)
{
    if (rText.isEmpty())
        return;
    // Per-string bidi: Arabic / Hebrew names need RTL, numeric dates stay LTR.
    basegfx::B2DVector aDummySize;
    const drawinglayer::attribute::FontAttribute aFontAttr
        = lcl_GetExtraFooterFontAttribute(aDummySize, bBold, lcl_HasRTLChars(rText));
    const OUString sDisplay = lcl_TruncateWithEllipsis(rText, aFontAttr, rFontSize, fAvailWidth);
    if (sDisplay.isEmpty())
        return;
    basegfx::B2DHomMatrix aTextMatrix(basegfx::utils::createScaleTranslateB2DHomMatrix(
        rFontSize.getX(), rFontSize.getY(), fPosX, fPosY));
    rContainer.append(new drawinglayer::primitive2d::TextSimplePortionPrimitive2D(
        aTextMatrix, sDisplay, /*nTextPosition*/ 0, sDisplay.getLength(), std::vector<double>(),
        std::vector<sal_Bool>(), aFontAttr, css::lang::Locale(), rColor));
}

/// Pick a footer color that stays legible against the caption's fill.
basegfx::BColor lcl_GetExtraFooterColor(const SdrCaptionObj& rCaptionObj)
{
    const basegfx::BColor aDarkOnLight(0.102, 0.102, 0.102); // #1A1A1A
    const basegfx::BColor aLightOnDark(0.902, 0.902, 0.902); // #E6E6E6
    const SfxItemSet& rItemSet = rCaptionObj.GetMergedItemSet();
    if (rItemSet.Get(XATTR_FILLSTYLE).GetValue() == css::drawing::FillStyle_SOLID
        && rItemSet.Get(XATTR_FILLCOLOR).GetColorValue().IsDark())
    {
        return aLightOnDark;
    }
    return aDarkOnLight;
}
} // namespace

namespace sdr::contact
{
drawinglayer::primitive2d::Primitive2DReference
createExtraFooterPrimitive(const SdrCaptionObj& rCaptionObj)
{
    const sal_Int32 nFooterH = rCaptionObj.GetExtraFooterHeight();
    if (nFooterH <= 0)
        return nullptr;
    const OUString& rLine1 = rCaptionObj.GetExtraFooterLine1();
    const OUString& rLine2 = rCaptionObj.GetExtraFooterLine2();
    if (rLine1.isEmpty() && rLine2.isEmpty())
        return nullptr;

    const tools::Rectangle aLogic = rCaptionObj.GetLogicRect();
    const tools::Rectangle aFooter(aLogic.Left(), aLogic.Bottom() - nFooterH, aLogic.Right(),
                                   aLogic.Bottom());

    const basegfx::BColor aDimColor = lcl_GetExtraFooterColor(rCaptionObj);

    // Collect into a container so we can MaskPrimitive2D-clip the band.
    drawinglayer::primitive2d::Primitive2DContainer aFooterContent;

    // Separator hairline.
    {
        basegfx::B2DPolygon aLine;
        aLine.append(basegfx::B2DPoint(aFooter.Left() + EXTRA_FOOTER_PADDING_X, aFooter.Top()));
        aLine.append(basegfx::B2DPoint(aFooter.Right() - EXTRA_FOOTER_PADDING_X, aFooter.Top()));
        aFooterContent.append(
            new drawinglayer::primitive2d::PolygonHairlinePrimitive2D(std::move(aLine), aDimColor));
    }

    // Measure font sizes once (RTL doesn't affect metrics); per-line
    // FontAttribute is rebuilt inside lcl_AppendExtraFooterText.
    basegfx::B2DVector aBoldSize, aSize;
    lcl_GetExtraFooterFontAttribute(aBoldSize, /*bold*/ true, /*bRTL*/ false);
    lcl_GetExtraFooterFontAttribute(aSize, /*bold*/ false, /*bRTL*/ false);

    const double fLeftX = static_cast<double>(aFooter.Left() + EXTRA_FOOTER_PADDING_X);
    const double fLine1Y
        = static_cast<double>(aFooter.Top() + EXTRA_FOOTER_PADDING_Y) + aBoldSize.getY();
    const double fLine2Y = fLine1Y + aSize.getY() + EXTRA_FOOTER_PADDING_Y;
    const double fAvailWidth = static_cast<double>(aFooter.GetWidth() - 2 * EXTRA_FOOTER_PADDING_X);

    lcl_AppendExtraFooterText(aFooterContent, rLine1, aBoldSize, /*bBold*/ true, fLeftX, fLine1Y,
                              fAvailWidth, aDimColor);
    lcl_AppendExtraFooterText(aFooterContent, rLine2, aSize, /*bBold*/ false, fLeftX, fLine2Y,
                              fAvailWidth, aDimColor);

    // Clip the band to the inner-padded footer rectangle.
    basegfx::B2DPolygon aClipRect(basegfx::utils::createPolygonFromRect(
        basegfx::B2DRange(aFooter.Left() + EXTRA_FOOTER_PADDING_X, aFooter.Top(),
                          aFooter.Right() - EXTRA_FOOTER_PADDING_X, aFooter.Bottom())));
    return new drawinglayer::primitive2d::MaskPrimitive2D(
        basegfx::B2DPolyPolygon(std::move(aClipRect)), std::move(aFooterContent));
}

} // namespace sdr::contact

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
