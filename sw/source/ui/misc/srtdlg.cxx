/*************************************************************************
 *
 *  $RCSfile: srtdlg.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:14:45 $
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

#ifndef _MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif
#ifndef _SFXINTITEM_HXX //autogen
#include <svtools/intitem.hxx>
#endif
#ifndef _SFXENUMITEM_HXX //autogen
#include <svtools/eitem.hxx>
#endif
#ifndef _SFXDISPATCH_HXX //autogen
#include <sfx2/dispatch.hxx>
#endif
#ifndef _SVX_SVXIDS_HRC //autogen
#include <svx/svxids.hrc>
#endif
#ifndef _SWWAIT_HXX
#include <swwait.hxx>
#endif
#include "view.hxx"
#include "cmdid.h"
#include "wrtsh.hxx"
#include "srtdlg.hxx"
#include "sortopt.hxx"
#include "misc.hrc"
#include "srtdlg.hrc"
#include "swtable.hxx"
#include "node.hxx"
#include "tblsel.hxx"

static BOOL bCheck1 = TRUE;
static BOOL bCheck2 = FALSE;
static BOOL bCheck3 = FALSE;

static USHORT nCol1 = 1;
static USHORT nCol2 = 1;
static USHORT nCol3 = 1;

static USHORT nType1 = 0;
static USHORT nType2 = 0;
static USHORT nType3 = 0;

static BOOL   bAsc1  = TRUE;
static BOOL   bAsc2  = TRUE;
static BOOL   bAsc3  = TRUE;
static BOOL   bCol   = FALSE;
static char   nDeli  = '\t';



/*--------------------------------------------------------------------
     Beschreibung:  Fuer Tabellenselektion sel. Zeilen und Spalten
                    feststellen
 --------------------------------------------------------------------*/


BOOL lcl_GetSelTbl( SwWrtShell &rSh,USHORT& rX, USHORT& rY )
{
    const SwTableNode* pTblNd = rSh.IsCrsrInTbl();
    if( !pTblNd )
        return FALSE;

    _FndBox aFndBox( 0, 0 );

    // suche alle Boxen / Lines
    {
        SwSelBoxes aSelBoxes;
        ::GetTblSel( rSh, aSelBoxes );
        _FndPara aPara( aSelBoxes, &aFndBox );
        const SwTable& rTbl = pTblNd->GetTable();
        ((SwTableLines&)rTbl.GetTabLines()).ForEach( &_FndLineCopyCol, &aPara );
    }
    rX = aFndBox.GetLines().Count();
    if( !rX )
        return FALSE;

    rY = aFndBox.GetLines()[0]->GetBoxes().Count();
    return TRUE;
}

/*--------------------------------------------------------------------
     Beschreibung: Init-Liste
 --------------------------------------------------------------------*/



