/*************************************************************************
 *
 *  $RCSfile: instable.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:14:47 $
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

#ifndef _SV_MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif

#include "wrtsh.hxx"
#include "view.hxx"
#include "itabenum.hxx"
#include "instable.hxx"
#include "tautofmt.hxx"
#include "tblafmt.hxx"
#include "modcfg.hxx"
#include "swmodule.hxx"
#ifndef _SVX_HTMLMODE_HXX //autogen
#include <svx/htmlmode.hxx>
#endif
#ifndef _VIEWOPT_HXX
#include <viewopt.hxx>
#endif

#include "table.hrc"
#include "instable.hrc"

#define ROW_COL_PROD 16384

void SwInsTableDlg::GetValues( String& rName, USHORT& rRow, USHORT& rCol,
                                USHORT& rInsTblFlags,
                                SwTableAutoFmt *& prTAFmt )
{
    rName = aNameEdit.GetText();
    rRow = (USHORT)aRowEdit.GetValue();
    rCol = (USHORT)aColEdit.GetValue();

    rInsTblFlags = 0;
    if (aBorderCB.IsChecked())
        rInsTblFlags |= DEFAULT_BORDER;
    if (aHeaderCB.IsChecked())
        rInsTblFlags |= HEADLINE;
    if (aRepeatHeaderCB.IsEnabled() && aRepeatHeaderCB.IsChecked())
        rInsTblFlags |= REPEAT;
    if (!aDontSplitCB.IsChecked())
        rInsTblFlags |= SPLIT_LAYOUT;
    if( pTAutoFmt )
        prTAFmt = new SwTableAutoFmt( *pTAutoFmt );
}

// CTOR / DTOR -----------------------------------------------------------


SwInsTableDlg::SwInsTableDlg( SwView& rView )
    : SfxModalDialog( rView.GetWindow(), SW_RES(DLG_INSERT_TABLE) ),
    aNameEdit       (this, SW_RES(ED_NAME)),
    aNameFrm        (this, SW_RES(GB_NAME)),
    aColLbl         (this, SW_RES(FT_COL)),
    aColEdit        (this, SW_RES(ED_COL)),
    aRowLbl         (this, SW_RES(FT_ROW)),
    aRowEdit        (this, SW_RES(ED_ROW)),
    aFrm            (this, SW_RES(GB_TABLE)),
    aHeaderCB       (this, SW_RES(CB_HEADER)),
    aRepeatHeaderCB (this, SW_RES(CB_REPEAT_HEADER)),
    aDontSplitCB    (this, SW_RES(CB_DONT_SPLIT)),
    aBorderCB       (this, SW_RES(CB_BORDER)),
    aOptionsGB      (this, SW_RES(GB_OPTIONS)),
    aOkBtn          (this, SW_RES(BT_OK)),
    aCancelBtn      (this, SW_RES(BT_CANCEL)),
    aHelpBtn        (this, SW_RES(BT_HELP)),
    aAutoFmtBtn     (this, SW_RES(BT_AUTOFORMAT)),
    pTAutoFmt( 0 ),
    pShell(&rView.GetWrtShell())
{
    FreeResource();
    aNameEdit.SetText(pShell->GetUniqueTblName());
    aNameEdit.SetModifyHdl(LINK(this, SwInsTableDlg, ModifyName));
    aColEdit.SetModifyHdl(LINK(this, SwInsTableDlg, ModifyRowCol));
    aRowEdit.SetModifyHdl(LINK(this, SwInsTableDlg, ModifyRowCol));

    aRowEdit.SetMax(ROW_COL_PROD/aColEdit.GetValue());
    aColEdit.SetMax(ROW_COL_PROD/aRowEdit.GetValue());
    aAutoFmtBtn.SetClickHdl(LINK(this, SwInsTableDlg, AutoFmtHdl));

    BOOL bHTMLMode = 0 != (::GetHtmlMode(rView.GetDocShell())&HTMLMODE_ON);
    const SwModuleOptions* pModOpt = SW_MOD()->GetModuleConfig();

    USHORT nInsTblFlags = pModOpt->GetInsTblFlags(bHTMLMode);

    aHeaderCB.Check(nInsTblFlags & HEADLINE);
    aRepeatHeaderCB.Check(nInsTblFlags & REPEAT);
    aDontSplitCB.Check(!(nInsTblFlags & SPLIT_LAYOUT));
    aBorderCB.Check(nInsTblFlags & DEFAULT_BORDER);

    aHeaderCB.SetClickHdl(LINK(this, SwInsTableDlg, CheckBoxHdl));
    CheckBoxHdl();
}

SwInsTableDlg::~SwInsTableDlg()
{
    delete pTAutoFmt;
}

IMPL_LINK_INLINE_START( SwInsTableDlg, ModifyName, Edit *, pEdit )
{
    String sTblName = pEdit->GetText();
    if(sTblName.Search(' ') != STRING_NOTFOUND)
    {
        sTblName.EraseAllChars( );
        pEdit->SetText(sTblName);
    }

    aOkBtn.Enable(pShell->GetTblStyle( sTblName ) == 0);
    return 0;
}
IMPL_LINK_INLINE_END( SwInsTableDlg, ModifyName, Edit *, EMPTYARG )

/*-----------------15.04.98 11:36-------------------

--------------------------------------------------*/
IMPL_LINK( SwInsTableDlg, ModifyRowCol, NumericField *, pField )
{
    if(pField == &aColEdit)
    {
        long nCol = aColEdit.GetValue();
        aRowEdit.SetMax(ROW_COL_PROD/nCol);
    }
    else
    {
        long nRow = aRowEdit.GetValue();
        aColEdit.SetMax(ROW_COL_PROD/nRow);
    }
    return 0;
}

