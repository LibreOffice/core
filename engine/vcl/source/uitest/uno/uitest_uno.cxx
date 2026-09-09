/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <comphelper/compbase.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <cpo/uno/XComponentContext.hpp>
#include <com/sun/star/ui/test/XUITest.hpp>

#include <memory>

#include <vcl/uitest/uitest.hxx>
#include <vcl/svapp.hxx>
#include <vcl/toolkit/unowrap.hxx>
#include <vcl/window.hxx>

#include "uiobject_uno.hxx"

namespace
{
    typedef ::comphelper::WeakComponentImplHelper <
        css::ui::test::XUITest, css::lang::XServiceInfo
        > UITestBase;

class UITestUnoObj : public UITestBase
{
public:

    UITestUnoObj();

    bool executeCommand(const OUString& rCommand) override;

    bool executeCommandWithParameters(const OUString& rCommand,
        const cpo::uno::Sequence< css::beans::PropertyValue >& rArgs) override;

    bool executeCommandForProvider(
        const OUString& rCommand,
        const css::uno::Reference<css::frame::XDispatchProvider>& xProvider) override;

    bool executeDialog(const OUString& rCommand) override;

    css::uno::Reference<css::ui::test::XUIObject> getTopFocusWindow() override;

    css::uno::Reference<css::ui::test::XUIObject> getFocusWindow() override;

    css::uno::Reference<css::ui::test::XUIObject> getFloatWindow() override;

    css::uno::Reference<css::ui::test::XUIObject>
        getWindow(const css::uno::Reference<css::awt::XWindow>& xWindow) override;

    OUString getImplementationName() override;

    bool supportsService(OUString const & ServiceName) override;

    cpo::uno::Sequence<OUString> getSupportedServiceNames() override;
};

}

UITestUnoObj::UITestUnoObj()
{
}

bool UITestUnoObj::executeCommand(const OUString& rCommand)
{
    SolarMutexGuard aGuard;
    return UITest::executeCommand(rCommand);
}

bool UITestUnoObj::executeCommandWithParameters(const OUString& rCommand,
    const cpo::uno::Sequence< css::beans::PropertyValue >& rArgs)
{
    SolarMutexGuard aGuard;
    return UITest::executeCommandWithParameters(rCommand,rArgs);
}

bool UITestUnoObj::executeCommandForProvider(
    const OUString& rCommand,
    const css::uno::Reference<css::frame::XDispatchProvider>& xProvider)
{
    SolarMutexGuard aGuard;
    return UITest::executeCommandForProvider(rCommand, xProvider);
}

bool UITestUnoObj::executeDialog(const OUString& rCommand)
{
    SolarMutexGuard aGuard;
    return UITest::executeDialog(rCommand);
}

css::uno::Reference<css::ui::test::XUIObject> UITestUnoObj::getTopFocusWindow()
{
    SolarMutexGuard aGuard;
    std::unique_ptr<UIObject> pObj = UITest::getFocusTopWindow();
    if (!pObj)
        throw cpo::uno::RuntimeException(u"UITest::getFocusTopWindow did not find a window"_ustr);
    return new UIObjectUnoObj(std::move(pObj));
}

css::uno::Reference<css::ui::test::XUIObject> UITestUnoObj::getFocusWindow()
{
    SolarMutexGuard aGuard;
    std::unique_ptr<UIObject> pObj = UITest::getFocusWindow();
    if (!pObj)
        throw cpo::uno::RuntimeException(u"UITest::getFocusWindow did not find a window"_ustr);
    return new UIObjectUnoObj(std::move(pObj));
}

css::uno::Reference<css::ui::test::XUIObject> UITestUnoObj::getFloatWindow()
{
    SolarMutexGuard aGuard;
    std::unique_ptr<UIObject> pObj = UITest::getFloatWindow();
    if (!pObj)
        throw cpo::uno::RuntimeException(u"UITest::getFloatWindow did not find a window"_ustr);
    return new UIObjectUnoObj(std::move(pObj));
}

css::uno::Reference<css::ui::test::XUIObject>
    UITestUnoObj::getWindow(const css::uno::Reference<::css::awt::XWindow>& xWindow)
{
    if (!xWindow.is())
        return {};

    UnoWrapperBase* pWrapper = UnoWrapperBase::GetUnoWrapper();
    assert(pWrapper);
    VclPtr<vcl::Window> pWindow = pWrapper->GetWindow(xWindow);
    assert(pWindow);

    return new UIObjectUnoObj(pWindow->GetUITestFactory()(pWindow));
}

OUString UITestUnoObj::getImplementationName()
{
    return u"org.libreoffice.uitest.UITest"_ustr;
}

bool UITestUnoObj::supportsService(OUString const & ServiceName)
{
    return cppu::supportsService(this, ServiceName);
}

cpo::uno::Sequence<OUString> UITestUnoObj::getSupportedServiceNames()
{
    return { u"com.sun.star.ui.test.UITest"_ustr };
}

extern "C" SAL_DLLPUBLIC_EXPORT cpo::uno::XInterface*
UITest_get_implementation(cpo::uno::XComponentContext*, cpo::uno::Sequence<cpo::uno::Any> const &)
{
    return cppu::acquire(new UITestUnoObj());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
