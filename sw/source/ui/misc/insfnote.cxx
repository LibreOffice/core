/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

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

static sal_Bool bFootnote = sal_True;

/*------------------------------------------------------------------------
 Description:   inserting a footnote with OK
------------------------------------------------------------------------*/

void SwInsFootNoteDlg::Apply()
{
    String aStr;

    if ( m_pNumberCharBtn->IsChecked() )
        aStr = m_pNumberCharEdit->GetText();

    if ( bEdit )
    {
        rSh.StartAction();
        rSh.Left(CRSR_SKIP_CHARS, sal_False, 1, sal_False );
        rSh.StartUndo( UNDO_START );
        SwFmtFtn aNote( m_pEndNoteBtn->IsChecked() );
        aNote.SetNumStr( aStr );

        if( rSh.SetCurFtn( aNote ) && bExtCharAvailable )
        {
            rSh.Right(CRSR_SKIP_CHARS, sal_True, 1, sal_False );
            SfxItemSet aSet( rSh.GetAttrPool(), RES_CHRATR_FONT, RES_CHRATR_FONT );
            rSh.GetCurAttr( aSet );
            SvxFontItem &rFont = (SvxFontItem &) aSet.Get( RES_CHRATR_FONT );
            SvxFontItem aFont( rFont.GetFamily(), aFontName,
                               rFont.GetStyleName(), rFont.GetPitch(),
                               eCharSet, RES_CHRATR_FONT );
            aSet.Put( aFont );
            rSh.SetAttr( aSet, nsSetAttrMode::SETATTR_DONTEXPAND );
            rSh.ResetSelect(0, sal_False);
            rSh.Left(CRSR_SKIP_CHARS, sal_False, 1, sal_False );
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
    m_pOkBtn->Enable( m_pNumberCharEdit->GetText().Len() || bExtCharAvailable );
    return 0;
}
IMPL_LINK_NOARG_INLINE_END(SwInsFootNoteDlg, NumberCharHdl)

IMPL_LINK_NOARG_INLINE_START(SwInsFootNoteDlg, NumberEditHdl)
{
    m_pNumberCharBtn->Check( sal_True );
    m_pOkBtn->Enable( 0 != m_pNumberCharEdit->GetText().Len() );

    return 0;
}
IMPL_LINK_NOARG_INLINE_END(SwInsFootNoteDlg, NumberEditHdl)

IMPL_LINK_NOARG_INLINE_START(SwInsFootNoteDlg, NumberAutoBtnHdl)
{
    m_pOkBtn->Enable( sal_True );
    return 0;
}
IMPL_LINK_NOARG_INLINE_END(SwInsFootNoteDlg, NumberAutoBtnHdl)

IMPL_LINK_NOARG(SwInsFootNoteDlg, NumberExtCharHdl)
{
    m_pNumberCharBtn->Check( sal_True );

    SfxItemSet aSet( rSh.GetAttrPool(), RES_CHRATR_FONT, RES_CHRATR_FONT );
    rSh.GetCurAttr( aSet );
    const SvxFontItem &rFont = (SvxFontItem &) aSet.Get( RES_CHRATR_FONT );

    SfxAllItemSet aAllSet( rSh.GetAttrPool() );
    aAllSet.Put( SfxBoolItem( FN_PARAM_1, sal_False ) );
    aAllSet.Put( rFont );

    SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
    SfxAbstractDialog* pDlg = pFact->CreateSfxDialog( this, aAllSet,
        rSh.GetView().GetViewFrame()->GetFrame().GetFrameInterface(), RID_SVXDLG_CHARMAP );
    if (RET_OK == pDlg->Execute())
    {
        SFX_ITEMSET_ARG( pDlg->GetOutputItemSet(), pItem, SfxStringItem, SID_CHARMAP, sal_False );
        SFX_ITEMSET_ARG( pDlg->GetOutputItemSet(), pFontItem, SvxFontItem, SID_ATTR_CHAR_FONT, sal_False );
        if ( pItem )
        {
            String sExtChars(pItem->GetValue());
            m_pNumberCharEdit->SetText( sExtChars );

            if ( pFontItem )
            {
                aFontName = pFontItem->GetFamilyName();
                eCharSet  = pFontItem->GetCharSet();
                Font aFont( aFontName, pFontItem->GetStyleName(), m_pNumberCharEdit->GetFont().GetSize() );
                aFont.SetCharSet( pFontItem->GetCharSet() );
                aFont.SetPitch( pFontItem->GetPitch() );
                m_pNumberCharEdit->SetFont( aFont  );
            }

            bExtCharAvailable = sal_True;
            m_pOkBtn->Enable(0 != m_pNumberCharEdit->GetText().Len());
        }
    }
    delete pDlg;

    return 0;
}

IMPL_LINK( SwInsFootNoteDlg, NextPrevHdl, Button *, pBtn )
{
    Apply();

    // go to the next foot/endnote here
    rSh.ResetSelect(0, sal_False);
    if (pBtn == m_pNextBT)
        rSh.GotoNextFtnAnchor();
    else
        rSh.GotoPrevFtnAnchor();

    Init();

    return 0;
}

SwInsFootNoteDlg::SwInsFootNoteDlg(Window *pParent, SwWrtShell &rShell, sal_Bool bEd) :
    SvxStandardDialog(pParent, "InsertFootnoteDialog", "modules/swriter/ui/insertfootnote.ui"),
    rSh(rShell),
    bExtCharAvailable(sal_False),
    bEdit(bEd)
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

    m_pNumberCharEdit->SetAccessibleName(String(SW_RES(STR_FOOTNOTE_DIALOG_CHAR)));
    m_pNumberExtChar->SetAccessibleRelationMemberOf(m_pNumberFrame->get_label_widget());
    m_pNumberCharEdit->SetAccessibleRelationLabeledBy(m_pNumberCharBtn);

    m_pNumberAutoBtn->SetClickHdl(LINK(this,SwInsFootNoteDlg,NumberAutoBtnHdl));
    m_pNumberExtChar->SetClickHdl(LINK(this,SwInsFootNoteDlg,NumberExtCharHdl));
    m_pNumberCharBtn->SetClickHdl(LINK(this,SwInsFootNoteDlg,NumberCharHdl));
    m_pNumberCharEdit->SetModifyHdl(LINK(this,SwInsFootNoteDlg,NumberEditHdl));
    m_pNumberCharEdit->SetMaxTextLen(10);
    m_pNumberCharEdit->Enable();

    m_pPrevBT->SetClickHdl(LINK(this, SwInsFootNoteDlg, NextPrevHdl));
    m_pNextBT->SetClickHdl(LINK(this, SwInsFootNoteDlg, NextPrevHdl));

    rSh.SetCareWin(this);

    if (bEdit)
    {
        Init();

        m_pPrevBT->Show();
        m_pNextBT->Show();
    }
}

SwInsFootNoteDlg::~SwInsFootNoteDlg()
{
    rSh.SetCareWin(0);

    if (bEdit)
        rSh.ResetSelect(0, sal_False);
}

void SwInsFootNoteDlg::Init()
{
    SwFmtFtn aFtnNote;
    String sNumStr;
    Font aFont;
    bExtCharAvailable = sal_False;

    rSh.StartAction();

    if( rSh.GetCurFtn( &aFtnNote ))
    {
        if(aFtnNote.GetNumStr().Len())
        {
            sNumStr = aFtnNote.GetNumStr();

            rSh.Right(CRSR_SKIP_CHARS, sal_True, 1, sal_False );
            SfxItemSet aSet( rSh.GetAttrPool(), RES_CHRATR_FONT, RES_CHRATR_FONT );
            rSh.GetCurAttr( aSet );
            const SvxFontItem &rFont = (SvxFontItem &) aSet.Get( RES_CHRATR_FONT );

            aFont = m_pNumberCharEdit->GetFont();
            aFontName = rFont.GetFamilyName();
            eCharSet = rFont.GetCharSet();
            aFont.SetName(aFontName);
            aFont.SetCharSet(eCharSet);
            bExtCharAvailable = sal_True;
            rSh.Left( CRSR_SKIP_CHARS, sal_False, 1, sal_False );
        }
        bFootnote = !aFtnNote.IsEndNote();
    }
    m_pNumberCharEdit->SetFont(aFont);

    sal_Bool bNumChar = sNumStr.Len() != 0;

    m_pNumberCharEdit->SetText(sNumStr);
    m_pNumberCharBtn->Check(bNumChar);
    m_pNumberAutoBtn->Check(!bNumChar);
    if (bNumChar)
        m_pNumberCharEdit->GrabFocus();

    if (bFootnote)
        m_pFtnBtn->Check();
    else
        m_pEndNoteBtn->Check();

    sal_Bool bNext = rSh.GotoNextFtnAnchor();

    if (bNext)
        rSh.GotoPrevFtnAnchor();

    sal_Bool bPrev = rSh.GotoPrevFtnAnchor();

    if (bPrev)
        rSh.GotoNextFtnAnchor();

    m_pPrevBT->Enable(bPrev);
    m_pNextBT->Enable(bNext);

    rSh.Right(CRSR_SKIP_CHARS, sal_True, 1, sal_False );

    rSh.EndAction();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
