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

#include <editeng/eeitem.hxx>

#include <sfx2/viewfrm.hxx>
#include <sfx2/request.hxx>
#include <sfx2/bindings.hxx>
#include <tools/urlobj.hxx>
#include <svx/svxdlg.hxx>
#include <svx/dialogs.hrc>
#include <svx/fmglob.hxx>
#include <svx/hlnkitem.hxx>
#include <svx/fontwork.hxx>
#include <svx/svdocapt.hxx>
#include <svx/svdoole2.hxx>
#include <svx/svdouno.hxx>
#include <svx/svdpage.hxx>
#include <svx/svdundo.hxx>
#include <svx/xdef.hxx>
#include <svx/xftsfit.hxx>
#include <vcl/msgbox.hxx>
#include <svx/extrusionbar.hxx>
#include <svx/fontworkbar.hxx>
#include <sfx2/docfile.hxx>

#include <com/sun/star/form/FormButtonType.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XPropertySetInfo.hpp>
#include <com/sun/star/awt/XControlModel.hpp>

#include "drawsh.hxx"
#include "drawview.hxx"
#include "viewdata.hxx"
#include "tabvwsh.hxx"
#include "docsh.hxx"
#include "scresid.hxx"
#include "undotab.hxx"
#include "drwlayer.hxx"
#include "userdat.hxx"
#include "postit.hxx"

#include "sc.hrc"

using namespace com::sun::star;

//------------------------------------------------------------------

void ScDrawShell::GetHLinkState( SfxItemSet& rSet )             //  Hyperlink
{
    ScDrawView* pView = pViewData->GetScDrawView();
    const SdrMarkList& rMarkList = pView->GetMarkedObjectList();
    sal_uLong nMarkCount = rMarkList.GetMarkCount();

        //  Hyperlink

    SvxHyperlinkItem aHLinkItem;

    if ( nMarkCount == 1 )              // URL-Button markiert ?
    {
        SdrObject* pObj = rMarkList.GetMark(0)->GetMarkedSdrObj();
        ScMacroInfo* pInfo = ScDrawLayer::GetMacroInfo( pObj );
        if ( pInfo && !pInfo->GetHlink().isEmpty() )
        {
            aHLinkItem.SetURL( pInfo->GetHlink() );
            aHLinkItem.SetInsertMode(HLINK_FIELD);
        }
        SdrUnoObj* pUnoCtrl = PTR_CAST(SdrUnoObj, pObj);
        if (pUnoCtrl && FmFormInventor == pUnoCtrl->GetObjInventor())
        {
            uno::Reference<awt::XControlModel> xControlModel = pUnoCtrl->GetUnoControlModel();
            OSL_ENSURE( xControlModel.is(), "UNO-Control ohne Model" );
            if( !xControlModel.is() )
                return;

            uno::Reference< beans::XPropertySet > xPropSet( xControlModel, uno::UNO_QUERY );
            uno::Reference< beans::XPropertySetInfo > xInfo = xPropSet->getPropertySetInfo();

            rtl::OUString sPropButtonType( "ButtonType" );
            rtl::OUString sPropTargetURL( "TargetURL" );
            rtl::OUString sPropTargetFrame( "TargetFrame" );
            rtl::OUString sPropLabel( "Label" );

            if(xInfo->hasPropertyByName( sPropButtonType ))
            {
                uno::Any aAny = xPropSet->getPropertyValue( sPropButtonType );
                form::FormButtonType eTmp;
                if ( (aAny >>= eTmp) && eTmp == form::FormButtonType_URL )
                {
                    rtl::OUString sTmp;
                    // Label
                    if(xInfo->hasPropertyByName( sPropLabel ))
                    {
                        aAny = xPropSet->getPropertyValue( sPropLabel );
                        if ( (aAny >>= sTmp) && !sTmp.isEmpty() )
                        {
                            aHLinkItem.SetName(sTmp);
                        }
                    }
                    // URL
                    if(xInfo->hasPropertyByName( sPropTargetURL ))
                    {
                        aAny = xPropSet->getPropertyValue( sPropTargetURL );
                        if ( (aAny >>= sTmp) && !sTmp.isEmpty() )
                        {
                            aHLinkItem.SetURL(sTmp);
                        }
                    }
                    // Target
                    if(xInfo->hasPropertyByName( sPropTargetFrame ))
                    {
                        aAny = xPropSet->getPropertyValue( sPropTargetFrame );
                        if ( (aAny >>= sTmp) && !sTmp.isEmpty() )
                        {
                            aHLinkItem.SetTargetFrame(sTmp);
                        }
                    }
                    aHLinkItem.SetInsertMode(HLINK_BUTTON);
                }
            }
        }
    }

    rSet.Put(aHLinkItem);
}

