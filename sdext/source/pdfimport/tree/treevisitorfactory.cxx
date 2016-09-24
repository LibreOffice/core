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


#include "treevisitorfactory.hxx"
#include "writertreevisiting.hxx"
#include "drawtreevisiting.hxx"

namespace pdfi
{
    struct WriterTreeVisitorFactory : public TreeVisitorFactory
    {
        WriterTreeVisitorFactory() {}

        virtual std::shared_ptr<ElementTreeVisitor> createOptimizingVisitor(PDFIProcessor& rProc) const override
        {
            return std::shared_ptr<ElementTreeVisitor>(new WriterXmlOptimizer(rProc));
        }

        virtual std::shared_ptr<ElementTreeVisitor> createStyleCollectingVisitor(
            StyleContainer& rStyles,
            PDFIProcessor&  rProc ) const override
        {
            return std::shared_ptr<ElementTreeVisitor>(new WriterXmlFinalizer(rStyles,rProc));
        }

        virtual std::shared_ptr<ElementTreeVisitor> createEmittingVisitor(EmitContext& rEmitContext) const override
        {
            return std::shared_ptr<ElementTreeVisitor>(new WriterXmlEmitter(rEmitContext));
        }
    };

    struct ImpressTreeVisitorFactory : public TreeVisitorFactory
    {
        ImpressTreeVisitorFactory() {}

        virtual std::shared_ptr<ElementTreeVisitor> createOptimizingVisitor(PDFIProcessor& rProc) const override
        {
            return std::shared_ptr<ElementTreeVisitor>(new DrawXmlOptimizer(rProc));
        }

        virtual std::shared_ptr<ElementTreeVisitor> createStyleCollectingVisitor(
            StyleContainer& rStyles,
            PDFIProcessor&  rProc ) const override
        {
            return std::shared_ptr<ElementTreeVisitor>(new DrawXmlFinalizer(rStyles,rProc));
        }

        virtual std::shared_ptr<ElementTreeVisitor> createEmittingVisitor(EmitContext& rEmitContext) const override
        {
            return std::shared_ptr<ElementTreeVisitor>(new DrawXmlEmitter(rEmitContext,
                                                                            DrawXmlEmitter::IMPRESS_DOC
                                                                            ));
        }
    };

    struct DrawTreeVisitorFactory : public TreeVisitorFactory
    {
        DrawTreeVisitorFactory() {}

        virtual std::shared_ptr<ElementTreeVisitor> createOptimizingVisitor(PDFIProcessor& rProc) const override
        {
            return std::shared_ptr<ElementTreeVisitor>(new DrawXmlOptimizer(rProc));
        }

        virtual std::shared_ptr<ElementTreeVisitor> createStyleCollectingVisitor(
            StyleContainer& rStyles,
            PDFIProcessor&  rProc ) const override
        {
            return std::shared_ptr<ElementTreeVisitor>(new DrawXmlFinalizer(rStyles,rProc));
        }

        virtual std::shared_ptr<ElementTreeVisitor> createEmittingVisitor(EmitContext& rEmitContext) const override
        {
            return std::shared_ptr<ElementTreeVisitor>(new DrawXmlEmitter(rEmitContext,
                                                                            DrawXmlEmitter::DRAW_DOC
                                                                            ));
        }
    };

    TreeVisitorFactorySharedPtr createWriterTreeVisitorFactory()
    {
        return TreeVisitorFactorySharedPtr(new WriterTreeVisitorFactory());
    }
    TreeVisitorFactorySharedPtr createImpressTreeVisitorFactory()
    {
        return TreeVisitorFactorySharedPtr(new ImpressTreeVisitorFactory());
    }
    TreeVisitorFactorySharedPtr createDrawTreeVisitorFactory()
    {
        return TreeVisitorFactorySharedPtr(new DrawTreeVisitorFactory());
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
