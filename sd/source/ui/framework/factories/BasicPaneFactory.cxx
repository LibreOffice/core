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


#include "BasicPaneFactory.hxx"

#include "ChildWindowPane.hxx"
#include "FrameWindowPane.hxx"
#include "FullScreenPane.hxx"

#include "framework/FrameworkHelper.hxx"
#include "ViewShellBase.hxx"
#include "PaneChildWindows.hxx"
#include "DrawController.hxx"
#include "DrawDocShell.hxx"
#include <com/sun/star/drawing/framework/XControllerManager.hpp>
#include <boost/bind.hpp>


using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::drawing::framework;

using ::sd::framework::FrameworkHelper;

namespace {
    enum PaneId {
        CenterPaneId,
        FullScreenPaneId,
        LeftImpressPaneId,
        LeftDrawPaneId
    };

    static const sal_Int32 gnConfigurationUpdateStartEvent(0);
    static const sal_Int32 gnConfigurationUpdateEndEvent(1);
}

namespace sd { namespace framework {


/** Store URL, XPane reference and (local) PaneId for every pane factory
    that is registered at the PaneController.
*/
class BasicPaneFactory::PaneDescriptor
{
public:
    OUString msPaneURL;
    Reference<XResource> mxPane;
    PaneId mePaneId;
    /** The mbReleased flag is set when the pane has been released.  Some
        panes are just hidden and destroyed.  When the pane is reused this
        flag is reset.
    */
    bool mbIsReleased;
    bool mbIsChildWindow;

