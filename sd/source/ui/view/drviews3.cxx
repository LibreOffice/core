/*************************************************************************
 *
 *  $RCSfile: drviews3.cxx,v $
 *
 *  $Revision: 1.26 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-12 15:16:41 $
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

#include "DrawViewShell.hxx"

#ifndef _EEITEMID_HXX
#include <svx/eeitemid.hxx>
#endif
#ifndef _SVX_TSPTITEM_HXX //autogen
#include <svx/tstpitem.hxx>
#endif
#ifndef _SVX_LRSPITEM_HXX //autogen
#include <svx/lrspitem.hxx>
#endif
#ifndef _SVX_PROTITEM_HXX //autogen
#include <svx/protitem.hxx>
#endif
#ifndef _SVX_FRMDIRITEM_HXX
#include <svx/frmdiritem.hxx>
#endif

#ifndef _SVX_RULER_HXX
#include <svx/ruler.hxx>
#endif
#ifndef _SVX_RULERITEM_HXX
#include <svx/rulritem.hxx>
#endif
#ifndef _SVX_ZOOMITEM_HXX
#include <svx/zoomitem.hxx>
#endif
#ifndef _SVXIDS_HRC
#include <svx/svxids.hrc>
#endif

#ifndef _SVDPAGV_HXX //autogen
#include <svx/svdpagv.hxx>
#endif

#ifndef _SFXREQUEST_HXX //autogen
#include <sfx2/request.hxx>
#endif
#ifndef _SFXDISPATCH_HXX //autogen
#include <sfx2/dispatch.hxx>
#endif
#ifndef _URLOBJ_HXX
#include <tools/urlobj.hxx>
#endif
#ifndef _AEITEM_HXX //autogen
#include <svtools/aeitem.hxx>
#endif
#ifndef _SFXENUMITEM_HXX //autogen
#include <svtools/eitem.hxx>
#endif
#ifndef _SFXRECTITEM_HXX //autogen
#include <svtools/rectitem.hxx>
#endif
#ifndef _SFXSTRITEM_HXX //autogen
#include <svtools/stritem.hxx>
#endif
#ifndef _SVDOOLE2_HXX //autogen
#include <svx/svdoole2.hxx>
#endif
#ifndef _SFXITEMPOOL_HXX //autogen
#include <svtools/itempool.hxx>
#endif
#ifndef _SFXPTITEM_HXX //autogen
#include <svtools/ptitem.hxx>
#endif
#ifndef _SB_SBSTAR_HXX //autogen
#include <basic/sbstar.hxx>
#endif
#ifndef _SB_SBERRORS_HXX //autogen
#include <basic/sberrors.hxx>
#endif
#ifndef _SVX_FMSHELL_HXX //autogen
#include <svx/fmshell.hxx>
#endif

#ifndef _SVX_F3DCHILD_HXX
#include <svx/f3dchild.hxx>
#endif
#ifndef _SVX_FLOAT3D_HXX
#include <svx/float3d.hxx>
#endif

#ifndef _SD_OPTSITEM_HXX
#include "optsitem.hxx"
#endif

#include "app.hrc"
#include "glob.hrc"
#include "strings.hrc"
#include "res_bmp.hrc"

#include "sdundogr.hxx"
#include "undopage.hxx"
#include "glob.hxx"
#include "app.hxx"
#ifndef SD_FU_POOR_HXX
#include "fupoor.hxx"
#endif
#ifndef SD_FU_SLIDE_SHOW_HXX
#include "fuslshow.hxx"
#endif
#ifndef SD_FRAME_VIEW
#include "FrameView.hxx"
#endif
#include "sdpage.hxx"
#ifndef SD_WINDOW_HXX
#include "Window.hxx"
#endif
#include "sdresid.hxx"
#ifndef SD_DRAW_VIEW_HXX
#include "drawview.hxx"
#endif
#include "drawdoc.hxx"
#include "DrawViewShell.hxx"
#include "Ruler.hxx"
#include "DrawDocShell.hxx"
#include "PreviewWindow.hxx"
#include "headerfooterdlg.hxx"
#include "masterlayoutdlg.hxx"
#include "Ruler.hxx"
#include "DrawDocShell.hxx"
#ifndef SD_PREVIEW_WINDOW_HXX
#include "PreviewWindow.hxx"
#endif
#ifndef SD_OBJECT_BAR_MANAGER_HXX
#include "ObjectBarManager.hxx"
#endif
#include "sdabstdlg.hxx" //CHINA001

namespace sd {

#ifndef SO2_DECL_SVINPLACEOBJECT_DEFINED
#define SO2_DECL_SVINPLACEOBJECT_DEFINED
SO2_DECL_REF(SvInPlaceObject)
#endif



/*************************************************************************
|*
|* SfxRequests fuer Controller bearbeiten
|*
\************************************************************************/

