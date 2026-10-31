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

#include <VectorText.hxx>

#include <drawfont.hxx>
#include <EnhancedPDFExportHelper.hxx>
#include <editeng/adjustitem.hxx>
#include <ndtxt.hxx>
#include <pagefrm.hxx>
#include <paratr.hxx>
#include <rootfrm.hxx>
#include <txtfrm.hxx>
#include <viewsh.hxx>
#include "inftxt.hxx"
#include "itrpaint.hxx"
#include "txtpaint.hxx"

#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <drawinglayer/primitive2d/PolyPolygonColorPrimitive2D.hxx>
#include <drawinglayer/primitive2d/PolyPolygonHairlinePrimitive2D.hxx>
#include <drawinglayer/primitive2d/fonttextprimitive.hxx>
#include <drawinglayer/primitive2d/graphicprimitive2d.hxx>
#include <drawinglayer/primitive2d/groupprimitive2d.hxx>
#include <drawinglayer/primitive2d/maskprimitive2d.hxx>
#include <drawinglayer/primitive2d/texthierarchyprimitive2d.hxx>
#include <drawinglayer/primitive2d/unifiedtransparenceprimitive2d.hxx>
#include <o3tl/safeint.hxx>
#include <tools/degree.hxx>
#include <vcl/GraphicObject.hxx>
#include <vcl/outdev.hxx>
#include <vcl/rendercontext/State.hxx>

#include <algorithm>
#include <cmath>
#include <iterator>
#include <optional>

