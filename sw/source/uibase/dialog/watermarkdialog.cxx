/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <watermarkdialog.hxx>
#include <comphelper/propertysequence.hxx>
#include <comphelper/dispatchcommand.hxx>
#include <sfx2/sfxsids.hrc>
#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>
#include <svl/eitem.hxx>
#include <sfx2/watermarkitem.hxx>

SwWatermarkDialog::SwWatermarkDialog( vcl::Window* pParent, SfxBindings& rBindings )
: ModelessDialog( pParent, "WatermarkDialog", "modules/swriter/ui/watermarkdialog.ui" )
, m_rBindings( rBindings )
{
    get( m_pTextGrid, "TextGrid" );
    get( m_pEnableWatermarkCB, "EnableWatermarkCB" );
    get( m_pTextInput, "TextInput" );
    get( m_pOKButton, "ok" );

    m_pEnableWatermarkCB->SetClickHdl( LINK( this, SwWatermarkDialog, CheckBoxHdl ) );
    m_pOKButton->SetClickHdl( LINK( this, SwWatermarkDialog, OKButtonHdl ) );

    InitFields();
    Update();
}

SwWatermarkDialog::~SwWatermarkDialog()
{
    disposeOnce();
}

void SwWatermarkDialog::dispose()
{
    m_pTextGrid.clear();
    m_pEnableWatermarkCB.clear();
    m_pTextInput.clear();
    m_pOKButton.clear();

    ModelessDialog::dispose();
}

void SwWatermarkDialog::InitFields()
{
    const SfxPoolItem* pItem;
    SfxItemState eState = m_rBindings.GetDispatcher()->QueryState( SID_WATERMARK, pItem );

    if( eState >= SfxItemState::DEFAULT && pItem )
    {
        OUString sText = static_cast<const SfxWatermarkItem*>( pItem )->GetText();
        m_pEnableWatermarkCB->Check( !sText.isEmpty() );
        m_pTextInput->SetText( sText );
    }
}

void SwWatermarkDialog::Update()
{
    if( m_pEnableWatermarkCB->IsChecked() )
        m_pTextGrid->Enable();
    else
        m_pTextGrid->Disable();
}

IMPL_LINK_NOARG( SwWatermarkDialog, CheckBoxHdl, Button*, void )
{
    Update();
}

IMPL_LINK_NOARG( SwWatermarkDialog, OKButtonHdl, Button*, void )
{
    OUString sText = "";
    if( m_pEnableWatermarkCB->IsChecked() )
        sText = m_pTextInput->GetText();

    css::uno::Sequence<css::beans::PropertyValue> aPropertyValues( comphelper::InitPropertySequence(
    {
        { "Text", css::uno::makeAny( sText ) }
    } ) );
    comphelper::dispatchCommand( ".uno:Watermark", aPropertyValues );

    Close();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
