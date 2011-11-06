/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
