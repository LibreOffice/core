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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_chart2.hxx"
#include "ChartFrameloader.hxx"
#include "servicenames.hxx"
#include "MediaDescriptorHelper.hxx"
#include "macros.hxx"
#include <comphelper/mediadescriptor.hxx>
#include <com/sun/star/document/XImporter.hpp>
#include <com/sun/star/document/XFilter.hpp>
#include <com/sun/star/frame/XLoadable.hpp>

//.............................................................................
namespace chart
{
//.............................................................................

using namespace ::com::sun::star;

ChartFrameLoader::ChartFrameLoader(
        uno::Reference<uno::XComponentContext> const & xContext)
        : m_bCancelRequired( sal_False )
{
    m_xCC = xContext;
    m_oCancelFinished.reset();
}

ChartFrameLoader::~ChartFrameLoader()
{
}

    sal_Bool ChartFrameLoader
::impl_checkCancel()
{
    if(m_bCancelRequired)
    {
        m_oCancelFinished.set();
        return sal_True;
    }
    return sal_False;
}

//-----------------------------------------------------------------
// lang::XServiceInfo
//-----------------------------------------------------------------

APPHELPER_XSERVICEINFO_IMPL(ChartFrameLoader,CHART_FRAMELOADER_SERVICE_IMPLEMENTATION_NAME)

    uno::Sequence< rtl::OUString > ChartFrameLoader
::getSupportedServiceNames_Static()
{
    uno::Sequence< rtl::OUString > aSNS( 1 );
    aSNS.getArray()[ 0 ] = CHART_FRAMELOADER_SERVICE_NAME;
    return aSNS;
}

//-----------------------------------------------------------------
// frame::XFrameLoader
//-----------------------------------------------------------------

    sal_Bool SAL_CALL ChartFrameLoader
::load( const uno::Sequence< beans::PropertyValue >& rMediaDescriptor
        , const uno::Reference<frame::XFrame >& xFrame )
        throw (uno::RuntimeException)
{
    //@todo ? need to add as terminate listener to desktop?

    uno::Reference< frame::XModel >         xModel;
    bool bHaveLoadedModel = false;

    comphelper::MediaDescriptor aMediaDescriptor(rMediaDescriptor);
    {
        comphelper::MediaDescriptor::const_iterator aIt( aMediaDescriptor.find( aMediaDescriptor.PROP_MODEL()));
        if( aIt != aMediaDescriptor.end())
        {
            xModel.set( (*aIt).second.get< uno::Reference< frame::XModel > >());
            bHaveLoadedModel = true;
        }
    }

    //create and initialize the model
    if( ! xModel.is())
    {
        //@todo?? load mechanism to cancel during loading of document
        xModel.set(
                m_xCC->getServiceManager()->createInstanceWithContext(
                CHART_MODEL_SERVICE_IMPLEMENTATION_NAME, m_xCC )
                , uno::UNO_QUERY );

        if( impl_checkCancel() )
            return sal_False;
    }

    //create the controller(+XWindow)
    uno::Reference< frame::XController >    xController = NULL;
    uno::Reference< awt::XWindow >          xComponentWindow = NULL;
    {
        xController = uno::Reference< frame::XController >(
            m_xCC->getServiceManager()->createInstanceWithContext(
            CHART_CONTROLLER_SERVICE_IMPLEMENTATION_NAME,m_xCC )
            , uno::UNO_QUERY );

        //!!!it is a special characteristic of the example application
        //that the controller simultaniously provides the XWindow controller functionality
        xComponentWindow =
                      uno::Reference< awt::XWindow >( xController, uno::UNO_QUERY );

        if( impl_checkCancel() )
            return sal_False;
    }


    //connect frame, controller and model one to each other:
    if(xController.is()&&xModel.is())
    {
        xModel->connectController(xController);
        xModel->setCurrentController(xController);
        xController->attachModel(xModel);
        if(xFrame.is())
            xFrame->setComponent(xComponentWindow,xController);
        //creates the view and menu
        //for correct menu creation the initialized component must be already set into the frame
        xController->attachFrame(xFrame);
    }

    // call initNew() or load() at XLoadable
    if(!bHaveLoadedModel)
    try
    {
        comphelper::MediaDescriptor::const_iterator aIt( aMediaDescriptor.find( aMediaDescriptor.PROP_URL()));
        if( aIt != aMediaDescriptor.end())
        {
            ::rtl::OUString aURL( (*aIt).second.get< ::rtl::OUString >());
            if( aURL.matchAsciiL(
                RTL_CONSTASCII_STRINGPARAM( "private:factory/schart" )))
            {
                // create new file
                uno::Reference< frame::XLoadable > xLoadable( xModel, uno::UNO_QUERY_THROW );
                xLoadable->initNew();
            }
            else
            {
                aMediaDescriptor.addInputStream();
                uno::Sequence< beans::PropertyValue > aCompleteMediaDescriptor;
                aMediaDescriptor >> aCompleteMediaDescriptor;
                apphelper::MediaDescriptorHelper aMDHelper( aCompleteMediaDescriptor );

                // load file
                // @todo: replace: aMediaDescriptorHelper.getReducedForModel()
                uno::Reference< frame::XLoadable > xLoadable( xModel, uno::UNO_QUERY_THROW );
                xLoadable->load( aCompleteMediaDescriptor );

                //resize standalone files to get correct size:
                if( xComponentWindow.is() && aMDHelper.ISSET_FilterName && aMDHelper.FilterName.equals( C2U("StarChart 5.0")) )
                {
                    awt::Rectangle aRect( xComponentWindow->getPosSize() );
                    sal_Int16 nFlags=0;
                    xComponentWindow->setPosSize( aRect.X, aRect.Y, aRect.Width, aRect.Height, nFlags );
                }
            }
        }
    }
    catch( uno::Exception & ex )
    {
        ASSERT_EXCEPTION( ex );
    }

    return sal_True;
}

    void SAL_CALL ChartFrameLoader
::cancel() throw (uno::RuntimeException)
{
    m_oCancelFinished.reset();
    m_bCancelRequired = sal_True;
    m_oCancelFinished.wait();
    m_bCancelRequired = sal_False;
}

//.............................................................................
} //namespace chart
//.............................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