IMPL_LINK( SwInsTableDlg, AutoFmtHdl, PushButton*, pButton )
{
    SwAutoFormatDlg aDlg( pButton, pShell, FALSE, pTAutoFmt );
    if( RET_OK == aDlg.Execute())
        aDlg.FillAutoFmtOfIndex( pTAutoFmt );
    return 0;
}

/*********************************************************************/
/*                                                                   */
/*********************************************************************/

IMPL_LINK(SwInsTableDlg, CheckBoxHdl, CheckBox*, EMPTYARG)
{
    aRepeatHeaderCB.Enable(aHeaderCB.IsChecked());

    return 0;
}

/*------------------------------------------------------------------------

    $Log: not supported by cvs2svn $
    Revision 1.49  2000/09/18 16:06:08  willem.vandorp
    OpenOffice header added.

    Revision 1.48  2000/02/11 14:58:35  hr
    #70473# changes for unicode ( patched by automated patchtool )

    Revision 1.47  1999/02/17 07:39:44  OS
    #58158# Einfuegen TabPage auch in HTML-Docs


      Rev 1.46   17 Feb 1999 08:39:44   OS
   #58158# Einfuegen TabPage auch in HTML-Docs

      Rev 1.45   02 Dec 1998 15:29:26   OM
   #59770# Tabellenueberschrift nur auf erster Seite

      Rev 1.44   30 Nov 1998 17:33:40   OM
   #59770# Tabellenoptionen: Ueberschrift nur auf erster Seite

      Rev 1.43   11 Nov 1998 16:51:58   OM
   #58158# Einfuegeoptionen fuer Tabellen

      Rev 1.42   29 May 1998 18:55:32   JP
   SS vom TableAutoFormatDialog hat sich geaendert

      Rev 1.41   28 May 1998 11:48:08   OS
   Leerzeichen auch aus dem Clipboard abfangen #50497#

      Rev 1.40   12 May 1998 23:42:44   JP
   neu: InserTable/TextToTable mit optionalen AutoFormat

      Rev 1.39   24 Apr 1998 17:25:26   OS
   Min/Max schon initial setzen

      Rev 1.38   15 Apr 1998 14:30:44   OS
   Zeilen/Spaltenzahl nur noch durch max. Zellenzahl begrenzt #49479#

      Rev 1.37   24 Nov 1997 15:52:22   MA
   includes

      Rev 1.36   03 Nov 1997 13:56:50   MA
   precomp entfernt

      Rev 1.35   06 Aug 1997 14:43:50   TRI
   VCL: GetpApp() statt pApp

      Rev 1.34   11 Nov 1996 11:20:14   MA
   ResMgr

      Rev 1.33   02 Oct 1996 19:06:06   MA
   Umstellung Enable/Disable

      Rev 1.32   28 Aug 1996 14:33:16   OS
   includes

      Rev 1.31   26 Jun 1996 15:25:20   OS
   Aufruf von Dispatcher.Execute an 324 angepasst

      Rev 1.30   22 Mar 1996 14:16:32   HJS
   umstellung 311

      Rev 1.29   06 Feb 1996 15:21:22   JP
   Link Umstellung 305

      Rev 1.28   24 Nov 1995 16:58:58   OM
   PCH->PRECOMPILED

      Rev 1.27   10 Nov 1995 15:26:46   OS
   Execute mit 0L abschliessen

      Rev 1.26   08 Nov 1995 13:34:40   OS
   Change => Set

      Rev 1.25   08 Nov 1995 07:12:18   OS
   Klammern verschoben

      Rev 1.24   07 Nov 1995 18:08:40   OS
   InsertTable ohne Manager, ausf. ueber Slot

      Rev 1.23   18 Oct 1995 10:12:54   OM
   Helpbutton fuer >Tabelle einfuegen<

      Rev 1.22   13 Sep 1995 17:09:44   OS
   Recording ausgeweitet

------------------------------------------------------------------------*/



