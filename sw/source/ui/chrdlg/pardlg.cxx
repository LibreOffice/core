/*************************************************************************
 *
 *  $RCSfile: pardlg.cxx,v $
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

#include "hintids.hxx"

#ifndef _SVX_TABSTPGE_HXX //autogen
#include <svx/tabstpge.hxx>
#endif
#ifndef _SVX_PARAGRPH_HXX //autogen
#include <svx/paragrph.hxx>
#endif
#ifndef _SVX_BACKGRND_HXX
#include <svx/backgrnd.hxx>
#endif
#ifndef _SVX_BORDER_HXX
#include <svx/border.hxx>
#endif
#ifndef _SVX_HTMLMODE_HXX
#include <svx/htmlmode.hxx>
#endif
#ifndef _SFXSTYLE_HXX //autogen
#include <svtools/style.hxx>
#endif
#ifndef _OFA_HTMLCFG_HXX //autogen
#include <offmgr/htmlcfg.hxx>
#endif
#ifndef _OFF_APP_HXX //autogen
#include <offmgr/app.hxx>
#endif

#ifndef _SVSTDARR_STRINGSISORTDTOR
#define _SVSTDARR_STRINGSISORTDTOR
#include <svtools/svstdarr.hxx>
#endif

#include "docsh.hxx"
#include "wrtsh.hxx"
#include "frmatr.hxx"
#include "view.hxx"
#include "globals.hrc"
#include "pardlg.hxx"
#include "pagedesc.hxx"
#include "paratr.hxx"
#include "drpcps.hxx"
#include "uitool.hxx"
#include "viewopt.hxx"

#ifndef _NUMPARA_HXX
#include <numpara.hxx>
#endif
#include "chrdlg.hrc"
#include "poolfmt.hrc"


// STATIC DATA -----------------------------------------------------------


SwParaDlg::SwParaDlg(Window *pParent,
                    SwView& rVw,
                    const SfxItemSet& rCoreSet,
                    BYTE nDialogMode,
                    const String *pTitle,
                    BOOL bDraw,
                    UINT16 nDefPage):

    SfxTabDialog(pParent, bDraw ? SW_RES(DLG_DRAWPARA) : SW_RES(DLG_PARA),
                    &rCoreSet,  0 != pTitle),

    rView(rVw),
    nDlgMode(nDialogMode),
    bDrawParaDlg(bDraw)

{
    FreeResource();

    nHtmlMode = ::GetHtmlMode(rVw.GetDocShell());
    BOOL bHtmlMode = nHtmlMode & HTMLMODE_ON;
    if(pTitle)
    {
            // Update des Titels
        String aTmp( GetText() );
        aTmp += SW_RESSTR(STR_TEXTCOLL_HEADER);
        aTmp += *pTitle;
        aTmp += ')';
        SetText(aTmp);
    }

    AddTabPage(TP_PARA_STD,  SvxStdParagraphTabPage::Create,SvxStdParagraphTabPage::GetRanges);
    AddTabPage(TP_PARA_ALIGN,  SvxParaAlignTabPage::Create,SvxParaAlignTabPage::GetRanges);

    OfaHtmlOptions* pHtmlOpt = OFF_APP()->GetHtmlOptions();
    if (!bDrawParaDlg && (!bHtmlMode || pHtmlOpt->IsPrintLayoutExtension()))
        AddTabPage(TP_PARA_EXT,  SvxExtParagraphTabPage::Create,SvxExtParagraphTabPage::GetRanges);
    else
        RemoveTabPage(TP_PARA_EXT);

    USHORT nWhich(rCoreSet.GetPool()->GetWhich(SID_ATTR_LRSPACE));
    BOOL bLRValid = SFX_ITEM_AVAILABLE <= rCoreSet.GetItemState(nWhich);
    if(bHtmlMode || !bLRValid)
        RemoveTabPage(TP_TABULATOR);
    else
        AddTabPage(TP_TABULATOR, SvxTabulatorTabPage::Create,   SvxTabulatorTabPage::GetRanges);

    if (!bDrawParaDlg)
    {
        if(!(nDlgMode & DLG_ENVELOP))
            AddTabPage(TP_NUMPARA,   SwParagraphNumTabPage::Create,SwParagraphNumTabPage::GetRanges);
        else
            RemoveTabPage(TP_NUMPARA);
        if(!bHtmlMode || (nHtmlMode & HTMLMODE_FULL_STYLES))
        {
            AddTabPage(TP_DROPCAPS,  SwDropCapsPage::Create,        SwDropCapsPage::GetRanges);
            AddTabPage(TP_BACKGROUND,SvxBackgroundTabPage::Create,  SvxBackgroundTabPage::GetRanges);
        }
        else
        {
            RemoveTabPage(TP_DROPCAPS);
            RemoveTabPage(TP_BACKGROUND);
        }
        if(!bHtmlMode || (nHtmlMode & HTMLMODE_PARA_BORDER))
            AddTabPage(TP_BORDER,    SvxBorderTabPage::Create,      SvxBorderTabPage::GetRanges);
        else
            RemoveTabPage(TP_BORDER);
    }

    if (nDefPage)
        SetCurPageId(nDefPage);
}


__EXPORT SwParaDlg::~SwParaDlg()
{
}


void __EXPORT SwParaDlg::PageCreated(USHORT nId, SfxTabPage& rPage)
{
    SwWrtShell& rSh = rView.GetWrtShell();

    // Bei Tabellenumrandung kann im Writer kein Schatten eingestellt werden
    if (nId == TP_BORDER)
    {
        ((SvxBorderTabPage&) rPage).SetSWMode(SW_BORDER_MODE_PARA);
    }
    else if( nId == TP_PARA_STD )
    {
        ((SvxStdParagraphTabPage&)rPage).SetPageWidth(
                                            rSh.GetAnyCurRect(RECT_PAGE_PRT).Width());
        if (!bDrawParaDlg)
        {
            ((SvxStdParagraphTabPage&)rPage).EnableRegisterMode();
            ((SvxStdParagraphTabPage&)rPage).EnableAutoFirstLine();
            ((SvxStdParagraphTabPage&)rPage).EnableAbsLineDist(MM50/2);
            ((SvxStdParagraphTabPage&)rPage).EnableNegativeMode();
        }
    }
    else if( TP_PARA_ALIGN == nId)
    {
        if (!bDrawParaDlg)
            ((SvxParaAlignTabPage&)rPage).EnableJustifyExt();
    }
    else if( TP_PARA_EXT == nId )
    {
        // Seitenumbruch nur, wenn der Cursor im Body-Bereich und nicht in
        // einer Tabelle steht
        const USHORT eType = rSh.GetFrmType(0,TRUE);
        if( !(FRMTYPE_BODY & eType) ||
            rSh.GetSelectionType() & SwWrtShell::SEL_TBL )
            ((SvxExtParagraphTabPage&)rPage).DisablePageBreak();
    }
    else if( TP_DROPCAPS == nId )
    {
        ((SwDropCapsPage&)rPage).SetFormat(FALSE);
    }
    else if( TP_BACKGROUND == nId )
    {
      if(!( nHtmlMode & HTMLMODE_ON ) ||
        nHtmlMode & HTMLMODE_SOME_STYLES)
            ((SvxBackgroundTabPage&)rPage).ShowSelector();
    }
    else if( TP_NUMPARA == nId)
    {
        ((SwParagraphNumTabPage&)rPage).EnableNewStart();
        ListBox & rBox = ((SwParagraphNumTabPage&)rPage).GetStyleBox();
        SfxStyleSheetBasePool* pPool = rView.GetDocShell()->GetStyleSheetPool();
        pPool->SetSearchMask(SFX_STYLE_FAMILY_PSEUDO, SFXSTYLEBIT_ALL);
        const SfxStyleSheetBase* pBase = pPool->First();
        SvStringsISortDtor aNames;
        while(pBase)
        {
            aNames.Insert(new String(pBase->GetName()));
            pBase = pPool->Next();
        }
        for(USHORT i = 0; i < aNames.Count(); i++)
            rBox.InsertEntry(*aNames.GetObject(i));
    }

}

/*------------------------------------------------------------------------

    $Log: not supported by cvs2svn $
    Revision 1.195  2000/09/18 16:05:14  willem.vandorp
    OpenOffice header added.

    Revision 1.194  2000/08/24 08:44:19  ama
    New: Negative margins

    Revision 1.193  1999/05/19 07:59:54  OS
    #66015# Absatzausrichtung: Erweiterungen nicht fuer Draw-Text


      Rev 1.192   19 May 1999 09:59:54   OS
   #66015# Absatzausrichtung: Erweiterungen nicht fuer Draw-Text

      Rev 1.191   27 Nov 1998 14:51:26   AMA
   Fix #59951#59825#: Unterscheiden zwischen Rahmen-,Seiten- und Bereichsspalten

      Rev 1.190   28 Apr 1998 09:08:54   OS
   Background: ShowSelector mit HTML-Beruecksichtigung #49862#

      Rev 1.189   21 Apr 1998 15:33:44   OS
   zwei ! an der falschen Stelle

      Rev 1.188   16 Apr 1998 16:00:10   OS
   Ranges der TabPages mitgeben #47097# Printing extensions fuer HTML

      Rev 1.187   05 Feb 1998 14:45:16   OS
   Remove-/AddTabPage alternativ, TableMode entsorgt, keine Numerierung fuer Umschlaege

      Rev 1.186   02 Feb 1998 09:13:32   OS
   neue Num-Tabpage

      Rev 1.185   09 Dec 1997 16:46:28   OS
   Numerierungsvorlagen sortieren

      Rev 1.184   04 Dec 1997 16:59:12   OS
   Numerierung im Absatzdialog

      Rev 1.183   28 Nov 1997 15:03:02   MA
   includes

      Rev 1.182   21 Nov 1997 12:24:36   MA
   includes

      Rev 1.181   02 Sep 1997 10:46:12   OS
   includes

      Rev 1.180   29 Aug 1997 17:30:02   HJS
   includes

      Rev 1.179   03 Jul 1997 12:55:34   OS
   SwSwMode jetzt mit BYTE #41255#

      Rev 1.178   11 Jun 1997 16:19:54   OS
   absolute Zeilenhoehe aktiviert

      Rev 1.177   10 Feb 1997 13:22:58   OS
   Absatz HG nur fuer FULL_STYLES

      Rev 1.176   05 Feb 1997 16:20:56   OS
   kein Hintergrund ohne Styles

      Rev 1.175   05 Feb 1997 13:32:12   OS
   im SW sind DropCaps auch fuer HTML erlaubt

      Rev 1.174   28 Jan 1997 16:32:54   OS
   alte defines entfernt

      Rev 1.173   27 Jan 1997 16:15:18   OS
   HtmlMode wird ueber GetHtmlMode ermittelt

      Rev 1.172   17 Jan 1997 12:56:34   OS
   neue Absatz-Tabpage ab 353: Ausrichtung

      Rev 1.171   16 Jan 1997 18:14:12   OM
   Tabulator-TP fuer DrawTextMode wiederbelebt

      Rev 1.170   08 Jan 1997 10:54:24   OS
   HtmlMode ueber GetHtmlMode ermitteln

      Rev 1.169   28 Nov 1996 14:57:32   OS
   keine Tabulatorpage mit uneindeutigen Absatzeinzuegen

      Rev 1.168   11 Nov 1996 09:16:40   MA
   ResMgr

      Rev 1.167   14 Oct 1996 17:43:56   HJS
   include

      Rev 1.166   14 Oct 1996 12:51:16   OS
   einige TabPages im Html-Mode verstecken

      Rev 1.165   12 Sep 1996 17:03:06   OS
   GetAnyCurRect() ersetzt GetCur*Rect

      Rev 1.164   28 Aug 1996 08:59:08   OS
   includes

      Rev 1.163   09 Aug 1996 11:38:06   OM
   #30084#GPF behoben

      Rev 1.162   23 Jul 1996 16:08:18   OS
   EnableRegisterMode am Absatzdialog setzen

      Rev 1.161   24 May 1996 14:31:50   OS
   Hintergrundgrafik enabled

      Rev 1.160   05 Feb 1996 17:27:14   OM
   #24879# Parent-Win in Ctor uebergeben

      Rev 1.159   28 Nov 1995 23:37:14   JP
   uisys.hxx gibt es nicht mehr

      Rev 1.158   24 Nov 1995 16:57:34   OM
   PCH->PRECOMPILED

      Rev 1.157   23 Nov 1995 17:57:00   OM
   Default-TabPage setzen

      Rev 1.156   09 Nov 1995 17:56:38   OS
   Format-Bit an der Initialen-Page FALSE setzen

      Rev 1.155   03 Nov 1995 20:17:10   MA
   GetUpper..Rect returne keine Objecte mehr

      Rev 1.154   31 Oct 1995 18:36:08   OM
   GetActive... entfernt

      Rev 1.153   07 Aug 1995 13:42:30   JP
   Breaks nur im Body-Bereich zulassen

      Rev 1.152   06 Aug 1995 18:26:46   OM
   DrawTextShell-Popup

      Rev 1.151   14 Jul 1995 17:47:04   OS
   Seitenbreite an der StdParagraphTabPage setzen

      Rev 1.150   07 Jul 1995 10:46:52   OS
   Seitenvorlagen werden jetzt vom Svx besorgt

      Rev 1.149   25 Apr 1995 17:21:52   OS
   PageModels fuellen BugFix 11100/11105

      Rev 1.148   30 Mar 1995 18:38:18   PK
   caption fuer absatzvorlagen

      Rev 1.147   29 Mar 1995 09:12:22   OS
   TabDialog - weniger Parameter

      Rev 1.146   01 Feb 1995 17:45:44   PK
   hideshadowcontrol()

      Rev 1.145   27 Jan 1995 06:59:44   MS
   HideShadowControls auskommentiert

      Rev 1.144   26 Jan 1995 17:36:26   PK
   hideshadowcontrols() scharfgeschaltet

      Rev 1.143   26 Jan 1995 17:00:34   PK
   pagecreated() ueberladen

      Rev 1.142   24 Jan 1995 12:51:14   SWG
   Map entfernt

      Rev 1.141   09 Jan 1995 16:31:52   ER
    del: paradlg hrc

      Rev 1.140   07 Dec 1994 09:44:04   MS
   Fix 122c

      Rev 1.139   06 Dec 1994 16:30:04   MS
   Einbau allgemeiner AbsatzDlg

      Rev 1.138   05 Dec 1994 16:54:16   MS
   Anpassung Tabdlg

      Rev 1.137   01 Dec 1994 16:49:58   MS
   GetActiveView

      Rev 1.136   01 Dec 1994 16:14:02   MS
   Id Tabpages in globals.hrc

      Rev 1.135   18 Nov 1994 15:31:58   MA
   min -> Min, max -> Max

      Rev 1.134   15 Nov 1994 09:02:36   SWG
   SetLineSpace static deklariert

      Rev 1.133   02 Nov 1994 11:25:38   AMA
   Reimport Absatzattr. als Svx (paraitem.hxx)
   SvxSizeItem->SvxFontHeight (textitem.hxx)

      Rev 1.132   28 Oct 1994 19:13:18   MA
   Reimport frmatr.

      Rev 1.131   25 Oct 1994 15:50:14   ER
   add: PCH

      Rev 1.130   14 Oct 1994 15:09:50   MS
   ctor umgestellt

      Rev 1.129   12 Oct 1994 08:12:14   MS
   Anpassung Sfx

      Rev 1.128   07 Oct 1994 10:50:02   PK
   sexport fuer dtor

      Rev 1.127   06 Oct 1994 12:09:12   VB
   Bugfix Aenderung Parent TabPages im Sfx

      Rev 1.126   06 Oct 1994 09:59:32   VB
   Bugfix Sfx

      Rev 1.125   05 Oct 1994 09:35:22   SWG
   Aenderungen Sfx

      Rev 1.124   04 Oct 1994 18:21:02   SWG
   Syntaxaederungen Sfx

      Rev 1.123   04 Oct 1994 18:07:50   SWG
   FillSet -> FillItemSet

      Rev 1.122   04 Oct 1994 16:49:40   VB
   Grenzwerte, Handler

      Rev 1.121   30 Sep 1994 16:50:32   VB
   Zwischensicherung

      Rev 1.120   29 Sep 1994 17:42:56   VB
   Syntaxprobleme

      Rev 1.119   29 Sep 1994 16:02:06   VB
   Umstellung SfxTabDlg

      Rev 1.118   28 Sep 1994 12:57:48   VB
   Absatzdialog als Huelle

------------------------------------------------------------------------*/


