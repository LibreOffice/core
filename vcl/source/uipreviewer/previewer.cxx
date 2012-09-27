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
#include <comphelper/processfactory.hxx>
#include <cppuhelper/bootstrap.hxx>
#include <osl/file.hxx>
#include <ucbhelper/configurationkeys.hxx>
#include <ucbhelper/contentbroker.hxx>
#include <vcl/builder.hxx>
#include <vcl/dialog.hxx>
#include <vcl/help.hxx>
#include <vcl/svapp.hxx>

class UIPreviewApp : public Application
{
public:
    virtual int Main();
};

using namespace com::sun::star;

int UIPreviewApp::Main()
{
    std::vector<rtl::OUString> uifiles;
    for (sal_uInt16 i = 0; i < GetCommandLineParamCount(); ++i)
    {
        rtl::OUString aFileUrl;
        osl::File::getFileURLFromSystemPath(GetCommandLineParam(i), aFileUrl);
        uifiles.push_back(aFileUrl);
    }

    if (uifiles.empty())
    {
        fprintf(stderr, "Usage: ui-previewer file.ui\n");
        return EXIT_FAILURE;
    }

    uno::Reference<uno::XComponentContext> xContext =
        cppu::defaultBootstrap_InitialComponentContext();
    uno::Reference<lang::XMultiComponentFactory> xFactory =
        xContext->getServiceManager();
    uno::Reference<lang::XMultiServiceFactory> xSFactory =
        uno::Reference<lang::XMultiServiceFactory> (xFactory, uno::UNO_QUERY_THROW);
    comphelper::setProcessServiceFactory(xSFactory);

    // Create UCB.
    uno::Sequence< uno::Any > aArgs(2);
    aArgs[ 0 ] <<= rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(UCB_CONFIGURATION_KEY1_LOCAL));
    aArgs[ 1 ] <<= rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(UCB_CONFIGURATION_KEY2_OFFICE));
    ::ucbhelper::ContentBroker::initialize(xSFactory, aArgs);

    // turn on tooltips
    Help::EnableQuickHelp();

    try
    {
        Dialog *pDialog = new Dialog(NULL, WB_STDDIALOG);

        {
            VclBuilder aBuilder(pDialog, rtl::OUString(), uifiles[0]);
            Dialog *pRealDialog = dynamic_cast<Dialog*>(aBuilder.get_widget_root());

            if (!pRealDialog)
                pRealDialog = pDialog;

            if (pRealDialog)
            {
                pRealDialog->SetText(rtl::OUString("LibreOffice ui-previewer"));
                pRealDialog->SetStyle(pDialog->GetStyle()|WB_CLOSEABLE);
                pRealDialog->Execute();
            }
        }

        delete pDialog;
    }
    catch (const uno::Exception &e)
    {
        fprintf(stderr, "fatal error: %s\n", rtl::OUStringToOString(e.Message, osl_getThreadTextEncoding()).getStr());
    }
    return false;


    ::ucbhelper::ContentBroker::deinitialize();

    return EXIT_SUCCESS;
}

UIPreviewApp aApp;

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
