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

#ifndef INCLUDED_OOX_DRAWINGML_DIAGRAM_DIAGRAMHELPER_HXX
#define INCLUDED_OOX_DRAWINGML_DIAGRAM_DIAGRAMHELPER_HXX

#include <rtl/ustring.hxx>
#include <oox/drawingml/theme.hxx>
#include <oox/shape/ShapeFilterBase.hxx>
#include <svx/svdogrp.hxx>
#include <svx/diagram/IDiagramHelper.hxx>

namespace svx { namespace diagram {
    class DiagramDataState;
}}

namespace oox::drawingml {

class Diagram;

// Advanced DiagramHelper
//
// This helper tries to hold all necessary data to re-layout
// all XShapes/SdrObjects of an already imported Diagram. The
// Diagram holds the SmarArt model data before it gets layouted,
// while Theme holds the oox Fill/Line/Style definitions to
// apply.
// Re-Layouting (re-creating) is rather complex, for detailed
// information see ::reLayout implementation.
// This helper class may/should be extended to:
// - deliver representative data from the Diagram-Model
// - modify it eventually
// - im/export Diagram model to other representations
class AdvancedDiagramHelper final : public svx::diagram::IDiagramHelper
{
    const std::shared_ptr< Diagram >            mpDiagramPtr;
    std::shared_ptr<::oox::drawingml::Theme>    mpThemePtr;

    css::awt::Size maImportSize;

    bool hasDiagramData() const;

public:
    AdvancedDiagramHelper(
        std::shared_ptr< Diagram > xDiagramPtr,
        std::shared_ptr<::oox::drawingml::Theme> xTheme,
        css::awt::Size aImportSize,
        bool bSelfCreated);
    virtual ~AdvancedDiagramHelper();

    // re-create XShapes
    virtual void reLayout(SdrObjGroup& rTarget) override;

    // get text representation of data tree
    virtual OUString getString() const override;

    // get children of provided data node
    // use empty string for top-level nodes
    // returns vector of (id, text)
    virtual std::vector<std::pair<OUString, OUString>> getChildren(const OUString& rParentId) const override;

    // add/remove new top-level node to data model, returns its id
    virtual OUString addNode(const OUString& rText) override;
    virtual bool removeNode(const OUString& rNodeId) override;

    // Undo/Redo helpers to extract/restore Diagram-defining data
    virtual std::shared_ptr< svx::diagram::DiagramDataState > extractDiagramDataState() const override;
    virtual void applyDiagramDataState(const std::shared_ptr< svx::diagram::DiagramDataState >& rState) override;

    void doAnchor(SdrObjGroup& rTarget, ::oox::drawingml::Shape& rRootShape);
    const std::shared_ptr< ::oox::drawingml::Theme >& getOrCreateThemePtr(
        rtl::Reference< oox::shape::ShapeFilterBase>& rxFilter ) const;
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
