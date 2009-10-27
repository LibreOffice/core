/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: CandleStickChart.cxx,v $
 * $Revision: 1.6 $
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

#include "CandleStickChart.hxx"
#include "ShapeFactory.hxx"
//#include "chartview/servicenames_charttypes.hxx"
//#include "servicenames_coosystems.hxx"
#include "CommonConverters.hxx"
#include "ObjectIdentifier.hxx"
#include "LabelPositionHelper.hxx"
#include "BarPositionHelper.hxx"
#include "macros.hxx"
#include "VLegendSymbolFactory.hxx"
#include "FormattedStringHelper.hxx"
#include "DataSeriesHelper.hxx"
#include <tools/debug.hxx>
#include <rtl/math.hxx>
#include <svx/unoprnms.hxx>

//.............................................................................
namespace chart
{
//.............................................................................
using namespace ::com::sun::star;
using namespace ::rtl::math;
using namespace ::com::sun::star::chart2;
using ::com::sun::star::uno::Reference;
using ::rtl::OUString;

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

CandleStickChart::CandleStickChart( const uno::Reference<XChartType>& xChartTypeModel
                                    , sal_Int32 nDimensionCount )
        : VSeriesPlotter( xChartTypeModel, nDimensionCount )
        , m_pMainPosHelper( new BarPositionHelper() )
{
    PlotterBase::m_pPosHelper = m_pMainPosHelper;
    VSeriesPlotter::m_pMainPosHelper = m_pMainPosHelper;
}

CandleStickChart::~CandleStickChart()
{
    delete m_pMainPosHelper;
}

//-------------------------------------------------------------------------
// MinimumAndMaximumSupplier
//-------------------------------------------------------------------------

double CandleStickChart::getMinimumX()
{
    if( m_bCategoryXAxis )
        return 0.5;//first category (index 0) matches with real number 1.0
    return VSeriesPlotter::getMinimumX();
}
double CandleStickChart::getMaximumX()
{
    if( m_bCategoryXAxis )
    {
        //return category count
        sal_Int32 nPointCount = getPointCount();
        return nPointCount+0.5;//first category (index 0) matches with real number 1.0
    }
    return VSeriesPlotter::getMaximumX();
}
bool CandleStickChart::isSeperateStackingForDifferentSigns( sal_Int32 /* nDimensionIndex */ )
{
    return false;
}

//-----------------------------------------------------------------
//-----------------------------------------------------------------
//-----------------------------------------------------------------

LegendSymbolStyle CandleStickChart::getLegendSymbolStyle()
{
    return chart2::LegendSymbolStyle_VERTICAL_LINE;
}

//-----------------------------------------------------------------
// lang::XServiceInfo
//-----------------------------------------------------------------
/*
APPHELPER_XSERVICEINFO_IMPL(CandleStickChart,CHART2_VIEW_CANDLESTICKCHART_SERVICE_IMPLEMENTATION_NAME)

    uno::Sequence< rtl::OUString > CandleStickChart
::getSupportedServiceNames_Static()
{
    uno::Sequence< rtl::OUString > aSNS( 1 );
    aSNS.getArray()[ 0 ] = CHART2_VIEW_CANDLESTICKCHART_SERVICE_NAME;
    return aSNS;
}
*/
/*
//-----------------------------------------------------------------
// chart2::XPlotter
//-----------------------------------------------------------------

    ::rtl::OUString SAL_CALL CandleStickChart
::getCoordinateSystemTypeID()
    throw (uno::RuntimeException)
{
    return CHART2_COOSYSTEM_CARTESIAN_SERVICE_NAME;
}
*/

drawing::Direction3D CandleStickChart::getPreferredDiagramAspectRatio() const
{
    return drawing::Direction3D(-1,-1,-1);
}

void CandleStickChart::addSeries( VDataSeries* pSeries, sal_Int32 /* zSlot */, sal_Int32 xSlot, sal_Int32 ySlot )
{
    //ignore y stacking for candle stick chart
    VSeriesPlotter::addSeries( pSeries, 0, xSlot, ySlot );
}

void CandleStickChart::createShapes()
{
    if( m_aZSlots.begin() == m_aZSlots.end() ) //no series
        return;

    if( m_nDimension!=2 )
        return;

    DBG_ASSERT(m_pShapeFactory&&m_xLogicTarget.is()&&m_xFinalTarget.is(),"CandleStickChart is not proper initialized");
    if(!(m_pShapeFactory&&m_xLogicTarget.is()&&m_xFinalTarget.is()))
        return;

    //the text labels should be always on top of the other series shapes
    //therefore create an own group for the texts to move them to front
    //(because the text group is created after the series group the texts are displayed on top)

    uno::Reference< drawing::XShapes > xSeriesTarget(
        createGroupShape( m_xLogicTarget,rtl::OUString() ));
    uno::Reference< drawing::XShapes > xLossTarget(
        createGroupShape( m_xLogicTarget, ObjectIdentifier::createClassifiedIdentifier(
            OBJECTTYPE_DATA_STOCK_LOSS, rtl::OUString() )));
    uno::Reference< drawing::XShapes > xGainTarget(
        createGroupShape( m_xLogicTarget, ObjectIdentifier::createClassifiedIdentifier(
            OBJECTTYPE_DATA_STOCK_GAIN, rtl::OUString() )));
    uno::Reference< drawing::XShapes > xTextTarget(
        m_pShapeFactory->createGroup2D( m_xFinalTarget,rtl::OUString() ));

    //---------------------------------------------
    //check necessary here that different Y axis can not be stacked in the same group? ... hm?

    bool bJapaneseStyle=true;//@todo is this the correct default?
    bool bShowFirst = true;//is only important if bJapaneseStyle == false
    tNameSequence aWhiteBox_Names, aBlackBox_Names;
    tAnySequence  aWhiteBox_Values, aBlackBox_Values;
    try
    {
        if( m_xChartTypeModelProps.is() )
        {
            m_xChartTypeModelProps->getPropertyValue( C2U( "ShowFirst" ) ) >>= bShowFirst;

            uno::Reference< beans::XPropertySet > xWhiteDayProps(0);
            uno::Reference< beans::XPropertySet > xBlackDayProps(0);
            m_xChartTypeModelProps->getPropertyValue( C2U( "Japanese" ) ) >>= bJapaneseStyle;
            m_xChartTypeModelProps->getPropertyValue( C2U( "WhiteDay" ) ) >>= xWhiteDayProps;
            m_xChartTypeModelProps->getPropertyValue( C2U( "BlackDay" ) ) >>= xBlackDayProps;

            tPropertyNameValueMap aWhiteBox_Map;
            PropertyMapper::getValueMap( aWhiteBox_Map, PropertyMapper::getPropertyNameMapForFillAndLineProperties(), xWhiteDayProps );
            PropertyMapper::getMultiPropertyListsFromValueMap( aWhiteBox_Names, aWhiteBox_Values, aWhiteBox_Map );

            tPropertyNameValueMap aBlackBox_Map;
            PropertyMapper::getValueMap( aBlackBox_Map, PropertyMapper::getPropertyNameMapForFillAndLineProperties(), xBlackDayProps );
            PropertyMapper::getMultiPropertyListsFromValueMap( aBlackBox_Names, aBlackBox_Values, aBlackBox_Map );
        }
    }
    catch( uno::Exception& e )
    {
        ASSERT_EXCEPTION( e );
    }

    //(@todo maybe different iteration for breaks in axis ?)
    sal_Int32 nStartCategoryIndex = m_pMainPosHelper->getStartCategoryIndex(); // inclusive
    sal_Int32 nEndCategoryIndex   = m_pMainPosHelper->getEndCategoryIndex(); //inclusive
//=============================================================================
    //iterate through all shown categories
    for( sal_Int32 nIndex = nStartCategoryIndex; nIndex < nEndCategoryIndex; nIndex++ )
    {
        ::std::vector< ::std::vector< VDataSeriesGroup > >::iterator             aZSlotIter = m_aZSlots.begin();
        const ::std::vector< ::std::vector< VDataSeriesGroup > >::const_iterator  aZSlotEnd = m_aZSlots.end();
//=============================================================================
        for( sal_Int32 nZ=0; aZSlotIter != aZSlotEnd; aZSlotIter++, nZ++ )
        {
            ::std::vector< VDataSeriesGroup >::iterator             aXSlotIter = aZSlotIter->begin();
            const ::std::vector< VDataSeriesGroup >::const_iterator aXSlotEnd = aZSlotIter->end();

            sal_Int32 nAttachedAxisIndex = 0;
            BarPositionHelper* pPosHelper = m_pMainPosHelper;
            if( aXSlotIter != aXSlotEnd )
            {
                nAttachedAxisIndex = aXSlotIter->getAttachedAxisIndexForFirstSeries();
                //2ND_AXIS_IN_BARS so far one can assume to have the same plotter for each z slot
                pPosHelper = dynamic_cast<BarPositionHelper*>(&( this->getPlottingPositionHelper( nAttachedAxisIndex ) ) );
                if(!pPosHelper)
                    pPosHelper = m_pMainPosHelper;
            }
            PlotterBase::m_pPosHelper = pPosHelper;

            //update/create information for current group
            pPosHelper->updateSeriesCount( aZSlotIter->size() );
//=============================================================================
            //iterate through all x slots in this category
            for( double fSlotX=0; aXSlotIter != aXSlotEnd; aXSlotIter++, fSlotX+=1.0 )
            {
                ::std::vector< VDataSeries* >* pSeriesList = &(aXSlotIter->m_aSeriesVector);

                ::std::vector< VDataSeries* >::const_iterator       aSeriesIter = pSeriesList->begin();
                const ::std::vector< VDataSeries* >::const_iterator aSeriesEnd  = pSeriesList->end();
                aSeriesIter = pSeriesList->begin();
    //=============================================================================
                //iterate through all series in this x slot
                for( ; aSeriesIter != aSeriesEnd; aSeriesIter++ )
                {
                    //collect data point information (logic coordinates, style ):
                    double fLogicX = pPosHelper->getSlotPos( (*aSeriesIter)->getXValue( nIndex ), fSlotX );
                    double fY_First = (*aSeriesIter)->getY_First( nIndex );
                    double fY_Last = (*aSeriesIter)->getY_Last( nIndex );
                    double fY_Min = (*aSeriesIter)->getY_Min( nIndex );
                    double fY_Max = (*aSeriesIter)->getY_Max( nIndex );

                    bool bBlack=false;
                    if(fY_Last<=fY_First)
                    {
                        std::swap(fY_First,fY_Last);
                        bBlack=true;
                    }
                    if(fY_Max<fY_Min)
                        std::swap(fY_Min,fY_Max);
                    //transformation 3) -> 4)
                    double fHalfWidth = pPosHelper->getSlotWidth()/2.0;
                    drawing::Position3D aPosLeftFirst( pPosHelper->transformLogicToScene( fLogicX-fHalfWidth, fY_First ,0 ,true ) );
                    drawing::Position3D aPosRightLast( pPosHelper->transformLogicToScene( fLogicX+fHalfWidth, fY_Last  ,0 ,true ) );
                    drawing::Position3D aPosMiddleFirst( pPosHelper->transformLogicToScene( fLogicX, fY_First ,0 ,true ) );
                    drawing::Position3D aPosMiddleLast( pPosHelper->transformLogicToScene( fLogicX, fY_Last  ,0 ,true ) );
                    drawing::Position3D aPosMiddleMinimum( pPosHelper->transformLogicToScene( fLogicX, fY_Min ,0 ,true ) );
                    drawing::Position3D aPosMiddleMaximum( pPosHelper->transformLogicToScene( fLogicX, fY_Max ,0 ,true ) );

                    uno::Reference< drawing::XShapes > xLossGainTarget( xGainTarget );
                    if(bBlack)
                        xLossGainTarget = xLossTarget;

                    uno::Reference< beans::XPropertySet > xPointProp( (*aSeriesIter)->getPropertiesOfPoint( nIndex ));
                    uno::Reference< drawing::XShapes > xPointGroupShape_Shapes(0);
                    {
                        rtl::OUString aPointCID = ObjectIdentifier::createPointCID( (*aSeriesIter)->getPointCID_Stub(), nIndex );
                        uno::Reference< drawing::XShapes > xSeriesGroupShape_Shapes( getSeriesGroupShape(*aSeriesIter, xSeriesTarget) );
                        xPointGroupShape_Shapes = createGroupShape(xSeriesGroupShape_Shapes,aPointCID);
                    }

                    //create min-max line
                    if( isValidPosition(aPosMiddleMinimum) && isValidPosition(aPosMiddleMaximum) )
                    {
                        uno::Reference< drawing::XShape > xShape( m_xShapeFactory->createInstance( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(
                                "com.sun.star.drawing.PolyLineShape" ) ) ), uno::UNO_QUERY );
                        xPointGroupShape_Shapes->add(xShape);
                        uno::Reference< beans::XPropertySet > xProp( xShape, uno::UNO_QUERY );
                        if(xProp.is())
                        {
                            drawing::PolyPolygonShape3D aPoly;
                            sal_Int32 nLineIndex =0;
                            AddPointToPoly( aPoly, aPosMiddleMinimum, nLineIndex);
                            AddPointToPoly( aPoly, aPosMiddleMaximum, nLineIndex);
                            xProp->setPropertyValue( C2U( UNO_NAME_POLYPOLYGON ), uno::makeAny( PolyToPointSequence(aPoly) ) );
                        }
                        this->setMappedProperties( xShape, xPointProp, PropertyMapper::getPropertyNameMapForLineSeriesProperties() );
                    }

                    //create first-last shape
                    if(bJapaneseStyle && isValidPosition(aPosLeftFirst) && isValidPosition(aPosRightLast) )
                    {
                        uno::Reference< drawing::XShape > xShape( m_xShapeFactory->createInstance( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(
                                "com.sun.star.drawing.RectangleShape" ) ) ), uno::UNO_QUERY );
                        xLossGainTarget->add(xShape);

                        xShape->setPosition( Position3DToAWTPoint( aPosLeftFirst ) );
                        drawing::Direction3D aDiff = aPosRightLast-aPosLeftFirst;
                        awt::Size aAWTSize( Direction3DToAWTSize( aDiff ));
                        // workaround for bug in drawing: if height is 0 the box gets infinitely large
                        if( aAWTSize.Height == 0 )
                            aAWTSize.Height = 1;
                        xShape->setSize( aAWTSize );

                        uno::Reference< beans::XPropertySet > xProp( xShape, uno::UNO_QUERY );
                        if(xProp.is())
                        {
                            if(bBlack)
                                PropertyMapper::setMultiProperties( aBlackBox_Names, aBlackBox_Values, xProp );
                            else
                                PropertyMapper::setMultiProperties( aWhiteBox_Names, aWhiteBox_Values, xProp );
                        }
                    }
                    else
                    {
                        drawing::PolyPolygonShape3D aPoly;

                        sal_Int32 nLineIndex = 0;
                        if( bShowFirst &&  pPosHelper->isLogicVisible( fLogicX, fY_First ,0 )
                            && isValidPosition(aPosLeftFirst) && isValidPosition(aPosMiddleFirst) )
                        {
                            AddPointToPoly( aPoly, aPosLeftFirst, nLineIndex );
                            AddPointToPoly( aPoly, aPosMiddleFirst, nLineIndex++ );
                        }
                        if( pPosHelper->isLogicVisible( fLogicX, fY_Last ,0 )
                            && isValidPosition(aPosMiddleLast) && isValidPosition(aPosRightLast) )
                        {
                            AddPointToPoly( aPoly, aPosMiddleLast, nLineIndex );
                            AddPointToPoly( aPoly, aPosRightLast, nLineIndex );
                        }

                        if( aPoly.SequenceX.getLength() )
                        {
                            uno::Reference< drawing::XShape > xShape( m_xShapeFactory->createInstance( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(
                                "com.sun.star.drawing.PolyLineShape" ) ) ), uno::UNO_QUERY );
                            xPointGroupShape_Shapes->add(xShape);
                            uno::Reference< beans::XPropertySet > xProp( xShape, uno::UNO_QUERY );
                            if(xProp.is())
                            {
                                xProp->setPropertyValue( C2U( UNO_NAME_POLYPOLYGON ), uno::makeAny( PolyToPointSequence(aPoly) ) );
                                this->setMappedProperties( xShape, xPointProp, PropertyMapper::getPropertyNameMapForLineSeriesProperties() );
                            }
                        }
                    }

                    //create data point label
                    if( (**aSeriesIter).getDataPointLabelIfLabel(nIndex) )
                    {
                        if(isValidPosition(aPosMiddleFirst))
                            this->createDataLabel( xTextTarget, **aSeriesIter, nIndex
                                        , fY_First, 1.0, Position3DToAWTPoint(aPosMiddleFirst), LABEL_ALIGN_LEFT_BOTTOM );
                        if(isValidPosition(aPosMiddleLast))
                            this->createDataLabel( xTextTarget, **aSeriesIter, nIndex
                                        , fY_Last, 1.0, Position3DToAWTPoint(aPosMiddleLast), LABEL_ALIGN_RIGHT_TOP );
                        if(isValidPosition(aPosMiddleMinimum))
                            this->createDataLabel( xTextTarget, **aSeriesIter, nIndex
                                        , fY_Min, 1.0, Position3DToAWTPoint(aPosMiddleMinimum), LABEL_ALIGN_BOTTOM );
                        if(isValidPosition(aPosMiddleMaximum))
                            this->createDataLabel( xTextTarget, **aSeriesIter, nIndex
                                        , fY_Max, 1.0, Position3DToAWTPoint(aPosMiddleMaximum), LABEL_ALIGN_TOP );
                    }
                }//next series in x slot (next y slot)
            }//next x slot
        }//next z slot
    }//next category
//=============================================================================
//=============================================================================
//=============================================================================
    /* @todo remove series shapes if empty
    //remove and delete point-group-shape if empty
    if(!xSeriesGroupShape_Shapes->getCount())
    {
        (*aSeriesIter)->m_xShape.set(NULL);
        m_xLogicTarget->remove(xSeriesGroupShape_Shape);
    }
    */

    //remove and delete series-group-shape if empty

    //... todo
}

//.............................................................................
} //namespace chart
//.............................................................................
