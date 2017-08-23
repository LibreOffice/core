/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/main.h>
#include <tools/extendapplicationenvironment.hxx>

#include <cppuhelper/bootstrap.hxx>
#include <comphelper/processfactory.hxx>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

#include <vcl/svapp.hxx>
#include <vcl/window.hxx>
#include <vcl/dialog.hxx>
#include <vcl/outdev.hxx>
#include <vcl/virdev.hxx>

#include "output.hxx"
#include "document.hxx"
#include "fillinfo.hxx"
#include "scdll.hxx"

using namespace ::com::sun::star;

class CalcTestApp : public Application
{
public:
    virtual int Main() override;
    virtual void Exception( ExceptionCategory nCategory ) override;
};

class TestWindow : public Dialog
{
    std::unique_ptr<ScDocument> mpDoc;
    public:
        TestWindow() :
            Dialog( nullptr ),
            mpDoc(new ScDocument)
        {
            mpDoc->InsertTab(0, "test");
            mpDoc->SetValue(0, 0, 0, 10);
            mpDoc->SetValue(2, 2, 0, 10);
            SetText( "OutDev grinding" );
            SetSizePixel( Size( 1024, 1024 ) );
            EnablePaint( true );
            Show();
        }

        ~TestWindow()
        {
            disposeOnce();
        }

        void dispose() override
        {
            Dialog::dispose();
        }

        virtual void Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect) override;
};

void TestWindow::Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle&)
{
    rRenderContext.SetFillColor(Color(0, 255, 255));
    rRenderContext.DrawRect(tools::Rectangle(0, 0, 100, 100));
    ScTableInfo aTableInfo;
    mpDoc->FillInfo(aTableInfo, 0, 0, 10, 10, 0, 0.06666, 0.06666, false, false);
    ScOutputData aOutput(&rRenderContext, OUTTYPE_WINDOW, aTableInfo, mpDoc.get(), 0,
            0, 0, 0, 0, 10, 10, 0.06666, 0.06666);

    aOutput.SetGridColor(COL_BLACK);
    aOutput.SetSolidBackground(true);
    aOutput.DrawClear();
    aOutput.DrawDocumentBackground();
    aOutput.DrawGrid(rRenderContext, true, false);
    aOutput.DrawStrings();

    fflush(stdout);
}

void CalcTestApp::Exception( ExceptionCategory nCategory )
{
    switch( nCategory )
    {
        case ExceptionCategory::ResourceNotLoaded:
            Abort( "Error: could not load language resources.\nPlease check your installation.\n" );
            break;
        default: break;
    }
}

int CalcTestApp::Main()
{
    ScDLL::Init();
    ScGlobal::Init();
    ScopedVclPtrInstance<TestWindow> aWindow;
    aWindow->Execute();
    return 0;
}

SAL_IMPLEMENT_MAIN()
{
    try
    {
        bool bHelp = false;

        for( sal_uInt16 i = 0; i < Application::GetCommandLineParamCount(); i++ )
        {
            OUString aParam = Application::GetCommandLineParam( i );

            if( aParam == "--help" || aParam == "-h" )
                    bHelp = true;
        }

        if( bHelp )
        {
            printf( "sc\n" );
            return EXIT_SUCCESS;
        }

        tools::extendApplicationEnvironment();

        uno::Reference< uno::XComponentContext > xContext = cppu::defaultBootstrap_InitialComponentContext();
        uno::Reference< lang::XMultiServiceFactory > xServiceManager( xContext->getServiceManager(), uno::UNO_QUERY );

        if( !xServiceManager.is() )
            Application::Abort( "Failed to bootstrap" );

        comphelper::setProcessServiceFactory( xServiceManager );

        InitVCL();

        CalcTestApp aApp;
        aApp.Main();

        DeInitVCL();
    }
    catch (const css::uno::Exception& e)
    {
        SAL_WARN("vcl.app", "Fatal exception: " << e.Message);
        return EXIT_FAILURE;
    }
    catch (const std::exception& e)
    {
        SAL_WARN("vcl.app", "Fatal exception: " << e.what());
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
