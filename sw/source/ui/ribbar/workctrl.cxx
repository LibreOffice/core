/*************************************************************************
 *
 *  $RCSfile: workctrl.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:14:46 $
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

#include <string> // HACK: prevent conflict between STLPORT and Workshop headers

#ifndef _SFXENUMITEM_HXX //autogen
#include <svtools/eitem.hxx>
#endif
#ifndef _SVX_HTMLMODE_HXX //autogen
#include <svx/htmlmode.hxx>
#endif
#ifndef _SFXDISPATCH_HXX //autogen
#include <sfx2/dispatch.hxx>
#endif
#ifndef _SFX_BINDINGS_HXX //autogen
#include <sfx2/bindings.hxx>
#endif


#ifndef _SWMODULE_HXX
#include <swmodule.hxx>
#endif
#ifndef _VIEW_HXX
#include <view.hxx>
#endif
#ifndef _INITUI_HXX
#include <initui.hxx>
#endif
#ifndef _VIEWOPT_HXX
#include <viewopt.hxx>
#endif
#ifndef _DOCSH_HXX
#include <docsh.hxx>
#endif
#ifndef _GLOSHDL_HXX
#include <gloshdl.hxx>
#endif
#ifndef _GLOSSARY_HXX
#include <glossary.hxx>
#endif
#ifndef _GLOSDOC_HXX
#include <glosdoc.hxx>
#endif
#ifndef _GLOSLST_HXX
#include <gloslst.hxx>
#endif
#ifndef _TBXMGR_HXX
#include <tbxmgr.hxx>
#endif
#ifndef _WORKCTRL_HXX
#include <workctrl.hxx>
#endif
#ifndef _WORKCTRL_HRC
#include <workctrl.hrc>
#endif
#ifndef _CMDID_H
#include <cmdid.h>
#endif
#ifndef _HELPID_H
#include <helpid.h>
#endif
#ifndef _WRTSH_HXX //autogen
#include <wrtsh.hxx>
#endif


//JP 14.01.99: Size Abpruefung
#define NAVI_ENTRIES 20
#if NAVI_ENTRIES != NID_COUNT
#error SwScrollNaviPopup-CTOR static Array falsche Size. Wurden neue IDs zugefuegt ??
#endif


SFX_IMPL_TOOLBOX_CONTROL( SwTbxInsertCtrl, SfxUInt16Item);
SFX_IMPL_TOOLBOX_CONTROL( SwTbxAutoTextCtrl, SfxBoolItem );

/**********************************************************************

**********************************************************************/



SwTbxInsertCtrl::SwTbxInsertCtrl( USHORT nId,
                                    ToolBox& rTbx,
                                    SfxBindings& rBind ) :
        SfxToolBoxControl( nId, rTbx, rBind ),
        nLastSlotId(0)
{
}
/**********************************************************************

**********************************************************************/


SwTbxInsertCtrl::~SwTbxInsertCtrl()
{
}

/**********************************************************************

**********************************************************************/


void SwTbxInsertCtrl::StateChanged( USHORT nSID,
                                      SfxItemState eState,
                                      const SfxPoolItem* pState )
{
    USHORT nId = GetId();
    GetToolBox().EnableItem( nId, (GetItemState(pState) != SFX_ITEM_DISABLED) );

    if( eState == SFX_ITEM_AVAILABLE )
    {
        const SfxUInt16Item* pItem = PTR_CAST( SfxUInt16Item, pState );
        if(pItem)
        {
            nLastSlotId = pItem->GetValue();
            if( nLastSlotId )
                nId = nLastSlotId;
            Image aImage = SFX_IMAGEMANAGER()->GetImage( nId, SW_MOD() );
            GetToolBox().SetItemImage(GetId(), aImage);
        }
    }

}
/**********************************************************************

**********************************************************************/


SfxPopupWindow* SwTbxInsertCtrl::CreatePopupWindow()
{
    USHORT nWinResId, nTbxResId;
    if(GetId() == FN_INSERT_CTRL)
    {
        nWinResId = RID_INSERT_CTRL;
        nTbxResId = TBX_INSERT;
    }
    else /* FN_INSERT_OBJ_CTRL */
    {
        nWinResId = RID_INSERT_OBJ_CTRL;
        nTbxResId = TBX_OBJ_INSERT;
    }

    WindowAlign eAlign = WINDOWALIGN_TOP;
    if(GetToolBox().IsHorizontal())
        eAlign = WINDOWALIGN_LEFT;
    SwPopupWindowTbxMgr *pWin = new SwPopupWindowTbxMgr( GetId(),
                                    eAlign,
                                    SW_RES(nWinResId),
                                    SW_RES(nTbxResId),
                                    GetBindings());

    pWin->StartPopupMode(&GetToolBox(), TRUE);
    pWin->StartSelection();
    pWin->Show();
    return pWin;

}

