/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <rtl/ref.hxx>
#include <salhelper/simplereferenceobject.hxx>
#include <rtl/ustring.hxx>
#include <rtl/ustrbuf.hxx>

#include <com/sun/star/xml/dom/XDocument.hpp>
#include <com/sun/star/drawing/XShape.hpp>
#include <com/sun/star/frame/XModel.hpp>
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

// The values a Connection carries.
struct SVXCORE_DLLPUBLIC ConnectionValues
{
    /* Variable        varName    XML_Tag */
    TypeConstant    mnXMLType = XML_parOf; // XML_type
    OUString        msModelId; // XML_modelId
    OUString        msSourceId; // XML_srcId
    OUString        msDestId; // XML_destId
    OUString        msPresId; // XML_presId
    OUString        msSibTransId; // XML_sibTransId
    OUString        msParTransId; // XML_parTransId
    sal_Int32       mnSourceOrder = 0; // XML_srcOrd
    sal_Int32       mnDestOrder = 0; // XML_destOrd
};

// A RefCounted Connection
struct SVXCORE_DLLPUBLIC Connection : public salhelper::SimpleReferenceObject,
                                      public ConnectionValues
{
    Connection();
    Connection(const Connection& rSource);
    virtual ~Connection() override;

    Connection& operator=(const Connection& rSource)
    {
        ConnectionValues::operator=(rSource);
        return *this;
    }

    void writeDiagramData_connection(sax_fastparser::FSHelperPtr& rTarget);
};

typedef std::vector< rtl::Reference< Connection > > Connections;

// A Point
struct SVXCORE_DLLPUBLIC PointValues
{
    // PT: dgm:pt
    // PRS: dgm:prSet
    // PLV: dgm:presLayoutVars

    /* TYP */ OUString     msCnxId; // XML_cxnId
    /* PT  */ OUString     msModelId; // XML_modelId
    /* PRS */ OUString     msColorTransformCategoryId; // XML_csCatId
    /* PRS */ OUString     msColorTransformTypeId; // XML_csTypeId
    /* PRS */ OUString     msLayoutCategoryId; // XML_loCatId
    /* PRS */ OUString     msLayoutTypeId; // XML_loTypeId
    /* PRS */ OUString     msPlaceholderText; // XML_phldrT
    /* PRS */ OUString     msPresentationAssociationId; // XML_presAssocID
    /* PRS */ OUString     msPresentationLayoutName; // XML_presName
    /* PRS */ OUString     msPresentationLayoutStyleLabel; // XML_presStyleLbl
    /* PRS */ OUString     msQuickStyleCategoryId; // XML_qsCatId
    /* PRS */ OUString     msQuickStyleTypeId; // XML_qsTypeId
    /* PLV */ OUString     msResizeHandles; // XML_resizeHandles

    /* PT  */ TypeConstant mnXMLType = XML_node; // XML_type
    /* PLV */ sal_Int32    mnMaxChildren = -1; // XML_chMax
    /* PLV */ sal_Int32    mnPreferredChildren = -1; // XML_chPref
    /* PLV */ sal_Int32    mnDirection = XML_norm; // XML_dir
    /* PLV */ std::optional<sal_Int32> moHierarchyBranch; // XML_hierBranch

    /* PRS */ sal_Int32    mnCustomAngle = -1; // XML_custAng
    /* PRS */ sal_Int32    mnPercentageNeighbourWidth = -1; // XML_custLinFactNeighborX
    /* PRS */ sal_Int32    mnPercentageNeighbourHeight = -1; // XML_custLinFactNeighborY
    /* PRS */ sal_Int32    mnPercentageOwnWidth = -1; // XML_custLinFactX
    /* PRS */ sal_Int32    mnPercentageOwnHeight = -1; // XML_custLinFactY
    /* PRS */ sal_Int32    mnIncludeAngleScale = -1; // XML_custRadScaleInc
    /* PRS */ sal_Int32    mnRadiusScale = -1; // XML_custRadScaleRad
    /* PRS */ sal_Int32    mnWidthScale = -1; // XML_custScaleX
    /* PRS */ sal_Int32    mnHeightScale = -1; // XML_custScaleY
    /* PRS */ sal_Int32    mnWidthOverride = -1; // XML_custSzX
    /* PRS */ sal_Int32    mnHeightOverride = -1; // XML_custSzY
    /* PRS */ sal_Int32    mnLayoutStyleCount = -1; // XML_presStyleCnt
    /* PRS */ sal_Int32    mnLayoutStyleIndex = -1; // XML_presStyleIdx

