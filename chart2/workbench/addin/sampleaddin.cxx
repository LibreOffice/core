/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
#include "sampleaddin.hxx"

#include <cppuhelper/factory.hxx>
#include <osl/diagnose.h>

#include <com/sun/star/drawing/XDrawPageSupplier.hpp>
#include <com/sun/star/drawing/XDrawPage.hpp>
#include <com/sun/star/chart/XChartDataArray.hpp>
#include <com/sun/star/text/XTextRange.hpp>
#include <com/sun/star/chart/X3DDisplay.hpp>

using namespace com::sun::star;
using namespace rtl;

// code for creating instances of SampleAddIn

extern "C" {

void SAL_CALL component_getImplementationEnvironment(
    const sal_Char ** ppEnvTypeName, uno_Environment ** /*ppEnv*/ )
{
    *ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
}

sal_Bool SAL_CALL component_writeInfo(
    void * /*pServiceManager*/, registry::XRegistryKey * pRegistryKey )
{
    if( pRegistryKey )
    {
        try
        {
            OUString aImpl( RTL_CONSTASCII_USTRINGPARAM( "/" ));
            aImpl += SampleAddIn::getImplementationName_Static();
            aImpl += OUString( RTL_CONSTASCII_USTRINGPARAM( "/UNO/SERVICES" ));

            uno::Reference< registry::XRegistryKey> xNewKey(
                reinterpret_cast<registry::XRegistryKey*>( pRegistryKey )->createKey( aImpl ) );

            uno::Sequence< OUString > aSequ = SampleAddIn::getSupportedServiceNames_Static();
            const OUString * pArray = aSequ.getConstArray();
            for( sal_Int32 i = 0; i < aSequ.getLength(); i++ )
                xNewKey->createKey( pArray[i] );

            return sal_True;
        }
        catch( registry::InvalidRegistryException& )
        {
            OSL_ENSURE( sal_False, "### InvalidRegistryException!" );
        }
    }
    return sal_False;
}

void * SAL_CALL component_getFactory(
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

}   // extern C


// --------------------
// class SampleAddIn
// --------------------

SampleAddIn::SampleAddIn()
{

}

SampleAddIn::~SampleAddIn()
{}


// this functionality should be provided by the chart API some day
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
                uno::Any aAny = xProp->getPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM( "Min" )));
                aAny >>= fMin;
                aAny = xProp->getPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM( "Max" )));
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
                        aOutPosition.Y = static_cast<sal_Int32>(aPos.Y + nLength * (1.0 - fPercentage));    // y scale goes from top to bottom
                    }
                    else
                    {
                        aOutPosition.X = static_cast<sal_Int32>(aPos.X + nLength * fPercentage);
                        aOutPosition.Y = aPos.Y;
                    }
                    bRet = sal_True;
                }
            }
            catch( beans::UnknownPropertyException )
            {
                // the shape xAxis was no chart axis
            }
        }
    }

    return bRet;
}

OUString SampleAddIn::getImplementationName_Static()
{
    return OUString( RTL_CONSTASCII_USTRINGPARAM( "SampleAddIn" ));
}

uno::Sequence< ::rtl::OUString > SampleAddIn::getSupportedServiceNames_Static()
{
    uno::Sequence< OUString > aSeq( 4 );

    aSeq[ 0 ] = OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.chart.ChartAxisXSupplier" ));
    aSeq[ 1 ] = OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.chart.ChartAxisYSupplier" ));
    aSeq[ 2 ] = OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.chart.Diagram" ));
    aSeq[ 3 ] = OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.chart.SampleAddIn" ));

    return aSeq;
}

uno::Reference< uno::XInterface > SAL_CALL SampleAddIn_CreateInstance(
    const uno::Reference< lang::XMultiServiceFactory >& )
{
    uno::Reference< uno::XInterface > xInst = (cppu::OWeakObject*)new SampleAddIn();

    return xInst;
}

// implementation of interface methods

// XInitialization
void SAL_CALL SampleAddIn::initialize( const uno::Sequence< uno::Any >& aArguments )
    throw( uno::Exception, uno::RuntimeException )
{
    // first argument should be the XChartDocument
    OSL_ENSURE( aArguments.getLength() > 0, "Please initialize Chart AddIn with ChartDocument!" );

    if( aArguments.getLength())
    {
        aArguments[ 0 ] >>= mxChartDoc;
        OSL_ENSURE( mxChartDoc.is(), "First argument in initialization is not an XChartDocument!" );

        // set XY chart as base type to be drawn
        uno::Reference< beans::XPropertySet > xDocProp( mxChartDoc, uno::UNO_QUERY );
        if( xDocProp.is())
        {
            uno::Any aBaseType;
            aBaseType <<= rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.chart.XYDiagram" ));
            try
            {
                xDocProp->setPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "BaseDiagram" )), aBaseType );
            }
            catch( ... )
            {}
        }

        // change background of plot area to light blue
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
                xDiaProp->setPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM( "FillColor" )), aAny );
                xLegendProp->setPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM( "FillColor" )), aAny );
            }
        }
    }
}

