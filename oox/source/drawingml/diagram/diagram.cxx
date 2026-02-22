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
#include <com/sun/star/xml/dom/XDocumentBuilder.hpp>
#include <com/sun/star/xml/dom/DocumentBuilder.hpp>
#include <sal/log.hxx>
#include <editeng/unoprnms.hxx>
#include <drawingml/fillproperties.hxx>
#include <drawingml/customshapeproperties.hxx>
#include <o3tl/unit_conversion.hxx>
#include <oox/drawingml/theme.hxx>
#include <oox/token/namespaces.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <svx/svdpage.hxx>
#include <oox/ppt/pptimport.hxx>
#include <comphelper/xmltools.hxx>
#include "diagramlayoutatoms.hxx"
#include "layoutatomvisitors.hxx"
#include "diagramfragmenthandler.hxx"
#include <comphelper/processfactory.hxx>
#include <com/sun/star/io/TempFile.hpp>
#include <oox/export/drawingml.hxx>

#ifdef DBG_UTIL
#include <osl/file.hxx>
#include <o3tl/environment.hxx>
#include <tools/stream.hxx>
#include <unotools/streamwrap.hxx>
#include <comphelper/storagehelper.hxx>
#include <com/sun/star/embed/XRelationshipAccess.hpp>
#endif

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


