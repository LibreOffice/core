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

#include "datamodel_oox.hxx"

#include <rtl/ustrbuf.hxx>
#include <sal/log.hxx>
#include <drawingml/fillproperties.hxx>
#include <drawingml/textbody.hxx>
#include <drawingml/textparagraph.hxx>
#include <drawingml/textrun.hxx>
#include <oox/drawingml/shape.hxx>
#include <com/sun/star/beans/XPropertyState.hpp>
#include <com/sun/star/drawing/FillStyle.hpp>
#include <com/sun/star/drawing/LineStyle.hpp>
#include <com/sun/star/drawing/XShapes.hpp>
#include <editeng/unoprnms.hxx>
#include <oox/token/namespaces.hxx>
#include <oox/export/drawingml.hxx>
#include <sax/fastattribs.hxx>
#include <oox/export/shapes.hxx>

#include <unordered_set>

using namespace ::com::sun::star;
using namespace ::svx::diagram;

namespace oox::drawingml {

Shape* DiagramData_oox::getOrCreateAssociatedShape(const svx::diagram::Point& rPoint, bool bCreateOnDemand) const
{
    if(maPointShapeMap.end() == maPointShapeMap.find(rPoint.msModelId))
    {
        const_cast<DiagramData_oox*>(this)->maPointShapeMap[rPoint.msModelId] = ShapePtr();
    }

    const ShapePtr& rShapePtr = maPointShapeMap.find(rPoint.msModelId)->second;

    if(!rShapePtr && bCreateOnDemand)
    {
        const_cast<ShapePtr&>(rShapePtr) = std::make_shared<Shape>();
    }

    return rShapePtr.get();
}

void DiagramData_oox::writeDiagramReplacement(DrawingML& rOriginalDrawingML, sax_fastparser::FSHelperPtr& rTarget)
{
    if (!rTarget)
        return;

    uno::Reference<drawing::XShapes> xShapes(accessRootShape(), uno::UNO_QUERY);
    if (!xShapes.is())
        return;

    // create an own ShapeExport with the needed target and namespace, mark as DiagramExport
    ::oox::core::XmlFilterBase* pOriginalFB(rOriginalDrawingML.GetFB());
    ShapeExport aShapeExport(XML_dsp, rTarget, nullptr, pOriginalFB, rOriginalDrawingML.GetDocumentType(), rOriginalDrawingML.GetTextExport(), true);
    aShapeExport.setDiagaramExport(true);
    const sal_Int32 nCount(xShapes->getCount());

    // write header infos
    const OUString aNsDmlDiagram(pOriginalFB->getNamespaceURL(OOX_NS(dmlDiagram)));
    const OUString aNsDsp(pOriginalFB->getNamespaceURL(OOX_NS(dsp)));
    const OUString aNsDml(pOriginalFB->getNamespaceURL(OOX_NS(dml)));
    rTarget->startElementNS(XML_dsp, XML_drawing,
        FSNS(XML_xmlns, XML_dgm), aNsDmlDiagram,
        FSNS(XML_xmlns, XML_dsp), aNsDsp,
        FSNS(XML_xmlns, XML_a), aNsDml);
    rTarget->startElementNS(XML_dsp, XML_spTree);

    for ( sal_Int32 i = 0; i < nCount; ++i )
    {
        uno::Reference< drawing::XShape > xShape;
        if ( xShapes->getByIndex( i ) >>= xShape )
        {
            if (xShape)
            {
                // do *not* write BackgroundShape. MSO has BG infos just as fill properties
                // as part of the OOXData DomTree
                SdrObject* pTarget(SdrObject::getSdrObjectFromXShape(xShape));
                if (nullptr == pTarget || getBackgroundShapeModelID() == pTarget->getDiagramDataModelID())
                    continue;

                // write Shape & sub-shapes
                aShapeExport.WriteShape(xShape);
            }
        }
    }

    rTarget->endElementNS(XML_dsp, XML_spTree);
    rTarget->endElementNS(XML_dsp, XML_drawing);
    rTarget->endDocument();
}

void DiagramData_oox::writeDiagramData(DrawingML& rOriginalDrawingML, sax_fastparser::FSHelperPtr& rTarget, std::u16string_view rDrawingRelId)
{
    if (!rTarget)
        return;

    // write header infos
    ::oox::core::XmlFilterBase* pOriginalFB(rOriginalDrawingML.GetFB());
    const OUString aNsDmlDiagram(pOriginalFB->getNamespaceURL(OOX_NS(dmlDiagram)));
    const OUString aNsDml(pOriginalFB->getNamespaceURL(OOX_NS(dml)));
    rTarget->startElementNS(XML_dgm, XML_dataModel,
        FSNS(XML_xmlns, XML_dgm), aNsDmlDiagram,
        FSNS(XML_xmlns, XML_a), aNsDml);

    // write PointList
    rTarget->startElementNS(XML_dgm, XML_ptLst);
    for (auto& rPoint : getPoints())
    {
        rtl::Reference<sax_fastparser::FastAttributeList> pAttributeList(sax_fastparser::FastSerializerHelper::createAttrList());

        pAttributeList->add(XML_modelId, rPoint.msModelId);
        addTypeConstantToFastAttributeList(rPoint.mnXMLType, pAttributeList, true);
        if (!rPoint.msCnxId.isEmpty())
            pAttributeList->add(XML_cxnId, rPoint.msCnxId);
        rTarget->startElementNS(XML_dgm, XML_pt, pAttributeList);

        // write basic Point infos
        rPoint.writeDiagramData_data(rTarget);

        // we need to find a XShape related to this Node (rPoint). Not
        // all Nodes have an associated XShape. First try direct find,
        // that will work e.g. for Objects in the Background (NOT our
        // own BGShape) that have no text, but might have a fill
        uno::Reference<drawing::XShape> xAssociatedShape(getXShapeByModelID(rPoint.msModelId));
        uno::Reference<beans::XPropertySet> xProps;
        bool bWriteFill(false);
        bool bWriteText(false);

        if (xAssociatedShape)
        {
            // only for those mentioned BgShapes because for TextNodes
            // (presName="textNode") the fill is written to the associated
            // text node (phldrT="[Text]")
            if (u"bgShp"_ustr == rPoint.msPresentationLayoutStyleLabel)
            {
                // check for fill
                xProps = uno::Reference<beans::XPropertySet>(xAssociatedShape, uno::UNO_QUERY);
                bWriteFill = xProps->getPropertyValue(u"FillStyle"_ustr) != drawing::FillStyle_NONE;
            }
        }
        else
        {
            // for TextShapes it's more complex: this Node (rPoint) may be the
            // Node holding the text, but the XShape referencing it is associated
            // with a Node that references this by using presAssocID. Use
            // getMasterXShapeForPoint that uses that association and try
            // to access the XShape containing the Text ModelData
            xAssociatedShape = getMasterXShapeForPoint(rPoint);

            if (xAssociatedShape)
            {
                // check for text
                uno::Reference<text::XText> xText(xAssociatedShape, uno::UNO_QUERY);
                bWriteText= xText && !xText->getString().isEmpty();

                // check for fill. This is the associated TextNode (phldrT="[Text]")
                // and the fill is added here, *not* at the XShape/Model node
                xProps = uno::Reference<beans::XPropertySet>(xAssociatedShape, uno::UNO_QUERY);
                bWriteFill = xProps->getPropertyValue(u"FillStyle"_ustr) != drawing::FillStyle_NONE;
            }
        }

        if (bWriteText)
        {
            rTarget->startElementNS(XML_dgm, XML_t);
            DrawingML aTempML(rTarget, pOriginalFB);
            aTempML.setDiagaramExport(true);
            aTempML.WriteText(xAssociatedShape, false, true, XML_a);
            rTarget->endElementNS(XML_dgm, XML_t);
        }
        else
        {
            const bool bWriteEmptyText(
                TypeConstant::XML_parTrans == rPoint.mnXMLType ||
                TypeConstant::XML_sibTrans == rPoint.mnXMLType ||
                "textNode" == rPoint.msPresentationLayoutName);

            // empty text is written by MSO, but may not be needed. For now, just do it
            static bool bSuppressEmptyText(false);

            if (bWriteEmptyText && !bSuppressEmptyText)
            {
                rTarget->startElementNS(XML_dgm, XML_t);
                rTarget->singleElementNS(XML_a, XML_bodyPr);
                rTarget->singleElementNS(XML_a, XML_lstStyle);
                rTarget->startElementNS(XML_a, XML_p);
                rTarget->singleElementNS(XML_a, XML_endParaRPr, XML_lang, "en-US");
                rTarget->endElementNS(XML_a, XML_p);
                rTarget->endElementNS(XML_dgm, XML_t);
            }
        }

        if (bWriteFill)
        {
            rTarget->startElementNS(XML_dgm, XML_spPr);

            DrawingML aTempML(rTarget, pOriginalFB);
            aTempML.setDiagaramExport(true);
            aTempML.WriteFill( xProps, xAssociatedShape->getSize());

            rTarget->endElementNS(XML_dgm, XML_spPr);
        }
        else
        {
            // write empty fill
            rTarget->singleElementNS(XML_dgm, XML_spPr);
        }

        rTarget->endElementNS(XML_dgm, XML_pt);
    }

    rTarget->endElementNS(XML_dgm, XML_ptLst);

    // write ConnectorList
    rTarget->startElementNS(XML_dgm, XML_cxnLst);
    for (auto& rConnection : getConnections())
        rConnection.writeDiagramData(rTarget);
    rTarget->endElementNS(XML_dgm, XML_cxnLst);

    // write BGFill
    rTarget->startElementNS(XML_dgm, XML_bg);

    // there is *no* export for oox::drawingml::FillProperties which we have in mpBackgroundShapeFillProperties.
    // searching for 'XML_.*XML_noFill' shows that there is also a pretty direct export for FillStyle in Chart2,
    // maybe that could be adapted for general use.
    // But there is DrawingML::WriteFill, that needs the XShape, sax_fastparser::FSHelper and a XmlFilterBase. We
    // can organize all that and then export from XShape model data.
    // For The BGShape it is okay since in MSO Diagram data no shape for that exists anyways, it's just
    // FillAttributes and a XShape for BG needs to exist in DrawObject model anytime anyways, see
    // Diagram::createShapeHierarchyFromModel. This will also allow to use existing stuff like standard dialogs
    // and more for later offering changing the Background of a Diagram.
    // Note that an incarnation of BG as XShape is also needed to 'carry' the correct Size for that Diagram.
    uno::Reference<drawing::XShape> xBgShape(getXShapeByModelID(getBackgroundShapeModelID()));

    if (xBgShape.is())
    {
        // if we have the BGShape as XShape, export using a temp DrawingML which uses
        // the target file combined with the XmlFilterBase representing the ongoing Diagram export
        DrawingML aTempML(rTarget, pOriginalFB);
        aTempML.setDiagaramExport(true);
        uno::Reference<beans::XPropertySet> xProps(xBgShape, uno::UNO_QUERY);
        aTempML.WriteFill( xProps, xBgShape->getSize());
    }

    rTarget->endElementNS(XML_dgm, XML_bg);

    rTarget->singleElementNS(XML_dgm, XML_whole);

    // write ExtList & it's contents
    // Note: I *tried* to use XML_dsp and xmlns:dsp, but these are not defined, thus
    // for this case where the only relevant data is the 'relId' entry I will allow
    // to construct the XML statement by own string concatenation
    rTarget->startElementNS(XML_dgm, XML_extLst);
    const OUString rNsDsp(pOriginalFB->getNamespaceURL(OOX_NS(dsp)));
    rTarget->startElementNS(XML_a, XML_ext, XML_uri, rNsDsp);
    OUString aDspLine(u"<dsp:dataModelExt xmlns:dsp=\""_ustr + rNsDsp + u"\" "_ustr);
    if (!rDrawingRelId.empty())
    {
        aDspLine += u"relId=\""_ustr + rDrawingRelId + u"\" "_ustr;
    }
    aDspLine += u"minVer=\""_ustr + aNsDmlDiagram + u"\"/>"_ustr;
    rTarget->write(aDspLine);
    rTarget->endElementNS(XML_a, XML_ext);
    rTarget->endElementNS(XML_dgm, XML_extLst);

    rTarget->endElementNS(XML_dgm, XML_dataModel);
    rTarget->endDocument();
}

DiagramData_oox::DiagramData_oox()
: svx::diagram::DiagramData_svx()
, mpBackgroundShapeFillProperties( std::make_shared<FillProperties>() )
{
}

DiagramData_oox::DiagramData_oox(DiagramData_oox const& rSource)
: svx::diagram::DiagramData_svx(rSource)
, mpBackgroundShapeFillProperties()
, maPointShapeMap()
{
}

DiagramData_oox::~DiagramData_oox()
{
}

#ifdef DBG_UTIL
static void Connection_dump(const svx::diagram::Connection& rConnection)
{
    SAL_INFO(
        "oox.drawingml",
        "  CNX: modelId " << rConnection.msModelId << ", srcId " << rConnection.msSourceId << ", dstId "
            << rConnection.msDestId << ", parTransId " << rConnection.msParTransId << ", presId "
            << rConnection.msPresId << ", sibTransId " << rConnection.msSibTransId << ", srcOrd "
            << rConnection.mnSourceOrder << ", dstOrd " << rConnection.mnDestOrder);
}

static void Point_dump(const svx::diagram::Point& rPoint, const uno::Reference<drawing::XShape>& rXShape)
{
    SAL_INFO("oox.drawingml", "PT: " << rXShape.is() << ", cnxId " << rPoint.msCnxId << ", modelId " << rPoint.msModelId << ", type " << rPoint.mnXMLType);

    SAL_INFO("oox.drawingml", "  PRS_0: " << rPoint.msColorTransformCategoryId << "," << rPoint.msColorTransformTypeId << "," << rPoint.msLayoutCategoryId << "," << rPoint.msLayoutTypeId << "," << rPoint.msPlaceholderText);
    SAL_INFO("oox.drawingml", "  PRS_1: " << rPoint.msPresentationAssociationId << "," << rPoint.msPresentationLayoutName << "," << rPoint.msPresentationLayoutStyleLabel << "," << rPoint.msQuickStyleCategoryId << "," << rPoint.msQuickStyleTypeId);
    SAL_INFO("oox.drawingml", "  PRS_2: " << rPoint.mnCustomAngle << "," << rPoint.mnPercentageNeighbourWidth << "," << rPoint.mnPercentageNeighbourHeight << "," << rPoint.mnPercentageOwnWidth << "," << rPoint.mnPercentageOwnHeight);
    SAL_INFO("oox.drawingml", "  PRS_3: " << rPoint.mnIncludeAngleScale<< rPoint.mnRadiusScale << "," << rPoint.mnWidthScale << "," << rPoint.mnHeightScale << "," << rPoint.mnWidthOverride << "," << rPoint.mnHeightOverride << "," <<  rPoint.mnLayoutStyleCount << "," << rPoint.mnLayoutStyleIndex);
    SAL_INFO("oox.drawingml", "  PRS_4: " << rPoint.mbCoherent3DOffset << "," << rPoint.mbCustomHorizontalFlip << "," << rPoint.mbCustomVerticalFlip << "," << rPoint.mbCustomText << "," << rPoint.mbIsPlaceholder);

    SAL_INFO("oox.drawingml", "  PLV_0: " << rPoint.msResizeHandles << "," << rPoint.mnMaxChildren << "," << rPoint.mnPreferredChildren << "," << rPoint.mnDirection << "," << rPoint.mbOrgChartEnabled << "," << rPoint.mbBulletEnabled);
    SAL_INFO("oox.drawingml", "  PLV_1: " << (rPoint.moHierarchyBranch.has_value() ? rPoint.moHierarchyBranch.value() : 0));

}

void DiagramData_oox::dump() const
{
    size_t a = maConnections.size();
    SAL_INFO("oox.drawingml", "Dgm: DiagramData_oox # of cnx: " << a );
    a = 0;
    for (const auto& rConnection : maConnections)
    {
        SAL_INFO("oox.drawingml", "cnx #" << a++ << ":");
        Connection_dump(rConnection);
    }

    a = maPoints.size();
    SAL_INFO("oox.drawingml", "Dgm: DiagramData_oox # of pt: " << a );
    a = 0;
    for (const auto& rPoint : maPoints)
    {
        SAL_INFO("oox.drawingml", "pt #" << a++ << ":");
        Point_dump(rPoint, getXShapeByModelID(rPoint.msModelId));
    }
}
#endif

void DiagramData_oox::buildDiagramDataModel(bool bClearOoxShapes)
{
    if(bClearOoxShapes)
    {
        // Delete/remove all existing oox::drawingml::Shape
        maPointShapeMap.clear();
    }

    // call parent
    svx::diagram::DiagramData_svx::buildDiagramDataModel(bClearOoxShapes);

    if(bClearOoxShapes)
    {
        // re-create all existing oox::drawingml::Shape
        svx::diagram::Points& rPoints = getPoints();

        for (auto & point : rPoints)
        {
            // Create/get shape. Re-create here, that may also set needed
            // and available data from the Diagram ModelData at the Shape
            getOrCreateAssociatedShape(point, true);
        }
    }
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
