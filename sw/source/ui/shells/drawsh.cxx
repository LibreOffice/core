/*************************************************************************
 *
 *  $RCSfile: drawsh.cxx,v $
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


#ifndef _SVDVIEW_HXX //autogen
#include <svx/svdview.hxx>
#endif
#ifndef _SVDOTEXT_HXX //autogen
#include <svx/svdotext.hxx>
#endif
#ifndef _SFX_WHITER_HXX //autogen
#include <svtools/whiter.hxx>
#endif
#ifndef _SFXENUMITEM_HXX //autogen
#include <svtools/eitem.hxx>
#endif
#ifndef _SVX_FONTWORK_HXX //autogen
#include <svx/fontwork.hxx>
#endif
#ifndef _SFXREQUEST_HXX //autogen
#include <sfx2/request.hxx>
#endif
#ifndef _SFX_BINDINGS_HXX //autogen
#include <sfx2/bindings.hxx>
#endif
#ifndef _SFXOBJFACE_HXX //autogen
#include <sfx2/objface.hxx>
#endif
#ifndef _SFXITEMITER_HXX //autogen
#include <svtools/itemiter.hxx>
#endif
#ifndef _OFF_APP_HXX //autogen
#include <offmgr/app.hxx>
#endif
#ifndef _SVX_SRCHITEM_HXX
#include <svx/srchitem.hxx>
#endif
#ifndef _SVX_XFTSFIT_HXX //autogen
#include <svx/xftsfit.hxx>
#endif
#ifndef _SFXVIEWFRM_HXX
#include <sfx2/viewfrm.hxx>
#endif


#include "swundo.hxx"
#include "view.hxx"
#include "wrtsh.hxx"
#include "cmdid.h"
#include "globals.hrc"
#include "helpid.h"
#include "popup.hrc"
#include "shells.hrc"
#include "drwbassh.hxx"
#include "drawsh.hxx"

#define SwDrawShell
#include "itemdef.hxx"
#include "swslots.hxx"


SFX_IMPL_INTERFACE(SwDrawShell, SwDrawBaseShell, SW_RES(STR_SHELLNAME_DRAW))
{
    SFX_POPUPMENU_REGISTRATION(SW_RES(MN_DRAW_POPUPMENU));
    SFX_OBJECTBAR_REGISTRATION(SFX_OBJECTBAR_OBJECT, SW_RES(RID_DRAW_TOOLBOX));
    SFX_OBJECTMENU_REGISTRATION(SID_OBJECTMENU0, SW_RES(MN_OBJECTMENU_DRAW));
    SFX_CHILDWINDOW_REGISTRATION(SvxFontWorkChildWindow::GetChildWindowId());
}

TYPEINIT1(SwDrawShell,SwDrawBaseShell)

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/


void SwDrawShell::Execute(SfxRequest &rReq)
{
    SwWrtShell          &rSh = GetShell();
    SdrView             *pSdrView = rSh.GetDrawView();
    const SfxItemSet    *pArgs = rReq.GetArgs();
    SfxBindings         &rBnd  = GetView().GetViewFrame()->GetBindings();
    USHORT               nSlotId = rReq.GetSlot();
    BOOL                 bChanged = pSdrView->GetModel()->IsChanged();

    pSdrView->GetModel()->SetChanged(FALSE);

    const SfxPoolItem* pItem;
    if(pArgs)
        pArgs->GetItemState(nSlotId, FALSE, &pItem);

    BOOL bMirror = TRUE, bTopParam = TRUE, bBottomParam = TRUE;

    switch (nSlotId)
    {
        case SID_OBJECT_ROTATE:
            if (rSh.IsObjSelected() && pSdrView->IsRotateAllowed())
            {
                if (GetView().IsDrawRotate())
                    rSh.SetDragMode(SDRDRAG_MOVE);
                else
                    rSh.SetDragMode(SDRDRAG_ROTATE);

                GetView().FlipDrawRotate();
            }
            break;

        case SID_BEZIER_EDIT:
            if (GetView().IsDrawRotate())
            {
                rSh.SetDragMode(SDRDRAG_MOVE);
                GetView().FlipDrawRotate();
            }
            GetView().FlipDrawSelMode();
            pSdrView->SetFrameDragSingles(GetView().IsDrawSelMode());
            GetView().AttrChangedNotify(&rSh); // Shellwechsel...
            break;

        case SID_OBJECT_HELL:
            if (rSh.IsObjSelected())
            {
                rSh.StartUndo( UNDO_START );
                SetWrapMode(FN_FRAME_WRAPTHRU_TRANSP);
                rSh.SelectionToHell();
                rSh.EndUndo( UNDO_END );
                rBnd.Invalidate(SID_OBJECT_HEAVEN);
            }
            break;

        case SID_OBJECT_HEAVEN:
            if (rSh.IsObjSelected())
            {
                rSh.StartUndo( UNDO_START );
                SetWrapMode(FN_FRAME_WRAPTHRU);
                rSh.SelectionToHeaven();
                rSh.EndUndo( UNDO_END );
                rBnd.Invalidate(SID_OBJECT_HELL);
            }
            break;

        case FN_TOOL_HIERARCHIE:
            if (rSh.IsObjSelected())
            {
                rSh.StartUndo( UNDO_START );
                if (rSh.GetLayerId() == 0)
                {
                    SetWrapMode(FN_FRAME_WRAPTHRU);
                    rSh.SelectionToHeaven();
                }
                else
                {
                    SetWrapMode(FN_FRAME_WRAPTHRU_TRANSP);
                    rSh.SelectionToHell();
                }
                rSh.EndUndo( UNDO_END );
                rBnd.Invalidate( SID_OBJECT_HELL, SID_OBJECT_HEAVEN, 0 );
            }
            break;

        case FN_TOOL_VMIRROR:
            bMirror = FALSE;
            /* no break */
        case FN_TOOL_HMIRROR:
            rSh.MirrorSelection( bMirror );
            break;

        case SID_FONTWORK:
        {
            SfxViewFrame* pVFrame = GetView().GetViewFrame();
            if (pArgs)
            {
                pVFrame->SetChildWindow(SvxFontWorkChildWindow::GetChildWindowId(),
                    ((const SfxBoolItem&)(pArgs->Get(SID_FONTWORK))).GetValue());
            }
            else
                pVFrame->ToggleChildWindow( SvxFontWorkChildWindow::GetChildWindowId() );
            pVFrame->GetBindings().Invalidate(SID_FONTWORK);
        }
        break;

        default:
            DBG_ASSERT(!this, "falscher Dispatcher");
            return;
    }
    if (pSdrView->GetModel()->IsChanged())
        rSh.SetModified();
    else if (bChanged)
        pSdrView->GetModel()->SetChanged(TRUE);
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/