void  DrawViewShell::ExecCtrl(SfxRequest& rReq)
{
    // waehrend einer Diashow wird nichts ausser dem Seitenwechsel und dem
    // Sprung zur Bookmark ausgefuehrt!
    if (pFuActual &&
        pFuActual->GetSlotID() == SID_PRESENTATION &&
        rReq.GetSlot() != SID_SWITCHPAGE &&
        rReq.GetSlot() != SID_JUMPTOMARK)
        return;

    CheckLineTo (rReq);

    // End text edit mode for some requests.
    USHORT nSlot = rReq.GetSlot();
    switch (nSlot)
    {
        case SID_OUTPUT_QUALITY_COLOR:
        case SID_OUTPUT_QUALITY_GRAYSCALE:
        case SID_OUTPUT_QUALITY_BLACKWHITE:
        case SID_OUTPUT_QUALITY_CONTRAST:
        case SID_PREVIEW_QUALITY_COLOR:
        case SID_PREVIEW_QUALITY_GRAYSCALE:
        case SID_PREVIEW_QUALITY_BLACKWHITE:
        case SID_PREVIEW_QUALITY_CONTRAST:
            // Do nothing.
            break;
        default:
            if ( pDrView->IsTextEdit() )
            {
                pDrView->EndTextEdit();
            }
    }

    //  USHORT nSlot = rReq.GetSlot();
    switch (nSlot)
    {
        case SID_SWITCHPAGE:  // BASIC
        {
            BOOL bWasBasic = FALSE;

            // Seitenwechsel in der Diashow abspielen
            if (pFuSlideShow && rReq.GetArgs())
            {
                SFX_REQUEST_ARG(rReq, pWhatPage, SfxUInt32Item, ID_VAL_WHATPAGE, FALSE);
                pFuSlideShow->JumpToPage((USHORT)pWhatPage->GetValue());
            }
            else
            {
                const SfxItemSet *pArgs = rReq.GetArgs ();
                USHORT nSelectedPage;

                if (! pArgs)
                {
                    nSelectedPage = aTabControl.GetCurPageId() - 1;
                }
                else if (pArgs->Count () == 2)
                {
                    SFX_REQUEST_ARG (rReq, pWhatPage, SfxUInt32Item, ID_VAL_WHATPAGE, FALSE);
                    SFX_REQUEST_ARG (rReq, pWhatKind, SfxUInt32Item, ID_VAL_WHATKIND, FALSE);

                    if (! CHECK_RANGE (PK_STANDARD, pWhatKind->GetValue (), PK_HANDOUT))
                    {
                        StarBASIC::FatalError (SbERR_BAD_PROP_VALUE);
                        rReq.Ignore ();
                        break;
                    }
                    else if (eEditMode != EM_MASTERPAGE)
                    {
                        if (! CHECK_RANGE (0, pWhatPage->GetValue (),
                            GetDoc()->GetSdPageCount ((PageKind) pWhatKind->GetValue ())))
                        {
                            StarBASIC::FatalError (SbERR_BAD_PROP_VALUE);
                            rReq.Ignore ();
                            break;
                        }

                        nSelectedPage = (short) pWhatPage->GetValue ();
                        ePageKind     = (PageKind) pWhatKind->GetValue ();
                        bWasBasic     = TRUE;
                    }
                }
                else
                {
                    StarBASIC::FatalError (SbERR_WRONG_ARGS);
                    rReq.Ignore ();
                    break;
                }


                if( GetDocSh() && (GetDocSh()->GetCreateMode() == SFX_CREATE_MODE_EMBEDDED))
                    GetDocSh()->SetModified();

                SwitchPage(nSelectedPage);

                if (pFuActual && pFuActual->GetSlotID() == SID_BEZIER_EDIT)
                    GetViewFrame()->GetDispatcher()->Execute(SID_OBJECT_SELECT, SFX_CALLMODE_ASYNCHRON);

                Invalidate();
                InvalidateWindows();
                rReq.Done ();
            }
            break;
        }

        case SID_SWITCHLAYER:  // BASIC
        {
            const SfxItemSet *pArgs = rReq.GetArgs ();
            USHORT nCurPage = aLayerTab.GetCurPageId ();

            if( pArgs && pArgs->Count () == 1)
            {
                SFX_REQUEST_ARG (rReq, pWhatLayer, SfxUInt32Item, ID_VAL_WHATLAYER, FALSE);
                if( pWhatLayer )
                    nCurPage = (short) pWhatLayer->GetValue ();
            }

            pDrView->SetActiveLayer( aLayerTab.GetPageText(nCurPage) );
            Invalidate();
            rReq.Done ();

            break;
        }

        case SID_PAGEMODE:  // BASIC
        {

            const SfxItemSet *pArgs = rReq.GetArgs ();

            if ( pArgs && pArgs->Count () == 2)
            {
                SFX_REQUEST_ARG (rReq, pIsActive, SfxBoolItem, ID_VAL_ISACTIVE, FALSE);
                SFX_REQUEST_ARG (rReq, pWhatKind, SfxUInt32Item, ID_VAL_WHATKIND, FALSE);

                if (CHECK_RANGE (PK_STANDARD, pWhatKind->GetValue (), PK_HANDOUT))
                {
                    bLayerMode = pIsActive->GetValue ();
                    ePageKind = (PageKind) pWhatKind->GetValue ();
                }
            }

            // Default-Layer der Page einschalten
            pDrView->SetActiveLayer( String( SdResId(STR_LAYER_LAYOUT) ) );

            ChangeEditMode(EM_PAGE, bLayerMode);

            Invalidate();
            rReq.Done ();

            break;
        }

        case SID_LAYERMODE:  // BASIC
        {
            const SfxItemSet *pArgs = rReq.GetArgs ();

            if ( pArgs && pArgs->Count () == 2)
            {
                SFX_REQUEST_ARG (rReq, pWhatLayerMode, SfxBoolItem, ID_VAL_ISACTIVE, FALSE);
                SFX_REQUEST_ARG (rReq, pWhatLayer, SfxUInt32Item, ID_VAL_WHATLAYER, FALSE);

                if (CHECK_RANGE (EM_PAGE, pWhatLayer->GetValue (), EM_MASTERPAGE))
                {
                    bLayerMode = pWhatLayerMode->GetValue ();
                    eEditMode = (EditMode) pWhatLayer->GetValue ();
                }
            }

            ChangeEditMode(eEditMode, !bLayerMode);

            Invalidate();
            rReq.Done ();

            break;
        }

        case SID_HEADER_AND_FOOTER:
        case SID_INSERT_PAGE_NUMBER:
        case SID_INSERT_DATE_TIME:
        {
            SdAbstractDialogFactory* pFact = SdAbstractDialogFactory::Create();//CHINA001
            DBG_ASSERT(pFact, "SdAbstractDialogFactory fail!");//CHINA001
            AbstractHeaderFooterDialog* pDlg = pFact->CreateHeaderFooterDialog( (::ViewShell*)this, pWindow, GetDoc(), pActualPage );
            DBG_ASSERT(pDlg, "Dialogdiet fail!");//CHINA001
            pDlg->Execute();
            delete pDlg;

            pWindow->Invalidate();
            UpdatePreview( pActualPage );

            Invalidate();
            rReq.Done ();

            break;
        }

        case SID_MASTER_LAYOUTS_SLIDE:
        {
            SdPage* pPage = pActualPage;
            if( pPage->GetPageKind() == PK_NOTES )
            {
                pPage = (SdPage*)GetDoc()->GetPage( pPage->GetPageNum() - 1 );
            }
            else if( pPage->GetPageKind() == PK_HANDOUT )
            {
                pPage = GetDoc()->GetMasterSdPage(0,PK_STANDARD);
            }

            SdAbstractDialogFactory* pFact = SdAbstractDialogFactory::Create();//CHINA001
            DBG_ASSERT(pFact, "SdAbstractDialogFactory fail!");//CHINA001
            VclAbstractDialog* pDlg = pFact->CreateMasterLayoutDialog( pWindow, GetDoc(), pPage );
            DBG_ASSERT(pDlg, "Dialogdiet fail!");//CHINA001
            pDlg->Execute();
            delete pDlg;
            Invalidate();
            rReq.Done ();
            break;
        }

        case SID_MASTER_LAYOUTS_NOTES:
        {
            SdPage* pPage = pActualPage;
            if( pPage->GetPageKind() == PK_STANDARD )
            {
                pPage = (SdPage*)GetDoc()->GetPage( pPage->GetPageNum() + 1 );
            }
            else if( pPage->GetPageKind() == PK_HANDOUT )
            {
                pPage = GetDoc()->GetMasterSdPage(0,PK_NOTES);
            }

            SdAbstractDialogFactory* pFact = SdAbstractDialogFactory::Create();//CHINA001
            DBG_ASSERT(pFact, "SdAbstractDialogFactory fail!");//CHINA001
            VclAbstractDialog* pDlg = pFact->CreateMasterLayoutDialog( pWindow, GetDoc(), pPage );
            DBG_ASSERT(pDlg, "Dialogdiet fail!");//CHINA001
            pDlg->Execute();
            delete pDlg;

            Invalidate();
            rReq.Done ();
            break;
        }

        case SID_MASTER_LAYOUTS_HANDOUTS:
        {
            SdAbstractDialogFactory* pFact = SdAbstractDialogFactory::Create();//CHINA001
            DBG_ASSERT(pFact, "SdAbstractDialogFactory fail!");//CHINA001
            VclAbstractDialog* pDlg = pFact->CreateMasterLayoutDialog( pWindow, GetDoc(), GetDoc()->GetMasterSdPage(0,PK_HANDOUT) );
            DBG_ASSERT(pDlg, "Dialogdiet fail!");//CHINA001
            pDlg->Execute();
            delete pDlg;

            Invalidate();
            rReq.Done ();
            break;
        }

        case SID_OBJECTRESIZE:
        {
            /******************************************************************
            * Der Server moechte die Clientgrosse verandern
            ******************************************************************/
            OSL_ASSERT (GetViewShell()!=NULL);
            SfxInPlaceClient* pIPClient = GetViewShell()->GetIPClient();

            if ( pIPClient && pIPClient->IsInPlaceActive() )
            {
                const SfxRectangleItem& rRect =
                    (SfxRectangleItem&)rReq.GetArgs()->Get(SID_OBJECTRESIZE);
                Rectangle aRect( pWindow->PixelToLogic( rRect.GetValue() ) );

                if ( pDrView->AreObjectsMarked() )
                {
                    const SdrMarkList& rMarkList = pDrView->GetMarkedObjectList();

                    if (rMarkList.GetMarkCount() == 1)
                    {
                        SdrMark* pMark = rMarkList.GetMark(0);
                        SdrObject* pObj = pMark->GetObj();

                        UINT16 nSdrObjKind = pObj->GetObjIdentifier();

                        if (pObj->GetObjInventor() == SdrInventor &&
                            pObj->GetObjIdentifier() == OBJ_OLE2)
                        {
                            SvInPlaceObjectRef aIPObj =
                            ( (SdrOle2Obj*) pObj)->GetObjRef();

                            if ( aIPObj.Is() )
                            {
                                pObj->SetLogicRect(aRect);
                            }
                        }
                    }
                }
            }
            rReq.Ignore ();
            break;
        }

        case SID_RELOAD:
        {
            // #83951#
            USHORT nId = Svx3DChildWindow::GetChildWindowId();
            SfxChildWindow* pWindow = GetViewFrame()->GetChildWindow(nId);
            if(pWindow)
            {
                Svx3DWin* p3DWin = (Svx3DWin*)(pWindow->GetWindow());
                if(p3DWin)
                    p3DWin->DocumentReload();
            }

            // DER SLOT KANN IN DEN SFX VERSCHOBEN WERDEN
            SFX_REQUEST_ARG(rReq, pForceReloadItem, SfxBoolItem, SID_NOCACHE, FALSE);
            // Normale Weiterleitung an ViewFrame zur Ausfuehrung
            GetViewFrame()->ExecuteSlot(rReq);

            // Muss sofort beendet werden
            return;
        }
        break;

        case SID_JUMPTOMARK:
        {
            if( rReq.GetArgs() )
            {
                SFX_REQUEST_ARG(rReq, pBookmark, SfxStringItem, SID_JUMPTOMARK, FALSE);

                UniString sBookmark( INetURLObject::decode( pBookmark->GetValue(), '%', INetURLObject::DECODE_WITH_CHARSET ) );
                if( sBookmark.Search( sal_Unicode('#') ) == 0 )
                    sBookmark = sBookmark.Copy( 1 );

                if (pBookmark)
                {
                    if (pFuSlideShow)
                    {
                        pFuSlideShow->JumpToBookmark(sBookmark);
                    }
                    else
                    {
                        GotoBookmark( sBookmark );
                    }
                }
            }
            rReq.Done();
            break;
        }

        case SID_OUTPUT_QUALITY_COLOR:
        case SID_OUTPUT_QUALITY_GRAYSCALE:
        case SID_OUTPUT_QUALITY_BLACKWHITE:
        case SID_OUTPUT_QUALITY_CONTRAST:

        case SID_PREVIEW_QUALITY_COLOR:
        case SID_PREVIEW_QUALITY_GRAYSCALE:
        case SID_PREVIEW_QUALITY_BLACKWHITE:
        case SID_PREVIEW_QUALITY_CONTRAST:
        {
            ExecReq( rReq );
            break;
        }

        case SID_MAIL_SCROLLBODY_PAGEDOWN:
        {
            ExecReq( rReq );
            break;
        }
        break;

        case SID_ATTR_YEAR2000:
        {
            FmFormShell* pFormShell = GetObjectBarManager().GetFormShell();
            if (pFormShell != NULL)
            {
                const SfxPoolItem* pItem;
                if (rReq.GetArgs()->GetItemState(
                    SID_ATTR_YEAR2000, TRUE, &pItem) == SFX_ITEM_SET)
                    pFormShell->SetY2KState (
                        static_cast<const SfxUInt16Item*>(pItem)->GetValue());
            }

            rReq.Done();
        }
        break;

        case SID_OPT_LOCALE_CHANGED:
        {
            pWindow->Invalidate();
            UpdatePreview( pActualPage );
            rReq.Done();
        }

        default:
        break;
    }
}

