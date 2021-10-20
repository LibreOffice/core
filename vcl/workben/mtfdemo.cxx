/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <comphelper/processfactory.hxx>
#include <cppuhelper/bootstrap.hxx>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/registry/XSimpleRegistry.hpp>
#include <com/sun/star/ucb/UniversalContentBroker.hpp>

#include <vcl/vclmain.hxx>
#include <vcl/layout.hxx>
#include <vcl/gdimtf.hxx>
#include <vcl/wmf.hxx>

#include <tools/diagnose_ex.h>
#include <tools/urlobj.hxx>
#include <tools/stream.hxx>
#include <tools/vcompat.hxx>
#include <vcl/svapp.hxx>
#include <vcl/wrkwin.hxx>
#include <vcl/virdev.hxx>
#include <sal/log.hxx>
#include <osl/file.hxx>
#include <osl/process.h>

#include <iostream>

using namespace css;

namespace {

class DemoMtfWin : public WorkWindow
{
    OUString maFileName;

public:
    explicit DemoMtfWin(const OUString& rFileName)
        : WorkWindow(nullptr, WB_APP | WB_STDWORK)
    {
        maFileName = rFileName;
    }

    virtual void Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect)  override;
};

}

void DemoMtfWin::Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect)
{
    GDIMetaFile aMtf;
    SvFileStream aFileStream(maFileName, StreamMode::READ);

    if (aFileStream.IsOpen())
    {
        ReadWindowMetafile(aFileStream, aMtf);
    }
    else
    {
        Application::Abort("Can't read metafile");
    }

    aMtf.Play(*GetOutDev(), aMtf.GetActionSize());
    aMtf.Stop();
    aFileStream.Close();

    WorkWindow::Paint(rRenderContext, rRect);
}

namespace {

class DemoMtfApp : public Application
{
    VclPtr<DemoMtfWin> mpWin;
    OUString maFileName;

    static void showHelp()
    {
        std::cerr << "Usage: mtfdemo --help | FILE | -d FILE" << std::endl;
        std::cerr << "A VCL test app that displays Windows metafiles or dumps metaactions." << std::endl;
        std::cerr << "If you want to dump as metadump.xml, use -d before FILE." << std::endl;
        std::exit(0);
    }

public:

    DemoMtfApp()
        : mpWin(nullptr)
    {
    }

    virtual int Main() override
    {
        try
        {
            mpWin = VclPtr<DemoMtfWin>::Create(maFileName);
            mpWin->SetText("Display metafile");

            mpWin->Show();

            Application::Execute();
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

private:
    uno::Reference<lang::XMultiServiceFactory> xMSF;
    void Init() override
    {
        try
        {
            const sal_uInt16 nCmdParams = GetCommandLineParamCount();
            OUString aArg, aFilename;
            bool bDumpXML = false;

            if (nCmdParams == 0)
            {
                showHelp();
                std::exit(1);
            }
            else
            {
                aArg = GetCommandLineParam(0);

                if (aArg == "--help" || aArg == "-h")
                {
                    showHelp();
                    std::exit(0);
                }
                else if (nCmdParams > 1 && (aArg == "--dump" || aArg == "-d"))
                {
                    aFilename = GetCommandLineParam(1);
                    bDumpXML = true;
                }
                else
                    aFilename = aArg;
            }

            OUString sWorkingDir, sFileUrl;
            osl_getProcessWorkingDir(&sWorkingDir.pData);
            osl::FileBase::getFileURLFromSystemPath(aFilename, sFileUrl);
            osl::FileBase::getAbsoluteFileURL(sWorkingDir, sFileUrl, maFileName);

            uno::Reference<uno::XComponentContext> xComponentContext
                = ::cppu::defaultBootstrap_InitialComponentContext();
            xMSF.set(xComponentContext->getServiceManager(), uno::UNO_QUERY);
            if(!xMSF.is())
                Application::Abort("Bootstrap failure - no service manager");

            ::comphelper::setProcessServiceFactory(xMSF);

            if(bDumpXML)
            {
                GDIMetaFile aMtf;
                SvFileStream aFileStream(maFileName, StreamMode::READ);
                ReadWindowMetafile(aFileStream, aMtf);
                OUString sAbsoluteDumpUrl, sDumpUrl;
                osl::FileBase::getFileURLFromSystemPath("metadump.xml", sDumpUrl);
                osl::FileBase::getAbsoluteFileURL(sWorkingDir, sDumpUrl, sAbsoluteDumpUrl);

                aMtf.dumpAsXml(rtl::OUStringToOString(sAbsoluteDumpUrl, RTL_TEXTENCODING_UTF8).getStr());
                std::cout << "Dumped metaactions as metadump.xml" << std::endl;
                std::exit(0);
            }

        }
        catch (const uno::Exception &e)
        {
            Application::Abort("Bootstrap exception " + e.Message);
        }
    }

    void DeInit() override
    {
        uno::Reference< lang::XComponent >(
            comphelper::getProcessComponentContext(),
        uno::UNO_QUERY_THROW)-> dispose();
        ::comphelper::setProcessServiceFactory(nullptr);
    }

};

}

void vclmain::createApplication()
{
    static DemoMtfApp aApp;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
