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

#include "precompiled_sd.hxx"
#include "TaskPanelFactory.hxx"
#include "taskpane/ToolPanelViewShell.hxx"
#include "DrawController.hxx"
#include "framework/FrameworkHelper.hxx"
#include <cppuhelper/compbase1.hxx>
#include <tools/diagnose_ex.h>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::drawing::framework;

using ::rtl::OUString;
using ::sd::framework::FrameworkHelper;

namespace sd { namespace framework {

Reference<XInterface> SAL_CALL TaskPanelFactory_createInstance (
    const Reference<XComponentContext>& rxContext)
{
    return Reference<XInterface>(static_cast<XWeak*>(new TaskPanelFactory(rxContext)));
}




::rtl::OUString TaskPanelFactory_getImplementationName (void) throw(RuntimeException)
{
    return ::rtl::OUString(
        RTL_CONSTASCII_USTRINGPARAM("com.sun.star.comp.Draw.framework.TaskPanelFactory"));
}




Sequence<rtl::OUString> SAL_CALL TaskPanelFactory_getSupportedServiceNames (void)
    throw (RuntimeException)
{
    static const OUString sServiceName(
        RTL_CONSTASCII_USTRINGPARAM("com.sun.star.drawing.framework.TaskPanelFactory"));
    return Sequence<rtl::OUString>(&sServiceName, 1);
}




//===== ToolPanelResource =====================================================

namespace {

typedef ::cppu::WeakComponentImplHelper1 <
    css::drawing::framework::XResource
    > TaskPanelResourceInterfaceBase;

class TaskPanelResource
    : private ::cppu::BaseMutex,
      public TaskPanelResourceInterfaceBase
{
public:
    TaskPanelResource (
        const Reference<XResourceId>& rxResourceId );
    virtual ~TaskPanelResource ();

    virtual void SAL_CALL disposing ();

    // XResource

    virtual Reference<XResourceId> SAL_CALL getResourceId (void)
        throw (css::uno::RuntimeException);

    virtual sal_Bool SAL_CALL isAnchorOnly () throw (RuntimeException)
    { return false; }

private:
    const Reference<XResourceId> mxResourceId;
};

} // end of anonymous namespace.




//===== TaskPanelFactory =======================================================

TaskPanelFactory::TaskPanelFactory (
    const ::com::sun::star::uno::Reference<com::sun::star::uno::XComponentContext>& rxContext)
    : TaskPanelFactoryInterfaceBase(m_aMutex),
      mpViewShellBase(NULL)
{
    (void)rxContext;
}




TaskPanelFactory::~TaskPanelFactory (void)
{
}




void SAL_CALL TaskPanelFactory::disposing (void)
{
}




//===== XInitialization =======================================================

void SAL_CALL TaskPanelFactory::initialize(
    const ::com::sun::star::uno::Sequence<com::sun::star::uno::Any>& aArguments)
    throw (::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException)
{
    if (aArguments.getLength() > 0)
    {
        try
        {
            // Get the XController from the first argument.
            Reference<frame::XController> xController (aArguments[0], UNO_QUERY_THROW);

            // Tunnel through the controller to obtain access to the ViewShellBase.
            try
            {
                Reference<lang::XUnoTunnel> xTunnel (xController, UNO_QUERY_THROW);
                DrawController* pController
                    = reinterpret_cast<DrawController*>(
                        sal::static_int_cast<sal_uIntPtr>(
                            xTunnel->getSomething(DrawController::getUnoTunnelId())));
                if (pController != NULL)
                    mpViewShellBase = pController->GetViewShellBase();

            }
            catch(RuntimeException&)
            {}


            Reference<XControllerManager> xCM (xController, UNO_QUERY_THROW);
            Reference<XConfigurationController> xCC (
                xCM->getConfigurationController(), UNO_QUERY_THROW);
            xCC->addResourceFactory(FrameworkHelper::msMasterPagesTaskPanelURL, this);
            xCC->addResourceFactory(FrameworkHelper::msLayoutTaskPanelURL, this);
            xCC->addResourceFactory(FrameworkHelper::msTableDesignPanelURL, this);
            xCC->addResourceFactory(FrameworkHelper::msCustomAnimationTaskPanelURL, this);
            xCC->addResourceFactory(FrameworkHelper::msSlideTransitionTaskPanelURL, this);
        }
        catch (RuntimeException&)
        {
        }
    }
}


//===== XResourceController ===================================================

namespace
{
    void lcl_collectResourceURLs( const Reference< XResourceId >& i_rResourceId, ::std::vector< ::rtl::OUString >& o_rResourceURLs )
    {
        ENSURE_OR_RETURN_VOID( i_rResourceId.is(), "illegal resource ID" );
        o_rResourceURLs.resize(0);

        Reference< XResourceId > xResourceId( i_rResourceId );
        ::rtl::OUString sResourceURL = xResourceId->getResourceURL();
        while ( sResourceURL.getLength() > 0 )
        {
            o_rResourceURLs.push_back( sResourceURL );
            xResourceId = xResourceId->getAnchor();
            sResourceURL = xResourceId->getResourceURL();
        }
    }
}

Reference<XResource> SAL_CALL TaskPanelFactory::createResource (
    const Reference<XResourceId>& rxResourceId)
    throw (RuntimeException, IllegalArgumentException, WrappedTargetException)
{
    Reference<XResource> xResource;

    if ( ! rxResourceId.is())
        return NULL;

    OUString sResourceURL (rxResourceId->getResourceURL());

    if ( sResourceURL.match( FrameworkHelper::msTaskPanelURLPrefix ) )
    {
        toolpanel::PanelId ePanelId( toolpanel::GetStandardPanelId( sResourceURL ) );

        if ( ( ePanelId != toolpanel::PID_UNKNOWN ) && ( mpViewShellBase != NULL ) )
        {
            ::boost::shared_ptr< FrameworkHelper > pFrameworkHelper( FrameworkHelper::Instance( *mpViewShellBase ) );

            // assume that the top-level anchor is the URL of the pane
            ::std::vector< ::rtl::OUString > aResourceURLs;
            lcl_collectResourceURLs( rxResourceId, aResourceURLs );

            const ::rtl::OUString sPaneURL = aResourceURLs[ aResourceURLs.size() - 1 ];
            const ::boost::shared_ptr< ViewShell > pPaneViewShell( pFrameworkHelper->GetViewShell( sPaneURL ) );

            toolpanel::ToolPanelViewShell* pToolPanel = dynamic_cast< toolpanel::ToolPanelViewShell* >( pPaneViewShell.get() );
            if ( pToolPanel != NULL )
                xResource = new TaskPanelResource( rxResourceId );

            OSL_POSTCOND( xResource.is(), "TaskPanelFactory::createResource: did not find the given resource!" );
        }
    }

    return xResource;
}




void SAL_CALL TaskPanelFactory::releaseResource (
    const Reference<XResource>& rxResource)
    throw (RuntimeException)
{
    ENSURE_OR_RETURN_VOID( rxResource.is(), "illegal resource" );
    const Reference< XResourceId > xResourceId( rxResource->getResourceId(), UNO_SET_THROW );

    // assume that the top-level anchor is the URL of the pane
    ::std::vector< ::rtl::OUString > aResourceURLs;
    lcl_collectResourceURLs( xResourceId, aResourceURLs );

    OSL_ENSURE( !aResourceURLs.empty(), "TaskPanelFactory::releaseResource: illegal resource/URL!" );
    if ( !aResourceURLs.empty() )
    {
        const ::rtl::OUString sPaneURL = aResourceURLs[ aResourceURLs.size() - 1 ];
        ::boost::shared_ptr< FrameworkHelper > pFrameworkHelper( FrameworkHelper::Instance( *mpViewShellBase ) );
        const ::boost::shared_ptr< ViewShell > pPaneViewShell( pFrameworkHelper->GetViewShell( sPaneURL ) );
        if ( pPaneViewShell != NULL )
        {
            const ::rtl::OUString sPanelResourceURL( xResourceId->getResourceURL() );
            const toolpanel::PanelId ePanelId( toolpanel::GetStandardPanelId( sPanelResourceURL ) );
            toolpanel::ToolPanelViewShell* pToolPanel = dynamic_cast< toolpanel::ToolPanelViewShell* >( pPaneViewShell.get() );

            if  (   ( ePanelId != toolpanel::PID_UNKNOWN )
                &&  ( pToolPanel != NULL )
                )
            {
                pToolPanel->DeactivatePanel( sPanelResourceURL );
            }
            else
            {
                OSL_FAIL( "TaskPanelFactory::releaseResource: don't know what to do with this resource!" );
            }
        }
    }

    Reference<XComponent> xComponent (rxResource, UNO_QUERY);
    if (xComponent.is())
        xComponent->dispose();
}




//===== ToolPanelResource =====================================================

namespace {

TaskPanelResource::TaskPanelResource (
    const Reference<XResourceId>& rxResourceId)
    : TaskPanelResourceInterfaceBase(m_aMutex),
      mxResourceId(rxResourceId)
{
}




TaskPanelResource::~TaskPanelResource (void)
{
}




void SAL_CALL TaskPanelResource::disposing ()
{
}




Reference<XResourceId> SAL_CALL TaskPanelResource::getResourceId ()
    throw (css::uno::RuntimeException)
{
    return mxResourceId;
}

} // end of anonymous namespace

} } // end of namespace sd::framework

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
