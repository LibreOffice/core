/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <drawingml/connectorhelper.hxx>

#include <sal/config.h>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XIdentifierContainer.hpp>
#include <com/sun/star/drawing/XGluePointsSupplier.hpp>

#include <basegfx/curve/b2dcubicbezier.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/point/b2dpoint.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <basegfx/vector/b2dvector.hxx>
#include <drawingml/customshapeproperties.hxx>
#include <oox/drawingml/drawingmltypes.hxx>
#include <oox/drawingml/shape.hxx>
#include <rtl/ustring.hxx>
#include <svl/itempool.hxx>
#include <svx/svdmodel.hxx>
#include <svx/svdoedge.hxx>
#include <svx/svdobj.hxx>
#include <tools/mapunit.hxx>
#include <tools/UnitConversion.hxx>

#include <map>
#include <set>
#include <string_view>
#include <vector>

using namespace ::com::sun::star;

// These shapes have no gluepoints defined in their mso_CustomShape struct, thus the gluepoint
// adaption to default gluepoints will be done. Other shapes having no gluepoint defined in the
// mso_CustomShape struct, have gluepoints in order top-left-bottom-right in OOXML. But the shapes
// below have order right-bottom-left-top. Adding gluepoints to mso_CustomShape structs does not
// solve the problem because MS binary gluepoints and OOXML gluepoints are different.

bool ConnectorHelper::hasClockwiseCxn(const OUString& rShapeType)
{
    static const std::set<OUString> aWithClockwiseCxnSet({ u"accentBorderCallout1"_ustr,
                                                           u"accentBorderCallout2"_ustr,
                                                           u"accentBorderCallout3"_ustr,
                                                           u"accentCallout1"_ustr,
                                                           u"accentCallout2"_ustr,
                                                           u"accentCallout3"_ustr,
                                                           u"actionButtonBackPrevious"_ustr,
                                                           u"actionButtonBeginning"_ustr,
                                                           u"actionButtonBlank"_ustr,
                                                           u"actionButtonDocument"_ustr,
                                                           u"actionButtonEnd"_ustr,
                                                           u"actionButtonForwardNext"_ustr,
                                                           u"actionButtonHelp"_ustr,
                                                           u"actionButtonHome"_ustr,
                                                           u"actionButtonInformation"_ustr,
                                                           u"actionButtonMovie"_ustr,
                                                           u"actionButtonReturn"_ustr,
                                                           u"actionButtonSound"_ustr,
                                                           u"borderCallout1"_ustr,
                                                           u"borderCallout2"_ustr,
                                                           u"borderCallout3"_ustr,
                                                           u"callout1"_ustr,
                                                           u"callout2"_ustr,
                                                           u"callout3"_ustr,
                                                           u"cloud"_ustr,
                                                           u"corner"_ustr,
                                                           u"diagStripe"_ustr,
                                                           u"flowChartOfflineStorage"_ustr,
                                                           u"halfFrame"_ustr,
                                                           u"mathDivide"_ustr,
                                                           u"mathMinus"_ustr,
                                                           u"mathPlus"_ustr,
                                                           u"nonIsoscelesTrapezoid"_ustr,
                                                           u"pie"_ustr,
                                                           u"round2DiagRect"_ustr,
                                                           u"round2SameRect"_ustr,
                                                           u"snip1Rect"_ustr,
                                                           u"snip2DiagRect"_ustr,
                                                           u"snip2SameRect"_ustr,
                                                           u"snipRoundRect"_ustr });
    return aWithClockwiseCxnSet.contains(rShapeType);
}

basegfx::B2DHomMatrix
ConnectorHelper::getConnectorTransformMatrix(const oox::drawingml::ShapePtr& pConnector)
{
    basegfx::B2DHomMatrix aTransform; // ctor generates unit matrix
    if (!pConnector)
        return aTransform;
    if (pConnector->getFlipH())
        aTransform.scale(-1.0, 1.0);
    if (pConnector->getFlipV())
        aTransform.scale(1.0, -1.0);
    if (pConnector->getRotation() == 0)
        return aTransform;
    if (pConnector->getRotation() == 5400000)
        aTransform *= basegfx::B2DHomMatrix(0, -1, 0, 1, 0, 0);
    else if (pConnector->getRotation() == 10800000)
        aTransform *= basegfx::B2DHomMatrix(-1, 0, 0, 0, -1, 0);
    else if (pConnector->getRotation() == 16200000)
        aTransform *= basegfx::B2DHomMatrix(0, 1, 0, -1, 0, 0);
    else
        SAL_WARN("oox", "tdf#157888 LibreOffice cannot handle such connector rotation");
    return aTransform;
}

