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
struct Point;

struct SVXCORE_DLLPUBLIC ConnectionValues
{
    // XML_type: Which relation this Connection states: parOf a parent and a child of the data tree,
    // presOf which presentation Point shows a Point, presParOf a parent and a child of the
    // presentation tree.
    TypeConstant    mnXMLType = XML_parOf;

    // XML_modelId: The id of this Connection. A parTrans or a sibTrans Point names it in msCnxId.
    OUString        msModelId;

    // XML_srcId: The modelId of the Point the relation starts at.
    OUString        msSourceId;

    // XML_destId: The modelId of the Point the relation ends at.
    OUString        msDestId;

    // XML_presId: The id of the layout node that states this Connection.
    OUString        msPresId;

    // XML_sibTransId: For a parOf the modelId of the sibTrans Point that draws what separates this
    // child from the next one.
    OUString        msSibTransId;

    // XML_parTransId: For a parOf the modelId of the parTrans Point that draws what joins this
    // child to its parent.
    OUString        msParTransId;

    // XML_srcOrd: For a parOf or a presParOf the place of the child among its siblings. For a
    // presOf which presentation of the Point this is, for a Point that is shown in more than one
    // place.
    sal_Int32       mnSourceOrder = 0;

    // XML_destOrd: For a presOf the paragraph of the shape that represents this Point, where one
    // shape represents several of them. Zero otherwise, and zero throughout many files.
    sal_Int32       mnDestOrder = 0;
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

    // The Points that msSourceId and msDestId name. Resolved by
    // DiagramData_svx::buildDiagramDataModel, which resets them before it resolves them again,
    // and left empty by a copy, which belongs to no model.
    Point* mpSourcePoint = nullptr;
    Point* mpDestinationPoint = nullptr;

    // The child index that a parOf or a presParOf Connection defines
    sal_Int32 getChildIndex() const;

    // Which presentation of its Point a presOf Connection defines, for a Point that is shown in
    // more than one place
    sal_Int32 getPresentationIndex() const;

    // Which paragraph of the shape a presOf Connection defines, for a shape that represents more
    // than one Point
    sal_Int32 getParagraphIndex() const;
};

typedef std::vector< rtl::Reference< Connection > > Connections;

// A Point
// The dgm:presLayoutVars of a presentation Point
struct SVXCORE_DLLPUBLIC PresentationLayoutVariables
{
    // XML_resizeHandles: What the file states about resizing this node. Empty when it states
    // nothing.
    OUString     msResizeHandles;

    // XML_chMax: The largest number of children the layout draws for this node. -1 when the file
    // states none.
    sal_Int32    mnMaxChildren = -1;

    // XML_chPref: The number of children the layout is built for. -1 when the file states none.
    sal_Int32    mnPreferredChildren = -1;

    // XML_dir: The direction the layout runs in, norm or rev. A layout condition tests it.
    sal_Int32    mnDirection = XML_norm;

    // XML_hierBranch: For a hierarchy the side the branches go to. A Point that holds none takes
    // the value of the nearest one above it that does.
    std::optional<sal_Int32> moHierarchyBranch;

    // XML_orgChart: The layout draws an organization chart, which is what gives an asst Point a
    // place of its own.
    bool         mbOrgChartEnabled : 1 = false;

    // XML_bulletEnabled: Bullets are drawn for the text of this node.
    bool         mbBulletEnabled : 1 = false;
};

// The layout, the quick style and the colours the whole Diagram was built with. Only its
// root Point states them, so no other Point holds them.
struct SVXCORE_DLLPUBLIC RootValues
{
    OUString     msColorTransformCategoryId; // XML_csCatId
    OUString     msColorTransformTypeId; // XML_csTypeId
    OUString     msLayoutCategoryId; // XML_loCatId
    OUString     msLayoutTypeId; // XML_loTypeId
    OUString     msQuickStyleCategoryId; // XML_qsCatId
    OUString     msQuickStyleTypeId; // XML_qsTypeId
};

