/*************************************************************************
 *
 *  $RCSfile: textidx.cxx,v $
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
#include <hintids.hxx>
#include <uiparam.hxx>
#ifndef _MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif
#ifndef _SFXREQUEST_HXX //autogen
#include <sfx2/request.hxx>
#endif
#ifndef _SFXITEMSET_HXX //autogen
#include <svtools/itemset.hxx>
#endif
#ifndef _SFXVIEWFRM_HXX //autogen
#include <sfx2/viewfrm.hxx>
#endif
#ifndef _SFXSTRITEM_HXX //autogen
#include <svtools/stritem.hxx>
#endif

#ifndef _OFF_APP_HXX //autogen
#include <offmgr/app.hxx>
#endif
#ifndef _SFXENUMITEM_HXX //autogen
#include <svtools/eitem.hxx>
#endif
#ifndef _SVX_SIZEITEM_HXX //autogen
#include <svx/sizeitem.hxx>
#endif
#ifndef _FMTFSIZE_HXX //autogen
#include <fmtfsize.hxx>
#endif
#ifndef _FLDBAS_HXX
#include <fldbas.hxx>
#endif

#include "viewopt.hxx"
#include "cmdid.h"
#include "view.hxx"
#include "wrtsh.hxx"
#include "swundo.hxx"                   // fuer Undo-Ids
#include "textsh.hxx"
#include "idx.hxx"
#include "idxmrk.hxx"
#include "multmrk.hxx"
#include "usridx.hxx"
#include "cnttab.hxx"
#include "toxmgr.hxx"

// STATIC DATA -----------------------------------------------------------

void SwTextShell::ExecIdx(SfxRequest &rReq)
{
    const SfxItemSet *pArgs = rReq.GetArgs();
    const SfxPoolItem* pItem = 0;
    USHORT nSlot = rReq.GetSlot();
    if(pArgs)
       pArgs->GetItemState(nSlot, FALSE, &pItem );

    SfxViewFrame* pVFrame = GetView().GetViewFrame();
    Window *pMDI = &pVFrame->GetWindow();

    switch( nSlot )
    {
        case FN_EDIT_AUTH_ENTRY_DLG :
        {
            SwAuthMarkModalDlg* pDlg = new SwAuthMarkModalDlg(pMDI, GetShell());
            pDlg->Execute();
            delete pDlg;
        }
        break;
        case FN_INSERT_AUTH_ENTRY_DLG:
        {
            // no BASIC support
            pVFrame->ToggleChildWindow(FN_INSERT_AUTH_ENTRY_DLG);

            SwInsertAuthMarkWrapper *pAuthMrk = (SwInsertAuthMarkWrapper*)
                                pVFrame->GetChildWindow(FN_INSERT_AUTH_ENTRY_DLG);

            Invalidate(rReq.GetSlot());
        }
        break;
        case FN_INSERT_IDX_ENTRY_DLG:
        {
            pVFrame->ToggleChildWindow(FN_INSERT_IDX_ENTRY_DLG);
            Invalidate(rReq.GetSlot());
        }
        break;
        case FN_EDIT_IDX_ENTRY_DLG:
        {
            SwTOXMgr aMgr(GetShellPtr());
            USHORT nRet = RET_OK;
            if(aMgr.GetTOXMarkCount() > 1)
            {   // Mehrere Marken, welche solls denn sein ?
                //
                SwMultiTOXMarkDlg* pMultDlg = new SwMultiTOXMarkDlg(pMDI, aMgr);
                nRet = pMultDlg->Execute();
                delete pMultDlg;
            }
            if( nRet == RET_OK)
            {
                SwIndexMarkModalDlg* pDlg = new SwIndexMarkModalDlg(pMDI, GetShell());
                pDlg->Execute();
                delete pDlg;
            }
            break;
        }
        case FN_IDX_MARK_TO_IDX:
        {
            GetShell().GotoTOXMarkBase();
            break;
        }
        case FN_INSERT_MULTI_TOX:
        {
            SfxItemSet aSet(GetPool(),
                            RES_COL, RES_COL,
                            RES_BACKGROUND, RES_BACKGROUND,
                            RES_FRM_SIZE, RES_FRM_SIZE,
                            SID_ATTR_PAGE_SIZE, SID_ATTR_PAGE_SIZE,
                            RES_LR_SPACE, RES_LR_SPACE,
                            FN_PARAM_TOX_TYPE, FN_PARAM_TOX_TYPE,
                            0   );
            SwWrtShell& rSh = GetShell();
            SwRect aRect;
            rSh.CalcBoundRect(aRect, FLY_IN_CNTNT);

            long nWidth = aRect.Width();
            aSet.Put(SwFmtFrmSize(ATT_VAR_SIZE, nWidth));
            // Hoehe=Breite fuer konsistentere Vorschau (analog zu Bereich bearbeiten)
            aSet.Put(SvxSizeItem(SID_ATTR_PAGE_SIZE, Size(nWidth, nWidth)));
            const SwTOXBase* pCurTOX = rSh.GetCurTOX();
            if(pCurTOX)
            {
                const SfxItemSet* pSet = pCurTOX->GetAttrSet();
                if(pSet)
                    aSet.Put(*pSet);
            }

            SwMultiTOXTabDialog* pDlg = new SwMultiTOXTabDialog(pMDI, aSet, rSh, pCurTOX);
            pDlg->Execute();
            delete pDlg;
        }
        break;
        case FN_REMOVE_CUR_TOX:
        {
            SwWrtShell& rSh = GetShell();
            const SwTOXBase* pBase = rSh.GetCurTOX();
            DBG_ASSERT(pBase, "no TOXBase to remove")
            if( pBase )
                rSh.DeleteTOX(*pBase, TRUE);
        }
        break;
        default:
            ASSERT(!this, falscher Dispatcher);
            return;
    }
}


void SwTextShell::GetIdxState(SfxItemSet &rSet)
{
    SwWrtShell& rSh = GetShell();
    SfxViewFrame* pVFrame = GetView().GetViewFrame();
    SwInsertIdxMarkWrapper *pIdxMrk = (SwInsertIdxMarkWrapper*)
                        pVFrame->GetChildWindow(FN_INSERT_IDX_ENTRY_DLG);

    SfxChildWindow* pAuthMark = pVFrame->GetChildWindow(FN_INSERT_AUTH_ENTRY_DLG);

    const BOOL bHtmlMode = 0 != ::GetHtmlMode( GetView().GetDocShell() );
    const SwTOXBase* pBase = 0;
    if( bHtmlMode || 0 != ( pBase = rSh.GetCurTOX()) )
    {
        USHORT nBase = 0;
        if( pBase )
        {
            if(pBase->IsTOXBaseInReadonly())
            {
                rSet.DisableItem( FN_INSERT_MULTI_TOX );
            }
        }

        rSet.DisableItem( FN_EDIT_IDX_ENTRY_DLG );
        rSet.DisableItem( FN_EDIT_AUTH_ENTRY_DLG );

        if(!pIdxMrk)
            rSet.DisableItem( FN_INSERT_IDX_ENTRY_DLG );
        else
            rSet.Put(SfxBoolItem(FN_INSERT_IDX_ENTRY_DLG, TRUE));

        if(!pAuthMark)
            rSet.DisableItem( FN_INSERT_AUTH_ENTRY_DLG );
        else
            rSet.Put(SfxBoolItem(FN_INSERT_AUTH_ENTRY_DLG, TRUE));

    }
    else //if( SFX_ITEM_UNKNOWN != rSet.GetItemState( FN_EDIT_IDX_ENTRY_DLG ))
    {

        BOOL bEnableEdit = TRUE;
        BOOL bInReadonly = rSh.HasReadonlySel();
        if( rSh.HasSelection() || bInReadonly)
            bEnableEdit = FALSE;
        else
        {
            SwTOXMarks aArr;
            rSh.GetCurTOXMarks( aArr );
            if( !aArr.Count())
                bEnableEdit = FALSE;
        }

        if(!bEnableEdit)
            rSet.DisableItem( FN_EDIT_IDX_ENTRY_DLG );

        if(bInReadonly)
        {
            rSet.DisableItem(FN_INSERT_IDX_ENTRY_DLG);
            rSet.DisableItem( FN_INSERT_MULTI_TOX );
        }
        else
            rSet.Put(SfxBoolItem(FN_INSERT_IDX_ENTRY_DLG,
                                    0 != pIdxMrk));


        SwField* pField = rSh.GetCurFld();

        if(bInReadonly)
            rSet.DisableItem(FN_INSERT_AUTH_ENTRY_DLG);
        else
            rSet.Put(SfxBoolItem(FN_INSERT_AUTH_ENTRY_DLG, 0 != pAuthMark));

        if( bInReadonly || !pField ||
            pField->GetTyp()->Which() != RES_AUTHORITY)
            rSet.DisableItem(FN_EDIT_AUTH_ENTRY_DLG);
        rSet.DisableItem(FN_REMOVE_CUR_TOX);
    }
}

// -----------------------------------------------------------------------
/*------------------------------------------------------------------------

    $Log: not supported by cvs2svn $
    Revision 1.42  2000/09/18 16:06:06  willem.vandorp
    OpenOffice header added.

    Revision 1.41  2000/09/08 08:12:52  os
    Change: Set/Toggle/Has/Knows/Show/GetChildWindow

    Revision 1.40  2000/05/26 07:21:33  os
    old SW Basic API Slots removed

    Revision 1.39  2000/02/11 14:57:51  hr
    #70473# changes for unicode ( patched by automated patchtool )

    Revision 1.38  2000/01/17 13:33:00  os
    #70703# apply to all similar texts: search options case sensitive and word only

    Revision 1.37  1999/12/10 15:17:04  os
    #70711# readonly check for index entries now in GetIdxState

    Revision 1.36  1999/10/15 11:44:08  os
    new: FN_REMOVE_TOX

    Revision 1.35  1999/10/06 12:15:01  os
    edit tox entry dialogs now modal

    Revision 1.34  1999/09/15 13:57:53  os
    dialog for authorities

    Revision 1.33  1999/09/08 12:43:18  os
    GetIdxState corrected

    Revision 1.32  1999/09/07 13:55:52  os
    Insert/EditIndexEntry as FloatingWindow

    Revision 1.31  1999/08/25 13:27:20  OS
    extended indexes: remove old code


      Rev 1.30   25 Aug 1999 15:27:20   OS
   extended indexes: remove old code

      Rev 1.29   23 Aug 1999 09:51:52   OS
   extended indexes: InsertTOXMark simplified

      Rev 1.28   17 Aug 1999 13:59:02   OS
   extended indexes: get/set section attributes

      Rev 1.27   30 Jul 1999 13:14:36   OS
   indexes: check readonly in status method

      Rev 1.26   07 Jul 1999 08:13:46   OS
   extended indexes: SwTOXMgr interface changed

      Rev 1.25   02 Jul 1999 11:07:50   OS
   extended indexes: smaller interface SwTOXMgr

      Rev 1.24   21 Jun 1999 10:00:52   OS
   extended indexes

      Rev 1.23   29 Mar 1999 14:37:24   OS
   #63929# Range fuer ItemSet festlegen

      Rev 1.22   29 Mar 1999 12:19:30   OS
   #63929# neuer Verzeichnisdialog vollstaendig

      Rev 1.21   26 Mar 1999 11:38:26   OS
   #63929# Neuer Verzeichnisdialog

      Rev 1.20   20 Jan 1999 11:35:58   OS
   #60881# Verzeichniseintraege mehrfach einf?gen

      Rev 1.19   24 Nov 1997 09:47:10   MA
   includes

      Rev 1.18   03 Nov 1997 13:55:44   MA
   precomp entfernt

      Rev 1.17   05 Sep 1997 13:36:00   OS
   pBase initialisieren, Rahmenslektion schliesst TextShell aus

      Rev 1.16   28 Aug 1997 19:48:02   JP
   GetIdxState: optimiert, im Verzeichnis InsertEntry abklemmen

      Rev 1.15   23 Jul 1997 21:28:56   HJS
   includes

      Rev 1.14   07 Jul 1997 09:39:34   OS
   fuer Userverzeichnisse aus Vorlagen muss immer ein Strin uebergeben werden

      Rev 1.13   03 Jul 1997 09:16:42   JP
   FN_INSERT_CNTNTIDX_ENTRY: Level auf 1 defaulten

      Rev 1.12   16 Jun 1997 12:02:26   OS
   Verzeichnisfunktionen und -dialoge ohne Record-Flag

      Rev 1.11   07 Apr 1997 17:44:46   MH
   chg: header

      Rev 1.10   27 Jan 1997 16:30:36   OS
   HtmlMode entfernt

      Rev 1.9   11 Dec 1996 10:50:56   MA
   Warnings

      Rev 1.8   23 Nov 1996 13:24:46   OS
   im HtmlMode Verzeichnis einfuegen disabled

      Rev 1.7   24 Oct 1996 20:59:38   JP
   State: bei Selektion darf kein VerzeichnisEintrag bearbeitet werden

      Rev 1.6   09 Sep 1996 21:16:28   MH
   Umstellungen wg. internal Compiler Errors

      Rev 1.5   28 Aug 1996 15:54:52   OS
   includes

      Rev 1.4   07 Jun 1996 09:12:44   OS
   CreateForm fuer Inhaltsverzeichnis abgesichert Bug #28429#

      Rev 1.3   03 Jun 1996 10:18:06   OS
   vohandene Verzeichnisse beim Einfuegen updaten (Basic) Bug #28233#

      Rev 1.2   22 Mar 1996 15:29:32   TRI
   sfxiiter.hxx included

      Rev 1.1   30 Nov 1995 15:09:58   OM
   Segs

      Rev 1.0   30 Nov 1995 14:26:00   OM
   Initial revision.

------------------------------------------------------------------------*/

