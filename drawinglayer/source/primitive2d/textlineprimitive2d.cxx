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

#include <primitive2d/textlineprimitive2d.hxx>
#include <drawinglayer/primitive2d/drawinglayer_primitivetypes2d.hxx>
#include <drawinglayer/attribute/strokeattribute.hxx>
#include <drawinglayer/attribute/lineattribute.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <drawinglayer/primitive2d/transformprimitive2d.hxx>
#include <drawinglayer/primitive2d/PolygonWavePrimitive2D.hxx>


namespace drawinglayer::primitive2d
{
        void TextLinePrimitive2D::create2DDecomposition(Primitive2DContainer& rContainer, const geometry::ViewInformation2D& /*rViewInformation*/) const
        {
            if(TEXT_LINE_NONE == getTextLine())
                return;

            bool bDoubleLine(false);
            bool bWaveLine(false);
            bool bBoldLine(false);
            const int* pDotDashArray(nullptr);
            basegfx::B2DLineJoin eLineJoin(basegfx::B2DLineJoin::NONE);
            double fOffset(getOffset());
            double fHeight(getHeight());

            static const int aDottedArray[]     = { 1, 1, 0};               // DOTTED LINE
            static const int aDotDashArray[]    = { 1, 1, 4, 1, 0};         // DASHDOT
            static const int aDashDotDotArray[] = { 1, 1, 1, 1, 4, 1, 0};   // DASHDOTDOT
            static const int aDashedArray[]     = { 5, 2, 0};               // DASHED LINE
            static const int aLongDashArray[]   = { 7, 2, 0};               // LONGDASH

            // get decomposition
            basegfx::B2DVector aScale, aTranslate;
            double fRotate, fShearX;
            getObjectTransformation().decompose(aScale, aTranslate, fRotate, fShearX);

            switch(getTextLine())
            {
                default: // case TEXT_LINE_SINGLE:
                {
                    break;
                }
                case TEXT_LINE_DOUBLE:
                {
                    bDoubleLine = true;
                    break;
                }
                case TEXT_LINE_DOTTED:
                {
                    pDotDashArray = aDottedArray;
                    break;
                }
                case TEXT_LINE_DASH:
                {
                    pDotDashArray = aDashedArray;
                    break;
                }
                case TEXT_LINE_LONGDASH:
                {
                    pDotDashArray = aLongDashArray;
                    break;
                }
                case TEXT_LINE_DASHDOT:
                {
                    pDotDashArray = aDotDashArray;
                    break;
                }
                case TEXT_LINE_DASHDOTDOT:
                {
                    pDotDashArray = aDashDotDotArray;
                    break;
                }
                case TEXT_LINE_SMALLWAVE:
                {
                    bWaveLine = true;
                    break;
                }
                case TEXT_LINE_WAVE:
                {
                    bWaveLine = true;
                    break;
                }
                case TEXT_LINE_DOUBLEWAVE:
                {
                    bDoubleLine = true;
                    bWaveLine = true;
                    break;
                }
                case TEXT_LINE_BOLD:
                {
                    bBoldLine = true;
                    break;
                }
                case TEXT_LINE_BOLDDOTTED:
                {
                    bBoldLine = true;
                    pDotDashArray = aDottedArray;
                    break;
                }
                case TEXT_LINE_BOLDDASH:
                {
                    bBoldLine = true;
                    pDotDashArray = aDashedArray;
                    break;
                }
                case TEXT_LINE_BOLDLONGDASH:
                {
                    bBoldLine = true;
                    pDotDashArray = aLongDashArray;
                    break;
                }
                case TEXT_LINE_BOLDDASHDOT:
                {
                    bBoldLine = true;
                    pDotDashArray = aDotDashArray;
                    break;
                }
                case TEXT_LINE_BOLDDASHDOTDOT:
                {
                    bBoldLine = true;
                    pDotDashArray = aDashDotDotArray;
                    break;
                }
                case TEXT_LINE_BOLDWAVE:
                {
                    bWaveLine = true;
                    bBoldLine = true;
                    break;
                }
            }

            if(bBoldLine)
            {
                fHeight *= 2.0;
            }

            if(bDoubleLine)
            {
                fOffset -= 0.50 * fHeight;
                fHeight *= 0.64;
            }

            if(bWaveLine)
            {
                eLineJoin = basegfx::B2DLineJoin::Round;
                fHeight *= 0.25;
            }

            // prepare Line and Stroke Attributes
            const attribute::LineAttribute aLineAttribute(getLineColor(), fHeight, eLineJoin);
            attribute::StrokeAttribute aStrokeAttribute;

            if(pDotDashArray)
            {
                std::vector< double > aDoubleArray;

                for(const int* p = pDotDashArray; *p; ++p)
                {
                    aDoubleArray.push_back(static_cast<double>(*p) * fHeight);
                }

                aStrokeAttribute = attribute::StrokeAttribute(std::move(aDoubleArray));
            }

            // create base polygon and new primitive
            basegfx::B2DPolygon aLine;
            Primitive2DReference aNewPrimitive;

            aLine.append(basegfx::B2DPoint(0.0, fOffset));
            aLine.append(basegfx::B2DPoint(getWidth(), fOffset));

            const basegfx::B2DHomMatrix aUnscaledTransform(
                basegfx::utils::createShearXRotateTranslateB2DHomMatrix(
                    fShearX, fRotate, aTranslate));

            aLine.transform(aUnscaledTransform);

            if(bWaveLine)
            {
                double fWaveWidth(10.6 * fHeight);

                if(TEXT_LINE_SMALLWAVE == getTextLine())
                {
                    fWaveWidth *= 0.7;
                }
                else if(TEXT_LINE_WAVE == getTextLine())
                {
                    // extra multiply to get the same WaveWidth as with the bold version
                    fWaveWidth *= 2.0;
                }

                aNewPrimitive = Primitive2DReference(new PolygonWavePrimitive2D(aLine, aLineAttribute, aStrokeAttribute, fWaveWidth, fWaveWidth * 0.5));
            }
            else
            {
                aNewPrimitive = Primitive2DReference(new PolygonStrokePrimitive2D(aLine, aLineAttribute, aStrokeAttribute));
            }

            // add primitive
            rContainer.push_back(aNewPrimitive);

            if(!bDoubleLine)
                return;

            // double line, create 2nd primitive with offset using TransformPrimitive based on
            // already created NewPrimitive
            double fLineDist(2.3 * fHeight);

            if(bWaveLine)
            {
                fLineDist = 6.3 * fHeight;
            }

            // move base point of text to 0.0 and de-rotate
            basegfx::B2DHomMatrix aTransform(basegfx::utils::createTranslateB2DHomMatrix(
                -aTranslate.getX(), -aTranslate.getY()));
            aTransform.rotate(-fRotate);

            // translate in Y by offset
            aTransform.translate(0.0, fLineDist);

            // move back and rotate
            aTransform.rotate(fRotate);
            aTransform.translate(aTranslate.getX(), aTranslate.getY());

            // add transform primitive
            Primitive2DContainer aContent { aNewPrimitive };
            rContainer.push_back( new TransformPrimitive2D(aTransform, std::move(aContent)) );
        }

