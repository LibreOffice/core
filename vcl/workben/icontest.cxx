/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

/*
 * =======================================================================
 *
 * This is a quick hack to test some stuff. Work in progress. Don't touch
 * and don't bother inspecting too closely.
 *
 * =======================================================================
 */

#include <iostream>

#include <math.h>

#include <glm/gtx/bit.hpp>

#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/XMultiComponentFactory.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/ucb/UniversalContentBroker.hpp>
#include <comphelper/processfactory.hxx>
#include <cppuhelper/bootstrap.hxx>
#include <osl/file.hxx>
#include <vcl/builder.hxx>
#include <vcl/button.hxx>
#include <vcl/dialog.hxx>
#include <vcl/fixed.hxx>
#include <vcl/graph.hxx>
#include <vcl/graphicfilter.hxx>
#include <vcl/image.hxx>
#include <vcl/opengl/OpenGLContext.hxx>
#include <vcl/opengl/OpenGLHelper.hxx>
#include <vcl/svapp.hxx>
#include <vcl/vclmain.hxx>
#include <vcl/wrkwin.hxx>

using namespace com::sun::star;

namespace {
    const int WIDTH = 1000, HEIGHT = 800;

    double getTimeNow()
    {
        TimeValue aValue;
        osl_getSystemTime(&aValue);
        return static_cast<double>(aValue.Seconds) +
            static_cast<double>(aValue.Nanosec) / (1000*1000*1000);
    }

}

class MyWorkWindow : public WorkWindow
{
    double mnStartTime;
    int mnPaintCount;

public:
    Graphic maGraphic;
    BitmapEx *mpBitmap;
    VclPtr<FixedBitmap> mpFixedBitmap;

    MyWorkWindow( vcl::Window* pParent, WinBits nWinStyle );
    virtual ~MyWorkWindow() override { disposeOnce(); }
    virtual void dispose() override { mpFixedBitmap.clear(); WorkWindow::dispose(); }
    void LoadGraphic( const OUString& sImageFile );

    virtual void Paint( vcl::RenderContext& /*rRenderContext*/, const tools::Rectangle& rRect ) override;
    virtual void Resize() override;
};

MyWorkWindow::MyWorkWindow( vcl::Window* pParent, WinBits nWinStyle )
    : WorkWindow(pParent, nWinStyle)
    , mpBitmap(nullptr)
    , mpFixedBitmap(nullptr)
{
    mnPaintCount = 0;
    mnStartTime = getTimeNow();
    EnableInput();
}

void MyWorkWindow::LoadGraphic( const OUString& sImageFile )
{
    SvFileStream aFileStream( sImageFile, StreamMode::READ );
    GraphicFilter aGraphicFilter(false);
    if (aGraphicFilter.ImportGraphic(maGraphic, sImageFile, aFileStream) != ERRCODE_NONE)
    {
        SAL_WARN("vcl.icontest", "Could not import image '" << sImageFile << "'");
        return;
    }
}

void MyWorkWindow::Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect)
{
    std::cout << "==> Paint! " << mnPaintCount++ << " (vcl) " << GetSizePixel() << " " << getTimeNow() - mnStartTime << std::endl;

    Size aGraphicSize( maGraphic.GetSizePixel() );
    float aspect = static_cast<float>(aGraphicSize.Width()) / aGraphicSize.Height();
    Size aSize;
    if( aspect >= (float(WIDTH)) / HEIGHT )
        aSize = Size( WIDTH, HEIGHT/aspect );
    else
        aSize = Size( WIDTH * aspect, HEIGHT );
    aSize.setWidth( aSize.Width() * (1 + (0.1*sin(mnPaintCount/60.))) );
    aSize.setHeight( aSize.Height() * (1 + (0.1*sin(mnPaintCount/50.))) );

    BitmapEx aEmpty;
    mpFixedBitmap->SetBitmap( aEmpty );
    GraphicConversionParameters aConv( aSize );
    mpBitmap = new BitmapEx(maGraphic.GetBitmapEx( aConv ));
    mpFixedBitmap->SetBitmap( *mpBitmap );
    mpFixedBitmap->SetSizePixel( aSize );

    WorkWindow::Paint(rRenderContext, rRect);

    if (mnPaintCount == 100)
        Application::Quit();

    Invalidate( InvalidateFlags::Children );
}

void MyWorkWindow::Resize()
{
    SAL_INFO("vcl.icontest", "Resize " << GetSizePixel());
}

class IconTestApp : public Application
{
public:
    virtual void Init() override;
    virtual int Main() override;

    IconTestApp() : nRet(EXIT_SUCCESS) {};

private:
    int nRet;

    void DoItWithVcl(const OUString& sImageFile);
};

void IconTestApp::Init()
{
    nRet = EXIT_SUCCESS;

    uno::Reference<uno::XComponentContext> xContext =
        cppu::defaultBootstrap_InitialComponentContext();
    uno::Reference<lang::XMultiComponentFactory> xFactory =
        xContext->getServiceManager();
    uno::Reference<lang::XMultiServiceFactory> xSFactory =
        uno::Reference<lang::XMultiServiceFactory> (xFactory, uno::UNO_QUERY_THROW);
    comphelper::setProcessServiceFactory(xSFactory);

    // Create UCB (for backwards compatibility, in case some code still uses
    // plain createInstance w/o args directly to obtain an instance):
    ::ucb::UniversalContentBroker::create(
        comphelper::getProcessComponentContext() );
}

int IconTestApp::Main()
{
    if (GetCommandLineParamCount() != 1)
    {
        fprintf(stderr, "Usage: imagetest <image>\n");
        return EXIT_FAILURE;
    }
    OUString sImageFile( GetCommandLineParam( 0 ) );
    DoItWithVcl( sImageFile );

    return nRet;
}

void IconTestApp::DoItWithVcl( const OUString& sImageFile)
{
    try
    {
        VclPtrInstance<MyWorkWindow> pWindow( nullptr, WB_APP | WB_STDWORK | WB_SIZEABLE | WB_CLOSEABLE | WB_CLIPCHILDREN );

        pWindow->SetText("VCL Image Test");

        pWindow->LoadGraphic( sImageFile );
        pWindow->mpFixedBitmap = VclPtr<FixedBitmap>::Create( pWindow );
        pWindow->mpFixedBitmap->SetPosPixel( Point( 0, 0 ) );
        pWindow->mpFixedBitmap->Show();

        pWindow->Hide();
        pWindow->Show();

        Execute();
    }
    catch (const uno::Exception &e)
    {
        fprintf(stderr, "fatal error: %s\n", OUStringToOString(e.Message, osl_getThreadTextEncoding()).getStr());
        nRet = EXIT_FAILURE;
    }
    catch (const std::exception &e)
    {
        fprintf(stderr, "fatal error: %s\n", e.what());
        nRet = EXIT_FAILURE;
    }
}

void vclmain::createApplication()
{
    static IconTestApp aApp;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
