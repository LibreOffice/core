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

#ifndef INCLUDED_OOX_SOURCE_DRAWINGML_DIAGRAM_DIAGRAM_HXX
#define INCLUDED_OOX_SOURCE_DRAWINGML_DIAGRAM_DIAGRAM_HXX

#include <map>
#include <memory>
#include <vector>

#include <rtl/ustring.hxx>

#include <oox/drawingml/shape.hxx>
#include <drawingml/fillproperties.hxx>
#include <oox/token/tokens.hxx>

namespace com { namespace sun { namespace star {
    namespace xml { namespace dom { class XDocument; } }
} } }

namespace oox { namespace drawingml {

namespace dgm {

/** A Connection
 */
struct Connection
{
    Connection() :
        mnType( 0 ),
        mnSourceOrder( 0 ),
        mnDestOrder( 0 )
    {}

    void dump() const;

    sal_Int32 mnType;
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

/** A point
 */
struct Point
{
    Point() :
        mnType(0),
        mnMaxChildren(-1),
        mnPreferredChildren(-1),
        mnDirection(XML_norm),
        mnResizeHandles(XML_rel),
        mnCustomAngle(-1),
        mnPercentageNeighbourWidth(-1),
        mnPercentageNeighbourHeight(-1),
        mnPercentageOwnWidth(-1),
        mnPercentageOwnHeight(-1),
        mnIncludeAngleScale(-1),
        mnRadiusScale(-1),
        mnWidthScale(-1),
        mnHeightScale(-1),
        mnWidthOverride(-1),
        mnHeightOverride(-1),
        mnLayoutStyleCount(-1),
        mnLayoutStyleIndex(-1),

        mbOrgChartEnabled(false),
        mbBulletEnabled(false),
        mbCoherent3DOffset(false),
        mbCustomHorizontalFlip(false),
        mbCustomVerticalFlip(false),
        mbCustomText(false),
        mbIsPlaceholder(false)
    {}
    void dump() const;

    ShapePtr      mpShape;

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

    sal_Int32     mnType;
    sal_Int32     mnMaxChildren;
    sal_Int32     mnPreferredChildren;
    sal_Int32     mnDirection;
    OptValue<sal_Int32> moHierarchyBranch;
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

}

class Diagram;
class LayoutNode;
typedef std::shared_ptr< LayoutNode > LayoutNodePtr;

typedef std::map< OUString, css::uno::Reference<css::xml::dom::XDocument> > DiagramDomMap;

class DiagramData
{
public:
    typedef std::map< OUString, dgm::Point* > PointNameMap;
    typedef std::map< OUString,
                      std::vector<dgm::Point*> >   PointsNameMap;
    typedef std::map< OUString, const dgm::Connection* > ConnectionNameMap;
    struct SourceIdAndDepth
    {
        OUString msSourceId;
        sal_Int32 mnDepth = 0;
    };
    /// Tracks connections: destination id -> {destination order, details} map.
    typedef std::map< OUString,
                      std::map<sal_Int32, SourceIdAndDepth > > StringMap;

