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
#include <drawinglayer/primitive2d/groupprimitive2d.hxx>
#include <drawinglayer/primitive2d/textdecoratedprimitive2d.hxx>

namespace svgio
{
    namespace svgreader
    {
        SvgTextPositions::SvgTextPositions()
        :   maX(),
            maY(),
            maDx(),
            maDy(),
            maRotate(),
            maTextLength(),
            mbLengthAdjust(true)
        {
        }

        void SvgTextPositions::parseTextPositionAttributes(SVGToken aSVGToken, const OUString& aContent)
        {
            // parse own
            switch(aSVGToken)
            {
                case SVGTokenX:
                {
                    if(!aContent.isEmpty())
                    {
                        SvgNumberVector aVector;

                        if(readSvgNumberVector(aContent, aVector))
                        {
                            setX(aVector);
                        }
                    }
                    break;
                }
                case SVGTokenY:
                {
                    if(!aContent.isEmpty())
                    {
                        SvgNumberVector aVector;

                        if(readSvgNumberVector(aContent, aVector))
                        {
                            setY(aVector);
                        }
                    }
                    break;
                }
                case SVGTokenDx:
                {
                    if(!aContent.isEmpty())
                    {
                        SvgNumberVector aVector;

                        if(readSvgNumberVector(aContent, aVector))
                        {
                            setDx(aVector);
                        }
                    }
                    break;
                }
                case SVGTokenDy:
                {
                    if(!aContent.isEmpty())
                    {
                        SvgNumberVector aVector;

                        if(readSvgNumberVector(aContent, aVector))
                        {
                            setDy(aVector);
                        }
                    }
                    break;
                }
                case SVGTokenRotate:
                {
                    if(!aContent.isEmpty())
                    {
                        SvgNumberVector aVector;

                        if(readSvgNumberVector(aContent, aVector))
                        {
                            setRotate(aVector);
                        }
                    }
                    break;
                }
                case SVGTokenTextLength:
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
                case SVGTokenLengthAdjust:
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

    } // end of namespace svgreader
} // end of namespace svgio


namespace svgio
{
    namespace svgreader
    {
        class localTextBreakupHelper : public drawinglayer::primitive2d::TextBreakupHelper
        {
        private:
            SvgTextPosition&                    mrSvgTextPosition;

        protected:
            /// allow user callback to allow changes to the new TextTransformation. Default
            /// does nothing.
            virtual bool allowChange(sal_uInt32 nCount, basegfx::B2DHomMatrix& rNewTransform, sal_uInt32 nIndex, sal_uInt32 nLength) override;

        public:
            localTextBreakupHelper(
                const drawinglayer::primitive2d::TextSimplePortionPrimitive2D& rSource,
                SvgTextPosition& rSvgTextPosition)
            :   drawinglayer::primitive2d::TextBreakupHelper(rSource),
                mrSvgTextPosition(rSvgTextPosition)
            {
            }
        };

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

    } // end of namespace svgreader
} // end of namespace svgio


namespace svgio
{
    namespace svgreader
    {
        SvgCharacterNode::SvgCharacterNode(
            SvgDocument& rDocument,
            SvgNode* pParent,
            const OUString& rText)
        :   SvgNode(SVGTokenCharacter, rDocument, pParent),
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