    /* PLV */ bool         mbOrgChartEnabled : 1 = false; // XML_orgChart
    /* PLV */ bool         mbBulletEnabled : 1 = false; // XML_bulletEnabled
    /* PRS */ bool         mbCoherent3DOffset : 1 = false; // XML_coherent3DOff
    /* PRS */ bool         mbCustomHorizontalFlip : 1 = false; // XML_custFlipHor
    /* PRS */ bool         mbCustomVerticalFlip : 1 = false; // XML_custFlipVert
    /* PRS */ bool         mbCustomText : 1 = false; // XML_custT
    /* PRS */ bool         mbIsPlaceholder : 1 = false; // XML_phldr
};

// A ref-counted point
struct SVXCORE_DLLPUBLIC Point : public salhelper::SimpleReferenceObject, public PointValues
{
    Point();
    Point(const Point& rSource);
    virtual ~Point() override;

    Point& operator=(const Point& rSource)
    {
        PointValues::operator=(rSource);
        return *this;
    }

    void writeDiagramData_data(sax_fastparser::FSHelperPtr& rTarget);
};

std::u16string_view SVXCORE_DLLPUBLIC getNameForTypeConstant(TypeConstant aTypeConstant);
void SVXCORE_DLLPUBLIC addTypeConstantToFastAttributeList(TypeConstant aTypeConstant, rtl::Reference<sax_fastparser::FastAttributeList>& rAttributeList, bool bPoint);

typedef std::vector< rtl::Reference< Point > > Points;

// struct to return results of adding a node to a Diagram
struct SVXCORE_DLLPUBLIC AddedDiagramNode
{
    // the ModelId of the node that was added
    OUString msNewNodeId;
    // the node that handed over as template
    OUString msTemplateNodeId;
    // what DomMaps to delete
    DomMapFlags maChangedParts;
};

// Create non-refCounted, real copied Lists, e.g. for Undo/Redo
Points SVXCORE_DLLPUBLIC copyPoints(const Points& rSource);
Connections SVXCORE_DLLPUBLIC copyConnections(const Connections& rSource);

/** Snippet of Diagram ModelData for Diagram-defining data undo/redo
 */
class UNLESS_MERGELIBS(SVXCORE_DLLPUBLIC) DiagramDataState
{
    Connections maConnections;
    Points maPoints;
    std::vector<css::uno::Reference<css::drawing::XShape>> mxShapes;

    // The saved Transformation of the shapes
    std::vector<basegfx::B2DHomMatrix> maShapeTransformations;
    basegfx::B2DHomMatrix maTransformation;

public:
    DiagramDataState(const Connections& aConnections, const Points& aPoints, const css::uno::Reference< css::drawing::XShape >& rRootShape);//, const OUString& rBackgroundShapeModelID);

    const Connections& getConnections() const { return maConnections; }
    const Points& getPoints() const { return maPoints; }
    const std::vector<css::uno::Reference<css::drawing::XShape>>& getXShapes() const { return mxShapes;}
    const std::vector<basegfx::B2DHomMatrix>& getShapeTransformations() const { return maShapeTransformations; }
    const basegfx::B2DHomMatrix& getTransformation() const { return maTransformation; }
};

typedef std::shared_ptr< DiagramDataState > DiagramDataStatePtr;

/** The collected Diagram ModelData
 */
class SVXCORE_DLLPUBLIC DiagramData_svx
{
public:
    typedef std::map< OUString, Points > PointsNameMap;
    typedef std::map< OUString, rtl::Reference< Connection > > ConnectionNameMap;

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
    // access associated SdrObjGroup/XShape/RootShape/XModel
    css::uno::Reference< css::drawing::XShape >& accessRootShape() { return mxRootShape; }
    const css::uno::Reference< css::drawing::XShape >& accessRootShape() const { return mxRootShape; }
    const css::uno::Reference< css::frame::XModel >& accessRootModel() const;

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

