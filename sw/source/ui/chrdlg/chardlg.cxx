/*************************************************************************
 *
 *  $RCSfile: chardlg.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: jp $ $Date: 2000-10-20 13:46:39 $
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

#ifndef _HINTIDS_HXX
#include <hintids.hxx>
#endif

#ifndef _MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif
#ifndef SVTOOLS_URIHELPER_HXX
#include <svtools/urihelper.hxx>
#endif
#ifndef _SFXSTRITEM_HXX
#include <svtools/stritem.hxx>
#endif
#ifndef _IODLG_HXX
#include <sfx2/iodlg.hxx>
#endif
#ifndef _SVX_CHARDLG_HXX //autogen
#include <svx/chardlg.hxx>
#endif
#ifndef _SVX_BACKGRND_HXX //autogen
#include <svx/backgrnd.hxx>
#endif
#ifndef _SVX_HTMLMODE_HXX //autogen
#include <svx/htmlmode.hxx>
#endif

#ifndef _CMDID_H
#include <cmdid.h>
#endif
#ifndef _HELPID_H
#include <helpid.h>
#endif
#ifndef _SWTYPES_HXX
#include <swtypes.hxx>
#endif
#ifndef _UIPARAM_HXX
#include <uiparam.hxx>
#endif
#ifndef _ERRHDL_HXX
#include <errhdl.hxx>
#endif
#ifndef _VIEW_HXX
#include <view.hxx>
#endif
#ifndef _WRTSH_HXX
#include <wrtsh.hxx>
#endif
#ifndef _DOCSH_HXX
#include <docsh.hxx>
#endif
#ifndef _UITOOL_HXX
#include <uitool.hxx>
#endif
#ifndef _FMTINFMT_HXX //autogen
#include <fmtinfmt.hxx>
#endif
#ifndef _MACASSGN_HXX
#include <macassgn.hxx>
#endif
#ifndef _CHRDLG_HXX
#include <chrdlg.hxx>       // der Dialog
#endif
#ifndef _SWMODULE_HXX
#include <swmodule.hxx>
#endif
#ifndef _POOLFMT_HXX
#include <poolfmt.hxx>
#endif

#ifndef _GLOBALS_HRC
#include <globals.hrc>
#endif
#ifndef _CHRDLG_HRC
#include <chrdlg.hrc>
#endif
#ifndef _CHARDLG_HRC
#include <chardlg.hrc>
#endif


/*--------------------------------------------------------------------
    Beschreibung:   Der Traeger des Dialoges
 --------------------------------------------------------------------*/