/*************************************************************************
|*
|* SfxRequests fuer Lineale bearbeiten
|*
\************************************************************************/

void  DrawViewShell::ExecRuler(SfxRequest& rReq)
{
    // waehrend einer Diashow wird nichts ausgefuehrt!
    if (pFuActual &&
        pFuActual->GetSlotID() == SID_PRESENTATION)
        return;

    CheckLineTo (rReq);

    const SfxItemSet* pArgs = rReq.GetArgs();
    Point aPagePos = pWindow->GetViewOrigin();
    Size aPageSize = pActualPage->GetSize();
    Size aViewSize = pWindow->GetViewSize();
    SdUndoGroup* pUndoGroup = NULL;

    if ( rReq.GetSlot() == SID_ATTR_LONG_LRSPACE ||
         rReq.GetSlot() == SID_ATTR_LONG_ULSPACE )
    {
        pUndoGroup = new SdUndoGroup(GetDoc());
        String aString(SdResId(STR_UNDO_CHANGE_PAGEBORDER));
        pUndoGroup->SetComment(aString);
    }

    switch ( rReq.GetSlot() )
    {
        case SID_ATTR_LONG_LRSPACE:
        {
            const SvxLongLRSpaceItem& rLRSpace = (const SvxLongLRSpaceItem&)
                    pArgs->Get(GetPool().GetWhich(SID_ATTR_LONG_LRSPACE));

            if( pDrView->IsTextEdit() )
            {
                Point aPagePos = pWindow->GetViewOrigin();
                Rectangle aRect = aMarkRect;
                aRect.SetPos(aRect.TopLeft() + aPagePos);
                aRect.Left()  = rLRSpace.GetLeft();
                aRect.Right() = aViewSize.Width() - rLRSpace.GetRight();
                aRect.SetPos(aRect.TopLeft() - aPagePos);
                if ( aRect != aMarkRect)
                {
                    pDrView->SetAllMarkedRect(aRect);
                    aMarkRect = pDrView->GetAllMarkedRect();
                    Invalidate( SID_RULER_OBJECT );
                }
            }
            else
            {
                long nLeft = Max(0L, rLRSpace.GetLeft() - aPagePos.X());
                long nRight = Max(0L, rLRSpace.GetRight() + aPagePos.X() +
                                      aPageSize.Width() - aViewSize.Width());

                USHORT nPageCnt = GetDoc()->GetSdPageCount(ePageKind);
                USHORT i;
                for ( i = 0; i < nPageCnt; i++)
                {
                    SdPage* pPage = GetDoc()->GetSdPage(i, ePageKind);
                    SdUndoAction* pUndo = new SdPageLRUndoAction(GetDoc(),
                                            pPage,
                                            pPage->GetLftBorder(),
                                            pPage->GetRgtBorder(),
                                            nLeft, nRight);
                    pUndoGroup->AddAction(pUndo);
                    pPage->SetLftBorder(nLeft);
                    pPage->SetRgtBorder(nRight);
                }
                nPageCnt = GetDoc()->GetMasterSdPageCount(ePageKind);

                for (i = 0; i < nPageCnt; i++)
                {
                    SdPage* pPage = GetDoc()->GetMasterSdPage(i, ePageKind);
                    SdUndoAction* pUndo = new SdPageLRUndoAction(GetDoc(),
                                            pPage,
                                            pPage->GetLftBorder(),
                                            pPage->GetRgtBorder(),
                                            nLeft, nRight);
                    pUndoGroup->AddAction(pUndo);
                    pPage->SetLftBorder(nLeft);
                    pPage->SetRgtBorder(nRight);
                }
                InvalidateWindows();
            }
            break;
        }
        case SID_ATTR_LONG_ULSPACE:
        {
            const SvxLongULSpaceItem& rULSpace = (const SvxLongULSpaceItem&)
                    pArgs->Get(GetPool().GetWhich(SID_ATTR_LONG_ULSPACE));

            if( pDrView->IsTextEdit() )
            {
                Point aPagePos = pWindow->GetViewOrigin();
                Rectangle aRect = aMarkRect;
                aRect.SetPos(aRect.TopLeft() + aPagePos);
                aRect.Top()  = rULSpace.GetUpper();
                aRect.Bottom() = aViewSize.Height() - rULSpace.GetLower();
                aRect.SetPos(aRect.TopLeft() - aPagePos);

                if ( aRect != aMarkRect)
                {
                    pDrView->SetAllMarkedRect(aRect);
                    aMarkRect = pDrView->GetAllMarkedRect();
                    Invalidate( SID_RULER_OBJECT );
                }
            }
            else
            {
                long nUpper = Max(0L, rULSpace.GetUpper() - aPagePos.Y());
                long nLower = Max(0L, rULSpace.GetLower() + aPagePos.Y() +
                                      aPageSize.Height() - aViewSize.Height());

                USHORT nPageCnt = GetDoc()->GetSdPageCount(ePageKind);
                USHORT i;
                for ( i = 0; i < nPageCnt; i++)
                {
                    SdPage* pPage = GetDoc()->GetSdPage(i, ePageKind);
                    SdUndoAction* pUndo = new SdPageULUndoAction(GetDoc(),
                                            pPage,
                                            pPage->GetUppBorder(),
                                            pPage->GetLwrBorder(),
                                            nUpper, nLower);
                    pUndoGroup->AddAction(pUndo);
                    pPage->SetUppBorder(nUpper);
                    pPage->SetLwrBorder(nLower);
                }
                nPageCnt = GetDoc()->GetMasterSdPageCount(ePageKind);

                for (i = 0; i < nPageCnt; i++)
                {
                    SdPage* pPage = GetDoc()->GetMasterSdPage(i, ePageKind);
                    SdUndoAction* pUndo = new SdPageULUndoAction(GetDoc(),
                                            pPage,
                                            pPage->GetUppBorder(),
                                            pPage->GetLwrBorder(),
                                            nUpper, nLower);
                    pUndoGroup->AddAction(pUndo);
                    pPage->SetUppBorder(nUpper);
                    pPage->SetLwrBorder(nLower);
                }
                InvalidateWindows();
            }
            break;
        }

        case SID_RULER_OBJECT:
        {
            Point aPagePos = pWindow->GetViewOrigin();
            Rectangle aRect = aMarkRect;
            aRect.SetPos(aRect.TopLeft() + aPagePos);

            const SvxObjectItem& rOI = (const SvxObjectItem&)
                    pArgs->Get(GetPool().GetWhich(SID_RULER_OBJECT));

            if ( rOI.GetStartX() != rOI.GetEndX() )
            {
                aRect.Left()  = rOI.GetStartX();
                aRect.Right() = rOI.GetEndX();
            }
            if ( rOI.GetStartY() != rOI.GetEndY() )
            {
                aRect.Top()    = rOI.GetStartY();
                aRect.Bottom() = rOI.GetEndY();
            }
            aRect.SetPos(aRect.TopLeft() - aPagePos);
            if ( aRect != aMarkRect)
            {
                pDrView->SetAllMarkedRect(aRect);
                aMarkRect = pDrView->GetAllMarkedRect();
                Invalidate( SID_RULER_OBJECT );
            }
            break;
        }

        case SID_ATTR_TABSTOP:
        {
            if( pDrView->IsTextEdit() )
            {
                const SvxTabStopItem& rItem = (const SvxTabStopItem&)
                            pArgs->Get( ITEMID_TABSTOP );

                SfxItemSet aEditAttr( GetPool(), ITEMID_TABSTOP, ITEMID_TABSTOP );

                aEditAttr.Put( rItem );
                pDrView->SetAttributes( aEditAttr );

                // #91081# Invalidate is missing here
                Invalidate(SID_ATTR_TABSTOP);
            }
            break;
        }

        case SID_ATTR_PARA_LRSPACE:
        {
            if( pDrView->IsTextEdit() )
            {
                USHORT nId = SID_ATTR_PARA_LRSPACE;
                const SvxLRSpaceItem& rItem = (const SvxLRSpaceItem&)
                            pArgs->Get( nId );

                SfxItemSet aEditAttr( GetPool(), ITEMID_LRSPACE, ITEMID_LRSPACE );

                nId = ITEMID_LRSPACE;
                SvxLRSpaceItem aLRSpaceItem( rItem.GetLeft(),
                        rItem.GetRight(), rItem.GetTxtLeft(),
                        rItem.GetTxtFirstLineOfst(), nId );
                aEditAttr.Put( aLRSpaceItem );
                pDrView->SetAttributes( aEditAttr );

                // #92557# Invalidate is missing here
                Invalidate(SID_ATTR_PARA_LRSPACE);
            }
            break;
        }
    }
    if ( pUndoGroup )
        // Undo Gruppe dem Undo Manager uebergeben
        GetViewFrame()->GetObjectShell()->GetUndoManager()->
                                            AddUndoAction(pUndoGroup);
}

