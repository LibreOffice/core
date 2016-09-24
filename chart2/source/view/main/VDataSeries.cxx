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

#include "VDataSeries.hxx"
#include "ObjectIdentifier.hxx"
#include "macros.hxx"
#include "CommonConverters.hxx"
#include "LabelPositionHelper.hxx"
#include "ChartTypeHelper.hxx"
#include "DataSeriesHelper.hxx"
#include "RegressionCurveHelper.hxx"
#include <unonames.hxx>

#include <com/sun/star/chart/MissingValueTreatment.hpp>
#include <com/sun/star/chart2/Symbol.hpp>

#include <rtl/math.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XPropertyState.hpp>
#include <com/sun/star/drawing/LineStyle.hpp>
#include <com/sun/star/drawing/TextVerticalAdjust.hpp>
#include <com/sun/star/drawing/TextHorizontalAdjust.hpp>
#include <com/sun/star/text/WritingMode.hpp>
#include <com/sun/star/chart2/data/XDataSource.hpp>

namespace chart {

using namespace ::com::sun::star;
using namespace ::com::sun::star::chart2;
using ::com::sun::star::uno::Reference;

void VDataSequence::init( const uno::Reference< data::XDataSequence >& xModel )
{
    Model = xModel;
    Doubles = DataSequenceToDoubleSequence( xModel );
}

bool VDataSequence::is() const
{
    return Model.is();
}
void VDataSequence::clear()
{
    Model = nullptr;
    Doubles.realloc(0);
}

double VDataSequence::getValue( sal_Int32 index ) const
{
    if( 0<=index && index<Doubles.getLength() )
        return Doubles[index];
    else
    {
        double fNan;
        ::rtl::math::setNan( & fNan );
        return fNan;
    }
}

sal_Int32 VDataSequence::detectNumberFormatKey( sal_Int32 index ) const
{
    sal_Int32 nNumberFormatKey = -1;

    // -1 is allowed and means a key for the whole sequence
    if( -1<=index && index<Doubles.getLength() &&
        Model.is())
    {
        nNumberFormatKey = Model->getNumberFormatKeyByIndex( index );
    }

    return nNumberFormatKey;
}

sal_Int32 VDataSequence::getLength() const
{
    return Doubles.getLength();
}

namespace
{
struct lcl_LessXOfPoint
{
    inline bool operator() ( const std::vector< double >& first,
                             const std::vector< double >& second )
    {
        if( !first.empty() && !second.empty() )
        {
            return first[0]<second[0];
        }
        return false;
    }
};

void lcl_clearIfNoValuesButTextIsContained( VDataSequence& rData, const uno::Reference<data::XDataSequence>& xDataSequence )
{
    //#i71686#, #i101968#, #i102428#
    sal_Int32 nCount = rData.Doubles.getLength();
    for( sal_Int32 i = 0; i < nCount; ++i )
    {
        if( !::rtl::math::isNan( rData.Doubles[i] ) )
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
        && (::rtl::math::isNan(rfValue) || ::rtl::math::isInf(rfValue)) )
            rfValue = 0.0;
}

}

VDataSeries::VDataSeries( const uno::Reference< XDataSeries >& xDataSeries )
    : m_nPolygonIndex(0)
    , m_fLogicMinX(0.0)
    , m_fLogicMaxX(0.0)
    , m_fLogicZPos(0.0)
    , m_xGroupShape(nullptr)
    , m_xLabelsGroupShape(nullptr)
    , m_xErrorXBarsGroupShape(nullptr)
    , m_xErrorYBarsGroupShape(nullptr)
    , m_xFrontSubGroupShape(nullptr)
    , m_xBackSubGroupShape(nullptr)
    , m_xDataSeries(xDataSeries)
    , m_nPointCount(0)

    , m_pValueSequenceForDataLabelNumberFormatDetection(&m_aValues_Y)

    , m_fXMeanValue(1.0)
    , m_fYMeanValue(1.0)

    , m_aAttributedDataPointIndexList()

    , m_eStackingDirection(StackingDirection_NO_STACKING)
    , m_nAxisIndex(0)
    , m_bConnectBars(false)
    , m_bGroupBarsPerAxis(true)
    , m_nStartingAngle(90)

