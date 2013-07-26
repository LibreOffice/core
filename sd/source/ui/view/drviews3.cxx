/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */


#include "DrawViewShell.hxx"

#include <sfx2/viewfrm.hxx>
#include <editeng/eeitem.hxx>
#include <editeng/tstpitem.hxx>
#include <editeng/lrspitem.hxx>
#include <editeng/protitem.hxx>
#include <editeng/frmdiritem.hxx>
#include <editeng/adjustitem.hxx>
#include <svx/ruler.hxx>
#include <editeng/numitem.hxx>
#include <svx/rulritem.hxx>
#include <sfx2/zoomitem.hxx>
#include <svx/svxids.hrc>
#include <svx/svdpagv.hxx>
#include <sfx2/request.hxx>
#include <sfx2/dispatch.hxx>
#include <tools/urlobj.hxx>
#include <svl/aeitem.hxx>
#include <svl/eitem.hxx>
#include <svl/rectitem.hxx>
#include <svl/stritem.hxx>
#include <svx/svdoole2.hxx>
#include <svl/itempool.hxx>
#include <svl/ptitem.hxx>
#include <basic/sbstar.hxx>
#include <basic/sberrors.hxx>
#include <svx/fmshell.hxx>
#include <svx/f3dchild.hxx>
#include <svx/float3d.hxx>
#include "optsitem.hxx"

#include "app.hrc"
#include "glob.hrc"
#include "strings.hrc"
#include "res_bmp.hrc"

#include "sdundogr.hxx"
#include "undopage.hxx"
#include "glob.hxx"
#include "sdmod.hxx"
#include "fupoor.hxx"
#include "slideshow.hxx"
#ifndef SD_FRAME_VIEW
#include "FrameView.hxx"
#endif
#include "sdpage.hxx"
#include "Window.hxx"
#include "sdresid.hxx"
#include "drawview.hxx"
#include "drawdoc.hxx"
#include "Ruler.hxx"
#include "DrawDocShell.hxx"
#include "headerfooterdlg.hxx"
#include "masterlayoutdlg.hxx"
#include "sdabstdlg.hxx"
#include <sfx2/ipclient.hxx>
#include <tools/diagnose_ex.h>
#include "ViewShellBase.hxx"
#include "FormShellManager.hxx"
#include "LayerTabBar.hxx"
#include <com/sun/star/drawing/framework/XControllerManager.hpp>
#include <com/sun/star/drawing/framework/XConfigurationController.hpp>
#include <com/sun/star/drawing/framework/XConfiguration.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <editeng/lspcitem.hxx>
#include <editeng/ulspitem.hxx>
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::drawing::framework;
using ::com::sun::star::frame::XFrame;
using ::com::sun::star::frame::XController;

