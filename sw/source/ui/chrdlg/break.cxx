/*************************************************************************
 *
 *  $RCSfile: break.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:14:32 $
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

#ifdef PRECOMPILED
#include "ui_pch.hxx"
#endif

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


#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
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


void SwBreakDlg::Apply()
{
    String aTemplate( aEmptyStr );
    BOOL bTemplate = FALSE;
    if(aLineBtn.IsChecked())
    {
        rSh.InsertLineBreak();
    }
    else if(aColumnBtn.IsChecked())
    {
        rSh.InsertColumnBreak();
    }
    else if(aPageBtn.IsChecked())
    {
        rSh.StartAllAction();
        const USHORT nPos = aPageCollBox.GetSelectEntryPos();

        // auf Position 0 steht 'Ohne' Seitenvorlage.
        if(0 != nPos && LISTBOX_ENTRY_NOTFOUND != nPos)
        {
            aTemplate = aPageCollBox.GetSelectEntry();
            USHORT nPgNum = aPageNumBox.IsChecked() ? (USHORT)aPageNumEdit.GetValue()
                                                    : 0;
            rSh.InsertPageBreak(&aTemplate, nPgNum);
            bTemplate = TRUE;
        }
        else
            rSh.InsertPageBreak();
        rSh.EndAllAction();
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
    aBreakFrm(this,SW_RES(GB_BREAK)),
    aOkBtn(this,SW_RES(BT_OK)),
    aCancelBtn(this,SW_RES(BT_CANCEL)),
    aHelpBtn(this,SW_RES(BT_HELP)),
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
    const International& rInt = Application::GetAppInternational();
    const USHORT nCount = rSh.GetPageDescCnt();
    for(USHORT i = 0; i < nCount; ++i)
    {
        const SwPageDesc &rPageDesc = rSh.GetPageDesc(i);
        ::InsertStringSorted(rPageDesc.GetName(), aPageCollBox, TRUE, rInt);
    }

    String aFmtName;
    for(i = RES_POOLPAGE_BEGIN; i <= RES_POOLPAGE_REGISTER; ++i)
        if(LISTBOX_ENTRY_NOTFOUND == aPageCollBox.GetEntryPos(
                                    GetDocPoolNm( i, aFmtName )))
            ::InsertStringSorted(aFmtName, aPageCollBox, TRUE, rInt);

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


SwBreakDlg::~SwBreakDlg() {}

/*------------------------------------------------------------------------

    $Log: not supported by cvs2svn $
    Revision 1.62  2000/09/18 16:05:13  willem.vandorp
    OpenOffice header added.

    Revision 1.61  2000/08/25 10:12:26  os
    #77214# style listbox sorted

    Revision 1.60  2000/07/26 16:32:15  jp
    use the new function GetDocPoolNm to get the collectionames

    Revision 1.59  2000/05/26 07:21:28  os
    old SW Basic API Slots removed

    Revision 1.58  2000/05/10 11:55:37  os
    Basic API removed

    Revision 1.57  2000/02/11 14:43:29  hr
    #70473# changes for unicode ( patched by automated patchtool )

    Revision 1.56  1999/06/18 13:45:28  OS
    #66704# wrong fix fixed


      Rev 1.55   18 Jun 1999 15:45:28   OS
   #66704# wrong fix fixed

      Rev 1.54   09 Jun 1999 08:47:48   OS
   #66704# no page breaks in header/footer/frame

      Rev 1.53   05 Feb 1999 17:44:20   JP
   Task #61467#/#61014#: neu FindPageDescByName

      Rev 1.52   21 Nov 1997 12:24:34   MA
   includes

      Rev 1.51   03 Nov 1997 13:10:20   MA
   precomp entfernt

      Rev 1.50   30 Jul 1997 18:15:32   HJS
   includes

      Rev 1.49   23 Jul 1997 20:22:14   HJS
   includes

      Rev 1.48   27 Jan 1997 16:14:42   OS
   HtmlMode wird ueber GetHtmlMode ermittelt

      Rev 1.47   18 Dec 1996 19:07:30   MA
   fix: richtiges enablen

      Rev 1.46   06 Dec 1996 13:53:20   MA
   Umbrueche fuer Tabellen

      Rev 1.45   11 Nov 1996 09:16:38   MA
   ResMgr

      Rev 1.44   10 Oct 1996 13:18:52   OS
   HtmlMode beruecksichtigen

      Rev 1.43   02 Oct 1996 08:36:16   MA
   Umstellung Enable/Disable

      Rev 1.42   28 Aug 1996 08:59:06   OS
   includes

      Rev 1.41   06 Feb 1996 15:18:38   JP
   Link Umstellung 305

      Rev 1.40   24 Nov 1995 16:57:32   OM
   PCH->PRECOMPILED

      Rev 1.39   08 Nov 1995 12:47:36   OM
   Change->Set

      Rev 1.38   23 Oct 1995 17:25:10   OS
   Umbrueche einfuegen recordable

      Rev 1.37   11 Sep 1995 18:47:40   OM
   Helpbutton eingefuegt

      Rev 1.36   30 Aug 1995 14:08:52   MA
   fix: sexport'iert

      Rev 1.35   21 Aug 1995 09:22:06   MA
   chg: swstddlg -> svxstandarddialog, Optimierungen

      Rev 1.34   05 Apr 1995 09:15:28   JP
   Benutzung vom Link-Makro eingeschraenkt

      Rev 1.33   25 Oct 1994 15:50:04   ER
   add: PCH

      Rev 1.32   05 Oct 1994 17:45:56   VB
   Umbruch freigeschaltet

------------------------------------------------------------------------*/