/**********************************************************************

**********************************************************************/


SfxPopupWindowType  SwTbxInsertCtrl::GetPopupWindowType() const
{
    return nLastSlotId ? SFX_POPUPWINDOW_ONTIMEOUT : SFX_POPUPWINDOW_ONCLICK;
}


/**********************************************************************

**********************************************************************/


void SwTbxInsertCtrl::Select( BOOL bMod1 )
{
    if(nLastSlotId)
        SfxViewFrame::Current()->GetDispatcher()->Execute(nLastSlotId);
}

/**********************************************************************

**********************************************************************/


SwTbxAutoTextCtrl::SwTbxAutoTextCtrl( USHORT nId,
                                    ToolBox& rTbx,
                                    SfxBindings& rBind ) :
    SfxToolBoxControl( nId, rTbx, rBind ),
    pView(0),
    pPopup(0)
{
}
/**********************************************************************

**********************************************************************/


SwTbxAutoTextCtrl::~SwTbxAutoTextCtrl()
{
    DelPopup();
}

/**********************************************************************

**********************************************************************/


SfxPopupWindow* SwTbxAutoTextCtrl::CreatePopupWindow()
{
    pView = ::GetActiveView();
    if(pView && !pView->GetDocShell()->IsReadOnly() &&
       !pView->GetWrtShell().HasReadonlySel() )
    {
        ToolBox& rBox = GetToolBox();
        Point aPt(rBox.OutputToScreenPixel(rBox.GetPointerPosPixel()));
        if(pView)
        {
            Link aLnk = LINK(this, SwTbxAutoTextCtrl, PopupHdl);

            if (GetId() == FN_INSERT_FIELD_CTRL)
            {
                pPopup = new PopupMenu(SW_RES(RID_INSERT_FIELD_CTRL));
                pPopup->SetSelectHdl(aLnk);

                if (::GetHtmlMode(pView->GetDocShell()) & HTMLMODE_ON)
                {
                    pPopup->RemoveItem(pPopup->GetItemPos(FN_INSERT_FLD_PGCOUNT));
                    pPopup->RemoveItem(pPopup->GetItemPos(FN_INSERT_FLD_TOPIC));
                }
            }
            else
            {
                pPopup = new PopupMenu;
                SwGlossaryList* pGlossaryList = ::GetGlossaryList();
                USHORT nGroupCount = pGlossaryList->GetGroupCount();
                for(USHORT i = 1; i <= nGroupCount; i++)
                {
                    // Gruppenname mit Pfad-Extension besorgen
                    String sTitle;
                    String sGroupName = pGlossaryList->GetGroupName(i - 1, FALSE, &sTitle);
                    USHORT nBlockCount = pGlossaryList->GetBlockCount(i -1);
                    if(nBlockCount)
                    {
                        USHORT nIndex = 100 * (i);
                        // aber ohne extension einfuegen
                        pPopup->InsertItem( i, sTitle);//sGroupName.GetToken(0, GLOS_DELIM));
                        PopupMenu* pSub = new PopupMenu;
                        pSub->SetSelectHdl(aLnk);
                        pPopup->SetPopupMenu(i, pSub);
                        for(USHORT j = 0; j < nBlockCount; j++)
                        {
                            String sEntry;
                            String sLongName(pGlossaryList->GetBlockName(i - 1, j, sEntry));
                            sEntry.AppendAscii(" - ");
                            sEntry += sLongName;
                            pSub->InsertItem(++nIndex, sEntry);
                        }
                    }
                }
            }
        }
        ToolBox* pToolBox = &GetToolBox();
        USHORT nId = GetId();
        pToolBox->SetItemDown( nId, TRUE );
        pPopup->Execute( pToolBox, pToolBox->ScreenToOutputPixel( aPt ) );
        pToolBox->SetItemDown( nId, FALSE );
    }
    GetToolBox().EndSelection();
    DelPopup();
    return 0;

}


/**********************************************************************

**********************************************************************/


SfxPopupWindowType SwTbxAutoTextCtrl::GetPopupWindowType() const
{
    return SFX_POPUPWINDOW_ONTIMEOUT;
}

