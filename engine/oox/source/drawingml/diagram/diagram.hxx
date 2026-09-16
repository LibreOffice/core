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

#ifndef INCLUDED_OOX_SOURCE_DRAWINGML_DIAGRAM_DIAGRAM_HXX
#define INCLUDED_OOX_SOURCE_DRAWINGML_DIAGRAM_DIAGRAM_HXX

#include <map>
#include <memory>
#include <set>
#include <vector>

#include <rtl/ustring.hxx>
#include <tools/color.hxx>

#include "datamodel_oox.hxx"
#include <oox/drawingml/shape.hxx>
#include <oox/token/tokens.hxx>

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
typedef std::map< rtl::Reference< svx::diagram::Point >, ShapePtr > PresPointShapeMap;

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

/** One color list of a style label in the colors part of a Diagram.

    A list holds the colors that the shapes carrying the style label are given,
    together with the way the colors are spread over those shapes.
 */
struct DiagramColorList
{
    /// The colors of the list, in the order the file gives them.
    std::vector<oox::drawingml::Color> maColors;

    /** The colors of maColors resolved against the theme, in the same order.

        Empty when the list holds fewer than two colors, or when mnMethod is neither
        XML_span nor XML_cycle, because a single color or a plain pick of one needs no
        resolved value.
     */
    std::vector<::Color> maResolvedColors;

    /** The meth attribute: XML_span, XML_cycle or XML_repeat.

        XML_span spreads the colors evenly over the shapes, XML_cycle walks them to the
        end and back again, and XML_repeat takes one color of the list per shape and starts
        the list over when it runs out. XML_span is the default.
     */
    sal_Int32 mnMethod = XML_span;
};

struct DiagramColor
{
    DiagramColorList maFillColors;
    DiagramColorList maLineColors;
    DiagramColorList maEffectColors;
    DiagramColorList maTextFillColors;
    DiagramColorList maTextLineColors;
    DiagramColorList maTextEffectColors;

    /** The color that the nIndex-th of nCount shapes carrying this list gets.

        Pass -1 as nIndex for the last color of the list.
     */
    static oox::drawingml::Color getColorByIndex(const DiagramColorList& rList, sal_Int32 nIndex,
                                                 sal_Int32 nCount);
};

typedef std::map<OUString,DiagramColor> DiagramColorMap;
typedef std::map<svx::diagram::DomMapFlag,cpo::uno::Any> DiagramPRDomMap;

class SmartArtDiagram
{
public:
    explicit SmartArtDiagram();
    explicit SmartArtDiagram(SmartArtDiagram const& rSource);
    explicit SmartArtDiagram(std::u16string_view rLayout, std::u16string_view rData, std::u16string_view rColors, std::u16string_view rQuickstyle);
    ~SmartArtDiagram();
    const OoxDiagramDataPtr& getData() const { return mpData; }
    const DiagramLayoutPtr& getLayout() const { return mpLayout; }

    DiagramQStyleMap& getStyles() { return maStyles; }
    const DiagramQStyleMap& getStyles() const { return maStyles; }
    DiagramColorMap& getColors() { return maColors; }
    const DiagramColorMap& getColors() const { return maColors; }
    void createShapeHierarchyFromModel( const ShapePtr & pShape, bool bCreate );

    oox::core::NamedShapePairs& getDiagramFontHeights() { return maDiagramFontHeights; }

    /// The values a layout has worked out under names of its own, userA to userZ, by the token
    /// of the name. A layout states such a value at one node for the whole of what hangs below
    /// it, and the layout is walked from the top down, so the value stands from where it is
    /// stated until another node states it again.
    std::map<sal_Int32, sal_Int32>& getUserVariables() { return maUserVariables; }
    void syncDiagramFontHeights();

    /// A value under a name of its own, userA and the rest, that a layout states as a part of the
    /// size of a shape levels below the node that states it. The node cannot see that shape when
    /// it runs, so the statement is kept by the token of the name, and the value is read off the
    /// shape when a node below takes the name.
    struct DeferredUserVariable
    {
        OUString msShapeName;
        sal_Int32 mnRefType = 0;
        double mfFactor = 1.0;
    };
    std::map<sal_Int32, DeferredUserVariable>& getDeferredUserVariables()
    {
        return maDeferredUserVariables;
    }

    /// The shapes of a hierarchy that lies sideways which its branch has placed as a whole,
    /// roots, branches and the shapes in them. Each holds its final place and size already.
    std::set<const Shape*>& getLaidOutSideways() { return maLaidOutSideways; }

    /// A spacer in a row whose extent along the row is a part of a shape that a hierarchy
    /// beside it lays out, and so is not known while the row is placed: the row, the spacer
    /// and its place among the row's children when the row was read, the name of that shape
    /// and which of its sides, the part, whether the row runs along x, and the way the row
    /// runs, 1 forward and -1 back.
    struct DeferredSpacer
    {
        ShapePtr mpRow;
        ShapePtr mpSpacer;
        sal_Int32 mnIndex;
        OUString maRefName;
        sal_Int32 mnRefType;
        double mfFactor;
        bool mbAlongX;
        sal_Int32 mnDirection;
    };
    std::vector<DeferredSpacer>& getDeferredSpacers() { return maDeferredSpacers; }

    void setOOXDomValue(svx::diagram::DomMapFlag aDomMapFlag, const cpo::uno::Any& rValue);
    cpo::uno::Any getOOXDomValue(svx::diagram::DomMapFlag aDomMapFlag) const;
    void resetOOXDomValues(svx::diagram::DomMapFlags aDomMapFlags);

    // check if mandatory DiagramDomS exist (or can be created)
    bool checkMinimalDataDoms() const;

    // helpers to write some specific DiagramDoms
    void writeDiagramOOXData(DrawingML& rOriginalDrawingML, cpo::uno::Reference<css::io::XOutputStream>& xOutputStream, std::u16string_view rDrawingRelId) const;
    void writeDiagramReducedOOXData(cpo::uno::Reference<css::io::XOutputStream>& xOutputStream) const;
    void writeDiagramOOXDrawing(DrawingML& rOriginalDrawingML, cpo::uno::Reference<css::io::XOutputStream>& xOutputStream) const;

private:
    // helpers
    cpo::uno::Reference<css::xml::dom::XDocument> convertAndSet(std::u16string_view rDOM, svx::diagram::DomMapFlag aDomMapFlag);
    cpo::uno::Reference<css::xml::dom::XDocument> convertAndSet(std::u16string_view rData, svx::diagram::DomMapFlag aDomMapFlag, bool bAdd);

    // This contains groups of shapes: automatic font size is the same in each group.
    oox::core::NamedShapePairs maDiagramFontHeights;
    std::map<sal_Int32, sal_Int32> maUserVariables;
    std::set<const Shape*> maLaidOutSideways;
    std::map<sal_Int32, DeferredUserVariable> maDeferredUserVariables;
    std::vector<DeferredSpacer> maDeferredSpacers;

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