SwCharDlg::SwCharDlg(Window* pParent, SwView& rVw, const SfxItemSet& rCoreSet,
                     const String* pStr, BOOL bIsDrwTxtDlg) :
    SfxTabDialog(pParent, SW_RES(DLG_CHAR), &rCoreSet, pStr != 0),
    rView(rVw),
    bIsDrwTxtMode(bIsDrwTxtDlg)
{
    FreeResource();

    // bspFonr fuer beide Bsp-TabPages

    if(pStr)
    {
        String aTmp( GetText() );
        aTmp += SW_RESSTR(STR_TEXTCOLL_HEADER);
        aTmp += *pStr;
        aTmp += ')';
        SetText(aTmp);
    }

    //OS: Unter OS/2 darf die erste TabPage nie per RemoveTabPage entfernt werden
    AddTabPage(TP_CHAR_STD, SvxCharStdPage::Create, 0);
    AddTabPage(TP_CHAR_EXT, SvxCharExtPage::Create, 0);
    AddTabPage(TP_CHAR_URL, SwCharURLPage::Create, 0);
    AddTabPage(TP_BACKGROUND,SvxBackgroundTabPage::Create,  0);
    if(bIsDrwTxtMode)
    {
        RemoveTabPage(TP_CHAR_URL);
        RemoveTabPage(TP_BACKGROUND);
    }
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/


SwCharDlg::~SwCharDlg()
{
}

/*--------------------------------------------------------------------
    Beschreibung:   FontList setzen
 --------------------------------------------------------------------*/



void SwCharDlg::PageCreated( USHORT nId, SfxTabPage &rPage )
{
    switch( nId )
    {
        case TP_CHAR_STD:
            ((SvxCharStdPage&)rPage).SetFontList( *( (SvxFontListItem*)
               ( rView.GetDocShell()->GetItem( SID_ATTR_CHAR_FONTLIST ) ) ) );
            break;
        case TP_CHAR_EXT:
            if(bIsDrwTxtMode)
                ((SvxCharExtPage&)rPage).DisableControls(DISABLE_CASEMAP);
            else
                ((SvxCharExtPage&)rPage).EnableFlash();
            break;
    }
}

/*-----------------14.08.96 11.28-------------------

--------------------------------------------------*/

SwCharURLPage::SwCharURLPage(   Window* pParent,
                                const SfxItemSet& rCoreSet ) :
    SfxTabPage( pParent, SW_RES( TP_CHAR_URL ), rCoreSet ),
    aURLGB (        this, SW_RES(GB_URL)),
    aURLFT(         this, SW_RES(FT_URL        )),
    aURLED(         this, SW_RES(ED_URL        )),
    aTextFT(        this, SW_RES(FT_TEXT          )),
    aTextED(        this, SW_RES(ED_TEXT          )),
    aNameFT(        this, SW_RES(FT_NAME    )),
    aNameED(        this, SW_RES(ED_NAME)),
    aTargetFrmFT(   this, SW_RES(FT_TARGET     )),
    aTargetFrmLB(   this, SW_RES(LB_TARGET    )),
    aStyleGB(       this, SW_RES(GB_STYLE      )),
    aVisitedFT(     this, SW_RES(FT_VISITED    )),
    aVisitedLB(     this, SW_RES(LB_VISITED    )),
    aNotVisitedFT(  this, SW_RES(FT_NOT_VISITED)),
    aNotVisitedLB(  this, SW_RES(LB_NOT_VISITED)),
    aEventPB(       this, SW_RES(PB_EVENT      )),
    aURLPB(         this, SW_RES(PB_URL        )),
    pINetItem(0),
    bModified(FALSE)

{
    FreeResource();

    const SfxPoolItem* pItem;
    SfxObjectShell* pShell;
    if(SFX_ITEM_SET == rCoreSet.GetItemState(SID_HTML_MODE, FALSE, &pItem) ||
        ( 0 != ( pShell = SfxObjectShell::Current()) &&
                    0 != (pItem = pShell->GetItem(SID_HTML_MODE))))
    {
        USHORT nHtmlMode = ((const SfxUInt16Item*)pItem)->GetValue();
        if(HTMLMODE_ON & nHtmlMode)
        {
            aStyleGB.Hide();
            aVisitedFT.Hide();
            aVisitedLB.Hide();
            aNotVisitedFT.Hide();
            aNotVisitedLB.Hide();
        }
    }

    aURLPB.SetClickHdl  (LINK( this, SwCharURLPage, InsertFileHdl));
    aEventPB.SetClickHdl(LINK( this, SwCharURLPage, EventHdl ));

    SwView *pView = ::GetActiveView();
    ::FillCharStyleListBox(aVisitedLB, pView->GetDocShell());
    ::FillCharStyleListBox(aNotVisitedLB, pView->GetDocShell());

    TargetList* pList = new TargetList;
    const SfxFrame* pFrame = pView->GetViewFrame()->GetTopFrame();
    pFrame->GetTargetList(*pList);
    USHORT nCount = (USHORT)pList->Count();
    if( nCount )
    {
        for ( USHORT i = 0; i < nCount; i++ )
        {
            aTargetFrmLB.InsertEntry(*pList->GetObject(i));
        }
        for ( i = nCount; i; i-- )
        {
            delete pList->GetObject( i - 1 );
        }
    }
    delete pList;


}

/*-----------------15.08.96 09.04-------------------

--------------------------------------------------*/

SwCharURLPage::~SwCharURLPage()
{
    delete pINetItem;
}

/*-----------------14.08.96 11.31-------------------

--------------------------------------------------*/

void SwCharURLPage::Reset(const SfxItemSet& rSet)
{
    const SfxPoolItem* pItem;
    if(SFX_ITEM_SET == rSet.GetItemState(RES_TXTATR_INETFMT, FALSE, &pItem))
    {
        const SwFmtINetFmt* pINetFmt = (const SwFmtINetFmt*)pItem;
        aURLED.SetText( INetURLObject::decode( pINetFmt->GetValue(),
                                        INET_HEX_ESCAPE,
                                           INetURLObject::DECODE_UNAMBIGUOUS,
                                        RTL_TEXTENCODING_UTF8 ));
        aURLED.SaveValue();
        aNameED.SetText(pINetFmt->GetName());
        String sEntry = pINetFmt->GetVisitedFmt();
        if( !sEntry.Len() )
            GetDocPoolNm( RES_POOLCHR_INET_VISIT, sEntry );
        aVisitedLB.SelectEntry(sEntry);
        sEntry = pINetFmt->GetINetFmt();
        if(!sEntry.Len())
            GetDocPoolNm( RES_POOLCHR_INET_NORMAL, sEntry );
        aNotVisitedLB.SelectEntry(sEntry);

        aTargetFrmLB.SetText(pINetFmt->GetTargetFrame());
        aVisitedLB.   SaveValue();
        aNotVisitedLB.SaveValue();
        aTargetFrmLB. SaveValue();
        pINetItem = new SvxMacroItem(FN_INET_FIELD_MACRO);

        if( pINetFmt->GetMacroTbl() )
            pINetItem->SetMacroTable( *pINetFmt->GetMacroTbl() );
    }
    if(SFX_ITEM_SET == rSet.GetItemState(FN_PARAM_SELECTION, FALSE, &pItem))
    {
        aTextED.SetText(((const SfxStringItem*)pItem)->GetValue());
        aTextFT.Enable( FALSE );
        aTextED.Enable( FALSE );
    }
}

/*-----------------14.08.96 11.32-------------------

--------------------------------------------------*/

BOOL SwCharURLPage::FillItemSet(SfxItemSet& rSet)
{
    String sURL = aURLED.GetText();
    if(sURL.Len())
        sURL = URIHelper::SmartRelToAbs(sURL);
    SwFmtINetFmt aINetFmt(sURL, aTargetFrmLB.GetText());
    aINetFmt.SetName(aNameED.GetText());
    SwWrtShell &rSh = ::GetActiveView()->GetWrtShell();
    bModified |= aURLED.GetText() != aURLED.GetSavedValue();
    bModified |= aNameED.IsModified();
    bModified |= aTargetFrmLB.GetSavedValue() != aTargetFrmLB.GetText();

    //zuerst die gueltigen Einstellungen setzen
    String sEntry = aVisitedLB.GetSelectEntry();
    USHORT nId = rSh.GetPoolId( sEntry, GET_POOLID_CHRFMT);
    aINetFmt.SetVisitedFmtId(nId);
    aINetFmt.SetVisitedFmt(nId == RES_POOLCHR_INET_VISIT ? aEmptyStr : sEntry);

    sEntry = aNotVisitedLB.GetSelectEntry();
    nId = rSh.GetPoolId( sEntry, GET_POOLID_CHRFMT);
    aINetFmt.SetINetFmtId( nId );
    aINetFmt.SetINetFmt(nId == RES_POOLCHR_INET_NORMAL ? aEmptyStr : sEntry);

    if( pINetItem && pINetItem->GetMacroTable().Count() )
        aINetFmt.SetMacroTbl( &pINetItem->GetMacroTable() );

    if(aVisitedLB.GetSavedValue() != aVisitedLB.GetSelectEntryPos())
        bModified = TRUE;

    if(aNotVisitedLB.GetSavedValue() != aNotVisitedLB.GetSelectEntryPos())
        bModified = TRUE;

    if(aTextED.IsModified())
    {
        bModified = TRUE;
        rSet.Put(SfxStringItem(FN_PARAM_SELECTION, aTextED.GetText()));
    }
    if(bModified)
        rSet.Put(aINetFmt);
    return bModified;
}


/*-----------------14.08.96 11.30-------------------

--------------------------------------------------*/

SfxTabPage* SwCharURLPage::Create(  Window* pParent,
                        const SfxItemSet& rAttrSet )
{
    return ( new SwCharURLPage( pParent, rAttrSet ) );
}

/*-----------------14.08.96 15.00-------------------

--------------------------------------------------*/

IMPL_LINK( SwCharURLPage, InsertFileHdl, PushButton *, pBtn )
{
    SfxFileDialog* pFileDlg = new SfxFileDialog(pBtn, WB_OPEN);
    pFileDlg->DisableSaveLastDirectory();
    pFileDlg->SetHelpId(HID_FILEDLG_CHARDLG);

    if(RET_OK == pFileDlg->Execute())
    {
        aURLED.SetText(URIHelper::SmartRelToAbs(pFileDlg->GetPath()));
    }
    delete pFileDlg;

    return 0;
}
/*-----------------14.08.96 15.00-------------------

--------------------------------------------------*/

IMPL_LINK( SwCharURLPage, EventHdl, PushButton *, EMPTYARG )
{
    bModified |= SwMacroAssignDlg::INetFmtDlg( this,
                    ::GetActiveView()->GetWrtShell(), pINetItem );
    return 0;
}

/*------------------------------------------------------------------------

    $Log: not supported by cvs2svn $
    Revision 1.1.1.1  2000/09/18 17:14:32  hr
    initial import

    Revision 1.213  2000/09/18 16:05:13  willem.vandorp
    OpenOffice header added.

    Revision 1.212  2000/08/31 11:37:50  jp
    add missing include

    Revision 1.211  2000/08/17 13:48:23  jp
    UI with decode URL

    Revision 1.210  2000/07/26 16:32:19  jp
    use the new function GetDocPoolNm to get the collectionames

    Revision 1.209  2000/07/03 10:33:17  os
    #72742# resource warnings corrected

    Revision 1.208  2000/06/26 13:06:44  os
    INetURLObject::SmartRelToAbs removed

    Revision 1.207  2000/04/19 12:56:33  os
    include sfx2/filedlg.hxx removed

    Revision 1.206  2000/02/11 14:43:26  hr
    #70473# changes for unicode ( patched by automated patchtool )

    Revision 1.205  2000/01/24 12:53:41  os
    #72153# call SfxFileDialog::DisableSaveLastDirectory

    Revision 1.204  1999/02/19 07:48:02  MA
    #61949# CurrShell gibt es nicht mehr


      Rev 1.203   19 Feb 1999 08:48:02   MA
   #61949# CurrShell gibt es nicht mehr

      Rev 1.202   12 Feb 1999 07:35:36   OS
   #61800# Keine URL und kein Hintergrund fuer DrawText

      Rev 1.201   02 Sep 1998 14:11:10   OM
   #45378# HelpIDs fuer Dateidialoge

      Rev 1.200   09 Jul 1998 08:57:10   OS
   vor dem putten in jedem Fall Listbox-Inhalte auswerten #52425#

      Rev 1.199   29 Jun 1998 09:39:30   OS
   kein Blinken fuer DrawText#48019#

      Rev 1.198   15 Apr 1998 14:33:06   OS
   ::FillCharStyleListBox

      Rev 1.197   28 Nov 1997 15:01:48   MA
   includes

      Rev 1.196   01 Sep 1997 13:25:14   OS
   DLL-Umstellung

      Rev 1.195   15 Aug 1997 12:12:36   OS
   chartar/frmatr/txtatr aufgeteilt

      Rev 1.194   09 Aug 1997 13:01:46   OS
   paraitem/frmitems/textitem aufgeteilt

      Rev 1.193   08 Aug 1997 17:29:44   OM
   Headerfile-Umstellung

      Rev 1.192   18 Jun 1997 17:30:02   OS
   URL wird jetzt mit SfxFileDialog gesucht #40815#

      Rev 1.191   21 Apr 1997 16:27:00   OS
   TargetFrame jetzt in ComboBox

      Rev 1.190   08 Apr 1997 09:22:40   MA
   chg: falsche Definition entfernt

      Rev 1.189   07 Apr 1997 14:15:08   MH
   chg: header

      Rev 1.188   20 Mar 1997 16:58:20   OS
   Leerstring: URL zuruecksetzen

      Rev 1.187   10 Mar 1997 17:28:48   OS
   URL-Edit per GetSavedValue auf Modifikation testen

      Rev 1.186   14 Feb 1997 18:04:06   OM
   Zahlenformat-Dlg

      Rev 1.185   05 Feb 1997 13:31:32   OS
   keine Zeichenvorlagen fuer Hyperlinks im HTML


------------------------------------------------------------------------*/


