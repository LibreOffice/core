/*************************************************************************
 *
 *  $RCSfile: drawdlg.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: vg $ $Date: 2003-04-17 15:40:52 $
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

    switch (rReq.GetSlot())
    {
        case FN_DRAWTEXT_ATTR_DLG:
        {
            SvxTextTabDialog* pDlg = new SvxTextTabDialog( NULL, &aNewAttr , pView);
            USHORT nResult = pDlg->Execute();

            if (nResult == RET_OK)
            {
                if (pView->HasMarkedObj())
                {
                    pSh->StartAction();
                    pView->SetAttributes(*pDlg->GetOutputItemSet());
                    rReq.Done(*(pDlg->GetOutputItemSet()));
                    pSh->EndAction();
                }
            }
            delete( pDlg );
        }
        break;

        case SID_ATTRIBUTES_AREA:
        {
            BOOL bHasMarked = pView->HasMarkedObj();


            SvxAreaTabDialog* pDlg = new SvxAreaTabDialog( NULL, &aNewAttr, pDoc, pView );
            const SvxColorTableItem* pColorItem = (const SvxColorTableItem*)
                                    GetView().GetDocShell()->GetItem(SID_COLOR_TABLE);
            if(pColorItem->GetColorTable() == OFF_APP()->GetStdColorTable())
                pDlg->DontDeleteColorTable();
            if (pDlg->Execute() == RET_OK)
            {
                pSh->StartAction();
                if (bHasMarked)
                    pView->SetAttributes(*pDlg->GetOutputItemSet());
                else
                    pView->SetDefaultAttr(*pDlg->GetOutputItemSet(), FALSE);
                pSh->EndAction();

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
                pSh->StartAction();
                if(bHasMarked)
                    pView->SetAttrToMarked(*pDlg->GetOutputItemSet(), FALSE);
                else
                    pView->SetDefaultAttr(*pDlg->GetOutputItemSet(), FALSE);
                pSh->EndAction();

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



