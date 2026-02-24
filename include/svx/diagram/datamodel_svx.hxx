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
#include <boost/property_tree/ptree.hpp>

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
    explicit Connection(const boost::property_tree::ptree& rConnectionData);

    /* DEFAULT      Variable        varName    XML_Tag */
    /* XML_parOf */ TypeConstant    mnXMLType; // XML_type
    /* (empty)   */ OUString        msModelId; // XML_modelId
    /* (empty)   */ OUString        msSourceId; // XML_srcId
    /* (empty)   */ OUString        msDestId; // XML_destId
    /* (empty)   */ OUString        msPresId; // XML_presId
    /* (empty)   */ OUString        msSibTransId; // XML_sibTransId
    /* (empty)   */ OUString        msParTransId; // XML_parTransId
    /* 0         */ sal_Int32       mnSourceOrder; // XML_srcOrd
    /* 0         */ sal_Int32       mnDestOrder; // XML_destOrd

    void writeDiagramData(sax_fastparser::FSHelperPtr& rTarget);
    void addDiagramModelData(boost::property_tree::ptree& rTarget) const;
};

typedef std::vector< Connection > Connections;

/** A point
 */
struct SVXCORE_DLLPUBLIC Point
{
    Point();
    explicit Point(const boost::property_tree::ptree& rPointData);

    // PT: dgm:pt
    // PRS: dgm:prSet
    // PLV: dgm:presLayoutVars

    /* TYP DEFAULT     Variable     varName  XML_Tag */
    /* PT  (empty)  */ OUString     msCnxId; // XML_cxnId
    /* PT  (empty)  */ OUString     msModelId; // XML_modelId
    /* PRS (empty)  */ OUString     msColorTransformCategoryId; // XML_csCatId
    /* PRS (empty)  */ OUString     msColorTransformTypeId; // XML_csTypeId
    /* PRS (empty)  */ OUString     msLayoutCategoryId; // XML_loCatId
    /* PRS (empty)  */ OUString     msLayoutTypeId; // XML_loTypeId
    /* PRS (empty)  */ OUString     msPlaceholderText; // XML_phldrT
    /* PRS (empty)  */ OUString     msPresentationAssociationId; // XML_presAssocID
    /* PRS (empty)  */ OUString     msPresentationLayoutName; // XML_presName
    /* PRS (empty)  */ OUString     msPresentationLayoutStyleLabel; // XML_presStyleLbl
    /* PRS (empty)  */ OUString     msQuickStyleCategoryId; // XML_qsCatId
    /* PRS (empty)  */ OUString     msQuickStyleTypeId; // XML_qsTypeId
    /* PLV (empty)  */ OUString     msResizeHandles; // XML_resizeHandles

    /* PT  XML_node */ TypeConstant mnXMLType; // default is XML_node // XML_type
    /* PLV -1       */ sal_Int32    mnMaxChildren; // XML_chMax
    /* PLV -1       */ sal_Int32    mnPreferredChildren; // XML_chPref
    /* PLV XML_norm */ sal_Int32    mnDirection; // XML_dir
    /* PLV (opt)    */ std::optional<sal_Int32> moHierarchyBranch; // XML_hierBranch

    /* PRS -1       */ sal_Int32    mnCustomAngle; // XML_custAng
    /* PRS -1       */ sal_Int32    mnPercentageNeighbourWidth; // XML_custLinFactNeighborX
    /* PRS -1       */ sal_Int32    mnPercentageNeighbourHeight; // XML_custLinFactNeighborY
    /* PRS -1       */ sal_Int32    mnPercentageOwnWidth; // XML_custLinFactX
    /* PRS -1       */ sal_Int32    mnPercentageOwnHeight; // XML_custLinFactY
    /* PRS -1       */ sal_Int32    mnIncludeAngleScale; // XML_custRadScaleInc
    /* PRS -1       */ sal_Int32    mnRadiusScale; // XML_custRadScaleRad
    /* PRS -1       */ sal_Int32    mnWidthScale; // XML_custScaleX
    /* PRS -1       */ sal_Int32    mnHeightScale; // XML_custScaleY
    /* PRS -1       */ sal_Int32    mnWidthOverride; // XML_custSzX
    /* PRS -1       */ sal_Int32    mnHeightOverride; // XML_custSzY
    /* PRS -1       */ sal_Int32    mnLayoutStyleCount; // XML_presStyleCnt
    /* PRS -1       */ sal_Int32    mnLayoutStyleIndex; // XML_presStyleIdx

    /* PLV (false)  */ bool         mbOrgChartEnabled : 1; // XML_orgChart
    /* PLV (false)  */ bool         mbBulletEnabled : 1; // XML_bulletEnabled
    /* PRS (false)  */ bool         mbCoherent3DOffset : 1; // XML_coherent3DOff
    /* PRS (false)  */ bool         mbCustomHorizontalFlip : 1; // XML_custFlipHor
    /* PRS (false)  */ bool         mbCustomVerticalFlip : 1; // XML_custFlipVert
    /* PRS (false)  */ bool         mbCustomText : 1; // XML_custT
    /* PRS (false)  */ bool         mbIsPlaceholder : 1; // XML_phldr

    void writeDiagramData_data(sax_fastparser::FSHelperPtr& rTarget);
    void addDiagramModelData(boost::property_tree::ptree& rTarget) const;
};

TypeConstant SVXCORE_DLLPUBLIC getTypeConstantForName(std::u16string_view aName);
std::u16string_view SVXCORE_DLLPUBLIC getNameForTypeConstant(TypeConstant aTypeConstant);
void SVXCORE_DLLPUBLIC addTypeConstantToFastAttributeList(TypeConstant aTypeConstant, rtl::Reference<sax_fastparser::FastAttributeList>& rAttributeList, bool bPoint);
void SVXCORE_DLLPUBLIC addTypeConstantToDiagramModelData(TypeConstant aTypeConstant, boost::property_tree::ptree& rTarget, bool bPoint);

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
    explicit DiagramData_svx(const boost::property_tree::ptree& rDiagramModel);

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
    const Connections& getConnections() const { return maConnections; }
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

    // write data to boost::property_tree
    void addDiagramModelData(boost::property_tree::ptree& rTarget) const;

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