// The dgm:prSet attributes of a Point that are read on import and written on export and are not
// looked at in between. No file of the 167 layout examples states any of them.
struct SVXCORE_DLLPUBLIC PreservedValues
{
    /* Variable        varName    XML_Tag */
    sal_Int32    mnCustomAngle = -1; // XML_custAng
    sal_Int32    mnPercentageNeighbourWidth = -1; // XML_custLinFactNeighborX
    sal_Int32    mnPercentageNeighbourHeight = -1; // XML_custLinFactNeighborY
    sal_Int32    mnPercentageOwnWidth = -1; // XML_custLinFactX
    sal_Int32    mnPercentageOwnHeight = -1; // XML_custLinFactY
    sal_Int32    mnIncludeAngleScale = -1; // XML_custRadScaleInc
    sal_Int32    mnRadiusScale = -1; // XML_custRadScaleRad
    sal_Int32    mnWidthScale = -1; // XML_custScaleX
    sal_Int32    mnHeightScale = -1; // XML_custScaleY
    sal_Int32    mnWidthOverride = -1; // XML_custSzX
    sal_Int32    mnHeightOverride = -1; // XML_custSzY
    bool         mbCoherent3DOffset : 1 = false; // XML_coherent3DOff
    bool         mbCustomHorizontalFlip : 1 = false; // XML_custFlipHor
    bool         mbCustomVerticalFlip : 1 = false; // XML_custFlipVert
    bool         mbIsPlaceholder : 1 = false; // XML_phldr
};

// What a presentation Point states about itself. A Point of another role states none of
// it, so it holds no such values.
struct SVXCORE_DLLPUBLIC PresentationValues
{
    // XML_presAssocID: For a presentation Point the modelId of the Point it was built for. A layout
    // gives it to every presentation Point it builds for a Point, so it identifies none of them.
    OUString     msPresentationAssociationId;

    // XML_presName: The name the layout gives this presentation Point, rootText1 or ConnectLine1
    // for example.
    OUString     msPresentationLayoutName;

    // XML_presStyleLbl: Which style of the quickStyle the shape takes, node1 or revTx for example.
    // Empty for a Point that takes none.
    OUString     msPresentationLayoutStyleLabel;

    // XML_presStyleCnt: How many Points share this style label, so a style can vary over them. -1
    // for a Point that states none.
    sal_Int32    mnLayoutStyleCount = -1;

    // XML_presStyleIdx: The place of this Point among those sharing the style label, counted from
    // zero. -1 for a Point that states none.
    sal_Int32    mnLayoutStyleIndex = -1;
};

struct SVXCORE_DLLPUBLIC PointValues
{
    // PT: dgm:pt
    // PRS: dgm:prSet
    // PLV: dgm:presLayoutVars

    // XML_cxnId: For a parTrans or a sibTrans Point the modelId of the parOf Connection it belongs
    // to. Empty for every other Point.
    /* TYP */ OUString     msCnxId;

    // XML_modelId: The id of this Point. Connections name it, and the shape that draws it carries
    // it.
    /* PT  */ OUString     msModelId;

    // XML_phldrT: The text a node shows while it holds none of its own, [Text] for a node that was
    // just added.
    /* PRS */ OUString     msPlaceholderText;

    // XML_type: Which kind of Point this is: doc the single root of the data tree, node and asst
    // hold the text, parTrans and sibTrans draw what joins and separates, pres is built by the
    // layout.
    /* PT  */ TypeConstant mnXMLType = XML_node;

    // XML_custT: The text of the node is the user's own, so the layout leaves it as it is.
    /* PRS */ bool         mbCustomText : 1 = false;

    // The dgm:presLayoutVars of a presentation Point. A Point that states none of them holds no
    // variables at all, which is also what decides whether the element gets written again.
    std::optional<PresentationLayoutVariables> moLayoutVariables;

    // The variables, or the defaults for a Point that holds none
    const PresentationLayoutVariables& getLayoutVariables() const;

    // The variables, created at the defaults if the Point holds none yet
    PresentationLayoutVariables& ensureLayoutVariables();

    // The values that only travel from the file back to the file. Nothing reasons about them, so
    // a Point that states none of them holds none, and then writes none.
    std::optional<PreservedValues> moPreserved;

    // The preserved values, or the defaults for a Point that holds none
    const PreservedValues& getPreserved() const;

    // The preserved values, created at the defaults if the Point holds none yet
    PreservedValues& ensurePreserved();

    // The layout, the style and the colours of the whole Diagram, on its root Point only.
    std::optional<RootValues> moRoot;

    // The root values, or the defaults for a Point that holds none
    const RootValues& getRoot() const;

    // The root values, created at the defaults if the Point holds none yet
    RootValues& ensureRoot();

    // What a presentation Point states about itself. A Point of another role holds none of it.
    std::optional<PresentationValues> moPresentation;

    // The presentation values, or the defaults for a Point that holds none
    const PresentationValues& getPresentation() const;

    // The presentation values, created at the defaults if the Point holds none yet
    PresentationValues& ensurePresentation();

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

    // The presOf Connections that start at this Point, so the presentation Points that show it.
    // Resolved and reset like the Connection members above.
    std::vector<Connection*> maShownBy;

    // The presOf Connections that end at this Point, so the Points that this presentation Point
    // shows. Its size is how many Points one shape represents.
    std::vector<Connection*> maShows;
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
