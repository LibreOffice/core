/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: opredlin.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: kz $ $Date: 2006-07-21 14:16:25 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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
    aContentFT      ( this, ResId(FT_CONTENT    )),
    aContentColorLB ( this, ResId(CLB_CONTENT   )),
    aMoveFT         ( this, ResId(FT_MOVE       )),
    aMoveColorLB    ( this, ResId(CLB_MOVE  )),
    aInsertFT       ( this, ResId(FT_INSERT )),
    aInsertColorLB  ( this, ResId(CLB_INSERT    )),
    aRemoveFT       ( this, ResId(FT_REMOVE )),
    aRemoveColorLB  ( this, ResId(CLB_REMOVE    )),
    aChangedGB      ( this, ResId(GB_COLORCHGS)),
    aAuthorStr      (ResId(STR_AUTHOR))
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

BOOL __EXPORT ScRedlineOptionsTabPage::FillItemSet( SfxItemSet& rSet )
{
    ScAppOptions aAppOptions=SC_MOD()->GetAppOptions();

    ULONG nNew=0;
    USHORT nPos=0;

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

    return FALSE;
}

/*-----------------------------------------------------------------------
    Beschreibung:
 -----------------------------------------------------------------------*/

void __EXPORT ScRedlineOptionsTabPage::Reset( const SfxItemSet& rSet )
{

    XColorTable* pColorTbl = XColorTable::GetStdColorTable();
    aContentColorLB.InsertEntry(aAuthorStr);
    aMoveColorLB.InsertEntry(aAuthorStr);
    aInsertColorLB.InsertEntry(aAuthorStr);
    aRemoveColorLB.InsertEntry(aAuthorStr);

    aContentColorLB.SetUpdateMode( FALSE);
    aMoveColorLB.SetUpdateMode( FALSE);
    aInsertColorLB.SetUpdateMode( FALSE);
    aRemoveColorLB.SetUpdateMode( FALSE);

    for( USHORT i = 0; i < pColorTbl->Count(); ++i )
    {
        XColorEntry* pEntry = pColorTbl->GetColor( i );
        Color aColor = pEntry->GetColor();
        String sName = pEntry->GetName();

        aContentColorLB.InsertEntry( aColor, sName );
        aMoveColorLB.InsertEntry( aColor, sName );
        aInsertColorLB.InsertEntry( aColor, sName );
        aRemoveColorLB.InsertEntry( aColor, sName );
    }
    aContentColorLB.SetUpdateMode( TRUE );
    aMoveColorLB.SetUpdateMode( TRUE );
    aInsertColorLB.SetUpdateMode( TRUE );
    aRemoveColorLB.SetUpdateMode( TRUE );


    ScAppOptions aAppOptions=SC_MOD()->GetAppOptions();

    ULONG nColor = aAppOptions.GetTrackContentColor();
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


IMPL_LINK( ScRedlineOptionsTabPage, ColorHdl, ColorListBox *, pColorLB )
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
    USHORT nPos = pLB->GetSelectEntryPos();
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

