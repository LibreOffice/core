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

#include <xmloff/SchXMLSeriesHelper.hxx>
#include <com/sun/star/chart2/XChartDocument.hpp>
#include <com/sun/star/chart2/XChartTypeContainer.hpp>
#include <com/sun/star/chart2/XCoordinateSystemContainer.hpp>
#include <com/sun/star/chart2/XDataSeriesContainer.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>

#include <sal/log.hxx>
#include <comphelper/diagnose_ex.hxx>

using namespace ::com::sun::star;

using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;

::std::vector< Reference< chart2::XDataSeries > >
    SchXMLSeriesHelper::getDataSeriesFromDiagram(
        const Reference< chart2::XDiagram > & xDiagram )
{
    ::std::vector< Reference< chart2::XDataSeries > > aResult;

    try
    {
        Reference< chart2::XCoordinateSystemContainer > xCooSysCnt(
            xDiagram, uno::UNO_QUERY_THROW );
        const Sequence< Reference< chart2::XCoordinateSystem > > aCooSysSeq(
            xCooSysCnt->getCoordinateSystems());
        for( const auto& rCooSys : aCooSysSeq )
        {
            Reference< chart2::XChartTypeContainer > xCTCnt( rCooSys, uno::UNO_QUERY_THROW );
            const Sequence< Reference< chart2::XChartType > > aChartTypeSeq( xCTCnt->getChartTypes());
            for( const auto& rChartType : aChartTypeSeq )
            {
                Reference< chart2::XDataSeriesContainer > xDSCnt( rChartType, uno::UNO_QUERY_THROW );
                const Sequence< Reference< chart2::XDataSeries > > aSeriesSeq( xDSCnt->getDataSeries() );
                aResult.insert( aResult.end(), aSeriesSeq.begin(), aSeriesSeq.end() );
            }
        }
    }
    catch( const uno::Exception & )
    {
        DBG_UNHANDLED_EXCEPTION("xmloff.chart");
    }

    return aResult;
}

::std::map< Reference< chart2::XDataSeries >, sal_Int32 > SchXMLSeriesHelper::getDataSeriesIndexMapFromDiagram(
        const Reference< chart2::XDiagram > & xDiagram )
{
    ::std::map< Reference< chart2::XDataSeries >, sal_Int32 > aRet;

    sal_Int32 nIndex=0;

    ::std::vector< Reference< chart2::XDataSeries > > aSeriesVector( SchXMLSeriesHelper::getDataSeriesFromDiagram( xDiagram ));
    for( const Reference< chart2::XDataSeries >& xSeries : aSeriesVector )
    {
        if( xSeries.is() )
        {
            if( aRet.end() == aRet.find(xSeries) )
                aRet[xSeries]=nIndex;
        }
        nIndex++;
    }
    return aRet;
}

namespace {
uno::Reference< chart2::XChartType > lcl_getChartTypeOfSeries(
                                const uno::Reference< chart2::XDiagram >&   xDiagram
                              , const Reference< chart2::XDataSeries >& xSeries )
{
    if(!xDiagram.is())
        return nullptr;

    //iterate through the model to find the given xSeries
    //the found parent indicates the charttype

    //iterate through all coordinate systems
    uno::Reference< chart2::XCoordinateSystemContainer > xCooSysContainer( xDiagram, uno::UNO_QUERY );
    if( !xCooSysContainer.is())
        return nullptr;

    const uno::Sequence< uno::Reference< chart2::XCoordinateSystem > > aCooSysList( xCooSysContainer->getCoordinateSystems() );
    for( const auto& xCooSys : aCooSysList )
    {
        //iterate through all chart types in the current coordinate system
        uno::Reference< chart2::XChartTypeContainer > xChartTypeContainer( xCooSys, uno::UNO_QUERY );
        SAL_WARN_IF( !xChartTypeContainer.is(), "xmloff.chart", "xChartTypeContainer is NULL");
        if( !xChartTypeContainer.is() )
            continue;
        const uno::Sequence< uno::Reference< chart2::XChartType > > aChartTypeList( xChartTypeContainer->getChartTypes() );
        for( const auto& xChartType : aChartTypeList )
        {
            //iterate through all series in this chart type
            uno::Reference< chart2::XDataSeriesContainer > xDataSeriesContainer( xChartType, uno::UNO_QUERY );
            SAL_WARN_IF( !xDataSeriesContainer.is(), "xmloff.chart", "xDataSeriesContainer is NULL");
            if( !xDataSeriesContainer.is() )
                continue;

            const uno::Sequence< uno::Reference< chart2::XDataSeries > > aSeriesList( xDataSeriesContainer->getDataSeries() );
            if (std::find(aSeriesList.begin(), aSeriesList.end(), xSeries) != aSeriesList.end())
                return xChartType;
        }
    }
    return nullptr;
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
                OUString aServiceName( xChartType->getChartType() );
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
                xRet.set( xFactory->createInstance( u"com.sun.star.comp.chart2.DataSeriesWrapper"_ustr ), uno::UNO_QUERY );
                Reference< lang::XInitialization > xInit( xRet, uno::UNO_QUERY );
                if(xInit.is())
                {
                    xInit->initialize( { uno::Any(xSeries) });
                }
            }
        }
        catch( const uno::Exception & )
        {
            TOOLS_INFO_EXCEPTION("xmloff.chart", "Exception caught SchXMLSeriesHelper::createOldAPISeriesPropertySet" );
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
                xRet.set( xFactory->createInstance( u"com.sun.star.comp.chart2.DataSeriesWrapper"_ustr ), uno::UNO_QUERY );
                Reference< lang::XInitialization > xInit( xRet, uno::UNO_QUERY );
                if(xInit.is())
                {
                    xInit->initialize({ uno::Any(xSeries), uno::Any(nPointIndex) });
                }
            }
        }
        catch( const uno::Exception & )
        {
            TOOLS_INFO_EXCEPTION("xmloff.chart", "Exception caught SchXMLSeriesHelper::createOldAPIDataPointPropertySet" );
        }
    }

    return xRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