void ConnectorHelper::getOOXHandlePositionsHmm(const oox::drawingml::ShapePtr& pConnector,
                                               std::vector<basegfx::B2DPoint>& rHandlePositions)
{
    rHandlePositions.clear();

    if (!pConnector)
        return;

    if (pConnector->getConnectorName() == u"bentConnector2"_ustr
        || pConnector->getConnectorName() == u"curvedConnector2"_ustr)
        return; // These have no handles.

    // Convert string attribute to number. Set default 50000 if missing.
    std::vector<sal_Int32> aAdjustmentOOXVec; // 1/100000 of shape size
    for (size_t i = 0; i < 3; i++)
    {
        if (i < pConnector->getConnectorAdjustments().size())
            aAdjustmentOOXVec.push_back(pConnector->getConnectorAdjustments()[i].toInt32());
        else
            aAdjustmentOOXVec.push_back(50000);
    }

    // Handle positions depend on EdgeKind and ShapeSize. bendConnector and curvedConnector use the
    // same handle positions. The formulas here correspond to guides in the bendConnector in
    // presetShapeDefinitions.xml.
    const double fWidth = pConnector->getSize().Width; // EMU
    const double fHeight = pConnector->getSize().Height; // EMU
    const double fPosX = pConnector->getPosition().X; // EMU
    const double fPosY = pConnector->getPosition().Y; // EMU

    if (pConnector->getConnectorName() == u"bentConnector3"_ustr
        || pConnector->getConnectorName() == u"curvedConnector3"_ustr)
    {
        double fAdj1 = aAdjustmentOOXVec[0];
        double fX1 = fAdj1 / 100000.0 * fWidth;
        double fY1 = fHeight / 2.0;
        rHandlePositions.push_back({ fX1, fY1 });
    }
    else if (pConnector->getConnectorName() == u"bentConnector4"_ustr
             || pConnector->getConnectorName() == u"curvedConnector4"_ustr)
    {
        double fAdj1 = aAdjustmentOOXVec[0];
        double fAdj2 = aAdjustmentOOXVec[1];
        double fX1 = fAdj1 / 100000.0 * fWidth;
        double fX2 = (fX1 + fWidth) / 2.0;
        double fY2 = fAdj2 / 100000.0 * fHeight;
        double fY1 = fY2 / 2.0;
        rHandlePositions.push_back({ fX1, fY1 });
        rHandlePositions.push_back({ fX2, fY2 });
    }
    else if (pConnector->getConnectorName() == u"bentConnector5"_ustr
             || pConnector->getConnectorName() == u"curvedConnector5"_ustr)
    {
        double fAdj1 = aAdjustmentOOXVec[0];
        double fAdj2 = aAdjustmentOOXVec[1];
        double fAdj3 = aAdjustmentOOXVec[2];
        double fX1 = fAdj1 / 100000.0 * fWidth;
        double fX3 = fAdj3 / 100000.0 * fWidth;
        double fX2 = (fX1 + fX3) / 2.0;
        double fY2 = fAdj2 / 100000.0 * fHeight;
        double fY1 = fY2 / 2.0;
        double fY3 = (fHeight + fY2) / 2.0;
        rHandlePositions.push_back({ fX1, fY1 });
        rHandlePositions.push_back({ fX2, fY2 });
        rHandlePositions.push_back({ fX3, fY3 });
    }

    // The presetGeometry has the first segment horizontal and start point left/top with
    // coordinates (0|0). Other layouts are done by flipping and rotating.
    basegfx::B2DHomMatrix aTransform;
    const basegfx::B2DPoint aB2DCenter(fWidth / 2.0, fHeight / 2.0);
    aTransform.translate(-aB2DCenter);
    aTransform *= getConnectorTransformMatrix(pConnector);
    aTransform.translate(aB2DCenter);

    // Make coordinates absolute
    aTransform.translate(fPosX, fPosY);

    // Actually transform the handle coordinates
    for (auto& rElem : rHandlePositions)
        rElem *= aTransform;

    // Convert EMU -> Hmm
    for (auto& rElem : rHandlePositions)
        rElem /= 360.0;
}

