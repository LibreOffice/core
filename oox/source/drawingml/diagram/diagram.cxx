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
#include <oox/token/namespaces.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <svx/svdpage.hxx>

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

    rChildren.erase(std::remove_if(rChildren.begin(), rChildren.end(),
                                   [](const ShapePtr& aChild) {
                                       return aChild->getServiceName()
                                                  == "com.sun.star.drawing.GroupShape"
                                              && aChild->getChildren().empty();
                                   }),
                    rChildren.end());

    for (const auto& pChild : rChildren)
    {
        removeUnneededGroupShapes(pChild);
    }
}

void Diagram::addTo( const ShapePtr & pParentShape )
{
    if (pParentShape->getSize().Width == 0 || pParentShape->getSize().Height == 0)
        SAL_WARN("oox.drawingml", "Diagram cannot be correctly laid out. Size: "
            << pParentShape->getSize().Width << "x" << pParentShape->getSize().Height);

    pParentShape->setChildSize(pParentShape->getSize());

    const dgm::Point* pRootPoint = mpData->getRootPoint();
    if (mpLayout->getNode() && pRootPoint)
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
    pBackground->getFillProperties() = *mpData->getFillProperties();
    pBackground->setLocked(true);
    auto& aChildren = pParentShape->getChildren();
    aChildren.insert(aChildren.begin(), pBackground);
}

