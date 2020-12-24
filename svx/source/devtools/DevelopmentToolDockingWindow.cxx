/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <memory>

#include <svx/devtools/DevelopmentToolDockingWindow.hxx>

#include <com/sun/star/uno/XComponentContext.hpp>

#include <com/sun/star/beans/theIntrospection.hpp>
#include <com/sun/star/beans/XIntrospection.hpp>
#include <com/sun/star/beans/XIntrospectionAccess.hpp>
#include <com/sun/star/beans/Property.hpp>
#include <com/sun/star/beans/PropertyConcept.hpp>
#include <com/sun/star/beans/MethodConcept.hpp>
#include <com/sun/star/reflection/XIdlMethod.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>

#include <comphelper/processfactory.hxx>

#include <sfx2/dispatch.hxx>
#include <sfx2/sfxmodelfactory.hxx>
#include <svx/svxids.hrc>

#include <sfx2/objsh.hxx>

#include <sfx2/viewfrm.hxx>

#include <com/sun/star/frame/XController.hpp>
#include <com/sun/star/view/XSelectionChangeListener.hpp>

#include <cppuhelper/compbase.hxx>
#include <cppuhelper/basemutex.hxx>

#include <com/sun/star/view/XSelectionSupplier.hpp>

using namespace css;

namespace
{
void introspect(uno::Reference<uno::XInterface> const& xInterface)
{
    printf("DevelopmentToolDockingWindow::update\n");

    if (!xInterface.is())
        return;

    uno::Reference<uno::XComponentContext> xContext = comphelper::getProcessComponentContext();
    if (!xContext.is())
        return;

    auto xServiceInfo = uno::Reference<lang::XServiceInfo>(xInterface, uno::UNO_QUERY);
    OUString aImplementationName = xServiceInfo->getImplementationName();
    printf("Implementation name: %s\n", aImplementationName.toUtf8().getStr());

    printf("Service name\n");
    uno::Sequence<OUString> aServiceNames(xServiceInfo->getSupportedServiceNames());
    for (auto const& aServiceName : aServiceNames)
    {
        printf("   %s\n", aServiceName.toUtf8().getStr());
    }

    /*uno::Reference<beans::XIntrospection> xIntrospection;
    xIntrospection = beans::theIntrospection::get(xContext);

    uno::Reference<beans::XIntrospectionAccess> xIntrospectionAccess;
    xIntrospectionAccess = xIntrospection->inspect(uno::makeAny(xInterface));

    printf ("Properties\n");
    auto xProperties = xIntrospectionAccess->getProperties(beans::PropertyConcept::ALL - beans::PropertyConcept::DANGEROUS);
    for (auto const & xProperty : xProperties)
    {
        printf ("   %s\n", xProperty.Name.toUtf8().getStr());
    }

    printf ("Methods\n");
    auto xMethods = xIntrospectionAccess->getMethods(beans::MethodConcept::ALL);
    for (auto const & xMethod : xMethods)
    {
        printf ("   %s\n", xMethod->getName().toUtf8().getStr());
    }*/
}

} // end anonymous namespace

typedef cppu::WeakComponentImplHelper<css::view::XSelectionChangeListener>
    SelectionChangeHandlerInterfaceBase;

class SelectionChangeHandler final : private ::cppu::BaseMutex,
                                     public SelectionChangeHandlerInterfaceBase
{
private:
    css::uno::Reference<css::frame::XController> mxController;

public:
    SelectionChangeHandler(const css::uno::Reference<css::frame::XController>& rxController)
        : SelectionChangeHandlerInterfaceBase(m_aMutex)
        , mxController(rxController)
    {
    }

    virtual ~SelectionChangeHandler() override {}

    virtual void SAL_CALL selectionChanged(const css::lang::EventObject& rEvent) override
    {
        printf("selectionChanged\n");
        uno::Reference<view::XSelectionSupplier> xSupplier(mxController, uno::UNO_QUERY);
        if (xSupplier.is())
        {
            uno::Any aAny = xSupplier->getSelection();
            auto aRef = aAny.get<uno::Reference<uno::XInterface>>();
            introspect(aRef);
        }
    }
    virtual void SAL_CALL disposing(const css::lang::EventObject& rEvent) override {}
    virtual void SAL_CALL disposing() override {}

private:
    SelectionChangeHandler(const SelectionChangeHandler&) = delete;
    SelectionChangeHandler& operator=(const SelectionChangeHandler&) = delete;
};

SFX_IMPL_DOCKINGWINDOW_WITHID(DevelopmentToolChildWindow, SID_DEVELOPMENT_TOOLS_DOCKING_WINDOW);

DevelopmentToolChildWindow::DevelopmentToolChildWindow(vcl::Window* pParentWindow, sal_uInt16 nId,
                                                       SfxBindings* pBindings,
                                                       SfxChildWinInfo* pInfo)
    : SfxChildWindow(pParentWindow, nId)
{
    printf("DevelopmentToolChildWindow\n");
    VclPtr<DevelopmentToolDockingWindow> pWin
        = VclPtr<DevelopmentToolDockingWindow>::Create(pBindings, this, pParentWindow);
    SetWindow(pWin);
    SetAlignment(SfxChildAlignment::BOTTOM);
    pWin->Initialize(pInfo);
}

DevelopmentToolChildWindow::~DevelopmentToolChildWindow() {}

DevelopmentToolDockingWindow::DevelopmentToolDockingWindow(SfxBindings* pInputBindings,
                                                           SfxChildWindow* pChildWindow,
                                                           vcl::Window* pParent)
    : SfxDockingWindow(pInputBindings, pChildWindow, pParent, "DevelopmentTool",
                       "svx/ui/developmenttool.ui")
{
    auto* pViewFrame = pInputBindings->GetDispatcher()->GetFrame();

    uno::Reference<frame::XController> xCtrl = pViewFrame->GetFrame().GetController();

    uno::Reference<view::XSelectionSupplier> xSupplier(xCtrl, uno::UNO_QUERY);
    if (xSupplier.is())
    {
        uno::Reference<view::XSelectionChangeListener> xChangeListener(
            new SelectionChangeHandler(xCtrl));
        xSupplier->addSelectionChangeListener(xChangeListener);
        introspect(pInputBindings->GetDispatcher()->GetFrame()->GetObjectShell()->GetBaseModel());
    }
}

DevelopmentToolDockingWindow::~DevelopmentToolDockingWindow() { disposeOnce(); }

void DevelopmentToolDockingWindow::dispose() { SfxDockingWindow::dispose(); }

void DevelopmentToolDockingWindow::ToggleFloatingMode()
{
    SfxDockingWindow::ToggleFloatingMode();

    if (GetFloatingWindow())
        GetFloatingWindow()->SetMinOutputSizePixel(Size(300, 300));

    Invalidate();
}

void DevelopmentToolDockingWindow::EndDocking(const tools::Rectangle& rReactangle, bool bFloatMode)
{
    SfxDockingWindow::EndDocking(rReactangle, bFloatMode);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
