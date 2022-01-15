/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <osl/file.hxx>
#include <osl/process.h>
#include <rtl/textenc.h>
#include <sal/main.h>
#include <comphelper/processfactory.hxx>
#include <cppuhelper/bootstrap.hxx>
#include <tools/diagnose_ex.h>
#include <tools/degree.hxx>
#include <i18nlangtag/languagetag.hxx>
#include <i18nlangtag/mslangid.hxx>

#include <vcl/font/Feature.hxx>
#include <vcl/metric.hxx>
#include <vcl/svapp.hxx>
#include <vcl/vclmain.hxx>
#include <vcl/wrkwin.hxx>

#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

#include <iostream>
#include <fstream>
#include <string>
#include <string_view>

namespace
{
class ListFontsWin : public WorkWindow
{
public:
    explicit ListFontsWin()
        : WorkWindow(nullptr, WB_HIDE)
    {
    }
};

class ListFonts : public Application
{
public:
    virtual int Main() override;

private:
    void Init() override;
    void DeInit() override;

    css::uno::Reference<css::lang::XMultiServiceFactory> xServiceManager;
};

int ListFonts::Main()
{
    try
    {
        VclPtrInstance<ListFontsWin> pWin;
        OutputDevice* pOutDev = pWin->GetOutDev();

        std::cerr << Application::GetOSVersion() << std::endl;

        std::exit(0);
    }
    catch (const css::uno::Exception&)
    {
        TOOLS_WARN_EXCEPTION("vcl.app", "Fatal");
        return 1;
    }
    catch (const std::exception& e)
    {
        SAL_WARN("vcl.app", "Fatal: " << e.what());
        return 1;
    }
    return 0;
}
}

void ListFonts::Init()
{
    auto xContext = cppu::defaultBootstrap_InitialComponentContext();
    xServiceManager.set(xContext->getServiceManager(), css::uno::UNO_QUERY);

    if (!xServiceManager.is())
        Application::Abort("Bootstrap failure - no service manager");

    comphelper::setProcessServiceFactory(xServiceManager);

    LanguageTag::setConfiguredSystemLanguage(MsLangId::getSystemLanguage());
}

void ListFonts::DeInit()
{
    auto xContext = css::uno::Reference<css::lang::XComponent>(
        comphelper::getProcessComponentContext(), css::uno::UNO_QUERY_THROW);
    xContext->dispose();
    ::comphelper::setProcessServiceFactory(nullptr);
}

SAL_IMPLEMENT_MAIN()
{
    ListFonts aApp;
    InitVCL();
    int ret = aApp.Main();
    DeInitVCL();

    return ret;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
