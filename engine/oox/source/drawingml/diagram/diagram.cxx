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
#include <drawingml/lineproperties.hxx>
#include <drawingml/customshapeproperties.hxx>
#include <o3tl/unit_conversion.hxx>
#include <algorithm>
#include <cmath>
#include <oox/drawingml/theme.hxx>
#include <oox/token/namespaces.hxx>
#include <basegfx/color/bcolortools.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <cmath>
#include <basegfx/numeric/ftools.hxx>
#include <svx/svdpage.hxx>
#include <oox/ppt/pptimport.hxx>
#include <comphelper/xmltools.hxx>
#include "diagramlayoutatoms.hxx"
#include "layoutatomvisitors.hxx"
#include "diagramfragmenthandler.hxx"
#include <comphelper/processfactory.hxx>
#include <com/sun/star/io/TempFile.hpp>
#include <oox/export/drawingml.hxx>
#include <oox/export/shapes.hxx>

#include <com/sun/star/xml/sax/XSAXSerializable.hpp>
#include <com/sun/star/xml/sax/Writer.hpp>
#include <oox/core/fastparser.hxx>
#include <unotools/streamwrap.hxx>
#include <tools/stream.hxx>

#ifdef DBG_UTIL
#include <osl/file.hxx>
#include <o3tl/environment.hxx>
#include <tools/stream.hxx>
#include <unotools/streamwrap.hxx>
#include <comphelper/storagehelper.hxx>
#include <com/sun/star/embed/XRelationshipAccess.hpp>
#endif

using namespace ::com::sun::star;
using namespace ::cpo;

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

// A layout turns a shape by the rot of its dgm:shape, 73 degrees for the arrow of a descending
// process, and the box the layout gave it is the box the turned shape is to fill. The shape
// itself is smaller than that box: turned by the angle, its own width and height reach the box
// along both axes, W = w cos + h sin and H = w sin + h cos. The shape gets that own size, the
// middle stays where it was, and the angle becomes a plain rotation of the shape. A right angle
// changes nothing but which side is which, so the turn before the sizing does the same there and
// stays; near 45 degrees the two equations say the same thing, and the shape is taken square.
static void unturnRotatedShapes(const ShapePtr& pShape)
{
    for (const ShapePtr& pChild : pShape->getChildren())
    {
        unturnRotatedShapes(pChild);

        // the angle is in 60000ths of a degree, as everywhere in the format
        const sal_Int32 nTurn(pChild->getDiagramRotation());
        if (nTurn == 0 || nTurn % (90 * 60000) == 0)
            continue;

        const double fAngle(basegfx::deg2rad<60000>(nTurn));
        const double fCos(std::abs(cos(fAngle)));
        const double fSin(std::abs(sin(fAngle)));
        const awt::Size aBox(pChild->getSize());
        double fWidth(0.0);
        double fHeight(0.0);
        const double fApart(fCos * fCos - fSin * fSin);
        if (std::abs(fApart) < 0.05)
            fWidth = fHeight = aBox.Width / (fCos + fSin);
        else
        {
            fWidth = (aBox.Width * fCos - aBox.Height * fSin) / fApart;
            fHeight = (aBox.Height * fCos - aBox.Width * fSin) / fApart;
        }
        if (fWidth <= 1.0 || fHeight <= 1.0)
            continue;

        const awt::Size aOwn(static_cast<sal_Int32>(fWidth), static_cast<sal_Int32>(fHeight));
        awt::Point aAt(pChild->getPosition());
        aAt.X += (aBox.Width - aOwn.Width) / 2;
        aAt.Y += (aBox.Height - aOwn.Height) / 2;
        pChild->setPosition(aAt);
        pChild->setSize(aOwn);
        pChild->setChildSize(aOwn);
        pChild->setRotation(pChild->getRotation() + nTurn);
        pChild->setDiagramRotation(0);
    }
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

    // The names the layout settles, the statements it keeps for a shape levels down and the
    // shapes a sideways branch places as a whole all describe the tree this run builds. A run
    // starts with none of them, so a Diagram laid out a second time is measured afresh.
    maUserVariables.clear();
    maDeferredUserVariables.clear();
    maLaidOutSideways.clear();
    maDeferredSpacers.clear();

    const rtl::Reference<svx::diagram::Point> xRootPoint(mpData->getRootPoint());
    if (bCreate && mpLayout->getNode() && xRootPoint.is())
    {
        // create Shape hierarchy
        ShapeCreationVisitor aCreationVisitor(*this, xRootPoint, pParentShape);
        mpLayout->getNode()->setExistingShape(pParentShape);
        mpLayout->getNode()->accept(aCreationVisitor);

        // layout shapes - now all shapes are created
        ShapeLayoutingVisitor aLayoutingVisitor(*this, xRootPoint);
        mpLayout->getNode()->accept(aLayoutingVisitor);
        settleNamedConnectors(*this);
        settleDeferredSpacers(*this);
        sortChildrenByZOrder(pParentShape);
        unturnRotatedShapes(pParentShape);
        removeUnneededGroupShapes(pParentShape);
        centreDrawnShapes(*this, pParentShape);
        settleUprightText(*this);
    }

    ShapePtr pBackground = std::make_shared<Shape>(u"com.sun.star.drawing.CustomShape"_ustr);
    pBackground->setSubType(XML_rect);
    pBackground->getCustomShapeProperties()->setShapePresetType(XML_rect);
    pBackground->setSize(pParentShape->getSize());
    if (mpData->getBackgroundShapeFillProperties())
        pBackground->getFillProperties() = *mpData->getBackgroundShapeFillProperties();
    if (mpData->getBackgroundShapeLineProperties())
        pBackground->getLineProperties() = *mpData->getBackgroundShapeLineProperties();

    // create BackgroundShape, use empty string for identification
    pBackground->setDiagramDataModelID(EMPTY_OUSTRING);

    auto& aChildren = pParentShape->getChildren();
    aChildren.insert(aChildren.begin(), pBackground);
}

