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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"

#undef SC_DLLIMPLEMENTATION



//------------------------------------------------------------------

#include <svx/dlgutil.hxx>
#include <svx/drawitem.hxx>
#include <svx/xtable.hxx>

#include "appoptio.hxx"
#include "scmod.hxx"
#include "scitems.hxx"
#include "tpview.hxx"
#include "global.hxx"
#include "viewopti.hxx"
#include "tabvwsh.hxx"
#include "uiitems.hxx"
#include "scresid.hxx"
#include "docsh.hxx"
#include "sc.hrc"       // -> Slot-IDs
#include "optdlg.hrc"
#include "globstr.hrc"

#include "opredlin.hxx"
#include "opredlin.hrc"

//------------------------------------------------------------------

ScRedlineOptionsTabPage::ScRedlineOptionsTabPage( Window* pParent,
                                                    const SfxItemSet& rSet )
    : SfxTabPage(pParent, ScResId(RID_SCPAGE_OPREDLINE), rSet),
    aContentFT      ( this, ScResId(FT_CONTENT  )),
    aContentColorLB ( this, ScResId(CLB_CONTENT )),
    aRemoveFT       ( this, ScResId(FT_REMOVE   )),
    aRemoveColorLB  ( this, ScResId(CLB_REMOVE  )),
    aInsertFT       ( this, ScResId(FT_INSERT   )),
    aInsertColorLB  ( this, ScResId(CLB_INSERT  )),
    aMoveFT         ( this, ScResId(FT_MOVE     )),
    aMoveColorLB    ( this, ScResId(CLB_MOVE    )),
    aChangedGB      ( this, ScResId(GB_COLORCHGS)),
    aAuthorStr      (ScResId(STR_AUTHOR))
{
    FreeResource();

    Link aLk = LINK(this, ScRedlineOptionsTabPage, ColorHdl);
    aContentColorLB.SetSelectHdl( aLk );
    aMoveColorLB.SetSelectHdl( aLk );
    aInsertColorLB.SetSelectHdl( aLk );
    aRemoveColorLB.SetSelectHdl( aLk );
}

/*-----------------------------------------------------------------------
    Beschreibung:
 -----------------------------------------------------------------------*/

__EXPORT ScRedlineOptionsTabPage::~ScRedlineOptionsTabPage()
{
}

/*-----------------------------------------------------------------------
    Beschreibung:
 -----------------------------------------------------------------------*/

SfxTabPage* __EXPORT ScRedlineOptionsTabPage::Create( Window* pParent, const SfxItemSet& rSet )
{
    return new ScRedlineOptionsTabPage( pParent, rSet );
}

/*-----------------------------------------------------------------------
    Beschreibung:
 -----------------------------------------------------------------------*/

sal_Bool __EXPORT ScRedlineOptionsTabPage::FillItemSet( SfxItemSet& /* rSet */ )
{
    ScAppOptions aAppOptions=SC_MOD()->GetAppOptions();

    sal_uLong nNew=0;
    sal_uInt16 nPos=0;

    nPos = aContentColorLB.GetSelectEntryPos();
    if (nPos != LISTBOX_ENTRY_NOTFOUND)
    {
        nPos = aContentColorLB.GetSelectEntryPos();
        if (nPos!=0)
            nNew= aContentColorLB.GetEntryColor(nPos).GetColor();
        else
            nNew= COL_TRANSPARENT;

        aAppOptions.SetTrackContentColor(nNew);

    }
    nPos = aMoveColorLB.GetSelectEntryPos();
    if (nPos != LISTBOX_ENTRY_NOTFOUND)
    {
        nPos = aMoveColorLB.GetSelectEntryPos();
        if (nPos!=0)
            nNew= aMoveColorLB.GetEntryColor(nPos).GetColor();
        else
            nNew= COL_TRANSPARENT;

        aAppOptions.SetTrackMoveColor(nNew);

    }
    nPos = aInsertColorLB.GetSelectEntryPos();
    if (nPos != LISTBOX_ENTRY_NOTFOUND)
    {
        nPos = aInsertColorLB.GetSelectEntryPos();
        if (nPos!=0)
            nNew= aInsertColorLB.GetEntryColor(nPos).GetColor();
        else
            nNew= COL_TRANSPARENT;

        aAppOptions.SetTrackInsertColor(nNew);

    }
    nPos = aRemoveColorLB.GetSelectEntryPos();
    if (nPos != LISTBOX_ENTRY_NOTFOUND)
    {
        nPos = aRemoveColorLB.GetSelectEntryPos();
        if (nPos!=0)
            nNew= aRemoveColorLB.GetEntryColor(nPos).GetColor();
        else
            nNew= COL_TRANSPARENT;

        aAppOptions.SetTrackDeleteColor(nNew);

    }

    SC_MOD()->SetAppOptions(aAppOptions);

    //  Repaint (wenn alles ueber Items laufen wuerde, wie es sich gehoert,
    //  waere das nicht noetig...)
    ScDocShell* pDocSh = PTR_CAST(ScDocShell, SfxObjectShell::Current());
    if (pDocSh)
        pDocSh->PostPaintGridAll();

    return sal_False;
}

