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

#include <drawinglayer/primitive2d/textstrikeoutprimitive2d.hxx>
#include <drawinglayer/primitive2d/textlayoutdevice.hxx>
#include <drawinglayer/primitive2d/textprimitive2d.hxx>
#include <drawinglayer/primitive2d/drawinglayer_primitivetypes2d.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <drawinglayer/attribute/lineattribute.hxx>
#include <drawinglayer/primitive2d/polygonprimitive2d.hxx>
#include <drawinglayer/primitive2d/transformprimitive2d.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive2d
    {
        BaseTextStrikeoutPrimitive2D::BaseTextStrikeoutPrimitive2D(
            const basegfx::B2DHomMatrix& rObjectTransformation,
            double fWidth,
            const basegfx::BColor& rFontColor)
        :   BufferedDecompositionPrimitive2D(),
            maObjectTransformation(rObjectTransformation),
            mfWidth(fWidth),
            maFontColor(rFontColor)
        {
        }

        bool BaseTextStrikeoutPrimitive2D::operator==( const BasePrimitive2D& rPrimitive ) const
        {
            if(BufferedDecompositionPrimitive2D::operator==(rPrimitive))
            {
                const BaseTextStrikeoutPrimitive2D& rCompare = (BaseTextStrikeoutPrimitive2D&)rPrimitive;

                return (getObjectTransformation() == rCompare.getObjectTransformation()
                    && getWidth() == rCompare.getWidth()
                    && getFontColor() == rCompare.getFontColor());
            }

            return false;
        }
    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive2d
    {
        Primitive2DSequence TextCharacterStrikeoutPrimitive2D::create2DDecomposition(const geometry::ViewInformation2D& /*rViewInformation*/) const
        {
            // strikeout with character
            const OUString aSingleCharString(getStrikeoutChar());
            basegfx::B2DVector aScale, aTranslate;
            double fRotate, fShearX;

            // get decomposition
            getObjectTransformation().decompose(aScale, aTranslate, fRotate, fShearX);

            // prepare TextLayouter
            TextLayouterDevice aTextLayouter;

            aTextLayouter.setFontAttribute(
                getFontAttribute(),
                aScale.getX(),
                aScale.getY(),
                getLocale());

            const double fStrikeCharWidth(aTextLayouter.getTextWidth(aSingleCharString, 0, 1));
            const double fStrikeCharCount(fabs(getWidth()/fStrikeCharWidth));
            const sal_uInt32 nStrikeCharCount(static_cast< sal_uInt32 >(fStrikeCharCount + 0.5));
            std::vector<double> aDXArray(nStrikeCharCount);
            OUString aStrikeoutString;

            for(sal_uInt32 a(0); a < nStrikeCharCount; a++)
            {
                aStrikeoutString += aSingleCharString;
                aDXArray[a] = (a + 1) * fStrikeCharWidth;
            }

            Primitive2DReference xReference(
                new TextSimplePortionPrimitive2D(
                    getObjectTransformation(),
                    aStrikeoutString,
                    0,
                    aStrikeoutString.getLength(),
                    aDXArray,
                    getFontAttribute(),
                    getLocale(),
                    getFontColor()));

            return Primitive2DSequence(&xReference, 1);
        }

        TextCharacterStrikeoutPrimitive2D::TextCharacterStrikeoutPrimitive2D(
            const basegfx::B2DHomMatrix& rObjectTransformation,
            double fWidth,
            const basegfx::BColor& rFontColor,
            sal_Unicode aStrikeoutChar,
            const attribute::FontAttribute& rFontAttribute,
            const ::com::sun::star::lang::Locale& rLocale)
        :   BaseTextStrikeoutPrimitive2D(rObjectTransformation, fWidth, rFontColor),
            maStrikeoutChar(aStrikeoutChar),
            maFontAttribute(rFontAttribute),
            maLocale(rLocale)
        {
        }

        bool TextCharacterStrikeoutPrimitive2D::operator==( const BasePrimitive2D& rPrimitive ) const
        {
            if(BaseTextStrikeoutPrimitive2D::operator==(rPrimitive))
            {
                const TextCharacterStrikeoutPrimitive2D& rCompare = (TextCharacterStrikeoutPrimitive2D&)rPrimitive;

                return (getStrikeoutChar() == rCompare.getStrikeoutChar()
                    && getFontAttribute() == rCompare.getFontAttribute()
                    && LocalesAreEqual(getLocale(), rCompare.getLocale()));
            }

            return false;
        }

        // provide unique ID
        ImplPrimitive2DIDBlock(TextCharacterStrikeoutPrimitive2D, PRIMITIVE2D_ID_TEXTCHARACTERSTRIKEOUTPRIMITIVE2D)

    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive2d
    {
        Primitive2DSequence TextGeometryStrikeoutPrimitive2D::create2DDecomposition(const geometry::ViewInformation2D& /*rViewInformation*/) const
        {
            OSL_ENSURE(TEXT_STRIKEOUT_SLASH != getTextStrikeout() && TEXT_STRIKEOUT_X != getTextStrikeout(),
                "Wrong TEXT_STRIKEOUT type; a TextCharacterStrikeoutPrimitive2D should be used (!)");

            // strikeout with geometry
            double fStrikeoutHeight(getHeight());
            double fStrikeoutOffset(getOffset());
            bool bDoubleLine(false);

            // get decomposition
            basegfx::B2DVector aScale, aTranslate;
            double fRotate, fShearX;
            getObjectTransformation().decompose(aScale, aTranslate, fRotate, fShearX);

            // set line attribute
            switch(getTextStrikeout())
            {
                default : // case primitive2d::TEXT_STRIKEOUT_SINGLE:
                {
                    break;
                }
                case primitive2d::TEXT_STRIKEOUT_DOUBLE:
                {
                    bDoubleLine = true;
                    break;
                }
                case primitive2d::TEXT_STRIKEOUT_BOLD:
                {
                    fStrikeoutHeight *= 2.0;
                    break;
                }
            }

            if(bDoubleLine)
            {
                fStrikeoutOffset -= 0.50 * fStrikeoutHeight;
                fStrikeoutHeight *= 0.64;
            }

            // create base polygon and new primitive
            basegfx::B2DPolygon aStrikeoutLine;

            aStrikeoutLine.append(basegfx::B2DPoint(0.0, -fStrikeoutOffset));
            aStrikeoutLine.append(basegfx::B2DPoint(getWidth(), -fStrikeoutOffset));

            const basegfx::B2DHomMatrix aUnscaledTransform(
                basegfx::tools::createShearXRotateTranslateB2DHomMatrix(
                    fShearX, fRotate, aTranslate));

            aStrikeoutLine.transform(aUnscaledTransform);

            // add primitive
            const attribute::LineAttribute aLineAttribute(getFontColor(), fStrikeoutHeight, basegfx::B2DLINEJOIN_NONE);
            Primitive2DSequence xRetval(1);
            xRetval[0] = Primitive2DReference(new PolygonStrokePrimitive2D(aStrikeoutLine, aLineAttribute));

            if(bDoubleLine)
            {
                // double line, create 2nd primitive with offset using TransformPrimitive based on
                // already created NewPrimitive
                const double fLineDist(2.0 * fStrikeoutHeight);

                // move base point of text to 0.0 and de-rotate
                basegfx::B2DHomMatrix aTransform(basegfx::tools::createTranslateB2DHomMatrix(
                    -aTranslate.getX(), -aTranslate.getY()));
                aTransform.rotate(-fRotate);

                // translate in Y by offset
                aTransform.translate(0.0, -fLineDist);

                // move back and rotate
                aTransform.rotate(fRotate);
                aTransform.translate(aTranslate.getX(), aTranslate.getY());

                // add transform primitive
                appendPrimitive2DReferenceToPrimitive2DSequence(xRetval,
                    Primitive2DReference(
                        new TransformPrimitive2D(
                            aTransform,
                            xRetval)));
            }

            return xRetval;
        }

        TextGeometryStrikeoutPrimitive2D::TextGeometryStrikeoutPrimitive2D(
            const basegfx::B2DHomMatrix& rObjectTransformation,
            double fWidth,
            const basegfx::BColor& rFontColor,
            double fHeight,
            double fOffset,
            TextStrikeout eTextStrikeout)
        :   BaseTextStrikeoutPrimitive2D(rObjectTransformation, fWidth, rFontColor),
            mfHeight(fHeight),
            mfOffset(fOffset),
            meTextStrikeout(eTextStrikeout)
        {
        }

        bool TextGeometryStrikeoutPrimitive2D::operator==( const BasePrimitive2D& rPrimitive ) const
        {
            if(BaseTextStrikeoutPrimitive2D::operator==(rPrimitive))
            {
                const TextGeometryStrikeoutPrimitive2D& rCompare = (TextGeometryStrikeoutPrimitive2D&)rPrimitive;

                return (getHeight() == rCompare.getHeight()
                    && getOffset() == rCompare.getOffset()
                    && getTextStrikeout() == rCompare.getTextStrikeout());
            }

            return false;
        }

        // provide unique ID
        ImplPrimitive2DIDBlock(TextGeometryStrikeoutPrimitive2D, PRIMITIVE2D_ID_TEXTGEOMETRYSTRIKEOUTPRIMITIVE2D)

    } // end of namespace primitive2d
} // end of namespace drawinglayer

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
