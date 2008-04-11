/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: TaskPanelFactory.cxx,v $
 *
 * $Revision: 1.3 $
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
#include "TaskPaneViewShell.hxx"
#include "DrawController.hxx"
#include "framework/FrameworkHelper.hxx"
#include <cppuhelper/compbase1.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::drawing::framework;

using ::rtl::OUString;
using ::sd::framework::FrameworkHelper;
using ::sd::toolpanel::TaskPaneViewShell;

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
        OUString::createFromAscii("com.sun.star.drawing.framework.TaskPanelFactory"));
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
        const Reference<XResourceId>& rxResourceId,
        const TaskPaneViewShell::PanelId ePaneId);
    virtual ~TaskPanelResource (void);

    virtual void SAL_CALL disposing (void);

    const TaskPaneViewShell::PanelId GetPaneId (void) const;

    // XResource

    virtual Reference<XResourceId> SAL_CALL getResourceId (void)
        throw (css::uno::RuntimeException);

    virtual sal_Bool SAL_CALL isAnchorOnly (void) throw (RuntimeException)
    { return false; }

private:
    const Reference<XResourceId> mxResourceId;
    const TaskPaneViewShell::PanelId mePaneId;
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
            xCC->addResourceFactory(FrameworkHelper::msCustomAnimationTaskPanelURL, this);
            xCC->addResourceFactory(FrameworkHelper::msSlideTransitionTaskPanelURL, this);
        }
        catch (RuntimeException&)
        {
        }
    }
}


//===== XResourceController ===================================================

Reference<XResource> SAL_CALL TaskPanelFactory::createResource (
    const Reference<XResourceId>& rxResourceId)
    throw (RuntimeException)
{
    Reference<XResource> xResource;

    if ( ! rxResourceId.is())
        return NULL;

    OUString sResourceURL (rxResourceId->getResourceURL());

    if (sResourceURL.match(FrameworkHelper::msTaskPanelURLPrefix))
    {
        TaskPaneViewShell::PanelId ePaneId (TaskPaneViewShell::PID_UNKNOWN);

        if (sResourceURL.equals(FrameworkHelper::msMasterPagesTaskPanelURL))
        {
            ePaneId = TaskPaneViewShell::PID_MASTER_PAGES;
        }
        else if (sResourceURL.equals(FrameworkHelper::msLayoutTaskPanelURL))
        {
            ePaneId = TaskPaneViewShell::PID_LAYOUT;
        }
        else if (sResourceURL.equals(FrameworkHelper::msCustomAnimationTaskPanelURL))
        {
            ePaneId = TaskPaneViewShell::PID_CUSTOM_ANIMATION;
        }
        else if (sResourceURL.equals(FrameworkHelper::msSlideTransitionTaskPanelURL))
        {
            ePaneId = TaskPaneViewShell::PID_SLIDE_TRANSITION;
        }

        if (ePaneId!=TaskPaneViewShell::PID_UNKNOWN && mpViewShellBase!=NULL)
        {
            toolpanel::TaskPaneViewShell* pTaskPane
                = dynamic_cast<toolpanel::TaskPaneViewShell*>(
                    FrameworkHelper::Instance(*mpViewShellBase)
                        ->GetViewShell(FrameworkHelper::msRightPaneURL).get());
            if (pTaskPane != NULL)
            {
                xResource = new TaskPanelResource(
                    rxResourceId,
                    ePaneId);
                pTaskPane->ShowPanel(ePaneId);
                pTaskPane->ExpandPanel(ePaneId);
            }
        }
    }

    return xResource;
}




void SAL_CALL TaskPanelFactory::releaseResource (
    const Reference<XResource>& rxResource)
    throw (RuntimeException)
{
    toolpanel::TaskPaneViewShell* pTaskPane
        = dynamic_cast<toolpanel::TaskPaneViewShell*>(
            FrameworkHelper::Instance(*mpViewShellBase)
                ->GetViewShell(FrameworkHelper::msRightPaneURL).get());

    rtl::Reference<TaskPanelResource> pResource = dynamic_cast<TaskPanelResource*>(
        rxResource.get());

    if (pTaskPane != NULL && pResource.is())
        pTaskPane->CollapsePanel(pResource->GetPaneId());

    Reference<XComponent> xComponent (rxResource, UNO_QUERY);
    if (xComponent.is())
        xComponent->dispose();
}




//=============================================================================

void TaskPanelFactory::ThrowIfDisposed (void) const
    throw (lang::DisposedException)
{
    if (rBHelper.bDisposed || rBHelper.bInDispose)
    {
        throw lang::DisposedException (
            ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
                "TaskPanelFactory object has already been disposed")),
            const_cast<uno::XWeak*>(static_cast<const uno::XWeak*>(this)));
    }
}




//===== ToolPanelResource =====================================================

namespace {

TaskPanelResource::TaskPanelResource (
    const Reference<XResourceId>& rxResourceId,
    const TaskPaneViewShell::PanelId ePaneId)
    : TaskPanelResourceInterfaceBase(m_aMutex),
      mxResourceId(rxResourceId),
      mePaneId(ePaneId)
{
}




TaskPanelResource::~TaskPanelResource (void)
{
}




void SAL_CALL TaskPanelResource::disposing (void)
{
}




const TaskPaneViewShell::PanelId TaskPanelResource::GetPaneId (void) const
{
    return mePaneId;
}




Reference<XResourceId> SAL_CALL TaskPanelResource::getResourceId (void)
    throw (css::uno::RuntimeException)
{
    return mxResourceId;
}

} // end of anonymous namespace

} } // end of namespace sd::framework
