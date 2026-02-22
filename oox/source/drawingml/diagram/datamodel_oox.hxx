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

#ifndef INCLUDED_OOX_SOURCE_DRAWINGML_DIAGRAM_DATAMODEL_HXX
#define INCLUDED_OOX_SOURCE_DRAWINGML_DIAGRAM_DATAMODEL_HXX

#include <map>
#include <memory>
#include <vector>

#include <rtl/ustring.hxx>

#include <svx/diagram/datamodel_svx.hxx>
#include <oox/drawingml/drawingmltypes.hxx>
#include <oox/helper/helper.hxx>
#include <oox/token/tokens.hxx>
#include <oox/shape/ShapeFilterBase.hxx>

namespace oox::drawingml {

class DrawingML;

class DiagramData_oox : public svx::diagram::DiagramData_svx
{
public:
    typedef std::map< OUString, ShapePtr > PointShapeMap;

    DiagramData_oox();
    explicit DiagramData_oox(DiagramData_oox const& rSource);
    virtual ~DiagramData_oox();

    // creates temporary processing data from model data
    virtual void buildDiagramDataModel(bool bClearOoxShapes);

    FillPropertiesPtr& getBackgroundShapeFillProperties() { return mpBackgroundShapeFillProperties; }
#ifdef DBG_UTIL
    virtual void dump() const;
#endif

    Shape* getOrCreateAssociatedShape(const svx::diagram::Point& rPoint, bool bCreateOnDemand = false) const;

    // helpers to write some specific DiagramDoms
    void writeDiagramReplacement(DrawingML& rOriginalDrawingML, sax_fastparser::FSHelperPtr& rTarget);
    void writeDiagramData(DrawingML& rOriginalDrawingML, sax_fastparser::FSHelperPtr& rTarget, std::u16string_view rDrawingRelId);

private:
    // The model definition, the parts *only* available in oox. Also look for already
    // defined ModelData in svx::diagram::DiagramData_svx

    // - FillStyle for Diagram Background (empty constructed, may stay empty)
    FillPropertiesPtr mpBackgroundShapeFillProperties;

    // temporary processing data, deleted when using build(). Association
    // map between oox::drawingml::Shape and svx::diagram::Point ModelData
    PointShapeMap     maPointShapeMap;
};

// Oox-local definition of DiagramData_oox. Doing and using this on Oox
// allows to do much less static_cast(s) - if at all from svx::diagram::DiagramData_svx
typedef std::shared_ptr< DiagramData_oox > OoxDiagramDataPtr;

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
