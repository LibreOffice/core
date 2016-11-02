/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <cppuhelper/implbase.hxx>
#include <com/sun/star/frame/XSynchronousDispatch.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>

#include <comphelper/processfactory.hxx>
#include <cppuhelper/supportsservice.hxx>

#include <vcl/svapp.hxx>

#include "SafeModeDialog.hxx"

namespace {

class SafeModeUI : public ::cppu::WeakImplHelper< css::lang::XServiceInfo,
                                                  css::frame::XSynchronousDispatch > // => XDispatch!
{
public:
    SafeModeUI();
    virtual ~SafeModeUI() override;

    // css.lang.XServiceInfo
    virtual OUString SAL_CALL getImplementationName()
        throw(css::uno::RuntimeException, std::exception) override;

    virtual sal_Bool SAL_CALL supportsService(const OUString& sServiceName)
        throw(css::uno::RuntimeException, std::exception) override;

    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()
        throw(css::uno::RuntimeException, std::exception) override;


    virtual css::uno::Any SAL_CALL dispatchWithReturnValue(const css::util::URL& aURL,
                                        const css::uno::Sequence< css::beans::PropertyValue >& lArguments )
        throw(css::uno::RuntimeException, std::exception) override;
};

SafeModeUI::SafeModeUI()
{
}

SafeModeUI::~SafeModeUI()
{
}

OUString SAL_CALL SafeModeUI::getImplementationName()
    throw(css::uno::RuntimeException, std::exception)
{
    return OUString("com.sun.star.comp.svx.SafeModeUI");
}

sal_Bool SAL_CALL SafeModeUI::supportsService(const OUString& sServiceName)
    throw(css::uno::RuntimeException, std::exception)
{
    return cppu::supportsService(this, sServiceName);
}

css::uno::Sequence< OUString > SAL_CALL SafeModeUI::getSupportedServiceNames()
    throw(css::uno::RuntimeException, std::exception)
{
    css::uno::Sequence< OUString > lServiceNames { "com.sun.star.dialog.SafeModeUI" };
    return lServiceNames;
}

css::uno::Any SAL_CALL SafeModeUI::dispatchWithReturnValue(const css::util::URL&,
                                                   const css::uno::Sequence< css::beans::PropertyValue >& )
    throw(css::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    css::uno::Any aRet;
    ScopedVclPtrInstance<SafeModeDialog> xDialog(nullptr);
    xDialog->Execute();
    return aRet;
}

}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface * SAL_CALL
com_sun_star_comp_svx_SafeModeUI_get_implementation(
    css::uno::XComponentContext * /*context*/,
    css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new SafeModeUI);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
