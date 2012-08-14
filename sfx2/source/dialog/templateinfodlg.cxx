/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
* This file is part of the LibreOffice project.
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include <sfx2/templateinfodlg.hxx>

#include <comphelper/processfactory.hxx>
#include <sfx2/sfxresid.hxx>
#include <svtools/DocumentInfoPreview.hxx>
#include <toolkit/helper/vclunohelper.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/document/XDocumentProperties.hpp>
#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/task/XInteractionHandler.hpp>
#include <com/sun/star/util/URL.hpp>
#include <com/sun/star/util/URLTransformer.hpp>
#include <com/sun/star/util/XURLTransformer.hpp>

#include "templateinfodlg.hrc"

#define DLG_BORDER_SIZE 12

using namespace ::com::sun::star;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::document;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::task;
using namespace ::com::sun::star::util;

SfxTemplateInfoDlg::SfxTemplateInfoDlg (Window *pParent)
    : ModalDialog(pParent,SfxResId(DLG_TEMPLATE_INFORMATION)),
      maBtnClose(this,SfxResId(BTN_TEMPLATE_INFO_CLOSE)),
      mpPreviewView(new Window(this)),
      mpInfoView(new svtools::ODocumentInfoPreview(this,WB_LEFT | WB_VSCROLL | WB_READONLY | WB_BORDER | WB_3DLOOK))
{
    maBtnClose.SetClickHdl(LINK(this,SfxTemplateInfoDlg,CloseHdl));

    Size aWinSize = GetOutputSizePixel();
    aWinSize.setHeight( aWinSize.getHeight() - 3*DLG_BORDER_SIZE - maBtnClose.GetOutputHeightPixel() );
    aWinSize.setWidth( (aWinSize.getWidth() - 3*DLG_BORDER_SIZE)/2 );
    mpInfoView->SetPosSizePixel(Point(DLG_BORDER_SIZE,DLG_BORDER_SIZE),aWinSize);

    mpPreviewView->SetPosSizePixel(Point(aWinSize.getWidth()+2*DLG_BORDER_SIZE,DLG_BORDER_SIZE),aWinSize);

    xWindow = VCLUnoHelper::GetInterface(mpPreviewView);

    xFrame.set(comphelper::getProcessServiceFactory()->createInstance("com.sun.star.frame.Frame"), uno::UNO_QUERY );
    xFrame->initialize( xWindow );

    mpPreviewView->Show();
    mpInfoView->Show();
}

SfxTemplateInfoDlg::~SfxTemplateInfoDlg()
{
    xFrame->dispose();

    delete mpInfoView;
}

void SfxTemplateInfoDlg::loadDocument(const OUString &rURL)
{
    assert(!rURL.isEmpty());

    uno::Reference<lang::XMultiServiceFactory> xContext(comphelper::getProcessServiceFactory());

    try
    {
        uno::Reference<task::XInteractionHandler> xInteractionHandler(
                    xContext->createInstance("com.sun.star.task.InteractionHandler"), uno::UNO_QUERY );

        uno::Sequence<beans::PropertyValue> aProps(1);
        aProps[0].Name = "InteractionHandler";
        aProps[0].Value <<= xInteractionHandler;

        uno::Reference<document::XDocumentProperties> xDocProps(
                    xContext->createInstance("com.sun.star.document.DocumentProperties"), uno::UNO_QUERY );

        xDocProps->loadFromMedium( rURL, aProps );

        mpInfoView->fill( xDocProps, rURL );

        // Create template preview
        uno::Reference<util::XURLTransformer > xTrans(
                    util::URLTransformer::create(comphelper::getProcessComponentContext()));

        util::URL aURL;
        aURL.Complete = rURL;
        xTrans->parseStrict(aURL);

        uno::Reference<frame::XDispatchProvider> xProv( xFrame, uno::UNO_QUERY );

        uno::Reference<frame::XDispatch> xDisp = xProv.is() ?
            xProv->queryDispatch( aURL, "_self", 0 ) : uno::Reference<XDispatch>();

        if ( xDisp.is() )
        {
            mpPreviewView->EnableInput( false, true );

            bool b = true;
            uno::Sequence <beans::PropertyValue> aArgs( 4 );
            aArgs[0].Name = "Preview";
            aArgs[0].Value.setValue( &b, ::getBooleanCppuType() );
            aArgs[1].Name = "ReadOnly";
            aArgs[1].Value.setValue( &b, ::getBooleanCppuType() );
            aArgs[2].Name = "AsTemplate";    // prevents getting an empty URL with getURL()!
            aArgs[3].Name = "InteractionHandler";
            aArgs[3].Value <<= xInteractionHandler;

            b = false;
            aArgs[2].Value.setValue( &b, ::getBooleanCppuType() );
            xDisp->dispatch( aURL, aArgs );
        }
    }
    catch ( beans::UnknownPropertyException& )
    {
    }
    catch ( uno::Exception& )
    {
    }
}

IMPL_LINK_NOARG (SfxTemplateInfoDlg, CloseHdl)
{
    Close();
    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
