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

#include <oox/drawingml/diagram/diagram.hxx>
#include "diagram.hxx"
#include <com/sun/star/awt/Point.hpp>
#include <com/sun/star/awt/Size.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/drawing/XShape.hpp>
#include <com/sun/star/drawing/XShapes.hpp>
#include <com/sun/star/xml/dom/XDocument.hpp>
#include <com/sun/star/xml/sax/XFastSAXSerializable.hpp>
#include <sal/log.hxx>
#include <editeng/unoprnms.hxx>
#include <drawingml/fillproperties.hxx>
#include <drawingml/customshapeproperties.hxx>
#include <o3tl/unit_conversion.hxx>
#include <oox/token/namespaces.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <svx/svdpage.hxx>
#include <oox/ppt/pptimport.hxx>
#include <comphelper/xmltools.hxx>

#include "diagramlayoutatoms.hxx"
#include "layoutatomvisitors.hxx"
#include "diagramfragmenthandler.hxx"

using namespace ::com::sun::star;

namespace oox::drawingml {

static void sortChildrenByZOrder(const ShapePtr& pShape)
{
    std::vector<ShapePtr>& rChildren = pShape->getChildren();

    // Offset the children from their default z-order stacking, if necessary.
    for (size_t i = 0; i < rChildren.size(); ++i)
        rChildren[i]->setZOrder(i);

    for (size_t i = 0; i < rChildren.size(); ++i)
    {
        const ShapePtr& pChild = rChildren[i];
        sal_Int32 nZOrderOff = pChild->getZOrderOff();
        if (nZOrderOff <= 0)
            continue;

        // Increase my ZOrder by nZOrderOff.
        pChild->setZOrder(pChild->getZOrder() + nZOrderOff);
        pChild->setZOrderOff(0);

        for (sal_Int32 j = 0; j < nZOrderOff; ++j)
        {
            size_t nIndex = i + j + 1;
            if (nIndex >= rChildren.size())
                break;

            // Decrease the ZOrder of the next nZOrderOff elements by one.
            const ShapePtr& pNext = rChildren[nIndex];
            pNext->setZOrder(pNext->getZOrder() - 1);
        }
    }

    // Now that the ZOrders are adjusted, sort the children.
    std::sort(rChildren.begin(), rChildren.end(),
              [](const ShapePtr& a, const ShapePtr& b) { return a->getZOrder() < b->getZOrder(); });

    // Apply also for children.
    for (const auto& rChild : rChildren)
        sortChildrenByZOrder(rChild);
}

/// Removes empty group shapes, now that their spacing influenced the layout.
static void removeUnneededGroupShapes(const ShapePtr& pShape)
{
    std::vector<ShapePtr>& rChildren = pShape->getChildren();

    std::erase_if(rChildren,
                                   [](const ShapePtr& aChild) {
                                       return aChild->getServiceName()
                                                  == "com.sun.star.drawing.GroupShape"
                                              && aChild->getChildren().empty();
                                   });

    for (const auto& pChild : rChildren)
    {
        removeUnneededGroupShapes(pChild);
    }
}


void Diagram::addTo( const ShapePtr & pParentShape, bool bCreate )
{
    if (pParentShape->getSize().Width == 0 || pParentShape->getSize().Height == 0)
        SAL_WARN("oox.drawingml", "Diagram cannot be correctly laid out. Size: "
            << pParentShape->getSize().Width << "x" << pParentShape->getSize().Height);

    pParentShape->setChildSize(pParentShape->getSize());

    const svx::diagram::Point* pRootPoint = mpData->getRootPoint();
    if (bCreate && mpLayout->getNode() && pRootPoint)
    {
        // create Shape hierarchy
        ShapeCreationVisitor aCreationVisitor(*this, pRootPoint, pParentShape);
        mpLayout->getNode()->setExistingShape(pParentShape);
        mpLayout->getNode()->accept(aCreationVisitor);

        // layout shapes - now all shapes are created
        ShapeLayoutingVisitor aLayoutingVisitor(*this, pRootPoint);
        mpLayout->getNode()->accept(aLayoutingVisitor);

        sortChildrenByZOrder(pParentShape);
        removeUnneededGroupShapes(pParentShape);
    }

    ShapePtr pBackground = std::make_shared<Shape>("com.sun.star.drawing.CustomShape");
    pBackground->setSubType(XML_rect);
    pBackground->getCustomShapeProperties()->setShapePresetType(XML_rect);
    pBackground->setSize(pParentShape->getSize());
    pBackground->getFillProperties() = *mpData->getBackgroundShapeFillProperties();
    pBackground->setLocked(true);

    // create and set ModelID for BackgroundShape to allow later association
    getData()->setBackgroundShapeModelID(OStringToOUString(comphelper::xml::generateGUIDString(), RTL_TEXTENCODING_UTF8));
    pBackground->setDiagramDataModelID(getData()->getBackgroundShapeModelID());

    auto& aChildren = pParentShape->getChildren();
    aChildren.insert(aChildren.begin(), pBackground);
}

Diagram::Diagram()
: maDiagramFontHeights()
{
}

uno::Sequence<beans::PropertyValue> Diagram::getDomsAsPropertyValues() const
{
    sal_Int32 length = maMainDomMap.size();

    if (maDataRelsMap.hasElements())
        ++length;

    uno::Sequence<beans::PropertyValue> aValue(length);
    beans::PropertyValue* pValue = aValue.getArray();
    for (auto const& mainDom : maMainDomMap)
    {
        pValue->Name = mainDom.first;
        pValue->Value <<= mainDom.second;
        ++pValue;
    }

    if (maDataRelsMap.hasElements())
    {
        pValue->Name = "OOXDiagramDataRels";
        pValue->Value <<= maDataRelsMap;
        ++pValue;
    }

    return aValue;
}

using ShapePairs
    = std::map<std::shared_ptr<drawingml::Shape>, css::uno::Reference<css::drawing::XShape>>;

void Diagram::syncDiagramFontHeights()
{
    // Each name represents a group of shapes, for which the font height should have the same
    // scaling.
    for (const auto& rNameAndPairs : maDiagramFontHeights)
    {
        // Find out the minimum scale within this group.
        const ShapePairs& rShapePairs = rNameAndPairs.second;
        double fMinFontScale = 100.0;
        double fMinSpacingScale = 100.0;
        for (const auto& rShapePair : rShapePairs)
        {
            uno::Reference<beans::XPropertySet> xPropertySet(rShapePair.second, uno::UNO_QUERY);
            if (xPropertySet.is())
            {
                double fFontScale = 0.0;
                double fSpacingScale = 0.0;
                xPropertySet->getPropertyValue(u"TextFitToSizeFontScale"_ustr) >>= fFontScale;
                xPropertySet->getPropertyValue(u"TextFitToSizeSpacingScale"_ustr) >>= fSpacingScale;

                if (fFontScale > 0 && fSpacingScale > 0
                    && (fFontScale < fMinFontScale || (fFontScale == fMinFontScale && fSpacingScale < fMinSpacingScale)))
                {
                    fMinFontScale = fFontScale;
                    fMinSpacingScale = fSpacingScale;
                }
            }
        }

        // Set that minimum scale for all members of the group.
        if (fMinFontScale < 100.0 || fMinSpacingScale < 100.0)
        {
            for (const auto& rShapePair : rShapePairs)
            {
                uno::Reference<beans::XPropertySet> xPropertySet(rShapePair.second, uno::UNO_QUERY);
                if (xPropertySet.is())
                {
                    xPropertySet->setPropertyValue(u"TextFitToSizeFontScale"_ustr, uno::Any(fMinFontScale));
                    xPropertySet->setPropertyValue(u"TextFitToSizeSpacingScale"_ustr, uno::Any(fMinSpacingScale));
                }
            }
        }
    }

    // no longer needed after processing
    maDiagramFontHeights.clear();
}

static uno::Reference<xml::dom::XDocument> loadFragment(
    core::XmlFilterBase& rFilter,
    const OUString& rFragmentPath )
{
    // load diagramming fragments into DOM representation, that later
    // gets serialized back to SAX events and parsed
    return rFilter.importFragment( rFragmentPath );
}

static uno::Reference<xml::dom::XDocument> loadFragment(
    core::XmlFilterBase& rFilter,
    const rtl::Reference< core::FragmentHandler >& rxHandler )
{
    return loadFragment( rFilter, rxHandler->getFragmentPath() );
}

static void importFragment( core::XmlFilterBase& rFilter,
                     const uno::Reference<xml::dom::XDocument>& rXDom,
                     const OUString& rDocName,
                     const DiagramPtr& pDiagram,
                     const rtl::Reference< core::FragmentHandler >& rxHandler )
{
    DiagramDomMap& rMainDomMap = pDiagram->getDomMap();
    rMainDomMap[rDocName] = rXDom;

    uno::Reference<xml::sax::XFastSAXSerializable> xSerializer(
        rXDom, uno::UNO_QUERY_THROW);

    // now serialize DOM tree into internal data structures
    rFilter.importFragment( rxHandler, xSerializer );
}

namespace
{
/**
 * A fragment handler that just counts the number of <dsp:sp> elements in a
 * fragment.
 */
class DiagramShapeCounter : public oox::core::FragmentHandler2
{
public:
    DiagramShapeCounter(oox::core::XmlFilterBase& rFilter, const OUString& rFragmentPath,
                        sal_Int32& nCounter);
    oox::core::ContextHandlerRef onCreateContext(sal_Int32 nElement,
                                                 const AttributeList& rAttribs) override;

private:
    sal_Int32& m_nCounter;
};

DiagramShapeCounter::DiagramShapeCounter(oox::core::XmlFilterBase& rFilter,
                                         const OUString& rFragmentPath, sal_Int32& nCounter)
    : FragmentHandler2(rFilter, rFragmentPath)
    , m_nCounter(nCounter)
{
}

oox::core::ContextHandlerRef DiagramShapeCounter::onCreateContext(sal_Int32 nElement,
                                                                  const AttributeList& /*rAttribs*/)
{
    switch (nElement)
    {
        case DSP_TOKEN(drawing):
        case DSP_TOKEN(spTree):
            return this;
        case DSP_TOKEN(sp):
            ++m_nCounter;
            break;
        default:
            break;
    }

    return nullptr;
}
}

void loadDiagram( ShapePtr const & pShape,
                  core::XmlFilterBase& rFilter,
                  const OUString& rDataModelPath,
                  const OUString& rLayoutPath,
                  const OUString& rQStylePath,
                  const OUString& rColorStylePath,
                  const oox::core::Relations& rRelations )
{
    DiagramPtr pDiagram = std::make_shared<Diagram>();

    OoxDiagramDataPtr pData = std::make_shared<DiagramData>();
    pDiagram->setData( pData );

    DiagramLayoutPtr pLayout = std::make_shared<DiagramLayout>(*pDiagram);
    pDiagram->setLayout( pLayout );

    try
    {
        // set DiagramFontHeights at filter
        rFilter.setDiagramFontHeights(&pDiagram->getDiagramFontHeights());

        // data
        if( !rDataModelPath.isEmpty() )
        {
            rtl::Reference< core::FragmentHandler > xRefDataModel(
                    new DiagramDataFragmentHandler( rFilter, rDataModelPath, pData ));

            importFragment(rFilter,
                           loadFragment(rFilter,xRefDataModel),
                           u"OOXData"_ustr,
                           pDiagram,
                           xRefDataModel);

            pDiagram->getDataRelsMap() = pShape->resolveRelationshipsOfTypeFromOfficeDoc( rFilter,
                    xRefDataModel->getFragmentPath(), u"image" );

            // Pass the info to pShape
            for (auto const& extDrawing : pData->getExtDrawings())
            {
                OUString aFragmentPath = rRelations.getFragmentPathFromRelId(extDrawing);
                // Ignore RelIds which don't resolve to a fragment path.
                if (aFragmentPath.isEmpty())
                    continue;

                sal_Int32 nCounter = 0;
                rtl::Reference<core::FragmentHandler> xCounter(
                    new DiagramShapeCounter(rFilter, aFragmentPath, nCounter));
                rFilter.importFragment(xCounter);
                // Ignore ext drawings which don't actually have any shapes.
                if (nCounter == 0)
                    continue;

                pShape->addExtDrawingRelId(extDrawing);
            }
        }

        // Layout: always import to allow editing in the future. It's needed for
        // AdvancedDiagramHelper::reLayout to re-create the oox::Shape(s) for the
        // model. Without importing these the diagram model will be not complete.
        // NOTE: This also adds the DomMaps to rMainDomMap, so the lines
        //     DiagramDomMap& rMainDomMap = pDiagram->getDomMap();
        //     rMainDomMap[u"OOXLayout"_ustr] = loadFragment(rFilter,rLayoutPath);
        //     rMainDomMap[u"OOXStyle"_ustr] = loadFragment(rFilter,rQStylePath);
        // which were used before if !pShape->getExtDrawings().empty() are not
        // needed
        if (!rLayoutPath.isEmpty())
        {
            rtl::Reference< core::FragmentHandler > xRefLayout(
                    new DiagramLayoutFragmentHandler( rFilter, rLayoutPath, pLayout ));

            importFragment(rFilter,
                    loadFragment(rFilter,xRefLayout),
                    u"OOXLayout"_ustr,
                    pDiagram,
                    xRefLayout);
        }

        // Style: same as for Layout (above)
        if( !rQStylePath.isEmpty() )
        {
            rtl::Reference< core::FragmentHandler > xRefQStyle(
                    new DiagramQStylesFragmentHandler( rFilter, rQStylePath, pDiagram->getStyles() ));

            importFragment(rFilter,
                    loadFragment(rFilter,xRefQStyle),
                    u"OOXStyle"_ustr,
                    pDiagram,
                    xRefQStyle);
        }

        // colors
        if( !rColorStylePath.isEmpty() )
        {
            rtl::Reference< core::FragmentHandler > xRefColorStyle(
                new ColorFragmentHandler( rFilter, rColorStylePath, pDiagram->getColors() ));

            importFragment(rFilter,
                loadFragment(rFilter,xRefColorStyle),
                u"OOXColor"_ustr,
                pDiagram,
                xRefColorStyle);
        }

        if( !pData->getExtDrawings().empty() )
        {
            const DiagramColorMap::const_iterator aColor = pDiagram->getColors().find(u"node0"_ustr);
            if( aColor != pDiagram->getColors().end() && !aColor->second.maTextFillColors.empty())
            {
                // TODO(F1): well, actually, there might be *several* color
                // definitions in it, after all it's called list.
                pShape->setFontRefColorForNodes(DiagramColor::getColorByIndex(aColor->second.maTextFillColors, -1));
            }
        }

        // collect data, init maps
        // for Diagram import, do - for now - NOT clear all oox::drawingml::Shape
        pData->buildDiagramDataModel(false);

        // diagram loaded. now lump together & attach to shape
        // create own geometry if extLst is not present (no geometric
        // representation is available in file). This will - if false -
        // just create the BackgroundShape.
        // NOTE: Need to use pShape->getExtDrawings() here, this is the
        // already *filtered* version, see usage of DiagramShapeCounter
        // above. Moving to local bool, there might more coditions show
        // up
        const bool bCreate(pShape->getExtDrawings().empty());
        pDiagram->addTo(pShape, bCreate);
        pShape->setDiagramDoms(pDiagram->getDomsAsPropertyValues());

        // Get the oox::Theme definition and - if available - move/secure the
        // original ImportData directly to the Diagram ModelData
        std::shared_ptr<::oox::drawingml::Theme> aTheme(rFilter.getCurrentThemePtr());
        if(aTheme)
            pData->setThemeDocument(aTheme->getFragment()); //getTempFile());

        // Prepare support for the advanced DiagramHelper using Diagram & Theme data
        pShape->prepareDiagramHelper(pDiagram, rFilter.getCurrentThemePtr(), bCreate);
    }
    catch (...)
    {
        // unset DiagramFontHeights at filter if there was a failure
        // to avoid dangling pointer
        rFilter.setDiagramFontHeights(nullptr);
        throw;
    }
}

const oox::drawingml::Color&
DiagramColor::getColorByIndex(const std::vector<oox::drawingml::Color>& rColors, sal_Int32 nIndex)
{
    assert(!rColors.empty());
    if (nIndex == -1)
    {
        return rColors[rColors.size() - 1];
    }

    return rColors[nIndex % rColors.size()];
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
