/*************************************************************************
 *
 *  $RCSfile: break.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: vg $ $Date: 2003-04-17 15:15:09 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/


#pragma hdrstop

#ifndef _SFXREQUEST_HXX //autogen
#include <sfx2/request.hxx>
#endif
#ifndef _SFXSTRITEM_HXX //autogen
#include <svtools/stritem.hxx>
#endif
#ifndef _MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif

#ifndef _CMDID_H
#include <cmdid.h>
#endif
#ifndef _UITOOL_HXX
#include <uitool.hxx>
#endif
#ifndef _SWTYPES_HXX
#include <swtypes.hxx>
#endif
#ifndef _WRTSH_HXX
#include <wrtsh.hxx>
#endif
#ifndef _BASESH_HXX
#include <basesh.hxx>
#endif
#ifndef _VIEW_HXX
#include <view.hxx>
#endif
#ifndef _VIEWOPT_HXX
#include <viewopt.hxx>
#endif
#ifndef _BREAK_HXX
#include <break.hxx>
#endif
#ifndef _PAGEDESC_HXX
#include <pagedesc.hxx>
#endif
#ifndef _POOLFMT_HXX
#include <poolfmt.hxx>
#endif

#ifndef _BREAK_HRC
#include <break.hrc>
#endif
#ifndef _CHRDLG_HRC
#include <chrdlg.hrc>
#endif
#ifndef _SWSTYLENAMEMAPPER_HXX
#include <SwStyleNameMapper.hxx>
#endif

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
        const USHORT nPos = aPageCollBox.GetSelectEntryPos();
        if(0 != nPos && LISTBOX_ENTRY_NOTFOUND != nPos)
        {
            aTemplate = aPageCollBox.GetSelectEntry();
            nPgNum = aPageNumBox.IsChecked() ? (USHORT)aPageNumEdit.GetValue() : 0;
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
        const USHORT nPos = aPageCollBox.GetSelectEntryPos();
            // auf Position 0 steht 'Ohne'.
        const SwPageDesc *pPageDesc;
        if ( 0 != nPos && LISTBOX_ENTRY_NOTFOUND != nPos )
            pPageDesc = rSh.FindPageDescByName( aPageCollBox.GetSelectEntry(),
                                                TRUE );
        else
            pPageDesc = &rSh.GetPageDesc(rSh.GetCurPageDesc());

        ASSERT(pPageDesc, Seitenvorlage nicht gefunden.);
        const USHORT nUserPage = USHORT(aPageNumEdit.GetValue());
        BOOL bOk = TRUE;
        switch(pPageDesc->GetUseOn()) {
            case PD_MIRROR:
            case PD_ALL: break;
            case PD_LEFT: bOk = 0 == nUserPage % 2; break;
            case PD_RIGHT: bOk = nUserPage % 2; break;
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
    bHtmlMode(0 != ::GetHtmlMode(rS.GetView().GetDocShell())),
    nPgNum(0),
    nKind(0)
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
    const USHORT nCount = rSh.GetPageDescCnt();
    for(USHORT i = 0; i < nCount; ++i)
    {
        const SwPageDesc &rPageDesc = rSh.GetPageDesc(i);
        ::InsertStringSorted(rPageDesc.GetName(), aPageCollBox, 1 );
    }

    String aFmtName;
    for(i = RES_POOLPAGE_BEGIN; i <= RES_POOLPAGE_REGISTER; ++i)
        if(LISTBOX_ENTRY_NOTFOUND == aPageCollBox.GetEntryPos( aFmtName =
                                    SwStyleNameMapper::GetUIName( i, aFmtName )))
            ::InsertStringSorted(aFmtName, aPageCollBox, 1 );

    CheckEnable();
    aPageNumEdit.SetText( aEmptyStr );
    FreeResource();
}


void SwBreakDlg::CheckEnable()
{
    BOOL bEnable = TRUE;
    if ( bHtmlMode )
    {
        aColumnBtn  .Enable(FALSE);
        aPageCollBox.Enable(FALSE);
        bEnable = FALSE;
    }
    else if(rSh.GetFrmType(0,TRUE)
        & (FRMTYPE_FLY_ANY | FRMTYPE_HEADER | FRMTYPE_FOOTER  | FRMTYPE_FOOTNOTE))
    {
        aPageBtn.Enable(FALSE);
        if(aPageBtn.IsChecked())
            aLineBtn.Check(TRUE);
        bEnable = FALSE;
    }
    const BOOL bPage = aPageBtn.IsChecked();
    aPageCollText.Enable( bPage );
    aPageCollBox.Enable ( bPage );

    bEnable &= bPage;
    if ( bEnable )
    {
        // auf Position 0 steht 'Ohne' Seitenvorlage.
        const USHORT nPos = aPageCollBox.GetSelectEntryPos();
        if ( 0 == nPos || LISTBOX_ENTRY_NOTFOUND == nPos )
            bEnable = FALSE;
    }
    aPageNumBox .Enable(bEnable);
    aPageNumEdit.Enable(bEnable);
}

SwBreakDlg::~SwBreakDlg()
{
}
