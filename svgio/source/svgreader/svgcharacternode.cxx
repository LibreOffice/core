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

#include <svgcharacternode.hxx>
#include <svgstyleattributes.hxx>
#include <drawinglayer/primitive2d/textprimitive2d.hxx>
#include <drawinglayer/primitive2d/textlayoutdevice.hxx>
#include <drawinglayer/primitive2d/textbreakuphelper.hxx>
#include <drawinglayer/primitive2d/textdecoratedprimitive2d.hxx>
#include <drawinglayer/primitive2d/unifiedtransparenceprimitive2d.hxx>
#include <utility>
#include <o3tl/string_view.hxx>
#include <osl/diagnose.h>

using namespace drawinglayer::primitive2d;

namespace svgio::svgreader
{
        namespace {

        class localTextBreakupHelper : public TextBreakupHelper
        {
        private:
            SvgTextPosition&                    mrSvgTextPosition;

        protected:
            /// allow user callback to allow changes to the new TextTransformation. Default
            /// does nothing.
            virtual bool allowChange(sal_uInt32 nCount, basegfx::B2DHomMatrix& rNewTransform, sal_uInt32 nIndex, sal_uInt32 nLength) override;

        public:
            localTextBreakupHelper(
                const TextSimplePortionPrimitive2D& rSource,
                SvgTextPosition& rSvgTextPosition)
            :   TextBreakupHelper(rSource),
                mrSvgTextPosition(rSvgTextPosition)
            {
            }
        };

        }

        bool localTextBreakupHelper::allowChange(sal_uInt32 /*nCount*/, basegfx::B2DHomMatrix& rNewTransform, sal_uInt32 /*nIndex*/, sal_uInt32 /*nLength*/)
        {
            const double fRotation(mrSvgTextPosition.consumeRotation());

            if(0.0 != fRotation)
            {
                const basegfx::B2DPoint aBasePoint(rNewTransform * basegfx::B2DPoint(0.0, 0.0));

                rNewTransform.translate(-aBasePoint.getX(), -aBasePoint.getY());
                rNewTransform.rotate(fRotation);
                rNewTransform.translate(aBasePoint.getX(), aBasePoint.getY());
            }

            return true;
        }

        SvgCharacterNode::SvgCharacterNode(
            SvgDocument& rDocument,
            SvgNode* pParent,
            OUString aText)
        :   SvgNode(SVGToken::Character, rDocument, pParent),
            maText(std::move(aText)),
            mpParentLine(nullptr)
        {
        }

        SvgCharacterNode::~SvgCharacterNode()
        {
        }

        const SvgStyleAttributes* SvgCharacterNode::getSvgStyleAttributes() const
        {
            // no own style, use parent's
            if(getParent())
            {
                return getParent()->getSvgStyleAttributes();
            }
            else
            {
                return nullptr;
            }
        }

        drawinglayer::attribute::FontAttribute SvgCharacterNode::getFontAttribute(
            const SvgStyleAttributes& rSvgStyleAttributes)
        {
            const SvgStringVector& rFontFamilyVector = rSvgStyleAttributes.getFontFamily();
            OUString aFontFamily("Times New Roman");
            if(!rFontFamilyVector.empty())
                aFontFamily=rFontFamilyVector[0];

            // #i122324# if the FontFamily name ends on ' embedded' it is probably a re-import
            // of a SVG export with font embedding. Remove this to make font matching work. This
            // is pretty safe since there should be no font family names ending on ' embedded'.
            // Remove again when FontEmbedding is implemented in SVG import
            if(aFontFamily.endsWith(" embedded"))
            {
                aFontFamily = aFontFamily.copy(0, aFontFamily.getLength() - 9);
            }

            const ::FontWeight nFontWeight(getVclFontWeight(rSvgStyleAttributes.getFontWeight()));
            bool bItalic(FontStyle::italic == rSvgStyleAttributes.getFontStyle() || FontStyle::oblique == rSvgStyleAttributes.getFontStyle());

            return drawinglayer::attribute::FontAttribute(
                aFontFamily,
                OUString(),
                nFontWeight,
                false/*bSymbol*/,
                false/*bVertical*/,
                bItalic,
                false/*bMonospaced*/,
                false/*bOutline*/,
                false/*bRTL*/,
                false/*bBiDiStrong*/);
        }

