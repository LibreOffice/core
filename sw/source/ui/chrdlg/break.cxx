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
#include "precompiled_sw.hxx"

#ifdef SW_DLLIMPLEMENTATION
#undef SW_DLLIMPLEMENTATION
#endif


#include <sfx2/request.hxx>
#include <svl/stritem.hxx>
#ifndef _MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif

#ifndef _CMDID_H
#include <cmdid.h>
#endif
#include <uitool.hxx>
#include <swtypes.hxx>
#include <wrtsh.hxx>
#ifndef _BASESH_HXX
#include <basesh.hxx>
#endif
#ifndef _VIEW_HXX
#include <view.hxx>
#endif
#include <viewopt.hxx>
#include <break.hxx>
#include <pagedesc.hxx>
#include <poolfmt.hxx>

#ifndef _BREAK_HRC
#include <break.hrc>
#endif
#ifndef _CHRDLG_HRC
#include <chrdlg.hrc>
#endif
#include <SwStyleNameMapper.hxx>

void SwBreakDlg::Apply()
{
    nKind = 0;
    if(aLineBtn.IsChecked())
        nKind = 1;
    else if(aColumnBtn.IsChecked())
        nKind = 2;
    else if(aPageBtn.IsChecked())
    {
        nKind = 3;
        const sal_uInt16 nPos = aPageCollBox.GetSelectEntryPos();
        if(0 != nPos && LISTBOX_ENTRY_NOTFOUND != nPos)
        {
            aTemplate = aPageCollBox.GetSelectEntry();
            nPgNum = aPageNumBox.IsChecked() ? (sal_uInt16)aPageNumEdit.GetValue() : 0;
        }
    }
}


IMPL_LINK_INLINE_START( SwBreakDlg, ClickHdl, void *, EMPTYARG )
{
    CheckEnable();
    return 0;
}
IMPL_LINK_INLINE_END( SwBreakDlg, ClickHdl, void *, EMPTYARG )

/*------------------------------------------------------------------------
 Beschreibung:  Handler fuer Aendern Seitenummer
------------------------------------------------------------------------*/

IMPL_LINK_INLINE_START( SwBreakDlg, PageNumHdl, CheckBox *, pBox )
{
    if(pBox->IsChecked()) aPageNumEdit.SetValue(1);
    else aPageNumEdit.SetText(aEmptyStr);
    return 0;
}
IMPL_LINK_INLINE_END( SwBreakDlg, PageNumHdl, CheckBox *, pBox )

/*------------------------------------------------------------------------
 Beschreibung:  Durch Aendern der Seitennummer wird die Checkbox gecheckt.
------------------------------------------------------------------------*/

IMPL_LINK_INLINE_START( SwBreakDlg, PageNumModifyHdl, Edit *, EMPTYARG )
{
    aPageNumBox.Check();
    return 0;
}
IMPL_LINK_INLINE_END( SwBreakDlg, PageNumModifyHdl, Edit *, EMPTYARG )

/*------------------------------------------------------------------------
 Beschreibung:  Ok-Handler;
                prueft, ob die Seitenummer nPage eine legale Seitennummer
                ist (linke Seiten mit geraden Nummern etc. bei einer Seitenvorlage
                mit wechselnden Seiten)
------------------------------------------------------------------------*/

IMPL_LINK( SwBreakDlg, OkHdl, Button *, EMPTYARG )
{
    if(aPageNumBox.IsChecked()) {
            // wenn unterschiedliche Seitenvorlagen, testen auf Gueltigkeit
        const sal_uInt16 nPos = aPageCollBox.GetSelectEntryPos();
            // auf Position 0 steht 'Ohne'.
        const SwPageDesc *pPageDesc;
        if ( 0 != nPos && LISTBOX_ENTRY_NOTFOUND != nPos )
            pPageDesc = rSh.FindPageDescByName( aPageCollBox.GetSelectEntry(),
                                                sal_True );
        else
            pPageDesc = &rSh.GetPageDesc(rSh.GetCurPageDesc());

        ASSERT(pPageDesc, Seitenvorlage nicht gefunden.);
        const sal_uInt16 nUserPage = sal_uInt16(aPageNumEdit.GetValue());
        sal_Bool bOk = sal_True;
        switch(pPageDesc->GetUseOn())
        {
            case nsUseOnPage::PD_MIRROR:
            case nsUseOnPage::PD_ALL: break;
            case nsUseOnPage::PD_LEFT: bOk = 0 == nUserPage % 2; break;
            case nsUseOnPage::PD_RIGHT: bOk = static_cast< sal_Bool >(nUserPage % 2); break;
            default:; //prevent warning
        }
        if(!bOk) {
            InfoBox(this, SW_RES(MSG_ILLEGAL_PAGENUM)).Execute();
            aPageNumEdit.GrabFocus();
            return 0;
        }
    }
    EndDialog(RET_OK);
    return 0;
}