void ScDrawShell::ExecuteHLink( SfxRequest& rReq )
{
    const SfxItemSet* pReqArgs = rReq.GetArgs();

    sal_uInt16 nSlot = rReq.GetSlot();
    switch ( nSlot )
    {
        case SID_HYPERLINK_SETLINK:
            if( pReqArgs )
            {
                const SfxPoolItem* pItem;
                if ( pReqArgs->GetItemState( SID_HYPERLINK_SETLINK, sal_True, &pItem ) == SFX_ITEM_SET )
                {
                    const SvxHyperlinkItem* pHyper = (const SvxHyperlinkItem*) pItem;
                    const String& rName     = pHyper->GetName();
                    const String& rURL      = pHyper->GetURL();
                    const String& rTarget   = pHyper->GetTargetFrame();
                    SvxLinkInsertMode eMode = pHyper->GetInsertMode();

                    sal_Bool bDone = false;
                    if ( eMode == HLINK_FIELD || eMode == HLINK_BUTTON )
                    {
                        ScDrawView* pView = pViewData->GetScDrawView();
                        const SdrMarkList& rMarkList = pView->GetMarkedObjectList();
                        if ( rMarkList.GetMarkCount() == 1 )
                        {
                            SdrObject* pObj = rMarkList.GetMark(0)->GetMarkedSdrObj();
                            SdrUnoObj* pUnoCtrl = PTR_CAST(SdrUnoObj, pObj );
                            if (pUnoCtrl && FmFormInventor == pUnoCtrl->GetObjInventor())
                            {
                                uno::Reference<awt::XControlModel> xControlModel =
                                                        pUnoCtrl->GetUnoControlModel();
                                OSL_ENSURE( xControlModel.is(), "UNO-Control ohne Model" );
                                if( !xControlModel.is() )
                                    return;

                                uno::Reference< beans::XPropertySet > xPropSet( xControlModel, uno::UNO_QUERY );
                                uno::Reference< beans::XPropertySetInfo > xInfo = xPropSet->getPropertySetInfo();

                                rtl::OUString sPropTargetURL( "TargetURL" );

                                // Darf man eine URL an dem Objekt setzen?
                                if (xInfo->hasPropertyByName( sPropTargetURL ))
                                {
                                    // Ja!

                                    rtl::OUString sPropButtonType( "ButtonType");
                                    rtl::OUString sPropTargetFrame( "TargetFrame" );
                                    rtl::OUString sPropLabel( "Label" );

                                    uno::Any aAny;
                                    if ( xInfo->hasPropertyByName( sPropLabel ) )
                                    {
                                        aAny <<= rtl::OUString(rName);
                                        xPropSet->setPropertyValue( sPropLabel, aAny );
                                    }

                                    ::rtl::OUString aTmp = INetURLObject::GetAbsURL( pViewData->GetDocShell()->GetMedium()->GetBaseURL(), rURL );
                                    aAny <<= aTmp;
                                    xPropSet->setPropertyValue( sPropTargetURL, aAny );

                                    if( rTarget.Len() && xInfo->hasPropertyByName( sPropTargetFrame ) )
                                    {
                                        aAny <<= rtl::OUString(rTarget);
                                        xPropSet->setPropertyValue( sPropTargetFrame, aAny );
                                    }

                                    if ( xInfo->hasPropertyByName( sPropButtonType ) )
                                    {
                                        form::FormButtonType eButtonType = form::FormButtonType_URL;
                                        aAny <<= eButtonType;
                                        xPropSet->setPropertyValue( sPropButtonType, aAny );
                                    }

                                    //! Undo ???
                                    pViewData->GetDocShell()->SetDocumentModified();
                                    bDone = sal_True;
                                }
                            }
                            else
                            {
                                SetHlinkForObject( pObj, rURL );
                                bDone = sal_True;
                            }
                        }
                    }

                    if (!bDone)
                        pViewData->GetViewShell()->
                            InsertURL( rName, rURL, rTarget, (sal_uInt16) eMode );

                    //  InsertURL an der ViewShell schaltet bei "Text" die DrawShell ab !!!
                }
            }
            break;
        default:
            OSL_FAIL("falscher Slot");
    }
}