void SwDrawShell::GetState(SfxItemSet& rSet)
{
    SwWrtShell &rSh = GetShell();
    SdrView* pSdrView = rSh.GetDrawViewWithValidMarkList();
    SfxWhichIter aIter( rSet );
    USHORT nWhich = aIter.FirstWhich();
    BOOL bProtected = rSh.IsSelObjProtected(FLYPROTECT_CONTENT);

    if (!bProtected)    // Im Parent nachsehen
        bProtected |= rSh.IsSelObjProtected( (FlyProtectType)(FLYPROTECT_CONTENT|FLYPROTECT_PARENT) ) != 0;

    while( nWhich )
    {
        switch( nWhich )
        {
            case SID_OBJECT_HELL:
                if ( !rSh.IsObjSelected() || rSh.GetLayerId() == 0 || bProtected )
                    rSet.DisableItem( nWhich );
                break;

            case SID_OBJECT_HEAVEN:
                if ( !rSh.IsObjSelected() || rSh.GetLayerId() == 1 || bProtected )
                    rSet.DisableItem( nWhich );
                break;

            case FN_TOOL_HIERARCHIE:
                if ( !rSh.IsObjSelected() || bProtected )
                    rSet.DisableItem( nWhich );
                break;

            case SID_OBJECT_ROTATE:
            {
                const BOOL bIsRotate = GetView().IsDrawRotate();
                if ( !bIsRotate && !pSdrView->IsRotateAllowed() || bProtected )
                    rSet.DisableItem( nWhich );
                else
                    rSet.Put( SfxBoolItem( nWhich, bIsRotate ) );
            }
            break;

            case SID_BEZIER_EDIT:
                if (!Disable(rSet, nWhich))
                    rSet.Put( SfxBoolItem( nWhich, !GetView().IsDrawSelMode()));
            break;

            case FN_TOOL_VMIRROR:
                if ( !pSdrView->IsMirrorAllowed() || bProtected )
                    rSet.DisableItem( nWhich );
                break;

            case FN_TOOL_HMIRROR:
                if ( !pSdrView->IsMirrorAllowed() || bProtected )
                    rSet.DisableItem( nWhich );
                break;

            case SID_FONTWORK:
            {
                if (bProtected)
                    rSet.DisableItem( nWhich );
                else
                {
                    const USHORT nId = SvxFontWorkChildWindow::GetChildWindowId();
                    rSet.Put(SfxBoolItem( nWhich , GetView().GetViewFrame()->HasChildWindow(nId)));
                }
            }
            break;
        }
        nWhich = aIter.NextWhich();
    }
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/



SwDrawShell::SwDrawShell(SwView &rView) :
    SwDrawBaseShell(rView)
{
    SetHelpId(SW_DRAWSHELL);
    SetName(String::CreateFromAscii("Draw"));
}

/*************************************************************************
|*
|* SfxRequests fuer FontWork bearbeiten
|*
\************************************************************************/



void SwDrawShell::ExecFormText(SfxRequest& rReq)
{
    SwWrtShell &rSh = GetShell();
    SdrView*    pDrView = rSh.GetDrawView();
    BOOL        bChanged = pDrView->GetModel()->IsChanged();
    pDrView->GetModel()->SetChanged(FALSE);

    const SdrMarkList& rMarkList = pDrView->GetMarkList();

    if ( rMarkList.GetMarkCount() == 1 && rReq.GetArgs() )
    {
        const SfxItemSet& rSet = *rReq.GetArgs();
        const SfxPoolItem* pItem;

        if ( pDrView->IsTextEdit() )
        {
            pDrView->EndTextEdit( TRUE );
            GetView().AttrChangedNotify(&rSh);
        }

        if ( rSet.GetItemState(XATTR_FORMTXTSTDFORM, TRUE, &pItem) ==
             SFX_ITEM_SET &&
            ((const XFormTextStdFormItem*) pItem)->GetValue() != XFTFORM_NONE )
        {

            const USHORT nId = SvxFontWorkChildWindow::GetChildWindowId();

            SvxFontWorkDialog* pDlg = (SvxFontWorkDialog*)(GetView().GetViewFrame()->
                                        GetChildWindow(nId)->GetWindow());

            pDlg->CreateStdFormObj(*pDrView, *pDrView->GetPageViewPvNum(0),
                                    rSet, *rMarkList.GetMark(0)->GetObj(),
                                   ((const XFormTextStdFormItem*) pItem)->
                                   GetValue());

        }
        else
            pDrView->SetAttributes(rSet);
    }
    if (pDrView->GetModel()->IsChanged())
        rSh.SetModified();
    else
        if (bChanged)
            pDrView->GetModel()->SetChanged(TRUE);
}

/*************************************************************************
|*
|* Statuswerte fuer FontWork zurueckgeben
|*
\************************************************************************/



void SwDrawShell::GetFormTextState(SfxItemSet& rSet)
{
    SwWrtShell &rSh = GetShell();
    SdrView* pDrView = rSh.GetDrawView();
    const SdrMarkList& rMarkList = pDrView->GetMarkList();
    const SdrObject* pObj = NULL;
    SvxFontWorkDialog* pDlg = NULL;

    const USHORT nId = SvxFontWorkChildWindow::GetChildWindowId();

    SfxViewFrame* pVFrame = GetView().GetViewFrame();
    if ( pVFrame->HasChildWindow(nId) )
        pDlg = (SvxFontWorkDialog*)(pVFrame->GetChildWindow(nId)->GetWindow());

    if ( rMarkList.GetMarkCount() == 1 )
        pObj = rMarkList.GetMark(0)->GetObj();

    if ( pObj == NULL || !pObj->ISA(SdrTextObj) ||
        !((SdrTextObj*) pObj)->HasText() )
    {
#define XATTR_ANZ 12
        static const USHORT nXAttr[ XATTR_ANZ ] =
        {
            XATTR_FORMTXTSTYLE, XATTR_FORMTXTADJUST, XATTR_FORMTXTDISTANCE,
            XATTR_FORMTXTSTART, XATTR_FORMTXTMIRROR, XATTR_FORMTXTSTDFORM,
            XATTR_FORMTXTHIDEFORM, XATTR_FORMTXTOUTLINE, XATTR_FORMTXTSHADOW,
            XATTR_FORMTXTSHDWCOLOR, XATTR_FORMTXTSHDWXVAL, XATTR_FORMTXTSHDWYVAL
        };
        for( USHORT i = 0; i < XATTR_ANZ; )
            rSet.DisableItem( nXAttr[ i++ ] );
    }
    else
    {
        if ( pDlg )
            pDlg->SetColorTable(OFF_APP()->GetStdColorTable());

        pDrView->GetAttributes( rSet );
    }
}

/*************************************************************************

      Source Code Control System - History

      $Log: not supported by cvs2svn $
      Revision 1.143  2000/09/18 16:06:03  willem.vandorp
      OpenOffice header added.

      Revision 1.142  2000/09/08 08:12:51  os
      Change: Set/Toggle/Has/Knows/Show/GetChildWindow

      Revision 1.141  2000/09/07 15:59:29  os
      change: SFX_DISPATCHER/SFX_BINDINGS removed

      Revision 1.140  2000/05/26 07:21:32  os
      old SW Basic API Slots removed

      Revision 1.139  2000/05/10 11:53:01  os
      Basic API removed

      Revision 1.138  2000/04/18 14:58:23  os
      UNICODE

      Revision 1.137  1999/06/21 13:20:24  JP
      Interface changes: SdrView::GetAttributes


      Rev 1.136   21 Jun 1999 15:20:24   JP
   Interface changes: SdrView::GetAttributes

      Rev 1.135   12 Jan 1999 17:13:44   TJ
   include

      Rev 1.134   11 Jan 1999 12:41:48   MA
   #60294# Undo-Klammerung vervollstaendigt

      Rev 1.133   06 Oct 1998 12:44:58   JP
   Bug #57153#: neu GetDrawViewWithValidMarkList

      Rev 1.132   10 Jun 1998 11:23:18   OM
   Hell/Heaven nicht in FormShell

      Rev 1.131   15 Apr 1998 15:32:24   OM
   #49467 Objekte innerhalb von geschuetzten Rahmen duerfen nicht veraendert werden

      Rev 1.130   29 Nov 1997 15:52:14   MA
   includes

      Rev 1.129   24 Nov 1997 09:46:56   MA
   includes

      Rev 1.128   03 Nov 1997 13:55:44   MA
   precomp entfernt

      Rev 1.127   01 Sep 1997 13:23:04   OS
   DLL-Umstellung

      Rev 1.126   05 Aug 1997 16:21:54   TJ
   include svx/srchitem.hxx

      Rev 1.125   05 Aug 1997 13:29:54   MH
   chg: header

      Rev 1.124   01 Aug 1997 11:47:18   MH
   chg: header

      Rev 1.123   18 Jun 1997 18:10:34   HJS
   Typeinit wieder hinein

      Rev 1.122   17 Jun 1997 16:15:18   MA
   DrawTxtShell nicht von BaseShell ableiten + Opts

      Rev 1.121   13 May 1997 13:24:26   OM
   Unnoetigen Include entfernt

      Rev 1.120   07 Apr 1997 17:41:54   MH
   chg: header

      Rev 1.119   24 Feb 1997 16:53:08   OM
   Silbentrennung in Draw-Objekten

      Rev 1.118   22 Jan 1997 14:40:30   OM
   unnoetige Includes entfernt

      Rev 1.117   22 Jan 1997 11:20:50   OM
   Neue Shells: DrawBaseShell und DrawControlShell

      Rev 1.116   16 Jan 1997 16:46:46   OM
   Editmode on/off nur im richtigen Mode

      Rev 1.115   14 Jan 1997 15:37:58   OM
   Beziermode korrigiert

      Rev 1.114   13 Jan 1997 15:56:10   OM
   Bezier-Punkte loeschen

      Rev 1.113   13 Jan 1997 10:12:10   OM
   Selektionsmode fuer Bezier-Objekte

      Rev 1.112   09 Jan 1997 16:24:02   OM
   Selektionsmode fuer Bezier-Objekte

      Rev 1.111   08 Jan 1997 12:48:58   OM
   Neue Segs

      Rev 1.110   08 Jan 1997 12:47:22   OM
   Aenderungen fuer Bezier-Shell

      Rev 1.109   18 Dec 1996 15:47:28   OM
   #34500# Keine Texteingabe im Drawmode

      Rev 1.108   13 Dec 1996 12:19:10   OM
   Bezier-Controller Status und Execute

      Rev 1.107   11 Dec 1996 12:34:06   OM
   Bezier-Controller

      Rev 1.106   30 Nov 1996 14:06:16   OS
   Ausrichtung ueber Menue: EnumItem auswerten

      Rev 1.105   29 Nov 1996 17:02:48   OS
   FN_TOOL_ANKER jetzt in der Baseshell

      Rev 1.104   27 Nov 1996 16:15:18   OM
   #33427# Zeichenobjekte: automatischer Layerwechsel je nach Umlaufart

      Rev 1.103   26 Nov 1996 11:56:22   NF
   defines entfernt...

      Rev 1.102   25 Nov 1996 10:55:28   OS
   Slotumrechnung fuer SID_OBJECT_ALIGN entfernt

      Rev 1.101   18 Nov 1996 13:39:32   MA
   chg: State fuer align

      Rev 1.100   15 Nov 1996 19:38:56   MA
   opt, include

      Rev 1.99   11 Nov 1996 13:37:34   MA
   ResMgr

      Rev 1.97   08 Nov 1996 11:59:20   HJS
   include w.g. positivdefine

      Rev 1.96   07 Nov 1996 16:20:08   OS
   Wrap- und Anker-Methoden an die BaseShell

      Rev 1.95   17 Oct 1996 14:35:18   MH
   add: include

      Rev 1.94   01 Oct 1996 16:54:00   OM
   Hyperlinks editieren

      Rev 1.93   28 Aug 1996 15:54:54   OS
   includes

      Rev 1.92   23 Aug 1996 12:51:56   HJS
   clooks

      Rev 1.91   22 Aug 1996 16:37:44   NF
   clooks

      Rev 1.90   14 Aug 1996 19:33:06   JP
   svdraw.hxx entfernt

      Rev 1.89   13 Aug 1996 12:38:42   OS
   neue Shellnamen im IDL-Interface

      Rev 1.88   09 Aug 1996 15:15:04   OS
   neu: ObjectName und Value an Controls

      Rev 1.87   02 Aug 1996 10:39:36   MH
   Wer zum Teufel braucht sfxexp.hxx an dieser Stelle ?
   Win16 jedenfalls nicht

      Rev 1.86   30 Jul 1996 16:43:02   OS
   FN_TOOL_ANKER_PAGE wieder ohne Statusmethode

      Rev 1.85   27 Jul 1996 14:58:42   OS
   Aenderungen fuer den Html-Mode

      Rev 1.84   24 Jul 1996 11:06:36   NF
   defines in ifdef win geklammert...

      Rev 1.83   24 Jul 1996 09:13:04   NF
   defines entfernt ...

      Rev 1.82   23 Jul 1996 14:33:28   NF
   CLOOKS

*************************************************************************/



