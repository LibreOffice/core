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

#include <avmedia/mediaplayer.hxx>

#include <basic/sberrors.hxx>
#include <basic/sbstar.hxx>

#include <com/sun/star/drawing/XMasterPagesSupplier.hpp>
#include <com/sun/star/drawing/XDrawPages.hpp>
#include <com/sun/star/ui/dialogs/XExecutableDialog.hpp>
#include <com/sun/star/ui/dialogs/XSLTFilterDialog.hpp>
#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/util/URLTransformer.hpp>
#include <com/sun/star/util/XURLTransformer.hpp>

#include <comphelper/processfactory.hxx>

#include <editeng/editdata.hxx>
#include <editeng/eeitem.hxx>
#include <editeng/flditem.hxx>
#include <editeng/editeng.hxx>
#include <o3tl/make_unique.hxx>

#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/msgpool.hxx>
#include <sfx2/request.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/zoomitem.hxx>

#include <svl/aeitem.hxx>

#include <svx/SpellDialogChildWindow.hxx>
#include <svx/compressgraphicdialog.hxx>
#include <svx/dialogs.hrc>
#include <svx/bmpmask.hxx>
#include <svx/colrctrl.hxx>
#include <svx/extedit.hxx>
#include <svx/extrusionbar.hxx>
#include <svx/f3dchild.hxx>
#include <svx/fontwork.hxx>
#include <svx/fontworkbar.hxx>
#include <svx/graphichelper.hxx>
#include <svx/hlnkitem.hxx>
#include <svx/imapdlg.hxx>
#include <svx/sdtagitm.hxx>
#include <svx/sdtmfitm.hxx>
#include <svx/svdoattr.hxx>
#include <svx/svdograf.hxx>
#include <svx/svdoole2.hxx>
#include <svx/svdoutl.hxx>
#include <svx/svdpagv.hxx>
#include <svx/svdundo.hxx>
#include <svx/svxdlg.hxx>
#include <svx/svxids.hrc>
#include <svx/xfillit0.hxx>
#include <svx/xflclit.hxx>
#include <svx/xlineit0.hxx>
#include <svx/xlnedwit.hxx>
#include <svx/xlnstwit.hxx>
#include <svx/xlnwtit.hxx>

#include <tools/diagnose_ex.h>

#include <unotools/useroptions.hxx>

#include <vcl/graph.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/svapp.hxx>
#include <vcl/waitobj.hxx>

#include <editeng/cmapitem.hxx>
#include <editeng/escapementitem.hxx>
#include <editeng/kernitem.hxx>
#include <editeng/wghtitem.hxx>
#include <editeng/postitem.hxx>
#include <editeng/udlnitem.hxx>
#include <editeng/crossedoutitem.hxx>
#include <editeng/contouritem.hxx>
#include <editeng/shdditem.hxx>
#include <svx/xtable.hxx>
#include <svx/svdobj.hxx>
#include <svx/SvxColorChildWindow.hxx>
#include <editeng/outlobj.hxx>
#include <editeng/flstitem.hxx>
#include <editeng/scripttypeitem.hxx>
#include <editeng/fontitem.hxx>
#include <editeng/fhgtitem.hxx>
#include <editeng/colritem.hxx>
#include <editeng/brushitem.hxx>

#include <svl/whiter.hxx>

#include "app.hrc"
#include "glob.hrc"
#include "strings.hrc"

#include "framework/FrameworkHelper.hxx"

#include "AnimationChildWindow.hxx"
#include "DrawDocShell.hxx"
#include "DrawViewShell.hxx"
#include "GraphicViewShell.hxx"
#include "LayerTabBar.hxx"
#include "Outliner.hxx"
#include "ViewShellHint.hxx"
#include "ViewShellImplementation.hxx"
#include "Window.hxx"
#include "drawdoc.hxx"
#include "drawview.hxx"
#include "fuarea.hxx"
#include "fubullet.hxx"
#include "fuchar.hxx"
#include "fucushow.hxx"
#include "fuconnct.hxx"
#include "fucopy.hxx"
#include "fudspord.hxx"
#include "fuexpand.hxx"
#include "fuinsert.hxx"
#include "fuinsfil.hxx"
#include "fuline.hxx"
#include "fulinend.hxx"
#include "fulink.hxx"
#include "fumeasur.hxx"
#include "fumorph.hxx"
#include "fuoaprms.hxx"
#include "fuolbull.hxx"
#include "fupage.hxx"
#include "fuparagr.hxx"
#include "fuprlout.hxx"
#include "fuscale.hxx"
#include "fusel.hxx"
#include "fusldlg.hxx"
#include "fusnapln.hxx"
#include "fusumry.hxx"
#include "futempl.hxx"
#include "futhes.hxx"
#include "futransf.hxx"
#include "futxtatt.hxx"
#include "fuvect.hxx"
#include "futext.hxx"
#include "helpids.h"
#include "optsitem.hxx"
#include "sdabstdlg.hxx"
#include "sdattr.hxx"
#include "sdgrffilter.hxx"
#include "sdpage.hxx"
#include "sdresid.hxx"
#include "slideshow.hxx"
#include "stlpool.hxx"
#include "undolayer.hxx"
#include "unmodpg.hxx"
#include <sfx2/sidebar/Sidebar.hxx>
#include <sfx2/classificationhelper.hxx>

#include "ViewShellBase.hxx"
#include <memory>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

#define MIN_ACTIONS_FOR_DIALOG  5000    ///< if there are more meta objects, we show a dialog during the break up

