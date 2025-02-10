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

#include <hintids.hxx>
#include <svl/eitem.hxx>
#include <svl/stritem.hxx>
#include <editeng/fontitem.hxx>
#include <fmtftn.hxx>
#include <swundo.hxx>
#include <cmdid.h>
#include <wrtsh.hxx>
#include <insfnote.hxx>
#include <svx/svxdlg.hxx>

#include <memory>

static bool bFootnote = true;

// inserting a footnote with OK
void SwInsFootNoteDlg::Apply()
{
    OUString aStr;
    if ( m_xNumberCharBtn->get_active() )
        aStr = m_xNumberCharEdit->get_text();

    if (m_bEdit)
    {
        m_rSh.StartAction();
        m_rSh.Left(SwCursorSkipMode::Chars, false, 1, false );
        m_rSh.StartUndo( SwUndoId::START );
        SwFormatFootnote aNote( m_xEndNoteBtn->get_active() );
        aNote.SetNumStr( aStr );

        if (m_rSh.SetCurFootnote( aNote ) && m_bExtCharAvailable)
        {
            m_rSh.Right(SwCursorSkipMode::Chars, true, 1, false );
            SfxItemSetFixed<RES_CHRATR_FONT, RES_CHRATR_FONT> aSet(m_rSh.GetAttrPool());
            m_rSh.GetCurAttr(aSet);
            const SvxFontItem &rFont = aSet.Get( RES_CHRATR_FONT );
            SvxFontItem aFont( rFont.GetFamily(), m_aFontName,
                               rFont.GetStyleName(), rFont.GetPitch(),
                               m_eCharSet, RES_CHRATR_FONT );
            aSet.Put( aFont );
            m_rSh.SetAttrSet( aSet, SetAttrMode::DONTEXPAND );
            m_rSh.ResetSelect(nullptr, false);
            m_rSh.Left(SwCursorSkipMode::Chars, false, 1, false );
        }
        m_rSh.EndUndo( SwUndoId::END );
        m_rSh.EndAction();
    }

    bFootnote = m_xFootnoteBtn->get_active();
}

IMPL_LINK_NOARG(SwInsFootNoteDlg, NumberEditHdl, weld::Entry&, void)
{
    m_xNumberCharBtn->set_active(true);
    m_xOkBtn->set_sensitive( !m_xNumberCharEdit->get_text().isEmpty() );
}

IMPL_LINK(SwInsFootNoteDlg, NumberToggleHdl, weld::Toggleable&, rButton, void)
{
    if (!rButton.get_active())
        return;

    if (m_xNumberAutoBtn->get_active())
        m_xOkBtn->set_sensitive(true);
    else if (m_xNumberCharBtn->get_active())
    {
        m_xNumberCharEdit->grab_focus();
        m_xOkBtn->set_sensitive( !m_xNumberCharEdit->get_text().isEmpty() || m_bExtCharAvailable );
    }
}

IMPL_LINK_NOARG(SwInsFootNoteDlg, NumberExtCharHdl, weld::Button&, void)
{
    m_xNumberCharBtn->set_active(true);

    SfxItemSetFixed<RES_CHRATR_FONT, RES_CHRATR_FONT> aSet(m_rSh.GetAttrPool());
    m_rSh.GetCurAttr( aSet );
    const SvxFontItem &rFont = aSet.Get( RES_CHRATR_FONT );

    SfxAllItemSet aAllSet(m_rSh.GetAttrPool());
    aAllSet.Put( SfxBoolItem( FN_PARAM_1, false ) );
    aAllSet.Put( rFont );

    SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
    VclPtr<SfxAbstractDialog> pDlg(pFact->CreateCharMapDialog(m_xDialog.get(), aAllSet, nullptr));
    pDlg->StartExecuteAsync(
        [this, pDlg] (sal_Int32 nResult)->void
        {
            if (nResult == RET_OK)
            {
                const SfxStringItem* pItem = SfxItemSet::GetItem<SfxStringItem>(pDlg->GetOutputItemSet(), SID_CHARMAP, false);
                const SvxFontItem* pFontItem = SfxItemSet::GetItem<SvxFontItem>(pDlg->GetOutputItemSet(), SID_ATTR_CHAR_FONT, false);
                if ( pItem )
                {
                    m_xNumberCharEdit->set_text(pItem->GetValue());

                    if ( pFontItem )
                    {
                        m_aFontName = pFontItem->GetFamilyName();
                        m_eCharSet  = pFontItem->GetCharSet();
                        vcl::Font aFont(m_aFontName, pFontItem->GetStyleName(), m_xNumberCharEdit->get_font().GetFontSize());
                        aFont.SetCharSet( pFontItem->GetCharSet() );
                        aFont.SetPitch( pFontItem->GetPitch() );
                        m_xNumberCharEdit->set_font(aFont);
                    }

                    m_bExtCharAvailable = true;
                    m_xOkBtn->set_sensitive(!m_xNumberCharEdit->get_text().isEmpty());
                }
            }
            pDlg->disposeOnce();
        }
    );

}