namespace sw::vector
{
namespace
{
/// Tags a text portion, or the portion inside the group that puts a fill behind it, with the
/// line and the text area it is laid out in.
void tagPortion(drawinglayer::primitive2d::BasePrimitive2D* pPrimitive,
                const drawinglayer::primitive2d::TextPortionLayout& rLayout)
{
    if (auto* pPortion
        = dynamic_cast<drawinglayer::primitive2d::TextSimplePortionPrimitive2D*>(pPrimitive))
    {
        pPortion->setLayout(rLayout);
        return;
    }

    if (auto* pGroup = dynamic_cast<drawinglayer::primitive2d::GroupPrimitive2D*>(pPrimitive))
    {
        for (const drawinglayer::primitive2d::Primitive2DReference& rChild : pGroup->getChildren())
            tagPortion(rChild.get(), rLayout);
    }
}

/// The range of rArea, which is in document twips, relative to rOrigin.
basegfx::B2DRange rangeOf(const SwRect& rArea, const Point& rOrigin)
{
    return basegfx::B2DRange(rArea.Left() - rOrigin.X(), rArea.Top() - rOrigin.Y(),
                             rArea.Left() + rArea.Width() - rOrigin.X(),
                             rArea.Top() + rArea.Height() - rOrigin.Y());
}

/// Hairlines in rColor along rLines, which are in document twips, relative to rOrigin.
drawinglayer::primitive2d::Primitive2DReference
createHairlines(const basegfx::B2DPolyPolygon& rLines, const Color& rColor, const Point& rOrigin)
{
    basegfx::B2DPolyPolygon aLines(rLines);
    aLines.transform(basegfx::utils::createTranslateB2DHomMatrix(-rOrigin.X(), -rOrigin.Y()));
    return new drawinglayer::primitive2d::PolyPolygonHairlinePrimitive2D(std::move(aLines),
                                                                          rColor.getBColor());
}
}

/// rPrimitives cut to rArea, which is in document twips. The primitives and the result are
/// relative to rOrigin.
drawinglayer::primitive2d::Primitive2DReference
createMask(const SwRect& rArea, const Point& rOrigin,
           drawinglayer::primitive2d::Primitive2DContainer&& rPrimitives)
{
    return new drawinglayer::primitive2d::MaskPrimitive2D(
        basegfx::B2DPolyPolygon(basegfx::utils::createPolygonFromRect(rangeOf(rArea, rOrigin))),
        std::move(rPrimitives));
}

TextSink::TextSink(const Point& rOrigin, const SwRect& rTextArea,
                   drawinglayer::primitive2d::TextPortionAlign eAlign, bool bForView)
    : maOrigin(rOrigin)
    , maTextArea(rTextArea)
    , mbForView(bForView)
    , mbParagraph(true)
{
    // A text frame shows a single paragraph.
    maLayout.mnParagraph = 0;
    maLayout.meAlign = eAlign;
    maLayout.mfTextAreaWidth = rTextArea.Width();
    maLayout.mfTextAreaHeight = rTextArea.Height();
}

TextSink::TextSink(const Point& rOrigin, bool bForView)
    : maOrigin(rOrigin)
    , mbForView(bForView)
    , mbParagraph(false)
{
}

void TextSink::startLine(SwTwips nLeft, SwTwips nWidth, SwTwips nAscent, SwTwips nHeight)
{
    finishLine();
    maLayout.mfLineIndent = nLeft - maTextArea.Left();
    maLayout.mfLineWidth = nWidth;
    maLayout.mfLineAscent = nAscent;
    maLayout.mfLineHeight = nHeight;
}

void TextSink::addText(const OutputDevice& rDevice, const Point& rPosition, const OUString& rText,
                       sal_Int32 nIndex, sal_Int32 nLength, KernArraySpan aAdvances,
                       std::span<const bool> aKashida)
{
    if (nLength <= 0)
        return;

    // The array starts with the advances of the range. Any past it belong to another draw call.
    std::vector<double> aDXArray(aAdvances.begin(), aAdvances.end());
    if (aDXArray.empty())
    {
        KernArray aMeasured;
        rDevice.GetTextArray(rText, &aMeasured, nIndex, nLength);
        aDXArray.assign(aMeasured.begin(), aMeasured.end());
    }
    if (aDXArray.size() > o3tl::make_unsigned(nLength))
        aDXArray.resize(nLength);

    // The kashida flags start with the range too. Flags shorter than the range count as none.
    boost::container::vector<bool> aKashidaArray;
    if (aKashida.size() >= o3tl::make_unsigned(nLength))
        aKashidaArray.assign(aKashida.begin(), aKashida.begin() + nLength);

    // Writer draws the background of characters itself, so the text takes no fill of its own.
    drawinglayer::primitive2d::TextDrawState aState;
    aState.maTextColor = rDevice.GetTextColor().getBColor();
    // Text left in the automatic color stays automatic, which means the color that reads on the
    // paper under it.
    if (!mbForView && rDevice.GetFont().GetColor() == COL_AUTO)
    {
        aState.maTextColor = basegfx::BColor();
        aState.maTextColor.setAutomatic(true);
    }
    if (rDevice.IsOverlineColor())
        aState.moOverlineColor = rDevice.GetOverlineColor().getBColor();
    if (rDevice.IsTextLineColor())
        aState.moTextLineColor = rDevice.GetTextLineColor().getBColor();
    aState.meLayoutMode = rDevice.GetLayoutMode();
    aState.meLanguage = rDevice.GetFont().GetLanguage();

    // The device puts the right end of a right-to-left run at rPosition unless the layout mode
    // asks for the left end. A text primitive starts at the left end, so the start moves back
    // along the baseline by the width of the run. The last advance is the distance from the start
    // of the run to the end of its last character, which is that width.
    basegfx::B2DPoint aStart(rPosition.X() - maOrigin.X(), rPosition.Y() - maOrigin.Y());
    const vcl::text::ComplexTextLayoutFlags eLayoutMode = aState.meLayoutMode;
    const bool bRightToLeft(eLayoutMode & vcl::text::ComplexTextLayoutFlags::BiDiRtl);
    const bool bOriginLeft(eLayoutMode & vcl::text::ComplexTextLayoutFlags::TextOriginLeft);
    const bool bOriginRight(eLayoutMode & vcl::text::ComplexTextLayoutFlags::TextOriginRight);
    if (((bRightToLeft && !bOriginLeft) || bOriginRight) && !aDXArray.empty())
    {
        const double fAngle = -toRadians(rDevice.GetFont().GetOrientation());
        const double fWidth = aDXArray.back();
        aStart.adjustX(-std::cos(fAngle) * fWidth);
        aStart.adjustY(-std::sin(fAngle) * fWidth);
    }

    const drawinglayer::primitive2d::Primitive2DReference xText(
        drawinglayer::primitive2d::createTextPrimitiveFromFont(
            aStart, rText, nIndex, nLength, std::move(aDXArray), std::move(aKashidaArray),
            rDevice.GetFont(), aState));
    if (!xText)
        return;

    if (mbParagraph)
        tagPortion(xText.get(), maLayout);
    const Color aFontColor(rDevice.GetFont().GetColor());
    if (aFontColor != COL_AUTO && aFontColor.IsTransparent())
        maLine.push_back(new drawinglayer::primitive2d::UnifiedTransparencePrimitive2D(
            drawinglayer::primitive2d::Primitive2DContainer{ xText },
            1.0 - aFontColor.GetAlpha() / 255.0));
    else
        maLine.push_back(xText);
}

void TextSink::addFill(const SwRect& rArea, const Color& rColor)
{
    if (!rArea.HasArea() || rColor.IsFullyTransparent())
        return;

    drawinglayer::primitive2d::Primitive2DReference xFill(
        new drawinglayer::primitive2d::PolyPolygonColorPrimitive2D(
            basegfx::B2DPolyPolygon(
                basegfx::utils::createPolygonFromRect(rangeOf(rArea, maOrigin))),
            rColor.getBColor()));
    if (rColor.IsTransparent())
        xFill = new drawinglayer::primitive2d::UnifiedTransparencePrimitive2D(
            drawinglayer::primitive2d::Primitive2DContainer{ xFill },
            1.0 - rColor.GetAlpha() / 255.0);
    maLine.push_back(xFill);
}

void TextSink::addLines(const basegfx::B2DPolyPolygon& rLines, const Color& rColor)
{
    if (rLines.count() != 0)
        maLine.push_back(createHairlines(rLines, rColor, maOrigin));
}

void TextSink::addLinesOverObjects(const basegfx::B2DPolyPolygon& rLines, const Color& rColor)
{
    if (rLines.count() != 0)
        maOverObjects.push_back(createHairlines(rLines, rColor, maOrigin));
}

void TextSink::startClip(const SwRect& rArea)
{
    endClip();
    moClipArea = rArea;
    mnClipStart = maLine.size();
}

void TextSink::endClip()
{
    if (!moClipArea)
        return;

    const SwRect aArea(*moClipArea);
    moClipArea.reset();
    if (maLine.size() <= mnClipStart)
        return;

    drawinglayer::primitive2d::Primitive2DContainer aClipped;
    aClipped.insert(aClipped.end(), maLine.begin() + mnClipStart, maLine.end());
    maLine.erase(maLine.begin() + mnClipStart, maLine.end());
    maLine.push_back(createMask(aArea, maOrigin, std::move(aClipped)));
}

void TextSink::addGraphic(const SwRect& rArea, const GraphicObject& rGraphic)
{
    if (!rArea.HasArea())
        return;

    maLine.push_back(new drawinglayer::primitive2d::GraphicPrimitive2D(
        basegfx::utils::createScaleTranslateB2DHomMatrix(
            rArea.Width(), rArea.Height(), rArea.Left() - maOrigin.X(),
            rArea.Top() - maOrigin.Y()),
        rGraphic, rGraphic.GetAttr()));
}

void TextSink::add(drawinglayer::primitive2d::Primitive2DContainer&& rPrimitives)
{
    if (!rPrimitives.empty())
        maLine.append(std::move(rPrimitives));
}

void TextSink::startGroup() { mnGroupStart = maLine.size(); }

void TextSink::endGroup(PortionGroup eGroup)
{
    if (mnGroupStart >= maLine.size())
        return;

    const auto aStart = maLine.begin() + mnGroupStart;
    drawinglayer::primitive2d::Primitive2DContainer aGroup;
    std::move(aStart, maLine.end(), std::back_inserter(aGroup));
    maLine.erase(aStart, maLine.end());
    // A label is the bullet of the paragraph, as the text of a drawing object marks it. Its
    // text is not text of the paragraph.
    if (eGroup == PortionGroup::Label)
        maLine.push_back(
            new drawinglayer::primitive2d::TextHierarchyBulletPrimitive2D(std::move(aGroup)));
    else
        maLine.push_back(new drawinglayer::primitive2d::TextHierarchyFieldPrimitive2D(
            std::move(aGroup), drawinglayer::primitive2d::FIELD_TYPE_COMMON));
}

void TextSink::shiftLineStart(SwTwips nDistance)
{
    maLayout.mfLineIndent += nDistance;
    maLayout.mfLineWidth -= nDistance;
}

void TextSink::finishLine()
{
    endClip();
    if (maLine.empty())
        return;

    maLines.push_back(
        new drawinglayer::primitive2d::TextHierarchyLinePrimitive2D(std::move(maLine)));
    maLine = drawinglayer::primitive2d::Primitive2DContainer();
}

drawinglayer::primitive2d::Primitive2DContainer TextSink::take()
{
    if (!mbParagraph)
    {
        endClip();
        drawinglayer::primitive2d::Primitive2DContainer aDrawn(std::move(maLine));
        maLine = drawinglayer::primitive2d::Primitive2DContainer();
        return aDrawn;
    }

    finishLine();
    drawinglayer::primitive2d::Primitive2DContainer aBlock;
    if (maLines.empty())
        return aBlock;

    drawinglayer::primitive2d::Primitive2DContainer aParagraph{
        new drawinglayer::primitive2d::TextHierarchyParagraphPrimitive2D(std::move(maLines))
    };
    maLines = drawinglayer::primitive2d::Primitive2DContainer();
    aBlock.push_back(
        new drawinglayer::primitive2d::TextHierarchyBlockPrimitive2D(std::move(aParagraph)));
    return aBlock;
}

drawinglayer::primitive2d::Primitive2DContainer TextSink::takeOverObjects()
{
    drawinglayer::primitive2d::Primitive2DContainer aOverObjects(std::move(maOverObjects));
    maOverObjects = drawinglayer::primitive2d::Primitive2DContainer();
    return aOverObjects;
}

void drawTextArray(SwDrawTextInfo& rInfo, const Point& rPosition, const OUString& rText,
                   KernArraySpan aAdvances, std::span<const bool> aKashida, sal_Int32 nIndex,
                   sal_Int32 nLength)
{
    if (TextSink* pSink = rInfo.GetPrimitiveSink())
    {
        pSink->addText(rInfo.GetOut(), rPosition, rText, nIndex, nLength, aAdvances, aKashida);
        return;
    }
    rInfo.GetOut().DrawTextArray(rPosition, rText, aAdvances, aKashida, nIndex, nLength);
}

void drawText(SwDrawTextInfo& rInfo, const Point& rPosition, const OUString& rText,
              sal_Int32 nIndex, sal_Int32 nLength)
{
    if (TextSink* pSink = rInfo.GetPrimitiveSink())
    {
        pSink->addText(rInfo.GetOut(), rPosition, rText, nIndex, nLength, {}, {});
        return;
    }
    rInfo.GetOut().DrawText(rPosition, rText, nIndex, nLength);
}

void drawStretchText(SwDrawTextInfo& rInfo, const Point& rPosition, tools::Long nWidth,
                     const OUString& rText, sal_Int32 nIndex, sal_Int32 nLength)
{
    if (TextSink* pSink = rInfo.GetPrimitiveSink())
    {
        // Scale the natural advances so the text ends at nWidth.
        KernArray aAdvances;
        rInfo.GetOut().GetTextArray(rText, &aAdvances, nIndex, nLength);
        if (!aAdvances.empty() && aAdvances.back() > 0.0)
        {
            const double fScale = double(nWidth) / aAdvances.back();
            for (double& rAdvance : aAdvances)
                rAdvance *= fScale;
        }
        pSink->addText(rInfo.GetOut(), rPosition, rText, nIndex, nLength, aAdvances, {});
        return;
    }
    rInfo.GetOut().DrawStretchText(rPosition, nWidth, rText, nIndex, nLength);
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
