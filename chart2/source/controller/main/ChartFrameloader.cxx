/*************************************************************************
 *
 *  $RCSfile: ChartFrameloader.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: bm $ $Date: 2003-10-06 09:58:28 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2003 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#include "ChartFrameloader.hxx"
#include "servicenames.hxx"
#include "MediaDescriptorHelper.hxx"

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

    //create mediadescriptor to be attached to the model
    apphelper::MediaDescriptorHelper aMediaDescriptorHelper(rMediaDescriptor);

    //prepare data for model  ... missing interface XPersist or XLoadable at Model....
    {
        //@todo ...
    }

    /*
    static uno::Reference< frame::XFrame > xTestFrame = uno::Reference< frame::XFrame >(
                m_xCC->getServiceManager()->createInstanceWithContext(
                ::rtl::OUString::createFromAscii("com.sun.star.frame.Frame"), m_xCC )
                , uno::UNO_QUERY );

    static uno::Reference< awt::XWindow > xTestWindow = uno::Reference< awt::XWindow >(
                m_xCC->getServiceManager()->createInstanceWithContext(
                ::rtl::OUString::createFromAscii("com.sun.star.awt.Window"), m_xCC )
                , uno::UNO_QUERY );
    xTestFrame->initialize(xTestWindow);
    */

    //create and initialize the model
    uno::Reference< frame::XModel >         xModel = NULL;
    {
        //@todo?? load mechanism to cancel during loading of document
        xModel = uno::Reference< frame::XModel >(
                m_xCC->getServiceManager()->createInstanceWithContext(
                CHART_MODEL_SERVICE_IMPLEMENTATION_NAME, m_xCC )
                , uno::UNO_QUERY );

        if( impl_checkCancel() )
            return sal_False;

        //@todo load data to model ... missing interface XPersist or XLoadable at Model....
        xModel->attachResource( aMediaDescriptorHelper.URL.Complete, aMediaDescriptorHelper.getReducedForModel() );
    }

    //create the component context for the controller
    uno::Reference< uno::XComponentContext > xComponentContext_ForController = NULL;
    {
        xComponentContext_ForController = uno::Reference< uno::XComponentContext >(
                m_xCC->getServiceManager()->createInstanceWithContext(
                ::rtl::OUString::createFromAscii("com.sun.star.uno.ComponentContext"), m_xCC )
                , uno::UNO_QUERY );

        //@todo
        if( impl_checkCancel() )
            return sal_False;
    }

    //create the controller(+XWindow)
    uno::Reference< frame::XController >    xController = NULL;
    uno::Reference< awt::XWindow >          xComponentWindow = NULL;
    {
        xController = uno::Reference< frame::XController >(
            m_xCC->getServiceManager()->createInstanceWithContext(
            CHART_CONTROLLER_SERVICE_IMPLEMENTATION_NAME,m_xCC ) //, xComponentContext_ForController )
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
        xController->attachFrame(xFrame);
        xController->attachModel(xModel);
        xModel->connectController(xController);
        if(xFrame.is())
            xFrame->setComponent(xComponentWindow,xController);
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