uno::Reference<xml::dom::XDocument> SmartArtDiagram::convertAndSet(std::u16string_view rDOM, svx::diagram::DomMapFlag aDomMapFlag)
{
    // construct MemoryStream and OStreamWrapper
    const OString sUtf8(OUStringToOString(rDOM, RTL_TEXTENCODING_UTF8));
    SvMemoryStream aStream(const_cast<char*>(sUtf8.getStr()), sUtf8.getLength(), StreamMode::READ);
    rtl::Reference<utl::OStreamWrapper> pStreamWrapper = new utl::OStreamWrapper(aStream);

    // create the dom parser & create DomTree
    uno::Reference<xml::dom::XDocumentBuilder> xDomBuilder(xml::dom::DocumentBuilder::create(comphelper::getProcessComponentContext()));
    uno::Reference<xml::dom::XDocument> aDomTree(xDomBuilder->parse(pStreamWrapper->getInputStream()));

    // set DomTree locally
    setOOXDomValue(aDomMapFlag, cpo::uno::Any(aDomTree));
    return aDomTree;
}

cpo::uno::Reference<css::xml::dom::XDocument> SmartArtDiagram::convertAndSet(std::u16string_view rDOMData, svx::diagram::DomMapFlag aDomMapFlag, bool bAdd)
{
    // construct MemoryStream and OStreamWrapper
    const OString sUtf8(OUStringToOString(rDOMData, RTL_TEXTENCODING_UTF8));
    SvMemoryStream aStream(const_cast<char*>(sUtf8.getStr()), sUtf8.getLength(), StreamMode::READ);
    rtl::Reference<utl::OStreamWrapper> pStreamWrapper = new utl::OStreamWrapper(aStream);

    // create the dom parser & create DomTree
    uno::Reference<xml::dom::XDocumentBuilder> xDomBuilder(xml::dom::DocumentBuilder::create(comphelper::getProcessComponentContext()));
    uno::Reference<xml::dom::XDocument> aDomTree(xDomBuilder->parse(pStreamWrapper->getInputStream()));

    // set DomTree locally
    if (bAdd)
        setOOXDomValue(aDomMapFlag, cpo::uno::Any(aDomTree));
    return aDomTree;
}

