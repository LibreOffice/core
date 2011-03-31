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
#include <insfnote.hrc>
#include <svx/svxdlg.hxx>
#include <svx/dialogs.hrc>
#include <sfx2/viewfrm.hxx>

static sal_Bool bFootnote = sal_True;

/*------------------------------------------------------------------------
 Description:   inserting a footnote with OK
------------------------------------------------------------------------*/

void SwInsFootNoteDlg::Apply()
{
    String aStr;

    if ( aNumberCharBtn.IsChecked() )
        aStr = aNumberCharEdit.GetText();

    if ( bEdit )
    {
        rSh.StartAction();
        rSh.Left(CRSR_SKIP_CHARS, sal_False, 1, sal_False );
        rSh.StartUndo( UNDO_START );
        SwFmtFtn aNote( aEndNoteBtn.IsChecked() );
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

    bFootnote = aFtnBtn.IsChecked();
}

IMPL_LINK_NOARG_INLINE_START(SwInsFootNoteDlg, NumberCharHdl)
{
    aNumberCharEdit.GrabFocus();
    aOkBtn.Enable( aNumberCharEdit.GetText().Len() || bExtCharAvailable );
    return 0;
}
IMPL_LINK_NOARG_INLINE_END(SwInsFootNoteDlg, NumberCharHdl)

IMPL_LINK_NOARG_INLINE_START(SwInsFootNoteDlg, NumberEditHdl)
{
    aNumberCharBtn.Check( sal_True );
    aOkBtn.Enable( 0 != aNumberCharEdit.GetText().Len() );

    return 0;
}
IMPL_LINK_NOARG_INLINE_END(SwInsFootNoteDlg, NumberEditHdl)

IMPL_LINK_NOARG_INLINE_START(SwInsFootNoteDlg, NumberAutoBtnHdl)
{
    aOkBtn.Enable( sal_True );
    return 0;
}
IMPL_LINK_NOARG_INLINE_END(SwInsFootNoteDlg, NumberAutoBtnHdl)

IMPL_LINK_NOARG(SwInsFootNoteDlg, NumberExtCharHdl)
{
    aNumberCharBtn.Check( sal_True );

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
            aNumberCharEdit.SetText( sExtChars );

            if ( pFontItem )
            {
                aFontName = pFontItem->GetFamilyName();
                eCharSet  = pFontItem->GetCharSet();
                Font aFont( aFontName, pFontItem->GetStyleName(), aNumberCharEdit.GetFont().GetSize() );
                aFont.SetCharSet( pFontItem->GetCharSet() );
                aFont.SetPitch( pFontItem->GetPitch() );
                aNumberCharEdit.SetFont( aFont  );
            }

            bExtCharAvailable = sal_True;
            aOkBtn.Enable(0 != aNumberCharEdit.GetText().Len());
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
    if (pBtn == &aNextBT)
        rSh.GotoNextFtnAnchor();
    else
        rSh.GotoPrevFtnAnchor();

    Init();

    return 0;
}

SwInsFootNoteDlg::SwInsFootNoteDlg(Window *pParent, SwWrtShell &rShell, sal_Bool bEd) :

    SvxStandardDialog(pParent,SW_RES(DLG_INS_FOOTNOTE)),

    rSh(rShell),
    bExtCharAvailable(sal_False),
    bEdit(bEd),
    aNumberFL      (this,SW_RES(FL_NUMBER)),
    aNumberAutoBtn  (this,SW_RES(RB_NUMBER_AUTO)),
    aNumberCharBtn  (this,SW_RES(RB_NUMBER_CHAR)),
    aNumberCharEdit (this,SW_RES(ED_NUMBER_CHAR)),
    aNumberExtChar  (this,SW_RES(BT_NUMBER_CHAR)),

    aTypeFL        (this,SW_RES(FL_TYPE)),
    aFtnBtn         (this,SW_RES(RB_TYPE_FTN)),
    aEndNoteBtn     (this,SW_RES(RB_TYPE_ENDNOTE)),

    aOkBtn          (this,SW_RES(BT_OK)),
    aCancelBtn      (this,SW_RES(BT_CANCEL)),
    aHelpBtn        (this,SW_RES(BT_HELP)),
    aPrevBT         (this,SW_RES(BT_PREV)),
    aNextBT         (this,SW_RES(BT_NEXT))
{
    aNumberCharEdit.SetAccessibleName(String(SW_RES(STR_CHAR)));
    aNumberExtChar.SetAccessibleRelationMemberOf(&aNumberFL);
    aNumberCharEdit.SetAccessibleRelationLabeledBy(&aNumberCharBtn);

    aNumberAutoBtn.SetClickHdl(LINK(this,SwInsFootNoteDlg,NumberAutoBtnHdl));
    aNumberExtChar.SetClickHdl(LINK(this,SwInsFootNoteDlg,NumberExtCharHdl));
    aNumberCharBtn.SetClickHdl(LINK(this,SwInsFootNoteDlg,NumberCharHdl));
    aNumberCharEdit.SetModifyHdl(LINK(this,SwInsFootNoteDlg,NumberEditHdl));
    aNumberCharEdit.SetMaxTextLen(10);
    aNumberCharEdit.Enable();

    aPrevBT.SetClickHdl(LINK(this, SwInsFootNoteDlg, NextPrevHdl));
    aNextBT.SetClickHdl(LINK(this, SwInsFootNoteDlg, NextPrevHdl));

    FreeResource();
    rSh.SetCareWin(this);

    if (bEdit)
    {
        Init();

        aPrevBT.Show();
        aNextBT.Show();
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

            aFont = aNumberCharEdit.GetFont();
            aFontName = rFont.GetFamilyName();
            eCharSet = rFont.GetCharSet();
            aFont.SetName(aFontName);
            aFont.SetCharSet(eCharSet);
            bExtCharAvailable = sal_True;
            rSh.Left( CRSR_SKIP_CHARS, sal_False, 1, sal_False );
        }
        bFootnote = !aFtnNote.IsEndNote();
    }
    aNumberCharEdit.SetFont(aFont);

    sal_Bool bNumChar = sNumStr.Len() != 0;

    aNumberCharEdit.SetText(sNumStr);
    aNumberCharBtn.Check(bNumChar);
    aNumberAutoBtn.Check(!bNumChar);
    if (bNumChar)
        aNumberCharEdit.GrabFocus();

    if (bFootnote)
        aFtnBtn.Check();
    else
        aEndNoteBtn.Check();

    sal_Bool bNext = rSh.GotoNextFtnAnchor();

    if (bNext)
        rSh.GotoPrevFtnAnchor();

    sal_Bool bPrev = rSh.GotoPrevFtnAnchor();

    if (bPrev)
        rSh.GotoNextFtnAnchor();

    aPrevBT.Enable(bPrev);
    aNextBT.Enable(bNext);

    rSh.Right(CRSR_SKIP_CHARS, sal_True, 1, sal_False );

    rSh.EndAction();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
