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

#include "ChartRenderer.hxx"
#include "servicenames.hxx"
#include "chartview/ExplicitValueProvider.hxx"
#include "chartview/DrawModelWrapper.hxx"
#include "DrawViewWrapper.hxx"
#include "macros.hxx"
#include "ChartModelHelper.hxx"
#include <vcl/svapp.hxx>
#include <osl/mutex.hxx>

#include <com/sun/star/util/XUpdatable.hpp>

//.............................................................................
namespace chart
{
//.............................................................................

using namespace ::com::sun::star;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
using rtl::OUString;

ChartRenderer::ChartRenderer( const Reference< frame::XModel >& xChartModel )
    : m_xChartModel( uno::WeakReference< frame::XModel >(xChartModel) )
{
}

ChartRenderer::~ChartRenderer()
{
}

// ____ ChartPrettyPainter ___
bool ChartRenderer::DoPaint(OutputDevice* pOutDev, const Rectangle& rLogicObjectRect /*in given output dev map units*/ ) const
{
    if(!pOutDev)
        return false;

    Reference<frame::XModel> xModel(m_xChartModel);
    uno::Reference< lang::XMultiServiceFactory > xFact( xModel, uno::UNO_QUERY );
    if( !xFact.is() )
        return false;

    //#i84323# ensure that the size the container has of the chart and size that chart has itself are the same
    //maybe remove this workaround somewhere in future again
    {
        if( MAP_100TH_MM == pOutDev->GetMapMode().GetMapUnit() )//no problem for writer in issue #i84323#; but writer uses twips instead of 100thmm which causes small differences in size always when converting to 100thmm and back
        {
            awt::Size aChartSize( ChartModelHelper::getPageSize(xModel) );
            awt::Size aContainerChartSize( rLogicObjectRect.GetWidth(), rLogicObjectRect.GetHeight() );
            if( aContainerChartSize.Width != aChartSize.Width
                || aContainerChartSize.Height != aChartSize.Height )
            {
                OSL_FAIL("chart size does not equal size assumed by the container");
                //correct the state here on the fly -> let the container size win
                ChartModelHelper::setPageSize( aContainerChartSize, xModel );
            }
        }
    }

    // get the chart view
    Reference< lang::XUnoTunnel > xChartView( xFact->createInstance( CHART_VIEW_SERVICE_NAME ), uno::UNO_QUERY );

    try
    {
        //better performance for big data
        uno::Reference< beans::XPropertySet > xProp( xChartView, uno::UNO_QUERY );
        if( xProp.is() )
        {
            awt::Size aResolution(1000,1000);
            {
                SolarMutexGuard aGuard;
                Rectangle aPixelRect( pOutDev->LogicToPixel( rLogicObjectRect ) );
                aResolution.Width = aPixelRect.GetWidth();
                aResolution.Height = aPixelRect.GetHeight();
            }
            xProp->setPropertyValue( C2U("Resolution"), uno::makeAny( aResolution ));
        }
        //

        uno::Reference< util::XUpdatable > xUpdatable( xChartView, uno::UNO_QUERY );
        if( xUpdatable.is() )
            xUpdatable->update();

        SolarMutexGuard aGuard;
        ExplicitValueProvider* pProvider = ExplicitValueProvider::getExplicitValueProvider( xChartView );
        if( !pProvider )
            return false;
        ::boost::shared_ptr< DrawModelWrapper > pDrawModelWrapper = pProvider->getDrawModelWrapper();
        if( !pDrawModelWrapper.get() )
            return false;
        ::boost::shared_ptr< DrawViewWrapper > pDrawViewWrapper( new DrawViewWrapper(&pDrawModelWrapper->getSdrModel(),pOutDev,false) );
        pDrawViewWrapper->attachParentReferenceDevice( xModel );

        MapMode aOldMapMode( pOutDev->GetMapMode()  );
        Point aOldOrigin( aOldMapMode.GetOrigin() );
        MapMode aMapMode( aOldMapMode  );
        Point aOldOriginMM( OutputDevice::LogicToLogic( aOldOrigin, aOldMapMode.GetMapUnit(), MAP_100TH_MM ) );
        Point aObjectTopLeftMM( OutputDevice::LogicToLogic( rLogicObjectRect.TopLeft(), aOldMapMode.GetMapUnit(), MAP_100TH_MM ) );
        aMapMode.SetOrigin( aOldOriginMM + aObjectTopLeftMM );
        aMapMode.SetMapUnit( MAP_100TH_MM );
        pOutDev->SetMapMode(aMapMode);

        Rectangle aPaintRect( OutputDevice::LogicToLogic( rLogicObjectRect, aOldMapMode, aMapMode ) );
        pDrawViewWrapper->CompleteRedraw(pOutDev, Region(aPaintRect) );

        pOutDev->SetMapMode(aOldMapMode);
    }
    catch( const uno::Exception & ex )
    {
        ASSERT_EXCEPTION( ex );
    }
    catch( ... )
    {
    }

    return true;
}

//-----------------------------------------------------------------
// ____ XUnoTunnel ___
::sal_Int64 SAL_CALL ChartRenderer::getSomething( const Sequence< ::sal_Int8 >& aIdentifier )
        throw( uno::RuntimeException)
{
    if( aIdentifier.getLength() == 16 && 0 == memcmp(
            ChartPrettyPainter::getUnoTunnelId().getConstArray(),
                aIdentifier.getConstArray(), 16 ) )
    {
        ChartPrettyPainter* pPaintableObject = this;
        return reinterpret_cast<sal_Int64>(pPaintableObject);
    }
    return 0;
}

//-----------------------------------------------------------------
// lang::XServiceInfo

APPHELPER_XSERVICEINFO_IMPL(ChartRenderer,CHART_RENDERER_SERVICE_IMPLEMENTATION_NAME)

Sequence< OUString > ChartRenderer::getSupportedServiceNames_Static()
{
    Sequence< OUString > aSNS;
    return aSNS;
}

//.............................................................................
} //namespace chart
//.............................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
