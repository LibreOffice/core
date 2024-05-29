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

#include "datamodel.hxx"
#include <oox/drawingml/shape.hxx>

namespace com::sun::star {
    namespace xml::dom { class XDocument; }
}

namespace oox::drawingml {

class Diagram;
class LayoutNode;
typedef std::shared_ptr< LayoutNode > LayoutNodePtr;
class LayoutAtom;
typedef std::shared_ptr< LayoutAtom > LayoutAtomPtr;
typedef std::map< OUString, css::uno::Reference<css::xml::dom::XDocument> > DiagramDomMap;
typedef std::map< OUString, LayoutAtomPtr > LayoutAtomMap;
typedef std::map< const svx::diagram::Point*, ShapePtr > PresPointShapeMap;

class DiagramLayout
{
public:
    DiagramLayout(Diagram& rDgm)
        : mrDgm(rDgm)
    {
    }
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
    Diagram& getDiagram() { return mrDgm; }
    LayoutNodePtr & getNode()
        { return mpNode; }
    const LayoutNodePtr & getNode() const
        { return mpNode; }
    OoxDiagramDataPtr& getSampData()
        { return mpSampData; }
    const OoxDiagramDataPtr& getSampData() const
        { return mpSampData; }
    OoxDiagramDataPtr& getStyleData()
        { return mpStyleData; }
    const OoxDiagramDataPtr& getStyleData() const
        { return mpStyleData; }
    LayoutAtomMap & getLayoutAtomMap()
        { return maLayoutAtomMap; }
    PresPointShapeMap & getPresPointShapeMap()
        { return maPresPointShapeMap; }

private:
    Diagram& mrDgm;
    OUString msDefStyle;
    OUString msMinVer;
    OUString msUniqueId;

    OUString msTitle;
    OUString msDesc;
    LayoutNodePtr  mpNode;
    OoxDiagramDataPtr mpSampData;
    OoxDiagramDataPtr mpStyleData;
    // TODO
    // catLst
    // clrData

    LayoutAtomMap maLayoutAtomMap;
    PresPointShapeMap maPresPointShapeMap;
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
    std::vector<oox::drawingml::Color> maFillColors;
    std::vector<oox::drawingml::Color> maLineColors;
    std::vector<oox::drawingml::Color> maEffectColors;
    std::vector<oox::drawingml::Color> maTextFillColors;
    std::vector<oox::drawingml::Color> maTextLineColors;
    std::vector<oox::drawingml::Color> maTextEffectColors;

    static const oox::drawingml::Color&
    getColorByIndex(const std::vector<oox::drawingml::Color>& rColors, sal_Int32 nIndex);
};

typedef std::map<OUString,DiagramColor> DiagramColorMap;

class Diagram
{
public:
    explicit Diagram();
    void setData( OoxDiagramDataPtr& pData )
        { mpData = pData; }
    const OoxDiagramDataPtr& getData() const
        { return mpData; }
    void setLayout( const DiagramLayoutPtr & pLayout )
        { mpLayout = pLayout; }
    const DiagramLayoutPtr& getLayout() const
        { return mpLayout; }

    DiagramQStyleMap& getStyles() { return maStyles; }
    const DiagramQStyleMap& getStyles() const { return maStyles; }
    DiagramColorMap& getColors() { return maColors; }
    const DiagramColorMap& getColors() const { return maColors; }
    DiagramDomMap & getDomMap() { return maMainDomMap; }
    css::uno::Sequence< css::uno::Sequence< css::uno::Any > > & getDataRelsMap() { return maDataRelsMap; }
    void addTo( const ShapePtr & pShape, bool bCreate );

    css::uno::Sequence<css::beans::PropertyValue> getDomsAsPropertyValues() const;
    oox::core::NamedShapePairs& getDiagramFontHeights() { return maDiagramFontHeights; }
    void syncDiagramFontHeights();

private:
    // This contains groups of shapes: automatic font size is the same in each group.
    oox::core::NamedShapePairs maDiagramFontHeights;

    OoxDiagramDataPtr              mpData;
    DiagramLayoutPtr               mpLayout;
    DiagramQStyleMap               maStyles;
    DiagramColorMap                maColors;
    DiagramDomMap                  maMainDomMap;
    css::uno::Sequence< css::uno::Sequence< css::uno::Any > > maDataRelsMap;
};

typedef std::shared_ptr< Diagram > DiagramPtr;

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
