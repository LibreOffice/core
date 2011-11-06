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

