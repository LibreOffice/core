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
#include <editeng/editids.hrc>
#include <editeng/flstitem.hxx>
#include <sfx2/sfxsids.hrc>
#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>
#include <svl/eitem.hxx>
#include <sfx2/watermarkitem.hxx>
#include <svtools/ctrltool.hxx>

SwWatermarkDialog::SwWatermarkDialog( vcl::Window* pParent, SfxBindings& rBindings )
: ModelessDialog( pParent, "WatermarkDialog", "modules/swriter/ui/watermarkdialog.ui" )
, m_rBindings( rBindings )
{
    get( m_pTextGrid, "TextGrid" );
    get( m_pEnableWatermarkCB, "EnableWatermarkCB" );
    get( m_pTextInput, "TextInput" );
    get( m_pOKButton, "ok" );
    get( m_pFont, "FontBox" );
    get( m_pAngle, "Angle" );
    get( m_pTransparency, "Transparency" );
    get( m_pColor, "Color" );

    InitFields();
    Update();
}

SwWatermarkDialog::~SwWatermarkDialog()
{
    disposeOnce();
}

void SwWatermarkDialog::dispose()
{
    m_pFont.clear();
    m_pAngle.clear();
    m_pTransparency.clear();
    m_pColor.clear();
    m_pTextGrid.clear();
    m_pEnableWatermarkCB.clear();
    m_pTextInput.clear();
    m_pOKButton.clear();

    ModelessDialog::dispose();
}

void SwWatermarkDialog::InitFields()
{
    // Update font list
    SfxObjectShell* pDocSh = SfxObjectShell::Current();
    const SfxPoolItem* pFontItem;
    const FontList* pFontList = nullptr;

    if ( pDocSh && ( ( pFontItem = pDocSh->GetItem( SID_ATTR_CHAR_FONTLIST ) ) != nullptr ) )
        pFontList = static_cast<const SvxFontListItem*>( pFontItem )->GetFontList();

    if(!pFontList)
        pFontList = new FontList(Application::GetDefaultDevice(), nullptr);

    m_pFont->Fill( pFontList );

    m_pEnableWatermarkCB->SetClickHdl( LINK( this, SwWatermarkDialog, CheckBoxHdl ) );
    m_pOKButton->SetClickHdl( LINK( this, SwWatermarkDialog, OKButtonHdl ) );

    // Get watermark properties
    const SfxPoolItem* pItem;
    SfxItemState eState = m_rBindings.GetDispatcher()->QueryState( SID_WATERMARK, pItem );

    if( eState >= SfxItemState::DEFAULT && pItem && pItem->Which() == SID_WATERMARK)
    {
        const SfxWatermarkItem* pWatermark = static_cast<const SfxWatermarkItem*>( pItem );
        OUString sText = pWatermark->GetText();
        m_pEnableWatermarkCB->Check( !sText.isEmpty() );
        m_pTextInput->SetText( sText );
        m_pFont->SelectEntryPos( m_pFont->GetEntryPos( pWatermark->GetFont() ) );
        m_pAngle->SetValue( pWatermark->GetAngle() );
        m_pColor->SelectEntry( pWatermark->GetColor() );
        m_pTransparency->SetValue( pWatermark->GetTransparency() );
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
        { "Text", css::uno::makeAny( sText ) },
        { "Font", css::uno::makeAny( m_pFont->GetSelectEntry() ) },
        { "Angle", css::uno::makeAny( static_cast<sal_Int16>( m_pAngle->GetValue() ) ) },
        { "Transparency", css::uno::makeAny( static_cast<sal_Int16>( m_pTransparency->GetValue() ) ) },
        { "Color", css::uno::makeAny( static_cast<sal_Int32>( m_pColor->GetSelectEntryColor().GetRGBColor() ) ) }
    } ) );
    comphelper::dispatchCommand( ".uno:Watermark", aPropertyValues );

    Close();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
