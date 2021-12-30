/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <cppuhelper/bootstrap.hxx>
#include <comphelper/processfactory.hxx>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <i18nlangtag/languagetag.hxx>
#include <i18nlangtag/mslangid.hxx>

#include <vcl/svapp.hxx>
#include <vcl/wrkwin.hxx>

namespace
{
class TheWindow : public WorkWindow
{
public:
    TheWindow()
        : WorkWindow(nullptr, WB_APP | WB_STDWORK)
    {
    }
    virtual void Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect);
};

class TheApplication : public Application
{
public:
    virtual int Main();

private:
    VclPtr<TheWindow> mpWin;
};
}

void TheWindow::Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect)
{
    rRenderContext.DrawText(Point(rRect.GetWidth() / 2, rRect.getHeight() / 2),
                            OUString(u"VCL module in LibreOffice"));
}

int TheApplication::Main()
{
    mpWin = VclPtr<TheWindow>::Create();
    mpWin->SetText(u"VCL");
    mpWin->Show();
    Execute();
    mpWin.disposeAndClear();
    return 0;
}

int main()
{
    auto xContext = cppu::defaultBootstrap_InitialComponentContext();
    css::uno::Reference<css::lang::XMultiServiceFactory> xServiceManager(
        xContext->getServiceManager(), css::uno::UNO_QUERY);
    comphelper::setProcessServiceFactory(xServiceManager);
    LanguageTag::setConfiguredSystemLanguage(MsLangId::getSystemLanguage());

    TheApplication aApp;
    InitVCL();
    int ret = aApp.Main();
    DeInitVCL();

    css::uno::Reference<css::lang::XComponent>(xContext, css::uno::UNO_QUERY_THROW)->dispose();
    comphelper::setProcessServiceFactory(nullptr);

    return ret;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
