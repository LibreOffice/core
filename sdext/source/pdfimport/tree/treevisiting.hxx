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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
