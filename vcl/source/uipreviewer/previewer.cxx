/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/XMultiComponentFactory.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <comphelper/processfactory.hxx>
#include <cppuhelper/bootstrap.hxx>
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
        uifiles.push_back(GetCommandLineParam(i));

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

    VclBuilder aBuilder(NULL, uifiles[0]);
    Window *pWindow = aBuilder.get_widget_root();
    Dialog *pDialog = dynamic_cast<Dialog*>(pWindow);
    if (pDialog)
    {
        pDialog->Execute();
    }
    else
    {
        fprintf(stderr, "to-do: no toplevel dialog, make one\n");
    }

    ::ucbhelper::ContentBroker::deinitialize();

    return EXIT_SUCCESS;
}

UIPreviewApp aApp;

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
