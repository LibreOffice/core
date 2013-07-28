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

#include "CandleStickChart.hxx"
#include "ShapeFactory.hxx"
#include "CommonConverters.hxx"
#include "ObjectIdentifier.hxx"
#include "LabelPositionHelper.hxx"
#include "BarPositionHelper.hxx"
#include "macros.hxx"
#include "VLegendSymbolFactory.hxx"
#include "FormattedStringHelper.hxx"
#include "DataSeriesHelper.hxx"
#include "DateHelper.hxx"
#include <rtl/math.hxx>
#include <editeng/unoprnms.hxx>

namespace chart
{
using namespace ::com::sun::star;
using namespace ::rtl::math;
using namespace ::com::sun::star::chart2;
using ::com::sun::star::uno::Reference;

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

// MinimumAndMaximumSupplier

bool CandleStickChart::isSeparateStackingForDifferentSigns( sal_Int32 /* nDimensionIndex */ )
{
    return false;
}

LegendSymbolStyle CandleStickChart::getLegendSymbolStyle()
{
    return LegendSymbolStyle_LINE;
}

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

    OSL_ENSURE(m_pShapeFactory&&m_xLogicTarget.is()&&m_xFinalTarget.is(),"CandleStickChart is not proper initialized");
    if(!(m_pShapeFactory&&m_xLogicTarget.is()&&m_xFinalTarget.is()))
        return;

    //the text labels should be always on top of the other series shapes
    //therefore create an own group for the texts to move them to front
    //(because the text group is created after the series group the texts are displayed on top)

    uno::Reference< drawing::XShapes > xSeriesTarget(
        createGroupShape( m_xLogicTarget,OUString() ));
    uno::Reference< drawing::XShapes > xLossTarget(
        createGroupShape( m_xLogicTarget, ObjectIdentifier::createClassifiedIdentifier(
            OBJECTTYPE_DATA_STOCK_LOSS, OUString() )));
    uno::Reference< drawing::XShapes > xGainTarget(
        createGroupShape( m_xLogicTarget, ObjectIdentifier::createClassifiedIdentifier(
            OBJECTTYPE_DATA_STOCK_GAIN, OUString() )));
    uno::Reference< drawing::XShapes > xTextTarget(
        m_pShapeFactory->createGroup2D( m_xFinalTarget,OUString() ));

    //check necessary here that different Y axis can not be stacked in the same group? ... hm?

    bool bJapaneseStyle=true;//@todo is this the correct default?
    bool bShowFirst = true;//is only important if bJapaneseStyle == false
    tNameSequence aWhiteBox_Names, aBlackBox_Names;
    tAnySequence  aWhiteBox_Values, aBlackBox_Values;
    try
    {
        if( m_xChartTypeModelProps.is() )
        {
            m_xChartTypeModelProps->getPropertyValue( "ShowFirst" ) >>= bShowFirst;

            uno::Reference< beans::XPropertySet > xWhiteDayProps(0);
            uno::Reference< beans::XPropertySet > xBlackDayProps(0);
            m_xChartTypeModelProps->getPropertyValue( "Japanese" ) >>= bJapaneseStyle;
            m_xChartTypeModelProps->getPropertyValue( "WhiteDay" ) >>= xWhiteDayProps;
            m_xChartTypeModelProps->getPropertyValue( "BlackDay" ) >>= xBlackDayProps;

            tPropertyNameValueMap aWhiteBox_Map;
            PropertyMapper::getValueMap( aWhiteBox_Map, PropertyMapper::getPropertyNameMapForFillAndLineProperties(), xWhiteDayProps );
            PropertyMapper::getMultiPropertyListsFromValueMap( aWhiteBox_Names, aWhiteBox_Values, aWhiteBox_Map );

            tPropertyNameValueMap aBlackBox_Map;
            PropertyMapper::getValueMap( aBlackBox_Map, PropertyMapper::getPropertyNameMapForFillAndLineProperties(), xBlackDayProps );
            PropertyMapper::getMultiPropertyListsFromValueMap( aBlackBox_Names, aBlackBox_Values, aBlackBox_Map );
        }
    }
    catch( const uno::Exception& e )
    {
        ASSERT_EXCEPTION( e );
    }