/**********************************************************************

**********************************************************************/


void SwTbxAutoTextCtrl::StateChanged( USHORT nSID,
                                              SfxItemState eState,
                                              const SfxPoolItem* pState )
{
    GetToolBox().EnableItem( GetId(), (GetItemState(pState) != SFX_ITEM_DISABLED) );
    if(FN_INSERT_FIELD_CTRL == nSID && eState >= SFX_ITEM_DEFAULT)
    {
        GetToolBox().CheckItem( GetId(), ((SfxBoolItem*)pState)->GetValue() );
    }
}

/**********************************************************************

**********************************************************************/


IMPL_LINK(SwTbxAutoTextCtrl, PopupHdl, PopupMenu*, pMenu)
{
    int nId = pMenu->GetCurItemId();

    if (GetId() == FN_INSERT_FIELD_CTRL)
    {
        SfxViewFrame::Current()->GetDispatcher()->Execute(nId);
    }
    else
    {
        USHORT nBlock = nId/100;

        SwGlossaryList* pGlossaryList = ::GetGlossaryList();
        String sShortName;
        String sGroup = pGlossaryList->GetGroupName(nBlock - 1, FALSE);
        String sLongName(pGlossaryList->GetBlockName(nBlock - 1, nId - (100 * nBlock) - 1, sShortName));

        SwGlossaryHdl* pGlosHdl = pView->GetGlosHdl();
        SwGlossaryDlg::SetActGroup(sGroup);
        pGlosHdl->SetCurGroup(sGroup, TRUE);
        pGlosHdl->InsertGlossary(sShortName);
    }
    return 0;
}

/**********************************************************************

**********************************************************************/


void SwTbxAutoTextCtrl::DelPopup()
{
    if(pPopup)
    {
        if (GetId() != FN_INSERT_FIELD_CTRL)
        {
            for( USHORT i = 0; i < pPopup->GetItemCount(); i ++ )
            {
                PopupMenu* pSubPopup = pPopup->GetPopupMenu(pPopup->GetItemId(i));
                delete pSubPopup;
            }
        }
        delete pPopup;
        pPopup = 0;
    }
}

/*-----------------19.02.97 10.52-------------------
    Navigations-Popup
--------------------------------------------------*/

SwScrollNaviPopup::SwScrollNaviPopup( USHORT nId,
                                const ResId &rId,
                                SfxBindings & rBnd)
    : SfxPopupWindow(nId, rId, rBnd),
    aVSet(this, WB_NAMEFIELD),
    aIList(ResId(IL_VALUES)),
    rResId(rId),
    nFwdId(FN_START_OF_NEXT_PAGE),
    nBackId(FN_START_OF_PREV_PAGE)
{
    aVSet.SetHelpId(HID_NAVI_VS);
    // die Konvertierungstabelle legt die Reihenfolge fest
    static USHORT __READONLY_DATA aInsert[ NAVI_ENTRIES ] =
    {
        // -- erste Zeile
        NID_TBL,
        NID_FRM,
        NID_GRF,
        NID_OLE,
        NID_PGE,
        NID_OUTL,
        NID_MARK,
        NID_DRW,
        NID_CTRL,
        // -- zweite Zeile
        NID_REG,
        NID_BKM,
        NID_SEL,
        NID_FTN,
        NID_POSTIT,
        NID_SRCH_REP,
        NID_INDEX_ENTRY,
        NID_TABLE_FORMULA,
        NID_TABLE_FORMULA_ERROR
    };

    aVSet.SetLineCount( 2 );
    aVSet.SetColCount(NID_COUNT / 2 - 1);
    for(USHORT i = 0; i < NID_COUNT - 2; i++)
    {
        USHORT nId = aInsert[i];
        aVSet.InsertItem(nId, aIList.GetImage(nId));
        // -2, weil es fuer Next/Prev keinen String gibt
        USHORT nResStr = ST_TBL - 2 + nId - NID_START;
        aVSet.SetItemText(nId, ResId(nResStr));
    }
    // erst hier!
    FreeResource();

    // diese Strings sind global
    for( i = 0; i < 2 * NID_COUNT; i++)
    {
        sQuickHelp[i] = String(SW_RES(STR_IMGBTN_START + i));
    }

    Size aImgSize = aIList.GetImageSize();
    aImgSize.Width() += 5;
    aImgSize.Height() += 5;
    Size aSz = aVSet.CalcWindowSizePixel(aImgSize, aVSet.GetItemCount() / 2, 2);
    aVSet.SetPosSizePixel( Point(), aSz );
    SetOutputSizePixel(aSz);
    aVSet.SetSelectHdl(LINK(this, SwScrollNaviPopup, SelectHdl));
    aVSet.StartSelection();
    aVSet.Show();
    SetBackground(Wallpaper(Color(COL_WHITE)));
}
/*-----------------19.02.97 12.45-------------------

--------------------------------------------------*/