SwSortDlg::SwSortDlg(Window* pParent, SwWrtShell &rShell) :

    SvxStandardDialog(pParent, SW_RES(DLG_SORTING)),
    rSh(rShell),
    aOkBtn(this,        SW_RES(BT_OK    )),
    aCancelBtn(this,    SW_RES(BT_CANCEL)),
    aHelpBtn(this,      SW_RES(BT_HELP  )),
    aColLbl(this,       SW_RES(FT_COL   )),
    aTypLbl(this,       SW_RES(FT_KEYTYP)),
    aDirLbl(this,       SW_RES(FT_DIR   )),
    aKeyCB1(this,       SW_RES(CB_KEY1  )),
    aColEdt1(this,      SW_RES(ED_KEY1  )),
    aTypDLB1(this,      SW_RES(DLB_KEY1 )),
    aSortUpRB(this,     SW_RES(RB_UP    )),
    aSortDnRB(this,     SW_RES(RB_DN    )),
    aKeyCB2(this,       SW_RES(CB_KEY2  )),
    aColEdt2(this,      SW_RES(ED_KEY2  )),
    aTypDLB2(this,      SW_RES(DLB_KEY2 )),
    aSortUp2RB(this,    SW_RES(RB_UP2    )),
    aSortDn2RB(this,    SW_RES(RB_DN2    )),
    aKeyCB3(this,       SW_RES(CB_KEY3  )),
    aColEdt3(this,      SW_RES(ED_KEY3  )),
    aTypDLB3(this,      SW_RES(DLB_KEY3 )),
    aSortUp3RB(this,    SW_RES(RB_UP3    )),
    aSortDn3RB(this,    SW_RES(RB_DN3    )),
    aSortGrp(this,      SW_RES(GB_SORT  )),
    aColumnRB(this,     SW_RES(RB_COL   )),
    aRowRB(this,        SW_RES(RB_ROW   )),
    aDirGrp(this,       SW_RES(GB_DIR   )),
    aDelimTabRB(this,   SW_RES(RB_TAB   )),
    aDelimFreeRB(this,  SW_RES(RB_TABCH )),
    aDelimEdt(this,     SW_RES(ED_TABCH )),
    aDelimGrp(this,     SW_RES(GB_DELIM )),
    aColTxt(SW_RES(STR_COL)),
    aRowTxt(SW_RES(STR_ROW)),
    nX( 99 ),
    nY( 99 )
{
    aDelimEdt.SetMaxTextLen( 1 );
    if(rSh.GetSelectionType() &
            (SwWrtShell::SEL_TBL|SwWrtShell::SEL_TBL_CELLS) )
    {
        aColumnRB.Check(bCol);
        aColLbl.SetText(bCol ? aRowTxt : aColTxt);
        aRowRB.Check(!bCol);
        aDelimTabRB.Enable(FALSE);
        aDelimFreeRB.Enable(FALSE);
        aDelimEdt.Enable(FALSE);
    }
    else
    {
        aColumnRB.Enable(FALSE);
        aRowRB.Check(TRUE);
        aColLbl.SetText(aColTxt);
    }

    // Initialisieren
    Link aLk = LINK(this,SwSortDlg, CheckHdl);
    aKeyCB1.SetClickHdl( aLk );
    aKeyCB2.SetClickHdl( aLk );
    aKeyCB3.SetClickHdl( aLk );
    aColumnRB.SetClickHdl( aLk );
    aRowRB.SetClickHdl( aLk );

    aLk = LINK(this,SwSortDlg, DelimHdl);
    aDelimFreeRB.SetClickHdl(aLk);
    aDelimTabRB.SetClickHdl(aLk);

    aKeyCB1.Check(bCheck1);
    aKeyCB2.Check(bCheck2);
    aKeyCB3.Check(bCheck3);

    aColEdt1.SetValue(nCol1);
    aColEdt2.SetValue(nCol2);
    aColEdt3.SetValue(nCol3);

    aTypDLB1.SelectEntryPos(nType1);
    aTypDLB2.SelectEntryPos(nType2);
    aTypDLB3.SelectEntryPos(nType3);

    aSortUpRB.Check(bAsc1);
    aSortDnRB.Check(!bAsc1);
    aSortUp2RB.Check(bAsc2);
    aSortDn2RB.Check(!bAsc2);
    aSortUp3RB.Check(bAsc3);
    aSortDn3RB.Check(!bAsc3);
    aDelimTabRB.Check(nDeli == '\t');

    if(!aDelimTabRB.IsChecked())
    {
        aDelimEdt.SetText(nDeli);
        aDelimFreeRB.Check(TRUE);
        DelimHdl(&aDelimFreeRB);
    }
    else
        DelimHdl(&aDelimTabRB);

    FreeResource();
    if( ::lcl_GetSelTbl( rSh, nX, nY) )
    {
        USHORT nMax = aRowRB.IsChecked()? nY : nX;
        aColEdt1.SetMax(nMax);
        aColEdt2.SetMax(nMax);
        aColEdt3.SetMax(nMax);
    }
}




SwSortDlg::~SwSortDlg()
{
}

/*--------------------------------------------------------------------
    Beschreibung: An die Core weiterreichen
 --------------------------------------------------------------------*/
void SwSortDlg::Apply()
{
    SwSortOptions aOptions;
    if(aKeyCB1.IsChecked())
    {
        USHORT nSort = aSortUpRB.IsChecked() ?
                                (USHORT)SRT_ASCENDING : (USHORT)SRT_DESCENDING ;
        SwSortKey *pKey = new SwSortKey(aColEdt1.GetValue(),
                (SwSortKeyType)aTypDLB1.GetSelectEntryPos(),
                (SwSortOrder)nSort);
        aOptions.aKeys.C40_INSERT(SwSortKey, pKey, aOptions.aKeys.Count());
    }

    if(aKeyCB2.IsChecked())
    {
        USHORT nSort = aSortUp2RB.IsChecked() ?
                                (USHORT)SRT_ASCENDING : (USHORT)SRT_DESCENDING ;
        SwSortKey *pKey = new SwSortKey(aColEdt2.GetValue(),
                (SwSortKeyType)aTypDLB2.GetSelectEntryPos(),
                (SwSortOrder)nSort);
        aOptions.aKeys.C40_INSERT(SwSortKey, pKey, aOptions.aKeys.Count());
    }

    if(aKeyCB3.IsChecked())
    {
        USHORT nSort = aSortUp3RB.IsChecked() ?
                                (USHORT)SRT_ASCENDING : (USHORT)SRT_DESCENDING ;
        SwSortKey *pKey = new SwSortKey(aColEdt3.GetValue(),
                (SwSortKeyType)aTypDLB3.GetSelectEntryPos(),
                (SwSortOrder)nSort);
        aOptions.aKeys.C40_INSERT(SwSortKey, pKey, aOptions.aKeys.Count());
    }
    aOptions.eDirection =  aRowRB.IsChecked() ?
                        SRT_ROWS    : SRT_COLUMNS;
    sal_Unicode nDeli = '\t';
    if(!aDelimTabRB.IsChecked())
    {
        String aTmp(aDelimEdt.GetText());
        if( aTmp.Len() )
            nDeli = aTmp.GetChar( 0 );
    }
    aOptions.nDeli      =  nDeli;
    aOptions.bTable     =  rSh.IsTableMode();

    SwWait aWait( *rSh.GetView().GetDocShell(), TRUE );
    rSh.StartAllAction();
    BOOL bRet = rSh.Sort(aOptions);
    rSh.EndAllAction();
    if(bRet)
        InfoBox(this->GetParent(), SW_RES(MSG_SRTERR)).Execute();
    else
        rSh.SetModified();

    // Alte Einstellung speichern
    //
    bCheck1 = aKeyCB1.IsChecked();
    bCheck2 = aKeyCB2.IsChecked();
    bCheck3 = aKeyCB3.IsChecked();

    nCol1 = (USHORT)aColEdt1.GetValue();
    nCol2 = (USHORT)aColEdt2.GetValue();
    nCol3 = (USHORT)aColEdt3.GetValue();

    nType1 = aTypDLB1.GetSelectEntryPos();
    nType2 = aTypDLB2.GetSelectEntryPos();
    nType3 = aTypDLB3.GetSelectEntryPos();

    bAsc1 = aSortUpRB.IsChecked();
    bAsc2 = aSortUp2RB.IsChecked();
    bAsc3 = aSortUp3RB.IsChecked();
    bCol = aColumnRB.IsChecked();
}
/* -----------------30.09.98 10:03-------------------
 *
 * --------------------------------------------------*/