namespace sd {

/**
 * SfxRequests for temporary actions
 */

void DrawViewShell::FuTemporary(SfxRequest& rReq)
{
    // during a native slide show nothing gets executed!
    if(SlideShow::IsRunning( GetViewShellBase() ) && (rReq.GetSlot() != SID_NAVIGATOR))
        return;

    DBG_ASSERT( mpDrawView, "sd::DrawViewShell::FuTemporary(), no draw view!" );
    if( !mpDrawView )
        return;

    CheckLineTo (rReq);

    DeactivateCurrentFunction();

    sal_uInt16 nSId = rReq.GetSlot();

    switch ( nSId )
    {
        case SID_OUTLINE_TEXT_AUTOFIT:
        {
            ::svl::IUndoManager* pUndoManager = GetDocSh()->GetUndoManager();
            const SdrMarkList& rMarkList = mpDrawView->GetMarkedObjectList();
            if( rMarkList.GetMarkCount() == 1 )
            {
                pUndoManager->EnterListAction("", "");
                mpDrawView->BegUndo();

                SdrObject* pObj = rMarkList.GetMark(0)->GetMarkedSdrObj();
                bool bSet = static_cast<const SdrTextFitToSizeTypeItem*>(pObj->GetMergedItemSet().GetItem(SDRATTR_TEXT_FITTOSIZE))->GetValue() != SDRTEXTFIT_NONE;

                mpDrawView->AddUndo(GetDoc()->GetSdrUndoFactory().CreateUndoAttrObject(*pObj));

                if (!bSet)
                {
                    //If we are turning on AutoFit we have to turn these off if already on
                    if (static_cast<const SdrOnOffItem*>(pObj->GetMergedItemSet().GetItem(SDRATTR_TEXT_AUTOGROWHEIGHT))->GetValue())
                        pObj->SetMergedItem(makeSdrTextAutoGrowHeightItem(false));
                    if (static_cast<const SdrOnOffItem*>(pObj->GetMergedItemSet().GetItem(SDRATTR_TEXT_AUTOGROWWIDTH))->GetValue())
                        pObj->SetMergedItem(makeSdrTextAutoGrowWidthItem(false));
                }

                pObj->SetMergedItem(SdrTextFitToSizeTypeItem(bSet ? SDRTEXTFIT_NONE : SDRTEXTFIT_AUTOFIT));

                mpDrawView->EndUndo();
                pUndoManager->LeaveListAction();
            }
            Cancel();
            rReq.Done();
        }
        break;

        // area and line attributes: shall have
        // an own Execute method (like StateMethode)
        case SID_ATTR_FILL_STYLE:
        case SID_ATTR_FILL_COLOR:
        case SID_ATTR_FILL_GRADIENT:
        case SID_ATTR_FILL_HATCH:
        case SID_ATTR_FILL_BITMAP:
        case SID_ATTR_FILL_SHADOW:
        case SID_ATTR_SHADOW_COLOR:
        case SID_ATTR_SHADOW_TRANSPARENCE:
        case SID_ATTR_SHADOW_XDISTANCE:
        case SID_ATTR_SHADOW_YDISTANCE:
        case SID_ATTR_FILL_TRANSPARENCE:
        case SID_ATTR_FILL_FLOATTRANSPARENCE:

        case SID_ATTR_LINE_STYLE:
        case SID_ATTR_LINE_DASH:
        case SID_ATTR_LINE_WIDTH:
        case SID_ATTR_LINE_COLOR:
        case SID_ATTR_LINEEND_STYLE:
        case SID_ATTR_LINE_START:
        case SID_ATTR_LINE_END:
        case SID_ATTR_LINE_TRANSPARENCE:
        case SID_ATTR_LINE_JOINT:
        case SID_ATTR_LINE_CAP:

        case SID_ATTR_TEXT_FITTOSIZE:
        {
            if( rReq.GetArgs() )
            {
                mpDrawView->SetAttributes(*rReq.GetArgs());
                rReq.Done();
            }
            else
            {
                switch( rReq.GetSlot() )
                {
                    case SID_ATTR_FILL_SHADOW:
                    case SID_ATTR_SHADOW_COLOR:
                    case SID_ATTR_SHADOW_TRANSPARENCE:
                    case SID_ATTR_SHADOW_XDISTANCE:
                    case SID_ATTR_SHADOW_YDISTANCE:
                    case SID_ATTR_FILL_STYLE:
                    case SID_ATTR_FILL_COLOR:
                    case SID_ATTR_FILL_GRADIENT:
                    case SID_ATTR_FILL_HATCH:
                    case SID_ATTR_FILL_BITMAP:
                    case SID_ATTR_FILL_TRANSPARENCE:
                    case SID_ATTR_FILL_FLOATTRANSPARENCE:
                        GetViewFrame()->GetDispatcher()->Execute( SID_ATTRIBUTES_AREA, SfxCallMode::ASYNCHRON );
                        break;
                    case SID_ATTR_LINE_STYLE:
                    case SID_ATTR_LINE_DASH:
                    case SID_ATTR_LINE_WIDTH:
                    case SID_ATTR_LINE_COLOR:
                    case SID_ATTR_LINE_TRANSPARENCE:
                    case SID_ATTR_LINE_JOINT:
                    case SID_ATTR_LINE_CAP:
                        GetViewFrame()->GetDispatcher()->Execute( SID_ATTRIBUTES_LINE, SfxCallMode::ASYNCHRON );
                        break;
                    case SID_ATTR_TEXT_FITTOSIZE:
                        GetViewFrame()->GetDispatcher()->Execute( SID_TEXTATTR_DLG, SfxCallMode::ASYNCHRON );
                        break;
                }
            }
            Cancel();
        }
        break;

        case SID_HYPHENATION:
        {
            const SfxBoolItem* pItem = rReq.GetArg<SfxBoolItem>(SID_HYPHENATION);

            if( pItem )
            {
                SfxItemSet aSet( GetPool(), EE_PARA_HYPHENATE, EE_PARA_HYPHENATE );
                bool bValue = pItem->GetValue();
                aSet.Put( SfxBoolItem( EE_PARA_HYPHENATE, bValue ) );
                mpDrawView->SetAttributes( aSet );
            }
            else // only for testing purpose
            {
                OSL_FAIL(" no value for hyphenation!");
                SfxItemSet aSet( GetPool(), EE_PARA_HYPHENATE, EE_PARA_HYPHENATE );
                bool bValue = true;
                aSet.Put( SfxBoolItem( EE_PARA_HYPHENATE, bValue ) );
                mpDrawView->SetAttributes( aSet );
            }
            rReq.Done();
            Cancel();
        }
        break;

        case SID_INSERTPAGE:
        case SID_INSERTPAGE_QUICK:
        case SID_DUPLICATE_PAGE:
        {
            SdPage* pNewPage = CreateOrDuplicatePage (rReq, mePageKind, GetActualPage());
            Cancel();
            if(HasCurrentFunction(SID_BEZIER_EDIT) )
                GetViewFrame()->GetDispatcher()->Execute(SID_OBJECT_SELECT, SfxCallMode::ASYNCHRON);
            if (pNewPage != nullptr)
                SwitchPage((pNewPage->GetPageNum()-1)/2);
            rReq.Done ();
        }
        break;

        case SID_INSERT_MASTER_PAGE:
        {
            // Use the API to create a new page.
            Reference<drawing::XMasterPagesSupplier> xMasterPagesSupplier (
                GetDoc()->getUnoModel(), UNO_QUERY);
            if (xMasterPagesSupplier.is())
            {
                Reference<drawing::XDrawPages> xMasterPages (
                    xMasterPagesSupplier->getMasterPages());
                if (xMasterPages.is())
                {
                    sal_uInt16 nIndex = GetCurPageId();
                    xMasterPages->insertNewByIndex (nIndex);

                    // Create shapes for the default layout.
                    SdPage* pMasterPage = GetDoc()->GetMasterSdPage(
                        nIndex, PK_STANDARD);
                    pMasterPage->CreateTitleAndLayout (true,true);
                }
            }

            Cancel();
            if(HasCurrentFunction(SID_BEZIER_EDIT))
                GetViewFrame()->GetDispatcher()->Execute(
                    SID_OBJECT_SELECT, SfxCallMode::ASYNCHRON);
            rReq.Done ();
        }
        break;

        case SID_MODIFYPAGE:
        {
            if (mePageKind==PK_STANDARD || mePageKind==PK_NOTES ||
                (mePageKind==PK_HANDOUT && meEditMode==EM_MASTERPAGE) )
            {
                if ( mpDrawView->IsTextEdit() )
                {
                    mpDrawView->SdrEndTextEdit();
                }
                sal_uInt16 nPage = maTabControl->GetCurPageId() - 1;
                mpActualPage = GetDoc()->GetSdPage(nPage, mePageKind);
                ::sd::ViewShell::mpImpl->ProcessModifyPageSlot (
                    rReq,
                    mpActualPage,
                    mePageKind);
            }

            Cancel();
            rReq.Done ();
        }
        break;

        case SID_ASSIGN_LAYOUT:
        {
            if (mePageKind==PK_STANDARD || mePageKind==PK_NOTES || (mePageKind==PK_HANDOUT && meEditMode==EM_MASTERPAGE))
            {
                if ( mpDrawView->IsTextEdit() )
                    mpDrawView->SdrEndTextEdit();

                ::sd::ViewShell::mpImpl->AssignLayout(rReq, mePageKind);
            }
            Cancel();
            rReq.Done ();
        }
        break;

        case SID_RENAMEPAGE:
        case SID_RENAME_MASTER_PAGE:
        {
            if (mePageKind==PK_STANDARD || mePageKind==PK_NOTES )
            {
                if ( mpDrawView->IsTextEdit() )
                {
                    mpDrawView->SdrEndTextEdit();
                }

                sal_uInt16 nPageId = maTabControl->GetCurPageId();
                SdPage* pCurrentPage = ( GetEditMode() == EM_PAGE )
                    ? GetDoc()->GetSdPage( nPageId - 1, GetPageKind() )
                    : GetDoc()->GetMasterSdPage( nPageId - 1, GetPageKind() );

                OUString aTitle = SD_RESSTR(STR_TITLE_RENAMESLIDE);
                OUString aDescr = SD_RESSTR(STR_DESC_RENAMESLIDE);
                OUString aPageName = pCurrentPage->GetName();

                SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
                DBG_ASSERT(pFact, "Dialog creation failed!");
                std::unique_ptr<AbstractSvxNameDialog> aNameDlg(pFact->CreateSvxNameDialog( GetActiveWindow(), aPageName, aDescr ));
                DBG_ASSERT(aNameDlg, "Dialog creation failed!");
                aNameDlg->SetText( aTitle );
                aNameDlg->SetCheckNameHdl( LINK( this, DrawViewShell, RenameSlideHdl ), true );
                aNameDlg->SetEditHelpId( HID_SD_NAMEDIALOG_PAGE );

                if( aNameDlg->Execute() == RET_OK )
                {
                    OUString aNewName;
                    aNameDlg->GetName( aNewName );
                    if (aNewName != aPageName)
                    {
                        bool bResult = RenameSlide( nPageId, aNewName );
                        DBG_ASSERT( bResult, "Couldn't rename slide" );
                    }
                }
            }

            Cancel();
            rReq.Ignore ();
        }
        break;

        case SID_RENAMEPAGE_QUICK:
        {
            if (mePageKind==PK_STANDARD || mePageKind==PK_NOTES )
            {
                if ( mpDrawView->IsTextEdit() )
                {
                    mpDrawView->SdrEndTextEdit();
                }

                maTabControl->StartEditMode( maTabControl->GetCurPageId() );
            }

            Cancel();
            rReq.Ignore ();
        }
        break;

        case SID_PAGESIZE :  // either this (no menu entries or something else!)
        {
            const SfxItemSet *pArgs = rReq.GetArgs ();

            if (pArgs)
                if (pArgs->Count () == 3)
                {
                    const SfxUInt32Item* pWidth = rReq.GetArg<SfxUInt32Item>(ID_VAL_PAGEWIDTH);
                    const SfxUInt32Item* pHeight = rReq.GetArg<SfxUInt32Item>(ID_VAL_PAGEHEIGHT);
                    const SfxBoolItem* pScaleAll = rReq.GetArg<SfxBoolItem>(ID_VAL_SCALEOBJECTS);

                    Size aSize (pWidth->GetValue (), pHeight->GetValue ());

                    SetupPage (aSize, 0, 0, 0, 0, true, false, pScaleAll->GetValue ());
                    rReq.Ignore ();
                    break;
                }
#if HAVE_FEATURE_SCRIPTING
            StarBASIC::FatalError (ERRCODE_BASIC_WRONG_ARGS);
#endif
            rReq.Ignore ();
            break;
        }

        case SID_PAGEMARGIN :  // or this (no menu entries or something else!)
        {
            const SfxItemSet *pArgs = rReq.GetArgs ();

            if (pArgs)
                if (pArgs->Count () == 5)
                {
                    const SfxUInt32Item* pLeft = rReq.GetArg<SfxUInt32Item>(ID_VAL_PAGELEFT);
                    const SfxUInt32Item* pRight = rReq.GetArg<SfxUInt32Item>(ID_VAL_PAGERIGHT);
                    const SfxUInt32Item* pUpper = rReq.GetArg<SfxUInt32Item>(ID_VAL_PAGETOP);
                    const SfxUInt32Item* pLower = rReq.GetArg<SfxUInt32Item>(ID_VAL_PAGEBOTTOM);
                    const SfxBoolItem* pScaleAll = rReq.GetArg<SfxBoolItem>(ID_VAL_SCALEOBJECTS);

                    Size aEmptySize (0, 0);

                    SetupPage (aEmptySize, pLeft->GetValue (), pRight->GetValue (),
                               pUpper->GetValue (), pLower->GetValue (),
                               false, true, pScaleAll->GetValue ());
                    rReq.Ignore ();
                    break;
                }
#if HAVE_FEATURE_SCRIPTING
            StarBASIC::FatalError (ERRCODE_BASIC_WRONG_ARGS);
#endif
            rReq.Ignore ();
            break;
        }

        case SID_ATTR_ZOOMSLIDER:
        {
            const SfxItemSet* pArgs = rReq.GetArgs();

            if (pArgs && pArgs->Count () == 1 )
            {
                const SfxUInt16Item* pScale = rReq.GetArg<SfxUInt16Item>(SID_ATTR_ZOOMSLIDER);
                if (CHECK_RANGE (5, pScale->GetValue (), 3000))
                {
                    SetZoom (pScale->GetValue ());

                    SfxBindings& rBindings = GetViewFrame()->GetBindings();
                    rBindings.Invalidate( SID_ATTR_ZOOM );
                    rBindings.Invalidate( SID_ZOOM_IN );
                    rBindings.Invalidate( SID_ZOOM_OUT );
                    rBindings.Invalidate( SID_ATTR_ZOOMSLIDER );

                }
            }

            Cancel();
            rReq.Done ();
            break;
        }
        case SID_ZOOMING :  // no menu entry, but generated from zoom dialog
        {
            const SfxItemSet* pArgs = rReq.GetArgs();

            if (pArgs)
                if (pArgs->Count () == 1)
                {
                    const SfxUInt32Item* pScale = rReq.GetArg<SfxUInt32Item>(ID_VAL_ZOOM);
                    if (CHECK_RANGE (10, pScale->GetValue (), 1000))
                    {
                        SetZoom (pScale->GetValue ());

                        SfxBindings& rBindings = GetViewFrame()->GetBindings();
                        rBindings.Invalidate( SID_ATTR_ZOOM );
                        rBindings.Invalidate( SID_ZOOM_IN );
                        rBindings.Invalidate( SID_ZOOM_OUT );
                        rBindings.Invalidate( SID_ATTR_ZOOMSLIDER );
                    }
#if HAVE_FEATURE_SCRIPTING
                    else StarBASIC::FatalError (ERRCODE_BASIC_BAD_PROP_VALUE);
#endif
                    rReq.Ignore ();
                    break;
                }
#if HAVE_FEATURE_SCRIPTING
            StarBASIC::FatalError (ERRCODE_BASIC_WRONG_ARGS);
#endif
            rReq.Ignore ();
            break;
        }

        case SID_ATTR_ZOOM:
        {
            const SfxItemSet* pArgs = rReq.GetArgs();
            mbZoomOnPage = false;

            if ( pArgs )
            {
                SvxZoomType eZT = static_cast<const SvxZoomItem&>( pArgs->
                                            Get( SID_ATTR_ZOOM ) ).GetType();
                switch( eZT )
                {
                    case SvxZoomType::PERCENT:
                        SetZoom( (long) static_cast<const SvxZoomItem&>( pArgs->
                                            Get( SID_ATTR_ZOOM ) ).GetValue() );
                        break;

                    case SvxZoomType::OPTIMAL:
                        GetViewFrame()->GetDispatcher()->Execute( SID_SIZE_ALL,
                                    SfxCallMode::ASYNCHRON | SfxCallMode::RECORD );
                        break;

                    case SvxZoomType::PAGEWIDTH:
                        GetViewFrame()->GetDispatcher()->Execute( SID_SIZE_PAGE_WIDTH,
                                    SfxCallMode::ASYNCHRON | SfxCallMode::RECORD );
                        break;

                    case SvxZoomType::WHOLEPAGE:
                        GetViewFrame()->GetDispatcher()->Execute( SID_SIZE_PAGE,
                                    SfxCallMode::ASYNCHRON | SfxCallMode::RECORD );
                        break;
                    case SvxZoomType::PAGEWIDTH_NOBORDER:
                        OSL_FAIL("sd::DrawViewShell::FuTemporary(), SvxZoomType::PAGEWIDTH_NOBORDER not handled!" );
                        break;
                }
                rReq.Ignore ();
            }
            else
            {
                // open zoom dialog
                SetCurrentFunction( FuScale::Create( this, GetActiveWindow(), mpDrawView, GetDoc(), rReq ) );
            }
            Cancel();
        }
        break;

        case SID_CHANGEBEZIER:
        case SID_CHANGEPOLYGON:
            if ( mpDrawView->IsTextEdit() )
            {
                mpDrawView->SdrEndTextEdit();
                GetViewFrame()->GetDispatcher()->Execute(SID_OBJECT_SELECT, SfxCallMode::ASYNCHRON);
            }

            if ( mpDrawView->IsPresObjSelected() )
            {
                ::sd::Window* pWindow = GetActiveWindow();
                ScopedVclPtr<InfoBox>::Create(pWindow, SD_RESSTR(STR_ACTION_NOTPOSSIBLE))->Execute();
            }
            else
            {
                if( rReq.GetSlot() == SID_CHANGEBEZIER )
                {
                    WaitObject aWait( GetActiveWindow() );
                    mpDrawView->ConvertMarkedToPathObj(false);
                }
                else
                {
                    if( mpDrawView->IsVectorizeAllowed() )
                    {
                        SetCurrentFunction( FuVectorize::Create( this, GetActiveWindow(), mpDrawView, GetDoc(), rReq ) );
                    }
                    else
                    {
                        WaitObject aWait( GetActiveWindow() );
                        mpDrawView->ConvertMarkedToPolyObj();
                    }
                }

                Invalidate(SID_CHANGEBEZIER);
                Invalidate(SID_CHANGEPOLYGON);
            }
            Cancel();

            if( HasCurrentFunction(SID_BEZIER_EDIT) )
            {   // where applicable, activate right edit action
                GetViewFrame()->GetDispatcher()->Execute(SID_SWITCH_POINTEDIT,
                                        SfxCallMode::ASYNCHRON | SfxCallMode::RECORD);
            }
            rReq.Ignore ();
            break;

        case SID_CONVERT_TO_CONTOUR:
            if ( mpDrawView->IsTextEdit() )
            {
                mpDrawView->SdrEndTextEdit();
                GetViewFrame()->GetDispatcher()->Execute(SID_OBJECT_SELECT, SfxCallMode::ASYNCHRON);
            }

            if ( mpDrawView->IsPresObjSelected() )
            {
                ::sd::Window* pWindow = GetActiveWindow();
                ScopedVclPtr<InfoBox>::Create(pWindow, SD_RESSTR(STR_ACTION_NOTPOSSIBLE))->Execute();
            }
            else
            {
                WaitObject aWait( GetActiveWindow() );
                mpDrawView->ConvertMarkedToPathObj(true);

                Invalidate(SID_CONVERT_TO_CONTOUR);
            }
            Cancel();

            rReq.Ignore ();
            break;

        case SID_CONVERT_TO_METAFILE:
        case SID_CONVERT_TO_BITMAP:
        {
            // End text edit mode when it is active because the metafile or
            // bitmap that will be created does not support it.
            if ( mpDrawView->IsTextEdit() )
            {
                mpDrawView->SdrEndTextEdit();
                GetViewFrame()->GetDispatcher()->Execute(SID_OBJECT_SELECT, SfxCallMode::ASYNCHRON);
            }

            if ( mpDrawView->IsPresObjSelected(true,true,true) )
            {
                ::sd::Window* pWindow = GetActiveWindow();
                ScopedVclPtr<InfoBox>::Create(pWindow, SD_RESSTR(STR_ACTION_NOTPOSSIBLE))->Execute();
            }
            else
            {
                WaitObject aWait( GetActiveWindow() );

                // create SdrGrafObj from metafile/bitmap
                Graphic aGraphic;
                switch (nSId)
                {
                    case SID_CONVERT_TO_METAFILE:
                    {
                        // switch on undo for the next operations
                        mpDrawView->BegUndo(SD_RESSTR(STR_UNDO_CONVERT_TO_METAFILE));
                        GDIMetaFile aMetaFile(mpDrawView->GetMarkedObjMetaFile());
                        aGraphic = Graphic(aMetaFile);
                    }
                    break;
                    case SID_CONVERT_TO_BITMAP:
                    {
                        // switch on undo for the next operations
                        mpDrawView->BegUndo(SD_RESSTR(STR_UNDO_CONVERT_TO_BITMAP));
                        bool bDone(false);

                        // I have to get the image here directly since GetMarkedObjBitmapEx works
                        // based on Bitmaps, but not on BitmapEx, thus throwing away the alpha
                        // channel. Argh! GetMarkedObjBitmapEx itself is too widely used to safely
                        // change that, e.g. in the exchange formats. For now I can only add this
                        // exception to get good results for Svgs. This is how the code gets more
                        // and more crowded, at last I made a remark for myself to change this
                        // as one of the next tasks.
                        if(1 == mpDrawView->GetMarkedObjectCount())
                        {
                            const SdrGrafObj* pSdrGrafObj = dynamic_cast< const SdrGrafObj* >(mpDrawView->GetMarkedObjectByIndex(0));

                            if(pSdrGrafObj && pSdrGrafObj->isEmbeddedSvg())
                            {
                                aGraphic = Graphic(pSdrGrafObj->GetGraphic().getSvgData()->getReplacement());
                                bDone = true;
                            }
                        }

                        if(!bDone)
                        {
                            aGraphic = Graphic(mpDrawView->GetMarkedObjBitmapEx());
                        }
                    }
                    break;
                }

                // create new object
                SdrGrafObj* pGraphicObj = new SdrGrafObj (aGraphic);

                // get some necessary info and ensure it
                const SdrMarkList& rMarkList(mpDrawView->GetMarkedObjectList());
                const size_t nMarkCount(rMarkList.GetMarkCount());
                SdrPageView* pPageView = mpDrawView->GetSdrPageView();
                OSL_ENSURE(nMarkCount, "DrawViewShell::FuTemporary: SID_CONVERT_TO_BITMAP with empty selection (!)");
                OSL_ENSURE(pPageView, "DrawViewShell::FuTemporary: SID_CONVERT_TO_BITMAP without SdrPageView (!)");

                // fit rectangle of new graphic object to selection's mark rect
                Rectangle aAllMarkedRect;
                rMarkList.TakeBoundRect(pPageView, aAllMarkedRect);
                pGraphicObj->SetLogicRect(aAllMarkedRect);

                // #i71540# to keep the order, it is necessary to replace the lowest object
                // of the selection with the new object. This also means that with multi
                // selection, all other objects need to be deleted first
                SdrMark* pFirstMark = rMarkList.GetMark(0);
                SdrObject* pReplacementCandidate = pFirstMark->GetMarkedSdrObj();

                if(nMarkCount > 1)
                {
                    // take first object out of selection
                    mpDrawView->MarkObj(pReplacementCandidate, pPageView, true, true);

                    // clear remaining selection
                    mpDrawView->DeleteMarkedObj();
                }

                // #i124816# copy layer from lowest object which gets replaced
                pGraphicObj->SetLayer(pReplacementCandidate->GetLayer());

                // now replace lowest object with new one
                mpDrawView->ReplaceObjectAtView(pReplacementCandidate, *pPageView, pGraphicObj);

                // switch off undo
                mpDrawView->EndUndo();
            }
        }

        Cancel();

        rReq.Done ();
        break;

        case SID_SET_DEFAULT:
        {
            std::unique_ptr<SfxItemSet> pSet;

            if (mpDrawView->IsTextEdit())
            {
                ::Outliner* pOutl = mpDrawView->GetTextEditOutliner();
                if (pOutl)
                {
                    pOutl->RemoveFields(checkSvxFieldData<SvxURLField>);
                }

                pSet.reset(new SfxItemSet( GetPool(), EE_ITEMS_START, EE_ITEMS_END ));
                mpDrawView->SetAttributes( *pSet, true );
            }
            else
            {
                const SdrMarkList& rMarkList = mpDrawView->GetMarkedObjectList();
                const size_t nCount = rMarkList.GetMarkCount();

                // For every presentation object a SfxItemSet of hard attributes
                // and the UserCall is stored in this list. This is because
                // at the following mpDrawView->SetAttributes( *pSet, sal_True )
                // they get lost and have to be restored.
                std::vector<std::pair<SfxItemSet*,SdrObjUserCall*> > aAttrList;
                SdPage* pPresPage = static_cast<SdPage*>( mpDrawView->GetSdrPageView()->GetPage() );

                for ( size_t i = 0; i < nCount; ++i )
                {
                    SdrObject* pObj = rMarkList.GetMark(i)->GetMarkedSdrObj();

                    if( pPresPage->IsPresObj( pObj ) )
                    {
                        SfxItemSet* pNewSet = new SfxItemSet( GetDoc()->GetPool(), SDRATTR_TEXT_MINFRAMEHEIGHT, SDRATTR_TEXT_AUTOGROWHEIGHT, 0 );
                        pNewSet->Put(pObj->GetMergedItemSet());
                        aAttrList.push_back(std::make_pair(pNewSet, pObj->GetUserCall()));
                    }
                }

                pSet.reset(new SfxItemSet( GetPool() ));
                mpDrawView->SetAttributes( *pSet, true );

                sal_uLong j = 0;

                for ( size_t i = 0; i < nCount; ++i )
                {
                    SfxStyleSheet* pSheet = nullptr;
                    SdrObject* pObj = rMarkList.GetMark(i)->GetMarkedSdrObj();

                    if (pObj->GetObjIdentifier() == OBJ_TITLETEXT)
                    {
                        pSheet = mpActualPage->GetStyleSheetForPresObj(PRESOBJ_TITLE);
                        if (pSheet)
                            pObj->SetStyleSheet(pSheet, false);
                    }
                    else if(pObj->GetObjIdentifier() == OBJ_OUTLINETEXT)
                    {
                        for (sal_uInt16 nLevel = 1; nLevel < 10; nLevel++)
                        {
                            pSheet = mpActualPage->GetStyleSheetForPresObj( PRESOBJ_OUTLINE );
                            DBG_ASSERT(pSheet, "Template for outline object not found");
                            if (pSheet)
                            {
                                pObj->StartListening(*pSheet);

                                if( nLevel == 1 )
                                    // text frame listens on StyleSheet of level1
                                    pObj->NbcSetStyleSheet(pSheet, false);

                            }
                        }
                    }

                    if( pPresPage->IsPresObj( pObj ) )
                    {
                        std::pair<SfxItemSet*,SdrObjUserCall*> &rAttr = aAttrList[j++];

                        std::unique_ptr<SfxItemSet> pNewSet(rAttr.first);
                        SdrObjUserCall* pUserCall = rAttr.second;

                        if ( pNewSet && pNewSet->GetItemState( SDRATTR_TEXT_MINFRAMEHEIGHT ) == SfxItemState::SET )
                        {
                            pObj->SetMergedItem(pNewSet->Get(SDRATTR_TEXT_MINFRAMEHEIGHT));
                        }

                        if ( pNewSet && pNewSet->GetItemState( SDRATTR_TEXT_AUTOGROWHEIGHT ) == SfxItemState::SET )
                        {
                            pObj->SetMergedItem(pNewSet->Get(SDRATTR_TEXT_AUTOGROWHEIGHT));
                        }

                        if( pUserCall )
                            pObj->SetUserCall( pUserCall );
                    }
                }
            }

            pSet.reset();
            Cancel();
        }
        break;

        case SID_DELETE_SNAPITEM:
        {
            SdrPageView* pPV;
            Point   aMPos = GetActiveWindow()->PixelToLogic( maMousePos );
            sal_uInt16  nHitLog = (sal_uInt16) GetActiveWindow()->PixelToLogic( Size(
                FuPoor::HITPIX, 0 ) ).Width();
            sal_uInt16  nHelpLine;

            mbMousePosFreezed = false;

            if( mpDrawView->PickHelpLine( aMPos, nHitLog, *GetActiveWindow(), nHelpLine, pPV) )
            {
                pPV->DeleteHelpLine( nHelpLine );
            }
            Cancel();
            rReq.Ignore ();
        }
        break;

        case SID_DELETE_PAGE:
        case SID_DELETE_MASTER_PAGE:
            DeleteActualPage();
            Cancel();
            rReq.Ignore ();
        break;

        case SID_DELETE_LAYER:
            DeleteActualLayer();
            Cancel();
            rReq.Ignore ();
        break;

        case SID_ORIGINAL_SIZE:
            mpDrawView->SetMarkedOriginalSize();
            Cancel();
            rReq.Done();
        break;

        case SID_DRAW_FONTWORK:
        case SID_DRAW_FONTWORK_VERTICAL:
        {
            svx::FontworkBar::execute( mpView, rReq, GetViewFrame()->GetBindings() );       // SJ: can be removed  (I think)
            Cancel();
            rReq.Done();
        }
        break;

        case SID_SAVE_GRAPHIC:
        {
            const SdrMarkList& rMarkList = mpDrawView->GetMarkedObjectList();
            if( rMarkList.GetMarkCount() == 1 )
            {
                SdrObject* pObj = rMarkList.GetMark( 0 )->GetMarkedSdrObj();
                if( pObj && dynamic_cast< const SdrGrafObj *>( pObj ) !=  nullptr && static_cast<SdrGrafObj*>(pObj)->GetGraphicType() == GRAPHIC_BITMAP )
                {
                    GraphicObject aGraphicObject( static_cast<SdrGrafObj*>( pObj )->GetGraphicObject() );
                    {
                        GraphicHelper::ExportGraphic( aGraphicObject.GetGraphic(), "" );
                    }
                }
            }
            Cancel();
            rReq.Ignore();
        }
        break;

        case SID_EXTERNAL_EDIT:
        {
            const SdrMarkList& rMarkList = mpDrawView->GetMarkedObjectList();
            if( rMarkList.GetMarkCount() == 1 )
            {
                SdrObject* pObj = rMarkList.GetMark( 0 )->GetMarkedSdrObj();
                if( pObj && dynamic_cast< const SdrGrafObj *>( pObj ) !=  nullptr && static_cast<SdrGrafObj*>(pObj)->GetGraphicType() == GRAPHIC_BITMAP )
                {
                    GraphicObject aGraphicObject( static_cast<SdrGrafObj*>(pObj)->GetGraphicObject() );
                    m_ExternalEdits.push_back(
                        o3tl::make_unique<SdrExternalToolEdit>(
                            mpDrawView, pObj));
                    m_ExternalEdits.back()->Edit( &aGraphicObject );
                }
            }
            Cancel();
            rReq.Ignore();
        }
        break;

        case SID_COMPRESS_GRAPHIC:
        {
            const SdrMarkList& rMarkList = mpDrawView->GetMarkedObjectList();
            if( rMarkList.GetMarkCount() == 1 )
            {
                SdrObject* pObj = rMarkList.GetMark( 0 )->GetMarkedSdrObj();

                if( pObj && dynamic_cast< const SdrGrafObj *>( pObj ) !=  nullptr && static_cast<SdrGrafObj*>(pObj)->GetGraphicType() == GRAPHIC_BITMAP )
                {
                    SdrGrafObj* pGraphicObj = static_cast<SdrGrafObj*>(pObj);
                    ScopedVclPtrInstance< CompressGraphicsDialog > dialog( GetParentWindow(), pGraphicObj, GetViewFrame()->GetBindings() );
                    if ( dialog->Execute() == RET_OK )
                    {
                        SdrGrafObj* pNewObject = dialog->GetCompressedSdrGrafObj();
                        SdrPageView* pPageView = mpDrawView->GetSdrPageView();
                        OUString aUndoString = mpDrawView->GetDescriptionOfMarkedObjects();
                        aUndoString += " Compress";
                        mpDrawView->BegUndo( aUndoString );
                        mpDrawView->ReplaceObjectAtView( pObj, *pPageView, pNewObject );
                        mpDrawView->EndUndo();
                    }
                }
            }
            Cancel();
            rReq.Ignore();
        }
        break;

        case SID_ATTRIBUTES_LINE:  // BASIC
        {
            SetCurrentFunction( FuLine::Create( this, GetActiveWindow(), mpDrawView, GetDoc(), rReq ) );
            Cancel();
        }
        break;

        case SID_ATTRIBUTES_AREA:  // BASIC
        {
            SetCurrentFunction( FuArea::Create( this, GetActiveWindow(), mpDrawView, GetDoc(), rReq ) );
            Cancel();
        }
        break;

        case SID_ATTR_TRANSFORM:
        {
            SetCurrentFunction( FuTransform::Create( this, GetActiveWindow(), mpDrawView, GetDoc(), rReq ) );
            Invalidate(SID_RULER_OBJECT);
            Cancel();
        }
        break;

        case SID_CHAR_DLG_EFFECT:
        case SID_CHAR_DLG:  // BASIC
        {
            SetCurrentFunction( FuChar::Create( this, GetActiveWindow(), mpDrawView, GetDoc(), rReq ) );
            Cancel();
        }
        break;

        case SID_PARA_DLG:
        {
            SetCurrentFunction( FuParagraph::Create( this, GetActiveWindow(), mpDrawView, GetDoc(), rReq ) );
            Cancel();
        }
        break;

        case FN_NUM_BULLET_ON:
        {
            // The value (sal_uInt16)0xFFFF means set bullet on/off.
            SfxUInt16Item aItem(FN_SVX_SET_BULLET, (sal_uInt16)0xFFFF);
            GetViewFrame()->GetDispatcher()->ExecuteList(FN_SVX_SET_BULLET,
                    SfxCallMode::RECORD, { &aItem });
        }
        break;

        case FN_NUM_NUMBERING_ON:
        {
            // The value (sal_uInt16)0xFFFF means set bullet on/off.
            SfxUInt16Item aItem(FN_SVX_SET_NUMBER, (sal_uInt16)0xFFFF);
            GetViewFrame()->GetDispatcher()->ExecuteList(FN_SVX_SET_NUMBER,
                    SfxCallMode::RECORD, { &aItem });
        }
        break;

        case SID_OUTLINE_BULLET:
        case FN_SVX_SET_BULLET:
        case FN_SVX_SET_NUMBER:
        {
            SetCurrentFunction( FuOutlineBullet::Create( this, GetActiveWindow(), mpDrawView, GetDoc(), rReq ) );
            Cancel();
        }
        break;

        case FN_INSERT_SOFT_HYPHEN:
        case FN_INSERT_HARDHYPHEN:
        case FN_INSERT_HARD_SPACE:
        case SID_INSERT_RLM :
        case SID_INSERT_LRM :
        case SID_INSERT_ZWNBSP :
        case SID_INSERT_ZWSP:
        case SID_CHARMAP:
        {
            SetCurrentFunction( FuBullet::Create( this, GetActiveWindow(), mpDrawView, GetDoc(), rReq ) );
            Cancel();
        }
        break;

        case SID_PRESENTATION_LAYOUT:
        {
            SetCurrentFunction( FuPresentationLayout::Create(this, GetActiveWindow(), mpDrawView, GetDoc(), rReq) );
            Cancel();
        }
        break;

        case SID_PASTE_SPECIAL:
        {
            SetCurrentFunction( FuInsertClipboard::Create( this, GetActiveWindow(), mpDrawView, GetDoc(), rReq ) );
            Cancel();
            rReq.Ignore ();
        }
        break;

        case SID_CHANGE_PICTURE:
        case SID_INSERT_GRAPHIC:
        {
            SetCurrentFunction( FuInsertGraphic::Create( this, GetActiveWindow(), mpDrawView, GetDoc(), rReq,
                                                         nSId == SID_CHANGE_PICTURE ) );
            Cancel();
            rReq.Ignore ();
        }
        break;

        case SID_INSERT_AVMEDIA:
        {
            SetCurrentFunction( FuInsertAVMedia::Create( this, GetActiveWindow(), mpDrawView, GetDoc(), rReq ) );

            Cancel();
            rReq.Ignore ();
        }
        break;

        case SID_INSERT_OBJECT:
        case SID_INSERT_FLOATINGFRAME:
        case SID_INSERT_MATH:
        case SID_INSERT_DIAGRAM:
        case SID_ATTR_TABLE:
        {
            SetCurrentFunction( FuInsertOLE::Create( this, GetActiveWindow(), mpDrawView, GetDoc(), rReq ) );
            // Set the selection tool as the old one. This in particular important for the
            // zoom function, in which clicking without dragging zooms as well, and that
            // makes exiting the object editing mode impossible.
            SetOldFunction( FuSelection::Create( this, GetActiveWindow(), mpDrawView, GetDoc(), rReq ) );
            Cancel();
            rReq.Ignore ();
        }
        break;
#if HAVE_FEATURE_GLTF
        case SID_INSERT_3DMODEL:
        {
            SetCurrentFunction( FuInsert3DModel::Create( this, GetActiveWindow(), mpDrawView, GetDoc(), rReq ) );

            Cancel();
            rReq.Ignore ();
        }
        break;
#endif
        case SID_CLASSIFICATION_APPLY:
        {
            const SfxItemSet* pArgs = rReq.GetArgs();
            const SfxPoolItem* pItem = nullptr;
            if (pArgs && pArgs->GetItemState(nSId, false, &pItem) == SfxItemState::SET)
            {
                const OUString& rName = static_cast<const SfxStringItem*>(pItem)->GetValue();
                auto eType = SfxClassificationPolicyType::IntellectualProperty;
                if (pArgs->GetItemState(SID_TYPE_NAME, false, &pItem) == SfxItemState::SET)
                {
                    const OUString& rType = static_cast<const SfxStringItem*>(pItem)->GetValue();
                    eType = SfxClassificationHelper::stringToPolicyType(rType);
                }
                if (SfxViewFrame* pViewFrame = GetViewFrame())
                {
                    if (SfxObjectShell* pObjectShell = pViewFrame->GetObjectShell())
                    {
                        SfxClassificationHelper aHelper(pObjectShell->getDocProperties());
                        aHelper.SetBACName(rName, eType);
                    }
                }
            }
            else
                SAL_WARN("sd.ui", "missing parameter for SID_CLASSIFICATION_APPLY");

            Cancel();
            rReq.Ignore();
        }
        break;

        case SID_COPYOBJECTS:
        {
            if ( mpDrawView->IsPresObjSelected(false) )
            {
                ::sd::Window* pWindow = GetActiveWindow();
                ScopedVclPtr<InfoBox>::Create(pWindow, SD_RESSTR(STR_ACTION_NOTPOSSIBLE))->Execute();
            }
            else
            {
                if ( mpDrawView->IsTextEdit() )
                {
                    mpDrawView->SdrEndTextEdit();
                }

                SetCurrentFunction( FuCopy::Create( this, GetActiveWindow(), mpDrawView, GetDoc(), rReq ) );
            }
            Cancel();
            rReq.Ignore ();
        }
        break;

        case SID_INSERTFILE:  // BASIC
        {
            Broadcast (ViewShellHint(ViewShellHint::HINT_COMPLEX_MODEL_CHANGE_START));
            SetCurrentFunction( FuInsertFile::Create( this, GetActiveWindow(), mpDrawView, GetDoc(), rReq ) );
            Broadcast (ViewShellHint(ViewShellHint::HINT_COMPLEX_MODEL_CHANGE_END));
            Cancel();
            rReq.Done ();
        }
        break;

        case SID_SELECT_BACKGROUND:
        case SID_ATTR_PAGE_SIZE:
        case SID_ATTR_PAGE:
        case SID_PAGESETUP:  // BASIC ??
        {
            SetCurrentFunction( FuPage::Create( this, GetActiveWindow(), mpDrawView, GetDoc(), rReq ) );
            Cancel();
            rReq.Ignore (); // we generate independent macros !!
        }
        break;

        case SID_BEFORE_OBJ:
        case SID_BEHIND_OBJ:
        {
            SetCurrentFunction( FuDisplayOrder::Create(this, GetActiveWindow(), mpDrawView, GetDoc(), rReq) );
            rReq.Done();
            // finishes itself, no Cancel() needed!
        }
        break;

        case SID_REVERSE_ORDER:   // BASIC
        {
            mpDrawView->ReverseOrderOfMarked();
            Cancel();
            rReq.Done ();
        }
        break;

        case SID_ANIMATION_EFFECTS:
        {
            SetCurrentFunction( FuObjectAnimationParameters::Create( this, GetActiveWindow(), mpDrawView, GetDoc(), rReq) );
            Cancel();
        }
        break;

        case SID_LINEEND_POLYGON:
        {
            SetCurrentFunction( FuLineEnd::Create( this, GetActiveWindow(), mpDrawView, GetDoc(), rReq ) );
            Cancel();
        }
        break;

        case SID_CAPTUREPOINT:
            // negative value to signal call from menu
            maMousePos = Point(-1,-1);
            SAL_FALLTHROUGH;
        case SID_SET_SNAPITEM:
        {
            SetCurrentFunction( FuSnapLine::Create(this, GetActiveWindow(), mpDrawView, GetDoc(), rReq) );
            Cancel();
        }
        break;

        case SID_MANAGE_LINKS:
        {
            SetCurrentFunction( FuLink::Create( this, GetActiveWindow(), mpDrawView, GetDoc(), rReq ) );
            Cancel();
            rReq.Ignore ();
        }
        break;

        case SID_THESAURUS:
        {
            SetCurrentFunction( FuThesaurus::Create( this, GetActiveWindow(), mpDrawView, GetDoc(), rReq ) );
            Cancel();
            rReq.Ignore ();
        }
        break;

        case SID_TEXTATTR_DLG:
        {
            SetCurrentFunction( FuTextAttrDlg::Create( this, GetActiveWindow(), mpDrawView, GetDoc(), rReq ) );
            Cancel();
            rReq.Ignore ();
        }
        break;

        case SID_MEASURE_DLG:
        {
            SetCurrentFunction( FuMeasureDlg::Create( this, GetActiveWindow(), mpDrawView, GetDoc(), rReq ) );
            Cancel();
            rReq.Ignore ();
        }
        break;

        case SID_CONNECTION_DLG:
        {
            SetCurrentFunction( FuConnectionDlg::Create( this, GetActiveWindow(), mpDrawView, GetDoc(), rReq ) );
            Cancel();
            rReq.Done();
        }
        break;

        case SID_CONNECTION_NEW_ROUTING:
        {
            SfxItemSet aDefAttr( GetPool(), SDRATTR_EDGELINE1DELTA, SDRATTR_EDGELINE3DELTA );
            GetView()->SetAttributes( aDefAttr, true ); // (ReplaceAll)

            Cancel();
            rReq.Done();
        }
        break;

        case SID_TWAIN_SELECT:
        {
            if( mxScannerManager.is() )
            {
                try
                {
                    const css::uno::Sequence< css::scanner::ScannerContext >
                        aContexts( mxScannerManager->getAvailableScanners() );

                    if( aContexts.getLength() )
                    {
                        css::scanner::ScannerContext aContext( aContexts.getConstArray()[ 0 ] );
                        mxScannerManager->configureScannerAndScan( aContext, mxScannerListener );
                    }
                }
                catch(...)
                {
                }
            }

            Cancel();
            rReq.Done();
        }
        break;

        case SID_TWAIN_TRANSFER:
        {
            bool bDone = false;

            if( mxScannerManager.is() )
            {
                try
                {
                    const css::uno::Sequence< css::scanner::ScannerContext > aContexts( mxScannerManager->getAvailableScanners() );

                    if( aContexts.getLength() )
                    {
                        mxScannerManager->startScan( aContexts.getConstArray()[ 0 ], mxScannerListener );
                        bDone = true;
                    }
                }
                catch( ... )
                {
                }
            }

            if( !bDone )
            {
#ifndef UNX
                const sal_uInt16 nId = STR_TWAIN_NO_SOURCE;
#else
                const sal_uInt16 nId = STR_TWAIN_NO_SOURCE_UNX;
#endif

                ::sd::Window* pWindow = GetActiveWindow();
                ScopedVclPtr<InfoBox>::Create(pWindow, SD_RESSTR(nId))->Execute();
            }
            else
            {
                SfxBindings& rBindings = GetViewFrame()->GetBindings();
                rBindings.Invalidate( SID_TWAIN_SELECT );
                rBindings.Invalidate( SID_TWAIN_TRANSFER );
            }

            Cancel();
            rReq.Done();
        }
        break;

        case SID_POLYGON_MORPHING:
        {
            SetCurrentFunction( FuMorph::Create( this, GetActiveWindow(), mpDrawView, GetDoc(), rReq ) );
            Cancel();
        }
        break;

        case SID_VECTORIZE:
        {
            SetCurrentFunction( FuVectorize::Create( this, GetActiveWindow(), mpDrawView, GetDoc(), rReq ) );
            Cancel();
        }
        break;

        case SID_INSERTLAYER:
        {
            if ( mpDrawView->IsTextEdit() )
            {
                mpDrawView->SdrEndTextEdit();
            }

            SdrLayerAdmin& rLayerAdmin = GetDoc()->GetLayerAdmin();
            sal_uInt16 nLayerCnt = rLayerAdmin.GetLayerCount();
            sal_uInt16 nLayer = nLayerCnt - 2 + 1;
            OUString aLayerName = SD_RESSTR(STR_LAYER) + OUString::number(nLayer);
            OUString aLayerTitle, aLayerDesc;
            bool bIsVisible = false;
            bool bIsLocked = false;
            bool bIsPrintable = false;

            const SfxItemSet* pArgs = rReq.GetArgs();

            if (! pArgs)
            {
                SfxItemSet aNewAttr( GetDoc()->GetPool(), ATTR_LAYER_START, ATTR_LAYER_END );

                aNewAttr.Put( SdAttrLayerName( aLayerName ) );
                aNewAttr.Put( SdAttrLayerTitle() );
                aNewAttr.Put( SdAttrLayerDesc() );
                aNewAttr.Put( SdAttrLayerVisible() );
                aNewAttr.Put( SdAttrLayerPrintable() );
                aNewAttr.Put( SdAttrLayerLocked() );
                aNewAttr.Put( SdAttrLayerThisPage() );

                SdAbstractDialogFactory* pFact = SdAbstractDialogFactory::Create();
                std::unique_ptr<AbstractSdInsertLayerDlg> pDlg(pFact ? pFact->CreateSdInsertLayerDlg(GetActiveWindow(), aNewAttr, true, SD_RESSTR(STR_INSERTLAYER)) : nullptr);
                if( pDlg )
                {
                    pDlg->SetHelpId( SD_MOD()->GetSlotPool()->GetSlot( SID_INSERTLAYER )->GetCommand() );

                    // test for already existing names
                    bool bLoop = true;
                    while( bLoop && pDlg->Execute() == RET_OK )
                    {
                        pDlg->GetAttr( aNewAttr );
                        aLayerName   = static_cast<const SdAttrLayerName &>( aNewAttr.Get (ATTR_LAYER_NAME)).GetValue ();

                        if( rLayerAdmin.GetLayer( aLayerName, false )
                            || aLayerName.isEmpty() )
                        {
                            // name already exists
                            ScopedVclPtrInstance<WarningBox> aWarningBox (
                                GetParentWindow(),
                                WinBits( WB_OK ),
                                SD_RESSTR(STR_WARN_NAME_DUPLICATE));
                            aWarningBox->Execute();
                        }
                        else
                            bLoop = false;
                    }
                    if( bLoop ) // was canceled
                    {
                        pDlg.reset();
                        Cancel();
                        rReq.Ignore ();
                        break;
                    }
                    else
                    {
                        aLayerTitle  = static_cast<const SdAttrLayerTitle &>( aNewAttr.Get (ATTR_LAYER_TITLE)).GetValue ();
                        aLayerDesc   = static_cast<const SdAttrLayerDesc &>( aNewAttr.Get (ATTR_LAYER_DESC)).GetValue ();
                        bIsVisible   = static_cast<const SdAttrLayerVisible &>( aNewAttr.Get (ATTR_LAYER_VISIBLE)).GetValue ();
                        bIsLocked    = static_cast<const SdAttrLayerLocked &>( aNewAttr.Get (ATTR_LAYER_LOCKED)).GetValue () ;
                        bIsPrintable = static_cast<const SdAttrLayerPrintable &>( aNewAttr.Get (ATTR_LAYER_PRINTABLE)).GetValue () ;
                    }
                }
            }
            else if (pArgs->Count () != 4)
                 {
#if HAVE_FEATURE_SCRIPTING
                     StarBASIC::FatalError (ERRCODE_BASIC_WRONG_ARGS);
#endif
                     Cancel();
                     rReq.Ignore ();
                     break;
                 }
                 else
                 {
                     const SfxStringItem* pLayerName = rReq.GetArg<SfxStringItem>(ID_VAL_LAYERNAME);
                     const SfxBoolItem* pIsVisible = rReq.GetArg<SfxBoolItem>(ID_VAL_ISVISIBLE);
                     const SfxBoolItem* pIsLocked = rReq.GetArg<SfxBoolItem>(ID_VAL_ISLOCKED);
                     const SfxBoolItem* pIsPrintable = rReq.GetArg<SfxBoolItem>(ID_VAL_ISPRINTABLE);

                     aLayerName   = pLayerName->GetValue ();
                     bIsVisible   = pIsVisible->GetValue ();
                     bIsLocked    = pIsLocked->GetValue ();
                     bIsPrintable = pIsPrintable->GetValue ();
                 }

            OUString aPrevLayer = mpDrawView->GetActiveLayer();
            SdrLayer* pLayer;
            sal_uInt16 nPrevLayer = 0;
            nLayerCnt = rLayerAdmin.GetLayerCount();

            for ( nLayer = 0; nLayer < nLayerCnt; nLayer++ )
            {
                pLayer = rLayerAdmin.GetLayer(nLayer);
                OUString aName = pLayer->GetName();

                if ( aPrevLayer == aName )
                {
                    nPrevLayer = std::max(nLayer, (sal_uInt16) 4);
                }
            }

            mpDrawView->InsertNewLayer(aLayerName, nPrevLayer + 1);
            pLayer = rLayerAdmin.GetLayer(aLayerName, false);
            if( pLayer )
            {
                pLayer->SetTitle( aLayerTitle );
                pLayer->SetDescription( aLayerDesc );
            }

            mpDrawView->SetLayerVisible( aLayerName, bIsVisible );
            mpDrawView->SetLayerLocked( aLayerName, bIsLocked);
            mpDrawView->SetLayerPrintable(aLayerName, bIsPrintable);

            mpDrawView->SetActiveLayer(aLayerName);

            ResetActualLayer();

            GetDoc()->SetChanged();

            GetViewFrame()->GetDispatcher()->Execute(SID_SWITCHLAYER,
                    SfxCallMode::ASYNCHRON | SfxCallMode::RECORD);

            Cancel();
            rReq.Done ();
        }
        break;

        case SID_MODIFYLAYER:
        {
            if(!GetLayerTabControl()) // #i87182#
            {
                OSL_ENSURE(false, "No LayerTabBar (!)");
                Cancel();
                rReq.Ignore();
                break;
            }

            if ( mpDrawView->IsTextEdit() )
            {
                mpDrawView->SdrEndTextEdit();
            }

            SdrLayerAdmin& rLayerAdmin = GetDoc()->GetLayerAdmin();
            sal_uInt16 nCurPage = GetLayerTabControl()->GetCurPageId();
            OUString aLayerName = GetLayerTabControl()->GetPageText(nCurPage);
            SdrLayer* pLayer = rLayerAdmin.GetLayer(aLayerName, false);

            OUString aLayerTitle = pLayer->GetTitle();
            OUString aLayerDesc = pLayer->GetDescription();

            OUString aOldLayerName(aLayerName);
            OUString aOldLayerTitle(aLayerTitle);
            OUString aOldLayerDesc(aLayerDesc);

            bool bIsVisible, bIsLocked, bIsPrintable;
            bool bOldIsVisible = bIsVisible = mpDrawView->IsLayerVisible(aLayerName);
            bool bOldIsLocked = bIsLocked = mpDrawView->IsLayerLocked(aLayerName);
            bool bOldIsPrintable = bIsPrintable = mpDrawView->IsLayerPrintable(aLayerName);

            const SfxItemSet* pArgs = rReq.GetArgs();
            // is it allowed to delete the layer?
            bool bDelete = true;

            OUString aLayoutLayer = SD_RESSTR(STR_LAYER_LAYOUT);
            OUString aControlsLayer = SD_RESSTR(STR_LAYER_CONTROLS);
            OUString aMeasureLinesLayer = SD_RESSTR(STR_LAYER_MEASURELINES);
            OUString aBackgroundLayer = SD_RESSTR(STR_LAYER_BCKGRND);
            OUString aBackgroundObjLayer = SD_RESSTR(STR_LAYER_BCKGRNDOBJ);

            if( aLayerName == aLayoutLayer       || aLayerName == aControlsLayer ||
                aLayerName == aMeasureLinesLayer ||
                aLayerName == aBackgroundLayer   || aLayerName == aBackgroundObjLayer )
            {
                bDelete = false;
            }

            if (! pArgs)
            {
                SfxItemSet aNewAttr( GetDoc()->GetPool(), ATTR_LAYER_START, ATTR_LAYER_END );

                aNewAttr.Put( SdAttrLayerName( aLayerName ) );
                aNewAttr.Put( SdAttrLayerTitle( aLayerTitle ) );
                aNewAttr.Put( SdAttrLayerDesc( aLayerDesc ) );
                aNewAttr.Put( SdAttrLayerVisible( bIsVisible ) );
                aNewAttr.Put( SdAttrLayerLocked( bIsLocked ) );
                aNewAttr.Put( SdAttrLayerPrintable( bIsPrintable ) );
                aNewAttr.Put( SdAttrLayerThisPage() );

                SdAbstractDialogFactory* pFact = SdAbstractDialogFactory::Create();
                std::unique_ptr<AbstractSdInsertLayerDlg> pDlg(pFact ? pFact->CreateSdInsertLayerDlg(GetActiveWindow(), aNewAttr, bDelete, SD_RESSTR(STR_MODIFYLAYER)) : nullptr);
                if( pDlg )
                {
                    pDlg->SetHelpId( SD_MOD()->GetSlotPool()->GetSlot( SID_MODIFYLAYER )->GetCommand() );

                    // test for already existing names
                    bool    bLoop = true;
                    sal_uInt16  nRet = 0;
                    while( bLoop && ( (nRet = pDlg->Execute()) == RET_OK ) )
                    {
                        pDlg->GetAttr( aNewAttr );
                        aLayerName   = static_cast<const SdAttrLayerName &>( aNewAttr.Get (ATTR_LAYER_NAME)).GetValue ();

                        if( (rLayerAdmin.GetLayer( aLayerName, false ) &&
                             aLayerName != aOldLayerName) || aLayerName.isEmpty() )
                        {
                            // name already exists
                            ScopedVclPtrInstance<WarningBox> aWarningBox (
                                GetParentWindow(),
                                WinBits( WB_OK ),
                                SD_RESSTR(STR_WARN_NAME_DUPLICATE));
                            aWarningBox->Execute();
                        }
                        else
                            bLoop = false;
                    }
                    switch (nRet)
                    {
                        case RET_OK :
                            aLayerTitle  = static_cast<const SdAttrLayerTitle &>( aNewAttr.Get (ATTR_LAYER_TITLE)).GetValue ();
                            aLayerDesc   = static_cast<const SdAttrLayerDesc &>( aNewAttr.Get (ATTR_LAYER_DESC)).GetValue ();
                            bIsVisible   = static_cast<const SdAttrLayerVisible &>( aNewAttr.Get (ATTR_LAYER_VISIBLE)).GetValue ();
                            bIsLocked    = static_cast<const SdAttrLayerLocked &>( aNewAttr.Get (ATTR_LAYER_LOCKED)).GetValue ();
                            bIsPrintable = static_cast<const SdAttrLayerLocked &>( aNewAttr.Get (ATTR_LAYER_PRINTABLE)).GetValue ();
                            break;

                        default :
                            pDlg.reset();
                            rReq.Ignore ();
                            Cancel ();
                            return;
                    }
                }
            }
            else if (pArgs->Count () == 4)
            {
                const SfxStringItem* pLayerName = rReq.GetArg<SfxStringItem>(ID_VAL_LAYERNAME);
                const SfxBoolItem* pIsVisible = rReq.GetArg<SfxBoolItem>(ID_VAL_ISVISIBLE);
                const SfxBoolItem* pIsLocked = rReq.GetArg<SfxBoolItem>(ID_VAL_ISLOCKED);
                const SfxBoolItem* pIsPrintable = rReq.GetArg<SfxBoolItem>(ID_VAL_ISPRINTABLE);

                aLayerName   = pLayerName->GetValue ();
                bIsVisible   = pIsVisible->GetValue ();
                bIsLocked    = pIsLocked->GetValue ();
                bIsPrintable = pIsPrintable->GetValue ();
            }
            else
            {
#if HAVE_FEATURE_SCRIPTING
                StarBASIC::FatalError (ERRCODE_BASIC_WRONG_ARGS);
#endif
                Cancel ();
                rReq.Ignore ();
                break;
            }

            ::svl::IUndoManager* pManager = GetDoc()->GetDocSh()->GetUndoManager();
            SdLayerModifyUndoAction* pAction = new SdLayerModifyUndoAction(
                GetDoc(),
                pLayer,
                // old values
                aOldLayerName,
                aOldLayerTitle,
                aOldLayerDesc,
                bOldIsVisible,
                bOldIsLocked,
                bOldIsPrintable,
                // new values
                aLayerName,
                aLayerTitle,
                aLayerDesc,
                bIsVisible,
                bIsLocked,
                bIsPrintable
                );
            pManager->AddUndoAction( pAction );

            ModifyLayer( pLayer, aLayerName, aLayerTitle, aLayerDesc, bIsVisible, bIsLocked, bIsPrintable );

            Cancel();
            rReq.Done ();
        }
        break;

        case SID_RENAMELAYER:
        {
            if ( mpDrawView->IsTextEdit() )
            {
                mpDrawView->SdrEndTextEdit();
            }

            if(GetLayerTabControl()) // #i87182#
            {
                GetLayerTabControl()->StartEditMode(GetLayerTabControl()->GetCurPageId());
            }
            else
            {
                OSL_ENSURE(false, "No LayerTabBar (!)");
            }

            Cancel();
            rReq.Ignore ();
        }
        break;

        case SID_EDIT_HYPERLINK :
        {
            GetViewFrame()->GetDispatcher()->Execute( SID_HYPERLINK_DIALOG );

            Cancel();
            rReq.Done ();
        }
        break;

        case SID_OPEN_HYPERLINK:
        {
            OutlinerView* pOutView = mpDrawView->GetTextEditOutlinerView();
            if ( pOutView )
            {
                const SvxFieldItem* pFieldItem = pOutView->GetFieldAtSelection();
                if ( pFieldItem )
                {
                    const SvxFieldData* pField = pFieldItem->GetField();
                    if( pField && dynamic_cast< const SvxURLField *>( pField ) !=  nullptr )
                    {
                        const SvxURLField* pURLField = static_cast< const SvxURLField* >( pField );

                        SfxStringItem aUrl( SID_FILE_NAME, pURLField->GetURL() );
                        SfxStringItem aTarget( SID_TARGETNAME, pURLField->GetTargetFrame() );

                        OUString aReferName;
                        SfxViewFrame* pFrame = GetViewFrame();
                        SfxMedium* pMed = pFrame->GetObjectShell()->GetMedium();
                        if (pMed)
                            aReferName = pMed->GetName();

                        SfxFrameItem aFrm( SID_DOCFRAME, pFrame );
                        SfxStringItem aReferer( SID_REFERER, aReferName );

                        SfxBoolItem aNewView( SID_OPEN_NEW_VIEW, false );
                        SfxBoolItem aBrowsing( SID_BROWSE, true );

                        SfxViewFrame* pViewFrm = SfxViewFrame::Current();
                        if (pViewFrm)
                        {
                            pViewFrm->GetDispatcher()->ExecuteList(SID_OPENDOC,
                                SfxCallMode::ASYNCHRON | SfxCallMode::RECORD,
                                { &aUrl, &aTarget, &aFrm, &aReferer,
                                  &aNewView, &aBrowsing });
                        }
                    }
                }
            }
            Cancel();
            rReq.Done ();
        }
        break;

        case SID_HYPERLINK_SETLINK:
        {
            const SfxItemSet* pReqArgs = rReq.GetArgs();

            if (pReqArgs)
            {
                const SvxHyperlinkItem* pHLItem =
                    static_cast<const SvxHyperlinkItem*>( &pReqArgs->Get(SID_HYPERLINK_SETLINK) );

                if (pHLItem->GetInsertMode() == HLINK_FIELD)
                {
                    InsertURLField(pHLItem->GetURL(), pHLItem->GetName(),
                                   pHLItem->GetTargetFrame(), nullptr);
                }
                else if (pHLItem->GetInsertMode() == HLINK_BUTTON)
                {
                    InsertURLButton(pHLItem->GetURL(), pHLItem->GetName(),
                                    pHLItem->GetTargetFrame(), nullptr);
                }
                else if (pHLItem->GetInsertMode() == HLINK_DEFAULT)
                {
                    OutlinerView* pOlView = mpDrawView->GetTextEditOutlinerView();

                    if (pOlView)
                    {
                        InsertURLField(pHLItem->GetURL(), pHLItem->GetName(),
                                       pHLItem->GetTargetFrame(), nullptr);
                    }
                    else
                    {
                        InsertURLButton(pHLItem->GetURL(), pHLItem->GetName(),
                                        pHLItem->GetTargetFrame(), nullptr);
                    }
                }
            }

            Cancel();
            rReq.Ignore ();
        }
        break;

        case SID_HIDE_LAST_LEVEL:
        {
            ESelection aSel;
            // fdo#78151 editing a PRESOBJ_OUTLINE in a master page ?
            ::Outliner* pOL = GetOutlinerForMasterPageOutlineTextObj(aSel);
            if (pOL)
            {
                //we are on the last paragraph
                aSel.Adjust();
                if (aSel.nEndPara == pOL->GetParagraphCount() - 1)
                {
                    sal_uInt16 nDepth = pOL->GetDepth(aSel.nEndPara);
                    //there exists a previous numbering level
                    if (nDepth != sal_uInt16(-1) && nDepth > 0)
                    {
                        Paragraph* pPara = pOL->GetParagraph(aSel.nEndPara);
                        pOL->Remove(pPara, 1);
                    }
                }
            }
            Cancel();
            rReq.Done ();
        }
        break;

        case SID_SHOW_NEXT_LEVEL:
        {
            ESelection aSel;
            // fdo#78151 editing a PRESOBJ_OUTLINE in a master page ?
            ::Outliner* pOL = GetOutlinerForMasterPageOutlineTextObj(aSel);
            if (pOL)
            {
                //we are on the last paragraph
                aSel.Adjust();
                if (aSel.nEndPara == pOL->GetParagraphCount() - 1)
                {
                    sal_uInt16 nDepth = pOL->GetDepth(aSel.nEndPara);
                    //there exists a previous numbering level
                    if (nDepth != sal_uInt16(-1) && nDepth < 8)
                    {
                        sal_uInt16 nNewDepth = nDepth+1;
                        pOL->Insert(SD_RESSTR(STR_PRESOBJ_MPOUTLINE+nNewDepth), EE_PARA_APPEND, nNewDepth);
                    }
                }
            }
            Cancel();
            rReq.Done ();
        }
        break;

        case SID_INSERT_FLD_DATE_FIX:
        case SID_INSERT_FLD_DATE_VAR:
        case SID_INSERT_FLD_TIME_FIX:
        case SID_INSERT_FLD_TIME_VAR:
        case SID_INSERT_FLD_AUTHOR:
        case SID_INSERT_FLD_PAGE:
        case SID_INSERT_FLD_PAGE_TITLE:
        case SID_INSERT_FLD_PAGES:
        case SID_INSERT_FLD_FILE:
        {
            sal_uInt16 nMul = 1;
            std::unique_ptr<SvxFieldItem> pFieldItem;

            switch( nSId )
            {
                case SID_INSERT_FLD_DATE_FIX:
                    pFieldItem.reset(new SvxFieldItem(
                        SvxDateField( Date( Date::SYSTEM ), SVXDATETYPE_FIX ), EE_FEATURE_FIELD ));
                break;

                case SID_INSERT_FLD_DATE_VAR:
                    pFieldItem.reset(new SvxFieldItem( SvxDateField(), EE_FEATURE_FIELD ));
                break;

                case SID_INSERT_FLD_TIME_FIX:
                    pFieldItem.reset(new SvxFieldItem(
                        SvxExtTimeField( ::tools::Time( ::tools::Time::SYSTEM ), SVXTIMETYPE_FIX ), EE_FEATURE_FIELD ));
                break;

                case SID_INSERT_FLD_TIME_VAR:
                    pFieldItem.reset(new SvxFieldItem( SvxExtTimeField(), EE_FEATURE_FIELD ));
                break;

                case SID_INSERT_FLD_AUTHOR:
                {
                    SvtUserOptions aUserOptions;
                    pFieldItem.reset(new SvxFieldItem(
                            SvxAuthorField(
                                aUserOptions.GetFirstName(), aUserOptions.GetLastName(), aUserOptions.GetID() ), EE_FEATURE_FIELD ));
                }
                break;

                case SID_INSERT_FLD_PAGE:
                {
                    pFieldItem.reset(new SvxFieldItem( SvxPageField(), EE_FEATURE_FIELD ));
                    nMul = 3;
                }
                break;

                case SID_INSERT_FLD_PAGE_TITLE:
                {
                    pFieldItem.reset(new SvxFieldItem( SvxPageTitleField(), EE_FEATURE_FIELD));
                    nMul = 3;
                }
                break;

                case SID_INSERT_FLD_PAGES:
                {
                    pFieldItem.reset(new SvxFieldItem( SvxPagesField(), EE_FEATURE_FIELD ));
                    nMul = 3;
                }
                break;

                case SID_INSERT_FLD_FILE:
                {
                    OUString aName;
                    if( GetDocSh()->HasName() )
                        aName = GetDocSh()->GetMedium()->GetName();
                    pFieldItem.reset(new SvxFieldItem( SvxExtFileField( aName ), EE_FEATURE_FIELD ));
                }
                break;
            }

            OutlinerView* pOLV = mpDrawView->GetTextEditOutlinerView();

            if( pOLV )
            {
                const SvxFieldItem* pOldFldItem = pOLV->GetFieldAtSelection();

                if( pOldFldItem && ( nullptr != dynamic_cast< const SvxURLField *>( pOldFldItem->GetField() ) ||
                                    nullptr != dynamic_cast< const SvxDateField *>( pOldFldItem->GetField() ) ||
                                    nullptr != dynamic_cast< const SvxTimeField *>( pOldFldItem->GetField() ) ||
                                    nullptr != dynamic_cast< const SvxExtTimeField *>( pOldFldItem->GetField() ) ||
                                    nullptr != dynamic_cast< const SvxExtFileField *>( pOldFldItem->GetField() ) ||
                                    nullptr != dynamic_cast< const SvxAuthorField *>( pOldFldItem->GetField() ) ||
                                    nullptr != dynamic_cast< const SvxPageField *>( pOldFldItem->GetField() ) ) )
                {
                    // select field, then it will be deleted when inserting
                    ESelection aSel = pOLV->GetSelection();
                    if( aSel.nStartPos == aSel.nEndPos )
                        aSel.nEndPos++;
                    pOLV->SetSelection( aSel );
                }

                if( pFieldItem )
                    pOLV->InsertField( *pFieldItem );
            }
            else
            {
                Outliner* pOutl = GetDoc()->GetInternalOutliner();
                pOutl->Init( OutlinerMode::TextObject );
                OutlinerMode nOutlMode = pOutl->GetMode();
                pOutl->SetStyleSheet( 0, nullptr );
                pOutl->QuickInsertField( *pFieldItem, ESelection() );
                OutlinerParaObject* pOutlParaObject = pOutl->CreateParaObject();

                SdrRectObj* pRectObj = new SdrRectObj( OBJ_TEXT );
                pRectObj->SetMergedItem(makeSdrTextAutoGrowWidthItem(true));

                pOutl->UpdateFields();
                pOutl->SetUpdateMode( true );
                Size aSize( pOutl->CalcTextSize() );
                aSize.Width() *= nMul;
                pOutl->SetUpdateMode( false );

                Point aPos;
                Rectangle aRect( aPos, GetActiveWindow()->GetOutputSizePixel() );
                aPos = aRect.Center();
                aPos = GetActiveWindow()->PixelToLogic(aPos);
                aPos.X() -= aSize.Width() / 2;
                aPos.Y() -= aSize.Height() / 2;

                Rectangle aLogicRect(aPos, aSize);
                pRectObj->SetLogicRect(aLogicRect);
                pRectObj->SetOutlinerParaObject( pOutlParaObject );
                mpDrawView->InsertObjectAtView(pRectObj, *mpDrawView->GetSdrPageView());
                pOutl->Init( nOutlMode );
            }

            pFieldItem.reset();

            Cancel();
            rReq.Ignore ();
        }
        break;

        case SID_MODIFY_FIELD:
        {
            OutlinerView* pOLV = mpDrawView->GetTextEditOutlinerView();

            if( pOLV )
            {
                const SvxFieldItem* pFldItem = pOLV->GetFieldAtSelection();

                if( pFldItem && (nullptr != dynamic_cast< const SvxDateField *>( pFldItem->GetField() ) ||
                                 nullptr != dynamic_cast< const SvxAuthorField *>( pFldItem->GetField() ) ||
                                 nullptr != dynamic_cast< const SvxExtFileField *>( pFldItem->GetField() ) ||
                                 nullptr != dynamic_cast< const SvxExtTimeField *>( pFldItem->GetField() ) ) )
                {
                    // Dialog...
                    SdAbstractDialogFactory* pFact = SdAbstractDialogFactory::Create();
                    std::unique_ptr<AbstractSdModifyFieldDlg> pDlg(pFact ? pFact->CreateSdModifyFieldDlg(GetActiveWindow(), pFldItem->GetField(), pOLV->GetAttribs() ) : nullptr);
                    if( pDlg && pDlg->Execute() == RET_OK )
                    {
                        // To make a correct SetAttribs() call at the utlinerView
                        // it is necessary to split the actions here
                        std::unique_ptr<SvxFieldData> pField(pDlg->GetField());
                        ESelection aSel = pOLV->GetSelection();
                        bool bSelectionWasModified(false);

                        if( pField )
                        {
                            SvxFieldItem aFieldItem( *pField, EE_FEATURE_FIELD );

                            if( aSel.nStartPos == aSel.nEndPos )
                            {
                                bSelectionWasModified = true;
                                aSel.nEndPos++;
                                pOLV->SetSelection( aSel );
                            }

                            pOLV->InsertField( aFieldItem );

                            // select again for eventual SetAttribs call
                            pOLV->SetSelection( aSel );
                        }

                        SfxItemSet aSet( pDlg->GetItemSet() );

                        if( aSet.Count() )
                        {
                            pOLV->SetAttribs( aSet );

                            ::Outliner* pOutliner = pOLV->GetOutliner();
                            if( pOutliner )
                                pOutliner->UpdateFields();
                        }

                        if(pField)
                        {
                            // restore selection to original
                            if(bSelectionWasModified)
                            {
                                aSel.nEndPos--;
                                pOLV->SetSelection( aSel );
                            }
                        }
                    }
                }
            }

            Cancel();
            rReq.Ignore ();
        }
        break;

        case SID_OPEN_XML_FILTERSETTINGS:
        {
            try
            {
                css::uno::Reference < css::ui::dialogs::XExecutableDialog > xDialog = css::ui::dialogs::XSLTFilterDialog::create( ::comphelper::getProcessComponentContext() );
                xDialog->execute();
            }
            catch( css::uno::RuntimeException& )
            {
                DBG_UNHANDLED_EXCEPTION();
            }

            Cancel();
            rReq.Ignore ();
        }
        break;

        case SID_GROUP:  // BASIC
        {
            if ( mpDrawView->IsPresObjSelected( true, true, true ) )
            {
                ::sd::Window* pWindow = GetActiveWindow();
                ScopedVclPtr<InfoBox>::Create(pWindow, SD_RESSTR(STR_ACTION_NOTPOSSIBLE))->Execute();
            }
            else
            {
                mpDrawView->GroupMarked();
            }
            Cancel();
            rReq.Done ();
        }
        break;

        case SID_UNGROUP:  // BASIC
        {
            mpDrawView->UnGroupMarked();
            Cancel();
            rReq.Done ();
        }
        break;

        case SID_NAME_GROUP:
        {
            // only allow for single object selection since the name of an object needs
            // to be unique
            if(1 == mpDrawView->GetMarkedObjectCount())
            {
                // #i68101#
                SdrObject* pSelected = mpDrawView->GetMarkedObjectByIndex(0);
                OSL_ENSURE(pSelected, "DrawViewShell::FuTemp03: nMarkCount, but no object (!)");
                OUString aName(pSelected->GetName());

                SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
                OSL_ENSURE(pFact, "Dialog creation failed!");
                std::unique_ptr<AbstractSvxObjectNameDialog> pDlg(pFact->CreateSvxObjectNameDialog(aName));
                OSL_ENSURE(pDlg, "Dialog creation failed!");

                pDlg->SetCheckNameHdl(LINK(this, DrawViewShell, NameObjectHdl));

                if(RET_OK == pDlg->Execute())
                {
                    pDlg->GetName(aName);
                    pSelected->SetName(aName);
                }
            }

            SfxBindings& rBindings = GetViewFrame()->GetBindings();
            rBindings.Invalidate( SID_NAVIGATOR_STATE, true );
            rBindings.Invalidate( SID_CONTEXT );

            Cancel();
            rReq.Ignore();
            break;
        }

        // #i68101#
        case SID_OBJECT_TITLE_DESCRIPTION:
        {
            if(1 == mpDrawView->GetMarkedObjectCount())
            {
                SdrObject* pSelected = mpDrawView->GetMarkedObjectByIndex(0);
                OSL_ENSURE(pSelected, "DrawViewShell::FuTemp03: nMarkCount, but no object (!)");
                OUString aTitle(pSelected->GetTitle());
                OUString aDescription(pSelected->GetDescription());

                SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
                OSL_ENSURE(pFact, "Dialog creation failed!");
                std::unique_ptr<AbstractSvxObjectTitleDescDialog> pDlg(pFact->CreateSvxObjectTitleDescDialog(aTitle, aDescription));
                OSL_ENSURE(pDlg, "Dialog creation failed!");

                if(RET_OK == pDlg->Execute())
                {
                    pDlg->GetTitle(aTitle);
                    pDlg->GetDescription(aDescription);
                    pSelected->SetTitle(aTitle);
                    pSelected->SetDescription(aDescription);
                }
            }

            SfxBindings& rBindings = GetViewFrame()->GetBindings();
            rBindings.Invalidate( SID_NAVIGATOR_STATE, true );
            rBindings.Invalidate( SID_CONTEXT );

            Cancel();
            rReq.Ignore();
            break;
        }

        case SID_ENTER_GROUP:  // BASIC
        {
            mpDrawView->EnterMarkedGroup();
            Cancel();
            rReq.Done ();
        }
        break;

        case SID_LEAVE_GROUP:  // BASIC
        {
            mpDrawView->LeaveOneGroup();
            Cancel();
            rReq.Done ();
        }
        break;

        case SID_LEAVE_ALL_GROUPS:  // BASIC
        {
            mpDrawView->LeaveAllGroup();
            Cancel();
            rReq.Done ();
        }
        break;

        case SID_COMBINE:  // BASIC
        {
            // End text edit to avoid conflicts
            if(mpDrawView->IsTextEdit())
                mpDrawView->SdrEndTextEdit();

            if ( mpDrawView->IsPresObjSelected() )
            {
                ::sd::Window* pWindow = GetActiveWindow();
                ScopedVclPtr<InfoBox>::Create(pWindow, SD_RESSTR(STR_ACTION_NOTPOSSIBLE))->Execute();
            }
            else
            {
                WaitObject aWait( GetActiveWindow() );
                mpDrawView->CombineMarkedObjects(false);
            }
            Cancel();
            rReq.Done ();
        }
        break;

        case SID_DISTRIBUTE_DLG:
        {
            if ( mpDrawView->IsPresObjSelected() )
            {
                ::sd::Window* pWindow = GetActiveWindow();
                ScopedVclPtr<InfoBox>::Create(pWindow, SD_RESSTR(STR_ACTION_NOTPOSSIBLE))->Execute();
            }
            else
            {
                mpDrawView->DistributeMarkedObjects();
            }
            Cancel();
            rReq.Done ();
        }
        break;

        case SID_POLY_MERGE:
        {
            // End text edit to avoid conflicts
            if(mpDrawView->IsTextEdit())
                mpDrawView->SdrEndTextEdit();

            if ( mpDrawView->IsPresObjSelected() )
            {
                ::sd::Window* pWindow = GetActiveWindow();
                ScopedVclPtr<InfoBox>::Create(pWindow, SD_RESSTR(STR_ACTION_NOTPOSSIBLE))->Execute();
            }
            else
            {
                WaitObject aWait( GetActiveWindow() );
                mpDrawView->MergeMarkedObjects(SDR_MERGE_MERGE);
            }
            Cancel();
            rReq.Done ();
        }
        break;

        case SID_POLY_SUBSTRACT:
        {
            // End text edit to avoid conflicts
            if(mpDrawView->IsTextEdit())
                mpDrawView->SdrEndTextEdit();

            if ( mpDrawView->IsPresObjSelected() )
            {
                ::sd::Window* pWindow = GetActiveWindow();
                ScopedVclPtr<InfoBox>::Create(pWindow, SD_RESSTR(STR_ACTION_NOTPOSSIBLE))->Execute();
            }
            else
            {
                WaitObject aWait( GetActiveWindow() );
                mpDrawView->MergeMarkedObjects(SDR_MERGE_SUBSTRACT);
            }
            Cancel();
            rReq.Done ();
        }
        break;

        case SID_POLY_INTERSECT:
        {
            // End text edit to avoid conflicts
            if(mpDrawView->IsTextEdit())
                mpDrawView->SdrEndTextEdit();

            if ( mpDrawView->IsPresObjSelected() )
            {
                ::sd::Window* pWindow = GetActiveWindow();
                ScopedVclPtr<InfoBox>::Create(pWindow, SD_RESSTR(STR_ACTION_NOTPOSSIBLE))->Execute();
            }
            else
            {
                WaitObject aWait( GetActiveWindow() );
                mpDrawView->MergeMarkedObjects(SDR_MERGE_INTERSECT);
            }
            Cancel();
            rReq.Done ();
        }
        break;

        case SID_EQUALIZEWIDTH:
        case SID_EQUALIZEHEIGHT:
        {
            // End text edit to avoid conflicts
            if(mpDrawView->IsTextEdit())
                mpDrawView->SdrEndTextEdit();

            mpDrawView->EqualizeMarkedObjects(nSId == SID_EQUALIZEWIDTH);
            Cancel();
            rReq.Done ();
        }
        break;

        case SID_DISMANTLE:  // BASIC
        {
            if ( mpDrawView->IsDismantlePossible() )
            {
                WaitObject aWait( GetActiveWindow() );
                mpDrawView->DismantleMarkedObjects();
            }
            Cancel();
            rReq.Done ();
        }
        break;

        case SID_CONNECT:  // BASIC
        {
            if ( mpDrawView->IsPresObjSelected() )
            {
                ::sd::Window* pWindow = GetActiveWindow();
                ScopedVclPtr<InfoBox>::Create(pWindow, SD_RESSTR(STR_ACTION_NOTPOSSIBLE))->Execute();
            }
            else
            {
                WaitObject aWait( GetActiveWindow() );
                mpDrawView->CombineMarkedObjects();
            }
            Cancel();
            rReq.Done ();
        }
        break;

        case SID_BREAK:  // BASIC
        {
            if ( mpDrawView->IsTextEdit() )
            {
                mpDrawView->SdrEndTextEdit();
            }

            if ( mpDrawView->IsBreak3DObjPossible() )
            {
                WaitObject aWait( GetActiveWindow() );
                mpDrawView->Break3DObj();
            }
            else if ( mpDrawView->IsDismantlePossible(true) )
            {
                WaitObject aWait( GetActiveWindow() );
                mpDrawView->DismantleMarkedObjects(true);
            }
            else if ( mpDrawView->IsImportMtfPossible() )
            {
                WaitObject aWait( GetActiveWindow() );
                const SdrMarkList& rMarkList = mpDrawView->GetMarkedObjectList();
                const size_t nAnz=rMarkList.GetMarkCount();

                // determine the sum of meta objects of all selected meta files
                sal_uLong nCount = 0;
                for(size_t nm=0; nm<nAnz; ++nm)
                {
                    SdrMark*     pM=rMarkList.GetMark(nm);
                    SdrObject*   pObj=pM->GetMarkedSdrObj();
                    SdrGrafObj*  pGraf= dynamic_cast< SdrGrafObj *>( pObj );
                    SdrOle2Obj*  pOle2= dynamic_cast< SdrOle2Obj *>( pObj );

                    if(pGraf)
                    {
                        if(pGraf->HasGDIMetaFile())
                        {
                            nCount += pGraf->GetGraphic().GetGDIMetaFile().GetActionSize();
                        }
                        else if(pGraf->isEmbeddedSvg())
                        {
                            nCount += pGraf->getMetafileFromEmbeddedSvg().GetActionSize();
                        }
                    }

                    if(pOle2 && pOle2->GetGraphic())
                    {
                         nCount += pOle2->GetGraphic()->GetGDIMetaFile().GetActionSize();
                    }
                }

                // decide with the sum of all meta objects if we should show a dialog
                if(nCount < MIN_ACTIONS_FOR_DIALOG)
                {
                    // nope, no dialog
                    mpDrawView->DoImportMarkedMtf();
                }
                else
                {
                    SdAbstractDialogFactory* pFact = SdAbstractDialogFactory::Create();
                    if( pFact )
                    {
                        std::unique_ptr<VclAbstractDialog> pDlg(pFact->CreateBreakDlg(GetActiveWindow(), mpDrawView, GetDocSh(), nCount, static_cast<sal_uLong>(nAnz) ));
                        if( pDlg )
                        {
                            pDlg->Execute();
                        }
                    }
                }
            }

            Cancel();
            rReq.Done ();
        }
        break;

        case SID_CONVERT_TO_3D:
        {
            if ( mpDrawView->IsPresObjSelected() )
            {
                ::sd::Window* pWindow = GetActiveWindow();
                ScopedVclPtr<InfoBox>::Create(pWindow, SD_RESSTR(STR_ACTION_NOTPOSSIBLE))->Execute();
            }
            else
            {
                if (mpDrawView->IsConvertTo3DObjPossible())
                {
                    if (mpDrawView->IsTextEdit())
                    {
                        mpDrawView->SdrEndTextEdit();
                    }

                    WaitObject aWait( GetActiveWindow() );
                    mpDrawView->ConvertMarkedObjTo3D();
                }
            }

            Cancel();
            rReq.Done();
        }
        break;

        case SID_FRAME_TO_TOP:  // BASIC
        {
            mpDrawView->PutMarkedToTop();
            Cancel();
            rReq.Done ();
        }
        break;

        case SID_MOREFRONT:  // BASIC
        {
            mpDrawView->MovMarkedToTop();
            Cancel();
            rReq.Done ();
        }
        break;

        case SID_MOREBACK:  // BASIC
        {
            mpDrawView->MovMarkedToBtm();
            Cancel();
            rReq.Done ();
        }
        break;

        case SID_FRAME_TO_BOTTOM:   // BASIC
        {
            mpDrawView->PutMarkedToBtm();
            Cancel();
            rReq.Done ();
        }
        break;

        case SID_HORIZONTAL:  // BASIC
        case SID_FLIP_HORIZONTAL:
        {
            mpDrawView->MirrorAllMarkedHorizontal();
            Cancel();
            rReq.Done ();
        }
        break;

        case SID_VERTICAL:  // BASIC
        case SID_FLIP_VERTICAL:
        {
            mpDrawView->MirrorAllMarkedVertical();
            Cancel();
            rReq.Done ();
        }
        break;

        case SID_OBJECT_ALIGN_LEFT:  // BASIC
        {
            mpDrawView->AlignMarkedObjects(SDRHALIGN_LEFT, SDRVALIGN_NONE);
            Cancel();
            rReq.Done ();
        }
        break;

        case SID_OBJECT_ALIGN_CENTER:  // BASIC
        {
            mpDrawView->AlignMarkedObjects(SDRHALIGN_CENTER, SDRVALIGN_NONE);
            Cancel();
            rReq.Done ();
        }
        break;

        case SID_OBJECT_ALIGN_RIGHT:  // BASIC
        {
            mpDrawView->AlignMarkedObjects(SDRHALIGN_RIGHT, SDRVALIGN_NONE);
            Cancel();
            rReq.Done ();
        }
        break;

        case SID_OBJECT_ALIGN_UP:  // BASIC
        {
            mpDrawView->AlignMarkedObjects(SDRHALIGN_NONE, SDRVALIGN_TOP);
            Cancel();
            rReq.Done ();
        }
        break;

        case SID_OBJECT_ALIGN_MIDDLE:  // BASIC
        {
            mpDrawView->AlignMarkedObjects(SDRHALIGN_NONE, SDRVALIGN_CENTER);
            Cancel();
            rReq.Done ();
        }
        break;

        case SID_OBJECT_ALIGN_DOWN:  // BASIC
        {
            mpDrawView->AlignMarkedObjects(SDRHALIGN_NONE, SDRVALIGN_BOTTOM);
            Cancel();
            rReq.Done ();
        }
        break;

        case SID_SELECTALL:  // BASIC
        {
            if( (dynamic_cast<FuSelection*>( GetOldFunction().get() ) != nullptr) &&
                !GetView()->IsFrameDragSingles() && GetView()->HasMarkablePoints())
            {
                if ( !mpDrawView->IsAction() )
                    mpDrawView->MarkAllPoints();
            }
            else
                mpDrawView->SelectAll();

            Cancel();
            rReq.Done ();
        }
        break;

        case SID_STYLE_NEW: // BASIC ???
        case SID_STYLE_APPLY:
        case SID_STYLE_EDIT:
        case SID_STYLE_DELETE:
        case SID_STYLE_HIDE:
        case SID_STYLE_SHOW:
        case SID_STYLE_FAMILY:
        case SID_STYLE_WATERCAN:
        case SID_STYLE_UPDATE_BY_EXAMPLE:
        case SID_STYLE_NEW_BY_EXAMPLE:
        {
            if( rReq.GetSlot() == SID_STYLE_EDIT && !rReq.GetArgs() )
            {
                SfxStyleSheet* pStyleSheet = mpDrawView->GetStyleSheet();
                if( pStyleSheet && pStyleSheet->GetFamily() == SD_STYLE_FAMILY_MASTERPAGE)
                    pStyleSheet = static_cast<SdStyleSheet*>(pStyleSheet)->GetPseudoStyleSheet();

                if( (pStyleSheet == nullptr) && GetView()->IsTextEdit() )
                {
                    GetView()->SdrEndTextEdit();

                    pStyleSheet = mpDrawView->GetStyleSheet();
                    if(pStyleSheet && pStyleSheet->GetFamily() == SD_STYLE_FAMILY_MASTERPAGE)
                        pStyleSheet = static_cast<SdStyleSheet*>(pStyleSheet)->GetPseudoStyleSheet();
                }

                if( pStyleSheet == nullptr )
                {
                    rReq.Ignore();
                    break;
                }

                SfxAllItemSet aSet(GetDoc()->GetPool());

                SfxStringItem aStyleNameItem( SID_STYLE_EDIT, pStyleSheet->GetName() );
                aSet.Put(aStyleNameItem);

                SfxUInt16Item aStyleFamilyItem( SID_STYLE_FAMILY, (sal_uInt16)pStyleSheet->GetFamily() );
                aSet.Put(aStyleFamilyItem);

                rReq.SetArgs(aSet);
            }

            if( rReq.GetArgs() )
            {
                SetCurrentFunction( FuTemplate::Create( this, GetActiveWindow(), mpDrawView, GetDoc(), rReq ) );
                if( rReq.GetSlot() == SID_STYLE_APPLY )
                    GetViewFrame()->GetBindings().Invalidate( SID_STYLE_APPLY );
                Cancel();
            }
            else if( rReq.GetSlot() == SID_STYLE_APPLY )
                GetViewFrame()->GetDispatcher()->Execute( SID_STYLE_DESIGNER, SfxCallMode::ASYNCHRON );
            rReq.Ignore ();
        }
        break;

        case SID_IMAP:
        {
            sal_uInt16      nId = SvxIMapDlgChildWindow::GetChildWindowId();

            GetViewFrame()->ToggleChildWindow( nId );
            GetViewFrame()->GetBindings().Invalidate( SID_IMAP );

            if ( GetViewFrame()->HasChildWindow( nId )
                && ( ( ViewShell::Implementation::GetImageMapDialog() ) != nullptr ) )
            {
                const SdrMarkList&  rMarkList = mpDrawView->GetMarkedObjectList();

                if ( rMarkList.GetMarkCount() == 1 )
                    UpdateIMapDlg( rMarkList.GetMark( 0 )->GetMarkedSdrObj() );
            }

            Cancel();
            rReq.Ignore ();
        }
        break;

        case SID_GRID_FRONT:
        {
            mpDrawView->SetGridFront( !mpDrawView->IsGridFront() );
            Cancel();
            rReq.Done ();
        }
        break;

        case SID_HELPLINES_FRONT:
        {
            mpDrawView->SetHlplFront( !mpDrawView->IsHlplFront() );
            Cancel();
            rReq.Done ();
        }
        break;

        case SID_FONTWORK:
        {
            if ( rReq.GetArgs() )
            {
                GetViewFrame()->SetChildWindow(SvxFontWorkChildWindow::GetChildWindowId(),
                                        static_cast<const SfxBoolItem&>(rReq.GetArgs()->
                                        Get(SID_FONTWORK)).GetValue());
            }
            else
            {
                GetViewFrame()->ToggleChildWindow( SvxFontWorkChildWindow::GetChildWindowId() );
            }

            GetViewFrame()->GetBindings().Invalidate(SID_FONTWORK);
            Cancel();
            rReq.Ignore ();
        }
        break;

        case SID_COLOR_CONTROL:
        {
            if ( rReq.GetArgs() )
                GetViewFrame()->SetChildWindow(SvxColorChildWindow::GetChildWindowId(),
                                        static_cast<const SfxBoolItem&>(rReq.GetArgs()->
                                        Get(SID_COLOR_CONTROL)).GetValue());
            else
                GetViewFrame()->ToggleChildWindow(SvxColorChildWindow::GetChildWindowId() );

            GetViewFrame()->GetBindings().Invalidate(SID_COLOR_CONTROL);
            Cancel();
            rReq.Ignore ();
        }
        break;

        case SID_EXTRUSION_TOOGLE:
        case SID_EXTRUSION_TILT_DOWN:
        case SID_EXTRUSION_TILT_UP:
        case SID_EXTRUSION_TILT_LEFT:
        case SID_EXTRUSION_TILT_RIGHT:
        case SID_EXTRUSION_3D_COLOR:
        case SID_EXTRUSION_DEPTH:
        case SID_EXTRUSION_DIRECTION:
        case SID_EXTRUSION_PROJECTION:
        case SID_EXTRUSION_LIGHTING_DIRECTION:
        case SID_EXTRUSION_LIGHTING_INTENSITY:
        case SID_EXTRUSION_SURFACE:
        case SID_EXTRUSION_DEPTH_FLOATER:
        case SID_EXTRUSION_DIRECTION_FLOATER:
        case SID_EXTRUSION_LIGHTING_FLOATER:
        case SID_EXTRUSION_SURFACE_FLOATER:
        case SID_EXTRUSION_DEPTH_DIALOG:
            svx::ExtrusionBar::execute( mpDrawView, rReq, GetViewFrame()->GetBindings() );
            Cancel();
            rReq.Ignore ();
            break;

        case SID_FONTWORK_SHAPE:
        case SID_FONTWORK_SHAPE_TYPE:
        case SID_FONTWORK_ALIGNMENT:
        case SID_FONTWORK_SAME_LETTER_HEIGHTS:
        case SID_FONTWORK_CHARACTER_SPACING:
        case SID_FONTWORK_KERN_CHARACTER_PAIRS:
        case SID_FONTWORK_GALLERY_FLOATER:
        case SID_FONTWORK_CHARACTER_SPACING_FLOATER:
        case SID_FONTWORK_ALIGNMENT_FLOATER:
        case SID_FONTWORK_CHARACTER_SPACING_DIALOG:
            svx::FontworkBar::execute( mpDrawView, rReq, GetViewFrame()->GetBindings() );
            Cancel();
            rReq.Ignore ();
            break;

        case SID_BMPMASK:
        {
            GetViewFrame()->ToggleChildWindow( SvxBmpMaskChildWindow::GetChildWindowId() );
            GetViewFrame()->GetBindings().Invalidate( SID_BMPMASK );

            Cancel();
            rReq.Ignore ();
        }
        break;

        case SID_NAVIGATOR:
        {
            if ( rReq.GetArgs() )
                GetViewFrame()->SetChildWindow(SID_NAVIGATOR,
                                        static_cast<const SfxBoolItem&>(rReq.GetArgs()->
                                        Get(SID_NAVIGATOR)).GetValue());
            else
                GetViewFrame()->ToggleChildWindow( SID_NAVIGATOR );

            GetViewFrame()->GetBindings().Invalidate(SID_NAVIGATOR);
            Cancel();
            rReq.Ignore ();
        }
        break;

        case SID_SLIDE_TRANSITIONS_PANEL:
        case SID_CUSTOM_ANIMATION_PANEL:
        case SID_GALLERY:
        {
            // First make sure that the sidebar is visible
            GetViewFrame()->ShowChildWindow(SID_SIDEBAR);

            OUString panelId;
            if (nSId == SID_CUSTOM_ANIMATION_PANEL)
                panelId = "SdCustomAnimationPanel";
            else if (nSId == SID_GALLERY)
                panelId = "GalleryPanel";
            else if (nSId == SID_SLIDE_TRANSITIONS_PANEL)
                panelId = "SdSlideTransitionPanel";

            ::sfx2::sidebar::Sidebar::ShowPanel(
                panelId,
                GetViewFrame()->GetFrame().GetFrameInterface());

            Cancel();
            rReq.Done();
        }
        break;

        case SID_ANIMATION_OBJECTS:
        {
            if ( rReq.GetArgs() )
                GetViewFrame()->SetChildWindow(
                    AnimationChildWindow::GetChildWindowId(),
                    static_cast<const SfxBoolItem&>(rReq.GetArgs()->
                        Get(SID_ANIMATION_OBJECTS)).GetValue());
            else
                GetViewFrame()->ToggleChildWindow(
                    AnimationChildWindow::GetChildWindowId() );

            GetViewFrame()->GetBindings().Invalidate(SID_ANIMATION_OBJECTS);
            Cancel();
            rReq.Ignore ();
        }
        break;

        case SID_3D_WIN:
        {
            if ( rReq.GetArgs() )
                GetViewFrame()->SetChildWindow( Svx3DChildWindow::GetChildWindowId(),
                                        static_cast<const SfxBoolItem&>(rReq.GetArgs()->
                                        Get( SID_3D_WIN )).GetValue());
            else
                GetViewFrame()->ToggleChildWindow( Svx3DChildWindow::GetChildWindowId() );

            GetViewFrame()->GetBindings().Invalidate( SID_3D_WIN );
            Cancel();
            rReq.Ignore ();
        }
        break;

        case SID_CONVERT_TO_3D_LATHE_FAST:
        {
            /* The call is enough. The initialization via Start3DCreation and
               CreateMirrorPolygons is no longer needed if the parameter
               sal_True is provided. Then a tilted rotary body with an axis left
               besides the bounding rectangle of the selected objects is drawn
               immediately and without user interaction.  */
            mpDrawView->SdrEndTextEdit();
            if(GetActiveWindow())
                GetActiveWindow()->EnterWait();
            mpDrawView->End3DCreation(true);
            Cancel();
            rReq.Ignore();
            if(GetActiveWindow())
                GetActiveWindow()->LeaveWait();
        }
        break;

        case SID_PRESENTATION_DLG:
        {
            SetCurrentFunction( FuSlideShowDlg::Create( this, GetActiveWindow(), mpDrawView, GetDoc(), rReq ) );
            Cancel();
        }
        break;

        case SID_REMOTE_DLG:
        {
#ifdef ENABLE_SDREMOTE
             SdAbstractDialogFactory* pFact = SdAbstractDialogFactory::Create();
             VclAbstractDialog* pDlg = pFact ? pFact->CreateRemoteDialog(GetActiveWindow()) : nullptr;
             if (pDlg)
                 pDlg->Execute();
#endif
        }
        break;

        case SID_CUSTOMSHOW_DLG:
        {
            SetCurrentFunction( FuCustomShowDlg::Create( this, GetActiveWindow(), mpDrawView, GetDoc(), rReq ) );
            Cancel();
        }
        break;

        case SID_EXPAND_PAGE:
        {
            SetCurrentFunction( FuExpandPage::Create( this, GetActiveWindow(), mpDrawView, GetDoc(), rReq ) );
            Cancel();
        }
        break;

        case SID_SUMMARY_PAGE:
        {
            mpDrawView->SdrEndTextEdit();
            SetCurrentFunction( FuSummaryPage::Create( this, GetActiveWindow(), mpDrawView, GetDoc(), rReq ) );
            Cancel();
        }
        break;

        case SID_AVMEDIA_PLAYER:
        {
            GetViewFrame()->ToggleChildWindow( ::avmedia::MediaPlayer::GetChildWindowId() );
            GetViewFrame()->GetBindings().Invalidate( SID_AVMEDIA_PLAYER );
            Cancel();
            rReq.Ignore ();
        }
        break;

        case SID_PRESENTATION_MINIMIZER:
        {
            Reference<XComponentContext> xContext(::comphelper::getProcessComponentContext());
            Reference<util::XURLTransformer> xParser(util::URLTransformer::create(xContext));
            Reference<frame::XDispatchProvider> xProvider(GetViewShellBase().GetController()->getFrame(), UNO_QUERY);
            if (xProvider.is())
            {
                util::URL aURL;
                aURL.Complete = "vnd.com.sun.star.comp.PresentationMinimizer:execute";
                xParser->parseStrict(aURL);
                uno::Reference<frame::XDispatch> xDispatch(xProvider->queryDispatch(aURL, OUString(), 0));
                if (xDispatch.is())
                {
                    xDispatch->dispatch(aURL, uno::Sequence< beans::PropertyValue >());
                }
            }
            Cancel();
            rReq.Ignore();
        }
        break;

        case SID_DISPLAY_MASTER_BACKGROUND:
        case SID_DISPLAY_MASTER_OBJECTS:
        {
            // Determine current page and toggle visibility of layers
            // associated with master page background or master page shapes.
            SdPage* pPage = GetActualPage();
            if (pPage != nullptr
                && GetDoc() != nullptr)
            {
                SetOfByte aVisibleLayers = pPage->TRG_GetMasterPageVisibleLayers();
                SdrLayerAdmin& rLayerAdmin = GetDoc()->GetLayerAdmin();
                sal_uInt8 aLayerId;
                if (nSId == SID_DISPLAY_MASTER_BACKGROUND)
                    aLayerId = rLayerAdmin.GetLayerID(SD_RESSTR(STR_LAYER_BCKGRND), false);
                else
                    aLayerId = rLayerAdmin.GetLayerID(SD_RESSTR(STR_LAYER_BCKGRNDOBJ), false);
                aVisibleLayers.Set(aLayerId, !aVisibleLayers.IsSet(aLayerId));
                pPage->TRG_SetMasterPageVisibleLayers(aVisibleLayers);
            }
            Cancel();
            rReq.Ignore ();
        }
        break;

       case SID_PHOTOALBUM:
       {
            SdAbstractDialogFactory* pFact = SdAbstractDialogFactory::Create();
            std::unique_ptr<VclAbstractDialog> pDlg(pFact ? pFact->CreateSdPhotoAlbumDialog(
                GetActiveWindow(),
                GetDoc()) : nullptr);

            if (pDlg)
            {
                pDlg->Execute();
                pDlg.reset();
            }
            Cancel();
            rReq.Ignore ();
        }
        break;

        default:
        {
            DBG_ASSERT( false, "Slot without function" );
            Cancel();
            rReq.Ignore ();
        }
        break;
    }

    if(HasCurrentFunction())
    {
        GetCurrentFunction()->Activate();
    }
}

void DrawViewShell::ExecChar( SfxRequest &rReq )
{
    SdDrawDocument* pDoc = GetDoc();
    if (!pDoc || !mpDrawView)
        return;

    SfxItemSet aEditAttr( pDoc->GetPool() );
    mpDrawView->GetAttributes( aEditAttr );

    //modified by wj for sym2_1580, if put old itemset into new set,
    //when mpDrawView->SetAttributes(aNewAttr) it will invalidate all the item
    // and use old attr to update all the attributes
//  SfxItemSet aNewAttr( GetPool(),
//  EE_ITEMS_START, EE_ITEMS_END );
//  aNewAttr.Put( aEditAttr, sal_False );
    SfxItemSet aNewAttr( pDoc->GetPool() );
    //modified end

    sal_uInt16 nSId = rReq.GetSlot();

    switch ( nSId )
    {
    case SID_ATTR_CHAR_FONT:
        if( rReq.GetArgs() )
        {
            const SvxFontItem* pItem = rReq.GetArg<SvxFontItem>(SID_ATTR_CHAR_FONT);
            if (pItem)
            {
                aNewAttr.Put(*pItem);
            }
        }
        break;
    case SID_ATTR_CHAR_FONTHEIGHT:
        if( rReq.GetArgs() )
        {
            const SvxFontHeightItem* pItem = rReq.GetArg<SvxFontHeightItem>(SID_ATTR_CHAR_FONTHEIGHT);
            if (pItem)
            {
                aNewAttr.Put(*pItem);
            }
        }
        break;
    case SID_ATTR_CHAR_WEIGHT:
        if( rReq.GetArgs() )
        {
            const SvxWeightItem* pItem = rReq.GetArg<SvxWeightItem>(SID_ATTR_CHAR_WEIGHT);
            if (pItem)
            {
                aNewAttr.Put(*pItem);
            }
        }
        break;
    case SID_ATTR_CHAR_POSTURE:
        if( rReq.GetArgs() )
        {
            const SvxPostureItem* pItem = rReq.GetArg<SvxPostureItem>(SID_ATTR_CHAR_POSTURE);
            if (pItem)
            {
                aNewAttr.Put(*pItem);
            }
        }
        break;
    case SID_ATTR_CHAR_UNDERLINE:
        if( rReq.GetArgs() )
        {
            const SvxUnderlineItem* pItem = rReq.GetArg<SvxUnderlineItem>(SID_ATTR_CHAR_UNDERLINE);
            if (pItem)
            {
                aNewAttr.Put(*pItem);
            }
            else
            {
                FontLineStyle eFU = static_cast<const SvxUnderlineItem&>( aEditAttr.Get( EE_CHAR_UNDERLINE ) ).GetLineStyle();
                aNewAttr.Put( SvxUnderlineItem( eFU != LINESTYLE_NONE ?LINESTYLE_NONE : LINESTYLE_SINGLE,  EE_CHAR_UNDERLINE ) );
            }//aNewAttr.Put( (const SvxUnderlineItem&)aEditAttr.Get( EE_CHAR_UNDERLINE ) );
        }
        break;
    case SID_ATTR_CHAR_SHADOWED:
        if( rReq.GetArgs() )
        {
            const SvxShadowedItem* pItem = rReq.GetArg<SvxShadowedItem>(SID_ATTR_CHAR_SHADOWED);
            if (pItem)
            {
                aNewAttr.Put(*pItem);
            }
        }
        break;
    case SID_ATTR_CHAR_STRIKEOUT:
        if( rReq.GetArgs() )
        {
            const SvxCrossedOutItem* pItem = rReq.GetArg<SvxCrossedOutItem>(SID_ATTR_CHAR_STRIKEOUT);
            if (pItem)
            {
                aNewAttr.Put(*pItem);
            }
        }
        break;
    case SID_ATTR_CHAR_COLOR:
        if( rReq.GetArgs() )
        {
            const SvxColorItem* pItem = rReq.GetArg<SvxColorItem>(SID_ATTR_CHAR_COLOR);
            if (pItem)
            {
                aNewAttr.Put(*pItem);
            }
        }
        break;
    case SID_ATTR_CHAR_KERNING:
        if( rReq.GetArgs() )
        {
            const SvxKerningItem* pItem = rReq.GetArg<SvxKerningItem>(SID_ATTR_CHAR_KERNING);
            if (pItem)
            {
                aNewAttr.Put(*pItem);
            }
        }
        break;
    case SID_ATTR_CHAR_CASEMAP:
        if( rReq.GetArgs() )
        {
            const SvxCaseMapItem* pItem = rReq.GetArg<SvxCaseMapItem>(SID_ATTR_CHAR_CASEMAP);
            if (pItem)
            {
                aNewAttr.Put(*pItem);
            }
        }
        break;
    case SID_SET_SUB_SCRIPT:
        {
            SvxEscapementItem aItem( EE_CHAR_ESCAPEMENT );
            SvxEscapement eEsc = (SvxEscapement ) static_cast<const SvxEscapementItem&>(
                            aEditAttr.Get( EE_CHAR_ESCAPEMENT ) ).GetEnumValue();
            if( eEsc == SVX_ESCAPEMENT_SUBSCRIPT )
                aItem.SetEscapement( SVX_ESCAPEMENT_OFF );
            else
                aItem.SetEscapement( SVX_ESCAPEMENT_SUBSCRIPT );
            aNewAttr.Put( aItem );
        }
        break;
    case SID_SET_SUPER_SCRIPT:
        {
            SvxEscapementItem aItem( EE_CHAR_ESCAPEMENT );
            SvxEscapement eEsc = (SvxEscapement ) static_cast<const SvxEscapementItem&>(
                            aEditAttr.Get( EE_CHAR_ESCAPEMENT ) ).GetEnumValue();
            if( eEsc == SVX_ESCAPEMENT_SUPERSCRIPT )
                aItem.SetEscapement( SVX_ESCAPEMENT_OFF );
            else
                aItem.SetEscapement( SVX_ESCAPEMENT_SUPERSCRIPT );
            aNewAttr.Put( aItem );
        }
        break;
    case SID_SHRINK_FONT_SIZE:
    case SID_GROW_FONT_SIZE:
        {
            const SvxFontListItem* pFonts = dynamic_cast<const SvxFontListItem*>(GetDocSh()->GetItem( SID_ATTR_CHAR_FONTLIST ) );
            const FontList* pFontList = pFonts ? pFonts->GetFontList() : nullptr;
            if( pFontList )
            {
                FuText::ChangeFontSize( nSId == SID_GROW_FONT_SIZE, nullptr, pFontList, mpView );
                GetViewFrame()->GetBindings().Invalidate( SID_ATTR_CHAR_FONTHEIGHT );
            }
            break;
        }
    default:
        break;
    }

    mpDrawView->SetAttributes(aNewAttr);
    rReq.Done();
    Cancel();
}

/** This method consists basically of three parts:
    1. Process the arguments of the SFX request.
    2. Use the model to create a new page or duplicate an existing one.
    3. Update the tab control and switch to the new page.
*/
SdPage* DrawViewShell::CreateOrDuplicatePage (
    SfxRequest& rRequest,
    PageKind ePageKind,
    SdPage* pPage,
    const sal_Int32 nInsertPosition)
{
    SdPage* pNewPage = nullptr;
    if (ePageKind == PK_STANDARD && meEditMode != EM_MASTERPAGE)
    {
        if ( mpDrawView->IsTextEdit() )
        {
            mpDrawView->SdrEndTextEdit();
        }
        pNewPage = ViewShell::CreateOrDuplicatePage (rRequest, ePageKind, pPage, nInsertPosition);
    }
    return pNewPage;
}

void DrawViewShell::ExecutePropPanelAttr (SfxRequest& rReq)
{
    if(SlideShow::IsRunning( GetViewShellBase() ))
        return;

    SdDrawDocument* pDoc = GetDoc();
    if (!pDoc || !mpDrawView)
        return;

    sal_uInt16 nSId = rReq.GetSlot();
    SfxItemSet aAttrs( pDoc->GetPool() );

    switch ( nSId )
    {
    case SID_TABLE_VERT_NONE:
    case SID_TABLE_VERT_CENTER:
    case SID_TABLE_VERT_BOTTOM:
        SdrTextVertAdjust eTVA = SDRTEXTVERTADJUST_TOP;
        if (nSId == SID_TABLE_VERT_CENTER)
            eTVA = SDRTEXTVERTADJUST_CENTER;
        else if (nSId == SID_TABLE_VERT_BOTTOM)
            eTVA = SDRTEXTVERTADJUST_BOTTOM;

        aAttrs.Put( SdrTextVertAdjustItem(eTVA) );
        mpDrawView->SetAttributes(aAttrs);

        break;
    }
}

void DrawViewShell::GetStatePropPanelAttr(SfxItemSet& rSet)
{
    SfxWhichIter    aIter( rSet );
    sal_uInt16          nWhich = aIter.FirstWhich();

    SdDrawDocument* pDoc = GetDoc();
    if (!pDoc || !mpDrawView)
        return;

    SfxItemSet aAttrs( pDoc->GetPool() );
    mpDrawView->GetAttributes( aAttrs );

    while ( nWhich )
    {
        sal_uInt16 nSlotId = SfxItemPool::IsWhich(nWhich)
            ? GetPool().GetSlotId(nWhich)
            : nWhich;
        switch ( nSlotId )
        {
            case SID_TABLE_VERT_NONE:
            case SID_TABLE_VERT_CENTER:
            case SID_TABLE_VERT_BOTTOM:
                bool bContour = false;
                SfxItemState eConState = aAttrs.GetItemState( SDRATTR_TEXT_CONTOURFRAME );
                if( eConState != SfxItemState::DONTCARE )
                {
                    bContour = static_cast<const SdrOnOffItem&>( aAttrs.Get( SDRATTR_TEXT_CONTOURFRAME ) ).GetValue();
                }
                if (bContour) break;

                SfxItemState eVState = aAttrs.GetItemState( SDRATTR_TEXT_VERTADJUST );
                //SfxItemState eHState = aAttrs.GetItemState( SDRATTR_TEXT_HORZADJUST );

                //if(SfxItemState::DONTCARE != eVState && SfxItemState::DONTCARE != eHState)
                if(SfxItemState::DONTCARE != eVState)
                {
                    SdrTextVertAdjust eTVA = (SdrTextVertAdjust)static_cast<const SdrTextVertAdjustItem&>(aAttrs.Get(SDRATTR_TEXT_VERTADJUST)).GetValue();
                    bool bSet = (nSlotId == SID_TABLE_VERT_NONE && eTVA == SDRTEXTVERTADJUST_TOP) ||
                            (nSlotId == SID_TABLE_VERT_CENTER && eTVA == SDRTEXTVERTADJUST_CENTER) ||
                            (nSlotId == SID_TABLE_VERT_BOTTOM && eTVA == SDRTEXTVERTADJUST_BOTTOM);
                    rSet.Put(SfxBoolItem(nSlotId, bSet));
                }
                else
                {
                    rSet.Put(SfxBoolItem(nSlotId, false));
                }
                break;
        }
        nWhich = aIter.NextWhich();
    }
}

} // end of namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
