/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 */

#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/XMultiComponentFactory.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/ucb/UniversalContentBroker.hpp>
#include <comphelper/processfactory.hxx>
#include <cppuhelper/bootstrap.hxx>
#include <osl/file.hxx>
#include <vcl/builder.hxx>
#include <vcl/dialog.hxx>
#include <vcl/help.hxx>
#include <vcl/svapp.hxx>
#include <vcl/vclmain.hxx>

class UIPreviewApp : public Application
{
public:
    virtual void Init();
    virtual int Main();
};

using namespace com::sun::star;

void UIPreviewApp::Init()
{
    uno::Reference<uno::XComponentContext> xContext =
        cppu::defaultBootstrap_InitialComponentContext();
    uno::Reference<lang::XMultiComponentFactory> xFactory =
        xContext->getServiceManager();
    uno::Reference<lang::XMultiServiceFactory> xSFactory =
        uno::Reference<lang::XMultiServiceFactory> (xFactory, uno::UNO_QUERY_THROW);
    comphelper::setProcessServiceFactory(xSFactory);

    
    
    ::ucb::UniversalContentBroker::create(
        comphelper::getProcessComponentContext() );
}

int UIPreviewApp::Main()
{
    std::vector<OUString> uifiles;
    for (sal_uInt16 i = 0; i < GetCommandLineParamCount(); ++i)
    {
        OUString aFileUrl;
        osl::File::getFileURLFromSystemPath(GetCommandLineParam(i), aFileUrl);
        uifiles.push_back(aFileUrl);
    }

    if (uifiles.empty())
    {
        fprintf(stderr, "Usage: ui-previewer file.ui\n");
        return EXIT_FAILURE;
    }

    
    Help::EnableQuickHelp();

    try
    {
        Dialog *pDialog = new Dialog(DIALOG_NO_PARENT, WB_STDDIALOG);

        {
            VclBuilder aBuilder(pDialog, OUString(), uifiles[0]);
            Dialog *pRealDialog = dynamic_cast<Dialog*>(aBuilder.get_widget_root());

            if (!pRealDialog)
                pRealDialog = pDialog;

            if (pRealDialog)
            {
                pRealDialog->SetText(OUString("LibreOffice ui-previewer"));
                pRealDialog->SetStyle(pDialog->GetStyle()|WB_CLOSEABLE);
                pRealDialog->Execute();
            }
        }

        delete pDialog;
    }
    catch (const uno::Exception &e)
    {
        fprintf(stderr, "fatal error: %s\n", OUStringToOString(e.Message, osl_getThreadTextEncoding()).getStr());
    }

    return EXIT_SUCCESS;
}

void vclmain::createApplication()
{
    static UIPreviewApp aApp;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