SwBreakDlg::SwBreakDlg( Window *pParent, SwWrtShell &rS ) :

    SvxStandardDialog( pParent,SW_RES(DLG_BREAK) ),

    rSh(rS),
    aLineBtn(this,SW_RES(RB_LINE)),
    aColumnBtn(this,SW_RES(RB_COL)),
    aPageBtn(this,SW_RES(RB_PAGE)),
    aPageCollText(this, SW_RES(FT_COLL)),
    aPageCollBox(this, SW_RES(LB_COLL)),
    aPageNumBox(this, SW_RES(CB_PAGENUM)),
    aPageNumEdit(this, SW_RES(ED_PAGENUM)),
    aBreakFL(this,SW_RES(FL_BREAK)),

    aOkBtn(this,SW_RES(BT_OK)),
    aCancelBtn(this,SW_RES(BT_CANCEL)),
    aHelpBtn(this,SW_RES(BT_HELP)),

    nKind(0),
    nPgNum(0),

    bHtmlMode(0 != ::GetHtmlMode(rS.GetView().GetDocShell()))
{
    Link aLk = LINK(this,SwBreakDlg,ClickHdl);
    aPageBtn.SetClickHdl( aLk );
    aLineBtn.SetClickHdl( aLk );
    aColumnBtn.SetClickHdl( aLk );
    aPageCollBox.SetSelectHdl( aLk );

    aOkBtn.SetClickHdl(LINK(this,SwBreakDlg,OkHdl));
    aPageNumBox.SetClickHdl(LINK(this,SwBreakDlg,PageNumHdl));
    aPageNumEdit.SetModifyHdl(LINK(this,SwBreakDlg,PageNumModifyHdl));


    // Einfuegen der vorhandenen Seitenvorlagen in die Listbox
    const sal_uInt16 nCount = rSh.GetPageDescCnt();
    sal_uInt16 i;

    for( i = 0; i < nCount; ++i)
    {
        const SwPageDesc &rPageDesc = rSh.GetPageDesc(i);
        ::InsertStringSorted(rPageDesc.GetName(), aPageCollBox, 1 );
    }

    String aFmtName;
    for(i = RES_POOLPAGE_BEGIN; i < RES_POOLPAGE_END; ++i)
        if(LISTBOX_ENTRY_NOTFOUND == aPageCollBox.GetEntryPos( aFmtName =
                                    SwStyleNameMapper::GetUIName( i, aFmtName )))
            ::InsertStringSorted(aFmtName, aPageCollBox, 1 );
    //add landscape page
    if(LISTBOX_ENTRY_NOTFOUND == aPageCollBox.GetEntryPos( aFmtName =
                                    SwStyleNameMapper::GetUIName( RES_POOLPAGE_LANDSCAPE, aFmtName )))
            ::InsertStringSorted(aFmtName, aPageCollBox, 1 );
    CheckEnable();
    aPageNumEdit.SetText( aEmptyStr );
    FreeResource();
}


void SwBreakDlg::CheckEnable()
{
    sal_Bool bEnable = sal_True;
    if ( bHtmlMode )
    {
        aColumnBtn  .Enable(sal_False);
        aPageCollBox.Enable(sal_False);
        bEnable = sal_False;
    }
    else if(rSh.GetFrmType(0,sal_True)
        & (FRMTYPE_FLY_ANY | FRMTYPE_HEADER | FRMTYPE_FOOTER  | FRMTYPE_FOOTNOTE))
    {
        aPageBtn.Enable(sal_False);
        if(aPageBtn.IsChecked())
            aLineBtn.Check(sal_True);
        bEnable = sal_False;
    }
    const sal_Bool bPage = aPageBtn.IsChecked();
    aPageCollText.Enable( bPage );
    aPageCollBox.Enable ( bPage );

    bEnable &= bPage;
    if ( bEnable )
    {
        // auf Position 0 steht 'Ohne' Seitenvorlage.
        const sal_uInt16 nPos = aPageCollBox.GetSelectEntryPos();
        if ( 0 == nPos || LISTBOX_ENTRY_NOTFOUND == nPos )
            bEnable = sal_False;
    }
    aPageNumBox .Enable(bEnable);
    aPageNumEdit.Enable(bEnable);
}

SwBreakDlg::~SwBreakDlg()
{
}