    , m_nGlobalSeriesIndex(0)

    , m_apLabel_Series(nullptr)
    , m_apLabelPropNames_Series(nullptr)
    , m_apLabelPropValues_Series(nullptr)
    , m_apSymbolProperties_Series(nullptr)

    , m_apLabel_AttributedPoint(nullptr)
    , m_apLabelPropNames_AttributedPoint(nullptr)
    , m_apLabelPropValues_AttributedPoint(nullptr)
    , m_apSymbolProperties_AttributedPoint(nullptr)
    , m_apSymbolProperties_InvisibleSymbolForSelection(nullptr)
    , m_nCurrentAttributedPoint(-1)
    , m_nMissingValueTreatment(css::chart::MissingValueTreatment::LEAVE_GAP)
    , m_bAllowPercentValueInDataLabel(false)
    , mpOldSeries(nullptr)
    , mnPercent(0.0)
{
    ::rtl::math::setNan( & m_fXMeanValue );
    ::rtl::math::setNan( & m_fYMeanValue );

    uno::Reference<data::XDataSource> xDataSource =
            uno::Reference<data::XDataSource>( xDataSeries, uno::UNO_QUERY );

    uno::Sequence< uno::Reference<
        chart2::data::XLabeledDataSequence > > aDataSequences =
            xDataSource->getDataSequences();

    for(sal_Int32 nN = aDataSequences.getLength();nN--;)
    {
        if(!aDataSequences[nN].is())
            continue;
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
            catch( const uno::Exception& e )
            {
                ASSERT_EXCEPTION( e );
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

    uno::Reference<beans::XPropertySet> xProp(xDataSeries, uno::UNO_QUERY );
    if( xProp.is())
    {
        try
        {
            //get AttributedDataPoints
            xProp->getPropertyValue("AttributedDataPoints") >>= m_aAttributedDataPointIndexList;

            xProp->getPropertyValue("StackingDirection") >>= m_eStackingDirection;

            xProp->getPropertyValue("AttachedAxisIndex") >>= m_nAxisIndex;
            if(m_nAxisIndex<0)
                m_nAxisIndex=0;
        }
        catch( const uno::Exception& e )
        {
            ASSERT_EXCEPTION( e );
        }
    }
}

VDataSeries::~VDataSeries()
{
}

void VDataSeries::doSortByXValues()
{
    if( m_aValues_X.is() && m_aValues_X.Doubles.getLength() )
    {
        //prepare a vector for sorting
        std::vector< ::std::vector< double > > aTmp;//outer vector are points, inner vector are the different values of the point
        double fNan;
        ::rtl::math::setNan( & fNan );
        sal_Int32 nPointIndex = 0;
        for( nPointIndex=0; nPointIndex < m_nPointCount; nPointIndex++ )
        {
            std::vector< double > aSinglePoint;
            aSinglePoint.push_back( (nPointIndex < m_aValues_X.Doubles.getLength()) ? m_aValues_X.Doubles[nPointIndex] : fNan );
            aSinglePoint.push_back( (nPointIndex < m_aValues_Y.Doubles.getLength()) ? m_aValues_Y.Doubles[nPointIndex] : fNan );
            aTmp.push_back( aSinglePoint );
        }

        //do sort
        std::stable_sort( aTmp.begin(), aTmp.end(), lcl_LessXOfPoint() );

        //fill the sorted points back to the members
        m_aValues_X.Doubles.realloc( m_nPointCount );
        m_aValues_Y.Doubles.realloc( m_nPointCount );

        for( nPointIndex=0; nPointIndex < m_nPointCount; nPointIndex++ )
        {
            m_aValues_X.Doubles[nPointIndex]=aTmp[nPointIndex][0];
            m_aValues_Y.Doubles[nPointIndex]=aTmp[nPointIndex][1];
        }
    }
}

void VDataSeries::releaseShapes()
{
    m_xGroupShape.set(nullptr);
    m_xLabelsGroupShape.set(nullptr);
    m_xErrorXBarsGroupShape.set(nullptr);
    m_xErrorYBarsGroupShape.set(nullptr);
    m_xFrontSubGroupShape.set(nullptr);
    m_xBackSubGroupShape.set(nullptr);

    m_aPolyPolygonShape3D.SequenceX.realloc(0);
    m_aPolyPolygonShape3D.SequenceY.realloc(0);
    m_aPolyPolygonShape3D.SequenceZ.realloc(0);
    m_nPolygonIndex = 0;
}

const uno::Reference<css::chart2::XDataSeries>& VDataSeries::getModel() const
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
                        OBJECTTYPE_DATA_LABEL, OUString(), getLabelsCID() );
}
OUString VDataSeries::getErrorBarsCID(bool bYError) const
{
    OUString aChildParticle( ObjectIdentifier::getStringForType(
                                      bYError ? OBJECTTYPE_DATA_ERRORS_Y : OBJECTTYPE_DATA_ERRORS_X ) );
    aChildParticle += "=";

    return ObjectIdentifier::createClassifiedIdentifierForParticles(
            m_aSeriesParticle, aChildParticle );
}
OUString VDataSeries::getLabelsCID() const
{
    OUString aChildParticle( ObjectIdentifier::getStringForType( OBJECTTYPE_DATA_LABELS ) );
    aChildParticle += "=";

    return ObjectIdentifier::createClassifiedIdentifierForParticles(
            m_aSeriesParticle, aChildParticle );
}
OUString VDataSeries::getDataCurveCID( sal_Int32 nCurveIndex, bool bAverageLine ) const
{
    OUString aRet;
    aRet = ObjectIdentifier::createDataCurveCID( m_aSeriesParticle, nCurveIndex, bAverageLine );
    return aRet;
}

OUString VDataSeries::getDataCurveEquationCID( sal_Int32 nCurveIndex ) const
{
    OUString aRet;
    aRet = ObjectIdentifier::createDataCurveEquationCID( m_aSeriesParticle, nCurveIndex );
    return aRet;
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
    double fRet = 0.0;
    if(m_aValues_X.is())
    {
        if( 0<=index && index<m_aValues_X.getLength() )
        {
            fRet = m_aValues_X.Doubles[index];
            if(mpOldSeries && index < mpOldSeries->m_aValues_X.getLength())
            {
                double nOldVal = mpOldSeries->m_aValues_X.Doubles[index];
                fRet = nOldVal + (fRet - nOldVal) * mnPercent;
            }
        }
        else
            ::rtl::math::setNan( &fRet );
    }
    else
    {
        // #i70133# always return correct X position - needed for short data series
        if( 0<=index /*&& index < m_nPointCount*/ )
            fRet = index+1;//first category (index 0) matches with real number 1.0
        else
            ::rtl::math::setNan( &fRet );
    }
    lcl_maybeReplaceNanWithZero( fRet, getMissingValueTreatment() );
    return fRet;
}

double VDataSeries::getYValue( sal_Int32 index ) const
{
    double fRet = 0.0;
    if(m_aValues_Y.is())
    {
        if( 0<=index && index<m_aValues_Y.getLength() )
        {
            fRet = m_aValues_Y.Doubles[index];
            if(mpOldSeries && index < mpOldSeries->m_aValues_Y.getLength())
            {
                double nOldVal = mpOldSeries->m_aValues_Y.Doubles[index];
                fRet = nOldVal + (fRet - nOldVal) * mnPercent;
            }
        }
        else
            ::rtl::math::setNan( &fRet );
    }
    else
    {
        // #i70133# always return correct X position - needed for short data series
        if( 0<=index /*&& index < m_nPointCount*/ )
            fRet = index+1;//first category (index 0) matches with real number 1.0
        else
            ::rtl::math::setNan( &fRet );
    }
    lcl_maybeReplaceNanWithZero( fRet, getMissingValueTreatment() );
    return fRet;
}

void VDataSeries::getMinMaxXValue(double& fMin, double& fMax) const
{
    rtl::math::setNan( &fMax );
    rtl::math::setNan( &fMin );

    uno::Sequence< double > aValuesX = getAllX();

    if(aValuesX.getLength() > 0)
    {
        sal_Int32 i = 0;
        while ( i < aValuesX.getLength() && ::rtl::math::isNan(aValuesX[i]) )
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
    OUString aPropName = bForPercentage ? OUString("PercentageNumberFormat") : OUString(CHART_UNONAME_NUMFMT);
    bool bHasNumberFormat = false;
    uno::Reference< beans::XPropertySet > xPointProp( this->getPropertiesOfPoint( nPointIndex ));
    sal_Int32 nNumberFormat = -1;
    if( xPointProp.is() && (xPointProp->getPropertyValue(aPropName) >>= nNumberFormat) )
        bHasNumberFormat = true;
    return bHasNumberFormat;
}
sal_Int32 VDataSeries::getExplicitNumberFormat( sal_Int32 nPointIndex, bool bForPercentage ) const
{
    OUString aPropName = bForPercentage ? OUString("PercentageNumberFormat") : OUString(CHART_UNONAME_NUMFMT);
    sal_Int32 nNumberFormat = -1;
    uno::Reference< beans::XPropertySet > xPointProp( this->getPropertiesOfPoint( nPointIndex ));
    if( xPointProp.is() )
        xPointProp->getPropertyValue(aPropName) >>= nNumberFormat;
    return nNumberFormat;
}
void VDataSeries::setRoleOfSequenceForDataLabelNumberFormatDetection( const OUString& rRole )
{
    if (rRole == "values-y")
        m_pValueSequenceForDataLabelNumberFormatDetection = &m_aValues_Y;
    else if (rRole == "values-size")
        m_pValueSequenceForDataLabelNumberFormatDetection = &m_aValues_Bubble_Size;
    else if (rRole == "values-min")
        m_pValueSequenceForDataLabelNumberFormatDetection = &m_aValues_Y_Min;
    else if (rRole == "values-max")
        m_pValueSequenceForDataLabelNumberFormatDetection = &m_aValues_Y_Max;
    else if (rRole == "values-first")
        m_pValueSequenceForDataLabelNumberFormatDetection = &m_aValues_Y_First;
    else if (rRole == "values-last")
        m_pValueSequenceForDataLabelNumberFormatDetection = &m_aValues_Y_Last;
    else if (rRole == "values-x")
        m_pValueSequenceForDataLabelNumberFormatDetection = &m_aValues_X;
}
bool VDataSeries::shouldLabelNumberFormatKeyBeDetectedFromYAxis() const
{
    if( m_pValueSequenceForDataLabelNumberFormatDetection == &m_aValues_Bubble_Size )
        return false;
    else if( m_pValueSequenceForDataLabelNumberFormatDetection == &m_aValues_X )
        return false;
    return true;
}
sal_Int32 VDataSeries::detectNumberFormatKey( sal_Int32 index ) const
{
    sal_Int32 nRet = 0;
    if( m_pValueSequenceForDataLabelNumberFormatDetection )
        nRet = m_pValueSequenceForDataLabelNumberFormatDetection->detectNumberFormatKey( index );
    return nRet;
}

sal_Int32 VDataSeries::getLabelPlacement( sal_Int32 nPointIndex, const uno::Reference< chart2::XChartType >& xChartType, sal_Int32 nDimensionCount, bool bSwapXAndY ) const
{
    sal_Int32 nLabelPlacement=0;
    try
    {
        uno::Reference< beans::XPropertySet > xPointProps( this->getPropertiesOfPoint( nPointIndex ) );
        if( xPointProps.is() )
            xPointProps->getPropertyValue("LabelPlacement") >>= nLabelPlacement;

        //ensure that the set label placement is supported by this charttype

        uno::Sequence < sal_Int32 > aAvailablePlacements( ChartTypeHelper::getSupportedLabelPlacements(
                xChartType, nDimensionCount, bSwapXAndY, m_xDataSeries ) );

        for( sal_Int32 nN = 0; nN < aAvailablePlacements.getLength(); nN++ )
            if( aAvailablePlacements[nN] == nLabelPlacement )
                return nLabelPlacement; //ok

        //otherwise use the first supported one
        if( aAvailablePlacements.getLength() )
        {
            nLabelPlacement = aAvailablePlacements[0];
            return nLabelPlacement;
        }

        OSL_FAIL("no label placement supported");
    }
    catch( const uno::Exception& e )
    {
        ASSERT_EXCEPTION( e );
    }
    return nLabelPlacement;
}

double VDataSeries::getMinimumofAllDifferentYValues( sal_Int32 index ) const
{
    double fMin=0.0;
    ::rtl::math::setInf(&fMin, false);

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

    if( ::rtl::math::isInf(fMin) )
        ::rtl::math::setNan(&fMin);

    return fMin;
}

double VDataSeries::getMaximumofAllDifferentYValues( sal_Int32 index ) const
{
    double fMax=0.0;
    ::rtl::math::setInf(&fMax, true);

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

    if( ::rtl::math::isInf(fMax) )
        ::rtl::math::setNan(&fMax);

    return fMax;
}

uno::Sequence< double > const & VDataSeries::getAllX() const
{
    if(!m_aValues_X.is() && !m_aValues_X.getLength() && m_nPointCount)
    {
        //init x values from category indexes
        //first category (index 0) matches with real number 1.0
        m_aValues_X.Doubles.realloc( m_nPointCount );
        for(sal_Int32 nN=m_aValues_X.getLength();nN--;)
            m_aValues_X.Doubles[nN] = nN+1;
    }
    return m_aValues_X.Doubles;
}

uno::Sequence< double > const & VDataSeries::getAllY() const
{
    if(!m_aValues_Y.is() && !m_aValues_Y.getLength() && m_nPointCount)
    {
        //init y values from indexes
        //first y-value (index 0) matches with real number 1.0
        m_aValues_Y.Doubles.realloc( m_nPointCount );
        for(sal_Int32 nN=m_aValues_Y.getLength();nN--;)
            m_aValues_Y.Doubles[nN] = nN+1;
    }
    return m_aValues_Y.Doubles;
}

double VDataSeries::getXMeanValue() const
{
    if( ::rtl::math::isNan( m_fXMeanValue ) )
    {
        uno::Reference< XRegressionCurveCalculator > xCalculator( RegressionCurveHelper::createRegressionCurveCalculatorByServiceName( "com.sun.star.chart2.MeanValueRegressionCurve" ) );
        uno::Sequence< double > aXValuesDummy;
        xCalculator->recalculateRegression( aXValuesDummy, getAllX() );
        double fXDummy = 1.0;
        m_fXMeanValue = xCalculator->getCurveValue( fXDummy );
    }
    return m_fXMeanValue;
}

double VDataSeries::getYMeanValue() const
{
    if( ::rtl::math::isNan( m_fYMeanValue ) )
    {
        uno::Reference< XRegressionCurveCalculator > xCalculator(
            RegressionCurveHelper::createRegressionCurveCalculatorByServiceName("com.sun.star.chart2.MeanValueRegressionCurve"));
        uno::Sequence< double > aXValuesDummy;
        xCalculator->recalculateRegression( aXValuesDummy, getAllY() );
        double fXDummy = 1.0;
        m_fYMeanValue = xCalculator->getCurveValue( fXDummy );
    }
    return m_fYMeanValue;
}

std::unique_ptr<Symbol> getSymbolPropertiesFromPropertySet( const uno::Reference< beans::XPropertySet >& xProp )
{
    ::std::unique_ptr< Symbol > apSymbolProps( new Symbol() );
    try
    {
        if( xProp->getPropertyValue("Symbol") >>= *apSymbolProps )
        {
            //use main color to fill symbols
            xProp->getPropertyValue("Color") >>= apSymbolProps->FillColor;
            // border of symbols always same as fill color
            apSymbolProps->BorderColor = apSymbolProps->FillColor;
        }
        else
            apSymbolProps.reset();
    }
    catch(const uno::Exception &e)
    {
        ASSERT_EXCEPTION( e );
    }
    return apSymbolProps;
}

Symbol* VDataSeries::getSymbolProperties( sal_Int32 index ) const
{
    Symbol* pRet=nullptr;
    if( isAttributedDataPoint( index ) )
    {
        adaptPointCache( index );
        if (!m_apSymbolProperties_AttributedPoint)
            m_apSymbolProperties_AttributedPoint
                = getSymbolPropertiesFromPropertySet(this->getPropertiesOfPoint(index));
        pRet = m_apSymbolProperties_AttributedPoint.get();
        //if a single data point does not have symbols but the dataseries itself has symbols
        //we create an invisible symbol shape to enable selection of that point
        if( !pRet || pRet->Style == SymbolStyle_NONE )
        {
            if (!m_apSymbolProperties_Series)
                m_apSymbolProperties_Series
                    = getSymbolPropertiesFromPropertySet(this->getPropertiesOfSeries());
            if( m_apSymbolProperties_Series.get() && m_apSymbolProperties_Series->Style != SymbolStyle_NONE )
            {
                if (!m_apSymbolProperties_InvisibleSymbolForSelection)
                {
                    m_apSymbolProperties_InvisibleSymbolForSelection.reset(new Symbol);
                    m_apSymbolProperties_InvisibleSymbolForSelection->Style = SymbolStyle_STANDARD;
                    m_apSymbolProperties_InvisibleSymbolForSelection->StandardSymbol = 0;//square
                    m_apSymbolProperties_InvisibleSymbolForSelection->Size = m_apSymbolProperties_Series->Size;
                    m_apSymbolProperties_InvisibleSymbolForSelection->BorderColor = 0xff000000;//invisible
                    m_apSymbolProperties_InvisibleSymbolForSelection->FillColor = 0xff000000;//invisible
                }
                pRet = m_apSymbolProperties_InvisibleSymbolForSelection.get();
            }
        }
    }
    else
    {
        if (!m_apSymbolProperties_Series)
            m_apSymbolProperties_Series
                = getSymbolPropertiesFromPropertySet(this->getPropertiesOfSeries());
        pRet = m_apSymbolProperties_Series.get();
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

    uno::Reference< beans::XPropertySet > xPointProp( this->getPropertiesOfPoint( index ));
    if( xPointProp.is() )
        xPointProp->getPropertyValue(CHART_UNONAME_ERRORBAR_X) >>= xErrorBarProp;
    return xErrorBarProp;
}

uno::Reference< beans::XPropertySet > VDataSeries::getYErrorBarProperties( sal_Int32 index ) const
{
    uno::Reference< beans::XPropertySet > xErrorBarProp;

    uno::Reference< beans::XPropertySet > xPointProp( this->getPropertiesOfPoint( index ));
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
        uno::Reference< beans::XPropertyState > xPointState( this->getPropertiesOfPoint(index), uno::UNO_QUERY_THROW );
        return (xPointState->getPropertyState("Color") != beans::PropertyState_DEFAULT_VALUE );
    }
    catch(const uno::Exception& e)
    {
        ASSERT_EXCEPTION( e );
    }
    return false;
}

bool VDataSeries::isAttributedDataPoint( sal_Int32 index ) const
{
    //returns true if the data point assigned by the given index has set its own properties
    if( index>=m_nPointCount || m_nPointCount==0)
        return false;
    for(sal_Int32 nN=m_aAttributedDataPointIndexList.getLength();nN--;)
    {
        if(index==m_aAttributedDataPointIndexList[nN])
            return true;
    }
    return false;
}

bool VDataSeries::isVaryColorsByPoint() const
{
    bool bVaryColorsByPoint = false;
    Reference< beans::XPropertySet > xSeriesProp( this->getPropertiesOfSeries() );
    if( xSeriesProp.is() )
        xSeriesProp->getPropertyValue("VaryColorsByPoint") >>= bVaryColorsByPoint;
    return bVaryColorsByPoint;
}

uno::Reference< beans::XPropertySet > VDataSeries::getPropertiesOfPoint( sal_Int32 index ) const
{
    if( isAttributedDataPoint( index ) )
        return m_xDataSeries->getDataPointByIndex(index);
    return this->getPropertiesOfSeries();
}

uno::Reference<beans::XPropertySet> VDataSeries::getPropertiesOfSeries() const
{
    return uno::Reference<css::beans::XPropertySet>(m_xDataSeries, css::uno::UNO_QUERY);
}

std::unique_ptr<DataPointLabel> getDataPointLabelFromPropertySet( const uno::Reference< beans::XPropertySet >& xProp )
{
    ::std::unique_ptr< DataPointLabel > apLabel( new DataPointLabel() );
    try
    {
        if( !(xProp->getPropertyValue(CHART_UNONAME_LABEL) >>= *apLabel) )
            apLabel.reset();
    }
    catch(const uno::Exception &e)
    {
        ASSERT_EXCEPTION( e );
    }
    return apLabel;
}

void VDataSeries::adaptPointCache( sal_Int32 nNewPointIndex ) const
{
    if( m_nCurrentAttributedPoint != nNewPointIndex )
    {
        m_apLabel_AttributedPoint.reset();
        m_apLabelPropNames_AttributedPoint.reset();
        m_apLabelPropValues_AttributedPoint.reset();
        m_apSymbolProperties_AttributedPoint.reset();
        m_nCurrentAttributedPoint = nNewPointIndex;
    }
}

DataPointLabel* VDataSeries::getDataPointLabel( sal_Int32 index ) const
{
    DataPointLabel* pRet = nullptr;
    if( isAttributedDataPoint( index ) )
    {
        adaptPointCache( index );
        if( !m_apLabel_AttributedPoint.get() )
            m_apLabel_AttributedPoint
                = getDataPointLabelFromPropertySet(this->getPropertiesOfPoint(index));
        pRet = m_apLabel_AttributedPoint.get();
    }
    else
    {
        if (!m_apLabel_Series)
            m_apLabel_Series
                = getDataPointLabelFromPropertySet(this->getPropertiesOfPoint(index));
        pRet = m_apLabel_Series.get();
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
    DataPointLabel* pLabel = this->getDataPointLabel( index );
    if( !pLabel || (!pLabel->ShowNumber && !pLabel->ShowNumberInPercent
        && !pLabel->ShowCategoryName ) )
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
        if (!m_apLabelPropValues_AttributedPoint)
        {
            // Cache these properties for this point.
            m_apLabelPropNames_AttributedPoint.reset(new tNameSequence);
            m_apLabelPropValues_AttributedPoint.reset(new tAnySequence);
            xTextProp.set( this->getPropertiesOfPoint( index ));
            PropertyMapper::getTextLabelMultiPropertyLists(
                xTextProp, *m_apLabelPropNames_AttributedPoint, *m_apLabelPropValues_AttributedPoint);
            bDoDynamicFontResize = true;
        }
        pPropNames = m_apLabelPropNames_AttributedPoint.get();
        pPropValues = m_apLabelPropValues_AttributedPoint.get();
    }
    else
    {
        if (!m_apLabelPropValues_Series)
        {
            // Cache these properties for the whole series.
            m_apLabelPropNames_Series.reset(new tNameSequence);
            m_apLabelPropValues_Series.reset(new tAnySequence);
            xTextProp.set( this->getPropertiesOfPoint( index ));
            PropertyMapper::getTextLabelMultiPropertyLists(
                xTextProp, *m_apLabelPropNames_Series, *m_apLabelPropValues_Series);
            bDoDynamicFontResize = true;
        }
        pPropNames = m_apLabelPropNames_Series.get();
        pPropValues = m_apLabelPropValues_Series.get();
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
    {
        double fNan;
        ::rtl::math::setNan( &fNan );
        return fNan;
    }

    const VDataSequence* pData = &itr->second;
    double fValue = pData->getValue(nIndex);
    if(mpOldSeries && mpOldSeries->hasPropertyMapping(rPropName))
    {
        double fOldValue = mpOldSeries->getValueByProperty( nIndex, rPropName );
        if(rPropName.endsWith("Color"))
        {
            //optimized interpolation for color values
            Color aColor(static_cast<sal_uInt32>(fValue));
            Color aOldColor(static_cast<sal_uInt32>(fOldValue));
            sal_uInt8 r = aOldColor.GetRed() + (aColor.GetRed() - aOldColor.GetRed()) * mnPercent;
            sal_uInt8 g = aOldColor.GetGreen() + (aColor.GetGreen() - aOldColor.GetGreen()) * mnPercent;
            sal_uInt8 b = aOldColor.GetBlue() + (aColor.GetBlue() - aOldColor.GetBlue()) * mnPercent;
            sal_uInt8 t = aOldColor.GetTransparency() + (aColor.GetTransparency() - aOldColor.GetTransparency()) * mnPercent;
            Color aRet(t, r, g, b);
            return aRet.GetColor();
        }
        return fOldValue + (fValue - fOldValue) * mnPercent;
    }
    return fValue;
}

bool VDataSeries::hasPropertyMapping(const OUString& rPropName ) const
{
    return m_PropertyMap.find(rPropName) != m_PropertyMap.end();
}

} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