sal_uInt16 ScGetFontWorkId();       // wegen CLOOKs - in drtxtob2

//------------------------------------------------------------------

//
//          Funktionen auf Drawing-Objekten
//

void ScDrawShell::ExecDrawFunc( SfxRequest& rReq )
{
    SfxBindings& rBindings = pViewData->GetBindings();
    ScTabView*   pTabView  = pViewData->GetView();
    ScDrawView*  pView     = pTabView->GetScDrawView();
    const SfxItemSet *pArgs = rReq.GetArgs();
    sal_uInt16 nSlotId = rReq.GetSlot();

    //!!!
    // wer weiss, wie lange das funktioniert? (->vom Abreisscontrol funktioniert es)
    //
    if (nSlotId == SID_OBJECT_ALIGN && pArgs)
        nSlotId = SID_OBJECT_ALIGN + ((SfxEnumItem&)pArgs->Get(SID_OBJECT_ALIGN)).GetValue() + 1;

    switch (nSlotId)
    {
        case SID_OBJECT_HEAVEN:
            pView->SetMarkedToLayer( SC_LAYER_FRONT );
            rBindings.Invalidate(SID_OBJECT_HEAVEN);
            rBindings.Invalidate(SID_OBJECT_HELL);
            break;
        case SID_OBJECT_HELL:
            pView->SetMarkedToLayer( SC_LAYER_BACK );
            rBindings.Invalidate(SID_OBJECT_HEAVEN);
            rBindings.Invalidate(SID_OBJECT_HELL);
            //  leave draw shell if nothing selected (layer may be locked)
            pViewData->GetViewShell()->UpdateDrawShell();
            break;

        case SID_FRAME_TO_TOP:
            pView->PutMarkedToTop();
            break;
        case SID_FRAME_TO_BOTTOM:
            pView->PutMarkedToBtm();
            break;
        case SID_FRAME_UP:
            pView->MovMarkedToTop();
            break;
        case SID_FRAME_DOWN:
            pView->MovMarkedToBtm();
            break;

        case SID_GROUP:
            pView->GroupMarked();
            break;
        case SID_UNGROUP:
            pView->UnGroupMarked();
            break;
        case SID_ENTER_GROUP:
            pView->EnterMarkedGroup();
            break;
        case SID_LEAVE_GROUP:
            pView->LeaveOneGroup();
            break;

        case SID_MIRROR_HORIZONTAL:
            pView->MirrorAllMarkedHorizontal();
            break;
        case SID_MIRROR_VERTICAL:
            pView->MirrorAllMarkedVertical();
            break;

        case SID_OBJECT_ALIGN_LEFT:
        case SID_ALIGN_ANY_LEFT:
            if (pView->IsAlignPossible())
                pView->AlignMarkedObjects(SDRHALIGN_LEFT, SDRVALIGN_NONE);
            break;
        case SID_OBJECT_ALIGN_CENTER:
        case SID_ALIGN_ANY_HCENTER:
            if (pView->IsAlignPossible())
                pView->AlignMarkedObjects(SDRHALIGN_CENTER, SDRVALIGN_NONE);
            break;
        case SID_OBJECT_ALIGN_RIGHT:
        case SID_ALIGN_ANY_RIGHT:
            if (pView->IsAlignPossible())
                pView->AlignMarkedObjects(SDRHALIGN_RIGHT, SDRVALIGN_NONE);
            break;
        case SID_OBJECT_ALIGN_UP:
        case SID_ALIGN_ANY_TOP:
            if (pView->IsAlignPossible())
                pView->AlignMarkedObjects(SDRHALIGN_NONE, SDRVALIGN_TOP);
            break;
        case SID_OBJECT_ALIGN_MIDDLE:
        case SID_ALIGN_ANY_VCENTER:
            if (pView->IsAlignPossible())
                pView->AlignMarkedObjects(SDRHALIGN_NONE, SDRVALIGN_CENTER);
            break;
        case SID_OBJECT_ALIGN_DOWN:
        case SID_ALIGN_ANY_BOTTOM:
            if (pView->IsAlignPossible())
                pView->AlignMarkedObjects(SDRHALIGN_NONE, SDRVALIGN_BOTTOM);
            break;

        case SID_DELETE:
        case SID_DELETE_CONTENTS:
            pView->DeleteMarked();
            pViewData->GetViewShell()->UpdateDrawShell();
        break;

        case SID_CUT:
            pView->DoCut();
            pViewData->GetViewShell()->UpdateDrawShell();
            break;

        case SID_COPY:
            pView->DoCopy();
            break;

        case SID_PASTE:
            OSL_FAIL( "SdrView::PasteClipboard not supported anymore" );
            break;

        case SID_SELECTALL:
            pView->MarkAll();
            break;

        case SID_ANCHOR_PAGE:
            pView->SetPageAnchored();
            rBindings.Invalidate( SID_ANCHOR_PAGE );
            rBindings.Invalidate( SID_ANCHOR_CELL );
            break;

        case SID_ANCHOR_CELL:
            pView->SetCellAnchored();
            rBindings.Invalidate( SID_ANCHOR_PAGE );
            rBindings.Invalidate( SID_ANCHOR_CELL );
            break;

        case SID_ANCHOR_TOGGLE:
            {
                switch( pView->GetAnchorType() )
                {
                    case SCA_CELL:
                    pView->SetPageAnchored();
                    break;
                    default:
                    pView->SetCellAnchored();
                    break;
                }
            }
            rBindings.Invalidate( SID_ANCHOR_PAGE );
            rBindings.Invalidate( SID_ANCHOR_CELL );
            break;

        case SID_OBJECT_ROTATE:
            {
                SdrDragMode eMode;
                if (pView->GetDragMode() == SDRDRAG_ROTATE)
                    eMode = SDRDRAG_MOVE;
                else
                    eMode = SDRDRAG_ROTATE;
                pView->SetDragMode( eMode );
                rBindings.Invalidate( SID_OBJECT_ROTATE );
                rBindings.Invalidate( SID_OBJECT_MIRROR );
                if (eMode == SDRDRAG_ROTATE && !pView->IsFrameDragSingles())
                {
                    pView->SetFrameDragSingles( sal_True );
                    rBindings.Invalidate( SID_BEZIER_EDIT );
                }
            }
            break;
        case SID_OBJECT_MIRROR:
            {
                SdrDragMode eMode;
                if (pView->GetDragMode() == SDRDRAG_MIRROR)
                    eMode = SDRDRAG_MOVE;
                else
                    eMode = SDRDRAG_MIRROR;
                pView->SetDragMode( eMode );
                rBindings.Invalidate( SID_OBJECT_ROTATE );
                rBindings.Invalidate( SID_OBJECT_MIRROR );
                if (eMode == SDRDRAG_MIRROR && !pView->IsFrameDragSingles())
                {
                    pView->SetFrameDragSingles( sal_True );
                    rBindings.Invalidate( SID_BEZIER_EDIT );
                }
            }
            break;
        case SID_BEZIER_EDIT:
            {
                sal_Bool bOld = pView->IsFrameDragSingles();
                pView->SetFrameDragSingles( !bOld );
                rBindings.Invalidate( SID_BEZIER_EDIT );
                if (bOld && pView->GetDragMode() != SDRDRAG_MOVE)
                {
                    pView->SetDragMode( SDRDRAG_MOVE );
                    rBindings.Invalidate( SID_OBJECT_ROTATE );
                    rBindings.Invalidate( SID_OBJECT_MIRROR );
                }
            }
            break;

        case SID_FONTWORK:
        {
            sal_uInt16 nId = ScGetFontWorkId();
            SfxViewFrame* pViewFrm = pViewData->GetViewShell()->GetViewFrame();

            if ( rReq.GetArgs() )
                pViewFrm->SetChildWindow( nId,
                                           ((const SfxBoolItem&)
                                            (rReq.GetArgs()->Get(SID_FONTWORK))).
                                                GetValue() );
            else
                pViewFrm->ToggleChildWindow( nId );

            rBindings.Invalidate( SID_FONTWORK );
            rReq.Done();
        }
        break;

        case SID_ORIGINALSIZE:
            pView->SetMarkedOriginalSize();
            break;

        case SID_ENABLE_HYPHENATION:
            {
                SFX_REQUEST_ARG( rReq, pItem, SfxBoolItem, SID_ENABLE_HYPHENATION, false);
                if( pItem )
                {
                    SfxItemSet aSet( GetPool(), EE_PARA_HYPHENATE, EE_PARA_HYPHENATE );
                    sal_Bool bValue = ( (const SfxBoolItem*) pItem)->GetValue();
                    aSet.Put( SfxBoolItem( EE_PARA_HYPHENATE, bValue ) );
                    pView->SetAttributes( aSet );
                }
                rReq.Done();
            }
            break;

        case SID_RENAME_OBJECT:
            {
                if(1L == pView->GetMarkedObjectCount())
                {
                    // #i68101#
                    SdrObject* pSelected = pView->GetMarkedObjectByIndex(0L);
                    OSL_ENSURE(pSelected, "ScDrawShell::ExecDrawFunc: nMarkCount, but no object (!)");

                    if(SC_LAYER_INTERN != pSelected->GetLayer())
                    {
                        rtl::OUString aName = pSelected->GetName();

                        SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
                        OSL_ENSURE(pFact, "Dialogdiet fail!");
                        AbstractSvxObjectNameDialog* pDlg = pFact->CreateSvxObjectNameDialog(NULL, aName);
                        OSL_ENSURE(pDlg, "Dialogdiet fail!");

                        pDlg->SetCheckNameHdl(LINK(this, ScDrawShell, NameObjectHdl));

                        if(RET_OK == pDlg->Execute())
                        {
                            ScDocShell* pDocSh = pViewData->GetDocShell();
                            String aTmp;
                            pDlg->GetName(aTmp);
                            aName = aTmp;

                            if (!aName.equals(pSelected->GetName()))
                            {
                                // handle name change
                                const sal_uInt16 nObjType(pSelected->GetObjIdentifier());

                                if (OBJ_GRAF == nObjType && aName.isEmpty())
                                {
                                    //  graphics objects must have names
                                    //  (all graphics are supposed to be in the navigator)
                                    ScDrawLayer* pModel = pViewData->GetDocument()->GetDrawLayer();

                                    if(pModel)
                                    {
                                        aName = pModel->GetNewGraphicName();
                                    }
                                }

                                //  An undo action for renaming is missing in svdraw (99363).
                                //  For OLE objects (which can be identified using the persist name),
                                //  ScUndoRenameObject can be used until there is a common action for all objects.
                                if(OBJ_OLE2 == nObjType)
                                {
                                    const String aPersistName = static_cast<SdrOle2Obj*>(pSelected)->GetPersistName();

                                    if(aPersistName.Len())
                                    {
                                        pDocSh->GetUndoManager()->AddUndoAction(
                                            new ScUndoRenameObject(pDocSh, aPersistName, pSelected->GetName(), aName));
                                    }
                                }

                                // set new name
                                pSelected->SetName(aName);
                            }

                            // ChartListenerCollectionNeedsUpdate is needed for Navigator update
                            pDocSh->GetDocument()->SetChartListenerCollectionNeedsUpdate( sal_True );
                            pDocSh->SetDrawModified();
                        }

                        delete pDlg;
                    }
                }
                break;
            }

        // #i68101#
        case SID_TITLE_DESCRIPTION_OBJECT:
            {
                if(1L == pView->GetMarkedObjectCount())
                {
                    SdrObject* pSelected = pView->GetMarkedObjectByIndex(0L);
                    OSL_ENSURE(pSelected, "ScDrawShell::ExecDrawFunc: nMarkCount, but no object (!)");

                    if(SC_LAYER_INTERN != pSelected->GetLayer())
                    {
                        String aTitle(pSelected->GetTitle());
                        String aDescription(pSelected->GetDescription());

                        SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
                        OSL_ENSURE(pFact, "Dialogdiet fail!");
                        AbstractSvxObjectTitleDescDialog* pDlg = pFact->CreateSvxObjectTitleDescDialog(NULL, aTitle, aDescription);
                        OSL_ENSURE(pDlg, "Dialogdiet fail!");

                        if(RET_OK == pDlg->Execute())
                        {
                            ScDocShell* pDocSh = pViewData->GetDocShell();

                            // handle Title and Description
                            pDlg->GetTitle(aTitle);
                            pDlg->GetDescription(aDescription);
                            pSelected->SetTitle(aTitle);
                            pSelected->SetDescription(aDescription);

                            // ChartListenerCollectionNeedsUpdate is needed for Navigator update
                            pDocSh->GetDocument()->SetChartListenerCollectionNeedsUpdate( sal_True );
                            pDocSh->SetDrawModified();
                        }

                        delete pDlg;
                    }
                }
                break;
            }

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
            svx::ExtrusionBar::execute( pView, rReq, rBindings );
            rReq.Ignore ();
            break;

        case SID_FONTWORK_SHAPE:
        case SID_FONTWORK_SHAPE_TYPE:
        case SID_FONTWORK_ALIGNMENT:
        case SID_FONTWORK_SAME_LETTER_HEIGHTS:
        case SID_FONTWORK_CHARACTER_SPACING:
        case SID_FONTWORK_KERN_CHARACTER_PAIRS:
        case SID_FONTWORK_CHARACTER_SPACING_FLOATER:
        case SID_FONTWORK_ALIGNMENT_FLOATER:
        case SID_FONTWORK_CHARACTER_SPACING_DIALOG:
            svx::FontworkBar::execute( pView, rReq, rBindings );
            rReq.Ignore ();
            break;

        default:
            break;
    }
}