    // The presentation Points that carry each layout node name, in the order
    // the Diagram holds them as Xshapes/SdrObjects (the presParOf Connections define it)
    PointsNameMap& getPointsPresNameMap() { return maPointsPresNameMap; }
    ::std::vector<OUString>& getExtDrawings() { return maExtDrawings; }
    rtl::Reference<Point> getRootPoint() const;
    OUString getDiagramString() const;
    std::vector<std::pair<OUString, OUString>> getDiagramChildren(const OUString& rParentId) const;

    // Walks from a ModelId to the data node that it belongs to
    rtl::Reference<Point> getDataNodeForModelID(std::u16string_view rModelId) const;

    // True for a node that can take a node below it
    bool canHoldChildNode(std::u16string_view rNodeId) const;

    // True for a node that holds no nodes below it, false for a node that holds some and for a
    // ModelId that leads to no node at all.
    bool isChildNode(std::u16string_view rNodeId) const;

    // True for a presentation Point that draws one of the nodes of the Diagram. False for one that
    // draws the Diagram as a whole, such as the background shape of a layout, for one that draws a
    // transition between two nodes, and for a ModelId that names no presentation Point.
    bool isPresentationOfDataNode(std::u16string_view rModelId) const;

    const css::uno::Reference< css::xml::dom::XDocument >& getThemeDocument() const { return mxThemeDocument; }
    void setThemeDocument( const css::uno::Reference< css::xml::dom::XDocument >& xRef ) { mxThemeDocument = xRef; }

    // model modifiers
    // Removes the node that rNodeId leads to. A node that still holds nodes below it stays, unless
    // bRemoveAllChildren asks for the whole branch below it as well. The Diagram keeps its last
    // node, a removal that would leave it empty does not happen.
    DomMapFlags removeDiagramNode(std::u16string_view rNodeId, bool bRemoveAllChildren = false);

    // Adds a new node to the Diagram and gives back its ModelId, which stays empty when nothing
    // was added. Everything the new node needs, such as the name and the style label of the
    // presentation Points that draw it, is copied from a node that is already there, the template,
    // so a Diagram that holds no node to copy from will add no new node. rTargetNode names the
    // anchor node and bAsChild asks for a node below it instead of one beside it:
    // - an empty rTargetNode, and one that leads to no node of the Diagram, put the new node in
    //   front of the node that comes first at the top level, which is the template
    // - a node at the top level takes the new node right behind itself, with itself as the
    //   template
    // - a node at the top level with bAsChild takes the new node as the first of the nodes below
    //   it, with the node that comes first below it as the template. When it holds none, the node
    //   that comes first below the first top level node that holds any is the template
    // - a node that hangs below another node takes the new node right behind itself either way,
    //   with itself as the template
    AddedDiagramNode addDiagramNode(std::u16string_view rTargetNode, bool bAsChild = false);

    // Undo/Redo helpers to extract/restore Diagram-defining data
    DiagramDataStatePtr extractDiagramDataState() const;
    void applyDiagramDataState(const DiagramDataStatePtr& rState);

    css::uno::Reference<css::drawing::XShape> getMasterXShapeForPoint(const Point& rPoint,
                                                                      sal_Int32& rParagraph) const;
    OUString getTextForPoint(const Point& rPoint) const;
    css::uno::Reference<css::drawing::XShape> getXShapeByModelID(std::u16string_view rModelID) const;
    rtl::Reference<Point> getPointByModelID(std::u16string_view rModelID) const;

protected:
    // helpers
    // Adds a new node under the node that rParentId names, at the place among the nodes there
    // that nNewSourceOrder gives, and gives back as AddedDiagramNode the ModelId of the node, the
    // node that was used as templateand what DomMaps to delete. These stay empty when nothing
    // was added. The node rTemplateNodeId is used to set stuff like name and style label
    // of the presentation Points, and it may hang under another parent. The orders and the
    // presentation style indexes are renumbered afterwards to close gaps.
    AddedDiagramNode insertDiagramNode(std::u16string_view rParentId, sal_Int32 nNewSourceOrder,
                                       std::u16string_view rTemplateNodeId);

    void getDiagramChildrenString(OUStringBuffer& rBuf, const rtl::Reference<Point>& rPoint,
                                  sal_Int32 nLevel) const;
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
};

typedef std::shared_ptr< DiagramData_svx > DiagramDataPtr_svx;

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
