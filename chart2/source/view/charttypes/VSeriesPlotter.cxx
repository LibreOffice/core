/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: VSeriesPlotter.cxx,v $
 *
 *  $Revision: 1.34 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 13:34:36 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/


// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_chart2.hxx"
#include "VSeriesPlotter.hxx"
#include "ShapeFactory.hxx"

#include "CommonConverters.hxx"
#include "macros.hxx"
#include "ViewDefines.hxx"
#include "chartview/ObjectIdentifier.hxx"
#include "StatisticsHelper.hxx"
#include "PlottingPositionHelper.hxx"
#include "LabelPositionHelper.hxx"
#include "Clipping.hxx"

//only for creation: @todo remove if all plotter are uno components and instanciated via servicefactory
#include "BarChart.hxx"
#include "PieChart.hxx"
#include "AreaChart.hxx"
//

#ifndef _COM_SUN_STAR_CHART2_ERRORBARSTYLE_HPP_
#include <com/sun/star/chart2/ErrorBarStyle.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART2_XREGRESSIONCURVECONTAINER_HPP_
#include <com/sun/star/chart2/XRegressionCurveContainer.hpp>
#endif

#ifndef _SVX_UNOPRNMS_HXX
#include <svx/unoprnms.hxx>
#endif

#ifndef _TOOLS_COLOR_HXX
#include <tools/color.hxx>
#endif
// header for class OUStringBuffer
#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif
#ifndef INCLUDED_RTL_MATH_HXX
#include <rtl/math.hxx>
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

#include <algorithm>