    //(@todo maybe different iteration for breaks in axis ?)
    sal_Int32 nStartIndex = 0;
    sal_Int32 nEndIndex = VSeriesPlotter::getPointCount();
    double fLogicZ = 1.5;//as defined
    //iterate through all x values per indices
    for( sal_Int32 nIndex = nStartIndex; nIndex < nEndIndex; nIndex++ )
    {
        ::std::vector< ::std::vector< VDataSeriesGroup > >::iterator             aZSlotIter = m_aZSlots.begin();
        const ::std::vector< ::std::vector< VDataSeriesGroup > >::const_iterator  aZSlotEnd = m_aZSlots.end();
        for( sal_Int32 nZ=0; aZSlotIter != aZSlotEnd; ++aZSlotIter, nZ++ )
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
            //iterate through all x slots in this category
            for( double fSlotX=0; aXSlotIter != aXSlotEnd; ++aXSlotIter, fSlotX+=1.0 )
            {
                ::std::vector< VDataSeries* >* pSeriesList = &(aXSlotIter->m_aSeriesVector);

                ::std::vector< VDataSeries* >::const_iterator       aSeriesIter = pSeriesList->begin();
                const ::std::vector< VDataSeries* >::const_iterator aSeriesEnd  = pSeriesList->end();
                //iterate through all series in this x slot
                for( ; aSeriesIter != aSeriesEnd; ++aSeriesIter )
                {
                    //collect data point information (logic coordinates, style ):
                    double fUnscaledX = (*aSeriesIter)->getXValue( nIndex );
                    if( m_pExplicitCategoriesProvider && m_pExplicitCategoriesProvider->isDateAxis() )
                        fUnscaledX = DateHelper::RasterizeDateValue( fUnscaledX, m_aNullDate, m_nTimeResolution );
                    if(fUnscaledX<pPosHelper->getLogicMinX() || fUnscaledX>pPosHelper->getLogicMaxX())
                        continue;//point not visible
                    double fScaledX = pPosHelper->getScaledSlotPos( fUnscaledX, fSlotX );

                    double fUnscaledY_First = (*aSeriesIter)->getY_First( nIndex );
                    double fUnscaledY_Last = (*aSeriesIter)->getY_Last( nIndex );
                    double fUnscaledY_Min = (*aSeriesIter)->getY_Min( nIndex );
                    double fUnscaledY_Max = (*aSeriesIter)->getY_Max( nIndex );

                    bool bBlack=false;
                    if(fUnscaledY_Last<=fUnscaledY_First)
                    {
                        std::swap(fUnscaledY_First,fUnscaledY_Last);
                        bBlack=true;
                    }
                    if(fUnscaledY_Max<fUnscaledY_Min)
                        std::swap(fUnscaledY_Min,fUnscaledY_Max);
                    //transformation 3) -> 4)
                    double fHalfScaledWidth = pPosHelper->getScaledSlotWidth()/2.0;

                    double fScaledY_First(fUnscaledY_First);
                    double fScaledY_Last(fUnscaledY_Last);
                    double fScaledY_Min(fUnscaledY_Min);
                    double fScaledY_Max(fUnscaledY_Max);
                    pPosHelper->clipLogicValues( 0,&fScaledY_First,0 );
                    pPosHelper->clipLogicValues( 0,&fScaledY_Last,0 );
                    pPosHelper->clipLogicValues( 0,&fScaledY_Min,0 );
                    pPosHelper->clipLogicValues( 0,&fScaledY_Max,0 );
                    pPosHelper->doLogicScaling( 0,&fScaledY_First,0 );
                    pPosHelper->doLogicScaling( 0,&fScaledY_Last,0 );
                    pPosHelper->doLogicScaling( 0,&fScaledY_Min,0 );
                    pPosHelper->doLogicScaling( 0,&fScaledY_Max,0 );

                    drawing::Position3D aPosLeftFirst( pPosHelper->transformScaledLogicToScene( fScaledX-fHalfScaledWidth, fScaledY_First ,0 ,true ) );
                    drawing::Position3D aPosRightLast( pPosHelper->transformScaledLogicToScene( fScaledX+fHalfScaledWidth, fScaledY_Last  ,0 ,true ) );
                    drawing::Position3D aPosMiddleFirst( pPosHelper->transformScaledLogicToScene( fScaledX, fScaledY_First ,0 ,true ) );
                    drawing::Position3D aPosMiddleLast( pPosHelper->transformScaledLogicToScene( fScaledX, fScaledY_Last  ,0 ,true ) );
                    drawing::Position3D aPosMiddleMinimum( pPosHelper->transformScaledLogicToScene( fScaledX, fScaledY_Min ,0 ,true ) );
                    drawing::Position3D aPosMiddleMaximum( pPosHelper->transformScaledLogicToScene( fScaledX, fScaledY_Max ,0 ,true ) );

                    uno::Reference< drawing::XShapes > xLossGainTarget( xGainTarget );
                    if(bBlack)
                        xLossGainTarget = xLossTarget;

                    uno::Reference< beans::XPropertySet > xPointProp( (*aSeriesIter)->getPropertiesOfPoint( nIndex ));
                    uno::Reference< drawing::XShapes > xPointGroupShape_Shapes(0);
                    {
                        OUString aPointCID = ObjectIdentifier::createPointCID( (*aSeriesIter)->getPointCID_Stub(), nIndex );
                        uno::Reference< drawing::XShapes > xSeriesGroupShape_Shapes( getSeriesGroupShape(*aSeriesIter, xSeriesTarget) );
                        xPointGroupShape_Shapes = createGroupShape(xSeriesGroupShape_Shapes,aPointCID);
                    }

                    //create min-max line
                    if( isValidPosition(aPosMiddleMinimum) && isValidPosition(aPosMiddleMaximum) )
                    {
                        uno::Reference< drawing::XShape > xShape(
                            m_xShapeFactory->createInstance( "com.sun.star.drawing.PolyLineShape" ),
                            uno::UNO_QUERY );
                        xPointGroupShape_Shapes->add(xShape);
                        uno::Reference< beans::XPropertySet > xProp( xShape, uno::UNO_QUERY );
                        if(xProp.is())
                        {
                            drawing::PolyPolygonShape3D aPoly;
                            sal_Int32 nLineIndex =0;
                            AddPointToPoly( aPoly, aPosMiddleMinimum, nLineIndex);
                            AddPointToPoly( aPoly, aPosMiddleMaximum, nLineIndex);
                            xProp->setPropertyValue( UNO_NAME_POLYPOLYGON, uno::makeAny( PolyToPointSequence(aPoly) ) );
                        }
                        this->setMappedProperties( xShape, xPointProp, PropertyMapper::getPropertyNameMapForLineSeriesProperties() );
                    }

                    //create first-last shape
                    if(bJapaneseStyle && isValidPosition(aPosLeftFirst) && isValidPosition(aPosRightLast) )
                    {
                        uno::Reference< drawing::XShape > xShape(
                            m_xShapeFactory->createInstance( "com.sun.star.drawing.RectangleShape" ),
                            uno::UNO_QUERY );
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
                        if( bShowFirst &&  pPosHelper->isLogicVisible( fUnscaledX, fUnscaledY_First ,fLogicZ )
                            && isValidPosition(aPosLeftFirst) && isValidPosition(aPosMiddleFirst) )
                        {
                            AddPointToPoly( aPoly, aPosLeftFirst, nLineIndex );
                            AddPointToPoly( aPoly, aPosMiddleFirst, nLineIndex++ );
                        }
                        if( pPosHelper->isLogicVisible( fUnscaledX, fUnscaledY_Last ,fLogicZ )
                            && isValidPosition(aPosMiddleLast) && isValidPosition(aPosRightLast) )
                        {
                            AddPointToPoly( aPoly, aPosMiddleLast, nLineIndex );
                            AddPointToPoly( aPoly, aPosRightLast, nLineIndex );
                        }

                        if( aPoly.SequenceX.getLength() )
                        {
                            uno::Reference< drawing::XShape > xShape(
                                m_xShapeFactory->createInstance( "com.sun.star.drawing.PolyLineShape" ),
                                uno::UNO_QUERY );
                            xPointGroupShape_Shapes->add(xShape);
                            uno::Reference< beans::XPropertySet > xProp( xShape, uno::UNO_QUERY );
                            if(xProp.is())
                            {
                                xProp->setPropertyValue( UNO_NAME_POLYPOLYGON, uno::makeAny( PolyToPointSequence(aPoly) ) );
                                this->setMappedProperties( xShape, xPointProp, PropertyMapper::getPropertyNameMapForLineSeriesProperties() );
                            }
                        }
                    }

                    //create data point label
                    if( (**aSeriesIter).getDataPointLabelIfLabel(nIndex) )
                    {
                        if(isValidPosition(aPosMiddleFirst))
                            this->createDataLabel( xTextTarget, **aSeriesIter, nIndex
                                        , fUnscaledY_First, 1.0, Position3DToAWTPoint(aPosMiddleFirst), LABEL_ALIGN_LEFT_BOTTOM );
                        if(isValidPosition(aPosMiddleLast))
                            this->createDataLabel( xTextTarget, **aSeriesIter, nIndex
                                        , fUnscaledY_Last, 1.0, Position3DToAWTPoint(aPosMiddleLast), LABEL_ALIGN_RIGHT_TOP );
                        if(isValidPosition(aPosMiddleMinimum))
                            this->createDataLabel( xTextTarget, **aSeriesIter, nIndex
                                        , fUnscaledY_Min, 1.0, Position3DToAWTPoint(aPosMiddleMinimum), LABEL_ALIGN_BOTTOM );
                        if(isValidPosition(aPosMiddleMaximum))
                            this->createDataLabel( xTextTarget, **aSeriesIter, nIndex
                                        , fUnscaledY_Max, 1.0, Position3DToAWTPoint(aPosMiddleMaximum), LABEL_ALIGN_TOP );
                    }
                }//next series in x slot (next y slot)
            }//next x slot
        }//next z slot
    }//next category
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

} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
