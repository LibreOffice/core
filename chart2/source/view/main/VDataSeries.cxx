/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_chart2.hxx"
#include "VDataSeries.hxx"
#include "ObjectIdentifier.hxx"
#include "macros.hxx"
#include "CommonConverters.hxx"
#include "LabelPositionHelper.hxx"
#include "ChartTypeHelper.hxx"
#include "ContainerHelper.hxx"
#include "DataSeriesHelper.hxx"
#include "RegressionCurveHelper.hxx"

#include <com/sun/star/chart/MissingValueTreatment.hpp>
#include <com/sun/star/chart2/Symbol.hpp>

//#include "CommonConverters.hxx"
#include <rtl/math.hxx>
#include <tools/debug.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XPropertyState.hpp>
#include <com/sun/star/drawing/LineStyle.hpp>
#include <com/sun/star/drawing/TextVerticalAdjust.hpp>
#include <com/sun/star/drawing/TextHorizontalAdjust.hpp>
#include <com/sun/star/text/WritingMode.hpp>
#include <com/sun/star/chart2/data/XDataSource.hpp>

//.............................................................................
namespace chart
{
//.............................................................................
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
    Model = NULL;
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
        if( first.size() > 0 && second.size() > 0 )
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
    //no double value is countained
    //is there any text?
    uno::Sequence< rtl::OUString > aStrings( DataSequenceToStringSequence( xDataSequence ) );
    sal_Int32 nTextCount = aStrings.getLength();
    for( sal_Int32 j = 0; j < nTextCount; ++j )
    {
        if( aStrings[j].getLength() )
        {
            rData.clear();
            return;
        }
    }
    //no content at all
}

void lcl_maybeReplaceNanWithZero( double& rfValue, sal_Int32 nMissingValueTreatment )
{
    if( nMissingValueTreatment == ::com::sun::star::chart::MissingValueTreatment::USE_ZERO
        && (::rtl::math::isNan(rfValue) || ::rtl::math::isInf(rfValue)) )
            rfValue = 0.0;
}

}