SwScrollNaviPopup::~SwScrollNaviPopup()
{
}
/*-----------------19.02.97 13.58-------------------

--------------------------------------------------*/

SfxPopupWindow* SwScrollNaviPopup::Clone() const
{
    return new SwScrollNaviPopup(GetId(), rResId, ( SfxBindings & ) GetBindings());
}

/*-----------------19.02.97 14.10-------------------

--------------------------------------------------*/

IMPL_LINK(SwScrollNaviPopup, SelectHdl, ValueSet*, pSet)
{
    USHORT nSet = pSet->GetSelectItemId();
    if( nSet != NID_PREV && nSet != NID_NEXT )
    {
        SwView::SetMoveType(nSet);
        aVSet.SetItemText(NID_NEXT, sQuickHelp[nSet - NID_START]);
        aVSet.SetItemText(NID_PREV, sQuickHelp[nSet - NID_START + NID_COUNT]);
    }
    else
    {
        SfxBoolItem aNext(FN_SCROLL_NEXT_PREV, NID_NEXT == nSet);
        SfxViewFrame::Current()->GetDispatcher()->
                Execute(FN_SCROLL_NEXT_PREV,SFX_CALLMODE_SYNCHRON ,&aNext, 0L);
    }
    return 0;
}
/*-----------------23.02.97 18.21-------------------

--------------------------------------------------*/

void SwScrollNaviValueSet::MouseButtonUp( const MouseEvent& rMEvt )
{
    ValueSet::MouseButtonUp(rMEvt);
    if ( ((SwScrollNaviPopup*)GetParent())->IsInPopupMode() )
        ((SwScrollNaviPopup*)GetParent())->EndPopupMode( FLOATWIN_POPUPMODEEND_CLOSEALL );
}

/*-----------------20.06.97 13:28-------------------

--------------------------------------------------*/
void  SwScrollNaviValueSet::RequestHelp( const HelpEvent& rHEvt )
{
    USHORT nMoveType = SwView::GetMoveType();
    SetItemText(NID_NEXT, SwScrollNaviPopup::GetQuickHelpText(TRUE));
    SetItemText(NID_PREV, SwScrollNaviPopup::GetQuickHelpText(FALSE));
    ValueSet::RequestHelp( rHEvt );

}
/*-----------------21.02.97 11:25-------------------

--------------------------------------------------*/

void  SwScrollNaviPopup::PopupModeEnd()
{
    if(aVSet.GetItemCount() < NID_COUNT)
    {
        aVSet.InsertItem(NID_NEXT, aIList.GetImage(NID_NEXT), NID_COUNT);
        aVSet.InsertItem(NID_PREV, aIList.GetImage(NID_PREV), NID_COUNT/2 - 1);
        USHORT nItemBits = aVSet.GetItemBits(NID_NEXT);
        nItemBits |= VIB_NODOUBLECLICK;
        aVSet.SetItemBits(NID_NEXT, nItemBits);
        aVSet.SetItemBits(NID_PREV, nItemBits);
        Size aImgSize = aIList.GetImageSize();
        aImgSize.Width() += 5;
        aImgSize.Height() += 5;
        aVSet.SetColCount(NID_COUNT/2);
        Size aSz = aVSet.CalcWindowSizePixel(aImgSize, NID_COUNT / 2, 2);
        aVSet.SetPosSizePixel( Point(), aSz );
        SetOutputSizePixel(aSz);
    }
    SfxPopupWindow::PopupModeEnd();
}

/*-----------------20.06.97 13:41-------------------

--------------------------------------------------*/
String  SwScrollNaviPopup::GetQuickHelpText(BOOL bNext)
{
    USHORT nResId = STR_IMGBTN_START;
    nResId += SwView::GetMoveType() - NID_START;
    if(!bNext)
        nResId += NID_COUNT;
    return String(SW_RES(nResId));
}


/*-----------------20.02.97 10:17-------------------

--------------------------------------------------*/

