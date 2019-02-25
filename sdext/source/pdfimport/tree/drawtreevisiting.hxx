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

#ifndef INCLUDED_SDEXT_SOURCE_PDFIMPORT_TREE_DRAWTREEVISITING_HXX
#define INCLUDED_SDEXT_SOURCE_PDFIMPORT_TREE_DRAWTREEVISITING_HXX

#include <treevisiting.hxx>

#include <com/sun/star/i18n/XBreakIterator.hpp>
#include <com/sun/star/i18n/XCharacterClassification.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

namespace pdfi
{
    struct DrawElement;

    class DrawXmlOptimizer : public ElementTreeVisitor
    {
    private:
        PDFIProcessor& m_rProcessor;
        css::uno::Reference< css::i18n::XBreakIterator > mxBreakIter;

        void optimizeTextElements(Element& rParent);

    public:
        const css::uno::Reference< css::i18n::XBreakIterator >& GetBreakIterator();
        explicit DrawXmlOptimizer(PDFIProcessor& rProcessor) :
            m_rProcessor(rProcessor)
        {}

        virtual void visit( HyperlinkElement&, const std::list< std::unique_ptr<Element> >::const_iterator& ) override;
        virtual void visit( TextElement&, const std::list< std::unique_ptr<Element> >::const_iterator&  ) override;
        virtual void visit( ParagraphElement&, const std::list< std::unique_ptr<Element> >::const_iterator&  ) override;
        virtual void visit( FrameElement&, const std::list< std::unique_ptr<Element> >::const_iterator&  ) override;
        virtual void visit( PolyPolyElement&, const std::list< std::unique_ptr<Element> >::const_iterator&  ) override;
        virtual void visit( ImageElement&, const std::list< std::unique_ptr<Element> >::const_iterator&  ) override;
        virtual void visit( PageElement&, const std::list< std::unique_ptr<Element> >::const_iterator&  ) override;
        virtual void visit( DocumentElement&, const std::list< std::unique_ptr<Element> >::const_iterator&  ) override;
    };

    class DrawXmlFinalizer : public ElementTreeVisitor
    {
    private:
        StyleContainer& m_rStyleContainer;
        PDFIProcessor&  m_rProcessor;

    public:
        explicit DrawXmlFinalizer(StyleContainer& rStyleContainer,
                                  PDFIProcessor&  rProcessor) :
            m_rStyleContainer(rStyleContainer),
            m_rProcessor(rProcessor)
        {}

        virtual void visit( HyperlinkElement&, const std::list< std::unique_ptr<Element> >::const_iterator& ) override;
        virtual void visit( TextElement&, const std::list< std::unique_ptr<Element> >::const_iterator&  ) override;
        virtual void visit( ParagraphElement&, const std::list< std::unique_ptr<Element> >::const_iterator&  ) override;
        virtual void visit( FrameElement&, const std::list< std::unique_ptr<Element> >::const_iterator&  ) override;
        virtual void visit( PolyPolyElement&, const std::list< std::unique_ptr<Element> >::const_iterator&  ) override;
        virtual void visit( ImageElement&, const std::list< std::unique_ptr<Element> >::const_iterator&  ) override;
        virtual void visit( PageElement&, const std::list< std::unique_ptr<Element> >::const_iterator&  ) override;
        virtual void visit( DocumentElement&, const std::list< std::unique_ptr<Element> >::const_iterator&  ) override;
    };

    class DrawXmlEmitter : public ElementTreeVisitor
    {
    private:
        css::uno::Reference< css::i18n::XCharacterClassification > mxCharClass;

        EmitContext& m_rEmitContext ;
        /// writes Impress doc when false
        const bool   m_bWriteDrawDocument;

        static void fillFrameProps( DrawElement&       rElem,
                             PropertyMap&       rProps,
                             const EmitContext& rEmitContext,
                             bool               bWasTransformed
                             );

    public:
        const css::uno::Reference< css::i18n::XCharacterClassification >& GetCharacterClassification();
        enum DocType{ DRAW_DOC, IMPRESS_DOC };
        explicit DrawXmlEmitter(EmitContext& rEmitContext, DocType eDocType) :
            m_rEmitContext(rEmitContext),
            m_bWriteDrawDocument(eDocType==DRAW_DOC)
        {}

        virtual void visit( HyperlinkElement&, const std::list< std::unique_ptr<Element> >::const_iterator& ) override;
        virtual void visit( TextElement&, const std::list< std::unique_ptr<Element> >::const_iterator&  ) override;
        virtual void visit( ParagraphElement&, const std::list< std::unique_ptr<Element> >::const_iterator&  ) override;
        virtual void visit( FrameElement&, const std::list< std::unique_ptr<Element> >::const_iterator&  ) override;
        virtual void visit( PolyPolyElement&, const std::list< std::unique_ptr<Element> >::const_iterator&  ) override;
        virtual void visit( ImageElement&, const std::list< std::unique_ptr<Element> >::const_iterator&  ) override;
        virtual void visit( PageElement&, const std::list< std::unique_ptr<Element> >::const_iterator&  ) override;
        virtual void visit( DocumentElement&, const std::list< std::unique_ptr<Element> >::const_iterator&  ) override;
    };
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
