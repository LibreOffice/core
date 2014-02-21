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
#include <com/sun/star/frame/XModuleManager.hpp>

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

#include "headertablistbox.hxx"
#include "macropg_impl.hxx"

#include <dialmgr.hxx>
#include <cuires.hrc>
#include "eventdlg.hrc"
#include "helpid.hrc"
#include "selector.hxx"
#include "cfg.hxx"


using namespace ::com::sun::star;
// -----------------------------------------------------------------------

SvxEventConfigPage::SvxEventConfigPage( Window *pParent, const SfxItemSet& rSet, SvxEventConfigPage::EarlyInit ) :

    _SvxMacroTabPage( pParent, CUI_RES(RID_SVXPAGE_EVENTS), rSet ),
    aSaveInText( this, CUI_RES( TXT_SAVEIN ) ),
    aSaveInListBox( this, CUI_RES( LB_SAVEIN ) ),
    bAppConfig  ( sal_True )
{
    mpImpl->sStrEvent           = OUString( CUI_RES( STR_EVENT ));
    mpImpl->sAssignedMacro      = OUString( CUI_RES( STR_ASSMACRO ));
    mpImpl->pEventLB            = new MacroEventListBox( this, CUI_RES( LB_EVENT ));
    mpImpl->pAssignFT           = new FixedText( this,  CUI_RES( FT_ASSIGN ));
    mpImpl->pAssignPB           = new PushButton( this, CUI_RES( PB_ASSIGN ));
    mpImpl->pDeletePB           = new PushButton( this, CUI_RES( PB_DELETE ));
    mpImpl->aMacroImg           = Image( CUI_RES( IMG_MACRO) );
    mpImpl->aComponentImg       = Image( CUI_RES( IMG_COMPONENT) );

    FreeResource();

    // must be done after FreeResource is called
    InitResources();

    mpImpl->pEventLB->GetListBox().SetHelpId( HID_SVX_MACRO_LB_EVENT );

    aSaveInListBox.SetSelectHdl( LINK( this, SvxEventConfigPage,
                SelectHdl_Impl ) );

    uno::Reference< frame::XGlobalEventBroadcaster > xSupplier;

    xSupplier =
        frame::theGlobalEventBroadcaster::get(::comphelper::getProcessComponentContext());

    sal_uInt16 nPos(0);
    m_xAppEvents = xSupplier->getEvents();
    nPos = aSaveInListBox.InsertEntry(
        utl::ConfigManager::getProductName() );
    aSaveInListBox.SetEntryData( nPos, new bool(true) );
    aSaveInListBox.SelectEntryPos( nPos, true );
}

// -----------------------------------------------------------------------
void SvxEventConfigPage::LateInit( const uno::Reference< frame::XFrame >& _rxFrame  )
{
    SetFrame( _rxFrame );
    ImplInitDocument();

    InitAndSetHandler( m_xAppEvents, m_xDocumentEvents, m_xDocumentModifiable );

    SelectHdl_Impl( NULL );
}

// -----------------------------------------------------------------------

SvxEventConfigPage::~SvxEventConfigPage()
{
    // need to delete the user data
    SvHeaderTabListBox& rListBox = mpImpl->pEventLB->GetListBox();
    SvTreeListEntry* pE = rListBox.GetEntry( 0 );
    while( pE )
    {
        OUString* pEventName = (OUString*)pE->GetUserData();
        delete pEventName;
        pE->SetUserData((void*)0);
        pE = rListBox.NextSibling( pE );
    }
    delete mpImpl->pEventLB;

    delete mpImpl->pAssignFT;
    delete mpImpl->pAssignPB;
    delete mpImpl->pDeletePB;
}

// -----------------------------------------------------------------------

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
            m_xDocumentModifiable = m_xDocumentModifiable.query( xModel );

            OUString aTitle = ::comphelper::DocumentInfo::getDocumentTitle( xModel );
            sal_uInt16 nPos = aSaveInListBox.InsertEntry( aTitle );

            aSaveInListBox.SetEntryData( nPos, new bool(false) );
            aSaveInListBox.SelectEntryPos( nPos, true );

            bAppConfig = false;
        }
    }
    catch( const uno::Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }
}

// -----------------------------------------------------------------------

IMPL_LINK( SvxEventConfigPage, SelectHdl_Impl, ListBox *, pBox )
{
    (void)pBox;

    bool* bApp = (bool*) aSaveInListBox.GetEntryData(
            aSaveInListBox.GetSelectEntryPos());

    mpImpl->pEventLB->SetUpdateMode( false );
    bAppConfig = *bApp;
    if ( *bApp )
    {
        SetReadOnly( sal_False );
        _SvxMacroTabPage::DisplayAppEvents( true );
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
        _SvxMacroTabPage::DisplayAppEvents( false );
    }

    mpImpl->pEventLB->SetUpdateMode( true );
    return sal_True;
}

// -----------------------------------------------------------------------

sal_Bool SvxEventConfigPage::FillItemSet( SfxItemSet& rSet )
{
    return _SvxMacroTabPage::FillItemSet( rSet );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