//.............................................................................
namespace chart
{
//.............................................................................
using namespace ::com::sun::star;
using namespace ::com::sun::star::chart2;

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

VDataSeriesGroup::VDataSeriesGroup()
        : m_aSeriesVector()
        , m_bSumValuesDirty(true)
        , m_fPositiveSum(0.0)
        , m_fNegativeSum(0.0)
        , m_bMaxPointCountDirty(true)
        , m_nMaxPointCount(0)

{
}

VDataSeriesGroup::VDataSeriesGroup( VDataSeries* pSeries )
        : m_aSeriesVector(1,pSeries)
        , m_bSumValuesDirty(true)
        , m_fPositiveSum(0.0)
        , m_fNegativeSum(0.0)
        , m_bMaxPointCountDirty(true)
        , m_nMaxPointCount(0)
{
}

VDataSeriesGroup::VDataSeriesGroup( const ::std::vector< VDataSeries* >& rSeriesVector )
        : m_aSeriesVector(rSeriesVector)
        , m_bSumValuesDirty(true)
        , m_fPositiveSum(0.0)
        , m_fNegativeSum(0.0)
        , m_bMaxPointCountDirty(true)
        , m_nMaxPointCount(0)
{
}
VDataSeriesGroup::~VDataSeriesGroup()
{
}

void VDataSeriesGroup::deleteSeries()
{
    //delete all data series help objects:
    ::std::vector< VDataSeries* >::const_iterator       aIter = m_aSeriesVector.begin();
    const ::std::vector< VDataSeries* >::const_iterator aEnd  = m_aSeriesVector.end();
    for( ; aIter != aEnd; aIter++ )
    {
        delete *aIter;
    }
    m_aSeriesVector.clear();
}

void VDataSeriesGroup::addSeries( VDataSeries* pSeries )
{
    m_aSeriesVector.push_back(pSeries);
    m_bSumValuesDirty=true;
    m_bMaxPointCountDirty=true;
}

sal_Int32 VDataSeriesGroup::getSeriesCount() const
{
    return m_aSeriesVector.size();
}

void VDataSeriesGroup::setSums( double fPositiveSum, double fNegativeSum )
{
    m_fPositiveSum = fPositiveSum;
    m_fNegativeSum = fNegativeSum;
    m_bSumValuesDirty = false;
}

bool VDataSeriesGroup::getSums( double& rfPositiveSum, double& rfNegativeSum ) const
{
    if(m_bSumValuesDirty)
        return false;
    rfPositiveSum = m_fPositiveSum;
    rfNegativeSum = m_fNegativeSum;
    return true;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

VSeriesPlotter::VSeriesPlotter( const uno::Reference<XChartType>& xChartTypeModel, bool bCategoryXAxis )
        : PlotterBase(2)
        , m_xChartTypeModel(xChartTypeModel)
        , m_xChartTypeModelProps( uno::Reference< beans::XPropertySet >::query( xChartTypeModel ))
        , m_aXSlots()
        , m_bCategoryXAxis(bCategoryXAxis)
{
    DBG_ASSERT(m_xChartTypeModel.is(),"no XChartType available in view, fallback to default values may be wrong");
    if(m_xChartTypeModelProps.is() )
        m_xChartTypeModelProps->getPropertyValue( C2U( "Dimension" ) ) >>= m_nDimension;
}

VSeriesPlotter::~VSeriesPlotter()
{
    //delete all data series help objects:
    ::std::vector< VDataSeriesGroup >::iterator            aXSlotIter = m_aXSlots.begin();
    const ::std::vector< VDataSeriesGroup >::const_iterator aXSlotEnd =  m_aXSlots.end();
    for( ; aXSlotIter != aXSlotEnd; aXSlotIter++ )
    {
        aXSlotIter->deleteSeries();
    }
    m_aXSlots.clear();
}

//shape property -- chart model object property
tMakePropertyNameMap VSeriesPlotter::m_aShapePropertyMapForArea =
        tMakePropertyNameMap
        //area properties
        ( C2U( UNO_NAME_FILLSTYLE ), C2U("FillStyle") )
        ( C2U( UNO_NAME_FILLCOLOR ), C2U("Color") )
        ( C2U(UNO_NAME_FILLGRADIENT), C2U("Gradient") )
        ( C2U(UNO_NAME_FILL_TRANSPARENCE), C2U("Transparency") )
        ( C2U("FillTransparenceGradient"), C2U("TransparencyGradient") )
        ( C2U(UNO_NAME_FILLHATCH), C2U("Hatch") )
        //line properties
        ( C2U(UNO_NAME_LINESTYLE), C2U("BorderStyle") )
        ( C2U(UNO_NAME_LINECOLOR), C2U("BorderColor") )
        ( C2U(UNO_NAME_LINEWIDTH), C2U("BorderWidth") )
        ( C2U(UNO_NAME_LINEDASH), C2U("BorderDash") )
        ( C2U(UNO_NAME_LINETRANSPARENCE), C2U("BorderTransparency") )
    ;

void VSeriesPlotter::addSeries( VDataSeries* pSeries, sal_Int32 xSlot,sal_Int32 ySlot )
{
    //take ownership of pSeries

    DBG_ASSERT( pSeries, "series to add is NULL" );
    if(!pSeries)
        return;

    if(m_bCategoryXAxis)
        pSeries->setCategoryXAxis();

    if(xSlot<0 || xSlot>=static_cast<sal_Int32>(m_aXSlots.size()))
    {
        //append the series to already existing x series
        m_aXSlots.push_back( VDataSeriesGroup(pSeries) );
    }
    else
    {
        //x slot is already occupied
        //y slot decides what to do:

        VDataSeriesGroup& rYSlots = m_aXSlots[xSlot];
        sal_Int32 nYSlotCount = rYSlots.getSeriesCount();

        if( ySlot < -1 )
        {
            //move all existing series in the xSlot to next slot
            //@todo
            DBG_ASSERT(0,"Not implemented yet");
        }
        else if( ySlot == -1 || ySlot >= nYSlotCount)
        {
            //append the series to already existing y series
            rYSlots.addSeries(pSeries);
        }
        else
        {
            //y slot is already occupied
            //insert at given y and x position

            //@todo
            DBG_ASSERT(0,"Not implemented yet");
        }
    }
}

uno::Reference< drawing::XShapes > VSeriesPlotter::getSeriesGroupShape( VDataSeries* pDataSeries
                                        , const uno::Reference< drawing::XShapes >& xTarget )
{
    if(pDataSeries->m_xShape.is())
        return uno::Reference<drawing::XShapes>( pDataSeries->m_xShape, uno::UNO_QUERY );

    //create a group shape for this series and add to logic target:
    uno::Reference< drawing::XShapes > xShapes(
        createGroupShape( xTarget,pDataSeries->getCID() ));
    uno::Reference<drawing::XShape> xShape =
                uno::Reference<drawing::XShape>( xShapes, uno::UNO_QUERY );
    pDataSeries->m_xShape.set(xShape);
    return xShapes;
}

uno::Reference< drawing::XShapes > VSeriesPlotter::getSeriesGroupShapeFrontChild( VDataSeries* pDataSeries
                                        , const uno::Reference< drawing::XShapes >& xTarget )
{
    if(pDataSeries->m_xShapeFrontChild.is())
        return uno::Reference<drawing::XShapes>( pDataSeries->m_xShapeFrontChild, uno::UNO_QUERY );

    //ensure that the series group shape is already created
    uno::Reference< drawing::XShapes > xSeriesShapes( this->getSeriesGroupShape( pDataSeries, xTarget ) );
    //ensure that the back child is created first
    this->getSeriesGroupShapeBackChild( pDataSeries, xTarget );
    //use series group shape as parent for the new created front group shape
    uno::Reference< drawing::XShapes > xShapes( createGroupShape( xSeriesShapes ) );
    pDataSeries->m_xShapeFrontChild.set(xShapes);
    return xShapes;
}

uno::Reference< drawing::XShapes > VSeriesPlotter::getSeriesGroupShapeBackChild( VDataSeries* pDataSeries
                                        , const uno::Reference< drawing::XShapes >& xTarget )
{
    if(pDataSeries->m_xShapeBackChild.is())
        return uno::Reference<drawing::XShapes>( pDataSeries->m_xShapeBackChild, uno::UNO_QUERY );

    //ensure that the series group shape is already created
    uno::Reference< drawing::XShapes > xSeriesShapes( this->getSeriesGroupShape( pDataSeries, xTarget ) );
    //use series group shape as parent for the new created back group shape
    uno::Reference< drawing::XShapes > xShapes( createGroupShape( xSeriesShapes ) );
    pDataSeries->m_xShapeBackChild.set(xShapes);
    return xShapes;
}

uno::Reference< drawing::XShapes > VSeriesPlotter::getLabelsGroupShape( VDataSeries& rDataSeries
                                        , const uno::Reference< drawing::XShapes >& xTextTarget )
{
    //xTextTarget needs to be a 2D shape container always!

    if(rDataSeries.m_xLabelsShape.is())
        return uno::Reference<drawing::XShapes>( rDataSeries.m_xLabelsShape, uno::UNO_QUERY );

    //create a 2D group shape for texts of this series and add to text target:
    uno::Reference< drawing::XShapes > xShapes(
        m_pShapeFactory->createGroup2D( xTextTarget, rDataSeries.getLabelsCID() ));
    uno::Reference<drawing::XShape> xShape =
                uno::Reference<drawing::XShape>( xShapes, uno::UNO_QUERY );
    rDataSeries.m_xLabelsShape.set(xShape);
    return xShapes;
}

uno::Reference< drawing::XShapes > VSeriesPlotter::getErrorBarsGroupShape( VDataSeries& rDataSeries
                                        , const uno::Reference< drawing::XShapes >& xTarget )
{
    if(rDataSeries.m_xErrorBarsShape.is())
        return uno::Reference<drawing::XShapes>( rDataSeries.m_xErrorBarsShape, uno::UNO_QUERY );

    //create a group shape for this series and add to logic target:
    uno::Reference< drawing::XShapes > xShapes(
        this->createGroupShape( xTarget,rDataSeries.getErrorBarsCID() ));
    uno::Reference<drawing::XShape> xShape =
                uno::Reference<drawing::XShape>( xShapes, uno::UNO_QUERY );
    rDataSeries.m_xErrorBarsShape.set(xShape);
    return xShapes;

}

void VSeriesPlotter::createDataLabel( const uno::Reference< drawing::XShapes >& xTarget
                    , VDataSeries& rDataSeries
                    , sal_Int32 nPointIndex
                    , double fValue
                    , double fSumValue
                    , const awt::Point& rScreenPosition2D
                    , LabelAlignment eAlignment )
{
    uno::Reference< drawing::XShapes > xTarget_( this->getLabelsGroupShape(rDataSeries, xTarget) );

    //check wether the label needs to be created and how:
    DataPointLabel* pLabel = rDataSeries.getDataPointLabelIfLabel( nPointIndex );

    if( !pLabel )
        return;

    //------------------------------------------------
    //prepare text
    ::rtl::OUStringBuffer aText;
    {
        if(pLabel->ShowNumberInPercent)
        {
            fValue = fValue*100.0/fSumValue;
            sal_Int32 nRound = static_cast< sal_Int32 >( fValue * 100.0 );
            fValue = nRound/100.0;
            if( fValue < 0 )
                fValue*=-1.0;
        }
        if(pLabel->ShowCategoryName)
            aText.append( rDataSeries.getCategoryString(nPointIndex) );

        if(pLabel->ShowNumber || pLabel->ShowNumberInPercent)
        {
            if(aText.getLength())
                aText.append(sal_Unicode(' '));

            //@todo as default use numberformat and formatter from calc fro this range
            sal_Unicode cDecSeparator = '.';//@todo get this locale dependent
            aText.append( ::rtl::math::doubleToUString( fValue
                            , rtl_math_StringFormat_G //rtl_math_StringFormat
                            , 3// DecPlaces
                            , cDecSeparator
                            //,sal_Int32 const * pGroups
                            //,sal_Unicode cGroupSeparator
                            ,false //bEraseTrailingDecZeros
                            ) );
        }
        if(pLabel->ShowNumberInPercent)
            aText.append(sal_Unicode('%'));
    }
    //------------------------------------------------
    //prepare properties for multipropertyset-interface of shape
    tNameSequence* pPropNames;
    tAnySequence* pPropValues;
    if( !rDataSeries.getTextLabelMultiPropertyLists( nPointIndex, pPropNames, pPropValues ) )
        return;
    uno::Any* pCIDAny = PropertyMapper::getValuePointer(*pPropValues,*pPropNames,C2U("Name"));
    if(pCIDAny)
    {
        rtl::OUString aCID = ObjectIdentifier::createPointCID( rDataSeries.getLabelCID_Stub(),nPointIndex );
        *pCIDAny = uno::makeAny(aCID);
    }
    LabelPositionHelper::changeTextAdjustment( *pPropValues, *pPropNames, eAlignment );

    //------------------------------------------------
    //create text shape
    uno::Reference< drawing::XShape > xTextShape = ShapeFactory(m_xShapeFactory).
        createText( xTarget_, aText.makeStringAndClear()
                    , *pPropNames, *pPropValues
                    , ShapeFactory::makeTransformation( rScreenPosition2D ) );
}

namespace
{
double lcl_getErrorBarLogicLength(
    const uno::Sequence< double > & rData,
    uno::Reference< beans::XPropertySet > xProp,
    ErrorBarStyle eErrorBarStyle,
    sal_Int32 nIndex,
    bool bPositive )
{
    double fResult;
    ::rtl::math::setNan( & fResult );
    try
    {
        switch( eErrorBarStyle )
        {
            case ErrorBarStyle_VARIANCE:
                fResult = StatisticsHelper::getVariance( rData );
                break;
            case ErrorBarStyle_STANDARD_DEVIATION:
                fResult = StatisticsHelper::getStandardDeviation( rData );
                break;
            case ErrorBarStyle_RELATIVE:
            {
                double fPercent;
                if( xProp->getPropertyValue( bPositive
                                             ? C2U("PositiveError")
                                             : C2U("NegativeError")) >>= fPercent )
                {
                    if( nIndex >=0 && nIndex < rData.getLength() &&
                        ! ::rtl::math::isNan( rData[nIndex] ) &&
                        ! ::rtl::math::isNan( fPercent ))
                    {
                        fResult = rData[nIndex] * fPercent / 100.0;
                    }
                }
            }
            break;
            case ErrorBarStyle_ABSOLUTE:
                xProp->getPropertyValue( bPositive
                                         ? C2U("PositiveError")
                                         : C2U("NegativeError")) >>= fResult;
                break;
            case ErrorBarStyle_ERROR_MARGIN:
            {
                // todo: check if this is really what's called error-margin
                double fPercent;
                if( xProp->getPropertyValue( bPositive
                                             ? C2U("PositiveError")
                                             : C2U("NegativeError")) >>= fPercent )
                {
                    double fMaxValue = *(::std::max_element(
                                             rData.getConstArray(),
                                             rData.getConstArray() + rData.getLength()));
                    if( ! ::rtl::math::isNan( fMaxValue ) &&
                        ! ::rtl::math::isNan( fPercent ))
                    {
                        fResult = fMaxValue * fPercent / 100.0;
                    }
                }
            }
            break;
            case ErrorBarStyle_STANDARD_ERROR:
                fResult = StatisticsHelper::getStandardError( rData );
                break;
            case ErrorBarStyle_FROM_DATA:
                // todo: implement
                break;

                // to avoid warning
            case ErrorBarStyle_MAKE_FIXED_SIZE:
                break;
        }
    }
    catch( uno::Exception & e )
    {
        ASSERT_EXCEPTION( e );
    }

    return fResult;
}

bool lcl_getErrorBarPosAndSize(
    double fErrorBarLength,
    ShapeFactory::tErrorBarDirection eDirection,
    const uno::Reference< XTransformation > & xTrans,
    const PlottingPositionHelper & rPositionHelper,
    drawing::Position3D & rInOutNewPos,
    drawing::Direction3D & rOutNewSize )
{
    bool bClipped = false;

    if( xTrans.is())
    {
        drawing::Position3D aUpperLeft( rInOutNewPos ), aLowerRight( rInOutNewPos );
        switch( eDirection )
        {
            case ShapeFactory::ERROR_BAR_UP:
                aUpperLeft.PositionY += fErrorBarLength;
                break;
            case ShapeFactory::ERROR_BAR_DOWN:
                aLowerRight.PositionY -= fErrorBarLength;
                break;
            case ShapeFactory::ERROR_BAR_RIGHT:
                aLowerRight.PositionX += fErrorBarLength;
                break;
            case ShapeFactory::ERROR_BAR_LEFT:
                aUpperLeft.PositionX -= fErrorBarLength;
                break;
        }

        rPositionHelper.clipLogicValues(
            & rInOutNewPos.PositionX, & rInOutNewPos.PositionY, & rInOutNewPos.PositionZ );
        rPositionHelper.doLogicScaling(
            & rInOutNewPos.PositionX, & rInOutNewPos.PositionY, & rInOutNewPos.PositionZ );
        rInOutNewPos = drawing::Position3D(
            SequenceToPosition3D( xTrans->transform( Position3DToSequence( rInOutNewPos ))));

        double fOldX = aUpperLeft.PositionX;
        double fOldY = aUpperLeft.PositionY;
        rPositionHelper.clipLogicValues(
            & aUpperLeft.PositionX, & aUpperLeft.PositionY, & aUpperLeft.PositionZ );
        bClipped = bClipped ||
            ( ( eDirection == ShapeFactory::ERROR_BAR_UP ||
                eDirection == ShapeFactory::ERROR_BAR_LEFT ) &&
              ( fOldX != aUpperLeft.PositionX ||
                fOldY != aUpperLeft.PositionY ));
        rPositionHelper.doLogicScaling(
            & aUpperLeft.PositionX, & aUpperLeft.PositionY, & aUpperLeft.PositionZ );
        drawing::Position3D aNewUpperLeft(
            SequenceToPosition3D( xTrans->transform( Position3DToSequence( aUpperLeft ))));

        fOldX = aLowerRight.PositionX;
        fOldY = aLowerRight.PositionY;
        rPositionHelper.clipLogicValues(
            & aLowerRight.PositionX, & aLowerRight.PositionY, & aLowerRight.PositionZ );
        bClipped = bClipped ||
            ( ( eDirection == ShapeFactory::ERROR_BAR_DOWN ||
                eDirection == ShapeFactory::ERROR_BAR_RIGHT ) &&
              ( fOldX != aLowerRight.PositionX ||
                fOldY != aLowerRight.PositionY ));
        rPositionHelper.doLogicScaling(
            & aLowerRight.PositionX, & aLowerRight.PositionY, & aLowerRight.PositionZ );
        drawing::Position3D aNewLowerRight(
            SequenceToPosition3D( xTrans->transform( Position3DToSequence( aLowerRight ))));

        rOutNewSize = drawing::Direction3D(
            aNewLowerRight.PositionX - aNewUpperLeft.PositionX,
            aNewLowerRight.PositionY - aNewUpperLeft.PositionY,
            rInOutNewPos.PositionZ );

        // in 100th of a mm
        double fFixedWidth = 200.0;
        if( eDirection == ShapeFactory::ERROR_BAR_LEFT ||
            eDirection == ShapeFactory::ERROR_BAR_RIGHT )
        {
            rOutNewSize.DirectionY = fFixedWidth;
        }
        else
        {
            rOutNewSize.DirectionX = fFixedWidth;
        }
    }
    return bClipped;
}

} // anonymous namespace

// virtual
void VSeriesPlotter::createErrorBar(
      const uno::Reference< drawing::XShapes >& xTarget
    , const drawing::Position3D& rPos
    , const uno::Reference< beans::XPropertySet > & xErrorBarProperties
    , const uno::Sequence< double > & rData
    , sal_Int32 nIndex
    , bool bVertical /* = true */
    )
{
    if( ! xErrorBarProperties.is())
        return;

    try
    {
        sal_Bool bShowPos, bShowNeg;
        ErrorBarStyle eErrorBarStyle;

        if( ! (xErrorBarProperties->getPropertyValue( C2U( "ShowPositiveError" )) >>= bShowPos ))
            bShowPos = sal_False;
        if( ! (xErrorBarProperties->getPropertyValue( C2U( "ShowNegativeError" )) >>= bShowNeg ))
            bShowNeg = sal_False;
        if( ! (xErrorBarProperties->getPropertyValue( C2U( "ErrorBarStyle" )) >>= eErrorBarStyle ))
            eErrorBarStyle = ErrorBarStyle_VARIANCE;

        uno::Reference< XTransformation > xTrans( m_pPosHelper->getTransformationLogicToScene() );

        if( bShowPos )
        {
            ShapeFactory::tErrorBarDirection eErrorBarDir =
                bVertical
                ? ShapeFactory::ERROR_BAR_UP
                : ShapeFactory::ERROR_BAR_RIGHT;
            double fErrorBarLength = lcl_getErrorBarLogicLength(
                rData, xErrorBarProperties, eErrorBarStyle, nIndex, true /* positive */ );

            drawing::Position3D  aPos( rPos );
            drawing::Direction3D aSize;
            bool bClipped =
                lcl_getErrorBarPosAndSize( fErrorBarLength, eErrorBarDir, xTrans, *m_pPosHelper, aPos, aSize );

            setMappedProperties(
                m_pShapeFactory->createErrorBar2D( xTarget, aPos, aSize, eErrorBarDir, bClipped ),
                xErrorBarProperties,
                PropertyMapper::getPropertyNameMapForLineProperties() );
        }

        if( bShowNeg )
        {
            ShapeFactory::tErrorBarDirection eErrorBarDir =
                bVertical
                ? ShapeFactory::ERROR_BAR_DOWN
                : ShapeFactory::ERROR_BAR_LEFT;
            double fErrorBarLength = lcl_getErrorBarLogicLength(
                rData, xErrorBarProperties, eErrorBarStyle, nIndex, false /* negative */ );

            drawing::Position3D  aPos( rPos );
            drawing::Direction3D aSize;
            bool bClipped =
                lcl_getErrorBarPosAndSize( fErrorBarLength, eErrorBarDir, xTrans, *m_pPosHelper, aPos, aSize );

            setMappedProperties(
                m_pShapeFactory->createErrorBar2D( xTarget, aPos, aSize, eErrorBarDir, bClipped ),
                xErrorBarProperties,
                PropertyMapper::getPropertyNameMapForLineProperties() );
        }
    }
    catch( uno::Exception & e )
    {
        ASSERT_EXCEPTION( e );
    }

}

// virtual
void VSeriesPlotter::createErrorBar_Y( const drawing::Position3D& rUnscaledLogicPosition
                            , VDataSeries& rVDataSeries, sal_Int32 nPointIndex
                            , const uno::Reference< drawing::XShapes >& xTarget )
{
    if(m_nDimension!=2)
        return;
    // error bars
    uno::Reference< beans::XPropertySet > xPointProp( rVDataSeries.getPropertiesOfPoint( nPointIndex ));
    uno::Reference< beans::XPropertySet > xErrorBarProp;
    if( xPointProp.is() && ( xPointProp->getPropertyValue( C2U( "ErrorBarY" )) >>= xErrorBarProp ) &&
        xErrorBarProp.is())
    {
        uno::Reference< drawing::XShapes > xErrorBarsGroup_Shapes(
            this->getErrorBarsGroupShape(rVDataSeries, xTarget) );

        createErrorBar( xErrorBarsGroup_Shapes
            , rUnscaledLogicPosition, xErrorBarProp
            , rVDataSeries.getAllY(), nPointIndex
            , true /* bVertical */ );
    }
}

void VSeriesPlotter::createRegressionCurvesShapes( const VDataSeries& rVDataSeries
                            , const uno::Reference< drawing::XShapes >& xTarget )
{
    if(m_nDimension!=2)
        return;
    uno::Reference< XRegressionCurveContainer > xRegressionContainer(
                rVDataSeries.getModel(), uno::UNO_QUERY );
    if(!xRegressionContainer.is())
        return;
    double fMinX = m_pPosHelper->getLogicMinX();
    double fMaxX = m_pPosHelper->getLogicMaxX();

    uno::Sequence< uno::Reference< XRegressionCurve > > aCurveList =
        xRegressionContainer->getRegressionCurves();
    for(sal_Int32 nN=0; nN<aCurveList.getLength(); nN++)
    {
        uno::Reference< XRegressionCurveCalculator > xRegressionCurveCalculator(
            aCurveList[nN]->getCalculator() );
        if( ! xRegressionCurveCalculator.is())
            continue;
        xRegressionCurveCalculator->recalculateRegression( rVDataSeries.getAllX(), rVDataSeries.getAllY() );

        sal_Int32 nRegressionPointCount = 50;//@todo find a more optimal solution if more complicated curve types are introduced
        drawing::PolyPolygonShape3D aRegressionPoly;
        aRegressionPoly.SequenceX.realloc(1);
        aRegressionPoly.SequenceY.realloc(1);
        aRegressionPoly.SequenceZ.realloc(1);
        aRegressionPoly.SequenceX[0].realloc(nRegressionPointCount);
        aRegressionPoly.SequenceY[0].realloc(nRegressionPointCount);
        aRegressionPoly.SequenceZ[0].realloc(nRegressionPointCount);
        sal_Int32 nRealPointCount=0;
        for(sal_Int32 nP=0; nP<nRegressionPointCount; nP++)
        {
            double fLogicX = fMinX + nP*(fMaxX-fMinX)/double(nRegressionPointCount-1);
            double fLogicY = xRegressionCurveCalculator->getCurveValue( fLogicX );
            double fLogicZ = 0.0;//dummy

            m_pPosHelper->doLogicScaling( &fLogicX, &fLogicY, &fLogicZ );

            if(    !::rtl::math::isNan(fLogicX) && !::rtl::math::isInf(fLogicX)
                    && !::rtl::math::isNan(fLogicY) && !::rtl::math::isInf(fLogicY)
                    && !::rtl::math::isNan(fLogicZ) && !::rtl::math::isInf(fLogicZ) )
            {
                drawing::Position3D aScaledLogicPosition( fLogicX, fLogicY,fLogicZ);
                drawing::Position3D aTransformedPosition(
                        SequenceToPosition3D(
                            m_pPosHelper->getTransformationLogicToScene()->transform(
                                Position3DToSequence(aScaledLogicPosition) ) ) );
                aRegressionPoly.SequenceX[0][nRealPointCount] = aTransformedPosition.PositionX;
                aRegressionPoly.SequenceY[0][nRealPointCount] = aTransformedPosition.PositionY;
                nRealPointCount++;
            }
        }
        aRegressionPoly.SequenceX[0].realloc(nRealPointCount);
        aRegressionPoly.SequenceY[0].realloc(nRealPointCount);
        aRegressionPoly.SequenceZ[0].realloc(nRealPointCount);

        drawing::PolyPolygonShape3D aClippedPoly;
        Clipping::clipPolygonAtRectangle( aRegressionPoly, m_pPosHelper->getScaledLogicClipDoubleRect(), aClippedPoly );
        aRegressionPoly = aClippedPoly;
        m_pPosHelper->transformScaledLogicToScene( aRegressionPoly );

        if( !aRegressionPoly.SequenceX.getLength() || !aRegressionPoly.SequenceX[0].getLength() )
            continue;

        uno::Reference< beans::XPropertySet > xCurveModelProp( aCurveList[nN], uno::UNO_QUERY );
        VLineProperties aVLineProperties;
        aVLineProperties.initFromPropertySet( xCurveModelProp );

        //create an extra group shape for each curve for selection handling
        uno::Reference< drawing::XShapes > xRegressionGroupShapes =
            createGroupShape( xTarget, rVDataSeries.getDataCurveCID( xCurveModelProp ) );
        uno::Reference< drawing::XShape > xShape = m_pShapeFactory->createLine2D(
            xRegressionGroupShapes, PolyToPointSequence( aRegressionPoly ), aVLineProperties );
        m_pShapeFactory->setShapeName( xShape, C2U("MarkHandles") );
    }
}

void VSeriesPlotter::setMappedProperties(
          const uno::Reference< drawing::XShape >& xTargetShape
        , const uno::Reference< beans::XPropertySet >& xSource
        , const tPropertyNameMap& rMap )
{
    uno::Reference< beans::XPropertySet > xTargetProp( xTargetShape, uno::UNO_QUERY );
    PropertyMapper::setMappedProperties(xTargetProp,xSource,rMap);
}

//-------------------------------------------------------------------------
// MinimumAndMaximumSupplier
//-------------------------------------------------------------------------

double VSeriesPlotter::getMinimumX()
{
    if( m_bCategoryXAxis )
        return 0.5;//first category (index 0) matches with real number 1.0

    double fMinimum, fMaximum;
    this->getMinimumAndMaximiumX( fMinimum, fMaximum );
    return fMinimum;
}
double VSeriesPlotter::getMaximumX()
{
    if( m_bCategoryXAxis )
    {
        //return category count
        sal_Int32 nPointCount = getPointCount( m_aXSlots );
        return nPointCount+0.5;//first category (index 0) matches with real number 1.0
    }

    double fMinimum, fMaximum;
    this->getMinimumAndMaximiumX( fMinimum, fMaximum );
    return fMaximum;
}

double VSeriesPlotter::getMinimumYInRange( double fMinimumX, double fMaximumX )
{
    if( !m_bCategoryXAxis )
    {
        double fMinY, fMaxY;
        this->getMinimumAndMaximiumYInContinuousXRange( fMinY, fMaxY, fMinimumX, fMaximumX );
        return fMinY;
    }

    double fMinimum, fMaximum;
    ::rtl::math::setInf(&fMinimum, false);
    ::rtl::math::setInf(&fMaximum, true);
    for(size_t nN =0; nN<m_aXSlots.size();nN++ )
    {
        double fLocalMinimum, fLocalMaximum;
        if( m_aXSlots[nN].calculateYMinAndMaxForCategoryRange(
                            static_cast<sal_Int32>(fMinimumX-0.5) //first category (index 0) matches with real number 1.0
                            , static_cast<sal_Int32>(fMaximumX-0.5) //first category (index 0) matches with real number 1.0
                            , fLocalMinimum, fLocalMaximum ) )
        {
            if(fMaximum<fLocalMaximum)
                fMaximum=fLocalMaximum;
            if(fMinimum>fLocalMinimum)
                fMinimum=fLocalMinimum;
        }
    }
    if(::rtl::math::isInf(fMinimum))
        ::rtl::math::setNan(&fMinimum);
    return fMinimum;
}

double VSeriesPlotter::getMaximumYInRange( double fMinimumX, double fMaximumX )
{
    if( !m_bCategoryXAxis )
    {
        double fMinY, fMaxY;
        this->getMinimumAndMaximiumYInContinuousXRange( fMinY, fMaxY, fMinimumX, fMaximumX );
        return fMaxY;
    }

    double fMinimum, fMaximum;
    ::rtl::math::setInf(&fMinimum, false);
    ::rtl::math::setInf(&fMaximum, true);
    for(size_t nN =0; nN<m_aXSlots.size();nN++ )
    {
        double fLocalMinimum, fLocalMaximum;
        if( m_aXSlots[nN].calculateYMinAndMaxForCategoryRange(
                            static_cast<sal_Int32>(fMinimumX-0.5) //first category (index 0) matches with real number 1.0
                            , static_cast<sal_Int32>(fMaximumX-0.5) //first category (index 0) matches with real number 1.0
                            , fLocalMinimum, fLocalMaximum ) )
        {
            if(fMaximum<fLocalMaximum)
                fMaximum=fLocalMaximum;
            if(fMinimum>fLocalMinimum)
                fMinimum=fLocalMinimum;
        }
    }
    if(::rtl::math::isInf(fMaximum))
        ::rtl::math::setNan(&fMaximum);
    return fMaximum;
}

double VSeriesPlotter::getMinimumZ()
{
    //this is the default for all charts without a meaningfull z axis
    return -0.5;
}
double VSeriesPlotter::getMaximumZ()
{
    //this is the default for all charts without a meaningfull z axis
    return 0.5;
}

void VSeriesPlotter::getMinimumAndMaximiumX( double& rfMinimum, double& rfMaximum ) const
{
    ::rtl::math::setInf(&rfMinimum, false);
    ::rtl::math::setInf(&rfMaximum, true);

    ::std::vector< VDataSeriesGroup >::const_iterator aXSlotIter = m_aXSlots.begin();
    const ::std::vector< VDataSeriesGroup >::const_iterator aXSlotEnd = m_aXSlots.end();
    for( ; aXSlotIter != aXSlotEnd; aXSlotIter++ )
    {
        double fLocalMinimum, fLocalMaximum;
        aXSlotIter->getMinimumAndMaximiumX( fLocalMinimum, fLocalMaximum );
        if( !::rtl::math::isNan(fLocalMinimum) && fLocalMinimum< rfMinimum )
            rfMinimum = fLocalMinimum;
        if( !::rtl::math::isNan(fLocalMaximum) && fLocalMaximum> rfMaximum )
            rfMaximum = fLocalMaximum;
    }
    if(::rtl::math::isInf(rfMinimum))
        ::rtl::math::setNan(&rfMinimum);
    if(::rtl::math::isInf(rfMaximum))
        ::rtl::math::setNan(&rfMaximum);
}

void VSeriesPlotter::getMinimumAndMaximiumYInContinuousXRange( double& rfMinY, double& rfMaxY, double fMinX, double fMaxX ) const
{
    ::rtl::math::setInf(&rfMinY, false);
    ::rtl::math::setInf(&rfMaxY, true);

    ::std::vector< VDataSeriesGroup >::const_iterator aXSlotIter = m_aXSlots.begin();
    const ::std::vector< VDataSeriesGroup >::const_iterator aXSlotEnd = m_aXSlots.end();
    for( ; aXSlotIter != aXSlotEnd; aXSlotIter++ )
    {
        double fLocalMinimum, fLocalMaximum;
        aXSlotIter->getMinimumAndMaximiumYInContinuousXRange( fLocalMinimum, fLocalMaximum, fMinX, fMaxX );
        if( !::rtl::math::isNan(fLocalMinimum) && fLocalMinimum< rfMinY )
            rfMinY = fLocalMinimum;
        if( !::rtl::math::isNan(fLocalMaximum) && fLocalMaximum> rfMaxY )
            rfMaxY = fLocalMaximum;
    }
    if(::rtl::math::isInf(rfMinY))
        ::rtl::math::setNan(&rfMinY);
    if(::rtl::math::isInf(rfMaxY))
        ::rtl::math::setNan(&rfMaxY);
}

//static
sal_Int32 VSeriesPlotter::getPointCount( const ::std::vector< VDataSeriesGroup >& rSlots )
{
    sal_Int32 nRet = 0;

    ::std::vector< VDataSeriesGroup >::const_iterator       aXSlotIter = rSlots.begin();
    const ::std::vector< VDataSeriesGroup >::const_iterator aXSlotEnd = rSlots.end();

    for( ; aXSlotIter != aXSlotEnd; aXSlotIter++ )
    {
        sal_Int32 nPointCount = getPointCount( *aXSlotIter );
        if( nPointCount>nRet )
            nRet = nPointCount;
    }
    return nRet;
}

//static
sal_Int32 VSeriesPlotter::getPointCount( const VDataSeriesGroup& rSeriesGroup )
{
    sal_Int32 nRet = 0;

    const ::std::vector< VDataSeries* >* pSeriesList = &(rSeriesGroup.m_aSeriesVector);

    ::std::vector< VDataSeries* >::const_iterator       aSeriesIter = pSeriesList->begin();
    const ::std::vector< VDataSeries* >::const_iterator aSeriesEnd = pSeriesList->end();

    for( ; aSeriesIter != aSeriesEnd; aSeriesIter++ )
    {
        sal_Int32 nPointCount = (*aSeriesIter)->getTotalPointCount();
        if( nPointCount>nRet )
            nRet = nPointCount;
    }
    return nRet;
}

void VDataSeriesGroup::getMinimumAndMaximiumX( double& rfMinimum, double& rfMaximum ) const
{
    const ::std::vector< VDataSeries* >* pSeriesList = &this->m_aSeriesVector;

    ::std::vector< VDataSeries* >::const_iterator       aSeriesIter = pSeriesList->begin();
    const ::std::vector< VDataSeries* >::const_iterator aSeriesEnd  = pSeriesList->end();

    ::rtl::math::setInf(&rfMinimum, false);
    ::rtl::math::setInf(&rfMaximum, true);

    for( ; aSeriesIter != aSeriesEnd; aSeriesIter++ )
    {
        sal_Int32 nPointCount = (*aSeriesIter)->getTotalPointCount();
        for(sal_Int32 nN=0;nN<nPointCount;nN++)
        {
            double fX = (*aSeriesIter)->getX( nN );
            if( ::rtl::math::isNan(fX) )
                continue;
            if(rfMaximum<fX)
                rfMaximum=fX;
            if(rfMinimum>fX)
                rfMinimum=fX;
        }
    }
    if(::rtl::math::isInf(rfMinimum))
        ::rtl::math::setNan(&rfMinimum);
    if(::rtl::math::isInf(rfMaximum))
        ::rtl::math::setNan(&rfMaximum);
}
void VDataSeriesGroup::getMinimumAndMaximiumYInContinuousXRange( double& rfMinY, double& rfMaxY, double fMinX, double fMaxX ) const
{
    const ::std::vector< VDataSeries* >* pSeriesList = &this->m_aSeriesVector;

    ::std::vector< VDataSeries* >::const_iterator       aSeriesIter = pSeriesList->begin();
    const ::std::vector< VDataSeries* >::const_iterator aSeriesEnd  = pSeriesList->end();

    ::rtl::math::setInf(&rfMinY, false);
    ::rtl::math::setInf(&rfMaxY, true);

    for( ; aSeriesIter != aSeriesEnd; aSeriesIter++ )
    {
        sal_Int32 nPointCount = (*aSeriesIter)->getTotalPointCount();
        for(sal_Int32 nN=0;nN<nPointCount;nN++)
        {
            double fX = (*aSeriesIter)->getX( nN );
            if( ::rtl::math::isNan(fX) )
                continue;
            if( fX < fMinX || fX > fMaxX )
                continue;
            double fY = (*aSeriesIter)->getY( nN );
            if( ::rtl::math::isNan(fY) )
                continue;
            if(rfMaxY<fY)
                rfMaxY=fY;
            if(rfMinY>fY)
                rfMinY=fY;
        }
    }
    if(::rtl::math::isInf(rfMinY))
        ::rtl::math::setNan(&rfMinY);
    if(::rtl::math::isInf(rfMaxY))
        ::rtl::math::setNan(&rfMaxY);
}

void VDataSeriesGroup::calculateYSumsForCategory( sal_Int32 nCategoryIndex
        , double& rfPositiveSum, double& rfNegativeSum )
{
    //@todo maybe cach these values

    ::std::vector< VDataSeries* >* pSeriesList = &this->m_aSeriesVector;

    ::std::vector< VDataSeries* >::const_iterator       aSeriesIter = pSeriesList->begin();
    const ::std::vector< VDataSeries* >::const_iterator aSeriesEnd  = pSeriesList->end();

    double fLogicPositiveYSum = 0.0;
    double fLogicNegativeYSum = 0.0;
    for( ; aSeriesIter != aSeriesEnd; aSeriesIter++ )
    {
        double fY = (*aSeriesIter)->getY( nCategoryIndex );
        if( ::rtl::math::isNan(fY) )
            continue;
        if(fY>=0)
            fLogicPositiveYSum+=fY;
        else
            fLogicNegativeYSum+=fY;
    }

    rfPositiveSum = fLogicPositiveYSum;
    rfNegativeSum = fLogicNegativeYSum;
}

bool VDataSeriesGroup::calculateYMinAndMaxForCategoryRange(
        sal_Int32 nStartCategoryIndex, sal_Int32 nEndCategoryIndex
        , double& rfMinimum, double& rfMaximum )
{
    //return true if valid values were found otherwise false

    //@todo maybe cache these values
    ::rtl::math::setInf(&rfMinimum, false);
    ::rtl::math::setInf(&rfMaximum, true);

    double fPositiveValue=0.0, fNegativeValue=0.0;
    //iterate through the given categories
    if(nStartCategoryIndex<0)
        nStartCategoryIndex=0;
    if(nEndCategoryIndex<0)
        nEndCategoryIndex=0;
    for( sal_Int32 nCatIndex = nStartCategoryIndex; nCatIndex < nEndCategoryIndex; nCatIndex++ )
    {
        this->calculateYSumsForCategory( nCatIndex, fPositiveValue, fNegativeValue );
        if(rfMaximum<fPositiveValue)
            rfMaximum=fPositiveValue;
        if(rfMinimum>fNegativeValue)
            rfMinimum=fNegativeValue;
    }
    return !::rtl::math::isInf( rfMinimum ) && !::rtl::math::isInf( rfMaximum );
}

//static
VSeriesPlotter* VSeriesPlotter::createSeriesPlotter( const uno::Reference<XChartType>& xChartTypeModel )
{
    rtl::OUString aChartType = xChartTypeModel->getChartType();

    //@todo: in future the plotter should be instanciated via service factory
    VSeriesPlotter* pRet=NULL;
    if( aChartType.equalsIgnoreAsciiCase(C2U("com.sun.star.chart2.BarChart")) )
        pRet = new BarChart(xChartTypeModel);
    else if( aChartType.equalsIgnoreAsciiCase(C2U("com.sun.star.chart2.AreaChart")) )
        pRet = new AreaChart(xChartTypeModel,true);
    else if( aChartType.equalsIgnoreAsciiCase(C2U("com.sun.star.chart2.LineChart")) )
        pRet = new AreaChart(xChartTypeModel,true,true);
    else if( aChartType.equalsIgnoreAsciiCase(C2U("com.sun.star.chart2.ScatterChart")) )
        pRet = new AreaChart(xChartTypeModel,false,true);
    else if( aChartType.equalsIgnoreAsciiCase(C2U("com.sun.star.chart2.PieChart")) )
        pRet = new PieChart(xChartTypeModel);
    else if( aChartType.equalsIgnoreAsciiCase(C2U("com.sun.star.chart2.NetChart")) )
        pRet = new AreaChart(xChartTypeModel,true,true,new PolarPlottingPositionHelper(false) );
    else
    {
        //@todo create other charttypes
        //com.sun.star.chart2.NetChart?
        //com.sun.star.chart2.ScatterChart?
        pRet = new BarChart(xChartTypeModel);
    }
    return pRet;
}

//.............................................................................
} //namespace chart
//.............................................................................