VDataSeries::VDataSeries( const uno::Reference< XDataSeries >& xDataSeries )
    : m_nPolygonIndex(0)
    , m_fLogicMinX(0.0)
    , m_fLogicMaxX(0.0)
    , m_fLogicZPos(0.0)
    , m_xGroupShape(NULL)
    , m_xLabelsGroupShape(NULL)
    , m_xErrorBarsGroupShape(NULL)
    , m_xFrontSubGroupShape(NULL)
    , m_xBackSubGroupShape(NULL)
    , m_xDataSeries(xDataSeries)
    , m_aDataSequences()
    , m_nPointCount(0)

    , m_aValues_X()
    , m_aValues_Y()
    , m_aValues_Z()
    , m_aValues_Y_Min()
    , m_aValues_Y_Max()
    , m_aValues_Y_First()
    , m_aValues_Y_Last()
    , m_aValues_Bubble_Size()
    , m_pValueSequenceForDataLabelNumberFormatDetection(&m_aValues_Y)

    , m_fYMeanValue(1.0)

    , m_aAttributedDataPointIndexList()

    , m_eStackingDirection(StackingDirection_NO_STACKING)
    , m_nAxisIndex(0)
    , m_bConnectBars(sal_False)
    , m_bGroupBarsPerAxis(sal_True)
    , m_nStartingAngle(90)

    , m_aSeriesParticle()
    , m_aCID()
    , m_aPointCID_Stub()
    , m_aLabelCID_Stub()

    , m_nGlobalSeriesIndex(0)

    , m_apLabel_Series(NULL)
    , m_apLabelPropNames_Series(NULL)
    , m_apLabelPropValues_Series(NULL)
    , m_apSymbolProperties_Series(NULL)

    , m_apLabel_AttributedPoint(NULL)
    , m_apLabelPropNames_AttributedPoint(NULL)
    , m_apLabelPropValues_AttributedPoint(NULL)
    , m_apSymbolProperties_AttributedPoint(NULL)
    , m_apSymbolProperties_InvisibleSymbolForSelection(NULL)
    , m_nCurrentAttributedPoint(-1)
    , m_nMissingValueTreatment(::com::sun::star::chart::MissingValueTreatment::LEAVE_GAP)
    , m_bAllowPercentValueInDataLabel(false)
{
    ::rtl::math::setNan( & m_fYMeanValue );

    uno::Reference<data::XDataSource> xDataSource =
            uno::Reference<data::XDataSource>( xDataSeries, uno::UNO_QUERY );

    m_aDataSequences = xDataSource->getDataSequences();

    for(sal_Int32 nN = m_aDataSequences.getLength();nN--;)
    {
        if(!m_aDataSequences[nN].is())
            continue;
        uno::Reference<data::XDataSequence>  xDataSequence( m_aDataSequences[nN]->getValues());
        uno::Reference<beans::XPropertySet> xProp(xDataSequence, uno::UNO_QUERY );
        if( xProp.is())
        {
            try
            {
                uno::Any aARole = xProp->getPropertyValue( C2U( "Role" ) );
                rtl::OUString aRole;
                aARole >>= aRole;

                if( aRole.equals(C2U("values-x")) )
                {
                    m_aValues_X.init( xDataSequence );
                    lcl_clearIfNoValuesButTextIsContained( m_aValues_X, xDataSequence );
                }
                else if( aRole.equals(C2U("values-y")) )
                    m_aValues_Y.init( xDataSequence );
                else if( aRole.equals(C2U("values-min")) )
                    m_aValues_Y_Min.init( xDataSequence );
                else if( aRole.equals(C2U("values-max")) )
                    m_aValues_Y_Max.init( xDataSequence );
                else if( aRole.equals(C2U("values-first")) )
                    m_aValues_Y_First.init( xDataSequence );
                else if( aRole.equals(C2U("values-last")) )
                    m_aValues_Y_Last.init( xDataSequence );
                else if( aRole.equals(C2U("values-size")) )
                    m_aValues_Bubble_Size.init( xDataSequence );
            }
            catch( uno::Exception& e )
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
            xProp->getPropertyValue( C2U( "AttributedDataPoints" ) ) >>= m_aAttributedDataPointIndexList;

            xProp->getPropertyValue( C2U( "StackingDirection" ) ) >>= m_eStackingDirection;

            xProp->getPropertyValue( C2U( "AttachedAxisIndex" ) ) >>= m_nAxisIndex;
            if(m_nAxisIndex<0)
                m_nAxisIndex=0;
        }
        catch( uno::Exception& e )
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
        //prepare a vector vor sorting
        std::vector< ::std::vector< double > > aTmp;//outer vector are points, inner vector are the different values of athe point
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
        std::sort( aTmp.begin(), aTmp.end(), lcl_LessXOfPoint() );

        //fill the sorted points back to the mambers
        m_aValues_X.Doubles.realloc( m_nPointCount );
        m_aValues_Y.Doubles.realloc( m_nPointCount );

        for( nPointIndex=0; nPointIndex < m_nPointCount; nPointIndex++ )
        {
            m_aValues_X.Doubles[nPointIndex]=aTmp[nPointIndex][0];
            m_aValues_Y.Doubles[nPointIndex]=aTmp[nPointIndex][1];
        }
    }
}

uno::Reference< XDataSeries > VDataSeries::getModel() const
{
    return m_xDataSeries;
}

