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

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/document/XDocumentProperties.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/task/XInteractionHandler.hpp>

#include "templateinfodlg.hrc"

#define DLG_BORDER_SIZE 12

using namespace ::com::sun::star;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::document;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::task;

SfxTemplateInfoDlg::SfxTemplateInfoDlg (Window *pParent)
    : ModalDialog(pParent,SfxResId(DLG_TEMPLATE_INFORMATION)),
      maBtnClose(this,SfxResId(BTN_TEMPLATE_INFO_CLOSE)),
      mpInfoView(new svtools::ODocumentInfoPreview(this,WB_LEFT | WB_VSCROLL | WB_READONLY | WB_BORDER | WB_3DLOOK))
{
    Size aWinSize = GetOutputSizePixel();
    aWinSize.setHeight( aWinSize.getHeight() - 3*DLG_BORDER_SIZE - maBtnClose.GetOutputHeightPixel() );
    aWinSize.setWidth( (aWinSize.getWidth() - 3*DLG_BORDER_SIZE)/2 );
    mpInfoView->SetPosSizePixel(Point(DLG_BORDER_SIZE,DLG_BORDER_SIZE),aWinSize);
    mpInfoView->Show();
}

SfxTemplateInfoDlg::~SfxTemplateInfoDlg()
{
    delete mpInfoView;
}

void SfxTemplateInfoDlg::loadDocument(const OUString &rURL)
{
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
    }
    catch ( beans::UnknownPropertyException& )
    {
    }
    catch ( uno::Exception& )
    {
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