void SwNaviImageButton::MouseButtonDown( const MouseEvent& rMEvt )
{
    if(MOUSE_LEFT == rMEvt.GetButtons())
    {
        SfxBindings& rBind = SfxViewFrame::Current()->GetBindings();
        rBind.ENTERREGISTRATIONS();
        pPopup = new
            SwScrollNaviPopup(FN_SCROLL_NAVIGATION,
                            SW_RES(RID_SCROLL_NAVIGATION_WIN),
                            rBind);
        rBind.LEAVEREGISTRATIONS();
        Point aPos = OutputToScreenPixel(Point(0,0));
        Rectangle aRect(aPos, GetSizePixel());
        pPopup->StartPopupMode(aRect, FLOATWIN_POPUPMODE_LEFT|FLOATWIN_POPUPMODE_ALLOWTEAROFF);
    }
    else
        ImageButton::MouseButtonDown(rMEvt);
}
/*-----------------20.02.97 10:17-------------------

--------------------------------------------------*/

void SwNaviImageButton::MouseButtonUp( const MouseEvent& rMEvt )
{
}

/*-----------------21.02.97 09:41-------------------

--------------------------------------------------*/

void SwHlpImageButton::RequestHelp( const HelpEvent& rHEvt )
{

    SetQuickHelpText(SwScrollNaviPopup::GetQuickHelpText(!bUp));

    ImageButton::RequestHelp(rHEvt);
}

/*-----------------25.02.97 12:38-------------------

--------------------------------------------------*/

SwNaviImageButton::SwNaviImageButton(Window* pParent) :
            ImageButton(pParent, SW_RES(BTN_NAVI)), pPopup(0),
            aImage(ResId(IMG_BTN)),
            sQuickText(ResId(ST_QUICK))
{
    FreeResource();
    SetStyle(GetStyle()|WB_NOPOINTERFOCUS);
    SetQuickHelpText(sQuickText);
    SetImage(aImage);
}

