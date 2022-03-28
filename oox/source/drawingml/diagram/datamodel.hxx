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

#include <svx/diagram/datamodel.hxx>
#include <oox/drawingml/drawingmltypes.hxx>
#include <oox/helper/helper.hxx>
#include <oox/token/tokens.hxx>

namespace oox::drawingml {

class DiagramData
{
public:
    typedef std::map< OUString, ShapePtr > PointShapeMap;
    typedef std::map< OUString, TextBodyPtr > PointTextMap;
    typedef std::map< OUString, svx::diagram::Point* > PointNameMap;
    typedef std::map< OUString, std::vector< svx::diagram::Point* > > PointsNameMap;
    typedef std::map< OUString, const svx::diagram::Connection* > ConnectionNameMap;

    struct SourceIdAndDepth
    {
        OUString msSourceId;
        sal_Int32 mnDepth = 0;
    };

    /// Tracks connections: destination id -> {destination order, details} map.
    typedef std::map< OUString, std::map<sal_Int32, SourceIdAndDepth > > StringMap;

    DiagramData();
    virtual ~DiagramData() {}

    // creates temporary processing data from model data
    void build(bool bClearOoxShapes);

    FillPropertiesPtr & getFillProperties()
        { return mpFillProperties; }
    svx::diagram::Connections & getConnections()
        { return maConnections; }
    svx::diagram::Points & getPoints()
        { return maPoints; }
    StringMap & getPresOfNameMap()
        { return maPresOfNameMap; }
    PointNameMap & getPointNameMap()
        { return maPointNameMap; }
    PointsNameMap & getPointsPresNameMap()
        { return maPointsPresNameMap; }
    ::std::vector<OUString> &getExtDrawings()
        { return maExtDrawings; }
    const svx::diagram::Point* getRootPoint() const;
    void dump() const;

    OUString getString() const;
    std::vector<std::pair<OUString, OUString>> getChildren(const OUString& rParentId) const;
    OUString addNode(const OUString& rText);
    bool removeNode(const OUString& rNodeId);

    Shape* getOrCreateAssociatedShape(const svx::diagram::Point& rPoint, bool bCreateOnDemand = false) const;

    // get/set data between Diagram DataModel and oox::drawingml::Shape
    void secureDataFromShapeToModelAfterDiagramImport();
    void restoreDataFromModelToShapeAfterReCreation(const svx::diagram::Point& rPoint, Shape& rNewShape) const;

private:
    void getChildrenString(OUStringBuffer& rBuf, const svx::diagram::Point* pPoint, sal_Int32 nLevel) const;
    void addConnection(svx::diagram::TypeConstant nType, const OUString& sSourceId, const OUString& sDestId);

    // evtl. existing alternative imported visualization identifier
    ::std::vector<OUString>  maExtDrawings;

    // the model definition,
    // - FillStyle
    // - Texts for oox::drawingml::Points/svx::diagram::Points, associated by ModelId
    // - logic connections/associations
    // - data point entries
    FillPropertiesPtr mpFillProperties;
    PointTextMap      maPointTextMap;
    svx::diagram::Connections  maConnections;
    svx::diagram::Points       maPoints;

    // temporary processing data
    PointShapeMap     maPointShapeMap;
    PointNameMap      maPointNameMap;
    PointsNameMap     maPointsPresNameMap;
    ConnectionNameMap maConnectionNameMap;
    StringMap         maPresOfNameMap;
};

typedef std::shared_ptr< DiagramData > DiagramDataPtr;

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