// XRefreshable
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

    // first of all get the draw page
    uno::Reference< drawing::XDrawPageSupplier > xPageSupp( mxChartDoc, uno::UNO_QUERY );
    uno::Reference< lang::XMultiServiceFactory > xFactory( mxChartDoc, uno::UNO_QUERY );
    if( xPageSupp.is() &&
        xFactory.is() )
    {
        uno::Reference< drawing::XDrawPage > xPage = xPageSupp->getDrawPage();
        if( xPage.is())
        {
            // now we have the page to insert objects

            // add a horizontal line at the middle value of the first series
            // -------------------------------------------------------------


            // get the logical position from the coordinate
            // get x- and y-axis
            uno::Reference< drawing::XShape > xYAxisShape( getYAxis(), uno::UNO_QUERY );
            uno::Reference< drawing::XShape > xXAxisShape( getXAxis(), uno::UNO_QUERY );

            if( xXAxisShape.is() &&
                xYAxisShape.is() )
            {
                // create line first time
                if( ! mxMyRedLine.is())
                {
                    mxMyRedLine = uno::Reference< drawing::XShape >(
                        xFactory->createInstance( OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.drawing.LineShape" ))),
                        uno::UNO_QUERY );
                    xPage->add( mxMyRedLine );

                    // make line red and thick
                    uno::Reference< beans::XPropertySet > xShapeProp( mxMyRedLine, uno::UNO_QUERY );
                    if( xShapeProp.is())
                    {
                        uno::Any aColor, aWidth;
                        aColor <<= (sal_Int32)(0xe01010);
                        aWidth <<= (sal_Int32)(50);         // 0.5 mm
                        try
                        {
                            xShapeProp->setPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM( "LineColor" )), aColor );
                            xShapeProp->setPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM( "LineWidth" )), aWidth );
                        }
                        catch( ... )
                        {}
                    }
                }
                // create text object first time
                if( ! mxMyText.is())
                {
                    mxMyText = uno::Reference< drawing::XShape >(
                        xFactory->createInstance( OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.drawing.TextShape" ))),
                        uno::UNO_QUERY );
                    xPage->add( mxMyText );

                    // change text
                    OUString aText(RTL_CONSTASCII_USTRINGPARAM( "Little Example" ));
                    uno::Reference< beans::XPropertySet > xTextProp( mxMyText, uno::UNO_QUERY );
                    if( xTextProp.is())
                    {
                        uno::Any aTrueAny;
                        aTrueAny <<= (sal_Bool)(sal_True);
                        try
                        {
                            xTextProp->setPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "TextAutoGrowWidth" )), aTrueAny );
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


                // position line and text

                // get the array. Note: the first dimension is the length
                // of each series and the second one is the number of series
                // this should be changed in the future
                uno::Sequence< uno::Sequence< double > > aData;
                uno::Reference< chart::XChartData > xData = mxChartDoc->getData();
                uno::Reference< chart::XChartDataArray > xDataArray( xData, uno::UNO_QUERY );
                if( xDataArray.is())
                    aData = xDataArray->getData();

                // get row count == length of each series
                sal_Int32 nSize = aData.getLength();
                sal_Int32 nMiddle = nSize / 2;
                // get value for first series
                double fMiddleVal = xData->getNotANumber();     // set to NaN
                if( aData[ nMiddle ].getLength())               // we have at least one series
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
                        xShapeProp->setPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "PolyPolygon" )), aAny );
                    }
                }
                if( mxMyText.is())
                {
                    // put the text centered below the red line
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
    // not implemented - this is not necessary
    // (this method exists just because the interface requires it)
}

void SAL_CALL SampleAddIn::removeRefreshListener( const uno::Reference< util::XRefreshListener >&  )
    throw( uno::RuntimeException )
{
    // not implemented - this is not necessary
    // (this method exists just because the interface requires it)
}

// XDiagram
OUString SAL_CALL SampleAddIn::getDiagramType() throw( uno::RuntimeException )
{
    return OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.chart.SampleDiagram" ));
}

// the following methods just delegate to the "parent diagram" (which in the future might no longer exist)

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

// XShape ( ::XDiagram )
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

// XShapeDescriptor ( ::XShape ::XDiagram )
rtl::OUString SAL_CALL SampleAddIn::getShapeType() throw( com::sun::star::uno::RuntimeException )
{
    return OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.chart.SampleAddinShape" ));
}

// XAxisXSupplier
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

// XAxisYSupplier
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

// XStatisticDisplay
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

// XServiceName
OUString SAL_CALL SampleAddIn::getServiceName() throw( uno::RuntimeException )
{
    return OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.chart.SampleAddIn" ));
}

// XServiceInfo
OUString SAL_CALL SampleAddIn::getImplementationName() throw( uno::RuntimeException )
{
    return getImplementationName_Static();
}

sal_Bool SAL_CALL SampleAddIn::supportsService( const OUString& ServiceName )
    throw( uno::RuntimeException )
{
    uno::Sequence< OUString > aServiceSeq = getSupportedServiceNames_Static();

    sal_Int32 nLength = aServiceSeq.getLength();
    for( sal_Int32 i=0; i < nLength; i++ )
    {
        if( ServiceName.equals( aServiceSeq[ i ] ))
            return sal_True;
    }

    return sal_False;
}

uno::Sequence< OUString > SAL_CALL SampleAddIn::getSupportedServiceNames()
    throw( uno::RuntimeException )
{
    return getSupportedServiceNames_Static();
}

// XLocalizable
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