        rtl::Reference<BasePrimitive2D> SvgCharacterNode::createSimpleTextPrimitive(
            SvgTextPosition& rSvgTextPosition,
            const SvgStyleAttributes& rSvgStyleAttributes) const
        {
            // prepare retval, index and length
            rtl::Reference<BasePrimitive2D> pRetval;
            const sal_uInt32 nLength(getText().getLength());

            if(nLength)
            {
                const sal_uInt32 nIndex(0);

                // prepare FontAttribute
                const drawinglayer::attribute::FontAttribute aFontAttribute(getFontAttribute(rSvgStyleAttributes));

                // prepare FontSizeNumber
                double fFontWidth(rSvgStyleAttributes.getFontSizeNumber().solve(*this));
                double fFontHeight(fFontWidth);

                // prepare locale
                css::lang::Locale aLocale;

                // prepare TextLayouterDevice; use a larger font size for more linear size
                // calculations. Similar to nTextSizeFactor in sd/source/ui/view/sdview.cxx
                // (ViewRedirector::createRedirectedPrimitive2DSequence).
                const double sizeFactor = fFontHeight < 50000 ? 50000 / fFontHeight : 1.0;
                TextLayouterDevice aTextLayouterDevice;
                aTextLayouterDevice.setFontAttribute(aFontAttribute, fFontWidth * sizeFactor, fFontHeight * sizeFactor, aLocale);

                // prepare TextArray
                ::std::vector< double > aTextArray(rSvgTextPosition.getX());
                ::std::vector< double > aDxArray(rSvgTextPosition.getDx());

                // Do nothing when X and Dx arrays are empty
                if((!aTextArray.empty() || !aDxArray.empty()) && aTextArray.size() < nLength)
                {
                    const sal_uInt32 nArray(aTextArray.size());

                    double fStartX(0.0);
                    if (!aTextArray.empty())
                    {
                        if(rSvgTextPosition.getParent() && rSvgTextPosition.getParent()->getAbsoluteX())
                        {
                            fStartX = rSvgTextPosition.getParent()->getPosition().getX();
                        }
                        else
                        {
                            fStartX = aTextArray[nArray - 1];
                        }
                    }

                    ::std::vector< double > aExtendArray(aTextLayouterDevice.getTextArray(getText(), nArray, nLength - nArray));
                    double fComulativeDx(0.0);

                    aTextArray.reserve(nLength);
                    for(size_t a = 0; a < aExtendArray.size(); ++a)
                    {
                        if (a < aDxArray.size())
                        {
                            fComulativeDx += aDxArray[a];
                        }
                        aTextArray.push_back(aExtendArray[a] / sizeFactor + fStartX + fComulativeDx);
                    }
                }

                // get current TextPosition and TextWidth in units
                basegfx::B2DPoint aPosition(rSvgTextPosition.getPosition());
                double fTextWidth(aTextLayouterDevice.getTextWidth(getText(), nIndex, nLength) / sizeFactor);

                // check for user-given TextLength
                if(0.0 != rSvgTextPosition.getTextLength()
                    && !basegfx::fTools::equal(fTextWidth, rSvgTextPosition.getTextLength()))
                {
                    const double fFactor(rSvgTextPosition.getTextLength() / fTextWidth);

                    if(rSvgTextPosition.getLengthAdjust())
                    {
                        // spacing, need to create and expand TextArray
                        if(aTextArray.empty())
                        {
                            auto aExtendArray(aTextLayouterDevice.getTextArray(getText(), nIndex, nLength));
                            aTextArray.reserve(aExtendArray.size());
                            for (auto n : aExtendArray)
                                aTextArray.push_back(n / sizeFactor);
                        }

                        for(auto &a : aTextArray)
                        {
                            a *= fFactor;
                        }
                    }
                    else
                    {
                        // spacing and glyphs, just apply to FontWidth
                        fFontWidth *= fFactor;
                    }

                    fTextWidth = rSvgTextPosition.getTextLength();
                }

                // get TextAlign
                TextAlign aTextAlign(rSvgStyleAttributes.getTextAlign());

                // map TextAnchor to TextAlign, there seems not to be a difference
                if(TextAnchor::notset != rSvgStyleAttributes.getTextAnchor())
                {
                    switch(rSvgStyleAttributes.getTextAnchor())
                    {
                        case TextAnchor::start:
                        {
                            aTextAlign = TextAlign::left;
                            break;
                        }
                        case TextAnchor::middle:
                        {
                            aTextAlign = TextAlign::center;
                            break;
                        }
                        case TextAnchor::end:
                        {
                            aTextAlign = TextAlign::right;
                            break;
                        }
                        default:
                        {
                            break;
                        }
                    }
                }

                // apply TextAlign
                switch(aTextAlign)
                {
                    case TextAlign::right:
                    {
                        aPosition.setX(aPosition.getX() - mpParentLine->getTextLineWidth());
                        break;
                    }
                    case TextAlign::center:
                    {
                        aPosition.setX(aPosition.getX() - (mpParentLine->getTextLineWidth() * 0.5));
                        break;
                    }
                    case TextAlign::notset:
                    case TextAlign::left:
                    case TextAlign::justify:
                    {
                        // TextAlign::notset, TextAlign::left: nothing to do
                        // TextAlign::justify is not clear currently; handle as TextAlign::left
                        break;
                    }
                }

                // get DominantBaseline
                const DominantBaseline aDominantBaseline(rSvgStyleAttributes.getDominantBaseline());

                basegfx::B2DRange aRange(aTextLayouterDevice.getTextBoundRect(getText(), nIndex, nLength));
                // apply DominantBaseline
                switch(aDominantBaseline)
                {
                    case DominantBaseline::Middle:
                    case DominantBaseline::Central:
                    {
                        aPosition.setY(aPosition.getY() - aRange.getCenterY() / sizeFactor);
                        break;
                    }
                    case DominantBaseline::Hanging:
                    {
                        aPosition.setY(aPosition.getY() - aRange.getMinY() / sizeFactor);
                        break;
                    }
                    default: // DominantBaseline::Auto
                    {
                        // nothing to do
                        break;
                    }
                }

                // get BaselineShift
                const BaselineShift aBaselineShift(rSvgStyleAttributes.getBaselineShift());

                // apply BaselineShift
                switch(aBaselineShift)
                {
                    case BaselineShift::Sub:
                    {
                        aPosition.setY(aPosition.getY() + aTextLayouterDevice.getUnderlineOffset() / sizeFactor);
                        break;
                    }
                    case BaselineShift::Super:
                    {
                        aPosition.setY(aPosition.getY() + aTextLayouterDevice.getOverlineOffset() / sizeFactor);
                        break;
                    }
                    case BaselineShift::Percentage:
                    case BaselineShift::Length:
                    {
                        const SvgNumber aNumber(rSvgStyleAttributes.getBaselineShiftNumber());
                        const double mfBaselineShift(aNumber.solve(*this));

                        aPosition.setY(aPosition.getY() - mfBaselineShift);
                        break;
                    }
                    default: // BaselineShift::Baseline
                    {
                        // nothing to do
                        break;
                    }
                }

                // get fill color
                basegfx::BColor aFill(0, 0, 0);
                if(rSvgStyleAttributes.getFill())
                    aFill = *rSvgStyleAttributes.getFill();

                // get fill opacity
                double fFillOpacity = 1.0;
                if (rSvgStyleAttributes.getFillOpacity().isSet())
                {
                    fFillOpacity = rSvgStyleAttributes.getFillOpacity().getNumber();
                }

                // prepare TextTransformation
                basegfx::B2DHomMatrix aTextTransform;

                aTextTransform.scale(fFontWidth, fFontHeight);
                aTextTransform.translate(aPosition.getX(), aPosition.getY());

                // check TextDecoration and if TextDecoratedPortionPrimitive2D is needed
                const TextDecoration aDeco(rSvgStyleAttributes.getTextDecoration());

                if(TextDecoration::underline == aDeco
                    || TextDecoration::overline == aDeco
                    || TextDecoration::line_through == aDeco)
                {
                    // get the fill for decoration as described by SVG. We cannot
                    // have different stroke colors/definitions for those, though
                    const SvgStyleAttributes* pDecoDef = rSvgStyleAttributes.getTextDecorationDefiningSvgStyleAttributes();

                    basegfx::BColor aDecoColor(aFill);
                    if(pDecoDef && pDecoDef->getFill())
                        aDecoColor = *pDecoDef->getFill();

                    TextLine eFontOverline = TEXT_LINE_NONE;
                    if(TextDecoration::overline == aDeco)
                        eFontOverline = TEXT_LINE_SINGLE;

                    TextLine eFontUnderline = TEXT_LINE_NONE;
                    if(TextDecoration::underline == aDeco)
                        eFontUnderline = TEXT_LINE_SINGLE;

                    TextStrikeout eTextStrikeout = TEXT_STRIKEOUT_NONE;
                    if(TextDecoration::line_through == aDeco)
                        eTextStrikeout = TEXT_STRIKEOUT_SINGLE;

                    // create decorated text primitive
                    pRetval = new TextDecoratedPortionPrimitive2D(
                        aTextTransform,
                        getText(),
                        nIndex,
                        nLength,
                        std::move(aTextArray),
                        {},
                        aFontAttribute,
                        aLocale,
                        aFill,
                        COL_TRANSPARENT,

                        // extra props for decorated
                        aDecoColor,
                        aDecoColor,
                        eFontOverline,
                        eFontUnderline,
                        false,
                        eTextStrikeout,
                        false,
                        TEXT_FONT_EMPHASIS_MARK_NONE,
                        true,
                        false,
                        TEXT_RELIEF_NONE,
                        false);
                }
                else
                {
                    // create text primitive
                    pRetval = new TextSimplePortionPrimitive2D(
                        aTextTransform,
                        getText(),
                        nIndex,
                        nLength,
                        std::move(aTextArray),
                        {},
                        aFontAttribute,
                        aLocale,
                        aFill);
                }

                if (fFillOpacity != 1.0)
                {
                    pRetval = new UnifiedTransparencePrimitive2D(
                        drawinglayer::primitive2d::Primitive2DContainer{ pRetval },
                        1.0 - fFillOpacity);
                }

                // advance current TextPosition
                rSvgTextPosition.setPosition(rSvgTextPosition.getPosition() + basegfx::B2DVector(fTextWidth, 0.0));
            }

            return pRetval;
        }

