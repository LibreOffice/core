/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */
#include "sampleaddin.hxx"

#include <cppuhelper/supportsservice.hxx>
#include <cppuhelper/factory.hxx>
#include <osl/diagnose.h>

#include <com/sun/star/drawing/XDrawPageSupplier.hpp>
#include <com/sun/star/drawing/XDrawPage.hpp>
#include <com/sun/star/chart/XChartDataArray.hpp>
#include <com/sun/star/text/XTextRange.hpp>
#include <com/sun/star/chart/X3DDisplay.hpp>

using namespace com::sun::star;



extern "C" {

sal_Bool SAL_CALL component_writeInfo(
    void * /*pServiceManager*/, registry::XRegistryKey * pRegistryKey )
{
    if( pRegistryKey )
    {
        try
        {
            OUString aImpl( "/" );
            aImpl += SampleAddIn::getImplementationName_Static();
            aImpl += "/UNO/SERVICES";

            uno::Reference< registry::XRegistryKey> xNewKey(
                reinterpret_cast<registry::XRegistryKey*>( pRegistryKey )->createKey( aImpl ) );

            uno::Sequence< OUString > aSequ = SampleAddIn::getSupportedServiceNames_Static();
            const OUString * pArray = aSequ.getConstArray();
            for( sal_Int32 i = 0; i < aSequ.getLength(); i++ )
                xNewKey->createKey( pArray[i] );

            return sal_True;
        }
        catch( const registry::InvalidRegistryException& )
        {
            OSL_FAIL( "### InvalidRegistryException!" );
        }
    }
    return sal_False;
}

SAL_DLLPUBLIC_EXPORT void * SAL_CALL component_getFactory(
    const sal_Char * pImplName, void * pServiceManager, void * /*pRegistryKey*/ )
{
    void* pRet = 0;

    if ( pServiceManager &&
            OUString::createFromAscii( pImplName ) == SampleAddIn::getImplementationName_Static() )
    {
        uno::Reference< lang::XSingleServiceFactory> xFactory( cppu::createSingleFactory(
                reinterpret_cast<lang::XMultiServiceFactory*>( pServiceManager ),
                SampleAddIn::getImplementationName_Static(),
                SampleAddIn_CreateInstance,
                SampleAddIn::getSupportedServiceNames_Static() ) );

        if( xFactory.is())
        {
            xFactory->acquire();
            pRet = xFactory.get();
        }
    }

    return pRet;
}

}   



SampleAddIn::SampleAddIn()
{

}

SampleAddIn::~SampleAddIn()
{}


sal_Bool SampleAddIn::getLogicalPosition( uno::Reference< drawing::XShape >& xAxis,
                                          double fValue,
                                          sal_Bool bVertical,
                                          awt::Point& aOutPosition )
{
    sal_Bool bRet = sal_False;

    if( xAxis.is())
    {
        awt::Size aSize = xAxis->getSize();
        sal_Int32 nLength = bVertical? aSize.Height: aSize.Width;

        uno::Reference< beans::XPropertySet > xProp( xAxis, uno::UNO_QUERY );
        if( xProp.is())
        {
            try
            {
                double fMin(0.0), fMax(0.0);
                uno::Any aAny = xProp->getPropertyValue( "Min" );
                aAny >>= fMin;
                aAny = xProp->getPropertyValue( "Max" );
                aAny >>= fMax;

                double fRange = fMax - fMin;
                if( fMin <= fValue && fValue <= fMax &&
                    fRange != 0.0 )
                {
                    double fPercentage = (fValue - fMin) / fRange;
                    awt::Point aPos = xAxis->getPosition();

                    if( bVertical )
                    {
                        aOutPosition.X = aPos.X;
                        aOutPosition.Y = static_cast<sal_Int32>(aPos.Y + nLength * (1.0 - fPercentage));    
                    }
                    else
                    {
                        aOutPosition.X = static_cast<sal_Int32>(aPos.X + nLength * fPercentage);
                        aOutPosition.Y = aPos.Y;
                    }
                    bRet = sal_True;
                }
            }
            catch( const beans::UnknownPropertyException& )
            {
                
            }
        }
    }

    return bRet;
}

