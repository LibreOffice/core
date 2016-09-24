/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
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
    virtual void Init() override;
    virtual int Main() override;
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

    // Create UCB (for backwards compatibility, in case some code still uses
    // plain createInstance w/o args directly to obtain an instance):
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

    // turn on tooltips
    Help::EnableQuickHelp();

    int nRet = EXIT_SUCCESS;

    try
    {
        VclPtrInstance<Dialog> pDialog(nullptr, WB_STDDIALOG | WB_SIZEABLE, Dialog::InitFlag::NoParent);
        {
            VclBuilder aBuilder(pDialog, OUString(), uifiles[0]);
            vcl::Window *pRoot = aBuilder.get_widget_root();
            Dialog *pRealDialog = dynamic_cast<Dialog*>(pRoot);

            if (!pRealDialog)
                pRealDialog = pDialog;

            pRealDialog->SetText("LibreOffice ui-previewer");
            pRealDialog->SetStyle(pDialog->GetStyle()|WB_CLOSEABLE);
            /*
               Force a new StateChangedType::InitShow for the edge case where pRoot
               is not a dialog or contents of a dialog, but instead a visible floating window
               which may have had initshow already done before it was given children
            */
            pRoot->Hide();
            pRoot->Show();
            pRealDialog->Execute();
        }

        pDialog.disposeAndClear();
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

    return nRet;
}

void vclmain::createApplication()
{
    static UIPreviewApp aApp;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
