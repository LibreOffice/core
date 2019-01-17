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

SwWatermarkDialog::SwWatermarkDialog(weld::Window* pParent, SfxBindings& rBindings)
    : SfxDialogController(pParent, "modules/swriter/ui/watermarkdialog.ui", "WatermarkDialog")
    , m_rBindings(rBindings)
    , m_xTextInput(m_xBuilder->weld_entry("TextInput"))
    , m_xOKButton(m_xBuilder->weld_button("ok"))
    , m_xFont(m_xBuilder->weld_combo_box("FontBox"))
    , m_xAngle(m_xBuilder->weld_metric_spin_button("Angle", FieldUnit::DEGREE))
    , m_xTransparency(m_xBuilder->weld_metric_spin_button("Transparency", FieldUnit::PERCENT))
    , m_xColor(new ColorListBox(m_xBuilder->weld_menu_button("Color"), m_xDialog.get()))
{
    InitFields();
}

SwWatermarkDialog::~SwWatermarkDialog()
{
}

void SwWatermarkDialog::InitFields()
{
    // Update font list
    SfxObjectShell* pDocSh = SfxObjectShell::Current();
    const SfxPoolItem* pFontItem;
    const FontList* pFontList = nullptr;
    std::unique_ptr<FontList> xFontList;

    if ( pDocSh && ( ( pFontItem = pDocSh->GetItem( SID_ATTR_CHAR_FONTLIST ) ) != nullptr ) )
        pFontList = static_cast<const SvxFontListItem*>( pFontItem )->GetFontList();

    if (!pFontList)
    {
        xFontList.reset(new FontList(Application::GetDefaultDevice(), nullptr));
        pFontList = xFontList.get();
    }

    sal_uInt16 nFontCount = pFontList->GetFontNameCount();
    for (sal_uInt16 i = 0; i < nFontCount; ++i)
    {
        const FontMetric& rFontMetric = pFontList->GetFontName(i);
        m_xFont->append_text(rFontMetric.GetFamilyName());
    }

    m_xOKButton->connect_clicked(LINK(this, SwWatermarkDialog, OKButtonHdl));

    // Get watermark properties
    const SfxPoolItem* pItem;
    SfxItemState eState = m_rBindings.GetDispatcher()->QueryState( SID_WATERMARK, pItem );

    if( eState >= SfxItemState::DEFAULT && pItem && pItem->Which() == SID_WATERMARK)
    {
        const SfxWatermarkItem* pWatermark = static_cast<const SfxWatermarkItem*>( pItem );
        const OUString& sText = pWatermark->GetText();
        m_xTextInput->set_text(sText);
        m_xFont->set_entry_text(pWatermark->GetFont());
        m_xAngle->set_value(pWatermark->GetAngle(), FieldUnit::DEGREE);
        m_xColor->SelectEntry( pWatermark->GetColor() );
        m_xTransparency->set_value(pWatermark->GetTransparency(), FieldUnit::PERCENT);
    }
}

IMPL_LINK_NOARG(SwWatermarkDialog, OKButtonHdl, weld::Button&, void)
{
    OUString sText = m_xTextInput->get_text();

    css::uno::Sequence<css::beans::PropertyValue> aPropertyValues( comphelper::InitPropertySequence(
    {
        { "Text", css::uno::makeAny( sText ) },
        { "Font", css::uno::makeAny( m_xFont->get_active_text() ) },
        { "Angle", css::uno::makeAny( static_cast<sal_Int16>( m_xAngle->get_value(FieldUnit::DEGREE) ) ) },
        { "Transparency", css::uno::makeAny( static_cast<sal_Int16>( m_xTransparency->get_value(FieldUnit::PERCENT) ) ) },
        { "Color", css::uno::makeAny( static_cast<sal_uInt32>( m_xColor->GetSelectEntryColor().GetRGBColor() ) ) }
    } ) );
    comphelper::dispatchCommand( ".uno:Watermark", aPropertyValues );

    m_xDialog->response(RET_OK);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
