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

#ifndef INCLUDED_SDEXT_SOURCE_PDFIMPORT_TREE_GENERICELEMENTS_HXX
#define INCLUDED_SDEXT_SOURCE_PDFIMPORT_TREE_GENERICELEMENTS_HXX

#include "pdfihelper.hxx"
#include "treevisiting.hxx"

#include <com/sun/star/task/XStatusIndicator.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <basegfx/range/b2drange.hxx>
#include <rtl/ustring.hxx>
#include <rtl/ustrbuf.hxx>

#include <list>

namespace pdfi
{
    class XmlEmitter;
    class StyleContainer;
    class ImageContainer;
    class PDFIProcessor;
    class ElementFactory;


    struct EmitContext
    {
        EmitContext(
            XmlEmitter&                              _rEmitter,
            StyleContainer&                          _rStyles,
            ImageContainer&                          _rImages,
            PDFIProcessor&                           _rProcessor,
            const css::uno::Reference<
            css::task::XStatusIndicator>& _xStatusIndicator,
            css::uno::Reference< css::uno::XComponentContext >  xContext)
        :
            rEmitter(_rEmitter),
            rStyles(_rStyles),
            rImages(_rImages),
            rProcessor(_rProcessor),
            xStatusIndicator(_xStatusIndicator),
        m_xContext(xContext)
        {}

        XmlEmitter&     rEmitter;
        StyleContainer& rStyles;
        ImageContainer& rImages;
        PDFIProcessor&  rProcessor;
        css::uno::Reference<
            css::task::XStatusIndicator> xStatusIndicator;
        css::uno::Reference<
            css::uno::XComponentContext >  m_xContext;
    };

    struct Element
    {
    protected:
        explicit Element( Element* pParent )
            : x( 0 ), y( 0 ), w( 0 ), h( 0 ), StyleId( -1 ), Parent( pParent )
        {
            if( pParent )
                pParent->Children.push_back( this );
        }

    public:
        virtual ~Element();

        /**
            To be implemented by every tree node that needs to be
            visitable.
         */
        virtual void visitedBy( ElementTreeVisitor&, const std::list< Element* >::const_iterator& rParentIt ) = 0;
        /// Apply visitor to all children
        void applyToChildren( ElementTreeVisitor& );
        /// Union element geometry with given element
        void updateGeometryWith( const Element* pMergeFrom );

#if OSL_DEBUG_LEVEL > 1
        // xxx refac TODO: move code to visitor
        virtual void emitStructure( int nLevel );
#endif
        /** el must be a valid dereferenceable iterator of el->Parent->Children
            pNewParent must not be NULL
        */
        static void setParent( std::list<Element*>::iterator& el, Element* pNewParent );

        double              x, y, w, h;
        sal_Int32           StyleId;
        Element*            Parent;
        std::list<Element*> Children;
    };

    struct ListElement : public Element
    {
        ListElement() : Element( nullptr ) {}
        virtual void visitedBy( ElementTreeVisitor&, const std::list< Element* >::const_iterator& ) override;
    };

    struct HyperlinkElement : public Element
    {
        friend class ElementFactory;
    protected:
        HyperlinkElement( Element* pParent, const OUString& rURI )
        : Element( pParent ), URI( rURI ) {}
    public:
        virtual void visitedBy( ElementTreeVisitor&, const std::list< Element* >::const_iterator& ) override;

        OUString URI;
    };

    struct GraphicalElement : public Element
    {
    protected:
        GraphicalElement(Element* pParent, sal_Int32 nGCId)
            : Element(pParent)
            , GCId(nGCId)
            , MirrorVertical(false)
            , IsForText(false)
            , FontSize(0.0)
            , TextStyleId(0)
        {
        }

    public:
        sal_Int32 GCId;
        bool      MirrorVertical;
        bool      IsForText;
        double    FontSize;
        sal_Int32 TextStyleId;
    };

    struct DrawElement : public GraphicalElement
    {
    protected:
        DrawElement( Element* pParent, sal_Int32 nGCId )
        : GraphicalElement( pParent, nGCId ), isCharacter(false), ZOrder(0) {}

    public:
        bool      isCharacter;
        sal_Int32 ZOrder;
    };

    struct FrameElement : public DrawElement
    {
        friend class ElementFactory;
    protected:
        FrameElement( Element* pParent, sal_Int32 nGCId )
        : DrawElement( pParent, nGCId ) {}

    public:
        virtual void visitedBy( ElementTreeVisitor&, const std::list< Element* >::const_iterator& ) override;
    };

    struct TextElement : public GraphicalElement
    {
        friend class ElementFactory;
    protected:
        TextElement( Element* pParent, sal_Int32 nGCId, sal_Int32 nFontId )
        : GraphicalElement( pParent, nGCId ), FontId( nFontId ) {}

    public:
        virtual void visitedBy( ElementTreeVisitor&, const std::list< Element* >::const_iterator& ) override;

        OUStringBuffer Text;
        sal_Int32           FontId;
    };