OUString SampleAddIn::getImplementationName_Static()
{
    return "SampleAddIn";
}

uno::Sequence< OUString > SampleAddIn::getSupportedServiceNames_Static()
{
    uno::Sequence< OUString > aSeq( 4 );

    aSeq[ 0 ] = "com.sun.star.chart.ChartAxisXSupplier";
    aSeq[ 1 ] = "com.sun.star.chart.ChartAxisYSupplier";
    aSeq[ 2 ] = "com.sun.star.chart.Diagram";
    aSeq[ 3 ] = "com.sun.star.chart.SampleAddIn";

    return aSeq;
}

uno::Reference< uno::XInterface > SAL_CALL SampleAddIn_CreateInstance(
    const uno::Reference< lang::XMultiServiceFactory >& )
{
    uno::Reference< uno::XInterface > xInst = (cppu::OWeakObject*)new SampleAddIn();

    return xInst;
}




void SAL_CALL SampleAddIn::initialize( const uno::Sequence< uno::Any >& aArguments )
    throw( uno::Exception, uno::RuntimeException )
{
    
    OSL_ENSURE( aArguments.getLength() > 0, "Please initialize Chart AddIn with ChartDocument!" );

    if( aArguments.getLength())
    {
        aArguments[ 0 ] >>= mxChartDoc;
        OSL_ENSURE( mxChartDoc.is(), "First argument in initialization is not an XChartDocument!" );

        
        uno::Reference< beans::XPropertySet > xDocProp( mxChartDoc, uno::UNO_QUERY );
        if( xDocProp.is())
        {
            uno::Any aBaseType;
            aBaseType <<= "com.sun.star.chart.XYDiagram";
            try
            {
                xDocProp->setPropertyValue( "BaseDiagram" , aBaseType );
            }
            catch( ... )
            {}
        }

        
        uno::Reference< chart::X3DDisplay > xWallSupplier( mxChartDoc->getDiagram(), uno::UNO_QUERY );
        if( xWallSupplier.is())
        {
            uno::Reference< beans::XPropertySet > xDiaProp( xWallSupplier->getWall(), uno::UNO_QUERY );
            uno::Reference< beans::XPropertySet > xLegendProp( mxChartDoc->getLegend(), uno::UNO_QUERY );
            if( xDiaProp.is() &&
                xLegendProp.is())
            {
                uno::Any aAny;
                aAny <<= (sal_Int32)( 0xe0e0f0 );
                xDiaProp->setPropertyValue( "FillColor" , aAny );
                xLegendProp->setPropertyValue( "FillColor" , aAny );
            }
        }
    }
}


/********************************************************************************
 *
 * The method refresh is the most important method - here all objects that
 * are necessary for the chart are created
 *
 * in the first implementation you will have to insert everything in this
 * routine - all old objects are deleted beforehand
 *
 ********************************************************************************/