IMPL_LINK( SwSortDlg, DelimHdl, RadioButton*, pButton )
{
    aDelimEdt.Enable(pButton == &aDelimFreeRB && aDelimFreeRB.IsEnabled());
    return 0;
}


IMPL_LINK( SwSortDlg, CheckHdl, CheckBox *, pCheck )
{
    if( pCheck == ( CheckBox* ) &aRowRB)
    {
        aColLbl.SetText(aColTxt);
        aColEdt1.SetMax(nY);
        aColEdt2.SetMax(nY);
        aColEdt3.SetMax(nY);

    }
    else if( pCheck == ( CheckBox* ) &aColumnRB)
    {
        aColLbl.SetText(aRowTxt);
        aColEdt1.SetMax(nX);
        aColEdt2.SetMax(nX);
        aColEdt3.SetMax(nX);
    }
    else if(!aKeyCB1.IsChecked() &&
                !aKeyCB2.IsChecked() &&
                    !aKeyCB3.IsChecked())
        pCheck->Check(TRUE);
    return 0;
}

/*------------------------------------------------------------------------

    $Log: not supported by cvs2svn $
    Revision 1.58  2000/09/18 16:06:00  willem.vandorp
    OpenOffice header added.

    Revision 1.57  2000/05/26 07:21:31  os
    old SW Basic API Slots removed

    Revision 1.56  2000/02/11 14:56:48  hr
    #70473# changes for unicode ( patched by automated patchtool )

    Revision 1.55  1998/09/30 08:29:10  OS
    #52654# Sortable und SortDescriptor eingebaut, auf- und absteigend fuer jeden Schluessel


      Rev 1.54   30 Sep 1998 10:29:10   OS
   #52654# Sortable und SortDescriptor eingebaut, auf- und absteigend fuer jeden Schluessel

      Rev 1.53   07 Apr 1998 13:04:20   OM
   #49223 Infobox nicht mit hidden Parent executen

      Rev 1.52   24 Nov 1997 16:47:48   MA
   includes

      Rev 1.51   21 Nov 1997 13:50:44   OS
   richtigen Schluesseltext setzen #45697#

      Rev 1.50   03 Nov 1997 13:22:42   MA
   precomp entfernt

      Rev 1.49   25 Jul 1997 15:39:10   HJS
   includes

      Rev 1.48   11 Nov 1996 11:05:44   MA
   ResMgr

      Rev 1.47   24 Oct 1996 13:36:36   JP
   String Umstellung: [] -> GetChar()

      Rev 1.46   02 Oct 1996 18:29:32   MA
   Umstellung Enable/Disable

      Rev 1.45   11 Sep 1996 17:23:46   NF
   add: svxids.hrc

      Rev 1.44   28 Aug 1996 14:12:24   OS
   includes

      Rev 1.43   26 Jun 1996 15:25:54   OS
   Aufruf von Dispatcher.Execute an 324 angepasst

      Rev 1.42   25 Jun 1996 18:39:38   HJS
   includes

      Rev 1.41   07 Jun 1996 14:25:56   OS
   SelectionType auf SEL_TBL* testen, nicht auf SEL_TXT

      Rev 1.40   06 Feb 1996 15:21:14   JP
   Link Umstellung 305

      Rev 1.39   24 Nov 1995 16:58:46   OM
   PCH->PRECOMPILED

      Rev 1.38   13 Nov 1995 10:52:46   OM
   Neues Seg

      Rev 1.37   13 Nov 1995 10:51:34   OM
   static entfernt

      Rev 1.36   08 Nov 1995 13:31:48   JP
   Umstellung zur 301: Change -> Set

      Rev 1.35   24 Oct 1995 17:05:52   OS
   Sorting recordable und dazu ueber Slot imlementiert

      Rev 1.34   12 Sep 1995 17:30:20   OM
   Helpbutton eingefuegt


------------------------------------------------------------------------*/



