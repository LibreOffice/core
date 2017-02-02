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

#include "crashreportdlg.hxx"

namespace {

class CrashReportUI : public ::cppu::WeakImplHelper< css::lang::XServiceInfo        ,
                                                   css::frame::XSynchronousDispatch > // => XDispatch!
{
public:
    explicit CrashReportUI(const css::uno::Reference< css::uno::XComponentContext >& xContext);

    // css.lang.XServiceInfo

    virtual OUString SAL_CALL getImplementationName() override;

    virtual sal_Bool SAL_CALL supportsService(const OUString& sServiceName) override;

    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;


    virtual css::uno::Any SAL_CALL dispatchWithReturnValue(const css::util::URL& aURL,
                                        const css::uno::Sequence< css::beans::PropertyValue >& lArguments ) override;

private:
    css::uno::Reference< css::uno::XComponentContext > mxContext;
};

CrashReportUI::CrashReportUI(const css::uno::Reference<css::uno::XComponentContext>& xContext):
    mxContext(xContext)
{

}

OUString SAL_CALL CrashReportUI::getImplementationName()
{
    return OUString("com.sun.star.comp.svx.CrashReportUI");
}

sal_Bool SAL_CALL CrashReportUI::supportsService(const OUString& sServiceName)
{
    return cppu::supportsService(this, sServiceName);
}

css::uno::Sequence< OUString > SAL_CALL CrashReportUI::getSupportedServiceNames()
{
    return { "com.sun.star.dialog.CrashReportUI" };
}

css::uno::Any SAL_CALL CrashReportUI::dispatchWithReturnValue(const css::util::URL&,
                                                   const css::uno::Sequence< css::beans::PropertyValue >& )
{
    SolarMutexGuard aGuard;
    css::uno::Any aRet;
    ScopedVclPtrInstance<CrashReportDialog> xDialog(nullptr);
    xDialog->Execute();
    return aRet;
}

}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface * SAL_CALL
com_sun_star_comp_svx_CrashReportUI_get_implementation(
    css::uno::XComponentContext *context,
    css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new CrashReportUI(context));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