void SAL_CALL SampleAddIn::refresh() throw( uno::RuntimeException )
{
    if( ! mxChartDoc.is())
        return;

    
    uno::Reference< drawing::XDrawPageSupplier > xPageSupp( mxChartDoc, uno::UNO_QUERY );
    uno::Reference< lang::XMultiServiceFactory > xFactory( mxChartDoc, uno::UNO_QUERY );
    if( xPageSupp.is() &&
        xFactory.is() )
    {
        uno::Reference< drawing::XDrawPage > xPage = xPageSupp->getDrawPage();
        if( xPage.is())
        {
            

            

            
            
            uno::Reference< drawing::XShape > xYAxisShape( getYAxis(), uno::UNO_QUERY );
            uno::Reference< drawing::XShape > xXAxisShape( getXAxis(), uno::UNO_QUERY );

            if( xXAxisShape.is() &&
                xYAxisShape.is() )
            {
                
                if( ! mxMyRedLine.is())
                {
                    mxMyRedLine = uno::Reference< drawing::XShape >(
                        xFactory->createInstance( "com.sun.star.drawing.LineShape" ),
                        uno::UNO_QUERY );
                    xPage->add( mxMyRedLine );

                    
                    uno::Reference< beans::XPropertySet > xShapeProp( mxMyRedLine, uno::UNO_QUERY );
                    if( xShapeProp.is())
                    {
                        uno::Any aColor, aWidth;
                        aColor <<= (sal_Int32)(0xe01010);
                        aWidth <<= (sal_Int32)(50);         
                        try
                        {
                            xShapeProp->setPropertyValue( "LineColor" , aColor );
                            xShapeProp->setPropertyValue( "LineWidth" , aWidth );
                        }
                        catch( ... )
                        {}
                    }
                }
                
                if( ! mxMyText.is())
                {
                    mxMyText = uno::Reference< drawing::XShape >(
                        xFactory->createInstance( "com.sun.star.drawing.TextShape" ),
                        uno::UNO_QUERY );
                    xPage->add( mxMyText );

                    
                    OUString aText( "Little Example" );
                    uno::Reference< beans::XPropertySet > xTextProp( mxMyText, uno::UNO_QUERY );
                    if( xTextProp.is())
                    {
                        uno::Any aTrueAny;
                        aTrueAny <<= (sal_Bool)(sal_True);
                        try
                        {
                            xTextProp->setPropertyValue( "TextAutoGrowWidth" , aTrueAny );
                        }
                        catch( ... )
                        {}
                    }

                    uno::Reference< text::XTextRange > xTextRange( mxMyText, uno::UNO_QUERY );
                    if( xTextRange.is())
                    {
                        xTextRange->setString( aText );
                    }
                }

                

                
                
                
                uno::Sequence< uno::Sequence< double > > aData;
                uno::Reference< chart::XChartData > xData = mxChartDoc->getData();
                uno::Reference< chart::XChartDataArray > xDataArray( xData, uno::UNO_QUERY );
                if( xDataArray.is())
                    aData = xDataArray->getData();

                
                sal_Int32 nSize = aData.getLength();
                sal_Int32 nMiddle = nSize / 2;
                
                double fMiddleVal = xData->getNotANumber();     
                if( aData[ nMiddle ].getLength())               
                    fMiddleVal = aData[ nMiddle ][ 0 ];

                awt::Point aPos;
                getLogicalPosition( xYAxisShape, fMiddleVal, sal_True, aPos );
                awt::Size aSize = xXAxisShape->getSize();

                if( mxMyRedLine.is())
                {
                    awt::Point aEnd = aPos;
                    aEnd.X += aSize.Width;

                    uno::Sequence< uno::Sequence< awt::Point > > aPtSeq( 1 );
                    aPtSeq[ 0 ].realloc( 2 );
                    aPtSeq[ 0 ][ 0 ] = aPos;
                    aPtSeq[ 0 ][ 1 ] = aEnd;

                    uno::Reference< beans::XPropertySet > xShapeProp( mxMyRedLine, uno::UNO_QUERY );
                    if( xShapeProp.is())
                    {
                        uno::Any aAny;
                        aAny <<= aPtSeq;
                        xShapeProp->setPropertyValue( "PolyPolygon" , aAny );
                    }
                }
                if( mxMyText.is())
                {
                    
                    aPos.X += ( aSize.Width - mxMyRedLine->getPosition().X ) / 2;
                    aPos.Y += 1000;
                    aPos.Y += static_cast<sal_Int32>(0.1 * xYAxisShape->getSize().Height);
                    mxMyText->setPosition( aPos );
                }
            }
        }
    }
}

void SAL_CALL SampleAddIn::addRefreshListener( const uno::Reference< util::XRefreshListener >&  )
    throw( uno::RuntimeException )
{
    
    
}

void SAL_CALL SampleAddIn::removeRefreshListener( const uno::Reference< util::XRefreshListener >&  )
    throw( uno::RuntimeException )
{
    
    
}


OUString SAL_CALL SampleAddIn::getDiagramType() throw( uno::RuntimeException )
{
    return "com.sun.star.chart.SampleDiagram";
}



uno::Reference< beans::XPropertySet > SAL_CALL SampleAddIn::getDataRowProperties( sal_Int32 nRow )
    throw( lang::IndexOutOfBoundsException,
           uno::RuntimeException )
{
    if( mxChartDoc.is())
    {
        uno::Reference< chart::XDiagram > xDia = mxChartDoc->getDiagram();
        if( xDia.is())
            return xDia->getDataRowProperties( nRow );
    }

    return uno::Reference< beans::XPropertySet >();
}

