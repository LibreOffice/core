/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ChartRenderer.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: ihi $ $Date: 2008-01-14 13:58:13 $
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

#include "ChartRenderer.hxx"
#include "servicenames.hxx"
#include "chartview/ExplicitValueProvider.hxx"
#include "chartview/DrawModelWrapper.hxx"
#include "DrawViewWrapper.hxx"
#include "macros.hxx"
#include "ChartModelHelper.hxx"

#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif
#ifndef _VOS_MUTEX_HXX_
#include <vos/mutex.hxx>
#endif

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

ChartRenderer::ChartRenderer( const Reference< uno::XComponentContext >& /*xContext*/)
    : m_xChartModel()
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
                DBG_ERROR("chart size does not equal size assumed by the container");
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
                ::vos::OGuard aGuard( Application::GetSolarMutex());
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

        ::vos::OGuard aGuard( Application::GetSolarMutex());
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
    catch( uno::Exception & ex )
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
    if( aIdentifier.getLength() == 16 && 0 == rtl_compareMemory(
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