void ConnectorHelper::getLOBentHandlePositionsHmm(const oox::drawingml::ShapePtr& pConnector,
                                                  std::vector<basegfx::B2DPoint>& rHandlePositions)
{
    // This method is intended for Edgekind css::drawing::ConnectorType_STANDARD. Those connectors
    // correspond to OOX bentConnector, aka "ElbowConnector".
    rHandlePositions.clear();

    if (!pConnector)
        return;
    uno::Reference<drawing::XShape> xConnector(pConnector->getXShape());
    if (!xConnector.is())
        return;

    // Get the EdgeTrack polygon. We cannot use UNO "PolyPolygonBezier" because that includes
    // the yet not known anchor position in Writer. Thus get the polygon directly from the object.
    SdrEdgeObj* pEdgeObj = dynamic_cast<SdrEdgeObj*>(SdrObject::getSdrObjectFromXShape(xConnector));
    if (!pEdgeObj)
        return;
    basegfx::B2DPolyPolygon aB2DPolyPolygon(pEdgeObj->GetEdgeTrackPath());
    if (aB2DPolyPolygon.count() == 0)
        return;

    basegfx::B2DPolygon aEdgePolygon = aB2DPolyPolygon.getB2DPolygon(0);
    if (aEdgePolygon.count() < 4 || aEdgePolygon.areControlPointsUsed())
        return;

    // We need Hmm, the polygon might be e.g. in Twips, in Writer for example
    MapUnit eMapUnit = pEdgeObj->getSdrModelFromSdrObject().GetItemPool().GetMetric(0);
    if (eMapUnit != MapUnit::Map100thMM)
    {
        const auto eFrom = MapToO3tlLength(eMapUnit);
        if (eFrom == o3tl::Length::invalid)
            return;
        const double fConvert(o3tl::convert(1.0, eFrom, o3tl::Length::mm100));
        aEdgePolygon.transform(basegfx::B2DHomMatrix(fConvert, 0.0, 0.0, 0.0, fConvert, 0.0));
    }

    // LO has the handle in the middle of a segment, but not for first and last segment.
    for (sal_uInt32 i = 1; i < aEdgePolygon.count() - 2; i++)
    {
        const basegfx::B2DPoint aBeforePt(aEdgePolygon.getB2DPoint(i));
        const basegfx::B2DPoint aAfterPt(aEdgePolygon.getB2DPoint(i + 1));
        rHandlePositions.push_back((aBeforePt + aAfterPt) / 2.0);
    }
}

void ConnectorHelper::getLOCurvedHandlePositionsHmm(
    const oox::drawingml::ShapePtr& pConnector, std::vector<basegfx::B2DPoint>& rHandlePositions)
{
    // This method is intended for Edgekind css::drawing::ConnectorType_Curve for which OoXML
    // compatible routing is enabled.
    rHandlePositions.clear();

    if (!pConnector)
        return;
    uno::Reference<drawing::XShape> xConnector(pConnector->getXShape());
    if (!xConnector.is())
        return;

    // Get the EdgeTrack polygon. We cannot use UNO "PolyPolygonBezier" because that includes
    // the yet not known anchor position in Writer. Thus get the polygon directly from the object.
    SdrEdgeObj* pEdgeObj = dynamic_cast<SdrEdgeObj*>(SdrObject::getSdrObjectFromXShape(xConnector));
    if (!pEdgeObj)
        return;
    basegfx::B2DPolyPolygon aB2DPolyPolygon(pEdgeObj->GetEdgeTrackPath());
    if (aB2DPolyPolygon.count() == 0)
        return;

    basegfx::B2DPolygon aEdgePolygon = aB2DPolyPolygon.getB2DPolygon(0);
    if (aEdgePolygon.count() < 3 || !aEdgePolygon.areControlPointsUsed())
        return;

    // We need Hmm, the polygon might be e.g. in Twips, in Writer for example
    MapUnit eMapUnit = pEdgeObj->getSdrModelFromSdrObject().GetItemPool().GetMetric(0);
    if (eMapUnit != MapUnit::Map100thMM)
    {
        const auto eFrom = MapToO3tlLength(eMapUnit);
        if (eFrom == o3tl::Length::invalid)
            return;
        const double fConvert(o3tl::convert(1.0, eFrom, o3tl::Length::mm100));
        aEdgePolygon.transform(basegfx::B2DHomMatrix(fConvert, 0.0, 0.0, 0.0, fConvert, 0.0));
    }

    // The OOXML compatible routing has the handles as polygon points, but not start or
    // end point.
    for (sal_uInt32 i = 1; i < aEdgePolygon.count() - 1; i++)
    {
        rHandlePositions.push_back(aEdgePolygon.getB2DPoint(i));
    }
}

