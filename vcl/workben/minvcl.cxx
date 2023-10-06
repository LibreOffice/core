/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <framework/desktop.hxx>
#include <cppuhelper/bootstrap.hxx>
#include <comphelper/processfactory.hxx>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <i18nlangtag/languagetag.hxx>
#include <i18nlangtag/mslangid.hxx>

#include <vcl/svapp.hxx>
#include <vcl/wrkwin.hxx>
#include <sal/main.h>

#include <iostream>

namespace
{
class TheApplication : public Application
{
public:
    virtual int Main();

private:
    VclPtr<vcl::Window> mpWin;
};
}

int TheApplication::Main()
{
    mpWin = VclPtr<WorkWindow>::Create(nullptr, WB_APP | WB_STDWORK);
    mpWin->SetText(u"Minimum VCL application with a window"_ustr);
    mpWin->Show();
    Execute();
    mpWin.disposeAndClear();
    return 0;
}

SAL_IMPLEMENT_MAIN()
{
    try
    {
        TheApplication aApp;

        auto xContext = cppu::defaultBootstrap_InitialComponentContext();
        css::uno::Reference<css::lang::XMultiServiceFactory> xServiceManager(
            xContext->getServiceManager(), css::uno::UNO_QUERY);
        comphelper::setProcessServiceFactory(xServiceManager);
        LanguageTag::setConfiguredSystemLanguage(MsLangId::getSystemLanguage());
        InitVCL();

        aApp.Main();

        framework::getDesktop(::comphelper::getProcessComponentContext())->terminate();
        DeInitVCL();
        comphelper::setProcessServiceFactory(nullptr);
    }
    catch (...)
    {
        std::cout << "Exception has occurred\n";
        return 1;
    }

    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
