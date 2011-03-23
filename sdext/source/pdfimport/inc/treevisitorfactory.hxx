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

#ifndef INCLUDED_PDFI_TREEVISITORFACTORY_HXX
#define INCLUDED_PDFI_TREEVISITORFACTORY_HXX

#include "pdfihelper.hxx"
#include <boost/shared_ptr.hpp>

namespace pdfi
{
    struct ElementTreeVisitor;
    struct EmitContext;
    class PDFIProcessor;
    class StyleContainer;

    /** Tree manipulation factory

        Creates visitor objects performing various operations on the
        pdf parse tree
     */
    struct TreeVisitorFactory
    {
        virtual ~TreeVisitorFactory() {}

        /// Create visitor that combines tree nodes
        virtual boost::shared_ptr<ElementTreeVisitor> createOptimizingVisitor(PDFIProcessor&) const = 0;
        /// Create visitor that prepares style info
        virtual boost::shared_ptr<ElementTreeVisitor> createStyleCollectingVisitor(
            StyleContainer&, PDFIProcessor&) const = 0;
        /// Create visitor that emits tree to an output target
        virtual boost::shared_ptr<ElementTreeVisitor> createEmittingVisitor(EmitContext&, PDFIProcessor&) const = 0;
    };

    typedef boost::shared_ptr<TreeVisitorFactory> TreeVisitorFactorySharedPtr;

    TreeVisitorFactorySharedPtr createWriterTreeVisitorFactory();
    TreeVisitorFactorySharedPtr createImpressTreeVisitorFactory();
    TreeVisitorFactorySharedPtr createDrawTreeVisitorFactory();
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