void VDataSeries::releaseShapes()
{
    m_xGroupShape.set(0);
    m_xLabelsGroupShape.set(0);
    m_xErrorBarsGroupShape.set(0);
    m_xFrontSubGroupShape.set(0);
    m_xBackSubGroupShape.set(0);

    m_aPolyPolygonShape3D.SequenceX.realloc(0);
    m_aPolyPolygonShape3D.SequenceY.realloc(0);
    m_aPolyPolygonShape3D.SequenceZ.realloc(0);
    m_nPolygonIndex = 0;
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

void VDataSeries::setParticle( const rtl::OUString& rSeriesParticle )
{
    m_aSeriesParticle = rSeriesParticle;

    //get CID
    m_aCID = ObjectIdentifier::createClassifiedIdentifierForParticle( m_aSeriesParticle );
    m_aPointCID_Stub = ObjectIdentifier::createSeriesSubObjectStub( OBJECTTYPE_DATA_POINT, m_aSeriesParticle );

    m_aLabelCID_Stub = ObjectIdentifier::createClassifiedIdentifierWithParent(
                        OBJECTTYPE_DATA_LABEL, ::rtl::OUString(), getLabelsCID() );
}
rtl::OUString VDataSeries::getSeriesParticle() const
{
    return m_aSeriesParticle;
}
rtl::OUString VDataSeries::getCID() const
{
    return m_aCID;
}
rtl::OUString VDataSeries::getPointCID_Stub() const
{
    return m_aPointCID_Stub;
}
rtl::OUString VDataSeries::getErrorBarsCID() const
{
    rtl::OUString aChildParticle( ObjectIdentifier::getStringForType( OBJECTTYPE_DATA_ERRORS ) );
    aChildParticle+=(C2U("="));

    return ObjectIdentifier::createClassifiedIdentifierForParticles(
            m_aSeriesParticle, aChildParticle );
}
rtl::OUString VDataSeries::getLabelsCID() const
{
    rtl::OUString aChildParticle( ObjectIdentifier::getStringForType( OBJECTTYPE_DATA_LABELS ) );
    aChildParticle+=(C2U("="));

    return ObjectIdentifier::createClassifiedIdentifierForParticles(
            m_aSeriesParticle, aChildParticle );
}
rtl::OUString VDataSeries::getLabelCID_Stub() const
{
    return m_aLabelCID_Stub;
}
rtl::OUString VDataSeries::getDataCurveCID( sal_Int32 nCurveIndex, bool bAverageLine ) const
{
    rtl::OUString aRet;
    aRet = ObjectIdentifier::createDataCurveCID( m_aSeriesParticle, nCurveIndex, bAverageLine );
    return aRet;
}

rtl::OUString VDataSeries::getDataCurveEquationCID( sal_Int32 nCurveIndex ) const
{
    rtl::OUString aRet;
    aRet = ObjectIdentifier::createDataCurveEquationCID( m_aSeriesParticle, nCurveIndex );
    return aRet;
}
void VDataSeries::setPageReferenceSize( const awt::Size & rPageRefSize )
{
    m_aReferenceSize = rPageRefSize;
}

StackingDirection VDataSeries::getStackingDirection() const
{
    return m_eStackingDirection;
}
sal_Int32 VDataSeries::getAttachedAxisIndex() const
{
    return m_nAxisIndex;
}
void VDataSeries::setConnectBars( sal_Bool bConnectBars )
{
    m_bConnectBars = bConnectBars;
}
sal_Bool VDataSeries::getConnectBars() const
{
    return m_bConnectBars;
}
void VDataSeries::setGroupBarsPerAxis( sal_Bool bGroupBarsPerAxis )
{
    m_bGroupBarsPerAxis = bGroupBarsPerAxis;
}
sal_Bool VDataSeries::getGroupBarsPerAxis() const
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

void VDataSeries::setAttachedAxisIndex( sal_Int32 nAttachedAxisIndex )
{
    if( nAttachedAxisIndex < 0 )
        nAttachedAxisIndex = 0;
    m_nAxisIndex = nAttachedAxisIndex;
}

sal_Int32 VDataSeries::getTotalPointCount() const
{
    return m_nPointCount;
}

double VDataSeries::getXValue( sal_Int32 index ) const
{
    double fRet = 0.0;
    if(m_aValues_X.is())
    {
        if( 0<=index && index<m_aValues_X.getLength() )
            fRet = m_aValues_X.Doubles[index];
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
            fRet = m_aValues_Y.Doubles[index];
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
    return m_aValues_Bubble_Size.getValue( index );
}

bool VDataSeries::hasExplicitNumberFormat( sal_Int32 nPointIndex, bool bForPercentage ) const
{
    rtl::OUString aPropName( bForPercentage ? C2U( "PercentageNumberFormat" ) : C2U( "NumberFormat" ) );
    bool bHasNumberFormat = false;
    uno::Reference< beans::XPropertySet > xPointProp( this->getPropertiesOfPoint( nPointIndex ));
    sal_Int32 nNumberFormat = -1;
    if( xPointProp.is() && (xPointProp->getPropertyValue(aPropName) >>= nNumberFormat) )
        bHasNumberFormat = true;
    return bHasNumberFormat;
}
sal_Int32 VDataSeries::getExplicitNumberFormat( sal_Int32 nPointIndex, bool bForPercentage ) const
{
    rtl::OUString aPropName( bForPercentage ? C2U( "PercentageNumberFormat" ) : C2U( "NumberFormat" ) );
    sal_Int32 nNumberFormat = -1;
    uno::Reference< beans::XPropertySet > xPointProp( this->getPropertiesOfPoint( nPointIndex ));
    if( xPointProp.is() )
        xPointProp->getPropertyValue(aPropName) >>= nNumberFormat;
    return nNumberFormat;
}
void VDataSeries::setRoleOfSequenceForDataLabelNumberFormatDetection( const rtl::OUString& rRole )
{
    if( rRole.equals(C2U("values-y")) )
        m_pValueSequenceForDataLabelNumberFormatDetection = &m_aValues_Y;
    else if( rRole.equals(C2U("values-size")) )
        m_pValueSequenceForDataLabelNumberFormatDetection = &m_aValues_Bubble_Size;
    else if( rRole.equals(C2U("values-min")) )
        m_pValueSequenceForDataLabelNumberFormatDetection = &m_aValues_Y_Min;
    else if( rRole.equals(C2U("values-max")) )
        m_pValueSequenceForDataLabelNumberFormatDetection = &m_aValues_Y_Max;
    else if( rRole.equals(C2U("values-first")) )
        m_pValueSequenceForDataLabelNumberFormatDetection = &m_aValues_Y_First;
    else if( rRole.equals(C2U("values-last")) )
        m_pValueSequenceForDataLabelNumberFormatDetection = &m_aValues_Y_Last;
    else if( rRole.equals(C2U("values-x")) )
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

sal_Int32 VDataSeries::getLabelPlacement( sal_Int32 nPointIndex, const uno::Reference< chart2::XChartType >& xChartType, sal_Int32 nDimensionCount, sal_Bool bSwapXAndY ) const
{
    sal_Int32 nLabelPlacement=0;
    try
    {
        uno::Reference< beans::XPropertySet > xPointProps( this->getPropertiesOfPoint( nPointIndex ) );
        if( xPointProps.is() )
            xPointProps->getPropertyValue( C2U( "LabelPlacement" ) ) >>= nLabelPlacement;

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

        DBG_ERROR("no label placement supported");
    }
    catch( uno::Exception& e )
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

uno::Sequence< double > VDataSeries::getAllX() const
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

uno::Sequence< double > VDataSeries::getAllY() const
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

double VDataSeries::getYMeanValue() const
{
    if( ::rtl::math::isNan( m_fYMeanValue ) )
    {
        uno::Reference< XRegressionCurveCalculator > xCalculator( RegressionCurveHelper::createRegressionCurveCalculatorByServiceName( C2U("com.sun.star.chart2.MeanValueRegressionCurve") ) );
        uno::Sequence< double > aXValuesDummy;
        xCalculator->recalculateRegression( aXValuesDummy, getAllY() );
        double fXDummy = 1.0;
        m_fYMeanValue = xCalculator->getCurveValue( fXDummy );
    }
    return m_fYMeanValue;
}

::std::auto_ptr< Symbol > getSymbolPropertiesFromPropertySet(
        const uno::Reference< beans::XPropertySet >& xProp )
{
    ::std::auto_ptr< Symbol > apSymbolProps( new Symbol() );
    try
    {
        if( xProp->getPropertyValue( C2U( "Symbol" ) ) >>= *apSymbolProps )
        {
            //use main color to fill symbols
            xProp->getPropertyValue( C2U( "Color" ) ) >>= apSymbolProps->FillColor;
            // border of symbols always same as fill color
            apSymbolProps->BorderColor = apSymbolProps->FillColor;
        }
        else
            apSymbolProps.reset();
    }
    catch( uno::Exception &e)
    {
        ASSERT_EXCEPTION( e );
    }
    return apSymbolProps;
}

Symbol* VDataSeries::getSymbolProperties( sal_Int32 index ) const
{
    Symbol* pRet=NULL;
    if( isAttributedDataPoint( index ) )
    {
        adaptPointCache( index );
        if(!m_apSymbolProperties_AttributedPoint.get())
            m_apSymbolProperties_AttributedPoint = getSymbolPropertiesFromPropertySet( this->getPropertiesOfPoint( index ) );
        pRet = m_apSymbolProperties_AttributedPoint.get();
        //if a single data point does not have symbols but the dataseries itself has symbols
        //we create an invisible symbol shape to enable selection of that point
        if( !pRet || pRet->Style == SymbolStyle_NONE )
        {
            if(!m_apSymbolProperties_Series.get())
                m_apSymbolProperties_Series = getSymbolPropertiesFromPropertySet( this->getPropertiesOfSeries() );
            if( m_apSymbolProperties_Series.get() && m_apSymbolProperties_Series->Style != SymbolStyle_NONE )
            {
                if(!m_apSymbolProperties_InvisibleSymbolForSelection.get())
                {
                    m_apSymbolProperties_InvisibleSymbolForSelection = ::std::auto_ptr< Symbol >( new Symbol() );
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
        if(!m_apSymbolProperties_Series.get())
            m_apSymbolProperties_Series = getSymbolPropertiesFromPropertySet( this->getPropertiesOfSeries() );
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

uno::Reference< beans::XPropertySet > VDataSeries::getYErrorBarProperties( sal_Int32 index ) const
{
    uno::Reference< beans::XPropertySet > xErrorBarProp;

    uno::Reference< beans::XPropertySet > xPointProp( this->getPropertiesOfPoint( index ));
    if( xPointProp.is() )
        xPointProp->getPropertyValue( C2U( "ErrorBarY" )) >>= xErrorBarProp;
    return xErrorBarProp;
}

bool VDataSeries::hasPointOwnColor( sal_Int32 index ) const
{
    if( !isAttributedDataPoint(index) )
        return false;

    try
    {
        uno::Reference< beans::XPropertyState > xPointState( this->getPropertiesOfPoint(index), uno::UNO_QUERY_THROW );
        return (xPointState->getPropertyState( C2U("Color")) != beans::PropertyState_DEFAULT_VALUE );
    }
    catch( uno::Exception& e)
    {
        ASSERT_EXCEPTION( e );
    }
    return false;
}

bool VDataSeries::isAttributedDataPoint( sal_Int32 index ) const
{
    //returns true if the data point assigned by the given index has set it's own properties
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
        xSeriesProp->getPropertyValue( C2U("VaryColorsByPoint") ) >>= bVaryColorsByPoint;
    return bVaryColorsByPoint;
}

uno::Reference< beans::XPropertySet > VDataSeries::getPropertiesOfPoint( sal_Int32 index ) const
{
    if( isAttributedDataPoint( index ) )
        return m_xDataSeries->getDataPointByIndex(index);
    return this->getPropertiesOfSeries();
}

uno::Reference< beans::XPropertySet > VDataSeries::getPropertiesOfSeries() const
{
    return  uno::Reference<beans::XPropertySet>(m_xDataSeries, uno::UNO_QUERY );
}

::std::auto_ptr< DataPointLabel > getDataPointLabelFromPropertySet(
        const uno::Reference< beans::XPropertySet >& xProp )
{
    ::std::auto_ptr< DataPointLabel > apLabel( new DataPointLabel() );
    try
    {
        if( !(xProp->getPropertyValue( C2U( "Label" ) ) >>= *apLabel) )
            apLabel.reset();
    }
    catch( uno::Exception &e)
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
    DataPointLabel* pRet = NULL;
    if( isAttributedDataPoint( index ) )
    {
        adaptPointCache( index );
        if( !m_apLabel_AttributedPoint.get() )
            m_apLabel_AttributedPoint = getDataPointLabelFromPropertySet( this->getPropertiesOfPoint( index ) );
        pRet = m_apLabel_AttributedPoint.get();
    }
    else
    {
        if(!m_apLabel_Series.get())
            m_apLabel_Series = getDataPointLabelFromPropertySet( this->getPropertiesOfPoint( index ) );
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
        return 0;
    return pLabel;
}

bool VDataSeries::getTextLabelMultiPropertyLists( sal_Int32 index
    , tNameSequence*& pPropNames
    , tAnySequence*& pPropValues ) const
{
    pPropNames = NULL; pPropValues = NULL;
    uno::Reference< beans::XPropertySet > xTextProp;
    bool bDoDynamicFontResize = false;
    if( isAttributedDataPoint( index ) )
    {
        adaptPointCache( index );
        if(!m_apLabelPropValues_AttributedPoint.get())
        {
            pPropNames = new tNameSequence();
            pPropValues = new tAnySequence();
            xTextProp.set( this->getPropertiesOfPoint( index ));
            PropertyMapper::getTextLabelMultiPropertyLists( xTextProp, *pPropNames, *pPropValues );
            m_apLabelPropNames_AttributedPoint = ::std::auto_ptr< tNameSequence >(pPropNames);
            m_apLabelPropValues_AttributedPoint = ::std::auto_ptr< tAnySequence >(pPropValues);
            bDoDynamicFontResize = true;
        }
        pPropNames = m_apLabelPropNames_AttributedPoint.get();
        pPropValues = m_apLabelPropValues_AttributedPoint.get();
    }
    else
    {
        if(!m_apLabelPropValues_Series.get())
        {
            pPropNames = new tNameSequence();
            pPropValues = new tAnySequence();
            xTextProp.set( this->getPropertiesOfPoint( index ));
            PropertyMapper::getTextLabelMultiPropertyLists( xTextProp, *pPropNames, *pPropValues );
            m_apLabelPropNames_Series = ::std::auto_ptr< tNameSequence >(pPropNames);
            m_apLabelPropValues_Series = ::std::auto_ptr< tAnySequence >(pPropValues);
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
    if(pPropNames&&pPropValues)
        return true;
    return false;
}

void VDataSeries::setMissingValueTreatment( sal_Int32 nMissingValueTreatment )
{
    m_nMissingValueTreatment = nMissingValueTreatment;
}

sal_Int32 VDataSeries::getMissingValueTreatment() const
{
    return m_nMissingValueTreatment;
}

//.............................................................................
} //namespace chart
//.............................................................................
