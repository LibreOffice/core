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

#ifndef INCLUDED_PDFI_WRITERTREEVISITING_HXX
#define INCLUDED_PDFI_WRITERTREEVISITING_HXX

#include "treevisiting.hxx"

namespace pdfi
{
    struct DrawElement;

    class WriterXmlOptimizer : public ElementTreeVisitor
    {
    private:
        PDFIProcessor& m_rProcessor;
        void optimizeTextElements(Element& rParent);
        void checkHeaderAndFooter( PageElement& rElem );

    public:
        explicit WriterXmlOptimizer(PDFIProcessor& rProcessor) :
            m_rProcessor(rProcessor)
        {}
        virtual ~WriterXmlOptimizer()
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

    class WriterXmlFinalizer : public ElementTreeVisitor
    {
    private:
        StyleContainer& m_rStyleContainer;
        PDFIProcessor&  m_rProcessor;

        void setFirstOnPage( ParagraphElement&    rElem,
                             StyleContainer&      rStyles,
                             const rtl::OUString& rMasterPageName );

    public:
        explicit WriterXmlFinalizer(StyleContainer& rStyleContainer,
                                    PDFIProcessor&  rProcessor) :
            m_rStyleContainer(rStyleContainer),
            m_rProcessor(rProcessor)
        {}
        virtual ~WriterXmlFinalizer()
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

    class WriterXmlEmitter : public ElementTreeVisitor
    {
    private:
        EmitContext& m_rEmitContext ;
        void fillFrameProps( DrawElement&       rElem,
                             PropertyMap&       rProps,
                             const EmitContext& rEmitContext );

    public:
        explicit WriterXmlEmitter(EmitContext& rEmitContext) :
            m_rEmitContext(rEmitContext)
        {}
        virtual ~WriterXmlEmitter()
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