SmartArtDiagram::SmartArtDiagram()
: maDiagramFontHeights()
, mpData(std::make_shared<DiagramData_oox>())
, mpLayout(std::make_shared<DiagramLayout>(*this))
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

SmartArtDiagram::SmartArtDiagram(std::u16string_view rLayout, std::u16string_view rData, std::u16string_view rColors, std::u16string_view rQuickstyle)
: maDiagramFontHeights()
, mpData(std::make_shared<DiagramData_oox>())
, mpLayout(std::make_shared<DiagramLayout>(*this))
, maStyles()
, maColors()
, maDiagramPRDomMap()
{
    if (rLayout.empty() || rData.empty() || rQuickstyle.empty() || rColors.empty())
        return;

    // we need a PowerPointImport for the FragmentHandlers, so create a single
    // temporary one. Use this for all possible DomTrees
    rtl::Reference<oox::ppt::PowerPointImport> xPPTImport(new oox::ppt::PowerPointImport(comphelper::getProcessComponentContext()));

    if (!rData.empty())
    {
        // create and set DomTree locally/do *not* add to DomTree holder, this is a temporary instance
        uno::Reference<xml::dom::XDocument> xDom(convertAndSet(rData, svx::diagram::DomMapFlag::OOXLayout, false));

        // import DomTree to mpLayout
        uno::Reference<xml::sax::XFastSAXSerializable> xSerializer(xDom, uno::UNO_QUERY_THROW);
        rtl::Reference< core::FragmentHandler > xRefLayout(new DiagramDataFragmentHandler(*xPPTImport, u"internal"_ustr, mpData));
        xPPTImport->importFragment(xRefLayout, xSerializer);
    }

    if (!rLayout.empty())
    {
        // create and set DomTree locally
        uno::Reference<xml::dom::XDocument> xDom(convertAndSet(rLayout, svx::diagram::DomMapFlag::OOXLayout, true));

        // import DomTree to mpLayout
        uno::Reference<xml::sax::XFastSAXSerializable> xSerializer(xDom, uno::UNO_QUERY_THROW);
        rtl::Reference< core::FragmentHandler > xRefLayout(new DiagramLayoutFragmentHandler(*this, *xPPTImport, u"internal"_ustr, mpLayout));
        xPPTImport->importFragment(xRefLayout, xSerializer);
    }

    if (!rQuickstyle.empty())
    {
        // create and set DomTree locally
        uno::Reference<xml::dom::XDocument> xDom(convertAndSet(rQuickstyle, svx::diagram::DomMapFlag::OOXStyle, true));

        // import DomTree to maStyles
        uno::Reference<xml::sax::XFastSAXSerializable> xSerializer(xDom, uno::UNO_QUERY_THROW);
        rtl::Reference< core::FragmentHandler > xRefLayout(new DiagramQStylesFragmentHandler(*xPPTImport, u"internal"_ustr, maStyles));
        xPPTImport->importFragment(xRefLayout, xSerializer);
    }

    if (!rColors.empty())
    {
        // create and set DomTree locally
        uno::Reference<xml::dom::XDocument> xDom(convertAndSet(rColors, svx::diagram::DomMapFlag::OOXColor, true));

        // import DomTree to maColors
        uno::Reference<xml::sax::XFastSAXSerializable> xSerializer(xDom, uno::UNO_QUERY_THROW);
        rtl::Reference< core::FragmentHandler > xRefLayout(new ColorFragmentHandler(*xPPTImport, u"internal"_ustr, maColors));
        xPPTImport->importFragment(xRefLayout, xSerializer);
    }
}

SmartArtDiagram::~SmartArtDiagram()
{
}

