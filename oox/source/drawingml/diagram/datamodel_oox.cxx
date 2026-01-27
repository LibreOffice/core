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

void DiagramData_oox::writeDiagramData(oox::core::XmlFilterBase& rFB, sax_fastparser::FSHelperPtr& rTarget, const uno::Reference<drawing::XShape>& rRootShape)
{
    if (!rTarget)
        return;

    // write header infos
    const OUString aNsDmlDiagram(rFB.getNamespaceURL(OOX_NS(dmlDiagram)));
    const OUString aNsDml(rFB.getNamespaceURL(NMSP_dmlDiagram));
    rTarget->startElementNS(XML_dgm, XML_dataModel,
        FSNS(XML_xmlns, XML_dgm), aNsDmlDiagram,
        FSNS(XML_xmlns, XML_a), aNsDml);

    // write PointList
    rTarget->startElementNS(XML_dgm, XML_ptLst);
    for (auto& rPoint : getPoints())
    {
        rtl::Reference<sax_fastparser::FastAttributeList> pAttributeList(sax_fastparser::FastSerializerHelper::createAttrList());

        pAttributeList->add(XML_modelId, rPoint.msModelId);
        addTypeConstantToFastAttributeList(rPoint.mnXMLType, pAttributeList);
        if (!rPoint.msCnxId.isEmpty())
            pAttributeList->add(XML_cxnId, rPoint.msCnxId);
        rTarget->startElementNS(XML_dgm, XML_pt, pAttributeList);

        rPoint.writeDiagramData_data(rTarget);

        uno::Reference<drawing::XShape> xMasterText(getMasterXShapeForPoint(rPoint, rRootShape));

        if (xMasterText)
        {
            rTarget->startElementNS(XML_dgm, XML_t);
            DrawingML aTempML(rTarget, &rFB);
            aTempML.WriteText(xMasterText, false, true, XML_a);
            rTarget->endElementNS(XML_dgm, XML_t);

            // uno::Reference<beans::XPropertySet> xProps(xBgShape, uno::UNO_QUERY);
            // aTempML.WriteFill( xProps, xBgShape->getSize());
        }
        else
        {
            const bool bWriteEmptyText(
                TypeConstant::XML_parTrans == rPoint.mnXMLType ||
                TypeConstant::XML_sibTrans == rPoint.mnXMLType ||
                "textNode" == rPoint.msPresentationLayoutName);

            if (bWriteEmptyText)
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
    uno::Reference<drawing::XShape> xBgShape(getXShapeByModelID(rRootShape, getBackgroundShapeModelID()));

    if (xBgShape.is())
    {
        // if we have the BGShape as XShape, export using a temp DrawingML which uses
        // the target file combined with the XmlFilterBase representing the ongoing Diagram export
        DrawingML aTempML(rTarget, &rFB);
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
    const OUString rNsDsp(rFB.getNamespaceURL(OOX_NS(dsp)));
    rTarget->startElementNS(XML_a, XML_ext, XML_uri, rNsDsp);
    OUString aDspLine("<dsp:dataModelExt xmlns:dsp=\"" + rNsDsp + "\" ");
    if (!getExtDrawings().empty())
    {
        aDspLine += "relId=\"" + getExtDrawings().front() + "\" ";
    }
    aDspLine += "minVer=\"" + aNsDml + "\"/>";
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

DiagramData_oox::~DiagramData_oox()
{
}

static void Connection_dump(const svx::diagram::Connection& rConnection)
{
    SAL_INFO(
        "oox.drawingml",
        "cnx modelId " << rConnection.msModelId << ", srcId " << rConnection.msSourceId << ", dstId "
            << rConnection.msDestId << ", parTransId " << rConnection.msParTransId << ", presId "
            << rConnection.msPresId << ", sibTransId " << rConnection.msSibTransId << ", srcOrd "
            << rConnection.mnSourceOrder << ", dstOrd " << rConnection.mnDestOrder);
}

static void Point_dump(const svx::diagram::Point& rPoint, const Shape* pShape)
{
    SAL_INFO(
        "oox.drawingml",
        "pt text " << pShape << ", cnxId " << rPoint.msCnxId << ", modelId "
            << rPoint.msModelId << ", type " << rPoint.mnXMLType);
}

void DiagramData_oox::dump() const
{
    SAL_INFO("oox.drawingml", "Dgm: DiagramData_oox # of cnx: " << maConnections.size() );
    for (const auto& rConnection : maConnections)
        Connection_dump(rConnection);

    SAL_INFO("oox.drawingml", "Dgm: DiagramData_oox # of pt: " << maPoints.size() );
    for (const auto& rPoint : maPoints)
        Point_dump(rPoint, getOrCreateAssociatedShape(rPoint));
}

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
