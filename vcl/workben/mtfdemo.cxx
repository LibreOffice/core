/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <comphelper/processfactory.hxx>
#include <comphelper/random.hxx>
#include <cppuhelper/bootstrap.hxx>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/registry/XSimpleRegistry.hpp>
#include <com/sun/star/ucb/UniversalContentBroker.hpp>

#include <vcl/vclmain.hxx>
#include <vcl/layout.hxx>
#include <vcl/gdimtf.hxx>
#include <vcl/wmf.hxx>

#include <tools/urlobj.hxx>
#include <tools/stream.hxx>
#include <tools/vcompat.hxx>
#include <vcl/svapp.hxx>
#include <vcl/wrkwin.hxx>
#include <vcl/virdev.hxx>

#include <tools/stream.hxx>

#include <cstdlib>

using namespace com::sun::star;

using namespace css;

class DemoMtfWin : public WorkWindow
{
    GDIMetaFile *mpMtf;

public:
    DemoMtfWin(OUString& aFileName) :
        WorkWindow(NULL, WB_APP | WB_STDWORK)
    {


        SvFileStream aFileStream(aFileName, STREAM_READ);

        if (aFileStream.IsOpen())
        {
            ReadWindowMetafile(aFileStream, *mpMtf);
        }
        else
        {
            Application::Abort("Can't read metafile");
        }
    }

    virtual void Paint( const Rectangle& rRect )  SAL_OVERRIDE;
};

void DemoMtfWin::Paint( const Rectangle& rRect )
{
    mpMtf->Play(this, mpMtf->GetActionSize());

    WorkWindow::Paint( rRect );
}

class DemoMtfApp : public Application
{
    DemoMtfWin *mpWin;
    OUString *mpFileName;

    void showHelp()
    {
        fprintf(stderr, "Usage: mtfdemo --help | FILE\n");
        fprintf(stderr, "A VCL test app that displays Windows metafiles\n");
        std::exit(0);
    }

public:

    DemoMtfApp()
        : mpWin(NULL)
        , mpFileName(NULL)
    {
    }

    virtual int Main() SAL_OVERRIDE
    {
        mpWin = new DemoMtfWin(*mpFileName);
        mpWin->SetText(OUString("Display metafile"));

        mpWin->Show();

        Application::Execute();

        return 0;
    }

protected:
    uno::Reference<lang::XMultiServiceFactory> xMSF;
    void Init() SAL_OVERRIDE
    {
        try
        {
            sal_uInt32 nCmdParams = GetCommandLineParamCount();

            if (nCmdParams == 0)
                showHelp();
            else
            {
                OUString aArg = GetCommandLineParam(0);

                if (aArg == "--help" || aArg == "-h")
                    showHelp();
                else
                    mpFileName = new OUString(aArg);
            }

            uno::Reference<uno::XComponentContext> xComponentContext
                = ::cppu::defaultBootstrap_InitialComponentContext();
            xMSF = uno::Reference<lang::XMultiServiceFactory>
                (xComponentContext->getServiceManager(), uno::UNO_QUERY);
            if(!xMSF.is())
                Application::Abort("Bootstrap failure - no service manager");

            ::comphelper::setProcessServiceFactory(xMSF);
        }
        catch (const uno::Exception &e)
        {
            Application::Abort("Bootstrap exception " + e.Message);
        }
    }

    void DeInit() SAL_OVERRIDE
    {
        uno::Reference< lang::XComponent >(
            comphelper::getProcessComponentContext(),
        uno::UNO_QUERY_THROW)-> dispose();
        ::comphelper::setProcessServiceFactory(NULL);
    }

};


void vclmain::createApplication()
{
    static DemoMtfApp aApp;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