uno::Reference< beans::XPropertySet > SAL_CALL SampleAddIn::getDataPointProperties( sal_Int32 nCol, sal_Int32 nRow )
    throw( lang::IndexOutOfBoundsException,
           uno::RuntimeException )
{
    if( mxChartDoc.is())
    {
        uno::Reference< chart::XDiagram > xDia = mxChartDoc->getDiagram();
        if( xDia.is())
            return xDia->getDataPointProperties( nCol, nRow );
    }

    return uno::Reference< beans::XPropertySet >();
}


awt::Size SAL_CALL SampleAddIn::getSize()
    throw( uno::RuntimeException )
{
    if( mxChartDoc.is())
    {
        uno::Reference< drawing::XShape > xShape( mxChartDoc->getDiagram(), uno::UNO_QUERY );
        if( xShape.is())
            return xShape->getSize();
    }

    return awt::Size();
}

void SAL_CALL SampleAddIn::setSize( const awt::Size& aSize )
    throw( beans::PropertyVetoException, uno::RuntimeException )
{
    if( mxChartDoc.is())
    {
        uno::Reference< drawing::XShape > xShape( mxChartDoc->getDiagram(), uno::UNO_QUERY );
        if( xShape.is())
            xShape->setSize( aSize );
    }
}

awt::Point SAL_CALL SampleAddIn::getPosition()
    throw( uno::RuntimeException )
{
    if( mxChartDoc.is())
    {
        uno::Reference< drawing::XShape > xShape( mxChartDoc->getDiagram(), uno::UNO_QUERY );
        if( xShape.is())
            return xShape->getPosition();
    }

    return awt::Point();
}

void SAL_CALL SampleAddIn::setPosition( const awt::Point& aPos )
    throw( uno::RuntimeException )
{
    if( mxChartDoc.is())
    {
        uno::Reference< drawing::XShape > xShape( mxChartDoc->getDiagram(), uno::UNO_QUERY );
        if( xShape.is())
            xShape->setPosition( aPos );
    }
}


OUString SAL_CALL SampleAddIn::getShapeType() throw( com::sun::star::uno::RuntimeException )
{
    return "com.sun.star.chart.SampleAddinShape";
}


uno::Reference< drawing::XShape > SAL_CALL SampleAddIn::getXAxisTitle()
    throw( uno::RuntimeException )
{
    if( mxChartDoc.is())
    {
        uno::Reference< chart::XAxisXSupplier > xAxisSupp( mxChartDoc->getDiagram(), uno::UNO_QUERY );
        if( xAxisSupp.is())
            return xAxisSupp->getXAxisTitle();
    }

    return uno::Reference< drawing::XShape >();
}

uno::Reference< beans::XPropertySet > SAL_CALL SampleAddIn::getXAxis()
    throw( uno::RuntimeException )
{
    if( mxChartDoc.is())
    {
        uno::Reference< chart::XAxisXSupplier > xAxisSupp( mxChartDoc->getDiagram(), uno::UNO_QUERY );
        if( xAxisSupp.is())
            return xAxisSupp->getXAxis();
    }

    return uno::Reference< beans::XPropertySet >();
}

uno::Reference< beans::XPropertySet > SAL_CALL SampleAddIn::getXMainGrid()
    throw( uno::RuntimeException )
{
    if( mxChartDoc.is())
    {
        uno::Reference< chart::XAxisXSupplier > xAxisSupp( mxChartDoc->getDiagram(), uno::UNO_QUERY );
        if( xAxisSupp.is())
            return xAxisSupp->getXMainGrid();
    }

    return uno::Reference< beans::XPropertySet >();
}

uno::Reference< beans::XPropertySet > SAL_CALL SampleAddIn::getXHelpGrid()
    throw( uno::RuntimeException )
{
    if( mxChartDoc.is())
    {
        uno::Reference< chart::XAxisXSupplier > xAxisSupp( mxChartDoc->getDiagram(), uno::UNO_QUERY );
        if( xAxisSupp.is())
            return xAxisSupp->getXHelpGrid();
    }

    return uno::Reference< beans::XPropertySet >();
}


