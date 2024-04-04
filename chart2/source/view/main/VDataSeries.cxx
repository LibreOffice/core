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

#include <limits>
#include <memory>
#include <VDataSeries.hxx>
#include <DataSeries.hxx>
#include <DataSeriesProperties.hxx>
#include <ObjectIdentifier.hxx>
#include <CommonConverters.hxx>
#include <LabelPositionHelper.hxx>
#include <ChartType.hxx>
#include <ChartTypeHelper.hxx>
#include <RegressionCurveCalculator.hxx>
#include <RegressionCurveHelper.hxx>
#include <unonames.hxx>

#include <com/sun/star/chart/MissingValueTreatment.hpp>
#include <com/sun/star/chart2/DataPointLabel.hpp>
#include <com/sun/star/chart2/Symbol.hpp>
#include <com/sun/star/chart2/XRegressionCurveCalculator.hpp>
#include <com/sun/star/chart2/RelativePosition.hpp>
#include <com/sun/star/chart2/RelativeSize.hpp>

#include <o3tl/compare.hxx>
#include <osl/diagnose.h>
#include <tools/color.hxx>
#include <comphelper/diagnose_ex.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XPropertyState.hpp>

namespace chart {

using namespace ::com::sun::star;
using namespace ::com::sun::star::chart2;
using ::com::sun::star::uno::Reference;
using namespace ::chart::DataSeriesProperties;

void VDataSequence::init( const uno::Reference< data::XDataSequence >& xModel )
{
    m_xModel = xModel;
    m_aValues = DataSequenceToDoubleSequence( xModel );
}

bool VDataSequence::is() const
{
    return m_xModel.is();
}
void VDataSequence::clear()
{
    m_xModel = nullptr;
    m_aValues.realloc(0);
}

double VDataSequence::getValue( sal_Int32 index ) const
{
    if( 0<=index && index<m_aValues.getLength() )
        return m_aValues[index];
    return std::numeric_limits<double>::quiet_NaN();
}

sal_Int32 VDataSequence::detectNumberFormatKey( sal_Int32 index ) const
{
    sal_Int32 nNumberFormatKey = -1;

    // -1 is allowed and means a key for the whole sequence
    if( -1<=index && index<m_aValues.getLength() && m_xModel.is())
    {
        nNumberFormatKey = m_xModel->getNumberFormatKeyByIndex( index );
    }

    return nNumberFormatKey;
}

sal_Int32 VDataSequence::getLength() const
{
    return m_aValues.getLength();
}

namespace
{
struct lcl_LessXOfPoint
{
    bool operator() ( const std::vector< double >& first,
                             const std::vector< double >& second )
    {
        if( !first.empty() && !second.empty() )
        {
            return o3tl::strong_order(first[0], second[0]) < 0;
        }
        return false;
    }
};

void lcl_clearIfNoValuesButTextIsContained( VDataSequence& rData, const uno::Reference<data::XDataSequence>& xDataSequence )
{
    //#i71686#, #i101968#, #i102428#
    sal_Int32 nCount = rData.m_aValues.getLength();
    for( sal_Int32 i = 0; i < nCount; ++i )
    {
        if( !std::isnan( rData.m_aValues[i] ) )
            return;
    }
    //no double value is contained
    //is there any text?
    uno::Sequence< OUString > aStrings( DataSequenceToStringSequence( xDataSequence ) );
    sal_Int32 nTextCount = aStrings.getLength();
    for( sal_Int32 j = 0; j < nTextCount; ++j )
    {
        if( !aStrings[j].isEmpty() )
        {
            rData.clear();
            return;
        }
    }
    //no content at all
}

void lcl_maybeReplaceNanWithZero( double& rfValue, sal_Int32 nMissingValueTreatment )
{
    if( nMissingValueTreatment == css::chart::MissingValueTreatment::USE_ZERO
        && (std::isnan(rfValue) || std::isinf(rfValue)) )
            rfValue = 0.0;
}

}

VDataSeries::VDataSeries( const rtl::Reference< DataSeries >& xDataSeries )
    : m_nPolygonIndex(0)
    , m_fLogicMinX(0.0)
    , m_fLogicMaxX(0.0)
    , m_fLogicZPos(0.0)
    , m_xDataSeries(xDataSeries)
    , m_nPointCount(0)
    , m_pValueSequenceForDataLabelNumberFormatDetection(&m_aValues_Y)
    , m_fXMeanValue(std::numeric_limits<double>::quiet_NaN())
    , m_fYMeanValue(std::numeric_limits<double>::quiet_NaN())
    , m_eStackingDirection(StackingDirection_NO_STACKING)
    , m_nAxisIndex(0)
    , m_bConnectBars(false)
    , m_bGroupBarsPerAxis(true)
    , m_nStartingAngle(90)
    , m_ePieChartSubType(PieChartSubType_NONE)
    , m_nGlobalSeriesIndex(0)
    , m_nCurrentAttributedPoint(-1)
    , m_nMissingValueTreatment(css::chart::MissingValueTreatment::LEAVE_GAP)
    , m_bAllowPercentValueInDataLabel(false)
    , mpOldSeries(nullptr)
    , mnPercent(0.0)
{
    m_xDataSeriesProps = m_xDataSeries;

    const std::vector< uno::Reference< chart2::data::XLabeledDataSequence > > & aDataSequences =
            m_xDataSeries->getDataSequences2();

    for(sal_Int32 nN = aDataSequences.size();nN--;)
    {
        uno::Reference<data::XDataSequence>  xDataSequence( aDataSequences[nN]->getValues());
        uno::Reference<beans::XPropertySet> xProp(xDataSequence, uno::UNO_QUERY );
        if( xProp.is())
        {
            try
            {
                uno::Any aARole = xProp->getPropertyValue("Role");
                OUString aRole;
                aARole >>= aRole;

                if (aRole == "values-x")
                {
                    m_aValues_X.init( xDataSequence );
                    lcl_clearIfNoValuesButTextIsContained( m_aValues_X, xDataSequence );
                }
                else if (aRole =="values-y")
                    m_aValues_Y.init( xDataSequence );
                else if (aRole == "values-min")
                    m_aValues_Y_Min.init( xDataSequence );
                else if (aRole == "values-max")
                    m_aValues_Y_Max.init( xDataSequence );
                else if (aRole == "values-first")
                    m_aValues_Y_First.init( xDataSequence );
                else if (aRole == "values-last")
                    m_aValues_Y_Last.init( xDataSequence );
                else if (aRole == "values-size")
                    m_aValues_Bubble_Size.init( xDataSequence );
                else
                {
                    VDataSequence aSequence;
                    aSequence.init(xDataSequence);
                    m_PropertyMap.insert(std::make_pair(aRole, aSequence));
                }
            }
            catch( const uno::Exception& )
            {
                TOOLS_WARN_EXCEPTION("chart2", "" );
            }
        }
    }

    //determine the point count
    m_nPointCount = m_aValues_Y.getLength();
    {
        if( m_nPointCount < m_aValues_Bubble_Size.getLength() )
            m_nPointCount = m_aValues_Bubble_Size.getLength();
        if( m_nPointCount < m_aValues_Y_Min.getLength() )
            m_nPointCount = m_aValues_Y_Min.getLength();
        if( m_nPointCount < m_aValues_Y_Max.getLength() )
            m_nPointCount = m_aValues_Y_Max.getLength();
        if( m_nPointCount < m_aValues_Y_First.getLength() )
            m_nPointCount = m_aValues_Y_First.getLength();
        if( m_nPointCount < m_aValues_Y_Last.getLength() )
            m_nPointCount = m_aValues_Y_Last.getLength();
    }

    if( !xDataSeries.is())
        return;

    try
    {
        // "AttributedDataPoints"
        xDataSeries->getFastPropertyValue(PROP_DATASERIES_ATTRIBUTED_DATA_POINTS) >>= m_aAttributedDataPointIndexList;

        xDataSeries->getFastPropertyValue(PROP_DATASERIES_STACKING_DIRECTION) >>= m_eStackingDirection; // "StackingDirection"

        xDataSeries->getFastPropertyValue(PROP_DATASERIES_ATTACHED_AXIS_INDEX) >>= m_nAxisIndex; // "AttachedAxisIndex"
        if(m_nAxisIndex<0)
            m_nAxisIndex=0;
    }
    catch( const uno::Exception& )
    {
        TOOLS_WARN_EXCEPTION("chart2", "" );
    }
}

VDataSeries::~VDataSeries()
{
}

void VDataSeries::doSortByXValues()
{
    if( !(m_aValues_X.is() && m_aValues_X.m_aValues.hasElements()) )
        return;

    //prepare a vector for sorting
    std::vector< std::vector< double > > aTmp;//outer vector are points, inner vector are the different values of the point
    sal_Int32 nPointIndex = 0;
    for( nPointIndex=0; nPointIndex < m_nPointCount; nPointIndex++ )
    {
        aTmp.push_back(
                        { ((nPointIndex < m_aValues_X.m_aValues.getLength()) ? m_aValues_X.m_aValues[nPointIndex]
                                                                           : std::numeric_limits<double>::quiet_NaN()),
                          ((nPointIndex < m_aValues_Y.m_aValues.getLength()) ? m_aValues_Y.m_aValues[nPointIndex]
                                                                           : std::numeric_limits<double>::quiet_NaN())
                        }
                      );
    }

    //do sort
    std::stable_sort( aTmp.begin(), aTmp.end(), lcl_LessXOfPoint() );

    //fill the sorted points back to the members
    m_aValues_X.m_aValues.realloc( m_nPointCount );
    auto pDoublesX = m_aValues_X.m_aValues.getArray();
    m_aValues_Y.m_aValues.realloc( m_nPointCount );
    auto pDoublesY = m_aValues_Y.m_aValues.getArray();

    for( nPointIndex=0; nPointIndex < m_nPointCount; nPointIndex++ )
    {
        pDoublesX[nPointIndex]=aTmp[nPointIndex][0];
        pDoublesY[nPointIndex]=aTmp[nPointIndex][1];
    }
}

void VDataSeries::releaseShapes()
{
    m_xGroupShape.clear();
    m_xLabelsGroupShape.clear();
    m_xErrorXBarsGroupShape.clear();
    m_xErrorYBarsGroupShape.clear();
    m_xFrontSubGroupShape.clear();
    m_xBackSubGroupShape.clear();

    m_aPolyPolygonShape3D.clear();
    m_nPolygonIndex = 0;
}

const rtl::Reference<::chart::DataSeries>& VDataSeries::getModel() const
{
    return m_xDataSeries;
}

void VDataSeries::setCategoryXAxis()
{
    m_aValues_X.clear();
    m_bAllowPercentValueInDataLabel = true;
}

void VDataSeries::setXValues( const Reference< chart2::data::XDataSequence >& xValues )
{
    m_aValues_X.clear();
    m_aValues_X.init( xValues );
    m_bAllowPercentValueInDataLabel = true;
}

void VDataSeries::setXValuesIfNone( const Reference< chart2::data::XDataSequence >& xValues )
{
    if( m_aValues_X.is() )
        return;

    m_aValues_X.init( xValues );
    lcl_clearIfNoValuesButTextIsContained( m_aValues_X, xValues );
}

void VDataSeries::setGlobalSeriesIndex( sal_Int32 nGlobalSeriesIndex )
{
    m_nGlobalSeriesIndex = nGlobalSeriesIndex;
}

void VDataSeries::setParticle( const OUString& rSeriesParticle )
{
    m_aSeriesParticle = rSeriesParticle;

    //get CID
    m_aCID = ObjectIdentifier::createClassifiedIdentifierForParticle( m_aSeriesParticle );
    m_aPointCID_Stub = ObjectIdentifier::createSeriesSubObjectStub( OBJECTTYPE_DATA_POINT, m_aSeriesParticle );

    m_aLabelCID_Stub = ObjectIdentifier::createClassifiedIdentifierWithParent(
                        OBJECTTYPE_DATA_LABEL, u"", getLabelsCID() );
}
OUString VDataSeries::getErrorBarsCID(bool bYError) const
{
    OUString aChildParticle( ObjectIdentifier::getStringForType(
                                      bYError ? OBJECTTYPE_DATA_ERRORS_Y : OBJECTTYPE_DATA_ERRORS_X )
                             + "=" );

    return ObjectIdentifier::createClassifiedIdentifierForParticles(
            m_aSeriesParticle, aChildParticle );
}
OUString VDataSeries::getLabelsCID() const
{
    OUString aChildParticle( ObjectIdentifier::getStringForType( OBJECTTYPE_DATA_LABELS ) + "=" );

    return ObjectIdentifier::createClassifiedIdentifierForParticles(
            m_aSeriesParticle, aChildParticle );
}
OUString VDataSeries::getDataCurveCID( sal_Int32 nCurveIndex, bool bAverageLine ) const
{
    return ObjectIdentifier::createDataCurveCID( m_aSeriesParticle, nCurveIndex, bAverageLine );
}

OUString VDataSeries::getDataCurveEquationCID( sal_Int32 nCurveIndex ) const
{
    return ObjectIdentifier::createDataCurveEquationCID( m_aSeriesParticle, nCurveIndex );
}
void VDataSeries::setPageReferenceSize( const awt::Size & rPageRefSize )
{
    m_aReferenceSize = rPageRefSize;
}

void VDataSeries::setConnectBars( bool bConnectBars )
{
    m_bConnectBars = bConnectBars;
}

bool VDataSeries::getConnectBars() const
{
    return m_bConnectBars;
}

void VDataSeries::setGroupBarsPerAxis( bool bGroupBarsPerAxis )
{
    m_bGroupBarsPerAxis = bGroupBarsPerAxis;
}

bool VDataSeries::getGroupBarsPerAxis() const
{
    return m_bGroupBarsPerAxis;
}

void VDataSeries::setStartingAngle( sal_Int32 nStartingAngle )
{
    m_nStartingAngle = nStartingAngle;
}

sal_Int32 VDataSeries::getStartingAngle() const
{
    return m_nStartingAngle;
}

void VDataSeries::setPieChartSubType(chart2::PieChartSubType eSubType)
{
    m_ePieChartSubType = eSubType;
}

chart2::StackingDirection VDataSeries::getStackingDirection() const
{
    return m_eStackingDirection;
}

sal_Int32 VDataSeries::getAttachedAxisIndex() const
{
    return m_nAxisIndex;
}

void VDataSeries::setAttachedAxisIndex( sal_Int32 nAttachedAxisIndex )
{
    if( nAttachedAxisIndex < 0 )
        nAttachedAxisIndex = 0;
    m_nAxisIndex = nAttachedAxisIndex;
}

double VDataSeries::getXValue( sal_Int32 index ) const
{
    double fRet = std::numeric_limits<double>::quiet_NaN();
    if(m_aValues_X.is())
    {
        if( 0<=index && index<m_aValues_X.getLength() )
        {
            fRet = m_aValues_X.m_aValues[index];
            if(mpOldSeries && index < mpOldSeries->m_aValues_X.getLength())
            {
                double nOldVal = mpOldSeries->m_aValues_X.m_aValues[index];
                fRet = nOldVal + (fRet - nOldVal) * mnPercent;
            }
        }
    }
    else
    {
        // #i70133# always return correct X position - needed for short data series
        if( 0<=index /*&& index < m_nPointCount*/ )
            fRet = index+1;//first category (index 0) matches with real number 1.0
    }
    lcl_maybeReplaceNanWithZero( fRet, getMissingValueTreatment() );
    return fRet;
}

double VDataSeries::getYValue( sal_Int32 index ) const
{
    double fRet = std::numeric_limits<double>::quiet_NaN();
    if(m_aValues_Y.is())
    {
        if( 0<=index && index<m_aValues_Y.getLength() )
        {
            fRet = m_aValues_Y.m_aValues[index];
            if(mpOldSeries && index < mpOldSeries->m_aValues_Y.getLength())
            {
                double nOldVal = mpOldSeries->m_aValues_Y.m_aValues[index];
                fRet = nOldVal + (fRet - nOldVal) * mnPercent;
            }
        }
    }
    else
    {
        // #i70133# always return correct X position - needed for short data series
        if( 0<=index /*&& index < m_nPointCount*/ )
            fRet = index+1;//first category (index 0) matches with real number 1.0
    }
    lcl_maybeReplaceNanWithZero( fRet, getMissingValueTreatment() );
    return fRet;
}

void VDataSeries::getMinMaxXValue(double& fMin, double& fMax) const
{
    fMax = std::numeric_limits<double>::quiet_NaN();
    fMin = std::numeric_limits<double>::quiet_NaN();

    uno::Sequence< double > aValuesX = getAllX();

    if(!aValuesX.hasElements())
        return;

    sal_Int32 i = 0;
    while ( i < aValuesX.getLength() && std::isnan(aValuesX[i]) )
        i++;
    if ( i < aValuesX.getLength() )
        fMax = fMin = aValuesX[i++];

    for ( ; i < aValuesX.getLength(); i++)
    {
        const double aValue = aValuesX[i];
        if ( aValue > fMax)
        {
            fMax = aValue;
        }
        else if ( aValue < fMin)
        {
            fMin = aValue;
        }
    }
}
double VDataSeries::getY_Min( sal_Int32 index ) const
{
    return m_aValues_Y_Min.getValue( index );
}
double VDataSeries::getY_Max( sal_Int32 index ) const
{
    return m_aValues_Y_Max.getValue( index );
}
double VDataSeries::getY_First( sal_Int32 index ) const
{
    return m_aValues_Y_First.getValue( index );
}
double VDataSeries::getY_Last( sal_Int32 index ) const
{
    return m_aValues_Y_Last.getValue( index );
}
double VDataSeries::getBubble_Size( sal_Int32 index ) const
{
    double nNewVal = m_aValues_Bubble_Size.getValue( index );
    if(mpOldSeries && index < mpOldSeries->m_aValues_Bubble_Size.getLength())
    {
        double nOldVal = mpOldSeries->m_aValues_Bubble_Size.getValue( index );
        nNewVal = nOldVal + (nNewVal - nOldVal) * mnPercent;
    }

    return nNewVal;
}

bool VDataSeries::hasExplicitNumberFormat( sal_Int32 nPointIndex, bool bForPercentage ) const
{
    OUString aPropName = bForPercentage ? OUString("PercentageNumberFormat") : CHART_UNONAME_NUMFMT;
    bool bHasNumberFormat = false;
    bool bLinkToSource = true;
    uno::Reference< beans::XPropertySet > xPointProp( getPropertiesOfPoint( nPointIndex ));
    if( xPointProp.is() && (xPointProp->getPropertyValue(CHART_UNONAME_LINK_TO_SRC_NUMFMT) >>= bLinkToSource))
    {
        sal_Int32 nNumberFormat = -1;
        if( !bLinkToSource && (xPointProp->getPropertyValue(aPropName) >>= nNumberFormat))
            bHasNumberFormat = true;
    }
    return bHasNumberFormat;
}
sal_Int32 VDataSeries::getExplicitNumberFormat( sal_Int32 nPointIndex, bool bForPercentage ) const
{
    OUString aPropName = bForPercentage ? OUString("PercentageNumberFormat") : CHART_UNONAME_NUMFMT;
    sal_Int32 nNumberFormat = -1;
    uno::Reference< beans::XPropertySet > xPointProp( getPropertiesOfPoint( nPointIndex ));
    if( xPointProp.is() )
        xPointProp->getPropertyValue(aPropName) >>= nNumberFormat;
    return nNumberFormat;
}
void VDataSeries::setRoleOfSequenceForDataLabelNumberFormatDetection( std::u16string_view rRole )
{
    if (rRole == u"values-y")
        m_pValueSequenceForDataLabelNumberFormatDetection = &m_aValues_Y;
    else if (rRole == u"values-size")
        m_pValueSequenceForDataLabelNumberFormatDetection = &m_aValues_Bubble_Size;
    else if (rRole == u"values-min")
        m_pValueSequenceForDataLabelNumberFormatDetection = &m_aValues_Y_Min;
    else if (rRole == u"values-max")
        m_pValueSequenceForDataLabelNumberFormatDetection = &m_aValues_Y_Max;
    else if (rRole == u"values-first")
        m_pValueSequenceForDataLabelNumberFormatDetection = &m_aValues_Y_First;
    else if (rRole == u"values-last")
        m_pValueSequenceForDataLabelNumberFormatDetection = &m_aValues_Y_Last;
    else if (rRole == u"values-x")
        m_pValueSequenceForDataLabelNumberFormatDetection = &m_aValues_X;
}
sal_Int32 VDataSeries::detectNumberFormatKey( sal_Int32 index ) const
{
    sal_Int32 nRet = 0;
    if( m_pValueSequenceForDataLabelNumberFormatDetection )
        nRet = m_pValueSequenceForDataLabelNumberFormatDetection->detectNumberFormatKey( index );
    return nRet;
}

sal_Int32 VDataSeries::getLabelPlacement( sal_Int32 nPointIndex, const rtl::Reference< ChartType >& xChartType, bool bSwapXAndY ) const
{
    sal_Int32 nLabelPlacement=0;
    try
    {
        uno::Reference< beans::XPropertySet > xPointProps( getPropertiesOfPoint( nPointIndex ) );
        if( xPointProps.is() )
            xPointProps->getPropertyValue("LabelPlacement") >>= nLabelPlacement;

        const uno::Sequence < sal_Int32 > aAvailablePlacements( ChartTypeHelper::getSupportedLabelPlacements(
                xChartType, bSwapXAndY, m_xDataSeries ) );

        for( sal_Int32 n : aAvailablePlacements )
            if( n == nLabelPlacement )
                return nLabelPlacement; //ok

        //otherwise use the first supported one
        if( aAvailablePlacements.hasElements() )
        {
            nLabelPlacement = aAvailablePlacements[0];
            if( xPointProps.is() )
                xPointProps->setPropertyValue("LabelPlacement", uno::Any(nLabelPlacement));
            return nLabelPlacement;
        }

        OSL_FAIL("no label placement supported");
    }
    catch( const uno::Exception& )
    {
        TOOLS_WARN_EXCEPTION("chart2", "" );
    }
    return nLabelPlacement;
}

awt::Point VDataSeries::getLabelPosition( awt::Point aTextShapePos, sal_Int32 nPointIndex ) const
{
    awt::Point aPos(-1, -1);
    try
    {
        RelativePosition aCustomLabelPosition;
        uno::Reference< beans::XPropertySet > xPointProps(getPropertiesOfPoint(nPointIndex));
        if( xPointProps.is() && (xPointProps->getPropertyValue("CustomLabelPosition") >>= aCustomLabelPosition))
        {
            aPos.X = static_cast<sal_Int32>(aCustomLabelPosition.Primary * m_aReferenceSize.Width) + aTextShapePos.X;
            aPos.Y = static_cast<sal_Int32>(aCustomLabelPosition.Secondary * m_aReferenceSize.Height) + aTextShapePos.Y;
        }
    }
    catch (const uno::Exception&)
    {
        TOOLS_WARN_EXCEPTION("chart2", "");
    }
    return aPos;
}

bool VDataSeries::isLabelCustomPos(sal_Int32 nPointIndex) const
{
    bool bCustom = false;
    try
    {
        if( isAttributedDataPoint(nPointIndex) )
        {
            uno::Reference< beans::XPropertySet > xPointProps(m_xDataSeries->getDataPointByIndex(nPointIndex));
            RelativePosition aCustomLabelPosition;
            if( xPointProps.is() && (xPointProps->getPropertyValue("CustomLabelPosition") >>= aCustomLabelPosition) )
                bCustom = true;
        }
    }
    catch (const uno::Exception&)
    {
        TOOLS_WARN_EXCEPTION("chart2", "");
    }
    return bCustom;
}

awt::Size VDataSeries::getLabelCustomSize(sal_Int32 nPointIndex) const
{
    awt::Size aSize(-1, -1);
    try
    {
        RelativeSize aCustomLabelSize;
        const uno::Reference<beans::XPropertySet> xPointProps(getPropertiesOfPoint(nPointIndex));
        if (xPointProps.is() && (xPointProps->getPropertyValue("CustomLabelSize") >>= aCustomLabelSize))
        {
            aSize.Width = static_cast<sal_Int32>(aCustomLabelSize.Primary * m_aReferenceSize.Width);
            aSize.Height = static_cast<sal_Int32>(aCustomLabelSize.Secondary * m_aReferenceSize.Height);
        }
    }
    catch (const uno::Exception&)
    {
        DBG_UNHANDLED_EXCEPTION("chart2");
    }
    return aSize;
}

double VDataSeries::getMinimumofAllDifferentYValues( sal_Int32 index ) const
{
    double fMin = std::numeric_limits<double>::infinity();

    if( !m_aValues_Y.is() &&
        (m_aValues_Y_Min.is() || m_aValues_Y_Max.is()
        || m_aValues_Y_First.is() || m_aValues_Y_Last.is() ) )
    {
        double fY_Min = getY_Min( index );
        double fY_Max = getY_Max( index );
        double fY_First = getY_First( index );
        double fY_Last = getY_Last( index );

        if(fMin>fY_First)
            fMin=fY_First;
        if(fMin>fY_Last)
            fMin=fY_Last;
        if(fMin>fY_Min)
            fMin=fY_Min;
        if(fMin>fY_Max)
            fMin=fY_Max;
    }
    else
    {
        double fY = getYValue( index );
        if(fMin>fY)
            fMin=fY;
    }

    if( std::isinf(fMin) )
        return std::numeric_limits<double>::quiet_NaN();

    return fMin;
}

double VDataSeries::getMaximumofAllDifferentYValues( sal_Int32 index ) const
{
    double fMax = -std::numeric_limits<double>::infinity();

    if( !m_aValues_Y.is() &&
        (m_aValues_Y_Min.is() || m_aValues_Y_Max.is()
        || m_aValues_Y_First.is() || m_aValues_Y_Last.is() ) )
    {
        double fY_Min = getY_Min( index );
        double fY_Max = getY_Max( index );
        double fY_First = getY_First( index );
        double fY_Last = getY_Last( index );

        if(fMax<fY_First)
            fMax=fY_First;
        if(fMax<fY_Last)
            fMax=fY_Last;
        if(fMax<fY_Min)
            fMax=fY_Min;
        if(fMax<fY_Max)
            fMax=fY_Max;
    }
    else
    {
        double fY = getYValue( index );
        if(fMax<fY)
            fMax=fY;
    }

    if( std::isinf(fMax) )
        return std::numeric_limits<double>::quiet_NaN();

    return fMax;
}

uno::Sequence< double > const & VDataSeries::getAllX() const
{
    if(!m_aValues_X.is() && !m_aValues_X.getLength() && m_nPointCount)
    {
        //init x values from category indexes
        //first category (index 0) matches with real number 1.0
        m_aValues_X.m_aValues.realloc( m_nPointCount );
        auto pDoubles = m_aValues_X.m_aValues.getArray();
        for(sal_Int32 nN=m_aValues_X.getLength();nN--;)
            pDoubles[nN] = nN+1;
    }
    return m_aValues_X.m_aValues;
}

uno::Sequence< double > const & VDataSeries::getAllY() const
{
    if(!m_aValues_Y.is() && !m_aValues_Y.getLength() && m_nPointCount)
    {
        //init y values from indexes
        //first y-value (index 0) matches with real number 1.0
        m_aValues_Y.m_aValues.realloc( m_nPointCount );
        auto pDoubles = m_aValues_Y.m_aValues.getArray();
        for(sal_Int32 nN=m_aValues_Y.getLength();nN--;)
            pDoubles[nN] = nN+1;
    }
    return m_aValues_Y.m_aValues;
}

double VDataSeries::getXMeanValue() const
{
    if( std::isnan( m_fXMeanValue ) )
    {
        rtl::Reference< RegressionCurveCalculator > xCalculator( RegressionCurveHelper::createRegressionCurveCalculatorByServiceName( u"com.sun.star.chart2.MeanValueRegressionCurve" ) );
        uno::Sequence< double > aXValuesDummy;
        xCalculator->recalculateRegression( aXValuesDummy, getAllX() );
        m_fXMeanValue = xCalculator->getCurveValue( 1.0 );
    }
    return m_fXMeanValue;
}

double VDataSeries::getYMeanValue() const
{
    if( std::isnan( m_fYMeanValue ) )
    {
        rtl::Reference< RegressionCurveCalculator > xCalculator(
            RegressionCurveHelper::createRegressionCurveCalculatorByServiceName(u"com.sun.star.chart2.MeanValueRegressionCurve"));
        uno::Sequence< double > aXValuesDummy;
        xCalculator->recalculateRegression( aXValuesDummy, getAllY() );
        m_fYMeanValue = xCalculator->getCurveValue( 1.0 );
    }
    return m_fYMeanValue;
}

static std::optional<Symbol> getSymbolPropertiesFromPropertySet( const uno::Reference< beans::XPropertySet >& xProp )
{
    Symbol aSymbolProps;
    try
    {
        if( xProp->getPropertyValue("Symbol") >>= aSymbolProps )
        {
            //use main color to fill symbols
            xProp->getPropertyValue("Color") >>= aSymbolProps.FillColor;
            // border of symbols always same as fill color
            aSymbolProps.BorderColor = aSymbolProps.FillColor;
        }
        else
            return std::nullopt;
    }
    catch(const uno::Exception &)
    {
        TOOLS_WARN_EXCEPTION("chart2", "" );
    }
    return aSymbolProps;
}

Symbol* VDataSeries::getSymbolProperties( sal_Int32 index ) const
{
    Symbol* pRet=nullptr;
    if( isAttributedDataPoint( index ) )
    {
        adaptPointCache( index );
        if (!m_oSymbolProperties_AttributedPoint)
            m_oSymbolProperties_AttributedPoint
                = getSymbolPropertiesFromPropertySet(getPropertiesOfPoint(index));
        pRet = &*m_oSymbolProperties_AttributedPoint;
        //if a single data point does not have symbols but the dataseries itself has symbols
        //we create an invisible symbol shape to enable selection of that point
        if( !pRet || pRet->Style == SymbolStyle_NONE )
        {
            if (!m_oSymbolProperties_Series)
                m_oSymbolProperties_Series
                    = getSymbolPropertiesFromPropertySet(getPropertiesOfSeries());
            if( m_oSymbolProperties_Series && m_oSymbolProperties_Series->Style != SymbolStyle_NONE )
            {
                if (!m_oSymbolProperties_InvisibleSymbolForSelection)
                {
                    m_oSymbolProperties_InvisibleSymbolForSelection.emplace();
                    m_oSymbolProperties_InvisibleSymbolForSelection->Style = SymbolStyle_STANDARD;
                    m_oSymbolProperties_InvisibleSymbolForSelection->StandardSymbol = 0;//square
                    m_oSymbolProperties_InvisibleSymbolForSelection->Size = com::sun::star::awt::Size(0, 0);//tdf#126033
                    m_oSymbolProperties_InvisibleSymbolForSelection->BorderColor = 0xff000000;//invisible
                    m_oSymbolProperties_InvisibleSymbolForSelection->FillColor = 0xff000000;//invisible
                }
                pRet = &*m_oSymbolProperties_InvisibleSymbolForSelection;
            }
        }
    }
    else
    {
        if (!m_oSymbolProperties_Series)
            m_oSymbolProperties_Series
                = getSymbolPropertiesFromPropertySet(getPropertiesOfSeries());
        pRet = &*m_oSymbolProperties_Series;
    }

    if( pRet && pRet->Style == SymbolStyle_AUTO )
    {
        pRet->Style = SymbolStyle_STANDARD;

        sal_Int32 nIndex = m_nGlobalSeriesIndex;
        if(m_aValues_X.is())
            nIndex++;
        pRet->StandardSymbol = nIndex;
    }

    return pRet;
}

uno::Reference< beans::XPropertySet > VDataSeries::getXErrorBarProperties( sal_Int32 index ) const
{
    uno::Reference< beans::XPropertySet > xErrorBarProp;

    uno::Reference< beans::XPropertySet > xPointProp( getPropertiesOfPoint( index ));
    if( xPointProp.is() )
        xPointProp->getPropertyValue(CHART_UNONAME_ERRORBAR_X) >>= xErrorBarProp;
    return xErrorBarProp;
}

uno::Reference< beans::XPropertySet > VDataSeries::getYErrorBarProperties( sal_Int32 index ) const
{
    uno::Reference< beans::XPropertySet > xErrorBarProp;

    uno::Reference< beans::XPropertySet > xPointProp( getPropertiesOfPoint( index ));
    if( xPointProp.is() )
        xPointProp->getPropertyValue(CHART_UNONAME_ERRORBAR_Y) >>= xErrorBarProp;
    return xErrorBarProp;
}

bool VDataSeries::hasPointOwnColor( sal_Int32 index ) const
{
    if( !isAttributedDataPoint(index) )
        return false;

    try
    {
        uno::Reference< beans::XPropertyState > xPointState( getPropertiesOfPoint(index), uno::UNO_QUERY_THROW );
        return (xPointState->getPropertyState("Color") != beans::PropertyState_DEFAULT_VALUE );
    }
    catch(const uno::Exception&)
    {
        TOOLS_WARN_EXCEPTION("chart2", "" );
    }
    return false;
}

bool VDataSeries::isAttributedDataPoint( sal_Int32 index ) const
{
    //returns true if the data point assigned by the given index has set its own properties
    if( index>=m_nPointCount || m_nPointCount==0)
        return false;
    for(sal_Int32 n : m_aAttributedDataPointIndexList)
    {
        if(index == n)
            return true;
    }
    return false;
}

bool VDataSeries::isVaryColorsByPoint() const
{
    bool bVaryColorsByPoint = false;
    if( m_xDataSeries )
        m_xDataSeries->getFastPropertyValue(PROP_DATASERIES_VARY_COLORS_BY_POINT) >>= bVaryColorsByPoint; // "VaryColorsByPoint"
    return bVaryColorsByPoint;
}

uno::Reference< beans::XPropertySet > VDataSeries::getPropertiesOfPoint( sal_Int32 index ) const
{
    if( isAttributedDataPoint( index ) )
        return m_xDataSeries->getDataPointByIndex(index);
    return getPropertiesOfSeries();
}

const uno::Reference<beans::XPropertySet> & VDataSeries::getPropertiesOfSeries() const
{
    return m_xDataSeriesProps;
}

static std::optional<DataPointLabel> getDataPointLabelFromPropertySet( const uno::Reference< beans::XPropertySet >& xProp )
{
    std::optional< DataPointLabel > apLabel( std::in_place );
    try
    {
        if( !(xProp->getPropertyValue(CHART_UNONAME_LABEL) >>= *apLabel) )
            apLabel.reset();
    }
    catch(const uno::Exception &)
    {
        TOOLS_WARN_EXCEPTION("chart2", "" );
    }
    return apLabel;
}

void VDataSeries::adaptPointCache( sal_Int32 nNewPointIndex ) const
{
    if( m_nCurrentAttributedPoint != nNewPointIndex )
    {
        m_oLabel_AttributedPoint.reset();
        m_oLabelPropNames_AttributedPoint.reset();
        m_oLabelPropValues_AttributedPoint.reset();
        m_oSymbolProperties_AttributedPoint.reset();
        m_nCurrentAttributedPoint = nNewPointIndex;
    }
}

DataPointLabel* VDataSeries::getDataPointLabel( sal_Int32 index ) const
{
    DataPointLabel* pRet = nullptr;
    if( isAttributedDataPoint( index ) )
    {
        adaptPointCache( index );
        if (!m_oLabel_AttributedPoint)
            m_oLabel_AttributedPoint
                = getDataPointLabelFromPropertySet(getPropertiesOfPoint(index));
        if (m_oLabel_AttributedPoint)
            pRet = &*m_oLabel_AttributedPoint;
    }
    else
    {
        if (!m_oLabel_Series)
            m_oLabel_Series
                = getDataPointLabelFromPropertySet(getPropertiesOfPoint(index));
        if (m_oLabel_Series)
            pRet = &*m_oLabel_Series;
    }
    if( !m_bAllowPercentValueInDataLabel )
    {
        if( pRet )
            pRet->ShowNumberInPercent = false;
    }
    return pRet;
}

DataPointLabel* VDataSeries::getDataPointLabelIfLabel( sal_Int32 index ) const
{
    DataPointLabel* pLabel = getDataPointLabel( index );
    if( !pLabel || (!pLabel->ShowNumber && !pLabel->ShowNumberInPercent
        && !pLabel->ShowCategoryName && !pLabel->ShowCustomLabel && !pLabel->ShowSeriesName ) )
        return nullptr;
    return pLabel;
}

bool VDataSeries::getTextLabelMultiPropertyLists( sal_Int32 index
    , tNameSequence*& pPropNames
    , tAnySequence*& pPropValues ) const
{
    pPropNames = nullptr; pPropValues = nullptr;
    uno::Reference< beans::XPropertySet > xTextProp;
    bool bDoDynamicFontResize = false;
    if( isAttributedDataPoint( index ) )
    {
        adaptPointCache( index );
        if (!m_oLabelPropValues_AttributedPoint)
        {
            // Cache these properties for this point.
            m_oLabelPropNames_AttributedPoint.emplace();
            m_oLabelPropValues_AttributedPoint.emplace();
            xTextProp.set( getPropertiesOfPoint( index ));
            PropertyMapper::getTextLabelMultiPropertyLists(
                xTextProp, *m_oLabelPropNames_AttributedPoint, *m_oLabelPropValues_AttributedPoint);
            bDoDynamicFontResize = true;
        }
        pPropNames = &*m_oLabelPropNames_AttributedPoint;
        pPropValues = &*m_oLabelPropValues_AttributedPoint;
    }
    else
    {
        if (!m_oLabelPropValues_Series)
        {
            // Cache these properties for the whole series.
            m_oLabelPropNames_Series.emplace();
            m_oLabelPropValues_Series.emplace();
            xTextProp.set( getPropertiesOfPoint( index ));
            PropertyMapper::getTextLabelMultiPropertyLists(
                xTextProp, *m_oLabelPropNames_Series, *m_oLabelPropValues_Series);
            bDoDynamicFontResize = true;
        }
        pPropNames = &*m_oLabelPropNames_Series;
        pPropValues = &*m_oLabelPropValues_Series;
    }

    if( bDoDynamicFontResize &&
        pPropNames && pPropValues &&
        xTextProp.is())
    {
        LabelPositionHelper::doDynamicFontResize( *pPropValues, *pPropNames, xTextProp, m_aReferenceSize );
    }

    return (pPropNames && pPropValues);
}

void VDataSeries::setMissingValueTreatment( sal_Int32 nMissingValueTreatment )
{
    m_nMissingValueTreatment = nMissingValueTreatment;
}

sal_Int32 VDataSeries::getMissingValueTreatment() const
{
    return m_nMissingValueTreatment;
}

VDataSeries::VDataSeries()
    : m_nPolygonIndex(0)
    , m_fLogicMinX(0)
    , m_fLogicMaxX(0)
    , m_fLogicZPos(0)
    , m_nPointCount(0)
    , m_pValueSequenceForDataLabelNumberFormatDetection(nullptr)
    , m_fXMeanValue(0)
    , m_fYMeanValue(0)
    , m_eStackingDirection(chart2::StackingDirection_NO_STACKING)
    , m_nAxisIndex(0)
    , m_bConnectBars(false)
    , m_bGroupBarsPerAxis(false)
    , m_nStartingAngle(0)
    , m_ePieChartSubType(PieChartSubType_NONE)
    , m_nGlobalSeriesIndex(0)
    , m_nCurrentAttributedPoint(0)
    , m_nMissingValueTreatment(0)
    , m_bAllowPercentValueInDataLabel(false)
    , mpOldSeries(nullptr)
    , mnPercent(0)
{
}

void VDataSeries::setOldTimeBased( VDataSeries* pOldSeries, double nPercent )
{
    mnPercent = nPercent;
    mpOldSeries = pOldSeries;
    mpOldSeries->mpOldSeries = nullptr;
}

VDataSeries* VDataSeries::createCopyForTimeBased() const
{
    VDataSeries* pNew = new VDataSeries();
    pNew->m_aValues_X = m_aValues_X;
    pNew->m_aValues_Y = m_aValues_Y;
    pNew->m_aValues_Z = m_aValues_Z;
    pNew->m_aValues_Y_Min = m_aValues_Y_Min;
    pNew->m_aValues_Y_Max = m_aValues_Y_Max;
    pNew->m_aValues_Y_First = m_aValues_Y_First;
    pNew->m_aValues_Y_Last = m_aValues_Y_Last;
    pNew->m_aValues_Bubble_Size = m_aValues_Bubble_Size;
    pNew->m_PropertyMap = m_PropertyMap;

    pNew->m_nPointCount = m_nPointCount;

    return pNew;
}

double VDataSeries::getValueByProperty( sal_Int32 nIndex, const OUString& rPropName ) const
{
    auto const itr = m_PropertyMap.find(rPropName);
    if (itr == m_PropertyMap.end())
        return std::numeric_limits<double>::quiet_NaN();

    const VDataSequence* pData = &itr->second;
    double fValue = pData->getValue(nIndex);
    if(mpOldSeries && mpOldSeries->hasPropertyMapping(rPropName))
    {
        double fOldValue = mpOldSeries->getValueByProperty( nIndex, rPropName );
        if(rPropName.endsWith("Color"))
        {
            //optimized interpolation for color values
            Color aColor(ColorTransparency, static_cast<sal_uInt32>(fValue));
            Color aOldColor(ColorTransparency, static_cast<sal_uInt32>(fOldValue));
            sal_uInt8 r = aOldColor.GetRed() + (aColor.GetRed() - aOldColor.GetRed()) * mnPercent;
            sal_uInt8 g = aOldColor.GetGreen() + (aColor.GetGreen() - aOldColor.GetGreen()) * mnPercent;
            sal_uInt8 b = aOldColor.GetBlue() + (aColor.GetBlue() - aOldColor.GetBlue()) * mnPercent;
            sal_uInt8 a = aOldColor.GetAlpha() + (aColor.GetAlpha() - aOldColor.GetAlpha()) * mnPercent;
            Color aRet(ColorAlpha, a, r, g, b);
            return sal_uInt32(aRet);
        }
        return fOldValue + (fValue - fOldValue) * mnPercent;
    }
    return fValue;
}

bool VDataSeries::hasPropertyMapping(const OUString& rPropName ) const
{
    return m_PropertyMap.contains(rPropName);
}

} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