/*************************************************************************
|*
|* Statuswerte der Lineale bestimmen
|*
\************************************************************************/
void  DrawViewShell::GetRulerState(SfxItemSet& rSet)
{
    Point aOrigin;

    if (pDrView->GetPageViewPvNum(0))
    {
        aOrigin = pDrView->GetPageViewPvNum(0)->GetPageOrigin();
    }

    Size aViewSize = pWindow->GetViewSize();

    Point aPagePos = pWindow->GetViewOrigin();
    Size aPageSize = pActualPage->GetSize();

    Rectangle aRect(aPagePos, Point( aViewSize.Width() - (aPagePos.X() + aPageSize.Width()),
                                     aViewSize.Height() - (aPagePos.Y() + aPageSize.Height())));

    if( pDrView->IsTextEdit() )
    {
        Point aPnt1 = pWindow->GetWinViewPos();
        Point aPnt2 = pWindow->GetViewOrigin();
        Rectangle aMinMaxRect = Rectangle( aPnt1, Size(ULONG_MAX, ULONG_MAX) );
        rSet.Put( SfxRectangleItem(SID_RULER_LR_MIN_MAX, aMinMaxRect) );
    }
    else
    {
        rSet.Put( SfxRectangleItem(SID_RULER_LR_MIN_MAX, aRect) );
    }

    SvxLongLRSpaceItem aLRSpace(aPagePos.X() + pActualPage->GetLftBorder(),
                                aRect.Right() + pActualPage->GetRgtBorder(),
                                GetPool().GetWhich(SID_ATTR_LONG_LRSPACE));
    SvxLongULSpaceItem aULSpace(aPagePos.Y() + pActualPage->GetUppBorder(),
                                aRect.Bottom() + pActualPage->GetLwrBorder(),
                                GetPool().GetWhich(SID_ATTR_LONG_ULSPACE));
    rSet.Put(SvxPagePosSizeItem(Point(0,0) - aPagePos, aViewSize.Width(),
                                                       aViewSize.Height()));
    SfxPointItem aPointItem( SID_RULER_NULL_OFFSET, aPagePos + aOrigin );

    SvxProtectItem aProtect( SID_RULER_PROTECT );

    aMarkRect = pDrView->GetAllMarkedRect();

    const sal_Bool bRTL = GetDoc() && GetDoc()->GetDefaultWritingMode() == ::com::sun::star::text::WritingMode_RL_TB;
    rSet.Put(SfxBoolItem(SID_RULER_TEXT_RIGHT_TO_LEFT, bRTL));

    if( pDrView->AreObjectsMarked() )
    {
        if( pDrView->IsTextEdit() )
        {
            SdrObject* pObj = pDrView->GetMarkedObjectList().GetMark( 0 )->GetObj();
            if( pObj->GetObjInventor() == SdrInventor)
            {
                SfxItemSet aEditAttr( GetDoc()->GetPool() );
                pDrView->GetAttributes( aEditAttr );
                if( aEditAttr.GetItemState( ITEMID_TABSTOP ) >= SFX_ITEM_AVAILABLE )
                {
                    const SvxTabStopItem& rItem = (const SvxTabStopItem&) aEditAttr.Get( ITEMID_TABSTOP );
                    rSet.Put( rItem );

                    //Rectangle aRect = aMarkRect;

                    const SvxLRSpaceItem& rLRSpaceItem = (const SvxLRSpaceItem&) aEditAttr.Get( ITEMID_LRSPACE );
                    USHORT nId = SID_ATTR_PARA_LRSPACE;
                    SvxLRSpaceItem aLRSpaceItem( rLRSpaceItem.GetLeft(),
                            rLRSpaceItem.GetRight(), rLRSpaceItem.GetTxtLeft(),
                            rLRSpaceItem.GetTxtFirstLineOfst(), nId );
                    rSet.Put( aLRSpaceItem );

                    Point aPos( aPagePos + aMarkRect.TopLeft() );

                    if ( aEditAttr.GetItemState( SDRATTR_TEXT_LEFTDIST ) == SFX_ITEM_ON )
                    {
                        const SdrTextLeftDistItem& rTLDItem = (const SdrTextLeftDistItem&)
                                                              aEditAttr.Get( SDRATTR_TEXT_LEFTDIST );
                        long nLD = rTLDItem.GetValue();
                        aPos.X() += nLD;
                    }

                    aPointItem.SetValue( aPos );

                    aLRSpace.SetLeft( aPagePos.X() + aMarkRect.Left() );

                    if ( aEditAttr.GetItemState( SDRATTR_TEXT_LEFTDIST ) == SFX_ITEM_ON )
                    {
                        const SdrTextLeftDistItem& rTLDItem = (const SdrTextLeftDistItem&)
                                                              aEditAttr.Get( SDRATTR_TEXT_LEFTDIST );
                        long nLD = rTLDItem.GetValue();
                        aLRSpace.SetLeft( aLRSpace.GetLeft() + nLD );
                    }

                    aLRSpace.SetRight( aRect.Right() + aPageSize.Width() - aMarkRect.Right() );
                    aULSpace.SetUpper( aPagePos.Y() + aMarkRect.Top() );
                    aULSpace.SetLower( aRect.Bottom() + aPageSize.Height() - aMarkRect.Bottom() );

                    rSet.DisableItem( SID_RULER_OBJECT );

                    // Seitenraender werden gelocked
                    aProtect.SetSizeProtect( TRUE );
                    aProtect.SetPosProtect( TRUE );
                }

                if( aEditAttr.GetItemState( EE_PARA_WRITINGDIR ) >= SFX_ITEM_AVAILABLE )
                {
                    const SvxFrameDirectionItem& rItem = (const SvxFrameDirectionItem&) aEditAttr.Get( EE_PARA_WRITINGDIR );

                    const sal_Bool bRTL = rItem.GetValue() == ::com::sun::star::text::WritingMode_RL_TB;
                    rSet.Put(SfxBoolItem(SID_RULER_TEXT_RIGHT_TO_LEFT, bRTL));
                }
            }
        }
        else
        {
            rSet.DisableItem( ITEMID_TABSTOP );
            rSet.DisableItem( SID_RULER_TEXT_RIGHT_TO_LEFT );

            if( pDrView->IsResizeAllowed(TRUE) )
            {
                Rectangle aRect = aMarkRect;

                aRect.SetPos(aRect.TopLeft() + aPagePos);
                SvxObjectItem aObjItem(aRect.Left(), aRect.Right(),
                                       aRect.Top(), aRect.Bottom());
                rSet.Put(aObjItem);
                rSet.DisableItem( ITEMID_TABSTOP );
            }
            else
            {
                rSet.DisableItem( SID_RULER_OBJECT );
            }
        }
    }
    else
    {
        rSet.DisableItem( SID_RULER_OBJECT );
        rSet.DisableItem( ITEMID_TABSTOP );
//      rSet.DisableItem( SID_RULER_TEXT_RIGHT_TO_LEFT );
    }

    rSet.Put( aLRSpace );
    rSet.Put( aULSpace );

    rSet.Put( aPointItem );
    rSet.Put( aProtect );
}

