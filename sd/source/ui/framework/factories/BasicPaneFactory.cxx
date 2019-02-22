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

#include <memory>
#include <sal/config.h>

#include <utility>

#include "BasicPaneFactory.hxx"
#include <facreg.hxx>

#include "ChildWindowPane.hxx"
#include "FrameWindowPane.hxx"
#include "FullScreenPane.hxx"

#include <framework/FrameworkHelper.hxx>
#include <PaneShells.hxx>
#include <ViewShellBase.hxx>
#include <PaneChildWindows.hxx>
#include <DrawController.hxx>
#include <DrawDocShell.hxx>
#include <com/sun/star/drawing/framework/XControllerManager.hpp>

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

    bool CompareURL(const OUString& rsPaneURL) const { return msPaneURL == rsPaneURL; }
    bool ComparePane(const Reference<XResource>& rxPane) const { return mxPane == rxPane; }
};

class BasicPaneFactory::PaneContainer
    : public ::std::vector<PaneDescriptor>
{
public:
    PaneContainer() {}
};

//===== PaneFactory ===========================================================

BasicPaneFactory::BasicPaneFactory (
    const Reference<XComponentContext>& rxContext)
    : BasicPaneFactoryInterfaceBase(m_aMutex),
      mxComponentContext(rxContext),
      mxConfigurationControllerWeak(),
      mpViewShellBase(nullptr),
      mpPaneContainer(new PaneContainer)
{
}

BasicPaneFactory::~BasicPaneFactory()
{
}

void SAL_CALL BasicPaneFactory::disposing()
{
    Reference<XConfigurationController> xCC (mxConfigurationControllerWeak);
    if (xCC.is())
    {
        xCC->removeResourceFactoryForReference(this);
        xCC->removeConfigurationChangeListener(this);
        mxConfigurationControllerWeak.clear();
    }

    for (const auto& rDescriptor : *mpPaneContainer)
    {
        if (rDescriptor.mbIsReleased)
        {
            Reference<XComponent> xComponent (rDescriptor.mxPane, UNO_QUERY);
            if (xComponent.is())
            {
                xComponent->removeEventListener(this);
                xComponent->dispose();
            }
        }
    }
}

void SAL_CALL BasicPaneFactory::initialize (const Sequence<Any>& aArguments)
{
    if (aArguments.getLength() <= 0)
        return;

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
            mpPaneContainer->push_back(aDescriptor);
            xCC->addResourceFactory(aDescriptor.msPaneURL, this);

            aDescriptor.msPaneURL = FrameworkHelper::msFullScreenPaneURL;
            aDescriptor.mePaneId = FullScreenPaneId;
            mpPaneContainer->push_back(aDescriptor);
            xCC->addResourceFactory(aDescriptor.msPaneURL, this);

            aDescriptor.msPaneURL = FrameworkHelper::msLeftImpressPaneURL;
            aDescriptor.mePaneId = LeftImpressPaneId;
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

//===== XPaneFactory ==========================================================

Reference<XResource> SAL_CALL BasicPaneFactory::createResource (
    const Reference<XResourceId>& rxPaneId)
{
    ThrowIfDisposed();

    Reference<XResource> xPane;

    // Based on the ResourceURL of the given ResourceId look up the
    // corresponding factory descriptor.
    PaneContainer::iterator iDescriptor (
        ::std::find_if (
            mpPaneContainer->begin(),
            mpPaneContainer->end(),
            [&] (PaneDescriptor const& rPane) {
                return rPane.CompareURL(rxPaneId->getResourceURL());
            } ));

    if (iDescriptor == mpPaneContainer->end())
    {
        // The requested pane can not be created by any of the factories
        // managed by the called BasicPaneFactory object.
        throw lang::IllegalArgumentException("BasicPaneFactory::createPane() called for unknown resource id",
            nullptr,
            0);
    }

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


    return xPane;
}

void SAL_CALL BasicPaneFactory::releaseResource (
    const Reference<XResource>& rxPane)
{
    ThrowIfDisposed();

    // Based on the given XPane reference look up the corresponding factory
    // descriptor.
    PaneContainer::iterator iDescriptor (
        ::std::find_if(
            mpPaneContainer->begin(),
            mpPaneContainer->end(),
            [&] (PaneDescriptor const& rPane) { return rPane.ComparePane(rxPane); } ));

    if (iDescriptor == mpPaneContainer->end())
    {
        // The given XPane reference is either empty or the pane was not
        // created by any of the factories managed by the called
        // BasicPaneFactory object.
        throw lang::IllegalArgumentException("BasicPaneFactory::releasePane() called for pane that was not created by same factory.",
            nullptr,
            0);
    }

    // The given pane was created by one of the factories.  Child
    // windows are just hidden and will be reused when requested later.
    // Other windows are disposed and their reference is reset so that
    // on the next createPane() call for the same pane type the pane is
    // created anew.
    ChildWindowPane* pChildWindowPane = dynamic_cast<ChildWindowPane*>(rxPane.get());
    if (pChildWindowPane != nullptr)
    {
        iDescriptor->mbIsReleased = true;
        pChildWindowPane->Hide();
    }
    else
    {
        iDescriptor->mxPane = nullptr;
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

//===== XConfigurationChangeListener ==========================================

void SAL_CALL BasicPaneFactory::notifyConfigurationChange (
    const ConfigurationChangeEvent& /* rEvent */ )
{
    // FIXME: nothing to do
}

//===== lang::XEventListener ==================================================

void SAL_CALL BasicPaneFactory::disposing (
    const lang::EventObject& rEventObject)
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
                [&] (PaneDescriptor const& rPane) { return rPane.ComparePane(xPane); } ));
        if (iDescriptor != mpPaneContainer->end())
        {
            iDescriptor->mxPane = nullptr;
        }
    }
}

Reference<XResource> BasicPaneFactory::CreateFrameWindowPane (
    const Reference<XResourceId>& rxPaneId)
{
    Reference<XResource> xPane;

    if (mpViewShellBase != nullptr)
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

    if (mpViewShellBase != nullptr)
    {
        // Create the corresponding shell and determine the id of the child window.
        sal_uInt16 nChildWindowId = 0;
        ::std::unique_ptr<SfxShell> pShell;
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
        if (pShell != nullptr)
        {
            xPane = new ChildWindowPane(
                rxPaneId,
                nChildWindowId,
                *mpViewShellBase,
                std::move(pShell));
        }
    }

    return xPane;
}

void BasicPaneFactory::ThrowIfDisposed() const
{
    if (rBHelper.bDisposed || rBHelper.bInDispose)
    {
        throw lang::DisposedException ("BasicPaneFactory object has already been disposed",
            const_cast<uno::XWeak*>(static_cast<const uno::XWeak*>(this)));
    }
}

} } // end of namespace sd::framework


extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
com_sun_star_comp_Draw_framework_BasicPaneFactory_get_implementation(css::uno::XComponentContext* context,
                                                                     css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new sd::framework::BasicPaneFactory(context));
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
