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
#include <sfx2/objsh.hxx>
#include <vcl/svapp.hxx>
#include <sfx2/watermarkitem.hxx>
#include <svtools/ctrltool.hxx>
#include <comphelper/lok.hxx>
#include <sfx2/viewsh.hxx>
#include <svl/itemset.hxx>

static bool isLOKMobilePhone()
{
    if (!comphelper::LibreOfficeKit::isActive())
        return false;
    SfxViewShell* pCurrent = SfxViewShell::Current();
    return pCurrent && pCurrent->isLOKMobilePhone();
}

SwWatermarkDialog::SwWatermarkDialog(weld::Window* pParent, SfxBindings& rBindings)
    : SfxDialogController(pParent, u"modules/swriter/ui/watermarkdialog.ui"_ustr, u"WatermarkDialog"_ustr)
    , m_rBindings(rBindings)
    , m_xTextInput(m_xBuilder->weld_entry(u"TextInput"_ustr))
    , m_xOKButton(m_xBuilder->weld_button(u"ok"_ustr))
    , m_xFont(m_xBuilder->weld_combo_box(u"FontBox"_ustr))
    , m_xAngle(m_xBuilder->weld_metric_spin_button(u"Angle"_ustr, FieldUnit::DEGREE))
    , m_xTransparency(m_xBuilder->weld_metric_spin_button(u"Transparency"_ustr, FieldUnit::PERCENT))
    , m_xColor(new ColorListBox(m_xBuilder->weld_menu_button(u"Color"_ustr), [this]{ return m_xDialog.get(); }))
{
    InitFields();

    if (isLOKMobilePhone())
    {
        m_xBuilder->weld_label(u"ColorLabel"_ustr)->hide();
        m_xColor->hide();
        m_xBuilder->weld_button(u"cancel"_ustr)->hide();
        m_xBuilder->weld_button(u"help"_ustr)->hide();
    }
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

    m_xFont->freeze();
    sal_uInt16 nFontCount = pFontList->GetFontNameCount();
    for (sal_uInt16 i = 0; i < nFontCount; ++i)
    {
        const FontMetric& rFontMetric = pFontList->GetFontName(i);
        m_xFont->append_text(rFontMetric.GetFamilyName());
    }
    m_xFont->thaw();

    m_xOKButton->connect_clicked(LINK(this, SwWatermarkDialog, OKButtonHdl));

    // Get watermark properties
    SfxPoolItemHolder aResult;
    const SfxItemState eState(m_rBindings.GetDispatcher()->QueryState(SID_WATERMARK, aResult));
    const SfxWatermarkItem* pWatermark(static_cast<const SfxWatermarkItem*>(aResult.getItem()));

    if( !(eState >= SfxItemState::DEFAULT && pWatermark && pWatermark->Which() == SID_WATERMARK))
        return;

    const OUString& sText = pWatermark->GetText();
    m_xTextInput->set_text(sText);
    OUString sFontName = pWatermark->GetFont();
    int nFontIndex = m_xFont->find_text(sFontName);
    if (nFontIndex != -1)
        m_xFont->set_active(nFontIndex);
    else
        m_xFont->set_entry_text(sFontName);
    m_xAngle->set_value(pWatermark->GetAngle(), FieldUnit::DEGREE);
    m_xColor->SelectEntry( pWatermark->GetColor() );
    m_xTransparency->set_value(pWatermark->GetTransparency(), FieldUnit::PERCENT);
}

IMPL_LINK_NOARG(SwWatermarkDialog, OKButtonHdl, weld::Button&, void)
{
    OUString sText = m_xTextInput->get_text();

    css::uno::Sequence<css::beans::PropertyValue> aPropertyValues( comphelper::InitPropertySequence(
    {
        { "Text", css::uno::Any( sText ) },
        { "Font", css::uno::Any( m_xFont->get_active_text() ) },
        { "Angle", css::uno::Any( static_cast<sal_Int16>( m_xAngle->get_value(FieldUnit::DEGREE) ) ) },
        { "Transparency", css::uno::Any( static_cast<sal_Int16>( m_xTransparency->get_value(FieldUnit::PERCENT) ) ) },
        { "Color", css::uno::Any( static_cast<sal_uInt32>( m_xColor->GetSelectEntryColor().GetRGBColor() ) ) }
    } ) );
    comphelper::dispatchCommand( u".uno:Watermark"_ustr, aPropertyValues );

    m_xDialog->response(RET_OK);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