cpo::uno::Any SmartArtDiagram::getOOXDomValue(svx::diagram::DomMapFlag aDomMapFlag) const
{
    const DiagramPRDomMap::const_iterator aHit = maDiagramPRDomMap.find(aDomMapFlag);

    if (aHit != maDiagramPRDomMap.end())
        return aHit->second;

    return cpo::uno::Any();
}

void SmartArtDiagram::setOOXDomValue(svx::diagram::DomMapFlag aDomMapFlag, const cpo::uno::Any& rValue)
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
    if (!SdrObject::useAdvancedDiagramFeatures() && maDiagramPRDomMap.end() == maDiagramPRDomMap.find(svx::diagram::DomMapFlag::OOXData))
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
    getData()->writeDiagramData(rOriginalDrawingML, aFS, rDrawingRelId, false);

    // this call is *important*, without it xDocBuilder->parse below fails and some strange
    // and wrong assertion gets thrown in ~FastSerializerHelper that  shall get called
    xOutputStream->closeOutput();

#ifdef DBG_UTIL
    uno::Reference< embed::XRelationshipAccess > xRelations( xOutputStream, uno::UNO_QUERY );
    if( xRelations.is() )
    {
        const cpo::uno::Sequence<cpo::uno::Sequence<beans::StringPair>> aSeqs = xRelations->getAllRelationships();
        for (const cpo::uno::Sequence<beans::StringPair>& aSeq : aSeqs)
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

void SmartArtDiagram::writeDiagramReducedOOXData(cpo::uno::Reference<css::io::XOutputStream>& xOutputStream) const
{
    // need a XmlFilterBase for ShapeExport/DrawingML. All the 'big' exports classes for
    // this are in sd/sc/sw and we are not in an oox export here, so none exists. Use
    // a minimal one. It's main purpose is to host the XModel access. In this case we
    // will not write any Line/Fill/TextAttributes and/or text, so will be fine for the
    // reduced form.
    class LocalFilterBase final : public oox::core::XmlFilterBase
    {
    public:
        explicit LocalFilterBase(cpo::uno::Reference<cpo::uno::XComponentContext> const& rxContext)
        : XmlFilterBase(rxContext) {}
        // virtual ~LocalFilterBase() override;

        virtual const oox::drawingml::Theme* getCurrentTheme() const override { return mpTheme.get(); }
        virtual std::shared_ptr<oox::drawingml::Theme> getCurrentThemePtr() const override { return mpTheme; }

        virtual oox::vml::Drawing* getVmlDrawing() override { return nullptr; }
        virtual oox::drawingml::table::TableStyleListPtr getTableStyles() override { return oox::drawingml::table::TableStyleListPtr(); }
        virtual oox::drawingml::chart::ChartConverter* getChartConverter() override { return nullptr; }
        virtual oox::ole::VbaProject* implCreateVbaProject() const override { return nullptr; }

        virtual bool importDocument() override { return false; }
        virtual bool exportDocument() override { return true; }

    private:
        virtual OUString getImplementationName() override { return EMPTY_OUSTRING; }
        oox::drawingml::ThemePtr mpTheme;
    };

    rtl::Reference<LocalFilterBase> xLocalFilterBase(new LocalFilterBase(comphelper::getProcessComponentContext()));
    xLocalFilterBase->setSourceDocument(getData()->accessRootModel());

    // need a sax_fastparser
    sax_fastparser::FSHelperPtr aFS = std::make_shared<sax_fastparser::FastSerializerHelper>(xOutputStream, true);

    // need a DrawingML, use a ShapeExport. Claim to be a DOCUMENT_PPTX for all ODF targets
    oox::drawingml::ShapeExport aShapeExport(XML_dsp, aFS, nullptr, xLocalFilterBase.get(), DOCUMENT_PPTX, nullptr, true);
    aShapeExport.setDiagaramExport(true);

    // write reduced DiagramData
    getData()->writeDiagramData(aShapeExport, aFS, EMPTY_OUSTRING, true);
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
        const cpo::uno::Sequence<cpo::uno::Sequence<beans::StringPair>> aSeqs = xRelations->getAllRelationships();
        for (const cpo::uno::Sequence<beans::StringPair>& aSeq : aSeqs)
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
    = std::unordered_map<std::shared_ptr<drawingml::Shape>, uno::Reference<drawing::XShape>>;

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
                    xPropertySet->setPropertyValue(u"TextFitToSizeFontScale"_ustr, cpo::uno::Any(fMinFontScale));
                    xPropertySet->setPropertyValue(u"TextFitToSizeSpacingScale"_ustr, cpo::uno::Any(fMinSpacingScale));
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
    pDiagram->setOOXDomValue(aDomMapFlag, cpo::uno::Any(rXDom));

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

    try
    {
        // set DiagramFontHeights at filter
        rFilter.setDiagramFontHeights(&pDiagram->getDiagramFontHeights());

        // data
        if( !rDataModelPath.isEmpty() )
        {
            rtl::Reference< core::FragmentHandler > xRefDataModel(
                    new DiagramDataFragmentHandler( rFilter, rDataModelPath, pDiagram->getData() ));

            importFragment(rFilter,
                           loadFragment(rFilter,xRefDataModel),
                           svx::diagram::DomMapFlag::OOXData,
                           pDiagram,
                           xRefDataModel);

            cpo::uno::Sequence<cpo::uno::Sequence<cpo::uno::Any>> aDataImageRelsMap(
                pShape->resolveRelationshipsOfTypeFromOfficeDoc(
                    rFilter, xRefDataModel->getFragmentPath(), u"image"));
            cpo::uno::Sequence<cpo::uno::Sequence<cpo::uno::Any>> aDataHlinkRelsMap(
                pShape->resolveRelationshipsOfTypeFromOfficeDoc(
                    rFilter, xRefDataModel->getFragmentPath(), u"hlink"));

            pDiagram->setOOXDomValue(svx::diagram::DomMapFlag::OOXDataImageRels,
                                     cpo::uno::Any(aDataImageRelsMap));
            pDiagram->setOOXDomValue(svx::diagram::DomMapFlag::OOXDataHlinkRels,
                                     cpo::uno::Any(aDataHlinkRelsMap));

            // Pass the info to pShape
            for (auto const& extDrawing : pDiagram->getData()->getExtDrawings())
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
                    new DiagramLayoutFragmentHandler( *pDiagram, rFilter, rLayoutPath, pDiagram->getLayout()));

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

        if( !pDiagram->getData()->getExtDrawings().empty() )
        {
            const DiagramColorMap::const_iterator aColor = pDiagram->getColors().find(u"node0"_ustr);
            if( aColor != pDiagram->getColors().end()
                && !aColor->second.maTextFillColors.maColors.empty())
            {
                // TODO(F1): well, actually, there might be *several* color
                // definitions in it, after all it's called list.
                pShape->setFontRefColorForNodes(
                    DiagramColor::getColorByIndex(aColor->second.maTextFillColors, -1, 0));
            }
        }

        // collect data, init maps
        // for Diagram import, do - for now - NOT clear all oox::drawingml::Shape
        pDiagram->getData()->buildDiagramDataModel(false);
#ifdef DBG_UTIL
        pDiagram->getData()->dump();
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
            pDiagram->getData()->setThemeDocument(aTheme->getFragment());

        // Prepare support for the advanced DiagramHelper using Diagram & Theme data
        // This is where pDiagram is moved to where it will stay, else it wil get
        // cleaned up (what is intended)
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

oox::drawingml::Color DiagramColor::getColorByIndex(const DiagramColorList& rList,
                                                    sal_Int32 nIndex, sal_Int32 nCount)
{
    const std::vector<oox::drawingml::Color>& rColors(rList.maColors);
    assert(!rColors.empty());

    if (nIndex == -1)
        return rColors.back();

    const sal_Int32 nColors(static_cast<sal_Int32>(rColors.size()));

    // A span and a cycle list both spread their colors over the shapes, and only if there is
    // something to spread between. A repeat list, and any list of one colour, hands out one
    // of its colors as it stands.
    const bool bSpan(XML_span == rList.mnMethod);
    const bool bCycle(XML_cycle == rList.mnMethod);

    // A span puts a color on the first shape and another on the last, so it needs two shapes
    // to spread between; a cycle starts its walk on one shape and needs only that. Both ways
    // of spreading divide by the number of shapes below, and this is what keeps that divisor
    // away from zero.
    const sal_Int32 nShapesNeeded(bSpan ? 2 : 1);

    if ((!bSpan && !bCycle) || nColors < 2 || nCount < nShapesNeeded
        || rList.maResolvedColors.size() != rColors.size())
        return rColors[nIndex % nColors];

    const sal_Int32 nPlace(std::clamp<sal_Int32>(nIndex, 0, nCount - 1));

    // Where this shape sits in the run of colors, which says which pair of them it falls
    // between and how far along that pair it is.
    double fPosition(0.0);

    if (bSpan)
    {
        // The colors and the shapes are spread over the same run, the first color on the
        // first shape and the last color on the last one. There are two shapes at least,
        // so the divisor is one at least.
        fPosition = nPlace * (nColors - 1.0) / (nCount - 1.0);
    }
    else
    {
        // A cycle walks the colors to the end and back again, and it leaves room for one
        // shape more than the run holds, so that shapes laid out in a ring meet the color
        // they started from rather than repeat it.
        // There is one shape at least, so the divisor is one at least.
        const double fWholeWalk(2.0 * (nColors - 1));
        const double fWalked(nPlace * fWholeWalk / nCount);
        fPosition = fWalked <= nColors - 1 ? fWalked : fWholeWalk - fWalked;
    }

    const sal_Int32 nFirst(std::min<sal_Int32>(fPosition, nColors - 2));
    const double fFactor(fPosition - nFirst);

    const basegfx::BColor aStartRGB(rList.maResolvedColors[nFirst].getBColor());
    const basegfx::BColor aEndRGB(rList.maResolvedColors[nFirst + 1].getBColor());
    const basegfx::BColor aStartHSL(basegfx::utils::rgb2hsl(aStartRGB));
    const basegfx::BColor aEndHSL(basegfx::utils::rgb2hsl(aEndRGB));

    // The hue rises when the end color has the larger hue and falls otherwise, so the walk
    // stays between 0 and 360 degrees and never passes red at 0 degrees, even when the way
    // round the other side would be shorter. A list may state a direction to walk in, but the
    // producer of the format ignores that attribute, as [MS-OI29500] 2.1.1630 b. states, so it
    // is not read here. See also
    // https://learn.microsoft.com/en-us/answers/questions/5905698/the-huedir-attribute-of-a-dgm-fillclrlst-element-d
    const basegfx::BColor aWantedHSL(basegfx::utils::rgb2hsl(basegfx::utils::interpolateInHSL(
        aStartRGB, aEndRGB, fFactor, aStartHSL.getRed() <= aEndHSL.getRed())));

    // Keep the color of the list itself and add the distance to the wanted one as
    // hue, saturation and luminance offsets, which is how the format expresses a
    // spread color. A shape then still follows its theme color.
    oox::drawingml::Color aRetval(rColors[nFirst]);
    const auto aAddOffset = [&aRetval](sal_Int32 nToken, double fOffset, sal_Int32 nUnit) {
        const sal_Int32 nValue(static_cast<sal_Int32>(std::round(fOffset * nUnit)));
        if (0 != nValue)
            aRetval.addTransformation(nToken, nValue);
    };
    aAddOffset(XML_hueOff, aWantedHSL.getRed() - aStartHSL.getRed(), PER_DEGREE);
    aAddOffset(XML_satOff, aWantedHSL.getGreen() - aStartHSL.getGreen(), MAX_PERCENT);
    aAddOffset(XML_lumOff, aWantedHSL.getBlue() - aStartHSL.getBlue(), MAX_PERCENT);

    return aRetval;
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