    struct ParagraphElement : public Element
    {
        friend class ElementFactory;
    protected:
        explicit ParagraphElement( Element* pParent ) : Element( pParent ), Type( Normal ), bRtl( false ) {}

    public:
        virtual void visitedBy( ElementTreeVisitor&, const std::list< Element* >::const_iterator& rParentIt ) override;

        // returns true only if only a single line is contained
        bool isSingleLined( PDFIProcessor& rProc ) const;
        // returns the highest line height of the contained textelements
        // line height is font height if the text element is itself multilined
        double getLineHeight( PDFIProcessor& rProc ) const;
        // returns the first text element child; does not recurse through subparagraphs
        TextElement* getFirstTextChild() const;

        enum ParagraphType { Normal, Headline };
        ParagraphType       Type;
    bool bRtl;
    };

    struct PolyPolyElement : public DrawElement
    {
        friend class ElementFactory;
    protected:
        PolyPolyElement( Element* pParent, sal_Int32 nGCId,
                         const basegfx::B2DPolyPolygon& rPolyPoly,
                         sal_Int8 nAction );
    public:
        virtual void visitedBy( ElementTreeVisitor&, const std::list< Element* >::const_iterator& rParentIt ) override;

        void updateGeometry();

#if OSL_DEBUG_LEVEL > 1
        virtual void emitStructure( int nLevel );
#endif

        basegfx::B2DPolyPolygon PolyPoly;
        sal_Int8                Action;
    };

    struct ImageElement : public DrawElement
    {
        friend class ElementFactory;
    protected:
        ImageElement( Element* pParent, sal_Int32 nGCId, ImageId nImage )
        : DrawElement( pParent, nGCId ), Image( nImage ) {}

    public:
        virtual void visitedBy( ElementTreeVisitor&, const std::list< Element* >::const_iterator& ) override;

        ImageId Image;
    };

    struct PageElement : public Element
    {
        friend class ElementFactory;
    protected:
        PageElement( Element* pParent, sal_Int32 nPageNr )
        : Element( pParent ), PageNumber( nPageNr ), Hyperlinks(),
        TopMargin( 0.0 ), BottomMargin( 0.0 ), LeftMargin( 0.0 ), RightMargin( 0.0 ),
        HeaderElement( nullptr ), FooterElement( nullptr )
        {}
    private:
        // helper method for resolveHyperlinks
        bool resolveHyperlink( std::list<Element*>::iterator link_it, std::list<Element*>& rElements );
    public:
        virtual ~PageElement();

        virtual void visitedBy( ElementTreeVisitor&, const std::list< Element* >::const_iterator& rParentIt ) override;

        static void updateParagraphGeometry( Element* pEle );
        void resolveHyperlinks();
        void resolveFontStyles( PDFIProcessor& rProc );
        void resolveUnderlines( PDFIProcessor& rProc );

        sal_Int32      PageNumber;
        ListElement    Hyperlinks; // contains not yet realized links on this page
        double         TopMargin;
        double         BottomMargin;
        double         LeftMargin;
        double         RightMargin;
        Element*       HeaderElement;
        Element*       FooterElement;
    };

    struct DocumentElement : public Element
    {
        friend class ElementFactory;
    protected:
        DocumentElement() : Element( nullptr ) {}
    public:
        virtual ~DocumentElement();

        virtual void visitedBy( ElementTreeVisitor&, const std::list< Element* >::const_iterator& ) override;
    };

    // this class is the differentiator of document types: it will create
    // Element objects with an optimize() method suitable for the document type
    class ElementFactory
    {
    public:
        ElementFactory() {}
        virtual ~ElementFactory();

        static HyperlinkElement* createHyperlinkElement( Element* pParent, const OUString& rURI )
        { return new HyperlinkElement( pParent, rURI ); }

        static TextElement* createTextElement( Element* pParent, sal_Int32 nGCId, sal_Int32 nFontId )
        { return new TextElement( pParent, nGCId, nFontId ); }
        static ParagraphElement* createParagraphElement( Element* pParent )
        { return new ParagraphElement( pParent ); }

        static FrameElement* createFrameElement( Element* pParent, sal_Int32 nGCId )
        { return new FrameElement( pParent, nGCId ); }
        static PolyPolyElement*
            createPolyPolyElement( Element* pParent,
                                   sal_Int32 nGCId,
                                   const basegfx::B2DPolyPolygon& rPolyPoly,
                                   sal_Int8 nAction)
        { return new PolyPolyElement( pParent, nGCId, rPolyPoly, nAction ); }
        static ImageElement* createImageElement( Element* pParent, sal_Int32 nGCId, ImageId nImage )
        { return new ImageElement( pParent, nGCId, nImage ); }

        static PageElement* createPageElement( Element* pParent,
                                                sal_Int32 nPageNr )
        { return new PageElement( pParent, nPageNr ); }
        static DocumentElement* createDocumentElement()
        { return new DocumentElement(); }
    };
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