    bool CompareURL (const OUString& rsPaneURL) { return msPaneURL.equals(rsPaneURL); }
    bool ComparePane (const Reference<XResource>& rxPane) { return mxPane==rxPane; }
};


class BasicPaneFactory::PaneContainer
    : public ::std::vector<PaneDescriptor>
{
public:
    PaneContainer (void) {}
};



Reference<XInterface> SAL_CALL BasicPaneFactory_createInstance (
    const Reference<XComponentContext>& rxContext)
{
    return Reference<XInterface>(static_cast<XWeak*>(new BasicPaneFactory(rxContext)));
}




OUString BasicPaneFactory_getImplementationName (void) throw(RuntimeException)
{
    return OUString("com.sun.star.comp.Draw.framework.BasicPaneFactory");
}




Sequence<OUString> SAL_CALL BasicPaneFactory_getSupportedServiceNames (void)
    throw (RuntimeException)
{
    static const OUString sServiceName("com.sun.star.drawing.framework.BasicPaneFactory");
    return Sequence<OUString>(&sServiceName, 1);
}




//===== PaneFactory ===========================================================

BasicPaneFactory::BasicPaneFactory (
    const Reference<XComponentContext>& rxContext)
    : BasicPaneFactoryInterfaceBase(m_aMutex),
      mxComponentContext(rxContext),
      mxConfigurationControllerWeak(),
      mpViewShellBase(NULL),
      mpPaneContainer(new PaneContainer)
{
}





BasicPaneFactory::~BasicPaneFactory (void)
{
}




void SAL_CALL BasicPaneFactory::disposing (void)
{
    Reference<XConfigurationController> xCC (mxConfigurationControllerWeak);
    if (xCC.is())
    {
        xCC->removeResourceFactoryForReference(this);
        xCC->removeConfigurationChangeListener(this);
        mxConfigurationControllerWeak.clear();
    }

    for (PaneContainer::const_iterator iDescriptor = mpPaneContainer->begin();
         iDescriptor != mpPaneContainer->end();
         ++iDescriptor)
    {
        if (iDescriptor->mbIsReleased)
        {
            Reference<XComponent> xComponent (iDescriptor->mxPane, UNO_QUERY);
            if (xComponent.is())
            {
                xComponent->removeEventListener(this);
                xComponent->dispose();
            }
        }
    }
}




void SAL_CALL BasicPaneFactory::initialize (const Sequence<Any>& aArguments)
    throw (Exception, RuntimeException)
{
    if (aArguments.getLength() > 0)
    {
        try
        {
            // Get the XController from the first argument.
            Reference<frame::XController> xController (aArguments[0], UNO_QUERY_THROW);
            mxControllerWeak = xController;

            // Tunnel through the controller to obtain access to the ViewShellBase.
            try
            {
                Reference<lang::XUnoTunnel> xTunnel (xController, UNO_QUERY_THROW);
                DrawController* pController
                    = reinterpret_cast<DrawController*>(
                        (sal::static_int_cast<sal_uIntPtr>(
                            xTunnel->getSomething(DrawController::getUnoTunnelId()))));
                mpViewShellBase = pController->GetViewShellBase();
            }
            catch(RuntimeException&)
            {}

            Reference<XControllerManager> xCM (xController, UNO_QUERY_THROW);
            Reference<XConfigurationController> xCC (xCM->getConfigurationController());
            mxConfigurationControllerWeak = xCC;

            // Add pane factories for the two left panes (one for Impress and one for
            // Draw) and the center pane.
            if (xController.is() && xCC.is())
            {
                PaneDescriptor aDescriptor;
                aDescriptor.msPaneURL = FrameworkHelper::msCenterPaneURL;
                aDescriptor.mePaneId = CenterPaneId;
                aDescriptor.mbIsReleased = false;
                aDescriptor.mbIsChildWindow = false;
                mpPaneContainer->push_back(aDescriptor);
                xCC->addResourceFactory(aDescriptor.msPaneURL, this);

                aDescriptor.msPaneURL = FrameworkHelper::msFullScreenPaneURL;
                aDescriptor.mePaneId = FullScreenPaneId;
                mpPaneContainer->push_back(aDescriptor);
                xCC->addResourceFactory(aDescriptor.msPaneURL, this);

                aDescriptor.msPaneURL = FrameworkHelper::msLeftImpressPaneURL;
                aDescriptor.mePaneId = LeftImpressPaneId;
                aDescriptor.mbIsChildWindow = true;
                mpPaneContainer->push_back(aDescriptor);
                xCC->addResourceFactory(aDescriptor.msPaneURL, this);

                aDescriptor.msPaneURL = FrameworkHelper::msLeftDrawPaneURL;
                aDescriptor.mePaneId = LeftDrawPaneId;
                mpPaneContainer->push_back(aDescriptor);
                xCC->addResourceFactory(aDescriptor.msPaneURL, this);
            }

            // Register as configuration change listener.
            if (xCC.is())
            {
                xCC->addConfigurationChangeListener(
                    this,
                    FrameworkHelper::msConfigurationUpdateStartEvent,
                    makeAny(gnConfigurationUpdateStartEvent));
                xCC->addConfigurationChangeListener(
                    this,
                    FrameworkHelper::msConfigurationUpdateEndEvent,
                    makeAny(gnConfigurationUpdateEndEvent));
            }
        }
        catch (RuntimeException&)
        {
            Reference<XConfigurationController> xCC (mxConfigurationControllerWeak);
            if (xCC.is())
                xCC->removeResourceFactoryForReference(this);
        }
    }
}




//===== XPaneFactory ==========================================================

Reference<XResource> SAL_CALL BasicPaneFactory::createResource (
    const Reference<XResourceId>& rxPaneId)
    throw (RuntimeException, IllegalArgumentException, WrappedTargetException)
{
    ThrowIfDisposed();

    Reference<XResource> xPane;

    // Based on the ResourceURL of the given ResourceId look up the
    // corresponding factory descriptor.
    PaneContainer::iterator iDescriptor (
        ::std::find_if (
            mpPaneContainer->begin(),
            mpPaneContainer->end(),
            ::boost::bind(&PaneDescriptor::CompareURL, _1, rxPaneId->getResourceURL())));

    if (iDescriptor != mpPaneContainer->end())
    {
        if (iDescriptor->mxPane.is())
        {
            // The pane has already been created and is still active (has
            // not yet been released).  This should not happen.
            xPane = iDescriptor->mxPane;
        }
        else
        {
            // Create a new pane.
            switch (iDescriptor->mePaneId)
            {
                case CenterPaneId:
                    xPane = CreateFrameWindowPane(rxPaneId);
                    break;

                case FullScreenPaneId:
                    xPane = CreateFullScreenPane(mxComponentContext, rxPaneId);
                    break;

                case LeftImpressPaneId:
                case LeftDrawPaneId:
                    xPane = CreateChildWindowPane(
                        rxPaneId,
                        *iDescriptor);
                    break;
            }
            iDescriptor->mxPane = xPane;

            // Listen for the pane being disposed.
            Reference<lang::XComponent> xComponent (xPane, UNO_QUERY);
            if (xComponent.is())
                xComponent->addEventListener(this);
        }
        iDescriptor->mbIsReleased = false;
    }
    else
    {
        // The requested pane can not be created by any of the factories
        // managed by the called BasicPaneFactory object.
        throw lang::IllegalArgumentException("BasicPaneFactory::createPane() called for unknown resource id",
            NULL,
            0);
    }

    return xPane;
}





void SAL_CALL BasicPaneFactory::releaseResource (
    const Reference<XResource>& rxPane)
    throw (RuntimeException)
{
    ThrowIfDisposed();

    // Based on the given XPane reference look up the corresponding factory
    // descriptor.
    PaneContainer::iterator iDescriptor (
        ::std::find_if(
            mpPaneContainer->begin(),
            mpPaneContainer->end(),
            ::boost::bind(&PaneDescriptor::ComparePane, _1, rxPane)));

    if (iDescriptor != mpPaneContainer->end())
    {
        // The given pane was created by one of the factories.  Child
        // windows are just hidden and will be reused when requested later.
        // Other windows are disposed and their reference is reset so that
        // on the next createPane() call for the same pane type the pane is
        // created anew.
        ChildWindowPane* pChildWindowPane = dynamic_cast<ChildWindowPane*>(rxPane.get());
        if (pChildWindowPane != NULL)
        {
            iDescriptor->mbIsReleased = true;
            pChildWindowPane->Hide();
        }
        else
        {
            iDescriptor->mxPane = NULL;
            Reference<XComponent> xComponent (rxPane, UNO_QUERY);
            if (xComponent.is())
            {
                // We are disposing the pane and do not have to be informed of
                // that.
                xComponent->removeEventListener(this);
                xComponent->dispose();
            }
        }
    }
    else
    {
        // The given XPane reference is either empty or the pane was not
        // created by any of the factories managed by the called
        // BasicPaneFactory object.
        throw lang::IllegalArgumentException("BasicPaneFactory::releasePane() called for pane that that was not created by same factory.",
            NULL,
            0);
    }
}




//===== XConfigurationChangeListener ==========================================

void SAL_CALL BasicPaneFactory::notifyConfigurationChange (
    const ConfigurationChangeEvent& /* rEvent */ )
    throw (RuntimeException)
{
	// FIXME: nothing to do
}




//===== lang::XEventListener ==================================================

void SAL_CALL BasicPaneFactory::disposing (
    const lang::EventObject& rEventObject)
    throw (RuntimeException)
{
    if (mxConfigurationControllerWeak == rEventObject.Source)
    {
        mxConfigurationControllerWeak.clear();
    }
    else
    {
        // Has one of the panes been disposed?  If so, then release the
        // reference to that pane, but not the pane descriptor.
        Reference<XResource> xPane (rEventObject.Source, UNO_QUERY);
        PaneContainer::iterator iDescriptor (
            ::std::find_if (
                mpPaneContainer->begin(),
                mpPaneContainer->end(),
                ::boost::bind(&PaneDescriptor::ComparePane, _1, xPane)));
        if (iDescriptor != mpPaneContainer->end())
        {
            iDescriptor->mxPane = NULL;
        }
    }
}




//-----------------------------------------------------------------------------

Reference<XResource> BasicPaneFactory::CreateFrameWindowPane (
    const Reference<XResourceId>& rxPaneId)
{
    Reference<XResource> xPane;

    if (mpViewShellBase != NULL)
    {
        xPane = new FrameWindowPane(rxPaneId, mpViewShellBase->GetViewWindow());
    }

    return xPane;
}




Reference<XResource> BasicPaneFactory::CreateFullScreenPane (
    const Reference<XComponentContext>& rxComponentContext,
    const Reference<XResourceId>& rxPaneId)
{
    Reference<XResource> xPane (
        new FullScreenPane(
            rxComponentContext,
            rxPaneId,
            mpViewShellBase->GetViewWindow()));

    return xPane;
}




Reference<XResource> BasicPaneFactory::CreateChildWindowPane (
    const Reference<XResourceId>& rxPaneId,
    const PaneDescriptor& rDescriptor)
{
    Reference<XResource> xPane;

    if (mpViewShellBase != NULL)
    {
        // Create the corresponding shell and determine the id of the child window.
        sal_uInt16 nChildWindowId = 0;
        SAL_WNODEPRECATED_DECLARATIONS_PUSH
        ::std::auto_ptr<SfxShell> pShell;
        SAL_WNODEPRECATED_DECLARATIONS_POP
        switch (rDescriptor.mePaneId)
        {
            case LeftImpressPaneId:
                pShell.reset(new LeftImpressPaneShell());
                nChildWindowId = ::sd::LeftPaneImpressChildWindow::GetChildWindowId();
                break;

            case LeftDrawPaneId:
                pShell.reset(new LeftDrawPaneShell());
                nChildWindowId = ::sd::LeftPaneDrawChildWindow::GetChildWindowId();
                break;

            default:
                break;
        }

        // With shell and child window id create the ChildWindowPane
        // wrapper.
        if (pShell.get() != NULL)
        {
            xPane = new ChildWindowPane(
                rxPaneId,
                nChildWindowId,
                *mpViewShellBase,
                pShell);
        }
    }

    return xPane;
}

void BasicPaneFactory::ThrowIfDisposed (void) const
    throw (lang::DisposedException)
{
    if (rBHelper.bDisposed || rBHelper.bInDispose)
    {
        throw lang::DisposedException ("BasicPaneFactory object has already been disposed",
            const_cast<uno::XWeak*>(static_cast<const uno::XWeak*>(this)));
    }
}


} } // end of namespace sd::framework

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
