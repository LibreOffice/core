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

#ifndef INCLUDED_SVX_DIAGRAM_DATAMODEL_HXX
#define INCLUDED_SVX_DIAGRAM_DATAMODEL_HXX

#include <config_options.h>

#include <vector>
#include <optional>
#include <map>

#include <svx/svxdllapi.h>
#include <svx/diagram/DomMapFlag.hxx>
#include <rtl/ustring.hxx>
#include <rtl/ustrbuf.hxx>

#include <com/sun/star/xml/dom/XDocument.hpp>
#include <com/sun/star/drawing/XShape.hpp>
#include <oox/token/tokens.hxx>
#include <sax/fshelper.hxx>

namespace svx::diagram {

enum TypeConstant {
    XML_none = 0,
    XML_type = oox::XML_Type,
    XML_asst = oox::XML_asst,
    XML_doc = oox::XML_doc,
    XML_node = oox::XML_node,
    XML_norm = oox::XML_norm,
    XML_parOf = oox::XML_parOf,
    XML_parTrans = oox::XML_parTrans,
    XML_pres = oox::XML_pres,
    XML_presOf = oox::XML_presOf,
    XML_presParOf = oox::XML_presParOf,
    XML_rel = oox::XML_rel,
    XML_sibTrans = oox::XML_sibTrans,
};

/** A Connection
 */
struct SVXCORE_DLLPUBLIC Connection
{
    Connection();

    TypeConstant mnXMLType; // default is XML_parOf
    OUString msModelId;
    OUString msSourceId;
    OUString msDestId;
    OUString msParTransId;
    OUString msPresId;
    OUString msSibTransId;
    sal_Int32 mnSourceOrder;
    sal_Int32 mnDestOrder;

    void writeDiagramData(sax_fastparser::FSHelperPtr& rTarget);
};

typedef std::vector< Connection > Connections;

/** A point
 */
struct SVXCORE_DLLPUBLIC Point
{
    Point();

    // PT: dgm:pt
    // PRS: dgm:prSet
    // PLV: dgm:presLayoutVars

    /* PT  */ OUString msCnxId;
    /* PT  */ OUString msModelId;
    /* PRS */ OUString msColorTransformCategoryId;
    /* PRS */ OUString msColorTransformTypeId;
    /* PRS */ OUString msLayoutCategoryId;
    /* PRS */ OUString msLayoutTypeId;
    /* PRS */ OUString msPlaceholderText;
    /* PRS */ OUString msPresentationAssociationId;
    /* PRS */ OUString msPresentationLayoutName;
    /* PRS */ OUString msPresentationLayoutStyleLabel;
    /* PRS */ OUString msQuickStyleCategoryId;
    /* PRS */ OUString msQuickStyleTypeId;
    /* PLV */ OUString msResizeHandles;

    /* PT  */ TypeConstant mnXMLType; // default is XML_node
    /* PLV */ sal_Int32 mnMaxChildren;
    /* PLV */ sal_Int32 mnPreferredChildren;
    /* PLV */ sal_Int32 mnDirection;
    /* PLV */ std::optional<sal_Int32> moHierarchyBranch;

    /* PRS */ sal_Int32 mnCustomAngle;
    /* PRS */ sal_Int32 mnPercentageNeighbourWidth;
    /* PRS */ sal_Int32 mnPercentageNeighbourHeight;
    /* PRS */ sal_Int32 mnPercentageOwnWidth;
    /* PRS */ sal_Int32 mnPercentageOwnHeight;
    /* PRS */ sal_Int32 mnIncludeAngleScale;
    /* PRS */ sal_Int32 mnRadiusScale;
    /* PRS */ sal_Int32 mnWidthScale;
    /* PRS */ sal_Int32 mnHeightScale;
    /* PRS */ sal_Int32 mnWidthOverride;
    /* PRS */ sal_Int32 mnHeightOverride;
    /* PRS */ sal_Int32 mnLayoutStyleCount;
    /* PRS */ sal_Int32 mnLayoutStyleIndex;

    /* PLV */ bool mbOrgChartEnabled : 1;
    /* PLV */ bool mbBulletEnabled : 1;
    /* PRS */ bool mbCoherent3DOffset : 1;
    /* PRS */ bool mbCustomHorizontalFlip : 1;
    /* PRS */ bool mbCustomVerticalFlip : 1;
    /* PRS */ bool mbCustomText : 1;
    /* PRS */ bool mbIsPlaceholder : 1;

    void writeDiagramData_data(sax_fastparser::FSHelperPtr& rTarget);
};

void SVXCORE_DLLPUBLIC addTypeConstantToFastAttributeList(TypeConstant aTypeConstant, rtl::Reference<sax_fastparser::FastAttributeList>& rAttributeList, bool bPoint);
typedef std::vector< Point >        Points;

/** Snippet of Diagram ModelData for Diagram-defining data undo/redo
 */
class UNLESS_MERGELIBS(SVXCORE_DLLPUBLIC) DiagramDataState
{
    Connections maConnections;
    Points maPoints;

public:
    DiagramDataState(Connections aConnections, Points aPoints);

