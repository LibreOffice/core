/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the Collabora Office project.
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
#include <basegfx/matrix/b2dhommatrixtools.hxx>
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

namespace
{
// Builds the OOXML connector guide points in Hmm, absolute coordinates: the start point, the
// bend/control points and the end point, computed from the adjustment values and the shape's
// size, position, rotation and flip. For bent connectors this polygon is the exact rendered path;
// for curved connectors it is the control polygon the curve follows (which is why the draggable
// handles sit at the same places for both). Returns an empty polygon for connectors that have no
// handles (bentConnector2 / curvedConnector2) and for non-connector shapes.
// This backs both getOOXHandlePositionsHmm() (handles are the inner-segment midpoints) and the
// authored-path import for connectors that cannot be routed (tdf#154074).
basegfx::B2DPolygon getOOXConnectorPointsHmm(const oox::drawingml::ShapePtr& pConnector)
{
    basegfx::B2DPolygon aPolygon;
    if (!pConnector)
        return aPolygon;

    // Convert string attribute to number. Default 50000 (50%, i.e. the bend at the midpoint) if
    // missing, which is the preset default in presetShapeDefinitions.xml. Unit is 1/100000 of the
    // shape size.
    std::vector<sal_Int32> aAdjustmentOOXVec;
    for (size_t i = 0; i < 3; i++)
    {
        if (i < pConnector->getConnectorAdjustments().size())
            aAdjustmentOOXVec.push_back(pConnector->getConnectorAdjustments()[i].toInt32());
        else
            aAdjustmentOOXVec.push_back(50000);
    }

    const double fWidth = pConnector->getSize().Width; // EMU
    const double fHeight = pConnector->getSize().Height; // EMU
    const double fPosX = pConnector->getPosition().X; // EMU
    const double fPosY = pConnector->getPosition().Y; // EMU

    // Points in the untransformed preset, first segment horizontal, start point at (0|0). These
    // match the path in presetShapeDefinitions.xml; bent and curved connectors share the same
    // points.
    const OUString& rName = pConnector->getConnectorName();
    if (rName == u"bentConnector3"_ustr || rName == u"curvedConnector3"_ustr)
    {
        const double fX1 = aAdjustmentOOXVec[0] / 100000.0 * fWidth;
        aPolygon.append({ 0.0, 0.0 });
        aPolygon.append({ fX1, 0.0 });
        aPolygon.append({ fX1, fHeight });
        aPolygon.append({ fWidth, fHeight });
    }
    else if (rName == u"bentConnector4"_ustr || rName == u"curvedConnector4"_ustr)
    {
        const double fX1 = aAdjustmentOOXVec[0] / 100000.0 * fWidth;
        const double fY2 = aAdjustmentOOXVec[1] / 100000.0 * fHeight;
        aPolygon.append({ 0.0, 0.0 });
        aPolygon.append({ fX1, 0.0 });
        aPolygon.append({ fX1, fY2 });
        aPolygon.append({ fWidth, fY2 });
        aPolygon.append({ fWidth, fHeight });
    }
    else if (rName == u"bentConnector5"_ustr || rName == u"curvedConnector5"_ustr)
    {
        const double fX1 = aAdjustmentOOXVec[0] / 100000.0 * fWidth;
        const double fY2 = aAdjustmentOOXVec[1] / 100000.0 * fHeight;
        const double fX3 = aAdjustmentOOXVec[2] / 100000.0 * fWidth;
        aPolygon.append({ 0.0, 0.0 });
        aPolygon.append({ fX1, 0.0 });
        aPolygon.append({ fX1, fY2 });
        aPolygon.append({ fX3, fY2 });
        aPolygon.append({ fX3, fHeight });
        aPolygon.append({ fWidth, fHeight });
    }
    else
        return aPolygon;

    // The preset layout is turned into the actual layout by flipping and rotating around the center.
    basegfx::B2DHomMatrix aTransform;
    const basegfx::B2DPoint aB2DCenter(fWidth / 2.0, fHeight / 2.0);
    aTransform.translate(-aB2DCenter);
    aTransform *= ConnectorHelper::getConnectorTransformMatrix(pConnector);
    aTransform.translate(aB2DCenter);
    aTransform.translate(fPosX, fPosY); // make absolute

    aPolygon.transform(aTransform);

    constexpr double fEmuToHmm = o3tl::convert(1.0, o3tl::Length::emu, o3tl::Length::mm100);
    aPolygon.transform(basegfx::utils::createScaleB2DHomMatrix(fEmuToHmm, fEmuToHmm));
    return aPolygon;
}
}

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

    if (pConnector->getRotation() == 5400000 || pConnector->getRotation() == -16200000)
        aTransform *= basegfx::B2DHomMatrix(0, -1, 0, 1, 0, 0);
    else if (pConnector->getRotation() == 10800000 || pConnector->getRotation() == -10800000)
        aTransform *= basegfx::B2DHomMatrix(-1, 0, 0, 0, -1, 0);
    else if (pConnector->getRotation() == 16200000 || pConnector->getRotation() == -5400000)
        aTransform *= basegfx::B2DHomMatrix(0, 1, 0, -1, 0, 0);
    else
        SAL_WARN("oox", "tdf#157888 LibreOffice cannot handle such connector rotation");
    return aTransform;
}