/***********************************************************************

        $Log: not supported by cvs2svn $
        Revision 1.62  2000/09/18 16:06:02  willem.vandorp
        OpenOffice header added.

        Revision 1.61  2000/09/07 15:59:27  os
        change: SFX_DISPATCHER/SFX_BINDINGS removed

        Revision 1.60  2000/07/03 08:54:35  jp
        must changes for VCL

        Revision 1.59  2000/06/13 09:58:28  os
        using UCB

        Revision 1.58  2000/05/24 12:36:18  hr
        conflict between STLPORT and Workshop header

        Revision 1.57  2000/04/18 14:54:44  os
        UNICODE

        Revision 1.56  2000/03/03 15:17:03  os
        StarView remainders removed

        Revision 1.55  2000/02/10 10:37:03  os
        #70359# titles added to AutoText groups

        Revision 1.54  1999/04/01 11:58:32  OS
        #64278# Zugriff auf Gruppen und Eintraege nur noch ueber Ids


      Rev 1.53   01 Apr 1999 13:58:32   OS
   #64278# Zugriff auf Gruppen und Eintraege nur noch ueber Ids

      Rev 1.52   09 Feb 1999 10:47:44   OS
   #61205# AutoText-Gruppen koennen beliebige Namen erhalten

      Rev 1.51   05 Feb 1999 15:52:58   MA
   #61463# Kein Popup fuer Glossary in Readonly

      Rev 1.50   14 Jan 1999 15:49:58   JP
   Bug #60794#: Fehlererkennung beim Tabellenrechnen und anspringen von Formeln

      Rev 1.49   14 Jan 1999 14:18:46   JP
   Bug #60794#: Fehlererkennung beim Tabellenrechnen und anspringen von Formeln

      Rev 1.48   03 Dec 1998 10:25:20   OS
   #59441# Verzeichniseintrag in der Navigation

      Rev 1.47   03 Nov 1998 12:24:56   OS
   58830# Enter/LeaveRegistrations

      Rev 1.46   12 Oct 1998 08:46:42   OS
   #52973# GetImage() mit Module* aufrufen

      Rev 1.45   18 Aug 1998 17:28:44   OS
   GetShortName sollte auf bekannte Gruppe richtig reagieren #55219#

      Rev 1.44   15 Jun 1998 10:39:28   OS
   WB_NOPOINTERFOCUS

      Rev 1.43   15 Jun 1998 09:36:42   OS
   AutoText-Popup: Kuerzel mit angeben

      Rev 1.42   24 Feb 1998 12:10:32   OS
   Navigationstool erweitert

      Rev 1.41   18 Feb 1998 14:45:12   OS
   ItemBits an der richtigen Position setzen #46669#

      Rev 1.40   16 Feb 1998 08:03:22   OS
   VIB_NODOUBLECLICK setzen #46669#

      Rev 1.39   07 Feb 1998 11:09:16   OS
   initiale Groesse berichtigt #47144#

      Rev 1.38   20 Jan 1998 12:27:42   OS
   Next und Prev im SelectHdl ausfuehren #46669#

      Rev 1.37   28 Nov 1997 20:17:34   MA
   includes

      Rev 1.36   24 Nov 1997 14:53:58   MA
   includes

      Rev 1.35   17 Oct 1997 09:11:08   OS
   EndPopup rufen #44773#

      Rev 1.34   19 Sep 1997 12:51:24   PB
   chg: fuer VCL ToolBox::SetItemDown() eingebaut

      Rev 1.33   18 Sep 1997 13:59:46   OS
   Pointer::GetPosPixel fuer VCL ersetzt

      Rev 1.32   18 Sep 1997 13:18:08   TJ
   include

      Rev 1.31   29 Aug 1997 15:45:34   OS
   PopupMenu::Execute mit Window* fuer VCL

      Rev 1.30   29 Aug 1997 14:27:06   OS
   DLL-Umbau

      Rev 1.29   09 Jul 1997 17:36:10   HJS
   includes

      Rev 1.28   20 Jun 1997 13:52:52   OS
   Merker und Notizen in der Navigation

      Rev 1.27   07 May 1997 15:30:12   OS
   Execute mit 0L abschliessen fuer W16

      Rev 1.26   05 May 1997 12:26:24   OS
   QuickHelpText als Member, MouseButtonDwon: nur linke Taste!

      Rev 1.25   07 Apr 1997 12:36:44   OS
   HelpId fuer ValueSet

      Rev 1.24   24 Mar 1997 01:43:52   OS
   Alignment aussen einstellen

      Rev 1.23   23 Mar 1997 23:54:26   OS
   ToolBoxAlign statt ToolBoxControl uebergeben

      Rev 1.22   25 Feb 1997 13:04:24   OS
   Spruenge im MouseButtonUp

      Rev 1.21   23 Feb 1997 22:07:40   OS
   richtige Hilfetexte fuer Prev/Next-Buttons

      Rev 1.20   23 Feb 1997 19:36:52   OS
   MouseButtonDown im ValueSet erledigt die Spruenge -> kein hakliges Verhalten mehr

      Rev 1.19   22 Feb 1997 20:16:26   OS
   eigenes Image fuer Button

      Rev 1.18   21 Feb 1997 19:35:42   OS
   mit Prev/Next-Button

      Rev 1.17   21 Feb 1997 17:04:48   OS
   fast fertig

      Rev 1.16   20 Feb 1997 16:45:02   OS
   Navigation funktioniert

      Rev 1.15   19 Feb 1997 20:56:26   HJS
   auf nicht const gecastet

      Rev 1.14   19 Feb 1997 16:55:16   OS
   Popup fuer Navigation

      Rev 1.13   17 Feb 1997 10:39:42   OM
   Neue Feldbefehle im Webmode

      Rev 1.12   06 Feb 1997 14:50:26   OM
   feldbefehl Seitennummer im html-Mode erlauben

      Rev 1.11   06 Feb 1997 14:32:52   OM
   Popup-Controller fuer Feldbefehle einfuegen

      Rev 1.10   29 Jan 1997 14:52:24   OM
   Draw-Toolboxen ins Svx verschoben

      Rev 1.9   11 Nov 1996 11:15:14   MA
   ResMgr

      Rev 1.8   27 Sep 1996 16:21:26   HJS
   vertipper

      Rev 1.7   27 Sep 1996 12:56:42   OS
   PopupMenu wird bei Bedarf angelegt

      Rev 1.6   26 Sep 1996 16:57:00   OS
   Update der AutoText-Liste

      Rev 1.5   24 Sep 1996 16:45:06   OS
   View-Wechsel vor jedem Aufruf

      Rev 1.4   23 Sep 1996 13:43:46   OS
   beschleunigter Zugriff auf den GlosHdl

      Rev 1.3   23 Sep 1996 08:11:42   OS
   Menu fuer Autotexte

      Rev 1.2   20 Sep 1996 14:44:28   OS
   Zuerst kommt die Toolbox ohne Timer

      Rev 1.1   19 Sep 1996 19:40:04   HJS
   fehlende includes

      Rev 1.0   19 Sep 1996 16:02:36   OS
   Initial revision.

***********************************************************************/