IMPL_LINK( ScDrawShell, NameObjectHdl, AbstractSvxNameDialog*, pDialog )
{
    String aName;

    if( pDialog )
        pDialog->GetName( aName );

    ScDrawLayer* pModel = pViewData->GetDocument()->GetDrawLayer();
    if ( aName.Len() && pModel )
    {
        SCTAB nDummyTab;
        if ( pModel->GetNamedObject( aName, 0, nDummyTab ) )
        {
            // existing object found -> name invalid
            return 0;
        }
    }

    return 1;   // name is valid
}

//------------------------------------------------------------------

void ScDrawShell::ExecFormText(SfxRequest& rReq)
{
    ScDrawView*         pDrView     = pViewData->GetScDrawView();
    const SdrMarkList&  rMarkList   = pDrView->GetMarkedObjectList();

    if ( rMarkList.GetMarkCount() == 1 && rReq.GetArgs() )
    {
        const SfxItemSet& rSet = *rReq.GetArgs();
        const SfxPoolItem* pItem;

        if ( pDrView->IsTextEdit() )
            pDrView->ScEndTextEdit();

        if (    SFX_ITEM_SET ==
                rSet.GetItemState(XATTR_FORMTXTSTDFORM, sal_True, &pItem)
             && XFTFORM_NONE !=
                ((const XFormTextStdFormItem*) pItem)->GetValue() )
        {

            sal_uInt16 nId              = SvxFontWorkChildWindow::GetChildWindowId();
            SfxViewFrame* pViewFrm  = pViewData->GetViewShell()->GetViewFrame();
            SvxFontWorkDialog* pDlg = (SvxFontWorkDialog*)
                                       (pViewFrm->
                                            GetChildWindow(nId)->GetWindow());

            pDlg->CreateStdFormObj(*pDrView, *pDrView->GetSdrPageView(),
                                    rSet, *rMarkList.GetMark(0)->GetMarkedSdrObj(),
                                   ((const XFormTextStdFormItem*) pItem)->
                                   GetValue());
        }
        else
            pDrView->SetAttributes(rSet);
    }
}

