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

#include <config_features.h>

#include <DrawViewShell.hxx>

#include <sfx2/viewfrm.hxx>
#include <editeng/eeitem.hxx>
#include <editeng/tstpitem.hxx>
#include <editeng/lrspitem.hxx>
#include <editeng/protitem.hxx>
#include <editeng/frmdiritem.hxx>
#include <editeng/adjustitem.hxx>
#include <svx/ruler.hxx>
#include <svx/svdotable.hxx>
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
#include <optsitem.hxx>

#include <app.hrc>
#include <strings.hrc>

#include <sdundogr.hxx>
#include <undopage.hxx>
#include <glob.hxx>
#include <sdmod.hxx>
#include <fupoor.hxx>
#include <slideshow.hxx>
#include <FrameView.hxx>
#include <sdpage.hxx>
#include <Window.hxx>
#include <sdresid.hxx>
#include <unokywds.hxx>
#include <drawview.hxx>
#include <drawdoc.hxx>
#include <Ruler.hxx>
#include <DrawDocShell.hxx>
#include <headerfooterdlg.hxx>
#include <masterlayoutdlg.hxx>
#include <sdabstdlg.hxx>
#include <sfx2/ipclient.hxx>
#include <tools/diagnose_ex.h>
#include <ViewShellBase.hxx>
#include <FormShellManager.hxx>
#include <LayerTabBar.hxx>
#include <com/sun/star/drawing/framework/XControllerManager.hpp>
#include <com/sun/star/drawing/framework/XConfigurationController.hpp>
#include <com/sun/star/drawing/framework/XConfiguration.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <editeng/lspcitem.hxx>
#include <editeng/ulspitem.hxx>
#include <memory>

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
                const SfxUInt32Item* pWhatPage = rReq.GetArg<SfxUInt32Item>(ID_VAL_WHATPAGE);
                SlideShow::GetSlideShow(GetViewShellBase())->jumpToPageNumber(static_cast<sal_Int32>((pWhatPage->GetValue()-1)>>1));
            }
            else
            {
                const SfxItemSet *pArgs = rReq.GetArgs ();
                sal_uInt16 nSelectedPage = 0;

                if (! pArgs)
                {
                    nSelectedPage = maTabControl->GetCurPagePos();
                }
                else if (pArgs->Count () == 2)
                {
                    const SfxUInt32Item* pWhatPage = rReq.GetArg<SfxUInt32Item>(ID_VAL_WHATPAGE);
                    const SfxUInt32Item* pWhatKind = rReq.GetArg<SfxUInt32Item>(ID_VAL_WHATKIND);

                    sal_Int32 nWhatPage = static_cast<sal_Int32>(pWhatPage->GetValue ());
                    PageKind nWhatKind = static_cast<PageKind>(pWhatKind->GetValue ());
                    if (! (nWhatKind >= PageKind::Standard && nWhatKind <= PageKind::Handout))
                    {
#if HAVE_FEATURE_SCRIPTING
                        StarBASIC::FatalError (ERRCODE_BASIC_BAD_PROP_VALUE);
#endif
                        rReq.Ignore ();
                        break;
                    }
                    else if (meEditMode != EditMode::MasterPage)
                    {
                        if (! CHECK_RANGE (0, nWhatPage, GetDoc()->GetSdPageCount(nWhatKind)))
                        {
#if HAVE_FEATURE_SCRIPTING
                            StarBASIC::FatalError (ERRCODE_BASIC_BAD_PROP_VALUE);
#endif
                            rReq.Ignore ();
                            break;
                        }

                        nSelectedPage = static_cast<short>(nWhatPage);
                        mePageKind    = nWhatKind;
                    }
                }
                else
                {
#if HAVE_FEATURE_SCRIPTING
                    StarBASIC::FatalError (ERRCODE_BASIC_WRONG_ARGS);
#endif
                    rReq.Ignore ();
                    break;
                }

                if( GetDocSh() && (GetDocSh()->GetCreateMode() == SfxObjectCreateMode::EMBEDDED))
                    GetDocSh()->SetModified();

                SwitchPage(nSelectedPage);

                if(HasCurrentFunction(SID_BEZIER_EDIT))
                    GetViewFrame()->GetDispatcher()->Execute(SID_OBJECT_SELECT, SfxCallMode::ASYNCHRON);

                Invalidate();
                InvalidateWindows();
                rReq.Done ();
            }
            break;
        }

        case SID_SWITCHLAYER:  // BASIC
        {
            const SfxItemSet *pArgs = rReq.GetArgs ();

            // #i87182#
            bool bCurPageValid(false);
            sal_uInt16 nCurPage(0);

            if(GetLayerTabControl())
            {
                nCurPage = GetLayerTabControl()->GetCurPageId();
                bCurPageValid = true;
            }

            if(pArgs && 1 == pArgs->Count())
            {
                const SfxUInt32Item* pWhatLayer = rReq.GetArg<SfxUInt32Item>(ID_VAL_WHATLAYER);

                if(pWhatLayer)
                {
                    nCurPage = static_cast<short>(pWhatLayer->GetValue());
                    bCurPageValid = true;
                }
            }

            if(bCurPageValid)
            {
                OUString aLayerName( GetLayerTabControl()->GetLayerName(nCurPage));
                if (!aLayerName.isEmpty())
                {
                    mpDrawView->SetActiveLayer(aLayerName);
                }
                Invalidate();
            }

            rReq.Done ();

            break;
        }

        case SID_PAGEMODE:  // BASIC
        {

            const SfxItemSet *pArgs = rReq.GetArgs();

            if (pArgs && pArgs->Count () == 2)
            {
                const SfxBoolItem* pIsActive = rReq.GetArg<SfxBoolItem>(ID_VAL_ISACTIVE);
                const SfxUInt32Item* pWhatKind = rReq.GetArg<SfxUInt32Item>(ID_VAL_WHATKIND);

                PageKind nWhatKind = static_cast<PageKind>(pWhatKind->GetValue());
                if ( nWhatKind >= PageKind::Standard && nWhatKind <= PageKind::Handout)
                {
                    mbIsLayerModeActive = pIsActive->GetValue();
                    mePageKind = nWhatKind;
                }
            }

            // turn on default layer of page
            mpDrawView->SetActiveLayer(sUNO_LayerName_layout);

            ChangeEditMode(EditMode::Page, mbIsLayerModeActive);

            Invalidate();
            rReq.Done ();

            break;
        }

        case SID_LAYERMODE:  // BASIC
        {
            const SfxItemSet *pArgs = rReq.GetArgs();

            if (pArgs && pArgs->Count() == 2)
            {
                const SfxUInt32Item* pWhatLayer = rReq.GetArg<SfxUInt32Item>(ID_VAL_WHATLAYER);
                EditMode nWhatLayer = static_cast<EditMode>(pWhatLayer->GetValue());
                if (nWhatLayer == EditMode::Page || nWhatLayer == EditMode::MasterPage)
                {
                    mbIsLayerModeActive = rReq.GetArg<SfxBoolItem>(ID_VAL_ISACTIVE)->GetValue();
                    meEditMode = nWhatLayer;
                }
            }

            ChangeEditMode(meEditMode, !mbIsLayerModeActive);

            Invalidate();
            rReq.Done();

            break;
        }

        case SID_HEADER_AND_FOOTER:
        case SID_INSERT_PAGE_NUMBER:
        case SID_INSERT_DATE_TIME:
        {
            SdAbstractDialogFactory* pFact = SdAbstractDialogFactory::Create();
            VclPtr<AbstractHeaderFooterDialog> pDlg(pFact->CreateHeaderFooterDialog( this, GetActiveWindow(), GetDoc(), mpActualPage ));
            std::shared_ptr<SfxRequest> xRequest(new SfxRequest(rReq));
            rReq.Ignore(); // the 'old' request is not relevant any more
            pDlg->StartExecuteAsync([this, xRequest](sal_Int32 /*nResult*/){
                GetActiveWindow()->Invalidate();
                UpdatePreview( mpActualPage );

                Invalidate();
                xRequest->Done();
            });
            break;
        }

        case SID_MASTER_LAYOUTS:
        {
            SdPage* pPage = GetActualPage();
            if (meEditMode == EditMode::MasterPage)
                // Use the master page of the current page.
                pPage = static_cast<SdPage*>(&pPage->TRG_GetMasterPage());

            SdAbstractDialogFactory* pFact = SdAbstractDialogFactory::Create();
            vcl::Window* pWin = GetActiveWindow();
            ScopedVclPtr<VclAbstractDialog> pDlg(pFact->CreateMasterLayoutDialog(pWin ? pWin->GetFrameWeld() : nullptr, GetDoc(), pPage));
            pDlg->Execute();
            Invalidate();
            rReq.Done ();
            break;
        }
        case SID_OBJECTRESIZE:
        {
            // The server likes to change the client size
            OSL_ASSERT (GetViewShell()!=nullptr);
            SfxInPlaceClient* pIPClient = GetViewShell()->GetIPClient();

            if ( pIPClient && pIPClient->IsObjectInPlaceActive() )
            {
                const SfxRectangleItem& rRect =
                    rReq.GetArgs()->Get(SID_OBJECTRESIZE);
                ::tools::Rectangle aRect( GetActiveWindow()->PixelToLogic( rRect.GetValue() ) );

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
                    Svx3DWin* p3DWin = static_cast<Svx3DWin*>(pWindow->GetWindow());
                    if(p3DWin)
                        p3DWin->DocumentReload();
                }

                // normal forwarding to ViewFrame for execution
                GetViewFrame()->ExecuteSlot(rReq);

                // From here on we must cope with this object and the frame already being
                // deleted.  Do not call any methods or use data members.
                Reference<XController> xController( xFrame->getController(), UNO_SET_THROW );

                // Restore the configuration.
                xControllerManager.set( xController, UNO_QUERY_THROW );
                xConfigurationController.set( xControllerManager->getConfigurationController() );
                if ( ! xConfigurationController.is())
                    throw RuntimeException();
                xConfigurationController->restoreConfiguration(xConfiguration);
            }
            catch (RuntimeException&)
            {
                DBG_UNHANDLED_EXCEPTION("sd.view");
            }

            // We have to return immediately to avoid accessing this object.
            return;
        }

        case SID_JUMPTOMARK:
        {
            if( rReq.GetArgs() )
            {
                const SfxStringItem* pBookmark = rReq.GetArg<SfxStringItem>(SID_JUMPTOMARK);

                if (pBookmark)
                {
                    OUString sBookmark(INetURLObject::decode(pBookmark->GetValue(), INetURLObject::DecodeMechanism::WithCharset));

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
            if (pFormShell != nullptr)
            {
                const SfxPoolItem* pItem;
                if (rReq.GetArgs()->GetItemState(
                    SID_ATTR_YEAR2000, true, &pItem) == SfxItemState::SET)
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
        break;

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

    switch ( rReq.GetSlot() )
    {
        case SID_ATTR_LONG_LRSPACE:
            if (pArgs)
            {
                std::unique_ptr<SdUndoGroup> pUndoGroup(new SdUndoGroup(GetDoc()));
                pUndoGroup->SetComment(SdResId(STR_UNDO_CHANGE_PAGEBORDER));

                const SvxLongLRSpaceItem& rLRSpace = static_cast<const SvxLongLRSpaceItem&>(
                        pArgs->Get(GetPool().GetWhich(SID_ATTR_LONG_LRSPACE)));

                if( mpDrawView->IsTextEdit() )
                {
                    ::tools::Rectangle aRect = maMarkRect;
                    aRect.SetPos(aRect.TopLeft() + aPagePos);
                    aRect.SetLeft( rLRSpace.GetLeft() );
                    aRect.SetRight( aViewSize.Width() - rLRSpace.GetRight() );
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
                                                pPage->GetLeftBorder(),
                                                pPage->GetRightBorder(),
                                                nLeft, nRight);
                        pUndoGroup->AddAction(pUndo);
                        pPage->SetLeftBorder(nLeft);
                        pPage->SetRightBorder(nRight);
                    }
                    nPageCnt = GetDoc()->GetMasterSdPageCount(mePageKind);

                    for (i = 0; i < nPageCnt; i++)
                    {
                        SdPage* pPage = GetDoc()->GetMasterSdPage(i, mePageKind);
                        SdUndoAction* pUndo = new SdPageLRUndoAction(GetDoc(),
                                                pPage,
                                                pPage->GetLeftBorder(),
                                                pPage->GetRightBorder(),
                                                nLeft, nRight);
                        pUndoGroup->AddAction(pUndo);
                        pPage->SetLeftBorder(nLeft);
                        pPage->SetRightBorder(nRight);
                    }
                    InvalidateWindows();
                }

                // give the undo group to the undo manager
                GetViewFrame()->GetObjectShell()->GetUndoManager()->
                                                    AddUndoAction(std::move(pUndoGroup));
            }
            break;
        case SID_ATTR_LONG_ULSPACE:
            if (pArgs)
            {
                std::unique_ptr<SdUndoGroup> pUndoGroup(new SdUndoGroup(GetDoc()));
                pUndoGroup->SetComment(SdResId(STR_UNDO_CHANGE_PAGEBORDER));

                const SvxLongULSpaceItem& rULSpace = static_cast<const SvxLongULSpaceItem&>(
                        pArgs->Get(GetPool().GetWhich(SID_ATTR_LONG_ULSPACE)));

                if( mpDrawView->IsTextEdit() )
                {
                    ::tools::Rectangle aRect = maMarkRect;
                    aRect.SetPos(aRect.TopLeft() + aPagePos);
                    aRect.SetTop( rULSpace.GetUpper() );
                    aRect.SetBottom( aViewSize.Height() - rULSpace.GetLower() );
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
                                                pPage->GetUpperBorder(),
                                                pPage->GetLowerBorder(),
                                                nUpper, nLower);
                        pUndoGroup->AddAction(pUndo);
                        pPage->SetUpperBorder(nUpper);
                        pPage->SetLowerBorder(nLower);
                    }
                    nPageCnt = GetDoc()->GetMasterSdPageCount(mePageKind);

                    for (i = 0; i < nPageCnt; i++)
                    {
                        SdPage* pPage = GetDoc()->GetMasterSdPage(i, mePageKind);
                        SdUndoAction* pUndo = new SdPageULUndoAction(GetDoc(),
                                                pPage,
                                                pPage->GetUpperBorder(),
                                                pPage->GetLowerBorder(),
                                                nUpper, nLower);
                        pUndoGroup->AddAction(pUndo);
                        pPage->SetUpperBorder(nUpper);
                        pPage->SetLowerBorder(nLower);
                    }
                    InvalidateWindows();
                }

                // give the undo group to the undo manager
                GetViewFrame()->GetObjectShell()->GetUndoManager()->
                                                    AddUndoAction(std::move(pUndoGroup));
            }
            break;
        case SID_RULER_OBJECT:
            if (pArgs)
            {
                ::tools::Rectangle aRect = maMarkRect;
                aRect.SetPos(aRect.TopLeft() + aPagePos);

                const SvxObjectItem& rOI = static_cast<const SvxObjectItem&>(
                        pArgs->Get(GetPool().GetWhich(SID_RULER_OBJECT)));

                if ( rOI.GetStartX() != rOI.GetEndX() )
                {
                    aRect.SetLeft( rOI.GetStartX() );
                    aRect.SetRight( rOI.GetEndX() );
                }
                if ( rOI.GetStartY() != rOI.GetEndY() )
                {
                    aRect.SetTop( rOI.GetStartY() );
                    aRect.SetBottom( rOI.GetEndY() );
                }
                aRect.SetPos(aRect.TopLeft() - aPagePos);
                if ( aRect != maMarkRect)
                {
                    mpDrawView->SetAllMarkedRect(aRect);
                    maMarkRect = mpDrawView->GetAllMarkedRect();
                    Invalidate( SID_RULER_OBJECT );
                }
            }
            break;
        case SID_ATTR_TABSTOP:
            if (pArgs && mpDrawView->IsTextEdit())
            {
                const SvxTabStopItem& rItem = pArgs->Get( EE_PARA_TABS );

                SfxItemSet aEditAttr( GetPool(), svl::Items<EE_PARA_TABS, EE_PARA_TABS>{} );

                aEditAttr.Put( rItem );
                mpDrawView->SetAttributes( aEditAttr );

                Invalidate(SID_ATTR_TABSTOP);
            }
            break;
        case SID_ATTR_PARA_LINESPACE:
            if (pArgs)
            {
                SvxLineSpacingItem aParaLineSP = static_cast<const SvxLineSpacingItem&>(pArgs->Get(
                    GetPool().GetWhich(SID_ATTR_PARA_LINESPACE)));

                SfxItemSet aEditAttr( GetPool(), svl::Items<EE_PARA_SBL, EE_PARA_SBL>{} );
                aParaLineSP.SetWhich( EE_PARA_SBL );

                aEditAttr.Put( aParaLineSP );
                mpDrawView->SetAttributes( aEditAttr );

                Invalidate(SID_ATTR_PARA_LINESPACE);
            }
            break;
        case SID_ATTR_PARA_ADJUST_LEFT:
        {
            SvxAdjustItem aItem( SvxAdjust::Left, EE_PARA_JUST );
            SfxItemSet aEditAttr( GetPool(), svl::Items<EE_PARA_JUST, EE_PARA_JUST>{} );

            aEditAttr.Put( aItem );
            mpDrawView->SetAttributes( aEditAttr );

            Invalidate(SID_ATTR_PARA_ADJUST_LEFT);
            break;
        }
        case SID_ATTR_PARA_ADJUST_CENTER:
        {
            SvxAdjustItem aItem( SvxAdjust::Center, EE_PARA_JUST );
            SfxItemSet aEditAttr( GetPool(), svl::Items<EE_PARA_JUST, EE_PARA_JUST>{} );

            aEditAttr.Put( aItem );
            mpDrawView->SetAttributes( aEditAttr );

            Invalidate(SID_ATTR_PARA_ADJUST_CENTER);
            break;
        }
        case SID_ATTR_PARA_ADJUST_RIGHT:
        {
            SvxAdjustItem aItem( SvxAdjust::Right, EE_PARA_JUST );
            SfxItemSet aEditAttr( GetPool(), svl::Items<EE_PARA_JUST, EE_PARA_JUST>{} );

            aEditAttr.Put( aItem );
            mpDrawView->SetAttributes( aEditAttr );

            Invalidate(SID_ATTR_PARA_ADJUST_RIGHT);
            break;
        }
        case SID_ATTR_PARA_ADJUST_BLOCK:
        {
            SvxAdjustItem aItem( SvxAdjust::Block, EE_PARA_JUST );
            SfxItemSet aEditAttr( GetPool(), svl::Items<EE_PARA_JUST, EE_PARA_JUST>{} );

            aEditAttr.Put( aItem );
            mpDrawView->SetAttributes( aEditAttr );

            Invalidate(SID_ATTR_PARA_ADJUST_BLOCK);
            break;
        }
        case SID_ATTR_PARA_ULSPACE:
            if (pArgs)
            {
                SvxULSpaceItem aULSP = static_cast<const SvxULSpaceItem&>(pArgs->Get(
                    GetPool().GetWhich(SID_ATTR_PARA_ULSPACE)));
                SfxItemSet aEditAttr( GetPool(), svl::Items<EE_PARA_ULSPACE, EE_PARA_ULSPACE>{} );
                aULSP.SetWhich( EE_PARA_ULSPACE );

                aEditAttr.Put( aULSP );
                mpDrawView->SetAttributes( aEditAttr );

                Invalidate(SID_ATTR_PARA_ULSPACE);
            }
            break;
        case SID_ATTR_PARA_LRSPACE:
            if (pArgs)
            {
                SvxLRSpaceItem aLRSpace = static_cast<const SvxLRSpaceItem&>(pArgs->Get(
                    GetPool().GetWhich(SID_ATTR_PARA_LRSPACE)));

                SfxItemSet aEditAttr( GetPool(), svl::Items<EE_PARA_LRSPACE, EE_PARA_LRSPACE>{} );
                aLRSpace.SetWhich( EE_PARA_LRSPACE );

                aEditAttr.Put( aLRSpace );
                mpDrawView->SetAttributes( aEditAttr );

                Invalidate(SID_ATTR_PARA_LRSPACE);
            }
            break;
        case SID_ATTR_LRSPACE:
            if (pArgs && mpDrawView->IsTextEdit())
            {
                sal_uInt16 nId = SID_ATTR_PARA_LRSPACE;
                const SvxLRSpaceItem& rItem = static_cast<const SvxLRSpaceItem&>(
                            pArgs->Get( nId ));

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
                        rItem.GetRight(), rItem.GetTextLeft(),
                        rItem.GetTextFirstLineOfst(), nId );

                const sal_Int16 nOutlineLevel = aEditAttr.Get( EE_PARA_OUTLLEVEL ).GetValue();
                const SvxLRSpaceItem& rOrigLRSpaceItem = aEditAttr.Get( EE_PARA_LRSPACE );
                const SvxNumBulletItem& rNumBulletItem = aEditAttr.Get( EE_PARA_NUMBULLET );
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
                    const auto nAbsLSpace=aFormat.GetAbsLSpace();
                    const long  nTxtLeft=rItem.GetTextLeft();
                    const long  nLeftIndent=std::max(0L,nTxtLeft - nAbsLSpace);
                    aLRSpaceItem.SetTextLeft(nLeftIndent);
                    // control for clipped left indent - remainder
                    // reduces number format first line indent
                    aFormat.SetAbsLSpace(nTxtLeft - nLeftIndent);

                    // negative first line indent goes to the number
                    // format, positive to the lrSpace item
                    if( rItem.GetTextFirstLineOfst() < 0 )
                    {
                        aFormat.SetFirstLineOffset(
                            rItem.GetTextFirstLineOfst()
                            - rOrigLRSpaceItem.GetTextFirstLineOfst()
                            + aFormat.GetCharTextDistance());
                        aLRSpaceItem.SetTextFirstLineOfst(0);
                    }
                    else
                    {
                        aFormat.SetFirstLineOffset(0);
                        aLRSpaceItem.SetTextFirstLineOfst(
                            rItem.GetTextFirstLineOfst()
                            - aFormat.GetFirstLineOffset() //TODO: overflow
                            + aFormat.GetCharTextDistance());
                    }

                    if( rFormat != aFormat )
                    {
                        // put all items
                        rNumBulletItem.GetNumRule()->SetLevel(nOutlineLevel,aFormat);
                        aEditAttr.Put( rNumBulletItem );
                        aEditAttr.Put( aLRSpaceItem );
                        mpDrawView->SetAttributes( aEditAttr );

                        Invalidate(SID_ATTR_PARA_LRSPACE);
                        break;
                    }
                }

                // only put lrSpace item
                SfxItemSet aEditAttrReduced( GetDoc()->GetPool(),
                                             svl::Items<EE_PARA_LRSPACE, EE_PARA_LRSPACE>{} );
                aEditAttrReduced.Put( aLRSpaceItem );
                mpDrawView->SetAttributes( aEditAttrReduced );

                Invalidate(SID_ATTR_PARA_LRSPACE);
            }
            break;
    }
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

    ::tools::Rectangle aRect(aPagePos, Point( aViewSize.Width() - (aPagePos.X() + aPageSize.Width()),
                                     aViewSize.Height() - (aPagePos.Y() + aPageSize.Height())));

    if( mpDrawView->IsTextEdit() )
    {
        Point aPnt1 = GetActiveWindow()->GetWinViewPos();
        ::tools::Rectangle aMinMaxRect = ::tools::Rectangle( aPnt1, Size(ULONG_MAX, ULONG_MAX) );
        rSet.Put( SfxRectangleItem(SID_RULER_LR_MIN_MAX, aMinMaxRect) );
    }
    else
    {
        rSet.Put( SfxRectangleItem(SID_RULER_LR_MIN_MAX, aRect) );
    }

    SvxLongLRSpaceItem aLRSpace(aPagePos.X() + mpActualPage->GetLeftBorder(),
                                aRect.Right() + mpActualPage->GetRightBorder(),
                                GetPool().GetWhich(SID_ATTR_LONG_LRSPACE));
    SvxLongULSpaceItem aULSpace(aPagePos.Y() + mpActualPage->GetUpperBorder(),
                                aRect.Bottom() + mpActualPage->GetLowerBorder(),
                                GetPool().GetWhich(SID_ATTR_LONG_ULSPACE));
    rSet.Put(SvxPagePosSizeItem(Point(0,0) - aPagePos, aViewSize.Width(),
                                                       aViewSize.Height()));
    SfxPointItem aPointItem( SID_RULER_NULL_OFFSET, aPagePos + aOrigin );

    SvxProtectItem aProtect( SID_RULER_PROTECT );

    maMarkRect = mpDrawView->GetAllMarkedRect();

    const bool bRTL = GetDoc() && GetDoc()->GetDefaultWritingMode() == css::text::WritingMode_RL_TB;
    rSet.Put(SfxBoolItem(SID_RULER_TEXT_RIGHT_TO_LEFT, bRTL));

    if( mpDrawView->AreObjectsMarked() )
    {
        if( mpDrawView->IsTextEdit() )
        {
            SdrObject* pObj = mpDrawView->GetMarkedObjectList().GetMark( 0 )->GetMarkedSdrObj();
            if( pObj->GetObjInventor() == SdrInventor::Default)
            {
                SfxItemSet aEditAttr( GetDoc()->GetPool() );
                mpDrawView->GetAttributes( aEditAttr );
                if( aEditAttr.GetItemState( EE_PARA_TABS ) >= SfxItemState::DEFAULT )
                {
                    const SvxTabStopItem& rItem = aEditAttr.Get( EE_PARA_TABS );
                    rSet.Put( rItem );

                    const SvxLRSpaceItem& rLRSpaceItem = aEditAttr.Get( EE_PARA_LRSPACE );
                    SvxLRSpaceItem aLRSpaceItem( rLRSpaceItem.GetLeft(),
                            rLRSpaceItem.GetRight(), rLRSpaceItem.GetTextLeft(),
                            rLRSpaceItem.GetTextFirstLineOfst(), SID_ATTR_PARA_LRSPACE );

                    const sal_Int16 nOutlineLevel = aEditAttr.Get( EE_PARA_OUTLLEVEL ).GetValue();
                    const SvxNumBulletItem& rNumBulletItem = aEditAttr.Get( EE_PARA_NUMBULLET );
                    if( nOutlineLevel != -1 &&
                        rNumBulletItem.GetNumRule() &&
                        rNumBulletItem.GetNumRule()->GetLevelCount() > nOutlineLevel )
                    {
                        const SvxNumberFormat& rFormat = rNumBulletItem.GetNumRule()->GetLevel(nOutlineLevel);
                        aLRSpaceItem.SetTextLeft(rFormat.GetAbsLSpace() + rLRSpaceItem.GetTextLeft());
                        aLRSpaceItem.SetTextFirstLineOfst(
                            rLRSpaceItem.GetTextFirstLineOfst() + rFormat.GetFirstLineOffset()
                                //TODO: overflow
                            - rFormat.GetCharTextDistance());
                    }

                    rSet.Put( aLRSpaceItem );

                    Point aPos( aPagePos + maMarkRect.TopLeft() );

                    if ( aEditAttr.GetItemState( SDRATTR_TEXT_LEFTDIST ) == SfxItemState::SET )
                    {
                        const SdrMetricItem& rTLDItem = aEditAttr.Get( SDRATTR_TEXT_LEFTDIST );
                        long nLD = rTLDItem.GetValue();
                        aPos.AdjustX(nLD );
                    }

                    aPointItem.SetValue( aPos );

                    ::tools::Rectangle aParaRect(maMarkRect);
                    if (pObj->GetObjIdentifier() == OBJ_TABLE)
                    {
                        sdr::table::SdrTableObj* pTable = static_cast<sdr::table::SdrTableObj*>(pObj);
                        sdr::table::CellPos cellpos;
                        pTable->getActiveCellPos(cellpos);
                        pTable->getCellBounds(cellpos, aParaRect);
                    }

                    aLRSpace.SetLeft(aPagePos.X() + aParaRect.Left());

                    if ( aEditAttr.GetItemState( SDRATTR_TEXT_LEFTDIST ) == SfxItemState::SET )
                    {
                        const SdrMetricItem& rTLDItem = aEditAttr.Get( SDRATTR_TEXT_LEFTDIST );
                        long nLD = rTLDItem.GetValue();
                        aLRSpace.SetLeft( aLRSpace.GetLeft() + nLD );
                    }

                    aLRSpace.SetRight(aRect.Right() + aPageSize.Width() - aParaRect.Right());

                    if ( aEditAttr.GetItemState( SDRATTR_TEXT_RIGHTDIST ) == SfxItemState::SET )
                    {
                        const SdrMetricItem& rTRDItem = aEditAttr.Get( SDRATTR_TEXT_RIGHTDIST );
                        long nRD = rTRDItem.GetValue();
                        aLRSpace.SetRight( aLRSpace.GetRight() + nRD );
                    }

                    aULSpace.SetUpper( aPagePos.Y() + maMarkRect.Top() );
                    aULSpace.SetLower( aRect.Bottom() + aPageSize.Height() - maMarkRect.Bottom() );

                    rSet.DisableItem( SID_RULER_OBJECT );

                    // lock page margins
                    aProtect.SetSizeProtect( true );
                    aProtect.SetPosProtect( true );
                }

                if( aEditAttr.GetItemState( EE_PARA_WRITINGDIR ) >= SfxItemState::DEFAULT )
                {
                    const SvxFrameDirectionItem& rItem = aEditAttr.Get( EE_PARA_WRITINGDIR );
                    rSet.Put(SfxBoolItem(SID_RULER_TEXT_RIGHT_TO_LEFT, rItem.GetValue() == SvxFrameDirection::Horizontal_RL_TB));
                }
            }
        }
        else
        {
            rSet.DisableItem( EE_PARA_TABS );
            rSet.DisableItem( SID_RULER_TEXT_RIGHT_TO_LEFT );

            if( mpDrawView->IsResizeAllowed(true) )
            {
                ::tools::Rectangle aResizeRect( maMarkRect );

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
            GetViewFrame()->GetDispatcher()->Execute( SID_ATTR_TRANSFORM, SfxCallMode::ASYNCHRON );
        }
        break;

        case SID_STATUS_LAYOUT:
        {
            GetViewFrame()->GetDispatcher()->Execute( SID_PRESENTATION_LAYOUT, SfxCallMode::ASYNCHRON );
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
    sal_uInt16  nHitLog = static_cast<sal_uInt16>(GetActiveWindow()->PixelToLogic(
        Size(FuPoor::HITPIX,0)).Width());
    sal_uInt16  nHelpLine;

    if ( !mpDrawView->PickHelpLine(aMPos, nHitLog, *GetActiveWindow(), nHelpLine, pPV) )
        return;

    const SdrHelpLine& rHelpLine = (pPV->GetHelpLines())[nHelpLine];

    if ( rHelpLine.GetKind() == SdrHelpLineKind::Point )
    {
        rSet.Put( SfxStringItem( SID_SET_SNAPITEM,
                            SdResId( STR_POPUP_EDIT_SNAPPOINT)) );
        rSet.Put( SfxStringItem( SID_DELETE_SNAPITEM,
                            SdResId( STR_POPUP_DELETE_SNAPPOINT)) );
    }
    else
    {
        rSet.Put( SfxStringItem( SID_SET_SNAPITEM,
                            SdResId( STR_POPUP_EDIT_SNAPLINE)) );
        rSet.Put( SfxStringItem( SID_DELETE_SNAPITEM,
                            SdResId( STR_POPUP_DELETE_SNAPLINE)) );
    }
}

} // end of namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
