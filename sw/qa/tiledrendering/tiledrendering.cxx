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
#include <vcl/field.hxx>
#include <vcl/button.hxx>

class UIPreviewApp : public Application
{
public:
    virtual void Init();
    virtual int Main();
};

using namespace com::sun::star;

class TiledRenderingDialog: public ModalDialog{
public:
    TiledRenderingDialog() : ModalDialog(DIALOG_NO_PARENT, "TiledRendering", "qa/sw/ui/tiledrendering.ui"){
        PushButton * renderButton;
        get(renderButton,"buttonRenderTile");
        renderButton->SetClickHdl( LINK( this, TiledRenderingDialog, RenderHdl));
    }
    virtual ~TiledRenderingDialog(){}

    DECL_LINK ( RenderHdl, Button * );

    sal_Int32 extractInt(const char * name){
            NumericField * pField;
            get(pField,name);
            OUString aString(pField->GetText());
            SAL_INFO("TiledRenderingDialog","param " << name << " returned " << aString);
            return aString.toInt32();
    }

};

IMPL_LINK ( TiledRenderingDialog,  RenderHdl, Button *, EMPTYARG )
{
    extractInt("spinContextWidth");
    extractInt("spinContextHeight");
    extractInt("spinTilePosX");
    extractInt("spinTilePosY");
    extractInt("spinTileWidth");
    extractInt("spinTileHeight");

   return 1;
}

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
    //std::vector<OUString> uifiles;
    //for (sal_uInt16 i = 0; i < GetCommandLineParamCount(); ++i)
    //{
    //    OUString aFileUrl;
    //   osl::File::getFileURLFromSystemPath(GetCommandLineParam(i), aFileUrl);
    //    uifiles.push_back(aFileUrl);
    //}

    //if (uifiles.empty())
    //{
    //    fprintf(stderr, "Usage: ui-previewer file.ui\n");
    //    return EXIT_FAILURE;
    //}

    // turn on tooltips
    Help::EnableQuickHelp();

    try
    {

        TiledRenderingDialog pDialog;

        pDialog.Execute();
/*
        {
            VclBuilder aBuilder(pDialog, OUString(), "sw/qa/tiledrendering/tiledrendering.ui");
            Dialog *pRealDialog = dynamic_cast<Dialog*>(aBuilder.get_widget_root());

            if (!pRealDialog)
                pRealDialog = pDialog;

            if (pRealDialog)
            {
                pRealDialog->SetText(OUString("LibreOffice ui-previewer"));
                pRealDialog->SetStyle(pDialog->GetStyle()|WB_CLOSEABLE);
                pRealDialog->Execute();
            }
        }*/
    }
    catch (const uno::Exception &e)
    {
        fprintf(stderr, "fatal error: %s\n", OUStringToOString(e.Message, osl_getThreadTextEncoding()).getStr());
    }

    return EXIT_SUCCESS;
}

void render(){


}


void vclmain::createApplication()
{
    static UIPreviewApp aApp;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