    Connections& getConnections() { return maConnections; }
    Points& getPoints() { return maPoints; }
};

typedef std::shared_ptr< DiagramDataState > DiagramDataStatePtr;

/** The collected Diagram ModelData
 */
class SVXCORE_DLLPUBLIC DiagramData_svx
{
public:
    typedef std::map< OUString, std::vector< Point* > > PointsNameMap;
    typedef std::map< OUString, const Connection* > ConnectionNameMap;

    struct SourceIdAndDepth
    {
        OUString msSourceId;
        sal_Int32 mnDepth = 0;
    };

    /// Tracks connections: destination id -> {destination order, details} map.
    typedef std::map< OUString, std::map<sal_Int32, SourceIdAndDepth > > StringMap;

protected:
    // Make constructor protected to signal that this anyway pure virtual class
    // shall not be incarnated - target to use is oox::drawingml::DiagramData_oox
    DiagramData_svx();
    explicit DiagramData_svx(DiagramData_svx const& rSource);

public:
    // access associated SdrObjGroup/XShape/RootShape
    css::uno::Reference< css::drawing::XShape >& accessRootShape() { return mxRootShape; }
    const css::uno::Reference< css::drawing::XShape >& accessRootShape() const { return mxRootShape; }

    virtual ~DiagramData_svx();

    // creates temporary processing data from model data
    virtual void buildDiagramDataModel(bool bClearOoxShapes);

    // dump to readable format
#ifdef DBG_UTIL
    virtual void dump() const = 0;
    #endif

    // read accesses
    Connections& getConnections() { return maConnections; }
    Points& getPoints() { return maPoints; }
    const Points& getPoints() const { return maPoints; }
    StringMap& getPresOfNameMap() { return maPresOfNameMap; }
    PointsNameMap& getPointsPresNameMap() { return maPointsPresNameMap; }
    ::std::vector<OUString>& getExtDrawings() { return maExtDrawings; }
    const Point* getRootPoint() const;
    OUString getDiagramString() const;
    std::vector<std::pair<OUString, OUString>> getDiagramChildren(const OUString& rParentId) const;

    const css::uno::Reference< css::xml::dom::XDocument >& getThemeDocument() const { return mxThemeDocument; }
    void setThemeDocument( const css::uno::Reference< css::xml::dom::XDocument >& xRef ) { mxThemeDocument = xRef; }

    const OUString& getBackgroundShapeModelID() const { return msBackgroundShapeModelID; }
    void setBackgroundShapeModelID( const OUString& rModelID ) { msBackgroundShapeModelID = rModelID; }

    // model modifiers
    std::pair<OUString, DomMapFlags> addDiagramNode();
    DomMapFlags removeDiagramNode(const OUString& rNodeId);

    // Undo/Redo helpers to extract/restore Diagram-defining data
    DiagramDataStatePtr extractDiagramDataState() const;
    void applyDiagramDataState(const DiagramDataStatePtr& rState);

    css::uno::Reference<css::drawing::XShape> getMasterXShapeForPoint(const Point& rPoint) const;
    OUString getTextForPoint(const Point& rPoint) const;
    css::uno::Reference<css::drawing::XShape> getXShapeByModelID(std::u16string_view rModelID) const;
    const Point* getPointByModelID(std::u16string_view rModelID) const;

protected:
    // helpers
    void getDiagramChildrenString(OUStringBuffer& rBuf, const Point* pPoint, sal_Int32 nLevel) const;
    void addConnection(TypeConstant nType, const OUString& sSourceId, const OUString& sDestId);

    // remember associated SdrObjGroup/XShape/RootShape
    css::uno::Reference< css::drawing::XShape > mxRootShape;

    // evtl. existing alternative imported visualization identifier
    ::std::vector<OUString>  maExtDrawings;

    // The model definition, the parts available in svx.
    // See evtl. parts in oox::drawingml::DiagramData_oox that may need t obe accessed
    // - logic connections/associations
    Connections maConnections;

    // - data point entries
    Points maPoints;

    // - Theme definition as css::xml::dom::XDocument
    //    Note: I decided to use dom::XDocument which is already in use, instead of a
    //          temp file what is also possible (implemented that for POC) but would
    //          need to be created in PresentationFragmentHandler::importSlide. If
    //          this needs to be written to a File, please refer to
    //          fileDocxExport::WriteTheme(), look for "OOXTheme"
    css::uno::Reference< css::xml::dom::XDocument > mxThemeDocument;

    // temporary processing data, partially deleted when using build()
    PointsNameMap     maPointsPresNameMap;
    ConnectionNameMap maConnectionNameMap;
    StringMap         maPresOfNameMap;
    OUString          msBackgroundShapeModelID;
};

typedef std::shared_ptr< DiagramData_svx > DiagramDataPtr_svx;

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
