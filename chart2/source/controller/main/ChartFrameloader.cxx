/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ChartFrameloader.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: vg $ $Date: 2007-05-22 18:05:02 $
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
#include "ChartFrameloader.hxx"
#include "servicenames.hxx"
#include "MediaDescriptorHelper.hxx"
#include "macros.hxx"

#ifndef _COMPHELPER_MEDIADESCRIPTOR_HXX_
#include <comphelper/mediadescriptor.hxx>
#endif

#ifndef _COM_SUN_STAR_DOCUMENT_XIMPORTER_HPP_
#include <com/sun/star/document/XImporter.hpp>
#endif
#ifndef _COM_SUN_STAR_DOCUMENT_XFILTER_HPP_
#include <com/sun/star/document/XFilter.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XLOADABLE_HPP_
#include <com/sun/star/frame/XLoadable.hpp>
#endif

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
