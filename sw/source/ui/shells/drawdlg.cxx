/*************************************************************************
 *
 *  $RCSfile: drawdlg.cxx,v $
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

#define ITEMID_COLOR_TABLE      SID_COLOR_TABLE


#ifndef _SVX_SVXIDS_HRC //autogen
#include <svx/svxids.hrc>
#endif
#ifndef _MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif
#ifndef _SFXREQUEST_HXX //autogen
#include <sfx2/request.hxx>
#endif
#ifndef _OFF_APP_HXX //autogen
#include <offmgr/app.hxx>
#endif
#ifndef _SFXDISPATCH_HXX //autogen
#include <sfx2/dispatch.hxx>
#endif
#ifndef _SFX_BINDINGS_HXX //autogen
#include <sfx2/bindings.hxx>
#endif
#ifndef _SVDVIEW_HXX //autogen
#include <svx/svdview.hxx>
#endif
#ifndef _SVX_TEXTANIM_HXX //autogen
#include <svx/textanim.hxx>
#endif
#ifndef _SVX_TAB_AREA_HXX //autogen
#include <svx/tabarea.hxx>
#endif
#ifndef _SVX_TAB_LINE_HXX //autogen
#include <svx/tabline.hxx>
#endif
#ifndef _SVX_DRAWITEM_HXX //autogen
#include <svx/drawitem.hxx>
#endif


#include "view.hxx"
#include "wrtsh.hxx"
#include "docsh.hxx"
#include "cmdid.h"

#include "drawsh.hxx"


/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/


void SwDrawShell::ExecDrawDlg(SfxRequest& rReq)
{
    SwWrtShell*     pSh     = &GetShell();
    SdrView*        pView   = pSh->GetDrawView();
    SdrModel*       pDoc    = pView->GetModel();
    Window*         pWin    = GetView().GetWindow();
    BOOL            bChanged = pDoc->IsChanged();
    pDoc->SetChanged(FALSE);

    SfxItemSet aNewAttr( pDoc->GetItemPool() );
    pView->GetAttributes( aNewAttr );

    GetView().NoRotate();

    pSh->StartAction();

    switch (rReq.GetSlot())
    {
        case FN_DRAWTEXT_ATTR_DLG:
        {
            SvxTextTabDialog* pDlg = new SvxTextTabDialog( NULL, &aNewAttr , pView);
            USHORT nResult = pDlg->Execute();

            if (nResult == RET_OK)
            {
                if (pView->HasMarkedObj())
                    pView->SetAttributes(*pDlg->GetOutputItemSet());
            }
            delete( pDlg );
        }
        break;

        case SID_ATTRIBUTES_AREA:
        {
            BOOL bHasMarked = pView->HasMarkedObj();


#if SUPD>352
            SvxAreaTabDialog* pDlg = new SvxAreaTabDialog( NULL, &aNewAttr, pDoc, pView );
#else
            SvxAreaTabDialog* pDlg = new SvxAreaTabDialog( NULL, &aNewAttr, pDoc);
#endif
            const SvxColorTableItem* pColorItem = (const SvxColorTableItem*)
                                    GetView().GetDocShell()->GetItem(SID_COLOR_TABLE);
            if(pColorItem->GetColorTable() == OFF_APP()->GetStdColorTable())
                pDlg->DontDeleteColorTable();
            if (pDlg->Execute() == RET_OK)
            {
                if (bHasMarked)
                    pView->SetAttributes(*pDlg->GetOutputItemSet());
                else
                    pView->SetDefaultAttr(*pDlg->GetOutputItemSet(), FALSE);

                static USHORT __READONLY_DATA aInval[] =
                {
                    SID_ATTR_FILL_STYLE, SID_ATTR_FILL_COLOR, 0
                };
                SfxBindings &rBnd = GetView().GetViewFrame()->GetBindings();
                rBnd.Invalidate(aInval);
                rBnd.Update(SID_ATTR_FILL_STYLE);
                rBnd.Update(SID_ATTR_FILL_COLOR);
            }
            delete pDlg;
        }
        break;

        case SID_ATTRIBUTES_LINE:
        {
            BOOL bHasMarked = pView->HasMarkedObj();

            const SdrObject* pObj = NULL;
            const SdrMarkList& rMarkList = pView->GetMarkList();
            if( rMarkList.GetMarkCount() == 1 )
                pObj = rMarkList.GetMark(0)->GetObj();

            SvxLineTabDialog* pDlg = new SvxLineTabDialog(NULL, &aNewAttr,
                                                            pDoc, pObj, bHasMarked);

            if (pDlg->Execute() == RET_OK)
            {
                if(bHasMarked)
                    pView->SetAttrToMarked(*pDlg->GetOutputItemSet(), FALSE);
                else
                    pView->SetDefaultAttr(*pDlg->GetOutputItemSet(), FALSE);

                static USHORT __READONLY_DATA aInval[] =
                {
                    SID_ATTR_LINE_STYLE, SID_ATTR_LINE_WIDTH,
                    SID_ATTR_LINE_COLOR, 0
                };

                GetView().GetViewFrame()->GetBindings().Invalidate(aInval);
            }
            delete pDlg;
        }
        break;

        default:
            break;
    }

    pSh->EndAction();

    if (pDoc->IsChanged())
        GetShell().SetModified();
    else
        if (bChanged)
            pDoc->SetChanged(TRUE);
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/


void SwDrawShell::ExecDrawAttrArgs(SfxRequest& rReq)
{
    SwWrtShell* pSh   = &GetShell();
    SdrView*    pView = pSh->GetDrawView();
    const SfxItemSet* pArgs = rReq.GetArgs();
    BOOL        bChanged = pView->GetModel()->IsChanged();
    pView->GetModel()->SetChanged(FALSE);

    GetView().NoRotate();

    if (pArgs)
    {
        if(pView->HasMarkedObj())
            pView->SetAttrToMarked(*rReq.GetArgs(), FALSE);
        else
            pView->SetDefaultAttr(*rReq.GetArgs(), FALSE);
    }
    else
    {
        SfxDispatcher* pDis = pSh->GetView().GetViewFrame()->GetDispatcher();
        switch (rReq.GetSlot())
        {
            case SID_ATTR_FILL_STYLE:
            case SID_ATTR_FILL_COLOR:
            case SID_ATTR_FILL_GRADIENT:
            case SID_ATTR_FILL_HATCH:
            case SID_ATTR_FILL_BITMAP:
                pDis->Execute(SID_ATTRIBUTES_AREA, FALSE);
                break;
            case SID_ATTR_LINE_STYLE:
            case SID_ATTR_LINE_DASH:
            case SID_ATTR_LINE_WIDTH:
            case SID_ATTR_LINE_COLOR:
                pDis->Execute(SID_ATTRIBUTES_LINE, FALSE);
                break;
        }
    }
    if (pView->GetModel()->IsChanged())
        GetShell().SetModified();
    else
        if (bChanged)
            pView->GetModel()->SetChanged(TRUE);
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/


void SwDrawShell::GetDrawAttrState(SfxItemSet& rSet)
{
    SdrView* pSdrView = GetShell().GetDrawView();

    if (pSdrView->HasMarkedObj())
    {
        BOOL bDisable = Disable( rSet );

        if( !bDisable )
            pSdrView->GetAttributes( rSet );
    }
    else
        rSet.Put(pSdrView->GetDefaultAttr());
}

/*************************************************************************

      Source Code Control System - History

      $Log: not supported by cvs2svn $
      Revision 1.85  2000/09/18 16:06:03  willem.vandorp
      OpenOffice header added.

      Revision 1.84  2000/09/07 15:59:28  os
      change: SFX_DISPATCHER/SFX_BINDINGS removed

      Revision 1.83  2000/07/14 14:41:26  ama
      Fix #75805#: Start/EndAction prevents from performanceproblems

      Revision 1.82  2000/05/26 07:21:32  os
      old SW Basic API Slots removed

      Revision 1.81  2000/02/11 14:57:07  hr
      #70473# changes for unicode ( patched by automated patchtool )

      Revision 1.80  1999/06/21 13:20:24  JP
      Interface changes: SdrView::GetAttributes


      Rev 1.79   21 Jun 1999 15:20:24   JP
   Interface changes: SdrView::GetAttributes

      Rev 1.78   29 Nov 1997 15:54:02   MA
   includes

      Rev 1.77   24 Nov 1997 09:46:54   MA
   includes

      Rev 1.76   02 Sep 1997 13:23:16   OS
   includes

      Rev 1.75   01 Sep 1997 13:27:30   OS
   DLL-Umstellung

      Rev 1.74   26 Aug 1997 16:13:08   TRI
   VCL Anpassungen

      Rev 1.73   07 Apr 1997 17:42:54   MH
   chg: header

      Rev 1.72   23 Jan 1997 10:45:50   OM
   Neue Shells: Control- und DrawBaseShell

      Rev 1.71   22 Jan 1997 11:35:56   OM
   Neuer Parameter fuer SvxAreaTabDialog

      Rev 1.70   22 Jan 1997 11:20:50   OM
   Neue Shells: DrawBaseShell und DrawControlShell

      Rev 1.69   13 Jan 1997 15:56:10   OM
   Bezier-Punkte loeschen

      Rev 1.68   08 Jan 1997 12:48:56   OM
   Neue Segs

      Rev 1.67   08 Jan 1997 12:47:20   OM
   Aenderungen fuer Bezier-Shell

      Rev 1.66   20 Dec 1996 13:22:18   MA
   includes

      Rev 1.65   13 Dec 1996 12:19:10   OM
   Bezier-Controller Status und Execute

      Rev 1.64   29 Nov 1996 10:57:42   OM
   #33865# Fehlende Draw-Attribute gesetzt

      Rev 1.63   28 Nov 1996 16:32:32   OM
   Malattribute nur ohne selektiertes Objekt als Default setzen

      Rev 1.62   28 Nov 1996 13:07:44   OM
   #33823# Dok-Switch bei Draw-Dialogen unterbunden

      Rev 1.61   27 Nov 1996 16:15:08   OM
   #33427# Zeichenobjekte: automatischer Layerwechsel je nach Umlaufart

      Rev 1.60   27 Nov 1996 10:05:18   OM
   #33627# SetGeoAttr durch SetAttributes ersetzt

      Rev 1.59   19 Nov 1996 15:55:20   NF
   clooks...

      Rev 1.58   15 Nov 1996 13:34:00   OS
   SvxTextTabDialog angebunden

      Rev 1.57   08 Nov 1996 15:59:24   OM
   Schuetzen neu organisiert, transparenz im Draw-Mode

      Rev 1.56   29 Oct 1996 17:21:48   OM
   Abstaende auch im Drawmode

      Rev 1.55   23 Oct 1996 20:04:46   OM
   Drawobjekt Textumfluss-Dlg

      Rev 1.54   23 Oct 1996 13:04:46   OS
   DontDeleteColorTable am SvxAreaTabDialog ruen

      Rev 1.53   01 Oct 1996 16:54:00   OM
   Hyperlinks editieren

      Rev 1.52   28 Aug 1996 15:55:02   OS
   includes

      Rev 1.51   15 Aug 1996 13:44:20   JP
   svdraw.hxx entfernt

      Rev 1.50   31 May 1996 11:01:36   NF
   CLOOKS

      Rev 1.49   29 May 1996 13:05:26   OS
   headerstop wieder vor den Win31-defines

      Rev 1.48   29 May 1996 09:49:00   TRI
   CLOOKS

      Rev 1.47   21 May 1996 16:57:12   AMA
   Fix: Start+EndAction nach dem DrawDialog.

      Rev 1.46   21 May 1996 08:50:42   NF
   CLOOKS

      Rev 1.45   14 May 1996 15:17:06   HJS
   CLOOKS

      Rev 1.44   10 May 1996 15:57:18   OS
   Zeichenbindung an Drawobjekten

      Rev 1.43   30 Apr 1996 13:45:18   OS
   veraenderter TransformDlg ab 315

      Rev 1.42   07 Mar 1996 14:55:58   HJS
   2 defines zu viel

*************************************************************************/



