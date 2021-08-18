/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
* This file is part of the LibreOffice project.
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include <oox/export/DMLPresetShapeExport.hxx>
#include <oox/token/tokens.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/drawing/EnhancedCustomShapeAdjustmentValue.hpp>
#include <com/sun/star/drawing/EnhancedCustomShapeParameterPair.hpp>
#include <com/sun/star/drawing/XShape.hpp>

#include <osl/diagnose.h>
#include <filter/msfilter/util.hxx>

#include <string_view>

using namespace ::css;
using namespace ::css::drawing;

namespace oox::drawingml
{
// DMLPresetShapeExporter class

// ctor
DMLPresetShapeExporter::DMLPresetShapeExporter(DrawingML* pDMLExporter,
                                               css::uno::Reference<css::drawing::XShape> xShape)
    : m_pDMLexporter(pDMLExporter)
{
    // This class only work with custom shapes!
    OSL_ASSERT(xShape->getShapeType() == "com.sun.star.drawing.CustomShape");

    m_xShape = xShape;
    m_bHasHandleValues = false;
    uno::Reference<beans::XPropertySet> xShapeProps(m_xShape, uno::UNO_QUERY);
    css::uno::Sequence<css::beans::PropertyValue> aCustomShapeGeometry
        = xShapeProps->getPropertyValue("CustomShapeGeometry")
              .get<uno::Sequence<beans::PropertyValue>>();

    for (sal_uInt32 i = 0; i < aCustomShapeGeometry.size(); i++)
    {
        if (aCustomShapeGeometry[i].Name == "Type")
        {
            m_sPresetShapeType = aCustomShapeGeometry[i].Value.get<OUString>();
        }
        if (aCustomShapeGeometry[i].Name == "Handles")
        {
            m_bHasHandleValues = true;
            m_HandleValues
                = aCustomShapeGeometry[i]
                      .Value
                      .get<css::uno::Sequence<css::uno::Sequence<css::beans::PropertyValue>>>();
        }
        if (aCustomShapeGeometry[i].Name == "AdjustmentValues")
        {
            m_AdjustmentValues
                = aCustomShapeGeometry[i]
                      .Value
                      .get<css::uno::Sequence<css::drawing::EnhancedCustomShapeAdjustmentValue>>();
        }
        if (aCustomShapeGeometry[i].Name == "MirroredX")
        {
            m_bIsFlipped.first = aCustomShapeGeometry[i].Value.get<bool>();
        }
        if (aCustomShapeGeometry[i].Name == "MirroredY")
        {
            m_bIsFlipped.second = aCustomShapeGeometry[i].Value.get<bool>();
        }
        //if (aCustomShapeGeometry[i].Name == "Equations")
        //{
        //    m_Equations = aCustomShapeGeometry[i].Value.get<css::uno::Sequence<OUString>>();
        //}
        //if (aCustomShapeGeometry[i].Name == "Path")
        //{
        //    m_Path = aCustomShapeGeometry[i]
        //                 .Value.get<css::uno::Sequence<css::beans::PropertyValue>>();
        //}
        //if (aCustomShapeGeometry[i].Name == "ViewBox")
        //{
        //    m_ViewBox = aCustomShapeGeometry[i].Value.get<css::awt::Rectangle>();
        //}
    }
};

// dtor
DMLPresetShapeExporter::~DMLPresetShapeExporter(){
    // Do nothing
};

bool DMLPresetShapeExporter::HasHandleValue() { return m_bHasHandleValues; }

const OUString& DMLPresetShapeExporter::GetShapeType() { return m_sPresetShapeType; }

const css::uno::Sequence<css::uno::Sequence<css::beans::PropertyValue>>&
DMLPresetShapeExporter::GetHandleValues()
{
    return m_HandleValues;
};

const css::uno::Sequence<css::drawing::EnhancedCustomShapeAdjustmentValue>&
DMLPresetShapeExporter::GetAdjustmentValues()
{
    return m_AdjustmentValues;
};

css::uno::Any DMLPresetShapeExporter::GetHandleValueOfModificationPoint(sal_Int32 nPoint,
                                                                        std::u16string_view sType)
{
    uno::Any aRet;
    if (GetHandleValues().getLength() > nPoint)
    {
        for (sal_Int32 i = 0; i < GetHandleValues()[nPoint].getLength(); i++)
        {
            if (GetHandleValues()[nPoint][i].Name == sType)
            {
                aRet = GetHandleValues()[nPoint][i].Value;
                break;
            }
        }
    }
    return aRet;
};

DMLPresetShapeExporter::RadiusAdjustmentValue
DMLPresetShapeExporter::GetAdjustmentPointRadiusValue(sal_Int32 nPoint)
{
    RadiusAdjustmentValue aRet;
    try
    {
        auto aValPos = GetHandleValueOfModificationPoint(nPoint, u"Position")
                           .get<EnhancedCustomShapeParameterPair>();
        aRet.nMinVal = GetHandleValueOfModificationPoint(nPoint, u"RadiusRangeMinimum")
                           .get<EnhancedCustomShapeParameter>()
                           .Value.get<double>();
        aRet.nMaxVal = GetHandleValueOfModificationPoint(nPoint, u"RadiusRangeMaximum")
                           .get<EnhancedCustomShapeParameter>()
                           .Value.get<double>();
        aRet.nCurrVal = GetAdjustmentValues()[aValPos.First.Value.get<long>()].Value.get<double>();
    }
    catch (...)
    {
        // Do nothing.
    }
    return aRet;
};

DMLPresetShapeExporter::AngleAdjustmentValue
DMLPresetShapeExporter::GetAdjustmentPointAngleValue(sal_Int32 nPoint)
{
    AngleAdjustmentValue aRet;
    try
    {
        auto aValPos = GetHandleValueOfModificationPoint(nPoint, u"Position")
                           .get<EnhancedCustomShapeParameterPair>();
        aRet.nMinVal = 0;
        aRet.nMaxVal = 360;
        aRet.nCurrVal = GetAdjustmentValues()[aValPos.Second.Value.get<long>()].Value.get<double>();
    }
    catch (...)
    {
        // Do nothing.
    }
    return aRet;
};

DMLPresetShapeExporter::XAdjustmentValue
DMLPresetShapeExporter::GetAdjustmentPointXValue(sal_Int32 nPoint)
{
    XAdjustmentValue aRet;
    try
    {
        auto aValPos = GetHandleValueOfModificationPoint(nPoint, u"Position")
                           .get<EnhancedCustomShapeParameterPair>();
        aRet.nMinVal = GetHandleValueOfModificationPoint(nPoint, u"RangeXMinimum")
                           .get<EnhancedCustomShapeParameter>()
                           .Value.get<double>();
        aRet.nMaxVal = GetHandleValueOfModificationPoint(nPoint, u"RangeXMaximum")
                           .get<EnhancedCustomShapeParameter>()
                           .Value.get<double>();
        aRet.nCurrVal = GetAdjustmentValues()[aValPos.First.Value.get<long>()].Value.get<double>();
    }
    catch (...)
    {
        // Do nothing.
    }
    return aRet;
};

DMLPresetShapeExporter::YAdjustmentValue
DMLPresetShapeExporter::GetAdjustmentPointYValue(sal_Int32 nPoint)
{
    YAdjustmentValue aRet;
    try
    {
        auto aValPos = GetHandleValueOfModificationPoint(nPoint, u"Position")
                           .get<EnhancedCustomShapeParameterPair>();
        aRet.nMinVal = GetHandleValueOfModificationPoint(nPoint, u"RangeYMinimum")
                           .get<EnhancedCustomShapeParameter>()
                           .Value.get<double>();
        aRet.nMaxVal = GetHandleValueOfModificationPoint(nPoint, u"RangeYMaximum")
                           .get<EnhancedCustomShapeParameter>()
                           .Value.get<double>();
        aRet.nCurrVal = GetAdjustmentValues()[aValPos.Second.Value.get<long>()].Value.get<double>();
    }
    catch (...)
    {
        // Do nothing.
    }
    return aRet;
};

bool DMLPresetShapeExporter::WriteShape()
{
    if (m_pDMLexporter && m_xShape)
    {
        // Case 1: We do not have adjustment points of the shape: just export it as preset
        if (!m_bHasHandleValues)
        {
            OUString sShapeType = GetShapeType();
            const char* sPresetShape
                = msfilter::util::GetOOXMLPresetGeometry(sShapeType.toUtf8().getStr());
            m_pDMLexporter->WriteShapeTransformation(m_xShape, XML_a, IsXFlipped(), IsYFlipped(),
                                                     false, false);
            m_pDMLexporter->WritePresetShape(sPresetShape);
            return true;
        }
        else // Case2: There are adjustment points what have to be converted and exported.
        {
            return WriteShapeWithAVlist();
        }
    }
    return false;
};

bool DMLPresetShapeExporter::WriteAV(const OUString& sValName, const OUString& sVal)
{
    try
    {
        m_pDMLexporter->GetFS()->singleElementNS(XML_a, XML_gd, XML_name, sValName, XML_fmla, sVal);
        return true;
    }
    catch (...)
    {
        return false;
    }
};

bool DMLPresetShapeExporter::StartAVListWriting()
{
    try
    {
        const char* pShape
            = msfilter::util::GetOOXMLPresetGeometry(GetShapeType().toUtf8().getStr());
        m_pDMLexporter->GetFS()->startElementNS(XML_a, XML_prstGeom, XML_prst, pShape);
        m_pDMLexporter->GetFS()->startElementNS(XML_a, XML_avLst);
        return true;
    }
    catch (...)
    {
        return false;
    }
};
bool DMLPresetShapeExporter::EndAVListWriting()
{
    try
    {
        m_pDMLexporter->GetFS()->endElementNS(XML_a, XML_avLst);
        m_pDMLexporter->GetFS()->endElementNS(XML_a, XML_prstGeom);
        return true;
    }
    catch (...)
    {
        return false;
    }
};

bool DMLPresetShapeExporter::WriteShapeWithAVlist()
{
    // Remark: This method is under development. If a shape type is implemented, the corresponding,
    // return must be set to true. False means nothing done true, export done. There are many
    // types which do not have pairs in LO, they are do not have to be mapped, because import
    // filter it does with GrabBag, this method only maps the SDR ones to OOXML shapes.

    OString sShapeType(msfilter::util::GetOOXMLPresetGeometry(GetShapeType().toUtf8().getStr()));

    // OOXML uses 60th of degree, so 360 degree is 21 600 000 60thdeg
    const tools::Long nConstOfMaxDegreeOf60th = 21600000;
    try
    {
        if (sShapeType == "accentBorderCallout1")
        {
            // LO does not have this type, so it does not necessary to be mapped.
            return false;
        }
        if (sShapeType == "accentBorderCallout2")
        {
            // LO does not have this type, so it does not necessary to be mapped.
            return false;
        }
        if (sShapeType == "accentBorderCallout3")
        {
            // LO does not have this type, so it does not necessary to be mapped.
            return false;
        }
        if (sShapeType == "accentCallout1")
        {
            // LO does not have this type, so it does not necessary to be mapped.
            return false;
        }
        if (sShapeType == "accentCallout2")
        {
            // LO does not have this type, so it does not necessary to be mapped.
            return false;
        }
        if (sShapeType == "accentCallout3")
        {
            // LO does not have this type, so it does not necessary to be mapped.
            return false;
        }
        if (sShapeType == "actionButtonBackPrevious")
        {
            // LO does not have this type, so it does not necessary to be mapped.
            return false;
        }
        if (sShapeType == "actionButtonBeginning")
        {
            // LO does not have this type, so it does not necessary to be mapped.
            return false;
        }
        if (sShapeType == "actionButtonBlank")
        {
            // LO does not have this type, so it does not necessary to be mapped.
            return false;
        }
        if (sShapeType == "actionButtonDocument")
        {
            // LO does not have this type, so it does not necessary to be mapped.
            return false;
        }
        if (sShapeType == "actionButtonEnd")
        {
            // LO does not have this type, so it does not necessary to be mapped.
            return false;
        }
        if (sShapeType == "actionButtonForwardNext")
        {
            // LO does not have this type, so it does not necessary to be mapped.
            return false;
        }
        if (sShapeType == "actionButtonHelp")
        {
            // LO does not have this type, so it does not necessary to be mapped.
            return false;
        }
        if (sShapeType == "actionButtonHome")
        {
            // LO does not have this type, so it does not necessary to be mapped.
            return false;
        }
        if (sShapeType == "actionButtonInformation")
        {
            // LO does not have this type, so it does not necessary to be mapped.
            return false;
        }
        if (sShapeType == "actionButtonMovie")
        {
            // LO does not have this type, so it does not necessary to be mapped.
            return false;
        }
        if (sShapeType == "actionButtonReturn")
        {
            // LO does not have this type, so it does not necessary to be mapped.
            return false;
        }
        if (sShapeType == "actionButtonSound")
        {
            // LO does not have this type, so it does not necessary to be mapped.
            return false;
        }
        if (sShapeType == "arc")
        {
            // LO does not have handle points for this, so CustGeom is enough.
            return false;
        }
        if (sShapeType == "bentArrow")
        {
            // LO has only one type, which have to be rotated, without handling points
            // So CustGeom enough.
            return false;
        }
        if (sShapeType == "bentConnector2")
        {
            // CustGeom Enough
            return false;
        }
        if (sShapeType == "bentConnector3")
        {
            // CustGeom Enough
            return false;
        }
        if (sShapeType == "bentConnector4")
        {
            // CustGeom Enough
            return false;
        }
        if (sShapeType == "bentConnector5")
        {
            // CustGeom Enough
            return false;
        }
        if (sShapeType == "bentUpArrow")
        {
            // CustGeom Enough, no handle points
            return false;
        }
        if (sShapeType == "bevel")
        {
            auto aPoint1 = GetAdjustmentPointXValue(0);
            if (!aPoint1.nCurrVal.has_value() || !aPoint1.nMaxVal.has_value()
                || !aPoint1.nMinVal.has_value())
                return false;
            m_pDMLexporter->WriteShapeTransformation(m_xShape, XML_a, IsXFlipped(), IsYFlipped(),
                                                     false, false);

            tools::Long nVal1
                = std::lround(*aPoint1.nCurrVal / (*aPoint1.nMaxVal - *aPoint1.nMinVal) * 50000);
            return StartAVListWriting()
                   && WriteAV(u"adj", OUString(u"val " + OUString::number(nVal1)))
                   && EndAVListWriting();
        }
        if (sShapeType == "blockArc")
        {
            auto aPointR = GetAdjustmentPointRadiusValue(0);
            auto aPointA = GetAdjustmentPointAngleValue(0);
            if (!aPointA.nCurrVal.has_value() || !aPointA.nMaxVal.has_value()
                || !aPointA.nMinVal.has_value() || !aPointR.nCurrVal.has_value()
                || !aPointR.nMaxVal.has_value() || !aPointR.nMinVal.has_value())
                return false;
            m_pDMLexporter->WriteShapeTransformation(m_xShape, XML_a, IsXFlipped(), IsYFlipped(),
                                                     false, false);
            tools::Long nVal1
                = std::lround((*aPointA.nCurrVal < 0 ? 360 + *aPointA.nCurrVal : *aPointA.nCurrVal)
                              / (*aPointA.nMaxVal - *aPointA.nMinVal) * nConstOfMaxDegreeOf60th);
            tools::Long nVal2 = std::lround(
                (*aPointA.nCurrVal > 180 ? 360 - *aPointA.nCurrVal : 180 - *aPointA.nCurrVal)
                / (*aPointA.nMaxVal - *aPointA.nMinVal) * nConstOfMaxDegreeOf60th);
            tools::Long nVal3 = std::lround(
                50000 - (*aPointR.nCurrVal / (*aPointR.nMaxVal - *aPointR.nMinVal) * 50000));
            return StartAVListWriting()
                   && WriteAV(u"adj1", OUString(u"val " + OUString::number(nVal1)))
                   && WriteAV(u"adj2", OUString(u"val " + OUString::number(nVal2)))
                   && WriteAV(u"adj3", OUString(u"val " + OUString::number(nVal3)))
                   && EndAVListWriting();
        }
        if (sShapeType == "borderCallout1")
        {
            // LO does not have this type, so it does not necessary to be mapped.
            return false;
        }
        if (sShapeType == "borderCallout2")
        {
            // LO does not have this type, so it does not necessary to be mapped.
            return false;
        }
        if (sShapeType == "borderCallout3")
        {
            // LO does not have this type, so it does not necessary to be mapped.
            return false;
        }
        if (sShapeType == "bracePair")
        {
            auto aPoint1 = GetAdjustmentPointYValue(0);
            if (!aPoint1.nCurrVal.has_value() || !aPoint1.nMaxVal.has_value()
                || !aPoint1.nMinVal.has_value())
                return false;

            m_pDMLexporter->WriteShapeTransformation(m_xShape, XML_a, IsXFlipped(), IsYFlipped(),
                                                     false, false);
            tools::Long nVal1
                = std::lround(*aPoint1.nCurrVal / (*aPoint1.nMaxVal - *aPoint1.nMinVal) * 25000);
            return StartAVListWriting()
                   && WriteAV(u"adj", OUString(u"val " + OUString::number(nVal1)))
                   && EndAVListWriting();
        }
        if (sShapeType == "bracketPair")
        {
            auto aPoint1 = GetAdjustmentPointYValue(0);
            if (!aPoint1.nCurrVal.has_value() || !aPoint1.nMaxVal.has_value()
                || !aPoint1.nMinVal.has_value())
                return false;

            m_pDMLexporter->WriteShapeTransformation(m_xShape, XML_a, IsXFlipped(), IsYFlipped(),
                                                     false, false);
            tools::Long nVal1
                = std::lround(*aPoint1.nCurrVal / (*aPoint1.nMaxVal - *aPoint1.nMinVal) * 50000);
            return StartAVListWriting()
                   && WriteAV(u"adj", OUString(u"val " + OUString::number(nVal1)))
                   && EndAVListWriting();
        }
        if (sShapeType == "callout1")
        {
            // LO does not have this type, so it does not necessary to be mapped.
            return false;
        }
        if (sShapeType == "callout2")
        {
            // LO does not have this type, so it does not necessary to be mapped.
            return false;
        }
        if (sShapeType == "callout3")
        {
            // LO does not have this type, so it does not necessary to be mapped.
            return false;
        }
        if (sShapeType == "can")
        {
            return false;
            // Do the export as before.
        }
        if (sShapeType == "chartPlus")
        {
            // LO does not have this type, so it does not necessary to be mapped.
            return false;
        }
        if (sShapeType == "chartStar")
        {
            // LO does not have this type, so it does not necessary to be mapped.
            return false;
        }
        if (sShapeType == "chartX")
        {
            // LO does not have this type, so it does not necessary to be mapped.
            return false;
        }
        if (sShapeType == "chord")
        {
            // CustGeom, because LO does not have handle points
            return false;
        }
        if (sShapeType == "circularArrow")
        {
            // LO does not have this type, so it does not necessary to be mapped.
            return false;
        }
        if (sShapeType == "cloud")
        {
            // CustGeom enough
            return false;
        }
        if (sShapeType == "cloudCallout")
        {
            return false;
            // Works fine without this, so export it like before.
        }
        if (sShapeType == "cornerTabs")
        {
            // LO does not have this type, so it does not necessary to be mapped.
            return false;
        }
        if (sShapeType == "cube")
        {
            // Works fine without this, so export it like before.
            return false;
        }
        if (sShapeType == "curvedConnector2")
        {
            // Not necessary to be mapped
            return false;
        }
        if (sShapeType == "curvedConnector3")
        {
            // Not necessary to be mapped
            return false;
        }
        if (sShapeType == "curvedConnector4")
        {
            // Not necessary to be mapped
            return false;
        }
        if (sShapeType == "curvedConnector5")
        {
            // Not necessary to be mapped
            return false;
        }
        if (sShapeType == "curvedDownArrow")
        {
            // LO does not have this type, so it does not necessary to be mapped.
            return false;
        }
        if (sShapeType == "curvedLeftArrow")
        {
            // LO does not have this type, so it does not necessary to be mapped.
            return false;
        }
        if (sShapeType == "curvedRightArrow")
        {
            // LO does not have this type, so it does not necessary to be mapped.
            return false;
        }
        if (sShapeType == "curvedUpArrow")
        {
            // LO does not have this type, so it does not necessary to be mapped.
            return false;
        }
        if (sShapeType == "decagon")
        {
            // LO does not have this type, so it does not necessary to be mapped.
            return false;
        }
        if (sShapeType == "diagStripe")
        {
            // LO does not have this type, so it does not necessary to be mapped.
            return false;
        }
        if (sShapeType == "diamond")
        {
            // It does not have handle points so it do not have to be mapped.
            return false;
        }
        if (sShapeType == "dodecagon")
        {
            // LO does not have this type, so it does not necessary to be mapped.
            return false;
        }
        if (sShapeType == "donut")
        {
            // TODO
            return false;
        }
        if (sShapeType == "doubleWave")
        {
            // LO does not have this type, so it does not necessary to be mapped.
            return false;
        }
        if (sShapeType == "downArrow")
        {
            // TODO
            return false;
        }
        if (sShapeType == "downArrowCallout")
        {
            // TODO
            return false;
        }
        if (sShapeType == "ellipse")
        {
            // Does not have handle points, so preset enough.
            return false;
        }
        if (sShapeType == "ellipseRibbon")
        {
            // LO does not have this type, so it does not necessary to be mapped.
            return false;
        }
        if (sShapeType == "ellipseRibbon2")
        {
            // LO does not have this type, so it does not necessary to be mapped.
            return false;
        }
        if (sShapeType == "flowChartAlternateProcess")
        {
            // Does not have handle points, so preset enough.
            return false;
        }
        if (sShapeType == "flowChartCollate")
        {
            // Does not have handle points, so preset enough.
            return false;
        }
        if (sShapeType == "flowChartConnector")
        {
            // Does not have handle points, so preset enough.
            return false;
        }
        if (sShapeType == "flowChartDecision")
        {
            // Does not have handle points, so preset enough.
            return false;
        }
        if (sShapeType == "flowChartDecision")
        {
            // Does not have handle points, so preset enough.
            return false;
        }
        if (sShapeType == "flowChartDelay")
        {
            // Does not have handle points, so preset enough.
            return false;
        }
        if (sShapeType == "flowChartDisplay")
        {
            // Does not have handle points, so preset enough.
            return false;
        }
        if (sShapeType == "flowChartDocument")
        {
            // Does not have handle points, so preset enough.
            return false;
        }
        if (sShapeType == "flowChartExtract")
        {
            // Does not have handle points, so preset enough.
            return false;
        }
        if (sShapeType == "flowChartInputOutput")
        {
            // Does not have handle points, so preset enough.
            return false;
        }
        if (sShapeType == "flowChartInternalStorage")
        {
            // Does not have handle points, so preset enough.
            return false;
        }
        if (sShapeType == "flowChartMagneticDisk")
        {
            // Does not have handle points, so preset enough.
            return false;
        }
        if (sShapeType == "flowChartMagneticDrum")
        {
            // Does not have handle points, so preset enough.
            return false;
        }
        if (sShapeType == "flowChartMagneticTape")
        {
            // Does not have handle points, so preset enough.
            return false;
        }
        if (sShapeType == "flowChartManualInput")
        {
            // Does not have handle points, so preset enough.
            return false;
        }
        if (sShapeType == "flowChartManualOperation")
        {
            // Does not have handle points, so preset enough.
            return false;
        }
        if (sShapeType == "flowChartMerge")
        {
            // Does not have handle points, so preset enough.
            return false;
        }
        if (sShapeType == "flowChartMultidocument")
        {
            // Does not have handle points, so preset enough.
            return false;
        }
        if (sShapeType == "flowChartOfflineStorage")
        {
            // Does not have handle points, so preset enough.
            return false;
        }
        if (sShapeType == "flowChartOffpageConnector")
        {
            // Does not have handle points, so preset enough.
            return false;
        }
        if (sShapeType == "flowChartOnlineStorage")
        {
            // Does not have handle points, so preset enough.
            return false;
        }
        if (sShapeType == "flowChartOr")
        {
            // Does not have handle points, so preset enough.
            return false;
        }
        if (sShapeType == "flowChartDecision")
        {
            // Does not have handle points, so preset enough.
            return false;
        }
        if (sShapeType == "flowChartPredefinedProcess")
        {
            // Does not have handle points, so preset enough.
            return false;
        }
        if (sShapeType == "flowChartPreparation")
        {
            // Does not have handle points, so preset enough.
            return false;
        }
        if (sShapeType == "flowChartPunchedCard")
        {
            // Does not have handle points, so preset enough.
            return false;
        }
        if (sShapeType == "flowChartPunchedTape")
        {
            // Does not have handle points, so preset enough.
            return false;
        }
        if (sShapeType == "flowChartSort")
        {
            // Does not have handle points, so preset enough.
            return false;
        }
        if (sShapeType == "flowChartSummingJunction")
        {
            // Does not have handle points, so preset enough.
            return false;
        }
        if (sShapeType == "flowChartTerminator")
        {
            // Does not have handle points, so preset enough.
            return false;
        }
        if (sShapeType == "foldedCorner")
        {
            // TODO
            return false;
        }
        if (sShapeType == "frame")
        {
            // TODO
            return false;
        }
        if (sShapeType == "funnel")
        {
            // Not found in word
            return false;
        }
        if (sShapeType == "gear6")
        {
            // Not found in word
            return false;
        }
        if (sShapeType == "gear9")
        {
            // Not found in word
            return false;
        }
        if (sShapeType == "halfFrame")
        {
            // LO does not have this type, not necessary to map
            return false;
        }
        if (sShapeType == "heart")
        {
            // TODO
            return false;
        }
        if (sShapeType == "heptagon")
        {
            // LO does not have this type, not necessary to map
            return false;
        }
        if (sShapeType == "hexagon")
        {
            auto aPoint1 = GetAdjustmentPointXValue(0);
            if (!aPoint1.nCurrVal.has_value() || !aPoint1.nMaxVal.has_value()
                || !aPoint1.nMinVal.has_value())
                return false;

            m_pDMLexporter->WriteShapeTransformation(m_xShape, XML_a, IsXFlipped(), IsYFlipped(),
                                                     false, false);
            tools::Long nMaxVal = 50000 * m_xShape->getSize().Width
                                  / std::min(m_xShape->getSize().Width, m_xShape->getSize().Height);
            tools::Long nVal1
                = std::lround(*aPoint1.nCurrVal / (*aPoint1.nMaxVal - *aPoint1.nMinVal) * nMaxVal);
            return StartAVListWriting()
                   && WriteAV(u"adj", OUString(u"val " + OUString::number(nVal1)))
                   && WriteAV(u"vf", OUString(u"val " + OUString::number(115470)))
                   && EndAVListWriting();
        }
        if (sShapeType == "homePlate")
        {
            // Not found in word
            return false;
        }
        if (sShapeType == "horizontalScroll")
        {
            // TODO
            return false;
        }
        if (sShapeType == "irregularSeal1")
        {
            // Not found in word
            return false;
        }
        if (sShapeType == "irregularSeal2")
        {
            // Not found in word
            return false;
        }
        if (sShapeType == "leftArrow")
        {
            // TODO
            return false;
        }
        if (sShapeType == "leftArrowCallout")
        {
            // TODO
            return false;
        }
        if (sShapeType == "leftBrace")
        {
            // TODO
            return false;
        }
        if (sShapeType == "leftBracket")
        {
            // TODO
            return false;
        }
        if (sShapeType == "leftCircularArrow")
        {
            // TODO
            return false;
        }
        if (sShapeType == "leftRightArrow")
        {
            // TODO
            return false;
        }
        if (sShapeType == "leftRightArrowCallout")
        {
            // TODO
            return false;
        }
        if (sShapeType == "leftRightCircularArrow")
        {
            // Not found in word
            return false;
        }
        if (sShapeType == "leftRightRibbon")
        {
            // LO does not have this type so mapping not necessary
            return false;
        }
        if (sShapeType == "leftRightUpArrow")
        {
            // TODO
            return false;
        }
        if (sShapeType == "leftUpArrow")
        {
            // TODO
            return false;
        }
        if (sShapeType == "lightningBolt")
        {
            // Difference between the SDR and OOXML variants, custgeom?
            return false;
        }
        if (sShapeType == "line")
        {
            // Not necessary
            return false;
        }
        if (sShapeType == "lineInv")
        {
            // Not necessary
            return false;
        }
        if (sShapeType == "mathDivide")
        {
            // LO does not have this type so mapping not necessary
            return false;
        }
        if (sShapeType == "mathEqual")
        {
            // LO does not have this type so mapping not necessary
            return false;
        }
        if (sShapeType == "mathMinus")
        {
            // LO does not have this type so mapping not necessary
            return false;
        }
        if (sShapeType == "mathMultiply")
        {
            // LO does not have this type so mapping not necessary
            return false;
        }
        if (sShapeType == "mathNotEqual")
        {
            // LO does not have this type so mapping not necessary
            return false;
        }
        if (sShapeType == "mathPlus")
        {
            // LO does not have this type so mapping not necessary
            return false;
        }
        if (sShapeType == "nonIsoscelesTrapezoid")
        {
            // TODO
            return false;
        }
        if (sShapeType == "noSmoking")
        {
            // TODO
            return false;
        }
        if (sShapeType == "notchedRightArrow")
        {
            // TODO
            return false;
        }
        if (sShapeType == "octagon")
        {
            auto aPoint1 = GetAdjustmentPointXValue(0);
            if (!aPoint1.nCurrVal.has_value() || !aPoint1.nMaxVal.has_value()
                || !aPoint1.nMinVal.has_value())
                return false;

            m_pDMLexporter->WriteShapeTransformation(m_xShape, XML_a, IsXFlipped(), IsYFlipped(),
                                                     false, false);
            tools::Long nVal1
                = std::lround(*aPoint1.nCurrVal / (*aPoint1.nMaxVal - *aPoint1.nMinVal) * 50000);
            return StartAVListWriting()
                   && WriteAV(u"adj", OUString(u"val " + OUString::number(nVal1)))
                   && EndAVListWriting();
        }
        if (sShapeType == "parallelogram")
        {
            auto aPoint1 = GetAdjustmentPointXValue(0);
            if (!aPoint1.nCurrVal.has_value() || !aPoint1.nMaxVal.has_value()
                || !aPoint1.nMinVal.has_value())
                return false;

            m_pDMLexporter->WriteShapeTransformation(m_xShape, XML_a, IsXFlipped(), IsYFlipped(),
                                                     false, false);
            tools::Long nMaxVal = 100000 * m_xShape->getSize().Width
                                  / std::min(m_xShape->getSize().Width, m_xShape->getSize().Height);
            tools::Long nVal1
                = std::lround(*aPoint1.nCurrVal / (*aPoint1.nMaxVal - *aPoint1.nMinVal) * nMaxVal);
            return StartAVListWriting()
                   && WriteAV(u"adj", OUString(u"val " + OUString::number(nVal1)))
                   && EndAVListWriting();
        }
        if (sShapeType == "pentagon")
        {
            // TODO
            return false;
        }
        if (sShapeType == "pie")
        {
            // TODO
            return false;
        }
        if (sShapeType == "pieWedge")
        {
            // Not found in word.
            return false;
        }
        if (sShapeType == "plaque")
        {
            // TODO
            return false;
        }
        if (sShapeType == "plaqueTabs")
        {
            // LO does not have this, so not necessary to map.
            return false;
        }
        if (sShapeType == "plus")
        {
            auto aPoint1 = GetAdjustmentPointXValue(0);
            if (!aPoint1.nCurrVal.has_value() || !aPoint1.nMaxVal.has_value()
                || !aPoint1.nMinVal.has_value())
                return false;

            m_pDMLexporter->WriteShapeTransformation(m_xShape, XML_a, IsXFlipped(), IsYFlipped(),
                                                     false, false);
            tools::Long nVal1
                = std::lround(*aPoint1.nCurrVal / (*aPoint1.nMaxVal - *aPoint1.nMinVal) * 50000);
            return StartAVListWriting()
                   && WriteAV(u"adj", OUString(u"val " + OUString::number(nVal1)))
                   && EndAVListWriting();
        }
        if (sShapeType == "quadArrow")
        {
            // TODO
            return false;
        }
        if (sShapeType == "quadArrowCallout")
        {
            // TODO
            return false;
        }
        if (sShapeType == "rect")
        {
            // preset enough without AV points.
            return false;
        }
        if (sShapeType == "ribbon")
        {
            // LO does not have this, so not necessary to map.
            return false;
        }
        if (sShapeType == "ribbon2")
        {
            // LO does not have this, so not necessary to map.
            return false;
        }
        if (sShapeType == "rightArrow")
        {
            // TODO
            return false;
        }
        if (sShapeType == "rightArrowCallout")
        {
            // TODO
            return false;
        }
        if (sShapeType == "rightBrace")
        {
            // TODO
            return false;
        }
        if (sShapeType == "rightBracket")
        {
            // TODO
            return false;
        }
        if (sShapeType == "round1Rect")
        {
            // LO does not have this, so not necessary to map.
            return false;
        }
        if (sShapeType == "round2DiagRect")
        {
            // LO does not have this, so not necessary to map.
            return false;
        }
        if (sShapeType == "round2SameRect")
        {
            // LO does not have this, so not necessary to map.
            return false;
        }
        if (sShapeType == "roundRect")
        {
            tools::Long nVal1 = 0;
            if (m_xShape->getSize().Width >= m_xShape->getSize().Height)
            {
                auto aPointX = GetAdjustmentPointXValue(0);
                if (!aPointX.nCurrVal.has_value() || !aPointX.nMaxVal.has_value()
                    || !aPointX.nMinVal.has_value())
                    return false;
                nVal1 = std::lround(*aPointX.nCurrVal / (*aPointX.nMaxVal - *aPointX.nMinVal)
                                    * 50000);
            }
            else
            {
                auto aPointY = GetAdjustmentPointYValue(0);
                if (!aPointY.nCurrVal.has_value() || !aPointY.nMaxVal.has_value()
                    || !aPointY.nMinVal.has_value())
                    return false;
                nVal1 = std::lround(*aPointY.nCurrVal / (*aPointY.nMaxVal - *aPointY.nMinVal)
                                    * 50000);
            }

            m_pDMLexporter->WriteShapeTransformation(m_xShape, XML_a, IsXFlipped(), IsYFlipped(),
                                                     false, false);
            return StartAVListWriting()
                   && WriteAV(u"adj", OUString(u"val " + OUString::number(nVal1)))
                   && EndAVListWriting();
        }
        if (sShapeType == "rtTriangle")
        {
            // Does not have AV points not necessary to map
            return false;
        }
        if (sShapeType == "smileyFace")
        {
            // TODO
            return false;
        }
        if (sShapeType == "snip1Rect")
        {
            // LO does not have this, so not necessary to map.
            return false;
        }
        if (sShapeType == "snip2DiagRect")
        {
            // LO does not have this, so not necessary to map.
            return false;
        }
        if (sShapeType == "snip2SameRect")
        {
            // LO does not have this, so not necessary to map.
            return false;
        }
        if (sShapeType == "snipRoundRect")
        {
            // LO does not have this, so not necessary to map.
            return false;
        }
        if (sShapeType == "squareTabs")
        {
            // LO does not have this, so not necessary to map.
            return false;
        }
        if (sShapeType == "star10")
        {
            // LO does not have this, so not necessary to map.
            return false;
        }
        if (sShapeType == "star12")
        {
            // TODO
            return false;
        }
        if (sShapeType == "star16")
        {
            // LO does not have this, so not necessary to map.
            return false;
        }
        if (sShapeType == "star24")
        {
            // TODO
            return false;
        }
        if (sShapeType == "star32")
        {
            // LO does not have this, so not necessary to map.
            return false;
        }
        if (sShapeType == "star4")
        {
            // TODO
            return false;
        }
        if (sShapeType == "star5")
        {
            // TODO
            return false;
        }
        if (sShapeType == "star6")
        {
            // TODO
            return false;
        }
        if (sShapeType == "star7")
        {
            // LO does not have this, so not necessary to map.
            return false;
        }
        if (sShapeType == "star8")
        {
            // TODO
            return false;
        }
        if (sShapeType == "straightConnector1")
        {
            // Not necessary to map.
            return false;
        }
        if (sShapeType == "stripedRightArrow")
        {
            // TODO
            return false;
        }
        if (sShapeType == "sun")
        {
            // TODO
            return false;
        }
        if (sShapeType == "swooshArrow")
        {
            // Not found in word.
            return false;
        }
        if (sShapeType == "teardrop")
        {
            // TODO
            return false;
        }
        if (sShapeType == "trapezoid")
        {
            // Preset enough.
            return false;
        }
        if (sShapeType == "triangle")
        {
            auto aPoint1 = GetAdjustmentPointXValue(0);
            if (!aPoint1.nCurrVal.has_value() || !aPoint1.nMaxVal.has_value()
                || !aPoint1.nMinVal.has_value())
                return false;

            m_pDMLexporter->WriteShapeTransformation(m_xShape, XML_a, IsXFlipped(), IsYFlipped(),
                                                     false, false);
            tools::Long nMaxVal = 100000;
            tools::Long nVal1
                = std::lround(*aPoint1.nCurrVal / (*aPoint1.nMaxVal - *aPoint1.nMinVal) * nMaxVal);
            return StartAVListWriting()
                   && WriteAV(u"adj", OUString(u"val " + OUString::number(nVal1)))
                   && EndAVListWriting();
        }
        if (sShapeType == "upArrowCallout")
        {
            // TODO
            return false;
        }
        if (sShapeType == "upDownArrow")
        {
            // TODO
            return false;
        }
        if (sShapeType == "upArrow")
        {
            // TODO
            return false;
        }
        if (sShapeType == "upDownArrowCallout")
        {
            // TODO
            return false;
        }
        if (sShapeType == "uturnArrow")
        {
            // LO does not have like this.
            return false;
        }
        if (sShapeType == "verticalScroll")
        {
            // TODO
            return false;
        }
        if (sShapeType == "wave")
        {
            // LO does not have.
            return false;
        }
        if (sShapeType == "wedgeEllipseCallout")
        {
            // TODO
            return false;
        }
        if (sShapeType == "wedgeRectCallout")
        {
            // TODO
            return false;
        }
        if (sShapeType == "wedgeRoundRectCallout")
        {
            // TODO
            return false;
        }
    }
    catch (...)
    {
        // Problem detected with the writing, aborting and trying to find another way.
        return false;
    }

    // Default, nothing happened return.
    return false;
};
}
