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

#include <vcl/svapp.hxx>
#include <vcl/wrkwin.hxx>

class TheApplication : public Application
{
public:
    virtual int Main();
};
class TheWindow : public WorkWindow
{
public:
    TheWindow(Window* parent, WinBits windowStyle)
        : WorkWindow(parent, windowStyle)
    {
    }
    virtual void Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect);
};

void TheWindow::Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect)
{
    rRenderContext.DrawText(Point(rRect.GetWidth() / 2, rRect.getHeight() / 2),
                            OUString(u"VCL module in LibreOffice"));
}

int TheApplication::Main()
{
    TheWindow aWindow(NULL, WB_APP | WB_STDWORK);
    aWindow.SetText(u"VCL");
    aWindow.Show();
    Execute();
    return 0;
}

int main()
{
    auto xContext = cppu::defaultBootstrap_InitialComponentContext();
    css::uno::Reference<css::lang::XMultiServiceFactory> xServiceManager(
        xContext->getServiceManager(), css::uno::UNO_QUERY);
    comphelper::setProcessServiceFactory(xServiceManager);

    TheApplication anApplication;
    InitVCL();
    int ret = anApplication.Main();
    DeInitVCL();

    return ret;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
