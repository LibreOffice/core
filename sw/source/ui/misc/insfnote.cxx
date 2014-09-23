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
#include <sfx2/request.hxx>
#include <editeng/fontitem.hxx>
#include <vcl/msgbox.hxx>
#include <fmtftn.hxx>
#include <swundo.hxx>
#include <cmdid.h>
#include <wrtsh.hxx>
#include <view.hxx>
#include <basesh.hxx>
#include <insfnote.hxx>
#include <crsskip.hxx>
#include <misc.hrc>
#include <globals.hrc>
#include <svx/svxdlg.hxx>
#include <svx/dialogs.hrc>
#include <sfx2/viewfrm.hxx>
#include <vcl/layout.hxx>

#include <boost/scoped_ptr.hpp>

static bool bFootnote = true;

// inserting a footnote with OK
void SwInsFootNoteDlg::Apply()
{
    OUString aStr;
    if ( m_pNumberCharBtn->IsChecked() )
        aStr = m_pNumberCharEdit->GetText();

    if ( bEdit )
    {
        rSh.StartAction();
        rSh.Left(CRSR_SKIP_CHARS, false, 1, false );
        rSh.StartUndo( UNDO_START );
        SwFmtFtn aNote( m_pEndNoteBtn->IsChecked() );
        aNote.SetNumStr( aStr );

        if( rSh.SetCurFtn( aNote ) && bExtCharAvailable )
        {
            rSh.Right(CRSR_SKIP_CHARS, true, 1, false );
            SfxItemSet aSet( rSh.GetAttrPool(), RES_CHRATR_FONT, RES_CHRATR_FONT );
            rSh.GetCurAttr( aSet );
            SvxFontItem &rFont = (SvxFontItem &) aSet.Get( RES_CHRATR_FONT );
            SvxFontItem aFont( rFont.GetFamily(), m_aFontName,
                               rFont.GetStyleName(), rFont.GetPitch(),
                               eCharSet, RES_CHRATR_FONT );
            aSet.Put( aFont );
            rSh.SetAttrSet( aSet, nsSetAttrMode::SETATTR_DONTEXPAND );
            rSh.ResetSelect(0, false);
            rSh.Left(CRSR_SKIP_CHARS, false, 1, false );
        }
        rSh.EndUndo( UNDO_END );
        rSh.EndAction();
    }
    else
    {

    }

    bFootnote = m_pFtnBtn->IsChecked();
}

IMPL_LINK_NOARG_INLINE_START(SwInsFootNoteDlg, NumberCharHdl)
{
    m_pNumberCharEdit->GrabFocus();
    m_pOkBtn->Enable( !m_pNumberCharEdit->GetText().isEmpty() || bExtCharAvailable );
    return 0;
}
IMPL_LINK_NOARG_INLINE_END(SwInsFootNoteDlg, NumberCharHdl)

IMPL_LINK_NOARG_INLINE_START(SwInsFootNoteDlg, NumberEditHdl)
{
    m_pNumberCharBtn->Check( true );
    m_pOkBtn->Enable( !m_pNumberCharEdit->GetText().isEmpty() );

    return 0;
}
IMPL_LINK_NOARG_INLINE_END(SwInsFootNoteDlg, NumberEditHdl)

IMPL_LINK_NOARG_INLINE_START(SwInsFootNoteDlg, NumberAutoBtnHdl)
{
    m_pOkBtn->Enable( true );
    return 0;
}
IMPL_LINK_NOARG_INLINE_END(SwInsFootNoteDlg, NumberAutoBtnHdl)

IMPL_LINK_NOARG(SwInsFootNoteDlg, NumberExtCharHdl)
{
    m_pNumberCharBtn->Check( true );

    SfxItemSet aSet( rSh.GetAttrPool(), RES_CHRATR_FONT, RES_CHRATR_FONT );
    rSh.GetCurAttr( aSet );
    const SvxFontItem &rFont = (SvxFontItem &) aSet.Get( RES_CHRATR_FONT );

    SfxAllItemSet aAllSet( rSh.GetAttrPool() );
    aAllSet.Put( SfxBoolItem( FN_PARAM_1, false ) );
    aAllSet.Put( rFont );

    SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
    boost::scoped_ptr<SfxAbstractDialog> pDlg(pFact->CreateSfxDialog( this, aAllSet,
        rSh.GetView().GetViewFrame()->GetFrame().GetFrameInterface(), RID_SVXDLG_CHARMAP ));
    if (RET_OK == pDlg->Execute())
    {
        SFX_ITEMSET_ARG( pDlg->GetOutputItemSet(), pItem, SfxStringItem, SID_CHARMAP, false );
        SFX_ITEMSET_ARG( pDlg->GetOutputItemSet(), pFontItem, SvxFontItem, SID_ATTR_CHAR_FONT, false );
        if ( pItem )
        {
            m_pNumberCharEdit->SetText( pItem->GetValue() );

            if ( pFontItem )
            {
                m_aFontName = pFontItem->GetFamilyName();
                eCharSet  = pFontItem->GetCharSet();
                vcl::Font aFont( m_aFontName, pFontItem->GetStyleName(), m_pNumberCharEdit->GetFont().GetSize() );
                aFont.SetCharSet( pFontItem->GetCharSet() );
                aFont.SetPitch( pFontItem->GetPitch() );
                m_pNumberCharEdit->SetFont( aFont  );
            }

            bExtCharAvailable = true;
            m_pOkBtn->Enable(!m_pNumberCharEdit->GetText().isEmpty());
        }
    }

    return 0;
}

