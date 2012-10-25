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


#include "SchXMLSeriesHelper.hxx"
#include <com/sun/star/chart2/XChartDocument.hpp>
#include <com/sun/star/chart2/XChartTypeContainer.hpp>
#include <com/sun/star/chart2/XCoordinateSystemContainer.hpp>
#include <com/sun/star/chart2/XDataSeriesContainer.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>

// header for define RTL_CONSTASCII_USTRINGPARAM
#include <rtl/ustring.h>
// header for define DBG_ERROR1
#include <tools/debug.hxx>

#include <typeinfo>

using namespace ::com::sun::star;
using ::rtl::OUString;
using ::rtl::OUStringToOString;

using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
using ::rtl::OUString;

// ----------------------------------------

::std::vector< Reference< chart2::XDataSeries > >
    SchXMLSeriesHelper::getDataSeriesFromDiagram(
        const Reference< chart2::XDiagram > & xDiagram )
{
    ::std::vector< Reference< chart2::XDataSeries > > aResult;

    try
    {
        Reference< chart2::XCoordinateSystemContainer > xCooSysCnt(
            xDiagram, uno::UNO_QUERY_THROW );
        Sequence< Reference< chart2::XCoordinateSystem > > aCooSysSeq(
            xCooSysCnt->getCoordinateSystems());
        for( sal_Int32 i=0; i<aCooSysSeq.getLength(); ++i )
        {
            Reference< chart2::XChartTypeContainer > xCTCnt( aCooSysSeq[i], uno::UNO_QUERY_THROW );
            Sequence< Reference< chart2::XChartType > > aChartTypeSeq( xCTCnt->getChartTypes());
            for( sal_Int32 j=0; j<aChartTypeSeq.getLength(); ++j )
            {
                Reference< chart2::XDataSeriesContainer > xDSCnt( aChartTypeSeq[j], uno::UNO_QUERY_THROW );
                Sequence< Reference< chart2::XDataSeries > > aSeriesSeq( xDSCnt->getDataSeries() );
                ::std::copy( aSeriesSeq.getConstArray(), aSeriesSeq.getConstArray() + aSeriesSeq.getLength(),
                             ::std::back_inserter( aResult ));
            }
        }
    }
    catch( const uno::Exception & ex )
    {
        (void)ex; // avoid warning for pro build

        OSL_FAIL( OUStringToOString( OUString(
                        OUString(  "Exception caught. Type: " ) +
                        OUString::createFromAscii( typeid( ex ).name()) +
                        OUString(  ", Message: " ) +
                        ex.Message), RTL_TEXTENCODING_ASCII_US ).getStr());

    }

    return aResult;
}

::std::map< Reference< chart2::XDataSeries >, sal_Int32 > SchXMLSeriesHelper::getDataSeriesIndexMapFromDiagram(
        const Reference< chart2::XDiagram > & xDiagram )
{
    ::std::map< Reference< chart2::XDataSeries >, sal_Int32 > aRet;

    sal_Int32 nIndex=0;

    ::std::vector< Reference< chart2::XDataSeries > > aSeriesVector( SchXMLSeriesHelper::getDataSeriesFromDiagram( xDiagram ));
    for( ::std::vector< Reference< chart2::XDataSeries > >::const_iterator aSeriesIt( aSeriesVector.begin() )
        ; aSeriesIt != aSeriesVector.end()
        ; aSeriesIt++, nIndex++ )
    {
        Reference< chart2::XDataSeries > xSeries( *aSeriesIt );
        if( xSeries.is() )
        {
            if( aRet.end() == aRet.find(xSeries) )
                aRet[xSeries]=nIndex;
        }
    }
    return aRet;
}

namespace {
uno::Reference< chart2::XChartType > lcl_getChartTypeOfSeries(
                                const uno::Reference< chart2::XDiagram >&   xDiagram
                              , const Reference< chart2::XDataSeries >& xSeries )
{
    if(!xDiagram.is())
        return 0;

    //iterate through the model to find the given xSeries
    //the found parent indicates the charttype

    //iterate through all coordinate systems
    uno::Reference< chart2::XCoordinateSystemContainer > xCooSysContainer( xDiagram, uno::UNO_QUERY );
    if( !xCooSysContainer.is())
        return 0;

    uno::Sequence< uno::Reference< chart2::XCoordinateSystem > > aCooSysList( xCooSysContainer->getCoordinateSystems() );
    for( sal_Int32 nCS = 0; nCS < aCooSysList.getLength(); ++nCS )
    {
        uno::Reference< chart2::XCoordinateSystem > xCooSys( aCooSysList[nCS] );

        //iterate through all chart types in the current coordinate system
        uno::Reference< chart2::XChartTypeContainer > xChartTypeContainer( xCooSys, uno::UNO_QUERY );
        OSL_ASSERT( xChartTypeContainer.is());
        if( !xChartTypeContainer.is() )
            continue;
        uno::Sequence< uno::Reference< chart2::XChartType > > aChartTypeList( xChartTypeContainer->getChartTypes() );
        for( sal_Int32 nT = 0; nT < aChartTypeList.getLength(); ++nT )
        {
            uno::Reference< chart2::XChartType > xChartType( aChartTypeList[nT] );

            //iterate through all series in this chart type
            uno::Reference< chart2::XDataSeriesContainer > xDataSeriesContainer( xChartType, uno::UNO_QUERY );
            OSL_ASSERT( xDataSeriesContainer.is());
            if( !xDataSeriesContainer.is() )
                continue;

            uno::Sequence< uno::Reference< chart2::XDataSeries > > aSeriesList( xDataSeriesContainer->getDataSeries() );
            for( sal_Int32 nS = 0; nS < aSeriesList.getLength(); ++nS )
            {
                Reference< chart2::XDataSeries > xCurrentSeries( aSeriesList[nS] );

                if( xSeries == xCurrentSeries )
                    return xChartType;
            }
        }
    }
    return 0;
}
}