/*-----------------------------------------------------------------------
    Beschreibung:
 -----------------------------------------------------------------------*/

void __EXPORT ScRedlineOptionsTabPage::Reset( const SfxItemSet& /* rSet */ )
{

    XColorTable* pColorTbl = XColorTable::GetStdColorTable();
    aContentColorLB.InsertEntry(aAuthorStr);
    aMoveColorLB.InsertEntry(aAuthorStr);
    aInsertColorLB.InsertEntry(aAuthorStr);
    aRemoveColorLB.InsertEntry(aAuthorStr);

    aContentColorLB.SetUpdateMode( sal_False);
    aMoveColorLB.SetUpdateMode( sal_False);
    aInsertColorLB.SetUpdateMode( sal_False);
    aRemoveColorLB.SetUpdateMode( sal_False);

    for( sal_uInt16 i = 0; i < pColorTbl->Count(); ++i )
    {
        XColorEntry* pEntry = pColorTbl->GetColor( i );
        Color aColor = pEntry->GetColor();
        String sName = pEntry->GetName();

        aContentColorLB.InsertEntry( aColor, sName );
        aMoveColorLB.InsertEntry( aColor, sName );
        aInsertColorLB.InsertEntry( aColor, sName );
        aRemoveColorLB.InsertEntry( aColor, sName );
    }
    aContentColorLB.SetUpdateMode( sal_True );
    aMoveColorLB.SetUpdateMode( sal_True );
    aInsertColorLB.SetUpdateMode( sal_True );
    aRemoveColorLB.SetUpdateMode( sal_True );


    ScAppOptions aAppOptions=SC_MOD()->GetAppOptions();

    sal_uLong nColor = aAppOptions.GetTrackContentColor();
    if (nColor == COL_TRANSPARENT)
        aContentColorLB.SelectEntryPos(0);
    else
        aContentColorLB.SelectEntry(Color(nColor));

    nColor = aAppOptions.GetTrackMoveColor();
    if (nColor == COL_TRANSPARENT)
        aMoveColorLB.SelectEntryPos(0);
    else
        aMoveColorLB.SelectEntry(Color(nColor));


    nColor = aAppOptions.GetTrackInsertColor();
    if (nColor == COL_TRANSPARENT)
        aInsertColorLB.SelectEntryPos(0);
    else
        aInsertColorLB.SelectEntry(Color(nColor));


    nColor = aAppOptions.GetTrackDeleteColor();
    if (nColor == COL_TRANSPARENT)
        aRemoveColorLB.SelectEntryPos(0);
    else
        aRemoveColorLB.SelectEntry(Color(nColor));

}


IMPL_LINK( ScRedlineOptionsTabPage, ColorHdl, ColorListBox *, EMPTYARG )
{
/*
    SvxFontPrevWindow *pPrev;
    ListBox *pLB;

    if (pColorLB == &aInsertColorLB)
    {
        pPrev = &aInsertPreviewWN;
        pLB = &aInsertLB;
    }
    else
    {
        pPrev = &aDeletedPreviewWN;
        pLB = &aDeletedLB;
    }

    SvxFont& rFont = pPrev->GetFont();
    sal_uInt16 nPos = pLB->GetSelectEntryPos();
    if (nPos == LISTBOX_ENTRY_NOTFOUND)
        nPos = 0;

    CharAttr *pAttr = (CharAttr *)pLB->GetEntryData(nPos);

    if (pAttr->nItemId == SID_ATTR_BRUSH)
    {
        rFont.SetColor(Color(COL_BLACK));
        nPos = pColorLB->GetSelectEntryPos();
        if (nPos && nPos != LISTBOX_ENTRY_NOTFOUND)
        {
            Brush aBrush(Color(pColorLB->GetSelectEntryColor()));
            pPrev->SetBrush(aBrush);
        }
        else
        {
            Brush aBrush(Color(COL_LIGHTGRAY));
            pPrev->SetBrush(aBrush);
        }
    }
    else
    {
        nPos = pColorLB->GetSelectEntryPos();
        if (nPos && nPos != LISTBOX_ENTRY_NOTFOUND)
            rFont.SetColor(pColorLB->GetEntryColor(nPos));
        else
            rFont.SetColor(Color(COL_RED));
    }

    pPrev->Invalidate();
*/
    return 0;
}

