/*************************************************************************
 *
 *  $RCSfile: convert.cxx,v $
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
#ifndef _SFXSTRITEM_HXX //autogen
#include <svtools/stritem.hxx>
#endif
#ifndef _SFXVIEWFRM_HXX //autogen
#include <sfx2/viewfrm.hxx>
#endif

#ifndef _MODOPT_HXX //autogen
#include <modcfg.hxx>
#endif
#ifndef _SVX_HTMLMODE_HXX //autogen
#include <svx/htmlmode.hxx>
#endif
#ifndef _VIEWOPT_HXX
#include <viewopt.hxx>
#endif
#include "swmodule.hxx"
#include "cmdid.h"
#include "convert.hxx"
#include "tablemgr.hxx"
#include "wrtsh.hxx"
#include "view.hxx"
#include "tautofmt.hxx"
#include "tblafmt.hxx"

#include "table.hrc"
#include "convert.hrc"


void SwConvertTableDlg::GetValues(  char& rDelim,
                                    USHORT& rInsTblFlags,
                                    SwTableAutoFmt *& prTAFmt )
{
    if( aTabBtn.IsChecked() )
        rDelim = aKeepColumn.IsChecked() ? 0x09 : 0x0b;
    else if( aSemiBtn.IsChecked() )
        rDelim = ';';
    else if( aOtherBtn.IsChecked() && aOtherEd.GetText().Len() )
        rDelim = aOtherEd.GetText().GetChar( 0 );
    else
        rDelim = cParaDelim;

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


SwConvertTableDlg::SwConvertTableDlg( SwView& rView )

    : SfxModalDialog( &rView.GetViewFrame()->GetWindow(), SW_RES(DLG_CONV_TEXT_TABLE)),

    aTabBtn         (this, SW_RES(CB_TAB)),
    aSemiBtn        (this, SW_RES(CB_SEMI)),
    aParaBtn        (this, SW_RES(CB_PARA)),
    aDelimFrm       (this, SW_RES(GB_DELIM)),
    aOtherBtn       (this, SW_RES(RB_OTHER)),
    aOtherEd        (this, SW_RES(ED_OTHER)),
    aKeepColumn     (this, SW_RES(CB_KEEPCOLUMN)),
    aHeaderCB       (this, SW_RES(CB_HEADER)),
    aRepeatHeaderCB (this, SW_RES(CB_REPEAT_HEADER)),
    aDontSplitCB    (this, SW_RES(CB_DONT_SPLIT)),
    aBorderCB       (this, SW_RES(CB_BORDER)),
    aOptionsGB      (this, SW_RES(GB_OPTIONS)),

    aOkBtn(this,SW_RES(BT_OK)),
    aCancelBtn(this,SW_RES(BT_CANCEL)),
    aHelpBtn(this, SW_RES(BT_HELP)),
    aAutoFmtBtn(this,SW_RES(BT_AUTOFORMAT)),

    pTAutoFmt( 0 ),
    pShell( &rView.GetWrtShell() ),
    sConvertTextTable(SW_RES(STR_CONVERT_TEXT_TABLE))
{
    FreeResource();
    if( 0 == pShell->GetTableFmt() )
    {
        SetText( sConvertTextTable );
        aAutoFmtBtn.SetClickHdl(LINK(this, SwConvertTableDlg, AutoFmtHdl));
        aAutoFmtBtn.Show();
        aKeepColumn.Show();
        aKeepColumn.Enable( aTabBtn.IsChecked() );
        aKeepColumn.Check( !aTabBtn.IsChecked() );
    }
    else
    {
        aKeepColumn.Check( TRUE );
        //Einfuege-Optionen verstecken
        aHeaderCB          .Show(FALSE);
        aRepeatHeaderCB    .Show(FALSE);
        aDontSplitCB       .Show(FALSE);
        aBorderCB          .Show(FALSE);
        aOptionsGB         .Show(FALSE);

        //Groesse anpassen
        Size aSize(GetSizePixel());
        aSize.Height() = aDelimFrm.GetSizePixel().Height() + 2 * aDelimFrm.GetPosPixel().Y();
        SetOutputSizePixel(aSize);
    }
    aKeepColumn.SaveValue();

    Link aLk( LINK(this, SwConvertTableDlg, BtnHdl) );
    aTabBtn.SetClickHdl( aLk );
    aSemiBtn.SetClickHdl( aLk );
    aParaBtn.SetClickHdl( aLk );
    aOtherBtn.SetClickHdl(aLk );
    aOtherEd.Enable( aOtherBtn.IsChecked() );

    const SwModuleOptions* pModOpt = SW_MOD()->GetModuleConfig();

    BOOL bHTMLMode = 0 != (::GetHtmlMode(rView.GetDocShell())&HTMLMODE_ON);
    USHORT nInsTblFlags = pModOpt->GetInsTblFlags(bHTMLMode);

    aHeaderCB.Check(nInsTblFlags & HEADLINE);
    aRepeatHeaderCB.Check(nInsTblFlags & REPEAT);
    aDontSplitCB.Check(!(nInsTblFlags & SPLIT_LAYOUT));
    aBorderCB.Check(nInsTblFlags & DEFAULT_BORDER);

    aHeaderCB.SetClickHdl(LINK(this, SwConvertTableDlg, CheckBoxHdl));
    CheckBoxHdl();
}

SwConvertTableDlg:: ~SwConvertTableDlg()
{
    delete pTAutoFmt;
}

IMPL_LINK( SwConvertTableDlg, AutoFmtHdl, PushButton*, pButton )
{
    SwAutoFormatDlg aDlg( pButton, pShell, FALSE, pTAutoFmt );
    if( RET_OK == aDlg.Execute())
        aDlg.FillAutoFmtOfIndex( pTAutoFmt );
    return 0;
}

IMPL_LINK( SwConvertTableDlg, BtnHdl, Button*, pButton )
{
    if( pButton == &aTabBtn )
        aKeepColumn.SetState( aKeepColumn.GetSavedValue() );
    else
    {
        if( aKeepColumn.IsEnabled() )
            aKeepColumn.SaveValue();
        aKeepColumn.Check( TRUE );
    }
    aKeepColumn.Enable( aTabBtn.IsChecked() );
    aOtherEd.Enable( aOtherBtn.IsChecked() );
    return 0;
}

/*********************************************************************/
/*                                                                   */
/*********************************************************************/