bool SchXMLSeriesHelper::isCandleStickSeries(
                  const Reference< chart2::XDataSeries >& xSeries
                , const Reference< frame::XModel >& xChartModel )
{
    bool bRet = false;

    uno::Reference< chart2::XChartDocument > xNewDoc( xChartModel, uno::UNO_QUERY );
    if( xNewDoc.is() )
    {
        uno::Reference< chart2::XDiagram > xNewDiagram( xNewDoc->getFirstDiagram() );
        if( xNewDiagram.is() )
        {
            uno::Reference< chart2::XChartType > xChartType( lcl_getChartTypeOfSeries(
                                        xNewDiagram, xSeries ) );
            if( xChartType.is() )
            {
                rtl::OUString aServiceName( xChartType->getChartType() );
                if( aServiceName == "com.sun.star.chart2.CandleStickChartType" )
                    bRet = true;
            }
        }
    }
    return bRet;
}

//static
uno::Reference< beans::XPropertySet > SchXMLSeriesHelper::createOldAPISeriesPropertySet(
            const uno::Reference< chart2::XDataSeries >& xSeries
            , const uno::Reference< frame::XModel >& xChartModel )
{
    uno::Reference< beans::XPropertySet > xRet;

    if( xSeries.is() )
    {
        try
        {
            uno::Reference< lang::XMultiServiceFactory > xFactory( xChartModel, uno::UNO_QUERY );
            if( xFactory.is() )
            {
                xRet = uno::Reference< beans::XPropertySet >( xFactory->createInstance(
                    OUString( "com.sun.star.comp.chart2.DataSeriesWrapper" ) ), uno::UNO_QUERY );
                Reference< lang::XInitialization > xInit( xRet, uno::UNO_QUERY );
                if(xInit.is())
                {
                    Sequence< uno::Any > aArguments(1);
                    aArguments[0]=uno::makeAny(xSeries);
                    xInit->initialize(aArguments);
                }
            }
        }
        catch( const uno::Exception & rEx )
        {
            (void)rEx; // avoid warning for pro build
            OSL_TRACE( "Exception caught SchXMLSeriesHelper::createOldAPISeriesPropertySet: %s",
                        OUStringToOString( rEx.Message, RTL_TEXTENCODING_ASCII_US ).getStr() );
        }
    }

    return xRet;
}

//static
uno::Reference< beans::XPropertySet > SchXMLSeriesHelper::createOldAPIDataPointPropertySet(
            const uno::Reference< chart2::XDataSeries >& xSeries
            , sal_Int32 nPointIndex
            , const uno::Reference< frame::XModel >& xChartModel )
{
    uno::Reference< beans::XPropertySet > xRet;

    if( xSeries.is() )
    {
        try
        {
            uno::Reference< lang::XMultiServiceFactory > xFactory( xChartModel, uno::UNO_QUERY );
            if( xFactory.is() )
            {
                xRet = uno::Reference< beans::XPropertySet >( xFactory->createInstance(
                    OUString( "com.sun.star.comp.chart2.DataSeriesWrapper" ) ), uno::UNO_QUERY );
                Reference< lang::XInitialization > xInit( xRet, uno::UNO_QUERY );
                if(xInit.is())
                {
                    Sequence< uno::Any > aArguments(2);
                    aArguments[0]=uno::makeAny(xSeries);
                    aArguments[1]=uno::makeAny(nPointIndex);
                    xInit->initialize(aArguments);
                }
            }
        }
        catch( const uno::Exception & rEx )
        {
            (void)rEx; // avoid warning for pro build

            OSL_TRACE( "Exception caught SchXMLSeriesHelper::createOldAPIDataPointPropertySet: %s",
                        OUStringToOString( rEx.Message, RTL_TEXTENCODING_ASCII_US ).getStr() );
        }
    }

    return xRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