/*************************************************************************
|*
|* SfxRequests fuer StatusBar bearbeiten
|*
\************************************************************************/

void  DrawViewShell::ExecStatusBar(SfxRequest& rReq)
{
    // waehrend einer Diashow wird nichts ausgefuehrt!
    if (pFuActual &&
        pFuActual->GetSlotID() == SID_PRESENTATION)
        return;

    CheckLineTo (rReq);

    const SfxItemSet* pArgs = rReq.GetArgs();

    switch ( rReq.GetSlot() )
    {
        case SID_ATTR_SIZE:
        {
            GetViewFrame()->GetDispatcher()->Execute( SID_ATTR_TRANSFORM, SFX_CALLMODE_ASYNCHRON );
        }
        break;

        case SID_STATUS_LAYOUT:
        {
            GetViewFrame()->GetDispatcher()->Execute( SID_PRESENTATION_LAYOUT, SFX_CALLMODE_ASYNCHRON );
        }
        break;
    }
}

/*************************************************************************
|*
|* Status der Snap-Objekt-Eintraege im Popup setzen
|*
\************************************************************************/

void  DrawViewShell::GetSnapItemState( SfxItemSet &rSet )
{
    SdrPageView* pPV;
    Point   aMPos = pWindow->PixelToLogic(aMousePos);
    USHORT  nHitLog = (USHORT) pWindow->PixelToLogic(
        Size(FuPoor::HITPIX,0)).Width();
    USHORT  nHelpLine;

    if ( pDrView->PickHelpLine(aMPos, nHitLog, *pWindow, nHelpLine, pPV) )
    {
        const SdrHelpLine& rHelpLine = (pPV->GetHelpLines())[nHelpLine];

        if ( rHelpLine.GetKind() == SDRHELPLINE_POINT )
        {
            rSet.Put( SfxStringItem( SID_SET_SNAPITEM,
                                String( SdResId( STR_POPUP_EDIT_SNAPPOINT))) );
            rSet.Put( SfxStringItem( SID_DELETE_SNAPITEM,
                                String( SdResId( STR_POPUP_DELETE_SNAPPOINT))) );
        }
        else
        {
            rSet.Put( SfxStringItem( SID_SET_SNAPITEM,
                                String( SdResId( STR_POPUP_EDIT_SNAPLINE))) );
            rSet.Put( SfxStringItem( SID_DELETE_SNAPITEM,
                                String( SdResId( STR_POPUP_DELETE_SNAPLINE))) );
        }
    }
}


/*************************************************************************
|*
|*
|*
\************************************************************************/

void DrawViewShell::AddWindow (::sd::Window* pWin)
{
    pDrView->AddWin(pWin);
}

/*************************************************************************
|*
|*
|*
\************************************************************************/

void DrawViewShell::RemoveWindow(::sd::Window* pWin)
{
    pDrView->DelWin(pWin);
}

} // end of namespace sd
