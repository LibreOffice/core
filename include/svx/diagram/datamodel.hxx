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

#include <vector>
#include <optional>
#include <map>

#include <svx/svxdllapi.h>
#include <rtl/ustring.hxx>
#include <rtl/ustrbuf.hxx>

#include <com/sun/star/xml/dom/XDocument.hpp>
#include <com/sun/star/drawing/XShape.hpp>

namespace svx::diagram {

enum TypeConstant {
    XML_none = 0,
    XML_type = 395,
    XML_asst = 680,
    XML_doc = 1924,
    XML_node = 3596,
    XML_norm = 3609,
    XML_parOf = 3878,
    XML_parTrans = 3879,
    XML_pres = 4085,
    XML_presOf = 4090,
    XML_presParOf = 4091,
    XML_rel = 4298,
    XML_sibTrans = 4746,
};

/** A Connection
 */
struct SVXCORE_DLLPUBLIC Connection
{
    Connection();

    TypeConstant mnXMLType;
    OUString msModelId;
    OUString msSourceId;
    OUString msDestId;
    OUString msParTransId;
    OUString msPresId;
    OUString msSibTransId;
    sal_Int32 mnSourceOrder;
    sal_Int32 mnDestOrder;

};

typedef std::vector< Connection > Connections;

/** Text and properties for a point
 * For proof of concept to make TextData available in svx level this
 * is in a first run pretty simple, but may need to be extended accordingly
 * up to similar data as in oox::drawingml::TextBody.
 */
struct SVXCORE_DLLPUBLIC TextBody
{
    // text from 1st paragraph (1st run)
    OUString msText;

    // attributes from TextBody::getTextProperties()
    std::vector< std::pair< OUString, css::uno::Any >> maTextProps;
};

typedef std::shared_ptr< TextBody > TextBodyPtr;

/** Styles for a Point (FillStyle/LineStyle/...)
 */
struct SVXCORE_DLLPUBLIC PointStyle
{
    // attributes (LineStyle/FillStyle/...)
    std::vector< std::pair< OUString, css::uno::Any >> maProperties;
};

typedef std::shared_ptr< PointStyle > PointStylePtr;

/** A point
 */
struct SVXCORE_DLLPUBLIC Point
{
    Point();

    // The minimal text data from the imported Diagram
    // in source format
    TextBodyPtr msTextBody;

    // The property sequence of pairs<OUString, css::uno::Any>,
    // interpreted & assigned by the ::addShape(s) creators in the
    // import filter that created a XShape associated/based on this entry
    PointStylePtr msPointStylePtr;

    OUString msCnxId;
    OUString msModelId;
    OUString msColorTransformCategoryId;
    OUString msColorTransformTypeId;
    OUString msLayoutCategoryId;
    OUString msLayoutTypeId;
    OUString msPlaceholderText;
    OUString msPresentationAssociationId;
    OUString msPresentationLayoutName;
    OUString msPresentationLayoutStyleLabel;
    OUString msQuickStyleCategoryId;
    OUString msQuickStyleTypeId;

    TypeConstant mnXMLType;
    sal_Int32     mnMaxChildren;
    sal_Int32     mnPreferredChildren;
    sal_Int32     mnDirection;
    std::optional<sal_Int32> moHierarchyBranch;
    sal_Int32     mnResizeHandles;
    sal_Int32     mnCustomAngle;
    sal_Int32     mnPercentageNeighbourWidth;
    sal_Int32     mnPercentageNeighbourHeight;
    sal_Int32     mnPercentageOwnWidth;
    sal_Int32     mnPercentageOwnHeight;
    sal_Int32     mnIncludeAngleScale;
    sal_Int32     mnRadiusScale;
    sal_Int32     mnWidthScale;
    sal_Int32     mnHeightScale;
    sal_Int32     mnWidthOverride;
    sal_Int32     mnHeightOverride;
    sal_Int32     mnLayoutStyleCount;
    sal_Int32     mnLayoutStyleIndex;

