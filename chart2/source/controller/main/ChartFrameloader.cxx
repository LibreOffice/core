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

#include "ChartFrameloader.hxx"
#include "servicenames.hxx"
#include "MediaDescriptorHelper.hxx"
#include "macros.hxx"
#include <unotools/mediadescriptor.hxx>
#include <com/sun/star/document/XImporter.hpp>
#include <com/sun/star/document/XFilter.hpp>
#include <com/sun/star/frame/XLoadable.hpp>

namespace chart
{

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



APPHELPER_XSERVICEINFO_IMPL(ChartFrameLoader,CHART_FRAMELOADER_SERVICE_IMPLEMENTATION_NAME)

    uno::Sequence< OUString > ChartFrameLoader
::getSupportedServiceNames_Static()
{
    uno::Sequence< OUString > aSNS( 1 );
    aSNS.getArray()[ 0 ] = CHART_FRAMELOADER_SERVICE_NAME;
    return aSNS;
}



    sal_Bool SAL_CALL ChartFrameLoader
::load( const uno::Sequence< beans::PropertyValue >& rMediaDescriptor
        , const uno::Reference<frame::XFrame >& xFrame )
        throw (uno::RuntimeException)
{
    

    uno::Reference< frame::XModel >         xModel;
    bool bHaveLoadedModel = false;

    utl::MediaDescriptor aMediaDescriptor(rMediaDescriptor);
    {
        utl::MediaDescriptor::const_iterator aIt( aMediaDescriptor.find( aMediaDescriptor.PROP_MODEL()));
        if( aIt != aMediaDescriptor.end())
        {
            xModel.set( (*aIt).second.get< uno::Reference< frame::XModel > >());
            bHaveLoadedModel = true;
        }
    }

    
    if( ! xModel.is())
    {
        
        xModel.set(
                m_xCC->getServiceManager()->createInstanceWithContext(
                CHART_MODEL_SERVICE_IMPLEMENTATION_NAME, m_xCC )
                , uno::UNO_QUERY );

        if( impl_checkCancel() )
            return sal_False;
    }

    
    uno::Reference< frame::XController >    xController = NULL;
    uno::Reference< awt::XWindow >          xComponentWindow = NULL;
    {
        xController = uno::Reference< frame::XController >(
            m_xCC->getServiceManager()->createInstanceWithContext(
            CHART_CONTROLLER_SERVICE_IMPLEMENTATION_NAME,m_xCC )
            , uno::UNO_QUERY );

        
        
        xComponentWindow =
                      uno::Reference< awt::XWindow >( xController, uno::UNO_QUERY );

        if( impl_checkCancel() )
            return sal_False;
    }

    
    if(xController.is()&&xModel.is())
    {
        xModel->connectController(xController);
        xModel->setCurrentController(xController);
        xController->attachModel(xModel);
        if(xFrame.is())
            xFrame->setComponent(xComponentWindow,xController);
        
        
        xController->attachFrame(xFrame);
    }

    
    if(!bHaveLoadedModel)
        try
        {
            utl::MediaDescriptor::const_iterator aIt( aMediaDescriptor.find( aMediaDescriptor.PROP_URL()));
            if( aIt != aMediaDescriptor.end())
            {
                OUString aURL( (*aIt).second.get< OUString >());
                if( aURL.startsWith( "private:factory/schart" ) )
                {
                    
                    uno::Reference< frame::XLoadable > xLoadable( xModel, uno::UNO_QUERY_THROW );
                    xLoadable->initNew();
                }
                else
                {
                    aMediaDescriptor.addInputStream();
                    uno::Sequence< beans::PropertyValue > aCompleteMediaDescriptor;
                    aMediaDescriptor >> aCompleteMediaDescriptor;
                    apphelper::MediaDescriptorHelper aMDHelper( aCompleteMediaDescriptor );

                    
                    
                    uno::Reference< frame::XLoadable > xLoadable( xModel, uno::UNO_QUERY_THROW );
                    xLoadable->load( aCompleteMediaDescriptor );

                    
                    if( xComponentWindow.is() && aMDHelper.ISSET_FilterName && aMDHelper.FilterName.equals( "StarChart 5.0") )
                    {
                        awt::Rectangle aRect( xComponentWindow->getPosSize() );
                        sal_Int16 nFlags=0;
                        xComponentWindow->setPosSize( aRect.X, aRect.Y, aRect.Width, aRect.Height, nFlags );
                    }
                }
            }
        }
        catch( const uno::Exception & ex )
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

} 

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