void SmartArtDiagram::createShapeHierarchyFromModel( const ShapePtr & pParentShape, bool bCreate )
{
    if (pParentShape->getSize().Width == 0 || pParentShape->getSize().Height == 0)
        SAL_WARN("oox.drawingml", "SmartArtDiagram cannot be correctly laid out. Size: "
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
    if (mpData->getBackgroundShapeFillProperties())
        pBackground->getFillProperties() = *mpData->getBackgroundShapeFillProperties();
    // MoveProtect/SizeProtect..? Keep for now, but mnay be removed
    // when IA needs change - the Diagram will be a closed GroupObject.
    // If it gets 'broken' (un-grouped) the BGShape will keep that attributes,
    // despite main reason to break that Diagram is probably to edit it.
    pBackground->setLocked(true);

    // create and set ModelID for BackgroundShape to allow later association
    getData()->setBackgroundShapeModelID(OStringToOUString(comphelper::xml::generateGUIDString(), RTL_TEXTENCODING_UTF8));
    pBackground->setDiagramDataModelID(getData()->getBackgroundShapeModelID());

    auto& aChildren = pParentShape->getChildren();
    aChildren.insert(aChildren.begin(), pBackground);
}

SmartArtDiagram::SmartArtDiagram()
: maDiagramFontHeights()
, mpData()
, mpLayout()
, maStyles()
, maColors()
, maDiagramPRDomMap()
{
}

SmartArtDiagram::SmartArtDiagram(SmartArtDiagram const& rSource)
: maDiagramFontHeights()
, mpData(rSource.mpData ? new DiagramData_oox(*rSource.mpData) : nullptr)
, mpLayout(rSource.mpLayout)
, maStyles(rSource.maStyles)
, maColors(rSource.maColors)
, maDiagramPRDomMap(rSource.maDiagramPRDomMap)
{
}

uno::Any SmartArtDiagram::getOOXDomValue(svx::diagram::DomMapFlag aDomMapFlag) const
{
    const DiagramPRDomMap::const_iterator aHit = maDiagramPRDomMap.find(aDomMapFlag);

    if (aHit != maDiagramPRDomMap.end())
        return aHit->second;

    return uno::Any();
}

void SmartArtDiagram::setOOXDomValue(svx::diagram::DomMapFlag aDomMapFlag, const uno::Any& rValue)
{
    maDiagramPRDomMap[aDomMapFlag] = rValue;
}

void SmartArtDiagram::resetOOXDomValues(svx::diagram::DomMapFlags aDomMapFlags)
{
    for (const auto& rEntry : aDomMapFlags)
    {
        maDiagramPRDomMap.erase(rEntry);

        if (maDiagramPRDomMap.empty())
            return;
    }
}

bool SmartArtDiagram::checkMinimalDataDoms() const
{
    // check if re-creation is activated
    static bool bActivateAdvancedDiagramFeatures(nullptr != std::getenv("ACTIVATE_ADVANCED_DIAGRAM_FEATURES"));

    if (!bActivateAdvancedDiagramFeatures && maDiagramPRDomMap.end() == maDiagramPRDomMap.find(svx::diagram::DomMapFlag::OOXData))
        return false;

    if (maDiagramPRDomMap.end() == maDiagramPRDomMap.find(svx::diagram::DomMapFlag::OOXLayout))
        return false;

    if (maDiagramPRDomMap.end() == maDiagramPRDomMap.find(svx::diagram::DomMapFlag::OOXStyle))
        return false;

    if (maDiagramPRDomMap.end() == maDiagramPRDomMap.find(svx::diagram::DomMapFlag::OOXColor))
        return false;

    return true;
}

void SmartArtDiagram::writeDiagramOOXData(DrawingML& rOriginalDrawingML, uno::Reference<io::XOutputStream>& xOutputStream, std::u16string_view rDrawingRelId) const
{
    if (!xOutputStream)
        return;

    // re-create OOXData DomFile from model data
    sax_fastparser::FSHelperPtr aFS = std::make_shared<sax_fastparser::FastSerializerHelper>(xOutputStream, true);
    getData()->writeDiagramData(rOriginalDrawingML, aFS, rDrawingRelId);

    // this call is *important*, without it xDocBuilder->parse below fails and some strange
    // and wrong assertion gets thrown in ~FastSerializerHelper that  shall get called
    xOutputStream->closeOutput();

#ifdef DBG_UTIL
    uno::Reference< embed::XRelationshipAccess > xRelations( xOutputStream, uno::UNO_QUERY );
    if( xRelations.is() )
    {
        const uno::Sequence<uno::Sequence<beans::StringPair>> aSeqs = xRelations->getAllRelationships();
        for (const uno::Sequence<beans::StringPair>& aSeq : aSeqs)
        {
            SAL_INFO("oox", "RelationData:");
            for (const beans::StringPair& aPair : aSeq)
                SAL_INFO("oox", "  Key: " << aPair.First << ", Value: " << aPair.Second);
        }
    }

    const OUString env(o3tl::getEnvironment(u"DIAGRAM_DUMP_PATH"_ustr));
    if(!env.isEmpty())
    {
        OUString url;
        ::osl::FileBase::getFileURLFromSystemPath(env, url);
        SvFileStream aOutStream(url + "data_T.xml", StreamMode::WRITE|StreamMode::TRUNC);
        uno::Reference<io::XStream> xOutStream(new utl::OStreamWrapper(aOutStream));
        uno::Reference<io::XStream> xInStream(xOutputStream, uno::UNO_QUERY);
        comphelper::OStorageHelper::CopyInputToOutput(xInStream->getInputStream(), xOutStream->getOutputStream());
    }
#endif
}

void SmartArtDiagram::writeDiagramOOXDrawing(DrawingML& rOriginalDrawingML, uno::Reference<io::XOutputStream>& xOutputStream) const
{
    if (!xOutputStream)
        return;

    // re-create OOXDrawing DomFile from model data
    SAL_INFO("oox", "DiagramReCreate: creating DomMapFlag::OOXDrawing");
    sax_fastparser::FSHelperPtr aFS = std::make_shared<sax_fastparser::FastSerializerHelper>(xOutputStream, true);
    getData()->writeDiagramReplacement(rOriginalDrawingML, aFS);

    // this call is *important*, without it xDocBuilder->parse below fails and some strange
    // and wrong assertion gets thrown in ~FastSerializerHelper that  shall get called
    xOutputStream->closeOutput();

#ifdef DBG_UTIL
    uno::Reference< embed::XRelationshipAccess > xRelations( xOutputStream, uno::UNO_QUERY );
    if( xRelations.is() )
    {
        const uno::Sequence<uno::Sequence<beans::StringPair>> aSeqs = xRelations->getAllRelationships();
        for (const uno::Sequence<beans::StringPair>& aSeq : aSeqs)
        {
            SAL_INFO("oox", "RelationDrawing:");
            for (const beans::StringPair& aPair : aSeq)
                SAL_INFO("oox", "  Key: " << aPair.First << ", Value: " << aPair.Second);
        }
    }

    const OUString env(o3tl::getEnvironment(u"DIAGRAM_DUMP_PATH"_ustr));
    if(!env.isEmpty())
    {
        OUString url;
        ::osl::FileBase::getFileURLFromSystemPath(env, url);
        SvFileStream aOutStream(url + "drawing_T.xml", StreamMode::WRITE|StreamMode::TRUNC);
        uno::Reference<io::XStream> xOutStream(new utl::OStreamWrapper(aOutStream));
        uno::Reference<io::XStream> xInStream(xOutputStream, uno::UNO_QUERY);
        comphelper::OStorageHelper::CopyInputToOutput(xInStream->getInputStream(), xOutStream->getOutputStream());
    }
#endif
}

using ShapePairs
    = std::map<std::shared_ptr<drawingml::Shape>, uno::Reference<drawing::XShape>>;

void SmartArtDiagram::syncDiagramFontHeights()
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
                     svx::diagram::DomMapFlag aDomMapFlag,
                     const DiagramPtr& pDiagram,
                     const rtl::Reference< core::FragmentHandler >& rxHandler )
{
    pDiagram->setOOXDomValue(aDomMapFlag, uno::Any(rXDom));

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
    DiagramPtr pDiagram = std::make_shared<SmartArtDiagram>();

    OoxDiagramDataPtr pData = std::make_shared<DiagramData_oox>();
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
                           svx::diagram::DomMapFlag::OOXData,
                           pDiagram,
                           xRefDataModel);

            uno::Sequence<uno::Sequence<uno::Any>> aDataImageRelsMap(
                pShape->resolveRelationshipsOfTypeFromOfficeDoc(
                    rFilter, xRefDataModel->getFragmentPath(), u"image"));
            uno::Sequence<uno::Sequence<uno::Any>> aDataHlinkRelsMap(
                pShape->resolveRelationshipsOfTypeFromOfficeDoc(
                    rFilter, xRefDataModel->getFragmentPath(), u"hlink"));

            pDiagram->setOOXDomValue(svx::diagram::DomMapFlag::OOXDataImageRels,
                                     uno::Any(aDataImageRelsMap));
            pDiagram->setOOXDomValue(svx::diagram::DomMapFlag::OOXDataHlinkRels,
                                     uno::Any(aDataHlinkRelsMap));

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
        // DiagramHelper_oox::reLayout to re-create the oox::Shape(s) for the
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
                    new DiagramLayoutFragmentHandler( rFilter, rLayoutPath,
                                                      std::move(pLayout) ));

            importFragment(rFilter,
                    loadFragment(rFilter,xRefLayout),
                    svx::diagram::DomMapFlag::OOXLayout,
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
                    svx::diagram::DomMapFlag::OOXStyle,
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
                svx::diagram::DomMapFlag::OOXColor,
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
#ifdef DBG_UTIL
        pData->dump();
#endif

        // diagram loaded. now lump together & attach to shape
        // create own geometry if extLst is not present (no geometric
        // representation is available in file). This will - if false -
        // just create the BackgroundShape.
        // NOTE: Need to use pShape->getExtDrawings() here, this is the
        // already *filtered* version, see usage of DiagramShapeCounter
        // above. Moving to local bool, there might more conditions show
        // up
        static bool bIgnoreExtDrawings(nullptr != std::getenv("DIAGRAM_IGNORE_EXTDRAWINGS"));
        const bool bCreate(bIgnoreExtDrawings || pShape->getExtDrawings().empty());
        pDiagram->createShapeHierarchyFromModel(pShape, bCreate);

        // Get the oox::Theme definition and - if available - move/secure the
        // original ImportData directly to the Diagram ModelData
        std::shared_ptr<::oox::drawingml::Theme> aTheme(rFilter.getCurrentThemePtr());
        if(aTheme)
            pData->setThemeDocument(aTheme->getFragment()); //getTempFile());

        // Prepare support for the advanced DiagramHelper using Diagram & Theme data
        pShape->prepareDiagramHelper(pDiagram, rFilter.getCurrentThemePtr());
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