        drawinglayer::primitive2d::TextSimplePortionPrimitive2D* SvgCharacterNode::createSimpleTextPrimitive(
            SvgTextPosition& rSvgTextPosition,
            const SvgStyleAttributes& rSvgStyleAttributes) const
        {
            // prepare retval, index and length
            drawinglayer::primitive2d::TextSimplePortionPrimitive2D* pRetval = nullptr;
            sal_uInt32 nIndex(0);
            sal_uInt32 nLength(getText().getLength());

            if(nLength)
            {
                // prepare FontAttribute
                const SvgStringVector& rFontFamilyVector = rSvgStyleAttributes.getFontFamily();
                OUString aFontFamily = rFontFamilyVector.empty() ?
                    OUString("Times New Roman") :
                    rFontFamilyVector[0];

                // #i122324# if the FontFamily name ends on ' embedded' it is probably a re-import
                // of a SVG export with font embedding. Remove this to make font matching work. This
                // is pretty safe since there should be no font family names ending on ' embedded'.
                // Remove again when FontEmbedding is implemented in SVG import
                if(aFontFamily.endsWith(" embedded"))
                {
                    aFontFamily = aFontFamily.copy(0, aFontFamily.getLength() - 9);
                }

                const ::FontWeight nFontWeight(getVclFontWeight(rSvgStyleAttributes.getFontWeight()));
                bool bItalic(FontStyle_italic == rSvgStyleAttributes.getFontStyle() || FontStyle_oblique == rSvgStyleAttributes.getFontStyle());

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
                drawinglayer::primitive2d::TextLayouterDevice aTextLayouterDevice;
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

                        for(size_t a(0); a < aExtendArray.size(); a++)
                        {
                            aTextArray.push_back(aExtendArray[a] + fStartX);
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

                        for(size_t a(0); a < aTextArray.size(); a++)
                        {
                            aTextArray[a] *= fFactor;
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
                if(TextAnchor_notset != rSvgStyleAttributes.getTextAnchor())
                {
                    switch(rSvgStyleAttributes.getTextAnchor())
                    {
                        case TextAnchor_start:
                        {
                            aTextAlign = TextAlign_left;
                            break;
                        }
                        case TextAnchor_middle:
                        {
                            aTextAlign = TextAlign_center;
                            break;
                        }
                        case TextAnchor_end:
                        {
                            aTextAlign = TextAlign_right;
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
                    case TextAlign_right:
                    {
                        aPosition.setX(aPosition.getX() - fTextWidth);
                        break;
                    }
                    case TextAlign_center:
                    {
                        aPosition.setX(aPosition.getX() - (fTextWidth * 0.5));
                        break;
                    }
                    case TextAlign_notset:
                    case TextAlign_left:
                    case TextAlign_justify:
                    {
                        // TextAlign_notset, TextAlign_left: nothing to do
                        // TextAlign_justify is not clear currently; handle as TextAlign_left
                        break;
                    }
                }

                // get BaselineShift
                const BaselineShift aBaselineShift(rSvgStyleAttributes.getBaselineShift());

                // apply BaselineShift
                switch(aBaselineShift)
                {
                    case BaselineShift_Sub:
                    {
                        aPosition.setY(aPosition.getY() + aTextLayouterDevice.getUnderlineOffset());
                        break;
                    }
                    case BaselineShift_Super:
                    {
                        aPosition.setY(aPosition.getY() + aTextLayouterDevice.getOverlineOffset());
                        break;
                    }
                    case BaselineShift_Percentage:
                    case BaselineShift_Length:
                    {
                        const SvgNumber aNumber(rSvgStyleAttributes.getBaselineShiftNumber());
                        const double mfBaselineShift(aNumber.solve(*this));

                        aPosition.setY(aPosition.getY() + mfBaselineShift);
                        break;
                    }
                    default: // BaselineShift_Baseline
                    {
                        // nothing to do
                        break;
                    }
                }

                // get fill color
                const basegfx::BColor aFill(rSvgStyleAttributes.getFill()
                    ? *rSvgStyleAttributes.getFill()
                    : basegfx::BColor(0.0, 0.0, 0.0));

                // prepare TextTransformation
                basegfx::B2DHomMatrix aTextTransform;

                aTextTransform.scale(fFontWidth, fFontHeight);
                aTextTransform.translate(aPosition.getX(), aPosition.getY());

                // check TextDecoration and if TextDecoratedPortionPrimitive2D is needed
                const TextDecoration aDeco(rSvgStyleAttributes.getTextDecoration());

                if(TextDecoration_underline == aDeco
                    || TextDecoration_overline == aDeco
                    || TextDecoration_line_through == aDeco)
                {
                    // get the fill for decoration as described by SVG. We cannot
                    // have different stroke colors/definitions for those, though
                    const SvgStyleAttributes* pDecoDef = rSvgStyleAttributes.getTextDecorationDefiningSvgStyleAttributes();
                    const basegfx::BColor aDecoColor(pDecoDef && pDecoDef->getFill() ? *pDecoDef->getFill() : aFill);

                    // create decorated text primitive
                    pRetval = new drawinglayer::primitive2d::TextDecoratedPortionPrimitive2D(
                        aTextTransform,
                        getText(),
                        nIndex,
                        nLength,
                        aTextArray,
                        aFontAttribute,
                        aLocale,
                        aFill,
                        COL_TRANSPARENT,

                        // extra props for decorated
                        aDecoColor,
                        aDecoColor,
                        TextDecoration_overline == aDeco ? drawinglayer::primitive2d::TEXT_LINE_SINGLE : drawinglayer::primitive2d::TEXT_LINE_NONE,
                        TextDecoration_underline == aDeco ? drawinglayer::primitive2d::TEXT_LINE_SINGLE : drawinglayer::primitive2d::TEXT_LINE_NONE,
                        false,
                        TextDecoration_line_through == aDeco ? drawinglayer::primitive2d::TEXT_STRIKEOUT_SINGLE : drawinglayer::primitive2d::TEXT_STRIKEOUT_NONE,
                        false,
                        drawinglayer::primitive2d::TEXT_FONT_EMPHASIS_MARK_NONE,
                        true,
                        false,
                        drawinglayer::primitive2d::TEXT_RELIEF_NONE,
                        false);
                }
                else
                {
                    // create text primitive
                    pRetval = new drawinglayer::primitive2d::TextSimplePortionPrimitive2D(
                        aTextTransform,
                        getText(),
                        nIndex,
                        nLength,
                        aTextArray,
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
            drawinglayer::primitive2d::Primitive2DContainer& rTarget,
            SvgTextPosition& rSvgTextPosition,
            const SvgStyleAttributes& rSvgStyleAttributes) const
        {
            const drawinglayer::primitive2d::Primitive2DReference xRef(
                createSimpleTextPrimitive(
                    rSvgTextPosition,
                    rSvgStyleAttributes));

            if(!(xRef.is() && (Visibility_visible == rSvgStyleAttributes.getVisibility())))
                return;

            if(!rSvgTextPosition.isRotated())
            {
                rTarget.push_back(xRef);
            }
            else
            {
                // need to apply rotations to each character as given
                const drawinglayer::primitive2d::TextSimplePortionPrimitive2D* pCandidate =
                    dynamic_cast< const drawinglayer::primitive2d::TextSimplePortionPrimitive2D* >(xRef.get());

                if(pCandidate)
                {
                    const localTextBreakupHelper alocalTextBreakupHelper(*pCandidate, rSvgTextPosition);
                    const drawinglayer::primitive2d::Primitive2DContainer& aResult(
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
            if(XmlSpace_default == getXmlSpace())
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

        void SvgCharacterNode::concatenate(const OUString& rText)
        {
            maText += rText;
        }

        void SvgCharacterNode::decomposeText(drawinglayer::primitive2d::Primitive2DContainer& rTarget, SvgTextPosition& rSvgTextPosition) const
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

    } // end of namespace svgreader
} // end of namespace svgio


namespace svgio
{
    namespace svgreader
    {
        SvgTextPosition::SvgTextPosition(
            SvgTextPosition* pParent,
            const InfoProvider& rInfoProvider,
            const SvgTextPositions& rSvgTextPositions)
        :   mpParent(pParent),
            maX(), // computed below
            maY(), // computed below
            maRotate(solveSvgNumberVector(rSvgTextPositions.getRotate(), rInfoProvider)),
            mfTextLength(0.0),
            maPosition(), // computed below
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
                maPosition.setX(rSvgTextPositions.getX()[0].solve(rInfoProvider, xcoordinate));
                mbAbsoluteX = true;

                if(nSizeX > 1)
                {
                    // fill deltas to maX
                    maX.reserve(nSizeX);

                    for(sal_uInt32 a(1); a < nSizeX; a++)
                    {
                        maX.push_back(rSvgTextPositions.getX()[a].solve(rInfoProvider, xcoordinate) - maPosition.getX());
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
                    maPosition.setX(maPosition.getX() + rSvgTextPositions.getDx()[0].solve(rInfoProvider, xcoordinate));

                    if(nSizeDx > 1)
                    {
                        // fill deltas to maX
                        maX.reserve(nSizeDx);

                        for(sal_uInt32 a(1); a < nSizeDx; a++)
                        {
                            maX.push_back(rSvgTextPositions.getDx()[a].solve(rInfoProvider, xcoordinate));
                        }
                    }
                }
            }

            // get text positions Y
            const sal_uInt32 nSizeY(rSvgTextPositions.getY().size());

            if(nSizeY)
            {
                // we have absolute positions, get first one as current text position Y
                maPosition.setY(rSvgTextPositions.getY()[0].solve(rInfoProvider, ycoordinate));
                mbAbsoluteX = true;

                if(nSizeY > 1)
                {
                    // fill deltas to maY
                    maY.reserve(nSizeY);

                    for(sal_uInt32 a(1); a < nSizeY; a++)
                    {
                        maY.push_back(rSvgTextPositions.getY()[a].solve(rInfoProvider, ycoordinate) - maPosition.getY());
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
                    maPosition.setY(maPosition.getY() + rSvgTextPositions.getDy()[0].solve(rInfoProvider, ycoordinate));

                    if(nSizeDy > 1)
                    {
                        // fill deltas to maY
                        maY.reserve(nSizeDy);

                        for(sal_uInt32 a(1); a < nSizeDy; a++)
                        {
                            maY.push_back(rSvgTextPositions.getDy()[a].solve(rInfoProvider, ycoordinate));
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

    } // end of namespace svgreader
} // end of namespace svgio

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
