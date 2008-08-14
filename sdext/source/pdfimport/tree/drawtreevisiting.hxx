/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: drawtreevisiting.hxx,v $
 *
 * $Revision: 1.2 $
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

#ifndef INCLUDED_PDFI_DRAWTREEVISITING_HXX
#define INCLUDED_PDFI_DRAWTREEVISITING_HXX

#include "treevisiting.hxx"


namespace pdfi
{
    struct DrawElement;

    class DrawXmlOptimizer : public ElementTreeVisitor
    {
    private:
        PDFIProcessor& m_rProcessor;
        void optimizeTextElements(Element& rParent);

    public:
        explicit DrawXmlOptimizer(PDFIProcessor& rProcessor) :
            m_rProcessor(rProcessor)
        {}

        virtual void visit( HyperlinkElement&, const std::list< Element* >::const_iterator& );
        virtual void visit( TextElement&, const std::list< Element* >::const_iterator&  );
        virtual void visit( ParagraphElement&, const std::list< Element* >::const_iterator&  );
        virtual void visit( FrameElement&, const std::list< Element* >::const_iterator&  );
        virtual void visit( PolyPolyElement&, const std::list< Element* >::const_iterator&  );
        virtual void visit( ImageElement&, const std::list< Element* >::const_iterator&  );
        virtual void visit( PageElement&, const std::list< Element* >::const_iterator&  );
        virtual void visit( DocumentElement&, const std::list< Element* >::const_iterator&  );
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

        virtual void visit( HyperlinkElement&, const std::list< Element* >::const_iterator& );
        virtual void visit( TextElement&, const std::list< Element* >::const_iterator&  );
        virtual void visit( ParagraphElement&, const std::list< Element* >::const_iterator&  );
        virtual void visit( FrameElement&, const std::list< Element* >::const_iterator&  );
        virtual void visit( PolyPolyElement&, const std::list< Element* >::const_iterator&  );
        virtual void visit( ImageElement&, const std::list< Element* >::const_iterator&  );
        virtual void visit( PageElement&, const std::list< Element* >::const_iterator&  );
        virtual void visit( DocumentElement&, const std::list< Element* >::const_iterator&  );
    };

    class DrawXmlEmitter : public ElementTreeVisitor
    {
    private:
        EmitContext& m_rEmitContext ;
        /// writes Impress doc when false
        const bool   m_bWriteDrawDocument;

        void fillFrameProps( DrawElement&       rElem,
                             PropertyMap&       rProps,
                             const EmitContext& rEmitContext );

    public:
        enum DocType{ DRAW_DOC, IMPRESS_DOC };
        explicit DrawXmlEmitter(EmitContext& rEmitContext, DocType eDocType) :
            m_rEmitContext(rEmitContext),
            m_bWriteDrawDocument(eDocType==DRAW_DOC)
        {}

        virtual void visit( HyperlinkElement&, const std::list< Element* >::const_iterator& );
        virtual void visit( TextElement&, const std::list< Element* >::const_iterator&  );
        virtual void visit( ParagraphElement&, const std::list< Element* >::const_iterator&  );
        virtual void visit( FrameElement&, const std::list< Element* >::const_iterator&  );
        virtual void visit( PolyPolyElement&, const std::list< Element* >::const_iterator&  );
        virtual void visit( ImageElement&, const std::list< Element* >::const_iterator&  );
        virtual void visit( PageElement&, const std::list< Element* >::const_iterator&  );
        virtual void visit( DocumentElement&, const std::list< Element* >::const_iterator&  );
    };
}

#endif