IMPL_LINK(SwConvertTableDlg, CheckBoxHdl, CheckBox*, EMPTYARG)
{
    aRepeatHeaderCB.Enable(aHeaderCB.IsChecked());

    return 0;
}

/*------------------------------------------------------------------------

    $Log: not supported by cvs2svn $
    Revision 1.39  2000/09/18 16:06:07  willem.vandorp
    OpenOffice header added.

    Revision 1.38  2000/02/11 14:58:29  hr
    #70473# changes for unicode ( patched by automated patchtool )

    Revision 1.37  1999/05/03 09:03:16  OS
    #65612# Einfuegeoptionen nur in einer Richtung anzeigen


      Rev 1.36   03 May 1999 11:03:16   OS
   #65612# Einfuegeoptionen nur in einer Richtung anzeigen

      Rev 1.35   17 Feb 1999 08:39:46   OS
   #58158# Einfuegen TabPage auch in HTML-Docs

      Rev 1.34   02 Dec 1998 15:29:58   OM
   #59770# Tabellenueberschrift nur auf erster Seite

      Rev 1.33   30 Nov 1998 17:33:40   OM
   #59770# Tabellenoptionen: Ueberschrift nur auf erster Seite

      Rev 1.32   11 Nov 1998 16:51:58   OM
   #58158# Einfuegeoptionen fuer Tabellen

      Rev 1.31   09 Jul 1998 22:44:00   JP
   Bug #52543#: SaveState am KeepButton im CTOR immer rufen

      Rev 1.30   22 Jun 1998 12:54:30   JP
   Bug #51412#: CTOR TabelleToText - KeepButton checken

      Rev 1.29   15 Jun 1998 20:33:16   JP
   TextToTable: KommaBtn gegen AndereBtn und -Edit ausgetauscht, OptionCheckBox fuer TabBtnt

      Rev 1.28   29 May 1998 18:55:32   JP
   SS vom TableAutoFormatDialog hat sich geaendert

      Rev 1.27   13 May 1998 16:00:50   OS
   HelpButton

      Rev 1.26   12 May 1998 23:42:42   JP
   neu: InserTable/TextToTable mit optionalen AutoFormat

      Rev 1.25   24 Nov 1997 15:52:22   MA
   includes

      Rev 1.24   03 Nov 1997 13:56:52   MA
   precomp entfernt

      Rev 1.23   11 Nov 1996 11:20:02   MA
   ResMgr

      Rev 1.22   28 Aug 1996 14:33:14   OS
   includes

      Rev 1.21   26 Jun 1996 15:25:34   OS
   Aufruf von Dispatcher.Execute an 324 angepasst

      Rev 1.20   22 Mar 1996 14:16:30   HJS
   umstellung 311

      Rev 1.19   24 Nov 1995 16:58:56   OM
   PCH->PRECOMPILED

      Rev 1.18   10 Nov 1995 15:26:44   OS
   Execute mit 0L abschliessen

      Rev 1.17   07 Nov 1995 07:27:04   OS
   include tablemgr muss bleiben

      Rev 1.16   06 Nov 1995 17:14:14   OS
   ConvertDlg ohne TableMgr und RecordFlag

      Rev 1.15   30 Aug 1995 14:02:32   MA
   fix: sexport'iert

      Rev 1.14   24 Aug 1995 14:33:32   MA
   swstddlg -> svxstandarddialog

      Rev 1.13   25 Oct 1994 18:52:30   ER
   add: PCH

      Rev 1.12   26 Feb 1994 02:37:52   ER
   virt. dtor mit export

------------------------------------------------------------------------*/