void ConnectorHelper::applyConnections(oox::drawingml::ShapePtr& pConnector,
                                       oox::drawingml::ShapeIdMap& rShapeMap)
{
    uno::Reference<drawing::XShape> xConnector(pConnector->getXShape());
    if (!xConnector.is())
        return;
    uno::Reference<beans::XPropertySet> xPropSet(xConnector, uno::UNO_QUERY);
    if (!xPropSet.is())
        return;

    // MS Office allows route between shapes with small distance. LO default is 5mm.
    xPropSet->setPropertyValue(u"EdgeNode1HorzDist"_ustr, uno::Any(sal_Int32(0)));
    xPropSet->setPropertyValue(u"EdgeNode1VertDist"_ustr, uno::Any(sal_Int32(0)));
    xPropSet->setPropertyValue(u"EdgeNode2HorzDist"_ustr, uno::Any(sal_Int32(0)));
    xPropSet->setPropertyValue(u"EdgeNode2VertDist"_ustr, uno::Any(sal_Int32(0)));

    // A OOXML curvedConnector uses a routing method which is basically incompatible with the
    // traditional way of LibreOffice. A compatible way was added and needs to be enabled before
    // connections are set, so that the method is used in the default routing.
    xPropSet->setPropertyValue(u"EdgeOOXMLCurve"_ustr, uno::Any(true));

    oox::drawingml::ConnectorShapePropertiesList aConnectorShapeProperties
        = pConnector->getConnectorShapeProperties();
    // It contains maximal two items, each a struct with mbStartShape, maDestShapeId, mnDestGlueId
    for (const auto& aIt : aConnectorShapeProperties)
    {
        const auto& pItem = rShapeMap.find(aIt.maDestShapeId);
        if (pItem == rShapeMap.end())
            continue;

        uno::Reference<drawing::XShape> xShape(pItem->second->getXShape(), uno::UNO_QUERY);
        if (xShape.is())
        {
            // Connect to the found shape.
            if (aIt.mbStartShape)
                xPropSet->setPropertyValue(u"StartShape"_ustr, uno::Any(xShape));
            else
                xPropSet->setPropertyValue(u"EndShape"_ustr, uno::Any(xShape));

            // The first four glue points are the default glue points, which are set by LibreOffice.
            // They do not belong to the preset geometry of the shape.
            // Adapt gluepoint index to LibreOffice
            uno::Reference<drawing::XGluePointsSupplier> xSupplier(xShape, uno::UNO_QUERY);
            css::uno::Reference<css::container::XIdentifierContainer> xGluePoints(
                xSupplier->getGluePoints(), uno::UNO_QUERY);
            sal_Int32 nCountGluePoints = xGluePoints->getIdentifiers().getLength();
            sal_Int32 nGlueId = aIt.mnDestGlueId;

            if (nCountGluePoints > 4)
                nGlueId += 4;
            else
            {
                // In these cases the mso_CustomShape struct defines no gluepoints (Why not?), thus
                // our default gluepoints are used. The order of the default gluepoints might differ
                // from the order of the OOXML gluepoints. We try to change nGlueId so, that the
                // connector attaches to a default gluepoint at the same side as it attaches in OOXML.
                const OUString sShapeType
                    = pItem->second->getCustomShapeProperties()->getShapePresetTypeName();
                if (ConnectorHelper::hasClockwiseCxn(sShapeType))
                    nGlueId = (nGlueId + 1) % 4;
                else
                {
                    bool bFlipH = pItem->second->getFlipH();
                    bool bFlipV = pItem->second->getFlipV();
                    if (bFlipH == bFlipV)
                    {
                        // change id of the left and right glue points of the bounding box (1 <-> 3)
                        if (nGlueId == 1)
                            nGlueId = 3; // Right
                        else if (nGlueId == 3)
                            nGlueId = 1; // Left
                    }
                }
            }

            if (aIt.mbStartShape)
                xPropSet->setPropertyValue(u"StartGluePointIndex"_ustr, uno::Any(nGlueId));
            else
                xPropSet->setPropertyValue(u"EndGluePointIndex"_ustr, uno::Any(nGlueId));
        }
    }
}