void ConnectorHelper::getOOXHandlePositionsHmm(const oox::drawingml::ShapePtr& pConnector,
                                               std::vector<basegfx::B2DPoint>& rHandlePositions)
{
    rHandlePositions.clear();

    // The draggable handles sit at the midpoints of the connector's inner segments, i.e. every
    // segment except the first (leaving the start point) and the last (entering the end point).
    // bentConnector2 / curvedConnector2 have no inner segment and thus no handle. This is the same
    // relation getLOBentHandlePositionsHmm() uses to read the handles off the svx edge track.
    const basegfx::B2DPolygon aPoints(getOOXConnectorPointsHmm(pConnector));
    for (sal_uInt32 i = 1; i + 2 < aPoints.count(); ++i)
        rHandlePositions.push_back((aPoints.getB2DPoint(i) + aPoints.getB2DPoint(i + 1)) / 2.0);
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

void ConnectorHelper::applyConnections(const oox::drawingml::ShapePtr& pConnector,
                                       oox::drawingml::ShapeIdMap& rShapeMap)
{
    uno::Reference<drawing::XShape> xConnector(pConnector->getXShape());
    if (!xConnector.is())
        return;
    uno::Reference<beans::XPropertySet> xPropSet(xConnector, uno::UNO_QUERY);
    if (!xPropSet.is())
        return;

    // MS Office allows route between shapes with small distance. LO default is 5mm.
    xPropSet->setPropertyValue(u"EdgeNode1HorzDist"_ustr, cpo::uno::Any(sal_Int32(0)));
    xPropSet->setPropertyValue(u"EdgeNode1VertDist"_ustr, cpo::uno::Any(sal_Int32(0)));
    xPropSet->setPropertyValue(u"EdgeNode2HorzDist"_ustr, cpo::uno::Any(sal_Int32(0)));
    xPropSet->setPropertyValue(u"EdgeNode2VertDist"_ustr, cpo::uno::Any(sal_Int32(0)));

    // A OOXML curvedConnector uses a routing method which is basically incompatible with the
    // traditional way of LibreOffice. A compatible way was added and needs to be enabled before
    // connections are set, so that the method is used in the default routing.
    xPropSet->setPropertyValue(u"EdgeOOXMLCurve"_ustr, cpo::uno::Any(true));

    oox::drawingml::ConnectorShapePropertiesList aConnectorShapeProperties
        = pConnector->getConnectorShapeProperties();
    // It contains maximal two items, each a struct with mbStartShape, maDestShapeId, mnDestGlueId
    for (const auto& aIt : aConnectorShapeProperties)
    {
        const auto pItem = rShapeMap.find(aIt.maDestShapeId);
        if (pItem == rShapeMap.end())
            continue;

        uno::Reference<drawing::XShape> xShape(pItem->second->getXShape(), uno::UNO_QUERY);
        if (xShape.is())
        {
            // Connect to the found shape.
            if (aIt.mbStartShape)
                xPropSet->setPropertyValue(u"StartShape"_ustr, cpo::uno::Any(xShape));
            else
                xPropSet->setPropertyValue(u"EndShape"_ustr, cpo::uno::Any(xShape));

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
                xPropSet->setPropertyValue(u"StartGluePointIndex"_ustr, cpo::uno::Any(nGlueId));
            else
                xPropSet->setPropertyValue(u"EndGluePointIndex"_ustr, cpo::uno::Any(nGlueId));
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

    // A connector that is glued to a shape at both ends has no independently stored path: its route
    // follows from the two endpoints, so we let svx route it and transfer the OOXML handle positions
    // onto that route as EdgeLine deltas.
    // When a connector is not glued at both ends, the file instead stores its actual drawn path
    // (preset geometry plus adjustments, in a bounding box that may have been resized by hand). We
    // must keep that path as authored rather than re-route it: orthogonal routing is heuristic, so
    // re-routing from the endpoints would produce a different shape - it differs between applications
    // and changes as the algorithms evolve, and PowerPoint likewise keeps the drawn path and does not
    // reroute such connectors when they are moved or resized. So we impose the authored OOXML path
    // directly (tdf#154074).
    uno::Reference<drawing::XShape> xStartShape;
    uno::Reference<drawing::XShape> xEndShape;
    xPropSet->getPropertyValue(u"StartShape"_ustr) >>= xStartShape;
    xPropSet->getPropertyValue(u"EndShape"_ustr) >>= xEndShape;

    if (xStartShape.is() && xEndShape.is())
    {
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
                                           cpo::uno::Any(nDiff));
            }
        }
        return;
    }

    SdrEdgeObj* pEdgeObj = dynamic_cast<SdrEdgeObj*>(SdrObject::getSdrObjectFromXShape(xConnector));
    if (!pEdgeObj)
        return;

    basegfx::B2DPolygon aPolygon(getOOXConnectorPointsHmm(pConnector)); // Hmm
    if (aPolygon.count() < 2)
        return;

    // The polygon is in Hmm, but the edge track lives in the model's map unit (e.g. Twips in a text
    // document). Convert if needed.
    const MapUnit eMapUnit = pEdgeObj->getSdrModelFromSdrObject().GetItemPool().GetMetric(0);
    if (eMapUnit != MapUnit::Map100thMM)
    {
        const auto eTo = MapToO3tlLength(eMapUnit);
        if (eTo == o3tl::Length::invalid)
            return;
        const double fConvert(o3tl::convert(1.0, o3tl::Length::mm100, eTo));
        aPolygon.transform(basegfx::utils::createScaleB2DHomMatrix(fConvert, fConvert));
    }

    pEdgeObj->SetEdgeTrackPath(basegfx::B2DPolyPolygon(aPolygon));
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
                                       cpo::uno::Any(nDiff));
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