//------------------------------------------------------------------

void ScDrawShell::ExecFormatPaintbrush( SfxRequest& rReq )
{
    ScViewFunc* pView = pViewData->GetView();
    if ( pView->HasPaintBrush() )
    {
        // cancel paintbrush mode
        pView->ResetBrushDocument();
    }
    else
    {
        sal_Bool bLock = false;
        const SfxItemSet *pArgs = rReq.GetArgs();
        if( pArgs && pArgs->Count() >= 1 )
            bLock = static_cast<const SfxBoolItem&>(pArgs->Get(SID_FORMATPAINTBRUSH)).GetValue();

        ScDrawView* pDrawView = pViewData->GetScDrawView();
        if ( pDrawView && pDrawView->AreObjectsMarked() )
        {
            sal_Bool bOnlyHardAttr = sal_True;
            SfxItemSet* pItemSet = new SfxItemSet( pDrawView->GetAttrFromMarked(bOnlyHardAttr) );
            pView->SetDrawBrushSet( pItemSet, bLock );
        }
    }
}

void ScDrawShell::StateFormatPaintbrush( SfxItemSet& rSet )
{
    ScDrawView* pDrawView = pViewData->GetScDrawView();
    sal_Bool bSelection = pDrawView && pDrawView->AreObjectsMarked();
    sal_Bool bHasPaintBrush = pViewData->GetView()->HasPaintBrush();

    if ( !bHasPaintBrush && !bSelection )
        rSet.DisableItem( SID_FORMATPAINTBRUSH );
    else
        rSet.Put( SfxBoolItem( SID_FORMATPAINTBRUSH, bHasPaintBrush ) );
}

ScDrawView* ScDrawShell::GetDrawView()
{
    return pViewData->GetView()->GetScDrawView();
}




/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