void ConnectorHelper::applyBentHandleAdjustments(oox::drawingml::ShapePtr pConnector)
{
    uno::Reference<drawing::XShape> xConnector(pConnector->getXShape(), uno::UNO_QUERY);
    if (!xConnector.is())
        return;
    uno::Reference<beans::XPropertySet> xPropSet(xConnector, uno::UNO_QUERY);
    if (!xPropSet.is())
        return;

    std::vector<basegfx::B2DPoint> aOOXMLHandles;
    ConnectorHelper::getOOXHandlePositionsHmm(pConnector, aOOXMLHandles);
    std::vector<basegfx::B2DPoint> aLODefaultHandles;
    ConnectorHelper::getLOBentHandlePositionsHmm(pConnector, aLODefaultHandles);

    if (aOOXMLHandles.size() == aLODefaultHandles.size())
    {
        bool bUseYforHori
            = basegfx::fTools::equalZero(getConnectorTransformMatrix(pConnector).get(0, 0));
        for (size_t i = 0; i < aOOXMLHandles.size(); i++)
        {
            basegfx::B2DVector aDiff(aOOXMLHandles[i] - aLODefaultHandles[i]);
            sal_Int32 nDiff;
            if ((i == 1 && !bUseYforHori) || (i != 1 && bUseYforHori))
                nDiff = basegfx::fround(aDiff.getY());
            else
                nDiff = basegfx::fround(aDiff.getX());
            xPropSet->setPropertyValue("EdgeLine" + OUString::number(i + 1) + "Delta",
                                       uno::Any(nDiff));
        }
    }
}

void ConnectorHelper::applyCurvedHandleAdjustments(oox::drawingml::ShapePtr pConnector)
{
    uno::Reference<drawing::XShape> xConnector(pConnector->getXShape(), uno::UNO_QUERY);
    if (!xConnector.is())
        return;
    uno::Reference<beans::XPropertySet> xPropSet(xConnector, uno::UNO_QUERY);
    if (!xPropSet.is())
        return;

    std::vector<basegfx::B2DPoint> aOOXMLHandles;
    ConnectorHelper::getOOXHandlePositionsHmm(pConnector, aOOXMLHandles);
    std::vector<basegfx::B2DPoint> aLODefaultHandles;
    ConnectorHelper::getLOCurvedHandlePositionsHmm(pConnector, aLODefaultHandles);

    if (aOOXMLHandles.size() == aLODefaultHandles.size())
    {
        bool bUseYforHori
            = basegfx::fTools::equalZero(getConnectorTransformMatrix(pConnector).get(0, 0));
        for (size_t i = 0; i < aOOXMLHandles.size(); i++)
        {
            basegfx::B2DVector aDiff(aOOXMLHandles[i] - aLODefaultHandles[i]);
            sal_Int32 nDiff;
            if ((i == 1 && !bUseYforHori) || (i != 1 && bUseYforHori))
                nDiff = basegfx::fround(aDiff.getY());
            else
                nDiff = basegfx::fround(aDiff.getX());
            xPropSet->setPropertyValue("EdgeLine" + OUString::number(i + 1) + "Delta",
                                       uno::Any(nDiff));
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