namespace sd {

/**
 * handle SfxRequests for controller
 */
void  DrawViewShell::ExecCtrl(SfxRequest& rReq)
{
    // except a page switch and jumps to bookmarks, nothing is executed during
    // a slide show
    if( HasCurrentFunction(SID_PRESENTATION) &&
        rReq.GetSlot() != SID_SWITCHPAGE &&
        rReq.GetSlot() != SID_JUMPTOMARK)
        return;

    CheckLineTo (rReq);

    // End text edit mode for some requests.
    sal_uInt16 nSlot = rReq.GetSlot();
    switch (nSlot)
    {
        case SID_OUTPUT_QUALITY_COLOR:
        case SID_OUTPUT_QUALITY_GRAYSCALE:
        case SID_OUTPUT_QUALITY_BLACKWHITE:
        case SID_OUTPUT_QUALITY_CONTRAST:
            // Do nothing.
            break;
        default:
            if ( mpDrawView->IsTextEdit() )
            {
                mpDrawView->SdrEndTextEdit();
            }
    }

    //  sal_uInt16 nSlot = rReq.GetSlot();
    switch (nSlot)
    {
        case SID_SWITCHPAGE:  // BASIC
        {
            // switch page in running slide show
            if(SlideShow::IsRunning(GetViewShellBase()) && rReq.GetArgs())
            {
                SFX_REQUEST_ARG(rReq, pWhatPage, SfxUInt32Item, ID_VAL_WHATPAGE, sal_False);
                SlideShow::GetSlideShow(GetViewShellBase())->jumpToPageNumber((sal_Int32)((pWhatPage->GetValue()-1)>>1));
            }
            else
            {
                const SfxItemSet *pArgs = rReq.GetArgs ();
                sal_uInt16 nSelectedPage = 0;

                if (! pArgs)
                {
                    nSelectedPage = maTabControl.GetCurPageId() - 1;
                }
                else if (pArgs->Count () == 2)
                {
                    SFX_REQUEST_ARG (rReq, pWhatPage, SfxUInt32Item, ID_VAL_WHATPAGE, sal_False);
                    SFX_REQUEST_ARG (rReq, pWhatKind, SfxUInt32Item, ID_VAL_WHATKIND, sal_False);

                    sal_Int32 nWhatPage = (sal_Int32)pWhatPage->GetValue ();
                    sal_Int32 nWhatKind = (sal_Int32)pWhatKind->GetValue ();
                    if (! CHECK_RANGE (PK_STANDARD, nWhatKind, PK_HANDOUT))
                    {
#ifndef DISABLE_SCRIPTING
                        StarBASIC::FatalError (SbERR_BAD_PROP_VALUE);
#endif
                        rReq.Ignore ();
                        break;
                    }
                    else if (meEditMode != EM_MASTERPAGE)
                    {
                        if (! CHECK_RANGE (0, nWhatPage, GetDoc()->GetSdPageCount((PageKind)nWhatKind)))
                        {
#ifndef DISABLE_SCRIPTING
                            StarBASIC::FatalError (SbERR_BAD_PROP_VALUE);
#endif
                            rReq.Ignore ();
                            break;
                        }

                        nSelectedPage = (short) nWhatPage;
                        mePageKind    = (PageKind) nWhatKind;
                    }
                }
                else
                {
#ifndef DISABLE_SCRIPTING
                    StarBASIC::FatalError (SbERR_WRONG_ARGS);
#endif
                    rReq.Ignore ();
                    break;
                }


                if( GetDocSh() && (GetDocSh()->GetCreateMode() == SFX_CREATE_MODE_EMBEDDED))
                    GetDocSh()->SetModified();

                SwitchPage(nSelectedPage);

                if(HasCurrentFunction(SID_BEZIER_EDIT))
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
            sal_uInt16 nCurPage = GetLayerTabControl()->GetCurPageId ();

            if( pArgs && pArgs->Count () == 1)
            {
                SFX_REQUEST_ARG (rReq, pWhatLayer, SfxUInt32Item, ID_VAL_WHATLAYER, sal_False);
                if( pWhatLayer )
                    nCurPage = (short) pWhatLayer->GetValue ();
            }

            mpDrawView->SetActiveLayer( GetLayerTabControl()->GetPageText(nCurPage) );
            Invalidate();
            rReq.Done ();

            break;
        }

        case SID_PAGEMODE:  // BASIC
        {

            const SfxItemSet *pArgs = rReq.GetArgs ();

            if ( pArgs && pArgs->Count () == 2)
            {
                SFX_REQUEST_ARG (rReq, pIsActive, SfxBoolItem, ID_VAL_ISACTIVE, sal_False);
                SFX_REQUEST_ARG (rReq, pWhatKind, SfxUInt32Item, ID_VAL_WHATKIND, sal_False);

                sal_Int32 nWhatKind = (sal_Int32)pWhatKind->GetValue ();
                if (CHECK_RANGE (PK_STANDARD, nWhatKind, PK_HANDOUT))
                {
                    mbIsLayerModeActive = pIsActive->GetValue ();
                    mePageKind = (PageKind) nWhatKind;
                }
            }

            // turn on default layer of page
            mpDrawView->SetActiveLayer( String( SdResId(STR_LAYER_LAYOUT) ) );

            ChangeEditMode(EM_PAGE, mbIsLayerModeActive);

            Invalidate();
            rReq.Done ();

            break;
        }

        case SID_LAYERMODE:  // BASIC
        {
            const SfxItemSet *pArgs = rReq.GetArgs ();

            if ( pArgs && pArgs->Count () == 2)
            {
                SFX_REQUEST_ARG (rReq, pWhatLayerMode, SfxBoolItem, ID_VAL_ISACTIVE, sal_False);
                SFX_REQUEST_ARG (rReq, pWhatLayer, SfxUInt32Item, ID_VAL_WHATLAYER, sal_False);

                sal_Int32 nWhatLayer = (sal_Int32)pWhatLayer->GetValue ();
                if (CHECK_RANGE (EM_PAGE, nWhatLayer, EM_MASTERPAGE))
                {
                    mbIsLayerModeActive = pWhatLayerMode->GetValue ();
                    meEditMode = (EditMode) nWhatLayer;
                }
            }

            ChangeEditMode(meEditMode, !mbIsLayerModeActive);

            Invalidate();
            rReq.Done ();

            break;
        }

        case SID_HEADER_AND_FOOTER:
        case SID_INSERT_PAGE_NUMBER:
        case SID_INSERT_DATE_TIME:
        {
            SdAbstractDialogFactory* pFact = SdAbstractDialogFactory::Create();
            AbstractHeaderFooterDialog* pDlg = pFact ? pFact->CreateHeaderFooterDialog( (::ViewShell*)this, GetActiveWindow(), GetDoc(), mpActualPage ) : 0;
            if( pDlg )
            {
                pDlg->Execute();
                delete pDlg;

                GetActiveWindow()->Invalidate();
                UpdatePreview( mpActualPage );
            }

            Invalidate();
            rReq.Done ();

            break;
        }

        case SID_MASTER_LAYOUTS:
        {
            SdPage* pPage = GetActualPage();
            if (meEditMode == EM_MASTERPAGE)
                // Use the master page of the current page.
                pPage = static_cast<SdPage*>(&pPage->TRG_GetMasterPage());

            SdAbstractDialogFactory* pFact = SdAbstractDialogFactory::Create();
            VclAbstractDialog* pDlg = pFact ? pFact->CreateMasterLayoutDialog( GetActiveWindow(), GetDoc(), pPage ) : 0;
            if( pDlg )
            {
                pDlg->Execute();
                delete pDlg;
                Invalidate();
            }
            rReq.Done ();
            break;
        }
        case SID_OBJECTRESIZE:
        {
            // The server likes to change the client size
            OSL_ASSERT (GetViewShell()!=NULL);
            SfxInPlaceClient* pIPClient = GetViewShell()->GetIPClient();

            if ( pIPClient && pIPClient->IsObjectInPlaceActive() )
            {
                const SfxRectangleItem& rRect =
                    (SfxRectangleItem&)rReq.GetArgs()->Get(SID_OBJECTRESIZE);
                Rectangle aRect( GetActiveWindow()->PixelToLogic( rRect.GetValue() ) );

                if ( mpDrawView->AreObjectsMarked() )
                {
                    const SdrMarkList& rMarkList = mpDrawView->GetMarkedObjectList();

                    if (rMarkList.GetMarkCount() == 1)
                    {
                        SdrMark* pMark = rMarkList.GetMark(0);
                        SdrObject* pObj = pMark->GetMarkedSdrObj();

                        SdrOle2Obj* pOle2Obj = dynamic_cast< SdrOle2Obj* >( pObj );
                        if(pOle2Obj)
                        {
                            if( pOle2Obj->GetObjRef().is() )
                            {
                                pOle2Obj->SetLogicRect(aRect);
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
            sal_uInt16 nId = Svx3DChildWindow::GetChildWindowId();
            SfxViewFrame* pFrame = GetViewFrame();

            try
            {
                Reference< XFrame > xFrame( pFrame->GetFrame().GetFrameInterface(), UNO_SET_THROW );

                // Save the current configuration of panes and views.
                Reference<XControllerManager> xControllerManager (
                    GetViewShellBase().GetController(), UNO_QUERY_THROW);
                Reference<XConfigurationController> xConfigurationController (
                    xControllerManager->getConfigurationController(), UNO_QUERY_THROW );
                Reference<XConfiguration> xConfiguration (
                    xConfigurationController->getRequestedConfiguration(), UNO_SET_THROW );

                SfxChildWindow* pWindow = pFrame->GetChildWindow(nId);
                if(pWindow)
                {
                    Svx3DWin* p3DWin = (Svx3DWin*)(pWindow->GetWindow());
                    if(p3DWin)
                        p3DWin->DocumentReload();
                }

                // normal forwarding to ViewFrame for execution
                GetViewFrame()->ExecuteSlot(rReq);

                // From here on we must cope with this object and the frame already being
                // deleted.  Do not call any methods or use data members.
                Reference<XController> xController( xFrame->getController(), UNO_SET_THROW );

                // Restore the configuration.
                xControllerManager = Reference<XControllerManager>( xController, UNO_QUERY_THROW);
                xConfigurationController = Reference<XConfigurationController>(
                    xControllerManager->getConfigurationController());
                if ( ! xConfigurationController.is())
                    throw RuntimeException();
                xConfigurationController->restoreConfiguration(xConfiguration);
            }
            catch (RuntimeException&)
            {
                DBG_UNHANDLED_EXCEPTION();
            }

            // We have to return immediately to avoid accessing this object.
            return;
        }

        case SID_JUMPTOMARK:
        {
            if( rReq.GetArgs() )
            {
                SFX_REQUEST_ARG(rReq, pBookmark, SfxStringItem, SID_JUMPTOMARK, sal_False);

                if (pBookmark)
                {
                    OUString sBookmark(INetURLObject::decode(pBookmark->GetValue(), '%', INetURLObject::DECODE_WITH_CHARSET));

                    rtl::Reference< sd::SlideShow > xSlideshow( SlideShow::GetSlideShow( GetViewShellBase() ) );
                    if(xSlideshow.is() && xSlideshow->isRunning())
                    {
                        xSlideshow->jumpToBookmark(sBookmark);
                    }
                    else
                    {
                        GotoBookmark(sBookmark);
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
        {
            ExecReq( rReq );
            break;
        }

        case SID_MAIL_SCROLLBODY_PAGEDOWN:
        {
            ExecReq( rReq );
            break;
        }

        case SID_ATTR_YEAR2000:
        {
            FmFormShell* pFormShell = GetViewShellBase().GetFormShellManager()->GetFormShell();
            if (pFormShell != NULL)
            {
                const SfxPoolItem* pItem;
                if (rReq.GetArgs()->GetItemState(
                    SID_ATTR_YEAR2000, sal_True, &pItem) == SFX_ITEM_SET)
                    pFormShell->SetY2KState (
                        static_cast<const SfxUInt16Item*>(pItem)->GetValue());
            }

            rReq.Done();
        }
        break;

        case SID_OPT_LOCALE_CHANGED:
        {
            GetActiveWindow()->Invalidate();
            UpdatePreview( mpActualPage );
            rReq.Done();
        }

        default:
        break;
    }
}


void  DrawViewShell::ExecRuler(SfxRequest& rReq)
{
    // nothing is executed during a slide show!
    if(HasCurrentFunction(SID_PRESENTATION))
        return;

    CheckLineTo (rReq);

    const SfxItemSet* pArgs = rReq.GetArgs();
    const Point aPagePos( GetActiveWindow()->GetViewOrigin() );
    Size aPageSize = mpActualPage->GetSize();
    Size aViewSize = GetActiveWindow()->GetViewSize();
    SdUndoGroup* pUndoGroup = NULL;

    if ( rReq.GetSlot() == SID_ATTR_LONG_LRSPACE ||
         rReq.GetSlot() == SID_ATTR_LONG_ULSPACE )
    {
        pUndoGroup = new SdUndoGroup(GetDoc());
        OUString aString(SdResId(STR_UNDO_CHANGE_PAGEBORDER));
        pUndoGroup->SetComment(aString);
    }

    switch ( rReq.GetSlot() )
    {
        case SID_ATTR_LONG_LRSPACE:
        {
            const SvxLongLRSpaceItem& rLRSpace = (const SvxLongLRSpaceItem&)
                    pArgs->Get(GetPool().GetWhich(SID_ATTR_LONG_LRSPACE));

            if( mpDrawView->IsTextEdit() )
            {
                Rectangle aRect = maMarkRect;
                aRect.SetPos(aRect.TopLeft() + aPagePos);
                aRect.Left()  = rLRSpace.GetLeft();
                aRect.Right() = aViewSize.Width() - rLRSpace.GetRight();
                aRect.SetPos(aRect.TopLeft() - aPagePos);
                if ( aRect != maMarkRect)
                {
                    mpDrawView->SetAllMarkedRect(aRect);
                    maMarkRect = mpDrawView->GetAllMarkedRect();
                    Invalidate( SID_RULER_OBJECT );
                }
            }
            else
            {
                long nLeft = std::max(0L, rLRSpace.GetLeft() - aPagePos.X());
                long nRight = std::max(0L, rLRSpace.GetRight() + aPagePos.X() +
                                      aPageSize.Width() - aViewSize.Width());

                sal_uInt16 nPageCnt = GetDoc()->GetSdPageCount(mePageKind);
                sal_uInt16 i;
                for ( i = 0; i < nPageCnt; i++)
                {
                    SdPage* pPage = GetDoc()->GetSdPage(i, mePageKind);
                    SdUndoAction* pUndo = new SdPageLRUndoAction(GetDoc(),
                                            pPage,
                                            pPage->GetLftBorder(),
                                            pPage->GetRgtBorder(),
                                            nLeft, nRight);
                    pUndoGroup->AddAction(pUndo);
                    pPage->SetLftBorder(nLeft);
                    pPage->SetRgtBorder(nRight);
                }
                nPageCnt = GetDoc()->GetMasterSdPageCount(mePageKind);

                for (i = 0; i < nPageCnt; i++)
                {
                    SdPage* pPage = GetDoc()->GetMasterSdPage(i, mePageKind);
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

            if( mpDrawView->IsTextEdit() )
            {
                Rectangle aRect = maMarkRect;
                aRect.SetPos(aRect.TopLeft() + aPagePos);
                aRect.Top()  = rULSpace.GetUpper();
                aRect.Bottom() = aViewSize.Height() - rULSpace.GetLower();
                aRect.SetPos(aRect.TopLeft() - aPagePos);

                if ( aRect != maMarkRect)
                {
                    mpDrawView->SetAllMarkedRect(aRect);
                    maMarkRect = mpDrawView->GetAllMarkedRect();
                    Invalidate( SID_RULER_OBJECT );
                }
            }
            else
            {
                long nUpper = std::max(0L, rULSpace.GetUpper() - aPagePos.Y());
                long nLower = std::max(0L, rULSpace.GetLower() + aPagePos.Y() +
                                      aPageSize.Height() - aViewSize.Height());

                sal_uInt16 nPageCnt = GetDoc()->GetSdPageCount(mePageKind);
                sal_uInt16 i;
                for ( i = 0; i < nPageCnt; i++)
                {
                    SdPage* pPage = GetDoc()->GetSdPage(i, mePageKind);
                    SdUndoAction* pUndo = new SdPageULUndoAction(GetDoc(),
                                            pPage,
                                            pPage->GetUppBorder(),
                                            pPage->GetLwrBorder(),
                                            nUpper, nLower);
                    pUndoGroup->AddAction(pUndo);
                    pPage->SetUppBorder(nUpper);
                    pPage->SetLwrBorder(nLower);
                }
                nPageCnt = GetDoc()->GetMasterSdPageCount(mePageKind);

                for (i = 0; i < nPageCnt; i++)
                {
                    SdPage* pPage = GetDoc()->GetMasterSdPage(i, mePageKind);
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
            Rectangle aRect = maMarkRect;
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
            if ( aRect != maMarkRect)
            {
                mpDrawView->SetAllMarkedRect(aRect);
                maMarkRect = mpDrawView->GetAllMarkedRect();
                Invalidate( SID_RULER_OBJECT );
            }
            break;
        }

        case SID_ATTR_TABSTOP:
        {
            if( mpDrawView->IsTextEdit() )
            {
                const SvxTabStopItem& rItem = (const SvxTabStopItem&)
                            pArgs->Get( EE_PARA_TABS );

                SfxItemSet aEditAttr( GetPool(), EE_PARA_TABS, EE_PARA_TABS );

                aEditAttr.Put( rItem );
                mpDrawView->SetAttributes( aEditAttr );

                Invalidate(SID_ATTR_TABSTOP);
            }
            break;
        }

        case SID_ATTR_PARA_LINESPACE:
        {
            sal_uInt16 nSlot = SID_ATTR_PARA_LINESPACE;
            SvxLineSpacingItem aParaLineSP = (const SvxLineSpacingItem&)pArgs->Get(
                GetPool().GetWhich(nSlot));

            SfxItemSet aEditAttr( GetPool(), EE_PARA_SBL, EE_PARA_SBL );
            aParaLineSP.SetWhich( EE_PARA_SBL );

            aEditAttr.Put( aParaLineSP );
            mpDrawView->SetAttributes( aEditAttr );

            Invalidate(SID_ATTR_PARA_LINESPACE);
        }
        break;
        case SID_ATTR_PARA_ADJUST_LEFT:
        {
            SvxAdjustItem aItem( SVX_ADJUST_LEFT, EE_PARA_JUST );
            SfxItemSet aEditAttr( GetPool(), EE_PARA_JUST, EE_PARA_JUST );

            aEditAttr.Put( aItem );
            mpDrawView->SetAttributes( aEditAttr );

            Invalidate(SID_ATTR_PARA_ADJUST_LEFT);
        }
        break;
        case SID_ATTR_PARA_ADJUST_CENTER:
        {
            SvxAdjustItem aItem( SVX_ADJUST_CENTER, EE_PARA_JUST );
            SfxItemSet aEditAttr( GetPool(), EE_PARA_JUST, EE_PARA_JUST );

            aEditAttr.Put( aItem );
            mpDrawView->SetAttributes( aEditAttr );

            Invalidate(SID_ATTR_PARA_ADJUST_CENTER);
        }
        break;
        case SID_ATTR_PARA_ADJUST_RIGHT:
        {
            SvxAdjustItem aItem( SVX_ADJUST_RIGHT, EE_PARA_JUST );
            SfxItemSet aEditAttr( GetPool(), EE_PARA_JUST, EE_PARA_JUST );

            aEditAttr.Put( aItem );
            mpDrawView->SetAttributes( aEditAttr );

            Invalidate(SID_ATTR_PARA_ADJUST_RIGHT);
        }
        break;
        case SID_ATTR_PARA_ADJUST_BLOCK:
        {
            SvxAdjustItem aItem( SVX_ADJUST_BLOCK, EE_PARA_JUST );
            SfxItemSet aEditAttr( GetPool(), EE_PARA_JUST, EE_PARA_JUST );

            aEditAttr.Put( aItem );
            mpDrawView->SetAttributes( aEditAttr );

            Invalidate(SID_ATTR_PARA_ADJUST_BLOCK);
        }
        break;
        case SID_ATTR_PARA_ULSPACE:
        {
            sal_uInt16 nSlot = SID_ATTR_PARA_ULSPACE;
            SvxULSpaceItem aULSP = (const SvxULSpaceItem&)pArgs->Get(
                GetPool().GetWhich(nSlot));
            SfxItemSet aEditAttr( GetPool(), EE_PARA_ULSPACE, EE_PARA_ULSPACE );
            aULSP.SetWhich( EE_PARA_ULSPACE );

            aEditAttr.Put( aULSP );
            mpDrawView->SetAttributes( aEditAttr );

            Invalidate(SID_ATTR_PARA_ULSPACE);
        }
        break;

        case SID_ATTR_PARA_LRSPACE:
        {
            sal_uInt16 nSlot = SID_ATTR_PARA_LRSPACE;
            SvxLRSpaceItem aLRSpace = (const SvxLRSpaceItem&)pArgs->Get(
                GetPool().GetWhich(nSlot));

            SfxItemSet aEditAttr( GetPool(), EE_PARA_LRSPACE, EE_PARA_LRSPACE );
            aLRSpace.SetWhich( EE_PARA_LRSPACE );

            aEditAttr.Put( aLRSpace );
            mpDrawView->SetAttributes( aEditAttr );

            Invalidate(SID_ATTR_PARA_LRSPACE);
            break;
        }
        case SID_ATTR_LRSPACE:
        {
            if( mpDrawView->IsTextEdit() )
            {
                sal_uInt16 nId = SID_ATTR_PARA_LRSPACE;
                const SvxLRSpaceItem& rItem = (const SvxLRSpaceItem&)
                            pArgs->Get( nId );

                static const sal_uInt16 aWhichTable[]=
                {
                    EE_PARA_OUTLLEVEL, EE_PARA_OUTLLEVEL,
                    EE_PARA_LRSPACE, EE_PARA_LRSPACE,
                    EE_PARA_NUMBULLET, EE_PARA_NUMBULLET,
                    0, 0
                };

                SfxItemSet aEditAttr( GetDoc()->GetPool(),
                                      aWhichTable );
                mpDrawView->GetAttributes( aEditAttr );

                nId = EE_PARA_LRSPACE;
                SvxLRSpaceItem aLRSpaceItem( rItem.GetLeft(),
                        rItem.GetRight(), rItem.GetTxtLeft(),
                        rItem.GetTxtFirstLineOfst(), nId );

                const sal_Int16 nOutlineLevel = ((const SfxInt16Item&)aEditAttr.Get( EE_PARA_OUTLLEVEL )).GetValue();
                const SvxLRSpaceItem& rOrigLRSpaceItem = (const SvxLRSpaceItem&) aEditAttr.Get( EE_PARA_LRSPACE );
                const SvxNumBulletItem& rNumBulletItem = (const SvxNumBulletItem&) aEditAttr.Get( EE_PARA_NUMBULLET );
                if( nOutlineLevel != -1 &&
                    rNumBulletItem.GetNumRule() &&
                    rNumBulletItem.GetNumRule()->GetLevelCount() > nOutlineLevel )
                {
                    const SvxNumberFormat& rFormat = rNumBulletItem.GetNumRule()->GetLevel(nOutlineLevel);
                    SvxNumberFormat aFormat(rFormat);

                    // left margin gets distributed onto LRSpace item
                    // and number format AbsLSpace - this fixes
                    // n#707779 (previously, LRSpace left indent could
                    // become negative - EditEngine really does not
                    // like that.
                    const short nAbsLSpace=aFormat.GetAbsLSpace();
                    const long  nTxtLeft=rItem.GetTxtLeft();
                    const long  nLeftIndent=std::max(0L,nTxtLeft - nAbsLSpace);
                    aLRSpaceItem.SetTxtLeft(nLeftIndent);
                    // control for clipped left indent - remainder
                    // reduces number format first line indent
                    aFormat.SetAbsLSpace(nTxtLeft - nLeftIndent);

                    // negative first line indent goes to the number
                    // format, positive to the lrSpace item
                    if( rItem.GetTxtFirstLineOfst() < 0 )
                    {
                        aFormat.SetFirstLineOffset(
                            rItem.GetTxtFirstLineOfst()
                            - rOrigLRSpaceItem.GetTxtFirstLineOfst()
                            + aFormat.GetCharTextDistance());
                        aLRSpaceItem.SetTxtFirstLineOfst(0);
                    }
                    else
                    {
                        aFormat.SetFirstLineOffset(0);
                        aLRSpaceItem.SetTxtFirstLineOfst(
                            rItem.GetTxtFirstLineOfst()
                            - aFormat.GetFirstLineOffset()
                            + aFormat.GetCharTextDistance());
                    }

                    if( rFormat != aFormat )
                    {
                        // put all items
                        SvxNumBulletItem aNumBulletItem(rNumBulletItem);
                        aNumBulletItem.GetNumRule()->SetLevel(nOutlineLevel,aFormat);
                        aEditAttr.Put( aNumBulletItem );
                        aEditAttr.Put( aLRSpaceItem );
                        mpDrawView->SetAttributes( aEditAttr );

                        Invalidate(SID_ATTR_PARA_LRSPACE);
                        break;
                    }
                }

                // only put lrSpace item
                SfxItemSet aEditAttrReduced( GetDoc()->GetPool(),
                                             EE_PARA_LRSPACE, EE_PARA_LRSPACE );
                aEditAttrReduced.Put( aLRSpaceItem );
                mpDrawView->SetAttributes( aEditAttrReduced );

                Invalidate(SID_ATTR_PARA_LRSPACE);
            }
            break;
        }
    }
    if ( pUndoGroup )
        // give the undo group to the undo manager
        GetViewFrame()->GetObjectShell()->GetUndoManager()->
                                            AddUndoAction(pUndoGroup);
}


void  DrawViewShell::GetRulerState(SfxItemSet& rSet)
{
    Point aOrigin;

    if (mpDrawView->GetSdrPageView())
    {
        aOrigin = mpDrawView->GetSdrPageView()->GetPageOrigin();
    }

    Size aViewSize = GetActiveWindow()->GetViewSize();

    const Point aPagePos( GetActiveWindow()->GetViewOrigin() );
    Size aPageSize = mpActualPage->GetSize();

    Rectangle aRect(aPagePos, Point( aViewSize.Width() - (aPagePos.X() + aPageSize.Width()),
                                     aViewSize.Height() - (aPagePos.Y() + aPageSize.Height())));

    if( mpDrawView->IsTextEdit() )
    {
        Point aPnt1 = GetActiveWindow()->GetWinViewPos();
        Rectangle aMinMaxRect = Rectangle( aPnt1, Size(ULONG_MAX, ULONG_MAX) );
        rSet.Put( SfxRectangleItem(SID_RULER_LR_MIN_MAX, aMinMaxRect) );
    }
    else
    {
        rSet.Put( SfxRectangleItem(SID_RULER_LR_MIN_MAX, aRect) );
    }

    SvxLongLRSpaceItem aLRSpace(aPagePos.X() + mpActualPage->GetLftBorder(),
                                aRect.Right() + mpActualPage->GetRgtBorder(),
                                GetPool().GetWhich(SID_ATTR_LONG_LRSPACE));
    SvxLongULSpaceItem aULSpace(aPagePos.Y() + mpActualPage->GetUppBorder(),
                                aRect.Bottom() + mpActualPage->GetLwrBorder(),
                                GetPool().GetWhich(SID_ATTR_LONG_ULSPACE));
    rSet.Put(SvxPagePosSizeItem(Point(0,0) - aPagePos, aViewSize.Width(),
                                                       aViewSize.Height()));
    SfxPointItem aPointItem( SID_RULER_NULL_OFFSET, aPagePos + aOrigin );

    SvxProtectItem aProtect( SID_RULER_PROTECT );

    maMarkRect = mpDrawView->GetAllMarkedRect();

    const sal_Bool bRTL = GetDoc() && GetDoc()->GetDefaultWritingMode() == ::com::sun::star::text::WritingMode_RL_TB;
    rSet.Put(SfxBoolItem(SID_RULER_TEXT_RIGHT_TO_LEFT, bRTL));

    if( mpDrawView->AreObjectsMarked() )
    {
        if( mpDrawView->IsTextEdit() )
        {
            SdrObject* pObj = mpDrawView->GetMarkedObjectList().GetMark( 0 )->GetMarkedSdrObj();
            if( pObj->GetObjInventor() == SdrInventor)
            {
                SfxItemSet aEditAttr( GetDoc()->GetPool() );
                mpDrawView->GetAttributes( aEditAttr );
                if( aEditAttr.GetItemState( EE_PARA_TABS ) >= SFX_ITEM_AVAILABLE )
                {
                    const SvxTabStopItem& rItem = (const SvxTabStopItem&) aEditAttr.Get( EE_PARA_TABS );
                    rSet.Put( rItem );

                    const SvxLRSpaceItem& rLRSpaceItem = (const SvxLRSpaceItem&) aEditAttr.Get( EE_PARA_LRSPACE );
                    sal_uInt16 nId = SID_ATTR_PARA_LRSPACE;
                    SvxLRSpaceItem aLRSpaceItem( rLRSpaceItem.GetLeft(),
                            rLRSpaceItem.GetRight(), rLRSpaceItem.GetTxtLeft(),
                            rLRSpaceItem.GetTxtFirstLineOfst(), nId );

                    const sal_Int16 nOutlineLevel = ((const SfxInt16Item&)aEditAttr.Get( EE_PARA_OUTLLEVEL )).GetValue();
                    const SvxNumBulletItem& rNumBulletItem = (const SvxNumBulletItem&) aEditAttr.Get( EE_PARA_NUMBULLET );
                    if( nOutlineLevel != -1 &&
                        rNumBulletItem.GetNumRule() &&
                        rNumBulletItem.GetNumRule()->GetLevelCount() > nOutlineLevel )
                    {
                        const SvxNumberFormat& rFormat = rNumBulletItem.GetNumRule()->GetLevel(nOutlineLevel);
                        aLRSpaceItem.SetTxtLeft(rFormat.GetAbsLSpace() + rLRSpaceItem.GetTxtLeft());
                        aLRSpaceItem.SetTxtFirstLineOfst(
                            rLRSpaceItem.GetTxtFirstLineOfst() + rFormat.GetFirstLineOffset()
                            - rFormat.GetCharTextDistance());
                    }

                    rSet.Put( aLRSpaceItem );

                    Point aPos( aPagePos + maMarkRect.TopLeft() );

                    if ( aEditAttr.GetItemState( SDRATTR_TEXT_LEFTDIST ) == SFX_ITEM_ON )
                    {
                        const SdrTextLeftDistItem& rTLDItem = (const SdrTextLeftDistItem&)
                                                              aEditAttr.Get( SDRATTR_TEXT_LEFTDIST );
                        long nLD = rTLDItem.GetValue();
                        aPos.X() += nLD;
                    }

                    aPointItem.SetValue( aPos );

                    aLRSpace.SetLeft( aPagePos.X() + maMarkRect.Left() );

                    if ( aEditAttr.GetItemState( SDRATTR_TEXT_LEFTDIST ) == SFX_ITEM_ON )
                    {
                        const SdrTextLeftDistItem& rTLDItem = (const SdrTextLeftDistItem&)
                                                              aEditAttr.Get( SDRATTR_TEXT_LEFTDIST );
                        long nLD = rTLDItem.GetValue();
                        aLRSpace.SetLeft( aLRSpace.GetLeft() + nLD );
                    }

                    aLRSpace.SetRight( aRect.Right() + aPageSize.Width() - maMarkRect.Right() );
                    aULSpace.SetUpper( aPagePos.Y() + maMarkRect.Top() );
                    aULSpace.SetLower( aRect.Bottom() + aPageSize.Height() - maMarkRect.Bottom() );

                    rSet.DisableItem( SID_RULER_OBJECT );

                    // lock page margins
                    aProtect.SetSizeProtect( sal_True );
                    aProtect.SetPosProtect( sal_True );
                }

                if( aEditAttr.GetItemState( EE_PARA_WRITINGDIR ) >= SFX_ITEM_AVAILABLE )
                {
                    const SvxFrameDirectionItem& rItem = (const SvxFrameDirectionItem&) aEditAttr.Get( EE_PARA_WRITINGDIR );
                    rSet.Put(SfxBoolItem(SID_RULER_TEXT_RIGHT_TO_LEFT, rItem.GetValue() == ::com::sun::star::text::WritingMode_RL_TB));
                }
            }
        }
        else
        {
            rSet.DisableItem( EE_PARA_TABS );
            rSet.DisableItem( SID_RULER_TEXT_RIGHT_TO_LEFT );

            if( mpDrawView->IsResizeAllowed(sal_True) )
            {
                Rectangle aResizeRect( maMarkRect );

                aResizeRect.SetPos(aResizeRect.TopLeft() + aPagePos);
                SvxObjectItem aObjItem(aResizeRect.Left(), aResizeRect.Right(),
                                       aResizeRect.Top(), aResizeRect.Bottom());
                rSet.Put(aObjItem);
                rSet.DisableItem( EE_PARA_TABS );
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
        rSet.DisableItem( EE_PARA_TABS );
    }

    rSet.Put( aLRSpace );
    rSet.Put( aULSpace );

    rSet.Put( aPointItem );
    rSet.Put( aProtect );
}


void  DrawViewShell::ExecStatusBar(SfxRequest& rReq)
{
    // nothing is executed during a slide show!
    if(HasCurrentFunction(SID_PRESENTATION))
        return;

    CheckLineTo (rReq);

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

/**
 * set state of snap object entries in popup
 */
void  DrawViewShell::GetSnapItemState( SfxItemSet &rSet )
{
    SdrPageView* pPV;
    Point   aMPos = GetActiveWindow()->PixelToLogic(maMousePos);
    sal_uInt16  nHitLog = (sal_uInt16) GetActiveWindow()->PixelToLogic(
        Size(FuPoor::HITPIX,0)).Width();
    sal_uInt16  nHelpLine;

    if ( mpDrawView->PickHelpLine(aMPos, nHitLog, *GetActiveWindow(), nHelpLine, pPV) )
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

void DrawViewShell::AddWindow (::sd::Window* pWin)
{
    mpDrawView->AddWindowToPaintView(pWin);
}

void DrawViewShell::RemoveWindow(::sd::Window* pWin)
{
    mpDrawView->DeleteWindowFromPaintView(pWin);
}

} // end of namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
