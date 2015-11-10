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
#include <com/sun/star/document/DocumentProperties.hpp>
#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/frame/Frame.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/task/InteractionHandler.hpp>
#include <com/sun/star/util/URL.hpp>
#include <com/sun/star/util/URLTransformer.hpp>
#include <com/sun/star/util/XURLTransformer.hpp>

using namespace ::com::sun::star;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::document;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::task;
using namespace ::com::sun::star::util;

SfxTemplateInfoDlg::SfxTemplateInfoDlg (vcl::Window *pParent)
    : ModalDialog(pParent, "TemplateInfo", "sfx/ui/templateinfodialog.ui")
{
    get(mpBtnClose, "close");
    get(mpBox, "box");
    get(mpInfoView, "infoDrawingArea");
    mpPreviewView = VclPtr<vcl::Window>::Create(mpBox.get());

    Size aSize(LogicToPixel(Size(250, 160), MAP_APPFONT));
    mpBox->set_width_request(aSize.Width());
    mpBox->set_height_request(aSize.Height());

    mpBtnClose->SetClickHdl(LINK(this,SfxTemplateInfoDlg,CloseHdl));

    xWindow = VCLUnoHelper::GetInterface(mpPreviewView);

    m_xFrame = Frame::create( comphelper::getProcessComponentContext() );
    m_xFrame->initialize( xWindow );
}

SfxTemplateInfoDlg::~SfxTemplateInfoDlg()
{
    disposeOnce();
}

void SfxTemplateInfoDlg::dispose()
{
    m_xFrame->dispose();
    mpBtnClose.clear();
    mpBox.clear();
    mpPreviewView.clear();
    mpInfoView.clear();
    ModalDialog::dispose();
}

void SfxTemplateInfoDlg::loadDocument(const OUString &rURL)
{
    assert(!rURL.isEmpty());

    uno::Reference<uno::XComponentContext> xContext(comphelper::getProcessComponentContext());

    try
    {
        uno::Reference<task::XInteractionHandler2> xInteractionHandler(
            task::InteractionHandler::createWithParent(xContext, nullptr) );

        uno::Sequence<beans::PropertyValue> aProps(1);
        aProps[0].Name = "InteractionHandler";
        aProps[0].Value <<= xInteractionHandler;

        uno::Reference<document::XDocumentProperties> xDocProps(
                    document::DocumentProperties::create(comphelper::getProcessComponentContext()) );

        xDocProps->loadFromMedium( rURL, aProps );

        mpInfoView->fill( xDocProps, rURL );

        // Create template preview
        uno::Reference<util::XURLTransformer > xTrans(
                    util::URLTransformer::create(comphelper::getProcessComponentContext()));

        util::URL aURL;
        aURL.Complete = rURL;
        xTrans->parseStrict(aURL);

        uno::Reference<frame::XDispatch> xDisp = m_xFrame->queryDispatch( aURL, "_self", 0 );

        if ( xDisp.is() )
        {
            mpPreviewView->EnableInput( false );

            bool b = true;
            uno::Sequence <beans::PropertyValue> aArgs( 4 );
            aArgs[0].Name = "Preview";
            aArgs[0].Value.setValue( &b, cppu::UnoType<bool>::get() );
            aArgs[1].Name = "ReadOnly";
            aArgs[1].Value.setValue( &b, cppu::UnoType<bool>::get() );
            aArgs[2].Name = "AsTemplate";    // prevents getting an empty URL with getURL()!
            aArgs[3].Name = "InteractionHandler";
            aArgs[3].Value <<= xInteractionHandler;

            b = false;
            aArgs[2].Value.setValue( &b, cppu::UnoType<bool>::get() );
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

IMPL_LINK_NOARG_TYPED(SfxTemplateInfoDlg, CloseHdl, Button*, void)
{
    Close();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
