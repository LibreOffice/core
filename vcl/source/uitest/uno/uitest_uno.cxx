/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <cppuhelper/compbase.hxx>
#include <cppuhelper/basemutex.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/ui/test/XUITest.hpp>

#include <memory>

#include <vcl/uitest/uitest.hxx>
#include <vcl/svapp.hxx>

#include "uiobject_uno.hxx"

namespace
{
    typedef ::cppu::WeakComponentImplHelper <
        css::ui::test::XUITest, css::lang::XServiceInfo
        > UITestBase;
}

class UITestUnoObj : public cppu::BaseMutex,
    public UITestBase
{
private:
    std::unique_ptr<UITest> mpUITest;

public:

    UITestUnoObj();
    virtual ~UITestUnoObj();

    void SAL_CALL executeCommand(const OUString& rCommand)
        throw (css::uno::RuntimeException, std::exception) override;

    css::uno::Reference<css::ui::test::XUIObject> SAL_CALL getTopFocusWindow()
        throw (css::uno::RuntimeException, std::exception) override;

    OUString SAL_CALL getImplementationName()
        throw (css::uno::RuntimeException, std::exception) override;

    sal_Bool SAL_CALL supportsService(OUString const & ServiceName)
        throw (css::uno::RuntimeException, std::exception) override;

    css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames()
        throw (css::uno::RuntimeException, std::exception) override;
};

UITestUnoObj::UITestUnoObj():
    UITestBase(m_aMutex),
    mpUITest(new UITest)
{
}

UITestUnoObj::~UITestUnoObj()
{
}

void SAL_CALL UITestUnoObj::executeCommand(const OUString& rCommand)
    throw (css::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    mpUITest->executeCommand(rCommand);
}

css::uno::Reference<css::ui::test::XUIObject> SAL_CALL UITestUnoObj::getTopFocusWindow()
    throw (css::uno::RuntimeException, std::exception)
{
    std::unique_ptr<UIObject> pObj = mpUITest->getFocusTopWindow();
    return new UIObjectUnoObj(std::move(pObj));
}

OUString SAL_CALL UITestUnoObj::getImplementationName()
    throw (css::uno::RuntimeException, std::exception)
{
    return OUString("org.libreoffice.uitest.UITest");
}

sal_Bool UITestUnoObj::supportsService(OUString const & ServiceName)
    throw (css::uno::RuntimeException, std::exception)
{
    return cppu::supportsService(this, ServiceName);
}

css::uno::Sequence<OUString> UITestUnoObj::getSupportedServiceNames()
    throw (css::uno::RuntimeException, std::exception)
{
    css::uno::Sequence<OUString> aServiceNames(1);
    aServiceNames[0] = "com.sun.star.ui.test.UITest";
    return aServiceNames;
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface* SAL_CALL
UITest_get_implementation(css::uno::XComponentContext*, css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new UITestUnoObj());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