uno::Reference< drawing::XShape > SAL_CALL SampleAddIn::getYAxisTitle()
    throw( uno::RuntimeException )
{
    if( mxChartDoc.is())
    {
        uno::Reference< chart::XAxisYSupplier > xAxisSupp( mxChartDoc->getDiagram(), uno::UNO_QUERY );
        if( xAxisSupp.is())
            return xAxisSupp->getYAxisTitle();
    }

    return uno::Reference< drawing::XShape >();
}

uno::Reference< beans::XPropertySet > SAL_CALL SampleAddIn::getYAxis()
    throw( uno::RuntimeException )
{
    if( mxChartDoc.is())
    {
        uno::Reference< chart::XAxisYSupplier > xAxisSupp( mxChartDoc->getDiagram(), uno::UNO_QUERY );
        if( xAxisSupp.is())
            return xAxisSupp->getYAxis();
    }

    return uno::Reference< beans::XPropertySet >();
}

uno::Reference< beans::XPropertySet > SAL_CALL SampleAddIn::getYMainGrid()
    throw( uno::RuntimeException )
{
    if( mxChartDoc.is())
    {
        uno::Reference< chart::XAxisYSupplier > xAxisSupp( mxChartDoc->getDiagram(), uno::UNO_QUERY );
        if( xAxisSupp.is())
            return xAxisSupp->getYMainGrid();
    }

    return uno::Reference< beans::XPropertySet >();
}

uno::Reference< beans::XPropertySet > SAL_CALL SampleAddIn::getYHelpGrid()
    throw( uno::RuntimeException )
{
    if( mxChartDoc.is())
    {
        uno::Reference< chart::XAxisYSupplier > xAxisSupp( mxChartDoc->getDiagram(), uno::UNO_QUERY );
        if( xAxisSupp.is())
            return xAxisSupp->getYHelpGrid();
    }

    return uno::Reference< beans::XPropertySet >();
}


uno::Reference< beans::XPropertySet > SAL_CALL SampleAddIn::getUpBar()
    throw( uno::RuntimeException )
{
    if( mxChartDoc.is())
    {
        uno::Reference< chart::XStatisticDisplay > xStatDisp( mxChartDoc->getDiagram(), uno::UNO_QUERY );
        if( xStatDisp.is())
            return xStatDisp->getUpBar();
    }

    return uno::Reference< beans::XPropertySet >();
}

uno::Reference< beans::XPropertySet > SAL_CALL SampleAddIn::getDownBar()
    throw( uno::RuntimeException )
{
    if( mxChartDoc.is())
    {
        uno::Reference< chart::XStatisticDisplay > xStatDisp( mxChartDoc->getDiagram(), uno::UNO_QUERY );
        if( xStatDisp.is())
            return xStatDisp->getDownBar();
    }

    return uno::Reference< beans::XPropertySet >();
}

uno::Reference< beans::XPropertySet > SAL_CALL SampleAddIn::getMinMaxLine()
    throw( uno::RuntimeException )
{
    if( mxChartDoc.is())
    {
        uno::Reference< chart::XStatisticDisplay > xStatDisp( mxChartDoc->getDiagram(), uno::UNO_QUERY );
        if( xStatDisp.is())
            return xStatDisp->getMinMaxLine();
    }

    return uno::Reference< beans::XPropertySet >();
}


OUString SAL_CALL SampleAddIn::getServiceName() throw( uno::RuntimeException )
{
    return "com.sun.star.chart.SampleAddIn";
}


OUString SAL_CALL SampleAddIn::getImplementationName() throw( uno::RuntimeException )
{
    return getImplementationName_Static();
}

sal_Bool SAL_CALL SampleAddIn::supportsService( const OUString& ServiceName )
    throw( uno::RuntimeException )
{
    return cppu::supportsService(this, ServiceName);
}

uno::Sequence< OUString > SAL_CALL SampleAddIn::getSupportedServiceNames()
    throw( uno::RuntimeException )
{
    return getSupportedServiceNames_Static();
}


void SAL_CALL SampleAddIn::setLocale( const lang::Locale& eLocale )
    throw( uno::RuntimeException )
{
    maLocale = eLocale;
}

lang::Locale SAL_CALL SampleAddIn::getLocale()
    throw( uno::RuntimeException )
{
    return maLocale;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