    DiagramData();
    FillPropertiesPtr & getFillProperties()
        { return mpFillProperties; }
    dgm::Connections & getConnections()
        { return maConnections; }
    dgm::Points & getPoints()
        { return maPoints; }
    ConnectionNameMap & getConnectionNameMap()
        { return maConnectionNameMap; }
    StringMap & getPresOfNameMap()
        { return maPresOfNameMap; }
    PointNameMap & getPointNameMap()
        { return maPointNameMap; }
    PointsNameMap & getPointsPresNameMap()
        { return maPointsPresNameMap; }
    ::std::vector<OUString> &getExtDrawings()
        { return maExtDrawings; }
    const dgm::Point* getRootPoint() const;
    sal_Int32 getMaxDepth() const
        { return mnMaxDepth; }
    void setMaxDepth(sal_Int32 nDepth)
        { mnMaxDepth = nDepth; }
    void dump() const;
private:
    ::std::vector<OUString>  maExtDrawings;
    FillPropertiesPtr mpFillProperties;
    dgm::Connections  maConnections;
    dgm::Points       maPoints;
    PointNameMap      maPointNameMap;
    PointsNameMap     maPointsPresNameMap;
    ConnectionNameMap maConnectionNameMap;
    StringMap         maPresOfNameMap;
    sal_Int32         mnMaxDepth;
};

typedef std::shared_ptr< DiagramData > DiagramDataPtr;

class DiagramLayout
{
public:
    DiagramLayout(const Diagram& rDgm) : mrDgm(rDgm) {}
    void setDefStyle( const OUString & sDefStyle )
        { msDefStyle = sDefStyle; }
    void setMinVer( const OUString & sMinVer )
        { msMinVer = sMinVer; }
    void setUniqueId( const OUString & sUniqueId )
        { msUniqueId = sUniqueId; }
    void setTitle( const OUString & sTitle )
        { msTitle = sTitle; }
    void setDesc( const OUString & sDesc )
        { msDesc = sDesc; }
    const Diagram& getDiagram() const
        { return mrDgm; }
    LayoutNodePtr & getNode()
        { return mpNode; }
    const LayoutNodePtr & getNode() const
        { return mpNode; }
    DiagramDataPtr & getSampData()
        { return mpSampData; }
    const DiagramDataPtr & getSampData() const
        { return mpSampData; }
    DiagramDataPtr & getStyleData()
        { return mpStyleData; }
    const DiagramDataPtr & getStyleData() const
        { return mpStyleData; }

private:
    const Diagram& mrDgm;
    OUString msDefStyle;
    OUString msMinVer;
    OUString msUniqueId;

    OUString msTitle;
    OUString msDesc;
    LayoutNodePtr  mpNode;
    DiagramDataPtr mpSampData;
    DiagramDataPtr mpStyleData;
    // TODO
    // catLst
    // clrData
};

typedef std::shared_ptr< DiagramLayout > DiagramLayoutPtr;

struct DiagramStyle
{
    ShapeStyleRef maFillStyle;
    ShapeStyleRef maLineStyle;
    ShapeStyleRef maEffectStyle;
    ShapeStyleRef maTextStyle;
};

typedef std::map<OUString,DiagramStyle> DiagramQStyleMap;

struct DiagramColor
{
    oox::drawingml::Color maFillColor;
    oox::drawingml::Color maLineColor;
    oox::drawingml::Color maEffectColor;
    oox::drawingml::Color maTextFillColor;
    oox::drawingml::Color maTextLineColor;
    oox::drawingml::Color maTextEffectColor;
};

typedef std::map<OUString,DiagramColor> DiagramColorMap;

class Diagram
{
public:
    void setData( const DiagramDataPtr & pData )
        { mpData = pData; }
    const DiagramDataPtr& getData() const
        { return mpData; }
    void setLayout( const DiagramLayoutPtr & pLayout )
        { mpLayout = pLayout; }

    DiagramQStyleMap& getStyles() { return maStyles; }
    const DiagramQStyleMap& getStyles() const { return maStyles; }
    DiagramColorMap& getColors() { return maColors; }
    const DiagramColorMap& getColors() const { return maColors; }
    DiagramDomMap & getDomMap() { return maMainDomMap; }
    css::uno::Sequence< css::uno::Sequence< css::uno::Any > > & getDataRelsMap() { return maDataRelsMap; }
    void addTo( const ShapePtr & pShape );

    css::uno::Sequence<css::beans::PropertyValue> getDomsAsPropertyValues() const;
private:
    void build( );
    DiagramDataPtr                 mpData;
    DiagramLayoutPtr               mpLayout;
    DiagramQStyleMap               maStyles;
    DiagramColorMap                maColors;
    DiagramDomMap                  maMainDomMap;
    css::uno::Sequence< css::uno::Sequence< css::uno::Any > > maDataRelsMap;
};

typedef std::shared_ptr< Diagram > DiagramPtr;

} }

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
