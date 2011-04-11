/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_drawinglayer.hxx"

#include <drawinglayer/primitive2d/textlineprimitive2d.hxx>
#include <drawinglayer/primitive2d/drawinglayer_primitivetypes2d.hxx>
#include <drawinglayer/attribute/strokeattribute.hxx>
#include <drawinglayer/attribute/lineattribute.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <drawinglayer/primitive2d/polygonprimitive2d.hxx>
#include <drawinglayer/primitive2d/transformprimitive2d.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive2d
    {
        Primitive2DSequence TextLinePrimitive2D::create2DDecomposition(const geometry::ViewInformation2D& /*rViewInformation*/) const
        {
            Primitive2DSequence xRetval;

            if(TEXT_LINE_NONE != getTextLine())
            {
                bool bDoubleLine(false);
                bool bWaveLine(false);
                bool bBoldLine(false);
                const int* pDotDashArray(0);
                basegfx::B2DLineJoin eLineJoin(basegfx::B2DLINEJOIN_NONE);
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
                    eLineJoin = basegfx::B2DLINEJOIN_ROUND;
                    fHeight *= 0.25;
                }

                // prepare Line and Stroke Attributes
                const attribute::LineAttribute aLineAttribute(getLineColor(), fHeight, eLineJoin);
                attribute::StrokeAttribute aStrokeAttribute;

                if(pDotDashArray)
                {
                    ::std::vector< double > aDoubleArray;

                    for(const int* p = pDotDashArray; *p; ++p)
                    {
                        aDoubleArray.push_back((double)(*p) * fHeight);
                    }

                    aStrokeAttribute = attribute::StrokeAttribute(aDoubleArray);
                }

                // create base polygon and new primitive
                basegfx::B2DPolygon aLine;
                Primitive2DReference aNewPrimitive;

                aLine.append(basegfx::B2DPoint(0.0, fOffset));
                aLine.append(basegfx::B2DPoint(getWidth(), fOffset));

                const basegfx::B2DHomMatrix aUnscaledTransform(
                    basegfx::tools::createShearXRotateTranslateB2DHomMatrix(
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
                appendPrimitive2DReferenceToPrimitive2DSequence(xRetval, aNewPrimitive);

                if(bDoubleLine)
                {
                    // double line, create 2nd primitive with offset using TransformPrimitive based on
                    // already created NewPrimitive
                    double fLineDist(2.3 * fHeight);

                    if(bWaveLine)
                    {
                        fLineDist = 6.3 * fHeight;
                    }

                    // move base point of text to 0.0 and de-rotate
                    basegfx::B2DHomMatrix aTransform(basegfx::tools::createTranslateB2DHomMatrix(
                        -aTranslate.getX(), -aTranslate.getY()));
                    aTransform.rotate(-fRotate);

                    // translate in Y by offset
                    aTransform.translate(0.0, fLineDist);

                    // move back and rotate
                    aTransform.rotate(fRotate);
                    aTransform.translate(aTranslate.getX(), aTranslate.getY());

                    // add transform primitive
                    const Primitive2DSequence aContent(&aNewPrimitive, 1);
                    appendPrimitive2DReferenceToPrimitive2DSequence(xRetval,
                        Primitive2DReference(new TransformPrimitive2D(aTransform, aContent)));
                }
            }

            return xRetval;
        }

        TextLinePrimitive2D::TextLinePrimitive2D(
            const basegfx::B2DHomMatrix& rObjectTransformation,
            double fWidth,
            double fOffset,
            double fHeight,
            TextLine eTextLine,
            const basegfx::BColor& rLineColor)
        :   BufferedDecompositionPrimitive2D(),
            maObjectTransformation(rObjectTransformation),
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
                const TextLinePrimitive2D& rCompare = (TextLinePrimitive2D&)rPrimitive;

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
        ImplPrimitrive2DIDBlock(TextLinePrimitive2D, PRIMITIVE2D_ID_TEXTLINEPRIMITIVE2D)

    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////
// eof

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