IMPL_LINK( SwInsFootNoteDlg, NextPrevHdl, weld::Button&, rBtn, void )
{
    Apply();

    // go to the next foot/endnote here
    m_rSh.ResetSelect(nullptr, false);
    if (&rBtn == m_xNextBT.get())
        m_rSh.GotoNextFootnoteAnchor();
    else
        m_rSh.GotoPrevFootnoteAnchor();

    Init();
}

SwInsFootNoteDlg::SwInsFootNoteDlg(weld::Window *pParent, SwWrtShell &rShell, bool bEd)
    : GenericDialogController(pParent, u"modules/swriter/ui/insertfootnote.ui"_ustr, u"InsertFootnoteDialog"_ustr)
    , m_rSh(rShell)
    , m_eCharSet(RTL_TEXTENCODING_DONTKNOW)
    , m_bExtCharAvailable(false)
    , m_bEdit(bEd)
    , m_xNumberFrame(m_xBuilder->weld_widget(u"numberingframe"_ustr))
    , m_xNumberAutoBtn(m_xBuilder->weld_radio_button(u"automatic"_ustr))
    , m_xNumberCharBtn(m_xBuilder->weld_radio_button(u"character"_ustr))
    , m_xNumberCharEdit(m_xBuilder->weld_entry(u"characterentry"_ustr))
    , m_xNumberExtChar(m_xBuilder->weld_button(u"choosecharacter"_ustr))
    , m_xFootnoteBtn(m_xBuilder->weld_radio_button(u"footnote"_ustr))
    , m_xEndNoteBtn(m_xBuilder->weld_radio_button(u"endnote"_ustr))
    , m_xOkBtn(m_xBuilder->weld_button(u"ok"_ustr))
    , m_xPrevBT(m_xBuilder->weld_button(u"prev"_ustr))
    , m_xNextBT(m_xBuilder->weld_button(u"next"_ustr))
{
    m_xNumberAutoBtn->connect_toggled(LINK(this,SwInsFootNoteDlg,NumberToggleHdl));
    m_xNumberCharBtn->connect_toggled(LINK(this,SwInsFootNoteDlg,NumberToggleHdl));
    m_xNumberExtChar->connect_clicked(LINK(this,SwInsFootNoteDlg,NumberExtCharHdl));
    m_xNumberCharEdit->connect_changed(LINK(this,SwInsFootNoteDlg,NumberEditHdl));

    m_xPrevBT->connect_clicked(LINK(this, SwInsFootNoteDlg, NextPrevHdl));
    m_xNextBT->connect_clicked(LINK(this, SwInsFootNoteDlg, NextPrevHdl));

    SwViewShell::SetCareDialog(m_xDialog);

    if (m_bEdit)
    {
        Init();

        m_xPrevBT->show();
        m_xNextBT->show();
    }
}

SwInsFootNoteDlg::~SwInsFootNoteDlg() COVERITY_NOEXCEPT_FALSE
{
    SwViewShell::SetCareDialog(nullptr);

    if (m_bEdit)
        m_rSh.ResetSelect(nullptr, false);
}

void SwInsFootNoteDlg::Init()
{
    SwFormatFootnote aFootnoteNote;
    OUString sNumStr;
    vcl::Font aFont;
    m_bExtCharAvailable = false;

    m_rSh.StartAction();

    if (m_rSh.GetCurFootnote(&aFootnoteNote))
    {
        if (!aFootnoteNote.GetNumStr().isEmpty())
        {
            sNumStr = aFootnoteNote.GetNumStr();

            m_rSh.Right(SwCursorSkipMode::Chars, true, 1, false );
            SfxItemSetFixed<RES_CHRATR_FONT, RES_CHRATR_FONT> aSet(m_rSh.GetAttrPool());
            m_rSh.GetCurAttr(aSet);
            const SvxFontItem &rFont = aSet.Get( RES_CHRATR_FONT );
            aFont = m_xNumberCharEdit->get_font();
            m_aFontName = rFont.GetFamilyName();
            m_eCharSet = rFont.GetCharSet();
            aFont.SetFamilyName(m_aFontName);
            aFont.SetCharSet(m_eCharSet);
            m_bExtCharAvailable = true;
            m_rSh.Left( SwCursorSkipMode::Chars, false, 1, false );
        }
        bFootnote = !aFootnoteNote.IsEndNote();
    }
    m_xNumberCharEdit->set_font(aFont);

    const bool bNumChar = !sNumStr.isEmpty();

    m_xNumberCharEdit->set_text(sNumStr);
    m_xNumberCharBtn->set_active(bNumChar);
    m_xNumberAutoBtn->set_active(!bNumChar);
    if (bNumChar)
        m_xNumberCharEdit->grab_focus();

    if (bFootnote)
        m_xFootnoteBtn->set_active(true);
    else
        m_xEndNoteBtn->set_active(true);

    bool bNext = m_rSh.GotoNextFootnoteAnchor();

    if (bNext)
        m_rSh.GotoPrevFootnoteAnchor();

    bool bPrev = m_rSh.GotoPrevFootnoteAnchor();

    if (bPrev)
        m_rSh.GotoNextFootnoteAnchor();

    m_xPrevBT->set_sensitive(bPrev);
    m_xNextBT->set_sensitive(bNext);

    m_rSh.Right(SwCursorSkipMode::Chars, true, 1, false );

    m_rSh.EndAction();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
