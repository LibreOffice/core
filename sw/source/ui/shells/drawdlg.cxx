/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"





#ifndef _SVX_SVXIDS_HRC //autogen
#include <svx/svxids.hrc>
#endif
#ifndef _MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif
#include <sfx2/request.hxx>
#include <sfx2/dispatch.hxx>
#include <svx/svdview.hxx>
#include <svx/tabarea.hxx>
#include <svx/tabline.hxx>
#include <svx/drawitem.hxx>

#include <svx/xtable.hxx>
#include "view.hxx"
#include "wrtsh.hxx"
#include "docsh.hxx"
#include "cmdid.h"

#include "drawsh.hxx"
#include <svx/svxdlg.hxx>
#include <svx/dialogs.hrc>

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/


void SwDrawShell::ExecDrawDlg(SfxRequest& rReq)
{
    SwWrtShell*     pSh     = &GetShell();
    SdrView*        pView   = pSh->GetDrawView();
    SdrModel*       pDoc    = pView->GetModel();
    sal_Bool            bChanged = pDoc->IsChanged();
    pDoc->SetChanged(sal_False);

    SfxItemSet aNewAttr( pDoc->GetItemPool() );
    pView->GetAttributes( aNewAttr );

    GetView().NoRotate();

    switch (rReq.GetSlot())
    {
        case FN_DRAWTEXT_ATTR_DLG:
        {
            SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
            if ( pFact )
            {
                SfxAbstractTabDialog *pDlg = pFact->CreateTextTabDialog( NULL, &aNewAttr, pView );
                sal_uInt16 nResult = pDlg->Execute();

                if (nResult == RET_OK)
                {
                    if (pView->AreObjectsMarked())
                    {
                        pSh->StartAction();
                        pView->SetAttributes(*pDlg->GetOutputItemSet());
                        rReq.Done(*(pDlg->GetOutputItemSet()));
                        pSh->EndAction();
                    }
                }

                delete( pDlg );
            }
        }
        break;

        case SID_ATTRIBUTES_AREA:
        {
            sal_Bool bHasMarked = pView->AreObjectsMarked();

            SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
            DBG_ASSERT(pFact, "Dialogdiet Factory fail!");
            AbstractSvxAreaTabDialog * pDlg = pFact->CreateSvxAreaTabDialog( NULL,
                                                                            &aNewAttr,
                                                                            pDoc,
                                                                            pView);
            DBG_ASSERT(pDlg, "Dialogdiet fail!");
            const SvxColorTableItem* pColorItem = (const SvxColorTableItem*)
                                    GetView().GetDocShell()->GetItem(SID_COLOR_TABLE);
            if(pColorItem->GetColorTable() == XColorList::GetStdColorList())
                pDlg->DontDeleteColorTable();
            if (pDlg->Execute() == RET_OK)
            {
                pSh->StartAction();
                if (bHasMarked)
                    pView->SetAttributes(*pDlg->GetOutputItemSet());
                else
                    pView->SetDefaultAttr(*pDlg->GetOutputItemSet(), sal_False);
                pSh->EndAction();

                static sal_uInt16 __READONLY_DATA aInval[] =
                {
                    SID_ATTR_FILL_STYLE,
                    SID_ATTR_FILL_COLOR,
                    SID_ATTR_FILL_TRANSPARENCE,
                    SID_ATTR_FILL_FLOATTRANSPARENCE,
                    0
                };
                SfxBindings &rBnd = GetView().GetViewFrame()->GetBindings();
                rBnd.Invalidate(aInval);
                rBnd.Update(SID_ATTR_FILL_STYLE);
                rBnd.Update(SID_ATTR_FILL_COLOR);
                rBnd.Update(SID_ATTR_FILL_TRANSPARENCE);
                rBnd.Update(SID_ATTR_FILL_FLOATTRANSPARENCE);
            }
            delete pDlg;
        }
        break;

        case SID_ATTRIBUTES_LINE:
        {
            sal_Bool bHasMarked = pView->AreObjectsMarked();

            const SdrObject* pObj = NULL;
            const SdrMarkList& rMarkList = pView->GetMarkedObjectList();
            if( rMarkList.GetMarkCount() == 1 )
                pObj = rMarkList.GetMark(0)->GetMarkedSdrObj();

            SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
            DBG_ASSERT(pFact, "Dialogdiet Factory fail!");
            SfxAbstractTabDialog * pDlg = pFact->CreateSvxLineTabDialog( NULL,
                    &aNewAttr,
                pDoc,
                pObj,
                bHasMarked);
            DBG_ASSERT(pDlg, "Dialogdiet fail!");
            if (pDlg->Execute() == RET_OK)
            {
                pSh->StartAction();
                if(bHasMarked)
                    pView->SetAttrToMarked(*pDlg->GetOutputItemSet(), sal_False);
                else
                    pView->SetDefaultAttr(*pDlg->GetOutputItemSet(), sal_False);
                pSh->EndAction();

                static sal_uInt16 __READONLY_DATA aInval[] =
                {
                    SID_ATTR_LINE_STYLE,                // ( SID_SVX_START + 169 )
                    SID_ATTR_LINE_DASH,                 // ( SID_SVX_START + 170 )
                    SID_ATTR_LINE_WIDTH,                // ( SID_SVX_START + 171 )
                    SID_ATTR_LINE_COLOR,                // ( SID_SVX_START + 172 )
                    SID_ATTR_LINE_START,                // ( SID_SVX_START + 173 )
                    SID_ATTR_LINE_END,                  // ( SID_SVX_START + 174 )
                    SID_ATTR_LINE_TRANSPARENCE,         // (SID_SVX_START+1107)
                    SID_ATTR_LINE_JOINT,                // (SID_SVX_START+1110)
                    SID_ATTR_LINE_CAP,                  // (SID_SVX_START+1111)
                    0
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
            pDoc->SetChanged(sal_True);
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/


void SwDrawShell::ExecDrawAttrArgs(SfxRequest& rReq)
{
    SwWrtShell* pSh   = &GetShell();
    SdrView*    pView = pSh->GetDrawView();
    const SfxItemSet* pArgs = rReq.GetArgs();
    sal_Bool        bChanged = pView->GetModel()->IsChanged();
    pView->GetModel()->SetChanged(sal_False);

    GetView().NoRotate();

    if (pArgs)
    {
        if(pView->AreObjectsMarked())
            pView->SetAttrToMarked(*rReq.GetArgs(), sal_False);
        else
            pView->SetDefaultAttr(*rReq.GetArgs(), sal_False);
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
            case SID_ATTR_FILL_TRANSPARENCE:
            case SID_ATTR_FILL_FLOATTRANSPARENCE:
                pDis->Execute(SID_ATTRIBUTES_AREA, sal_False);
                break;
            case SID_ATTR_LINE_STYLE:
            case SID_ATTR_LINE_DASH:
            case SID_ATTR_LINE_WIDTH:
            case SID_ATTR_LINE_COLOR:
            case SID_ATTR_LINE_TRANSPARENCE:
            case SID_ATTR_LINE_JOINT:
            case SID_ATTR_LINE_CAP:
                pDis->Execute(SID_ATTRIBUTES_LINE, sal_False);
                break;
        }
    }
    if (pView->GetModel()->IsChanged())
        GetShell().SetModified();
    else
        if (bChanged)
            pView->GetModel()->SetChanged(sal_True);
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/


void SwDrawShell::GetDrawAttrState(SfxItemSet& rSet)
{
    SdrView* pSdrView = GetShell().GetDrawView();

    if (pSdrView->AreObjectsMarked())
    {
        sal_Bool bDisable = Disable( rSet );

        if( !bDisable )
            pSdrView->GetAttributes( rSet );
    }
    else
        rSet.Put(pSdrView->GetDefaultAttr());
}