    bool          mbOrgChartEnabled;
    bool          mbBulletEnabled;
    bool          mbCoherent3DOffset;
    bool          mbCustomHorizontalFlip;
    bool          mbCustomVerticalFlip;
    bool          mbCustomText;
    bool          mbIsPlaceholder;
};

typedef std::vector< Point >        Points;

/** Snippet of Diagram ModelData for Diagram-defining data undo/redo
 */
class SVXCORE_DLLPUBLIC DiagramDataState
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
class SVXCORE_DLLPUBLIC DiagramData
{
public:
    typedef std::map< OUString, Point* > PointNameMap;
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
    // shall not be incarnated - target to use is oox::drawingml::DiagramData
    DiagramData();

public:
    virtual ~DiagramData();

    // creates temporary processing data from model data
    virtual void buildDiagramDataModel(bool bClearOoxShapes);

    // dump to readable format
    virtual void dump() const = 0;

    // read accesses
    Connections& getConnections() { return maConnections; }
    Points& getPoints() { return maPoints; }
    StringMap& getPresOfNameMap() { return maPresOfNameMap; }
    PointNameMap& getPointNameMap() { return maPointNameMap; }
    PointsNameMap& getPointsPresNameMap() { return maPointsPresNameMap; }
    ::std::vector<OUString>& getExtDrawings() { return maExtDrawings; }
    const Point* getRootPoint() const;
    OUString getString() const;
    std::vector<std::pair<OUString, OUString>> getChildren(const OUString& rParentId) const;

    const css::uno::Reference< css::xml::dom::XDocument >& getThemeDocument() const { return mxThemeDocument; }
    void setThemeDocument( const css::uno::Reference< css::xml::dom::XDocument >& xRef ) { mxThemeDocument = xRef; }

    const OUString& getBackgroundShapeModelID() const { return msBackgroundShapeModelID; }
    void setBackgroundShapeModelID( const OUString& rModelID ) { msBackgroundShapeModelID = rModelID; }

    // model modifiers
    OUString addNode(const OUString& rText);
    bool removeNode(const OUString& rNodeId);

    // Undo/Redo helpers to extract/restore Diagram-defining data
    DiagramDataStatePtr extractDiagramDataState() const;
    void applyDiagramDataState(const DiagramDataStatePtr& rState);

protected:
    // helpers
    void getChildrenString(OUStringBuffer& rBuf, const Point* pPoint, sal_Int32 nLevel) const;
    void addConnection(TypeConstant nType, const OUString& sSourceId, const OUString& sDestId);

    // evtl. existing alternative imported visualization identifier
    ::std::vector<OUString>  maExtDrawings;

    // The model definition, the parts available in svx.
    // See evtl. parts in oox::drawingml::DiagramData that may need t obe accessed
    // - logic connections/associations
    Connections maConnections;

    // - data point entries
    Points maPoints;

    // - style for the BackgroundShape (if used)
    //   this is the property sequence of pairs<OUString, css::uno::Any>,
    //   as interpreted & assigned by the ::addShape(s) creators in the
    //   import filter
    PointStylePtr maBackgroundShapeStyle;

    // - Theme definition as css::xml::dom::XDocument
    //    Note: I decided to use dom::XDocument which is already in use, instead of a
    //          temp file what is also possible (implemented that for POC) but would
    //          need to be created in PresentationFragmentHandler::importSlide. If
    //          this needs to be written to a File, please refer to
    //          fileDocxExport::WriteTheme(), look for "OOXTheme"
    css::uno::Reference< css::xml::dom::XDocument > mxThemeDocument;

    // temporary processing data, partially deleted when using build()
    PointNameMap      maPointNameMap;
    PointsNameMap     maPointsPresNameMap;
    ConnectionNameMap maConnectionNameMap;
    StringMap         maPresOfNameMap;
    OUString          msBackgroundShapeModelID;
};

typedef std::shared_ptr< DiagramData > DiagramDataPtr;

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
