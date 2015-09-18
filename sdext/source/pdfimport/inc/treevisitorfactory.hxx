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

#ifndef INCLUDED_SDEXT_SOURCE_PDFIMPORT_INC_TREEVISITORFACTORY_HXX
#define INCLUDED_SDEXT_SOURCE_PDFIMPORT_INC_TREEVISITORFACTORY_HXX

#include "pdfihelper.hxx"
#include <memory>

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
        virtual std::shared_ptr<ElementTreeVisitor> createOptimizingVisitor(PDFIProcessor&) const = 0;
        /// Create visitor that prepares style info
        virtual std::shared_ptr<ElementTreeVisitor> createStyleCollectingVisitor(
            StyleContainer&, PDFIProcessor&) const = 0;
        /// Create visitor that emits tree to an output target
        virtual std::shared_ptr<ElementTreeVisitor> createEmittingVisitor(EmitContext&) const = 0;
    };

    typedef std::shared_ptr<TreeVisitorFactory> TreeVisitorFactorySharedPtr;

    TreeVisitorFactorySharedPtr createWriterTreeVisitorFactory();
    TreeVisitorFactorySharedPtr createImpressTreeVisitorFactory();
    TreeVisitorFactorySharedPtr createDrawTreeVisitorFactory();
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