IMPL_LINK( SwInsFootNoteDlg, NextPrevHdl, Button *, pBtn )
{
    Apply();

    // go to the next foot/endnote here
    rSh.ResetSelect(0, false);
    if (pBtn == m_pNextBT)
        rSh.GotoNextFtnAnchor();
    else
        rSh.GotoPrevFtnAnchor();

    Init();

    return 0;
}

SwInsFootNoteDlg::SwInsFootNoteDlg(vcl::Window *pParent, SwWrtShell &rShell, bool bEd)
    : SvxStandardDialog(pParent, "InsertFootnoteDialog", "modules/swriter/ui/insertfootnote.ui")
    , rSh(rShell)
    , eCharSet(RTL_TEXTENCODING_DONTKNOW)
    , bExtCharAvailable(false)
    , bEdit(bEd)
{
    get(m_pNumberFrame, "numberingframe");
    get(m_pNumberAutoBtn, "automatic");
    get(m_pNumberCharBtn, "character");
    get(m_pNumberCharEdit, "characterentry");
    get(m_pNumberExtChar, "choosecharacter");
    get(m_pFtnBtn, "footnote");
    get(m_pEndNoteBtn, "endnote");
    get(m_pOkBtn, "ok");
    get(m_pPrevBT, "prev");
    get(m_pNextBT, "next");

    m_pNumberAutoBtn->SetClickHdl(LINK(this,SwInsFootNoteDlg,NumberAutoBtnHdl));
    m_pNumberExtChar->SetClickHdl(LINK(this,SwInsFootNoteDlg,NumberExtCharHdl));
    m_pNumberCharBtn->SetClickHdl(LINK(this,SwInsFootNoteDlg,NumberCharHdl));
    m_pNumberCharEdit->SetModifyHdl(LINK(this,SwInsFootNoteDlg,NumberEditHdl));
    m_pNumberCharEdit->SetMaxTextLen(10);
    m_pNumberCharEdit->Enable();

    m_pPrevBT->SetClickHdl(LINK(this, SwInsFootNoteDlg, NextPrevHdl));
    m_pNextBT->SetClickHdl(LINK(this, SwInsFootNoteDlg, NextPrevHdl));

    SwViewShell::SetCareWin(this);

    if (bEdit)
    {
        Init();

        m_pPrevBT->Show();
        m_pNextBT->Show();
    }
}

SwInsFootNoteDlg::~SwInsFootNoteDlg()
{
    SwViewShell::SetCareWin(0);

    if (bEdit)
        rSh.ResetSelect(0, false);
}

void SwInsFootNoteDlg::Init()
{
    SwFmtFtn aFtnNote;
    OUString sNumStr;
    vcl::Font aFont;
    bExtCharAvailable = false;

    rSh.StartAction();

    if( rSh.GetCurFtn( &aFtnNote ))
    {
        if (!aFtnNote.GetNumStr().isEmpty())
        {
            sNumStr = aFtnNote.GetNumStr();

            rSh.Right(CRSR_SKIP_CHARS, true, 1, false );
            SfxItemSet aSet( rSh.GetAttrPool(), RES_CHRATR_FONT, RES_CHRATR_FONT );
            rSh.GetCurAttr( aSet );
            const SvxFontItem &rFont = (SvxFontItem &) aSet.Get( RES_CHRATR_FONT );

            aFont = m_pNumberCharEdit->GetFont();
            m_aFontName = rFont.GetFamilyName();
            eCharSet = rFont.GetCharSet();
            aFont.SetName(m_aFontName);
            aFont.SetCharSet(eCharSet);
            bExtCharAvailable = true;
            rSh.Left( CRSR_SKIP_CHARS, false, 1, false );
        }
        bFootnote = !aFtnNote.IsEndNote();
    }
    m_pNumberCharEdit->SetFont(aFont);

    const bool bNumChar = !sNumStr.isEmpty();

    m_pNumberCharEdit->SetText(sNumStr);
    m_pNumberCharBtn->Check(bNumChar);
    m_pNumberAutoBtn->Check(!bNumChar);
    if (bNumChar)
        m_pNumberCharEdit->GrabFocus();

    if (bFootnote)
        m_pFtnBtn->Check();
    else
        m_pEndNoteBtn->Check();

    bool bNext = rSh.GotoNextFtnAnchor();

    if (bNext)
        rSh.GotoPrevFtnAnchor();

    bool bPrev = rSh.GotoPrevFtnAnchor();

    if (bPrev)
        rSh.GotoNextFtnAnchor();

    m_pPrevBT->Enable(bPrev);
    m_pNextBT->Enable(bNext);

    rSh.Right(CRSR_SKIP_CHARS, true, 1, false );

    rSh.EndAction();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