        void SvgCharacterNode::decomposeTextWithStyle(
            Primitive2DContainer& rTarget,
            SvgTextPosition& rSvgTextPosition,
            const SvgStyleAttributes& rSvgStyleAttributes) const
        {
            const Primitive2DReference xRef(
                createSimpleTextPrimitive(
                    rSvgTextPosition,
                    rSvgStyleAttributes));

            if(!(xRef.is() && (Visibility::visible == rSvgStyleAttributes.getVisibility())))
                return;

            if(!rSvgTextPosition.isRotated())
            {
                rTarget.push_back(xRef);
            }
            else
            {
                // need to apply rotations to each character as given
                const TextSimplePortionPrimitive2D* pCandidate =
                    dynamic_cast< const TextSimplePortionPrimitive2D* >(xRef.get());

                if(pCandidate)
                {
                    localTextBreakupHelper alocalTextBreakupHelper(*pCandidate, rSvgTextPosition);
                    Primitive2DContainer aResult = alocalTextBreakupHelper.extractResult();

                    if(!aResult.empty())
                    {
                        rTarget.append(std::move(aResult));
                    }

                    // also consume for the implied single space
                    rSvgTextPosition.consumeRotation();
                }
                else
                {
                    OSL_ENSURE(false, "Used primitive is not a text primitive (!)");
                }
            }
        }

