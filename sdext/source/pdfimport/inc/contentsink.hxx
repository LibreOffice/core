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

#ifndef INCLUDED_SDEXT_SOURCE_PDFIMPORT_INC_CONTENTSINK_HXX
#define INCLUDED_SDEXT_SOURCE_PDFIMPORT_INC_CONTENTSINK_HXX

#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/rendering/ARGBColor.hpp>
#include <memory>

namespace com::sun::star {
    namespace rendering
    {
        class  XPolyPolygon2D;
    }
    namespace geometry
    {
        struct Matrix2D;
        struct AffineMatrix2D;
        struct RealRectangle2D;
        struct RealPoint2D;
        struct RealSize2D;
    }
    namespace beans
    {
        struct PropertyValue;
    }
}

namespace pdfi
{
    struct FontAttributes
    {
        FontAttributes( OUString             familyName_,
                        OUString             sFontWeight,
                        bool                 isItalic_,
                        bool                 isUnderline_,
                        double               size_,
                        double               ascent_) :
            familyName(std::move(familyName_)),
            fontWeight(std::move(sFontWeight)),
            isItalic(isItalic_),
            isUnderline(isUnderline_),
            isOutline(false),
            size(size_),
            ascent(ascent_)
        {}

        FontAttributes() :
            familyName(),
            fontWeight(u"normal"_ustr),
            isItalic(false),
            isUnderline(false),
            isOutline(false),
            size(0.0),
            ascent(1.0)
        {}

        OUString            familyName;
        OUString            fontWeight;
        bool                isItalic;
        bool                isUnderline;
        bool                isOutline;
        double              size; // device pixel
        double              ascent;

        bool operator==(const FontAttributes& rFont) const
        {
            return familyName == rFont.familyName &&
                fontWeight == rFont.fontWeight &&
                !isItalic == !rFont.isItalic &&
                !isUnderline == !rFont.isUnderline &&
                !isOutline == !rFont.isOutline &&
                size == rFont.size &&
                ascent == rFont.ascent;
        }
    };

    /** (preliminary) API wrapper around xpdf

        Wraps the functionality currently used from xpdf's OutputDev
        interface. Subject to change.
     */
    struct ContentSink
    {
        virtual ~ContentSink() {}

        /// Total number of pages for upcoming document
        virtual void setPageNum( sal_Int32 nNumPages ) = 0;
        virtual void startPage( const css::geometry::RealSize2D& rSize ) = 0;
        virtual void endPage() = 0;

        virtual void hyperLink( const css::geometry::RealRectangle2D& rBounds,
                                const OUString&                             rURI ) = 0;

        virtual void pushState() = 0;
        virtual void popState() = 0;

        virtual void setFlatness( double ) = 0;
        virtual void setTransformation( const css::geometry::AffineMatrix2D& rMatrix ) = 0;
        virtual void setLineDash( const css::uno::Sequence<double>& dashes,
                                  double                                         start ) = 0;
        virtual void setLineJoin( sal_Int8 lineJoin ) = 0;
        virtual void setLineCap( sal_Int8 lineCap ) = 0;
        virtual void setMiterLimit(double) = 0;
        virtual void setLineWidth(double) = 0;
        virtual void setFillColor( const css::rendering::ARGBColor& rColor ) = 0;
        virtual void setStrokeColor( const css::rendering::ARGBColor& rColor ) = 0;
        virtual void setFont( const FontAttributes& rFont ) = 0;
        virtual void setTextRenderMode( sal_Int32 ) = 0;


        virtual void strokePath( const css::uno::Reference<
                                       css::rendering::XPolyPolygon2D >& rPath ) = 0;
        virtual void fillPath( const css::uno::Reference<
                                     css::rendering::XPolyPolygon2D >& rPath ) = 0;
        virtual void eoFillPath( const css::uno::Reference<
                                       css::rendering::XPolyPolygon2D >& rPath ) = 0;

        virtual void intersectClip(const css::uno::Reference<
                                         css::rendering::XPolyPolygon2D >& rPath) = 0;
        virtual void intersectEoClip(const css::uno::Reference<
                                           css::rendering::XPolyPolygon2D >& rPath) = 0;

        virtual void drawGlyphs( const OUString& rGlyphs,
                                 const css::geometry::RealRectangle2D& rRect,
                                 const css::geometry::Matrix2D&        rFontMatrix,
                                 double fontSize) = 0;

        /// issued when a sequence of associated glyphs is drawn
        virtual void endText() = 0;

        /// draws given bitmap as a mask (using current fill color)
        virtual void drawMask(const css::uno::Sequence<
                                    css::beans::PropertyValue>& xBitmap,
                              bool                                           bInvert ) = 0;
        /// Given image must already be color-mapped and normalized to sRGB.
        virtual void drawImage(const css::uno::Sequence<
                                     css::beans::PropertyValue>& xBitmap ) = 0;
        /** Given image must already be color-mapped and normalized to sRGB.

            maskColors must contain two sequences of color components
         */
        virtual void drawColorMaskedImage(const css::uno::Sequence<
                                                css::beans::PropertyValue>& xBitmap,
                                          const css::uno::Sequence<
                                                css::uno::Any>&             xMaskColors ) = 0;
        virtual void drawMaskedImage(const css::uno::Sequence<
                                           css::beans::PropertyValue>& xBitmap,
                                     const css::uno::Sequence<
                                           css::beans::PropertyValue>& xMask,
                                     bool                                             bInvertMask) = 0;
        virtual void drawAlphaMaskedImage(const css::uno::Sequence<
                                                css::beans::PropertyValue>& xImage,
                                          const css::uno::Sequence<
                                                css::beans::PropertyValue>& xMask) = 0;
        virtual void tilingPatternFill(int nX0, int nY0, int nX1, int nY1,
                                       double nxStep, double nyStep,
                                       int nPaintType,
                                       css::geometry::AffineMatrix2D& rMat,
                                       const css::uno::Sequence<css::beans::PropertyValue>& xTile) = 0;
    };

    typedef std::shared_ptr<ContentSink> ContentSinkSharedPtr;
}

#endif // INCLUDED_SDEXT_SOURCE_PDFIMPORT_INC_CONTENTSINK_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
