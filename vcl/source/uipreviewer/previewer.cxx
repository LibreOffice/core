/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Version: MPL 1.1 / GPLv3+ / LGPLv3+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Initial Developer of the Original Code is
 *        Caolán McNamara <caolanm@redhat.com> (Red Hat, Inc.)
 * Portions created by the Initial Developer are Copyright (C) 2011 the
 * Initial Developer. All Rights Reserved.
 *
 * Contributor(s): Caolán McNamara <caolanm@redhat.com>
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
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

    try
    {
        VclBuilder aBuilder(NULL, uifiles[0]);
        Window *pWindow = aBuilder.get_widget_root();
        Dialog *pDialog = dynamic_cast<Dialog*>(pWindow);
        if (pDialog)
        {
            pDialog->SetText(rtl::OUString("LibreOffice ui-previewer"));
            pDialog->SetStyle(pDialog->GetStyle()|WB_CLOSEABLE);
            pDialog->Execute();
        }
        else
        {
            fprintf(stderr, "to-do: no toplevel dialog, make one\n");
        }
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
