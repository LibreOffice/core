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
#include <drawinglayer/attribute/fontattribute.hxx>
#include <drawinglayer/primitive2d/textprimitive2d.hxx>
#include <drawinglayer/primitive2d/textlayoutdevice.hxx>
#include <drawinglayer/primitive2d/textbreakuphelper.hxx>
#include <drawinglayer/primitive2d/textdecoratedprimitive2d.hxx>
#include <osl/diagnose.h>

using namespace drawinglayer::primitive2d;

namespace svgio::svgreader
{
        SvgTextPositions::SvgTextPositions()
        :  mbLengthAdjust(true)
        {
        }

        void SvgTextPositions::parseTextPositionAttributes(SVGToken aSVGToken, const OUString& aContent)
        {
            // parse own
            switch(aSVGToken)
            {
                case SVGToken::X:
                {
                    if(!aContent.isEmpty())
                    {
                        SvgNumberVector aVector;

                        if(readSvgNumberVector(aContent, aVector))
                        {
                            setX(std::move(aVector));
                        }
                    }
                    break;
                }
                case SVGToken::Y:
                {
                    if(!aContent.isEmpty())
                    {
                        SvgNumberVector aVector;

                        if(readSvgNumberVector(aContent, aVector))
                        {
                            setY(std::move(aVector));
                        }
                    }
                    break;
                }
                case SVGToken::Dx:
                {
                    if(!aContent.isEmpty())
                    {
                        SvgNumberVector aVector;

                        if(readSvgNumberVector(aContent, aVector))
                        {
                            setDx(std::move(aVector));
                        }
                    }
                    break;
                }
                case SVGToken::Dy:
                {
                    if(!aContent.isEmpty())
                    {
                        SvgNumberVector aVector;

                        if(readSvgNumberVector(aContent, aVector))
                        {
                            setDy(std::move(aVector));
                        }
                    }
                    break;
                }
                case SVGToken::Rotate:
                {
                    if(!aContent.isEmpty())
                    {
                        SvgNumberVector aVector;

                        if(readSvgNumberVector(aContent, aVector))
                        {
                            setRotate(std::move(aVector));
                        }
                    }
                    break;
                }
                case SVGToken::TextLength:
                {
                    SvgNumber aNum;

                    if(readSingleNumber(aContent, aNum))
                    {
                        if(aNum.isPositive())
                        {
                            setTextLength(aNum);
                        }
                    }
                    break;
                }
                case SVGToken::LengthAdjust:
                {
                    if(!aContent.isEmpty())
                    {
                        if(aContent.startsWith("spacing"))
                        {
                            setLengthAdjust(true);
                        }
                        else if(aContent.startsWith("spacingAndGlyphs"))
                        {
                            setLengthAdjust(false);
                        }
                    }
                    break;
                }
                default:
                {
                    break;
                }
            }
        }

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
            const OUString& rText)
        :   SvgNode(SVGToken::Character, rDocument, pParent),
            maText(rText)
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

        rtl::Reference<TextSimplePortionPrimitive2D> SvgCharacterNode::createSimpleTextPrimitive(
            SvgTextPosition& rSvgTextPosition,
            const SvgStyleAttributes& rSvgStyleAttributes) const
        {
            // prepare retval, index and length
            rtl::Reference<TextSimplePortionPrimitive2D> pRetval;
            sal_uInt32 nLength(getText().getLength());

            if(nLength)
            {
                sal_uInt32 nIndex(0);
                // prepare FontAttribute
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

                const drawinglayer::attribute::FontAttribute aFontAttribute(
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

                // prepare FontSizeNumber
                double fFontWidth(rSvgStyleAttributes.getFontSizeNumber().solve(*this));
                double fFontHeight(fFontWidth);

                // prepare locale
                css::lang::Locale aLocale;

                // prepare TextLayouterDevice
                TextLayouterDevice aTextLayouterDevice;
                aTextLayouterDevice.setFontAttribute(aFontAttribute, fFontWidth, fFontHeight, aLocale);

                // prepare TextArray
                ::std::vector< double > aTextArray(rSvgTextPosition.getX());

                if(!aTextArray.empty() && aTextArray.size() < nLength)
                {
                    const sal_uInt32 nArray(aTextArray.size());

                    if(nArray < nLength)
                    {
                        double fStartX(0.0);

                        if(rSvgTextPosition.getParent() && rSvgTextPosition.getParent()->getAbsoluteX())
                        {
                            fStartX = rSvgTextPosition.getParent()->getPosition().getX();
                        }
                        else
                        {
                            fStartX = aTextArray[nArray - 1];
                        }

                        ::std::vector< double > aExtendArray(aTextLayouterDevice.getTextArray(getText(), nArray, nLength - nArray));
                        aTextArray.reserve(nLength);

                        for(const auto &a : aExtendArray)
                        {
                            aTextArray.push_back(a + fStartX);
                        }
                    }
                }

                // get current TextPosition and TextWidth in units
                basegfx::B2DPoint aPosition(rSvgTextPosition.getPosition());
                double fTextWidth(aTextLayouterDevice.getTextWidth(getText(), nIndex, nLength));

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
                            aTextArray = aTextLayouterDevice.getTextArray(getText(), nIndex, nLength);
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
                        aPosition.setX(aPosition.getX() - fTextWidth);
                        break;
                    }
                    case TextAlign::center:
                    {
                        aPosition.setX(aPosition.getX() - (fTextWidth * 0.5));
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

                // get BaselineShift
                const BaselineShift aBaselineShift(rSvgStyleAttributes.getBaselineShift());

                // apply BaselineShift
                switch(aBaselineShift)
                {
                    case BaselineShift::Sub:
                    {
                        aPosition.setY(aPosition.getY() + aTextLayouterDevice.getUnderlineOffset());
                        break;
                    }
                    case BaselineShift::Super:
                    {
                        aPosition.setY(aPosition.getY() + aTextLayouterDevice.getOverlineOffset());
                        break;
                    }
                    case BaselineShift::Percentage:
                    case BaselineShift::Length:
                    {
                        const SvgNumber aNumber(rSvgStyleAttributes.getBaselineShiftNumber());
                        const double mfBaselineShift(aNumber.solve(*this));

                        aPosition.setY(aPosition.getY() + mfBaselineShift);
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
                        aFontAttribute,
                        aLocale,
                        aFill);
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
                    const localTextBreakupHelper alocalTextBreakupHelper(*pCandidate, rSvgTextPosition);
                    const Primitive2DContainer& aResult(
                        alocalTextBreakupHelper.getResult());

                    if(!aResult.empty())
                    {
                        rTarget.append(aResult);
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
            if (XmlSpace::Default == getXmlSpace())
            {
                maText = whiteSpaceHandlingDefault(maText);
            }
            else
            {
                maText = whiteSpaceHandlingPreserve(maText);
            }
        }

        void SvgCharacterNode::addGap()
        {
            maText += " ";
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


        SvgTextPosition::SvgTextPosition(
            SvgTextPosition* pParent,
            const InfoProvider& rInfoProvider,
            const SvgTextPositions& rSvgTextPositions)
        :   mpParent(pParent),
            maRotate(solveSvgNumberVector(rSvgTextPositions.getRotate(), rInfoProvider)),
            mfTextLength(0.0),
            mnRotationIndex(0),
            mbLengthAdjust(rSvgTextPositions.getLengthAdjust()),
            mbAbsoluteX(false)
        {
            // get TextLength if provided
            if(rSvgTextPositions.getTextLength().isSet())
            {
                mfTextLength = rSvgTextPositions.getTextLength().solve(rInfoProvider);
            }

            // SVG does not really define in which units a \91rotate\92 for Text/TSpan is given,
            // but it seems to be degrees. Convert here to radians
            if(!maRotate.empty())
            {
                for (double& f : maRotate)
                {
                    f = basegfx::deg2rad(f);
                }
            }

            // get text positions X
            const sal_uInt32 nSizeX(rSvgTextPositions.getX().size());

            if(nSizeX)
            {
                // we have absolute positions, get first one as current text position X
                maPosition.setX(rSvgTextPositions.getX()[0].solve(rInfoProvider, NumberType::xcoordinate));
                mbAbsoluteX = true;

                if(nSizeX > 1)
                {
                    // fill deltas to maX
                    maX.reserve(nSizeX);

                    for(sal_uInt32 a(1); a < nSizeX; a++)
                    {
                        maX.push_back(rSvgTextPositions.getX()[a].solve(rInfoProvider, NumberType::xcoordinate) - maPosition.getX());
                    }
                }
            }
            else
            {
                // no absolute position, get from parent
                if(pParent)
                {
                    maPosition.setX(pParent->getPosition().getX());
                }

                const sal_uInt32 nSizeDx(rSvgTextPositions.getDx().size());

                if(nSizeDx)
                {
                    // relative positions given, translate position derived from parent
                    maPosition.setX(maPosition.getX() + rSvgTextPositions.getDx()[0].solve(rInfoProvider, NumberType::xcoordinate));

                    if(nSizeDx > 1)
                    {
                        // fill deltas to maX
                        maX.reserve(nSizeDx);

                        for(sal_uInt32 a(1); a < nSizeDx; a++)
                        {
                            maX.push_back(rSvgTextPositions.getDx()[a].solve(rInfoProvider, NumberType::xcoordinate));
                        }
                    }
                }
            }

            // get text positions Y
            const sal_uInt32 nSizeY(rSvgTextPositions.getY().size());

            if(nSizeY)
            {
                // we have absolute positions, get first one as current text position Y
                maPosition.setY(rSvgTextPositions.getY()[0].solve(rInfoProvider, NumberType::ycoordinate));
                mbAbsoluteX = true;

                if(nSizeY > 1)
                {
                    // fill deltas to maY
                    maY.reserve(nSizeY);

                    for(sal_uInt32 a(1); a < nSizeY; a++)
                    {
                        maY.push_back(rSvgTextPositions.getY()[a].solve(rInfoProvider, NumberType::ycoordinate) - maPosition.getY());
                    }
                }
            }
            else
            {
                // no absolute position, get from parent
                if(pParent)
                {
                    maPosition.setY(pParent->getPosition().getY());
                }

                const sal_uInt32 nSizeDy(rSvgTextPositions.getDy().size());

                if(nSizeDy)
                {
                    // relative positions given, translate position derived from parent
                    maPosition.setY(maPosition.getY() + rSvgTextPositions.getDy()[0].solve(rInfoProvider, NumberType::ycoordinate));

                    if(nSizeDy > 1)
                    {
                        // fill deltas to maY
                        maY.reserve(nSizeDy);

                        for(sal_uInt32 a(1); a < nSizeDy; a++)
                        {
                            maY.push_back(rSvgTextPositions.getDy()[a].solve(rInfoProvider, NumberType::ycoordinate));
                        }
                    }
                }
            }
        }

        bool SvgTextPosition::isRotated() const
        {
            if(maRotate.empty())
            {
                if(getParent())
                {
                    return getParent()->isRotated();
                }
                else
                {
                    return false;
                }
            }
            else
            {
                return true;
            }
        }

        double SvgTextPosition::consumeRotation()
        {
            double fRetval(0.0);

            if(maRotate.empty())
            {
                if(getParent())
                {
                    fRetval = mpParent->consumeRotation();
                }
                else
                {
                    fRetval = 0.0;
                }
            }
            else
            {
                const sal_uInt32 nSize(maRotate.size());

                if(mnRotationIndex < nSize)
                {
                    fRetval = maRotate[mnRotationIndex++];
                }
                else
                {
                    fRetval = maRotate[nSize - 1];
                }
            }

            return fRetval;
        }

} // end of namespace svgio

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
