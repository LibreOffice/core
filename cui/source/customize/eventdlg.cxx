/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <svtools/svmedit.hxx>
#include <tools/diagnose_ex.h>
#include <com/sun/star/document/XEventsSupplier.hpp>
#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/frame/theGlobalEventBroadcaster.hpp>
#include <com/sun/star/frame/XStorable.hpp>

#include <comphelper/processfactory.hxx>
#include <comphelper/documentinfo.hxx>
#include <unotools/configmgr.hxx>
#include <rtl/ustring.hxx>

#include "eventdlg.hxx"

#include <sfx2/viewfrm.hxx>
#include <sfx2/evntconf.hxx>
#include <sfx2/minfitem.hxx>
#include <sfx2/app.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/docfac.hxx>
#include <sfx2/fcontnr.hxx>
#include <unotools/eventcfg.hxx>
#include <svtools/treelistentry.hxx>

#include <headertablistbox.hxx>
#include "macropg_impl.hxx"

#include <dialmgr.hxx>
#include <cfgutil.hxx>
#include <cfg.hxx>


using namespace ::com::sun::star;


SvxEventConfigPage::SvxEventConfigPage(vcl::Window *pParent, const SfxItemSet& rSet,
    SvxEventConfigPage::EarlyInit)
    : SvxMacroTabPage_(pParent, "EventsConfigPage",
        "cui/ui/eventsconfigpage.ui", rSet)
{
    get(m_pSaveInListBox, "savein");

    mpImpl->sStrEvent = get<FixedText>("eventft")->GetText();
    mpImpl->sAssignedMacro = get<FixedText>("actionft")->GetText();
    get(mpImpl->pEventLB, "events");
    Size aSize(LogicToPixel(Size(205, 229), MapMode(MapUnit::MapAppFont)));
    mpImpl->pEventLB->set_width_request(aSize.Width());
    mpImpl->pEventLB->set_height_request(aSize.Height());
    get(mpImpl->pAssignPB, "macro");
    get(mpImpl->pDeletePB, "delete");
    mpImpl->aMacroImg = get<FixedImage>("macroimg")->GetImage();
    mpImpl->aComponentImg = get<FixedImage>("componentimg")->GetImage();

    InitResources();

    m_pSaveInListBox->SetSelectHdl( LINK( this, SvxEventConfigPage,
                SelectHdl_Impl ) );

    uno::Reference< frame::XGlobalEventBroadcaster > xSupplier;

    xSupplier =
        frame::theGlobalEventBroadcaster::get(::comphelper::getProcessComponentContext());

    m_xAppEvents = xSupplier->getEvents();
    const sal_Int32 nPos = m_pSaveInListBox->InsertEntry(
        utl::ConfigManager::getProductName() );
    m_pSaveInListBox->SetEntryData( nPos, new bool(true) );
    m_pSaveInListBox->SelectEntryPos( nPos );
}

void SvxEventConfigPage::LateInit( const uno::Reference< frame::XFrame >& _rxFrame  )
{
    SetFrame( _rxFrame );
    ImplInitDocument();

    InitAndSetHandler( m_xAppEvents, m_xDocumentEvents, m_xDocumentModifiable );

    SelectHdl_Impl( *m_pSaveInListBox );
}

SvxEventConfigPage::~SvxEventConfigPage()
{
    disposeOnce();
}

void SvxEventConfigPage::dispose()
{
    // need to delete the user data
    SvHeaderTabListBox& rListBox = mpImpl->pEventLB->GetListBox();
    SvTreeListEntry* pE = rListBox.GetEntry( 0 );
    while( pE )
    {
        OUString const * pEventName = static_cast<OUString const *>(pE->GetUserData());
        delete pEventName;
        pE->SetUserData(nullptr);
        pE = SvTreeListBox::NextSibling( pE );
    }
    m_pSaveInListBox.clear();
    SvxMacroTabPage_::dispose();
}

void SvxEventConfigPage::ImplInitDocument()
{
    uno::Reference< frame::XFrame > xFrame( GetFrame() );
    OUString aModuleId = SvxConfigPage::GetFrameWithDefaultAndIdentify( xFrame );
    if ( !xFrame.is() )
        return;

    try
    {
        uno::Reference< frame::XModel > xModel;
        if ( !SvxConfigPage::CanConfig( aModuleId ) )
            return;

        uno::Reference< frame::XController > xController =
            xFrame->getController();

        if ( xController.is() )
        {
            xModel = xController->getModel();
        }

        if ( !xModel.is() )
            return;

        uno::Reference< document::XEventsSupplier > xSupplier( xModel, uno::UNO_QUERY );

        if ( xSupplier.is() )
        {
            m_xDocumentEvents = xSupplier->getEvents();
            m_xDocumentModifiable.set(xModel, css::uno::UNO_QUERY);

            OUString aTitle = ::comphelper::DocumentInfo::getDocumentTitle( xModel );
            const sal_Int32 nPos = m_pSaveInListBox->InsertEntry( aTitle );

            m_pSaveInListBox->SetEntryData( nPos, new bool(false) );
            m_pSaveInListBox->SelectEntryPos( nPos );
        }
    }
    catch( const uno::Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }
}

IMPL_LINK_NOARG( SvxEventConfigPage, SelectHdl_Impl, ListBox&, void )
{
    bool* bApp = static_cast<bool*>(m_pSaveInListBox->GetEntryData(
            m_pSaveInListBox->GetSelectedEntryPos()));

    mpImpl->pEventLB->SetUpdateMode( false );
    if ( *bApp )
    {
        SetReadOnly( false );
        SvxMacroTabPage_::DisplayAppEvents( true );
    }
    else
    {
        bool isReadonly = false;

        uno::Reference< frame::XDesktop2 > xFramesSupplier = frame::Desktop::create(
            ::comphelper::getProcessComponentContext() );

        uno::Reference< frame::XFrame > xFrame =
            xFramesSupplier->getActiveFrame();

        if ( xFrame.is() )
        {
            uno::Reference< frame::XController > xController =
                xFrame->getController();

            if ( xController.is() )
            {
                uno::Reference< frame::XStorable > xStorable(
                    xController->getModel(), uno::UNO_QUERY );
                isReadonly = xStorable->isReadonly();
            }
        }

        SetReadOnly( isReadonly );
        SvxMacroTabPage_::DisplayAppEvents( false );
    }

    mpImpl->pEventLB->SetUpdateMode( true );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