        void SvgCharacterNode::whiteSpaceHandling()
        {
            bool bIsDefault(XmlSpace::Default == getXmlSpace());
            // if xml:space="default" then remove all newline characters, otherwise convert them to space
            // convert tab to space too
            maText = maTextBeforeSpaceHandling = maText.replaceAll(u"\n", bIsDefault ? u"" : u" ").replaceAll(u"\t", u" ");

            if(bIsDefault)
            {
                // strip of all leading and trailing spaces
                // and consolidate contiguous space
                maText = consolidateContiguousSpace(maText.trim());
            }
        }

        SvgCharacterNode* SvgCharacterNode::addGap(SvgCharacterNode* pPreviousCharacterNode)
        {
            // maText may have lost all text. If that's the case, ignore as invalid character node
            // Also ignore if maTextBeforeSpaceHandling just have spaces
            if(!maText.isEmpty() && !o3tl::trim(maTextBeforeSpaceHandling).empty())
            {
                if(pPreviousCharacterNode)
                {
                    bool bAddGap(true);

                    // Do not add a gap if last node doesn't end with a space and
                    // current note doesn't start with a space
                    const sal_uInt32 nLastLength(pPreviousCharacterNode->maTextBeforeSpaceHandling.getLength());
                    if(pPreviousCharacterNode->maTextBeforeSpaceHandling[nLastLength - 1] != ' ' && maTextBeforeSpaceHandling[0] != ' ')
                        bAddGap = false;

                    // Do not add a gap if this node and last node are in different lines
                    if(pPreviousCharacterNode->mpParentLine != mpParentLine)
                        bAddGap = false;

                    // add in-between whitespace (single space) to the beginning of the current character node
                    if(bAddGap)
                    {
                        maText = " " + maText;
                    }
                }

                // this becomes the previous character node
                return this;
            }

            return pPreviousCharacterNode;
        }

        void SvgCharacterNode::concatenate(std::u16string_view rText)
        {
            maText += rText;
        }

        void SvgCharacterNode::decomposeText(Primitive2DContainer& rTarget, SvgTextPosition& rSvgTextPosition) const
        {
            if(!getText().isEmpty())
            {
                const SvgStyleAttributes* pSvgStyleAttributes = getSvgStyleAttributes();

                if(pSvgStyleAttributes)
                {
                    decomposeTextWithStyle(rTarget, rSvgTextPosition, *pSvgStyleAttributes);
                }
            }
        }

} // end of namespace svgio

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
