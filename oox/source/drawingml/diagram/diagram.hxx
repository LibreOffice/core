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

#include "datamodel_oox.hxx"
#include <oox/drawingml/shape.hxx>

namespace com::sun::star {
    namespace xml::dom { class XDocument; }
}

namespace oox::drawingml {

class SmartArtDiagram;
class LayoutNode;
typedef std::shared_ptr< LayoutNode > LayoutNodePtr;
class LayoutAtom;
typedef std::shared_ptr< LayoutAtom > LayoutAtomPtr;
typedef std::map< OUString, LayoutAtomPtr > LayoutAtomMap;
typedef std::map< const svx::diagram::Point*, ShapePtr > PresPointShapeMap;

class DiagramLayout
{
public:
    DiagramLayout(SmartArtDiagram& rDgm)
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
    SmartArtDiagram& getDiagram() { return mrDgm; }
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
    SmartArtDiagram& mrDgm;
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
typedef std::map<svx::diagram::DomMapFlag,css::uno::Any> DiagramPRDomMap;

class SmartArtDiagram
{
public:
    explicit SmartArtDiagram();
    explicit SmartArtDiagram(SmartArtDiagram const& rSource);
    void setData( const OoxDiagramDataPtr& pData )
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
    void createShapeHierarchyFromModel( const ShapePtr & pShape, bool bCreate );

    oox::core::NamedShapePairs& getDiagramFontHeights() { return maDiagramFontHeights; }
    void syncDiagramFontHeights();

    void setOOXDomValue(svx::diagram::DomMapFlag aDomMapFlag, const css::uno::Any& rValue);
    css::uno::Any getOOXDomValue(svx::diagram::DomMapFlag aDomMapFlag) const;
    void resetOOXDomValues(svx::diagram::DomMapFlags aDomMapFlags);

    // check if mandatory DiagramDomS exist (or can be created)
    bool checkMinimalDataDoms() const;

    // helpers to write some specific DiagramDoms
    void writeDiagramOOXData(DrawingML& rOriginalDrawingML, css::uno::Reference<css::io::XOutputStream>& xOutputStream, std::u16string_view rDrawingRelId) const;
    void writeDiagramOOXDrawing(DrawingML& rOriginalDrawingML, css::uno::Reference<css::io::XOutputStream>& xOutputStream) const;

private:
    // This contains groups of shapes: automatic font size is the same in each group.
    oox::core::NamedShapePairs maDiagramFontHeights;

    OoxDiagramDataPtr              mpData;
    DiagramLayoutPtr               mpLayout;
    DiagramQStyleMap               maStyles;
    DiagramColorMap                maColors;
    DiagramPRDomMap                maDiagramPRDomMap;
};

typedef std::shared_ptr< SmartArtDiagram > DiagramPtr;

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
