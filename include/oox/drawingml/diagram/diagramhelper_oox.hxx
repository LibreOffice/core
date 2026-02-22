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

#pragma once

#include <rtl/ustring.hxx>
#include <oox/drawingml/theme.hxx>
#include <oox/shape/ShapeFilterBase.hxx>
#include <svx/svdogrp.hxx>
#include <svx/diagram/DiagramHelper_svx.hxx>

namespace oox::drawingml {

class SmartArtDiagram;
class DrawingML;

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
class DiagramHelper_oox final : public svx::diagram::DiagramHelper_svx
{
    const std::shared_ptr< SmartArtDiagram >         mpDiagramPtr;
    std::shared_ptr<::oox::drawingml::Theme>    mpDiagramThemePtr;

    css::awt::Size maDiagramImportSize;

    // data values set by addDiagramNode to be used by next reLayout call
    // when a new ode gets added
    OUString msNewNodeId;
    OUString msNewNodeText;

    bool hasDiagramData() const;

    static void moveDiagramModelDataFromOldToNewXShape(
        const css::uno::Reference<css::drawing::XShape>& xOldShape,
        const css::uno::Reference<css::drawing::XShape>& xNewShape);

protected:
    // access associated SdrObjGroup/XShape/RootShape
    virtual css::uno::Reference< css::drawing::XShape >& accessRootShape() override;

public:
    DiagramHelper_oox(
        std::shared_ptr< SmartArtDiagram > xDiagramPtr,
        std::shared_ptr<::oox::drawingml::Theme> xTheme,
        css::awt::Size aImportSize);
    explicit DiagramHelper_oox(DiagramHelper_oox const& rSource);
    virtual ~DiagramHelper_oox();

    // re-create XShapes
    virtual void reLayout() override;

    // get text representation of data tree
    virtual OUString getDiagramString() const override;

    // get children of provided data node
    // use empty string for top-level nodes
    // returns vector of (id, text)
    virtual std::vector<std::pair<OUString, OUString>> getDiagramChildren(const OUString& rParentId) const override;

    // add/remove new top-level node to data model, returns its id
    virtual OUString addDiagramNode(const OUString& rText) override;
    virtual bool removeDiagramNode(const OUString& rNodeId) override;
    virtual void TextInformationChange() override;

    // Undo/Redo helpers to extract/restore Diagram-defining data
    virtual std::shared_ptr< svx::diagram::DiagramDataState > extractDiagramDataState() const override;
    virtual void applyDiagramDataState(const std::shared_ptr< svx::diagram::DiagramDataState >& rState) override;

    void doAnchor(css::uno::Reference<css::drawing::XShape>& rTarget);
    const std::shared_ptr< ::oox::drawingml::Theme >& getOrCreateThemePtr(
        const rtl::Reference< oox::shape::ShapeFilterBase>& rxFilter ) const;

    // access to get/set PropertyValues
    void setOOXDomValue(svx::diagram::DomMapFlag aDomMapFlag, const css::uno::Any& rValue);
    virtual css::uno::Any getOOXDomValue(svx::diagram::DomMapFlag aDomMapFlag) const override;

    // check if mandatory DiagramDomS exist and/or were not touched
    virtual bool checkMinimalDataDoms() const override;

    // helpers to write some specific DiagramDoms
    void writeDiagramOOXData(DrawingML& rOriginalDrawingML, css::uno::Reference<css::io::XOutputStream>& xOutputStream, std::u16string_view rDrawingRelId) const;
    void writeDiagramOOXDrawing(DrawingML& rOriginalDrawingML, css::uno::Reference<css::io::XOutputStream>& xOutputStream) const;

    // needed to create DiagramHelper_oox in svx' SdrObjGroup copy constructor
    virtual DiagramHelper_oox* clone() const override;
};

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