Diagram::Diagram(const ShapePtr& pShape)
    : mpShape(pShape)
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
    DiagramPtr pDiagram = std::make_shared<Diagram>(pShape);

    DiagramDataPtr pData = std::make_shared<DiagramData>();
    pDiagram->setData( pData );

    DiagramLayoutPtr pLayout = std::make_shared<DiagramLayout>(*pDiagram);
    pDiagram->setLayout( pLayout );

    // data
    if( !rDataModelPath.isEmpty() )
    {
        rtl::Reference< core::FragmentHandler > xRefDataModel(
                new DiagramDataFragmentHandler( rFilter, rDataModelPath, pData ));

        importFragment(rFilter,
                       loadFragment(rFilter,xRefDataModel),
                       "OOXData",
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

    // extLst is present, lets bet on that and ignore the rest of the data from here
    if( pShape->getExtDrawings().empty() )
    {
        // layout
        if( !rLayoutPath.isEmpty() )
        {
            rtl::Reference< core::FragmentHandler > xRefLayout(
                    new DiagramLayoutFragmentHandler( rFilter, rLayoutPath, pLayout ));

            importFragment(rFilter,
                    loadFragment(rFilter,xRefLayout),
                    "OOXLayout",
                    pDiagram,
                    xRefLayout);
        }

        // style
        if( !rQStylePath.isEmpty() )
        {
            rtl::Reference< core::FragmentHandler > xRefQStyle(
                    new DiagramQStylesFragmentHandler( rFilter, rQStylePath, pDiagram->getStyles() ));

            importFragment(rFilter,
                    loadFragment(rFilter,xRefQStyle),
                    "OOXStyle",
                    pDiagram,
                    xRefQStyle);
        }
    }
    else
    {
        // We still want to add the XDocuments to the DiagramDomMap
        DiagramDomMap& rMainDomMap = pDiagram->getDomMap();
        rMainDomMap[OUString("OOXLayout")] = loadFragment(rFilter,rLayoutPath);
        rMainDomMap[OUString("OOXStyle")] = loadFragment(rFilter,rQStylePath);
    }

    // colors
    if( !rColorStylePath.isEmpty() )
    {
        rtl::Reference< core::FragmentHandler > xRefColorStyle(
            new ColorFragmentHandler( rFilter, rColorStylePath, pDiagram->getColors() ));

        importFragment(rFilter,
            loadFragment(rFilter,xRefColorStyle),
            "OOXColor",
            pDiagram,
            xRefColorStyle);
    }

    if( !pData->getExtDrawings().empty() )
    {
        const DiagramColorMap::const_iterator aColor = pDiagram->getColors().find("node0");
        if( aColor != pDiagram->getColors().end() && !aColor->second.maTextFillColors.empty())
        {
            // TODO(F1): well, actually, there might be *several* color
            // definitions in it, after all it's called list.
            pShape->setFontRefColorForNodes(DiagramColor::getColorByIndex(aColor->second.maTextFillColors, -1));
        }
    }

    // collect data, init maps
    pData->build();

    // diagram loaded. now lump together & attach to shape
    pDiagram->addTo(pShape);
    pShape->setDiagramData(pData);
    pShape->setDiagramDoms(pDiagram->getDomsAsPropertyValues());
}

void loadDiagram(ShapePtr const& pShape,
                 DiagramDataPtr pDiagramData,
                 const uno::Reference<xml::dom::XDocument>& layoutDom,
                 const uno::Reference<xml::dom::XDocument>& styleDom,
                 const uno::Reference<xml::dom::XDocument>& colorDom,
                 core::XmlFilterBase& rFilter)
{
    DiagramPtr pDiagram = std::make_shared<Diagram>(pShape);

    pDiagram->setData(pDiagramData);

    DiagramLayoutPtr pLayout = std::make_shared<DiagramLayout>(*pDiagram);
    pDiagram->setLayout(pLayout);

    // layout
    if (layoutDom.is())
    {
        rtl::Reference<core::FragmentHandler> xRefLayout(
            new DiagramLayoutFragmentHandler(rFilter, OUString(), pLayout));

        importFragment(rFilter, layoutDom, "OOXLayout", pDiagram, xRefLayout);
    }

    // style
    if (styleDom.is())
    {
        rtl::Reference<core::FragmentHandler> xRefQStyle(
            new DiagramQStylesFragmentHandler(rFilter, OUString(), pDiagram->getStyles()));

        importFragment(rFilter, styleDom, "OOXStyle", pDiagram, xRefQStyle);
    }

    // colors
    if (colorDom.is())
    {
        rtl::Reference<core::FragmentHandler> xRefColorStyle(
            new ColorFragmentHandler(rFilter, OUString(), pDiagram->getColors()));

        importFragment(rFilter, colorDom, "OOXColor", pDiagram, xRefColorStyle);
    }

    // diagram loaded. now lump together & attach to shape
    pDiagram->addTo(pShape);
}

void reloadDiagram(SdrObject* pObj, core::XmlFilterBase& rFilter)
{
    DiagramDataPtr pDiagramData = std::dynamic_pointer_cast<DiagramData>(pObj->GetDiagramData());
    if (!pDiagramData)
        return;

    pObj->getChildrenOfSdrObject()->ClearSdrObjList();

    uno::Reference<css::drawing::XShape> xShape(pObj->getUnoShape(), uno::UNO_QUERY_THROW);
    uno::Reference<beans::XPropertySet> xPropSet(xShape, uno::UNO_QUERY_THROW);

    uno::Reference<xml::dom::XDocument> layoutDom;
    uno::Reference<xml::dom::XDocument> styleDom;
    uno::Reference<xml::dom::XDocument> colorDom;

    // retrieve the doms from the GrabBag
    uno::Sequence<beans::PropertyValue> propList;
    xPropSet->getPropertyValue(UNO_NAME_MISC_OBJ_INTEROPGRABBAG) >>= propList;
    for (const auto& rProp : std::as_const(propList))
    {
        OUString propName = rProp.Name;
        if (propName == "OOXLayout")
            rProp.Value >>= layoutDom;
        else if (propName == "OOXStyle")
            rProp.Value >>= styleDom;
        else if (propName == "OOXColor")
            rProp.Value >>= colorDom;
    }

    ShapePtr pShape = std::make_shared<Shape>();
    pShape->setDiagramType();
    pShape->setSize(awt::Size(xShape->getSize().Width * EMU_PER_HMM,
                              xShape->getSize().Height * EMU_PER_HMM));

    loadDiagram(pShape, pDiagramData, layoutDom, styleDom, colorDom, rFilter);

    uno::Reference<drawing::XShapes> xShapes(xShape, uno::UNO_QUERY_THROW);
    basegfx::B2DHomMatrix aTransformation;
    aTransformation.translate(xShape->getPosition().X * EMU_PER_HMM,
                              xShape->getPosition().Y * EMU_PER_HMM);
    for (auto const& child : pShape->getChildren())
        child->addShape(rFilter, rFilter.getCurrentTheme(), xShapes, aTransformation, pShape->getFillProperties());
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
