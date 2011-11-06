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



#ifndef INCLUDED_PDFI_TREEVISITING_HXX
#define INCLUDED_PDFI_TREEVISITING_HXX

#include <sal/config.h>
#include <list>


namespace pdfi
{
    class  PDFIProcessor;
    class  StyleContainer;
    struct HyperlinkElement;
    struct TextElement;
    struct ParagraphElement;
    struct FrameElement;
    struct PolyPolyElement;
    struct ImageElement;
    struct PageElement;
    struct DocumentElement;
    struct EmitContext;
    struct Element;

    /** To be visited by all tree element types

        Visitor interface from the "visitor pattern". Implementor gets
        called with actual tree node instances.
     */
    struct ElementTreeVisitor
    {
        virtual void visit( HyperlinkElement&, const std::list< Element* >::const_iterator& ) = 0;
        virtual void visit( TextElement&, const std::list< Element* >::const_iterator&  ) = 0;
        virtual void visit( ParagraphElement&, const std::list< Element* >::const_iterator&  ) = 0;
        virtual void visit( FrameElement&, const std::list< Element* >::const_iterator&  ) = 0;
        virtual void visit( PolyPolyElement&, const std::list< Element* >::const_iterator&  ) = 0;
        virtual void visit( ImageElement&, const std::list< Element* >::const_iterator&  ) = 0;
        virtual void visit( PageElement&, const std::list< Element* >::const_iterator&  ) = 0;
        virtual void visit( DocumentElement&, const std::list< Element* >::const_iterator&  ) = 0;
    };
    typedef boost::shared_ptr<ElementTreeVisitor> ElementTreeVisitorSharedPtr;

    /** Visitee interface

        To be implemented by every tree node that needs to be
        visitable.
     */
    struct ElementTreeVisitable
    {
        virtual void visitedBy( ElementTreeVisitor&, const std::list< Element* >::const_iterator& rParentIt ) = 0;
    };
    typedef boost::shared_ptr<ElementTreeVisitable> ElementTreeVisitableSharedPtr;
}

#endif
