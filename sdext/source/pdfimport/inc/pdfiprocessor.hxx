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

#ifndef INCLUDED_SDEXT_SOURCE_PDFIMPORT_INC_PDFIPROCESSOR_HXX
#define INCLUDED_SDEXT_SOURCE_PDFIMPORT_INC_PDFIPROCESSOR_HXX

#include <com/sun/star/util/XStringMapping.hpp>
#include <com/sun/star/xml/sax/XDocumentHandler.hpp>
#include <com/sun/star/task/XStatusIndicator.hpp>
#include <com/sun/star/rendering/XVolatileBitmap.hpp>
#include <com/sun/star/geometry/RealSize2D.hpp>
#include <com/sun/star/geometry/RealPoint2D.hpp>
#include <com/sun/star/geometry/RealRectangle2D.hpp>
#include <com/sun/star/geometry/Matrix2D.hpp>

#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/range/b2drange.hxx>

#include <rtl/ustring.hxx>
#include <rtl/ustrbuf.hxx>

#include <list>
#include <memory>
#include <unordered_map>

#include "imagecontainer.hxx"
#include "contentsink.hxx"
#include "treevisitorfactory.hxx"
#include "genericelements.hxx"

#include <boost/bimap/bimap.hpp>
#include <boost/bimap/unordered_set_of.hpp>

namespace pdfi
{

    class  PDFIProcessor;
    struct Element;
    struct DocumentElement;
    struct PageElement;
    class  ElementFactory;
    class  XmlEmitter;
    class  CharGlyph;

    /** Main entry from the parser

        Creates the internal DOM tree from the render calls
     */
    class PDFIProcessor : public ContentSink
    {
    public:
        css::uno::Reference<
            css::uno::XComponentContext >  m_xContext;
        basegfx::B2DHomMatrix prevTextMatrix;
        double prevCharWidth;

        explicit PDFIProcessor( const css::uno::Reference< css::task::XStatusIndicator >& xStat,
            css::uno::Reference< css::uno::XComponentContext > const & xContext) ;

        void emit( XmlEmitter&               rEmitter,
                   const TreeVisitorFactory& rVisitorFactory );

        sal_Int32 getGCId( const GraphicsContext& rGC );
        const GraphicsContext& getGraphicsContext( sal_Int32 nGCId ) const;
        GraphicsContext& getCurrentContext() { return m_aGCStack.back(); }
        const GraphicsContext& getCurrentContext() const { return m_aGCStack.back(); }

        const css::uno::Reference< css::task::XStatusIndicator >& getStatusIndicator() const
        { return m_xStatusIndicator; }

        const FontAttributes& getFont( sal_Int32 nFontId ) const;
        sal_Int32 getFontId( const FontAttributes& rAttr ) const;

        static void sortElements( Element* pElement );

        static OUString mirrorString( const OUString& i_rInString );

    private:
        void processGlyphLine();

        // ContentSink interface implementation

        virtual void setPageNum( sal_Int32 nNumPages ) override;
        virtual void startPage( const css::geometry::RealSize2D& rSize ) override;
        virtual void endPage() override;

        virtual void hyperLink( const css::geometry::RealRectangle2D& rBounds,
                                const OUString&                             rURI ) override;
        virtual void pushState() override;
        virtual void popState() override;
        virtual void setFlatness( double ) override;
        virtual void setTransformation( const css::geometry::AffineMatrix2D& rMatrix ) override;
        virtual void setLineDash( const css::uno::Sequence<double>& dashes,
                                  double                                         start ) override;
        virtual void setLineJoin(sal_Int8) override;
        virtual void setLineCap(sal_Int8) override;
        virtual void setMiterLimit(double) override;
        virtual void setLineWidth(double) override;
        virtual void setFillColor( const css::rendering::ARGBColor& rColor ) override;
        virtual void setStrokeColor( const css::rendering::ARGBColor& rColor ) override;
        virtual void setFont( const FontAttributes& rFont ) override;
        virtual void setTextRenderMode( sal_Int32 ) override;

