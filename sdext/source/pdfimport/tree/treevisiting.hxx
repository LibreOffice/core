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

#ifndef INCLUDED_PDFI_TREEVISITING_HXX
#define INCLUDED_PDFI_TREEVISITING_HXX

#include <sal/config.h>
#include <list>


namespace pdfi
{
    struct HyperlinkElement;
    struct TextElement;
    struct ParagraphElement;
    struct FrameElement;
    struct PolyPolyElement;
    struct ImageElement;
    struct PageElement;
    struct DocumentElement;
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
        virtual ~ElementTreeVisitor() {}
    };
    typedef boost::shared_ptr<ElementTreeVisitor> ElementTreeVisitorSharedPtr;

    /** Visitee interface

        To be implemented by every tree node that needs to be
        visitable.
     */
    struct ElementTreeVisitable
    {
        virtual void visitedBy( ElementTreeVisitor&, const std::list< Element* >::const_iterator& rParentIt ) = 0;

    protected:
        ~ElementTreeVisitable() {}
    };
    typedef boost::shared_ptr<ElementTreeVisitable> ElementTreeVisitableSharedPtr;
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