        TextLinePrimitive2D::TextLinePrimitive2D(
            const basegfx::B2DHomMatrix& rObjectTransformation,
            double fWidth,
            double fOffset,
            double fHeight,
            TextLine eTextLine,
            const basegfx::BColor& rLineColor)
        :   maObjectTransformation(rObjectTransformation),
            mfWidth(fWidth),
            mfOffset(fOffset),
            mfHeight(fHeight),
            meTextLine(eTextLine),
            maLineColor(rLineColor)
        {
        }

        bool TextLinePrimitive2D::operator==( const BasePrimitive2D& rPrimitive ) const
        {
            if(BufferedDecompositionPrimitive2D::operator==(rPrimitive))
            {
                const TextLinePrimitive2D& rCompare = static_cast<const TextLinePrimitive2D&>(rPrimitive);

                return (getObjectTransformation() == rCompare.getObjectTransformation()
                    && getWidth() == rCompare.getWidth()
                    && getOffset() == rCompare.getOffset()
                    && getHeight() == rCompare.getHeight()
                    && getTextLine() == rCompare.getTextLine()
                    && getLineColor() == rCompare.getLineColor());
            }

            return false;
        }

        // provide unique ID
        sal_uInt32 TextLinePrimitive2D::getPrimitive2DID() const
        {
            return PRIMITIVE2D_ID_TEXTLINEPRIMITIVE2D;
        }

} // end of namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