        virtual void strokePath( const css::uno::Reference<
                                       css::rendering::XPolyPolygon2D >& rPath ) override;
        virtual void fillPath( const css::uno::Reference<
                                     css::rendering::XPolyPolygon2D >& rPath ) override;
        virtual void eoFillPath( const css::uno::Reference<
                                       css::rendering::XPolyPolygon2D >& rPath ) override;

        virtual void intersectClip(const css::uno::Reference<
                                         css::rendering::XPolyPolygon2D >& rPath) override;
        virtual void intersectEoClip(const css::uno::Reference<
                                           css::rendering::XPolyPolygon2D >& rPath) override;

        virtual void drawGlyphs( const OUString&                               rGlyphs,
                                 const css::geometry::RealRectangle2D& rRect,
                                 const css::geometry::Matrix2D&        rFontMatrix,
                                 double fontSize) override;
        virtual void endText() override;

        virtual void drawMask(const css::uno::Sequence<
                                    css::beans::PropertyValue>& xBitmap,
                              bool                                           bInvert ) override;
        /// Given image must already be color-mapped and normalized to sRGB.
        virtual void drawImage(const css::uno::Sequence<
                                     css::beans::PropertyValue>& xBitmap ) override;
        /** Given image must already be color-mapped and normalized to sRGB.

            maskColors must contain two sequences of color components
         */
        virtual void drawColorMaskedImage(const css::uno::Sequence<
                                                css::beans::PropertyValue>& xBitmap,
                                          const css::uno::Sequence<
                                                css::uno::Any>&             xMaskColors ) override;
        virtual void drawMaskedImage(const css::uno::Sequence<
                                           css::beans::PropertyValue>& xBitmap,
                                     const css::uno::Sequence<
                                           css::beans::PropertyValue>& xMask,
                                     bool                                             bInvertMask) override;
        virtual void drawAlphaMaskedImage(const css::uno::Sequence<
                                                css::beans::PropertyValue>& xImage,
                                          const css::uno::Sequence<
                                                css::beans::PropertyValue>& xMask) override;

        void startIndicator( const OUString& rText );
        void endIndicator();

        void setupImage(ImageId nImage);

        typedef std::unordered_map<sal_Int32,FontAttributes> IdToFontMap;
        typedef std::unordered_map<FontAttributes,sal_Int32,FontAttrHash> FontToIdMap;

        typedef boost::bimaps::bimap<
                             boost::bimaps::unordered_set_of<GraphicsContext, GraphicsContextHash>,
                             boost::bimaps::unordered_set_of<sal_Int32>
                            > GCToIdBiMap;

        typedef std::vector<GraphicsContext> GraphicsContextStack;

        std::vector<CharGlyph>             m_GlyphsList;

        std::shared_ptr<DocumentElement> m_pDocument;
        PageElement*                       m_pCurPage;
        Element*                           m_pCurElement;
        sal_Int32                          m_nNextFontId;
        IdToFontMap                        m_aIdToFont;
        FontToIdMap                        m_aFontToId;

        GraphicsContextStack               m_aGCStack;
        sal_Int32                          m_nNextGCId;
        GCToIdBiMap                        m_aGCToId;

        ImageContainer                     m_aImages;

        sal_Int32                          m_nPages;
        sal_Int32                          m_nNextZOrder;
        css::uno::Reference< css::task::XStatusIndicator >
                                           m_xStatusIndicator;
    };
    class CharGlyph final
    {
        public:
            CharGlyph(Element* pCurElement, const GraphicsContext& rCurrentContext,
                double width, double prevSpaceWidth, const OUString& rGlyphs  )
               : m_pCurElement(pCurElement), m_rCurrentContext(rCurrentContext),
                 m_Width(width), m_PrevSpaceWidth(prevSpaceWidth), m_rGlyphs(rGlyphs) {};

            OUString& getGlyph(){ return m_rGlyphs; }
            double getWidth(){ return m_Width; }
            double getPrevSpaceWidth(){ return m_PrevSpaceWidth; }
            GraphicsContext&  getGC(){ return m_rCurrentContext; }
            Element*  getCurElement(){ return m_pCurElement; }

        private:
            Element* const              m_pCurElement ;
            GraphicsContext             m_rCurrentContext ;
            double const                m_Width ;
            double const                m_PrevSpaceWidth ;
            OUString                    m_rGlyphs ;
    };
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
