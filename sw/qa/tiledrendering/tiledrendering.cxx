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
#include <osl/file.hxx>
#include <vcl/builder.hxx>
#include <vcl/dialog.hxx>
#include <vcl/help.hxx>
#include <vcl/svapp.hxx>
#include <vcl/vclmain.hxx>
#include <vcl/field.hxx>
#include <vcl/button.hxx>
#include <vcl/fixed.hxx>
#include <vcl/virdev.hxx>
#include <sfx2/filedlghelper.hxx>
#include <swmodule.hxx>
#include <wrtsh.hxx>

#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/XMultiComponentFactory.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/ucb/UniversalContentBroker.hpp>
#include <com/sun/star/ui/dialogs/TemplateDescription.hpp>
#include <com/sun/star/ui/dialogs/XFilePicker.hpp>
#include <com/sun/star/uno/XInterface.hpp>
#include <com/sun/star/frame/XComponentLoader.hpp>
#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::ui::dialogs;
using namespace ::sfx2;

class TiledRenderingApp : public Application
{
private:
    uno::Reference<uno::XComponentContext> xContext;
    uno::Reference<lang::XMultiComponentFactory> xFactory;
    uno::Reference<lang::XMultiServiceFactory> xSFactory;
    uno::Reference<uno::XInterface> xDesktop;
    uno::Reference<frame::XComponentLoader> xLoader;
    uno::Reference<lang::XComponent> xComponent;
public:
    virtual void Init() override;
    virtual int Main() override;
    void Open(OUString & aFileUrl);
};

class TiledRenderingDialog: public ModalDialog
{
private:
    TiledRenderingApp *mpApp;
    VclPtr<NumericField> mpContextWidth;
    VclPtr<NumericField> mpContextHeight;
    VclPtr<NumericField> mpTilePosX;
    VclPtr<NumericField> mpTilePosY;
    VclPtr<NumericField> mpTileWidth;
    VclPtr<NumericField> mpTileHeight;
    VclPtr<FixedImage> mpImage;

public:
    explicit TiledRenderingDialog(TiledRenderingApp * app) :
        ModalDialog(nullptr, "TiledRendering", "qa/sw/ui/tiledrendering.ui", Dialog::InitFlag::NoParent),
        mpApp(app)
    {
        VclPtr<PushButton> renderButton;
        get(renderButton, "buttonRenderTile");
        renderButton->SetClickHdl( LINK( this, TiledRenderingDialog, RenderHdl));

        VclPtr<PushButton> chooseDocumentButton;
        get(chooseDocumentButton, "buttonChooseDocument");
        chooseDocumentButton->SetClickHdl( LINK( this, TiledRenderingDialog, ChooseDocumentHdl));

        SetStyle(GetStyle()|WB_CLOSEABLE);

        get(mpContextWidth, "spinContextWidth");
        get(mpContextHeight, "spinContextHeight");
        get(mpTilePosX, "spinTilePosX");
        get(mpTilePosY, "spinTilePosY");
        get(mpTileWidth, "spinTileWidth");
        get(mpTileHeight, "spinTileHeight");
        get(mpImage, "imageTile");
    }

    virtual ~TiledRenderingDialog()
    {
        disposeOnce();
    }

    virtual void dispose() override
    {
        mpContextWidth.clear();
        mpContextHeight.clear();
        mpTilePosX.clear();
        mpTilePosY.clear();
        mpTileWidth.clear();
        mpTileHeight.clear();
        mpImage.clear();
        ModalDialog::dispose();
    }

    DECL_LINK_TYPED( RenderHdl, Button *, void );
    DECL_LINK_TYPED( ChooseDocumentHdl, Button *, void );
};

IMPL_LINK_NOARG_TYPED( TiledRenderingDialog, RenderHdl, Button *, void)
{
    int contextWidth = mpContextWidth->GetValue();
    int contextHeight = mpContextHeight->GetValue();
    int tilePosX = mpTilePosX->GetValue();
    int tilePosY = mpTilePosY->GetValue();
    long tileWidth = mpTileWidth->GetValue();
    long tileHeight = mpTileHeight->GetValue();

    // do the same thing we are doing in touch_lo_draw_tile()
    SwWrtShell *pViewShell = GetActiveWrtShell();

    if (pViewShell)
    {
        // TODO create a VirtualDevice based on SystemGraphicsData instead so
        // that we get direct rendering; something like:
        //
        // SystemGraphicsData aData;
        // [setup the aData]
        // VirtualDevice pDevice(&aData, [color depth]);
        ScopedVclPtrInstance< VirtualDevice > pDevice;

        // paint to it
        pViewShell->PaintTile(*pDevice.get(), contextWidth, contextHeight, tilePosX, tilePosY, tileWidth, tileHeight);

        // copy the pDevice content to mpImage
        Bitmap aBitmap(pDevice->GetBitmap(pDevice->PixelToLogic(Point(0,0)), pDevice->PixelToLogic(Size(contextWidth, contextHeight))));
        mpImage->SetImage(Image(aBitmap));

        // update the dialog size
        setOptimalLayoutSize();
    }
}

IMPL_LINK_NOARG_TYPED( TiledRenderingDialog,  ChooseDocumentHdl, Button *, void )
{
    FileDialogHelper aDlgHelper( TemplateDescription::FILEOPEN_SIMPLE, 0 );
    uno::Reference < XFilePicker > xFP = aDlgHelper.GetFilePicker();
    if( aDlgHelper.Execute() == ERRCODE_NONE )
    {
        OUString aFileUrl =xFP->getFiles().getConstArray()[0];
        mpApp->Open(aFileUrl);
    }
}

void TiledRenderingApp::Open(OUString & aFileUrl)
{
    static const char TARGET[] = "_default";
    static const Sequence<beans::PropertyValue> PROPS (0);
    if(xComponent.get())
    {
        xComponent->dispose();
        xComponent.clear();
    }
    xComponent.set(xLoader->loadComponentFromURL(aFileUrl, TARGET, 0, PROPS));
}

void TiledRenderingApp::Init()
{
    xContext.set(cppu::defaultBootstrap_InitialComponentContext());
    xFactory.set(xContext->getServiceManager());
    xSFactory.set(uno::Reference<lang::XMultiServiceFactory> (xFactory, uno::UNO_QUERY_THROW));
    comphelper::setProcessServiceFactory(xSFactory);

    // Create UCB (for backwards compatibility, in case some code still uses
    // plain createInstance w/o args directly to obtain an instance):
    ::ucb::UniversalContentBroker::create(comphelper::getProcessComponentContext() );

    xDesktop.set(xFactory->createInstanceWithContext("com.sun.star.frame.Desktop", xContext));
    xLoader.set(frame::Desktop::create(xContext));
}

int TiledRenderingApp::Main()
{
    if(GetCommandLineParamCount()>0)
    {
        OUString aFileUrl;
        osl::File::getFileURLFromSystemPath(GetCommandLineParam(0), aFileUrl);
        Open(aFileUrl);
    }
    Help::EnableQuickHelp();
    try
    {
        ScopedVclPtrInstance< TiledRenderingDialog > pDialog(this);
        pDialog->Execute();
    }
    catch (const uno::Exception &e)
    {
        fprintf(stderr, "fatal error: %s\n", OUStringToOString(e.Message, osl_getThreadTextEncoding()).getStr());
    }
    return EXIT_SUCCESS;
}

void vclmain::createApplication()
{
    static TiledRenderingApp aApp;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
