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
#include <cliputil.hxx>
#include <svx/svxdlg.hxx>
#include <svx/hlnkitem.hxx>
#include <svx/svdoole2.hxx>
#include <svx/svdouno.hxx>
#include <svx/extrusionbar.hxx>
#include <svx/fontworkbar.hxx>
#include <sfx2/docfile.hxx>

#include <com/sun/star/form/FormButtonType.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XPropertySetInfo.hpp>

#include <drawsh.hxx>
#include <drawview.hxx>
#include <viewdata.hxx>
#include <tabvwsh.hxx>
#include <docsh.hxx>
#include <undotab.hxx>
#include <drwlayer.hxx>
#include <userdat.hxx>
#include <drtxtob.hxx>
#include <memory>

#include <sc.hrc>

using namespace com::sun::star;

void ScDrawShell::GetHLinkState( SfxItemSet& rSet )             //  Hyperlink
{
    ScDrawView* pView = rViewData.GetScDrawView();
    const SdrMarkList& rMarkList = pView->GetMarkedObjectList();

        //  Hyperlink

    SvxHyperlinkItem aHLinkItem;

    if ( rMarkList.GetMarkCount() == 1 )              // URL-Button marked ?
    {
        SdrObject* pObj = rMarkList.GetMark(0)->GetMarkedSdrObj();
        ScMacroInfo* pInfo = ScDrawLayer::GetMacroInfo( pObj );
        if ( pInfo && !pInfo->GetHlink().isEmpty() )
        {
            aHLinkItem.SetURL( pInfo->GetHlink() );
            aHLinkItem.SetInsertMode(HLINK_FIELD);
        }
        SdrUnoObj* pUnoCtrl = dynamic_cast<SdrUnoObj*>( pObj );
        if (pUnoCtrl && SdrInventor::FmForm == pUnoCtrl->GetObjInventor())
        {
            const uno::Reference<awt::XControlModel>& xControlModel = pUnoCtrl->GetUnoControlModel();
            OSL_ENSURE( xControlModel.is(), "UNO-Control without model" );
            if( !xControlModel.is() )
                return;

            uno::Reference< beans::XPropertySet > xPropSet( xControlModel, uno::UNO_QUERY );
            uno::Reference< beans::XPropertySetInfo > xInfo = xPropSet->getPropertySetInfo();

            OUString sPropButtonType( "ButtonType" );

            if(xInfo->hasPropertyByName( sPropButtonType ))
            {
                uno::Any aAny = xPropSet->getPropertyValue( sPropButtonType );
                form::FormButtonType eTmp;
                if ( (aAny >>= eTmp) && eTmp == form::FormButtonType_URL )
                {
                    OUString sTmp;
                    // Label
                    OUString sPropLabel( "Label" );
                    if(xInfo->hasPropertyByName( sPropLabel ))
                    {
                        aAny = xPropSet->getPropertyValue( sPropLabel );
                        if ( (aAny >>= sTmp) && !sTmp.isEmpty() )
                        {
                            aHLinkItem.SetName(sTmp);
                        }
                    }
                    // URL
                    OUString sPropTargetURL( "TargetURL" );
                    if(xInfo->hasPropertyByName( sPropTargetURL ))
                    {
                        aAny = xPropSet->getPropertyValue( sPropTargetURL );
                        if ( (aAny >>= sTmp) && !sTmp.isEmpty() )
                        {
                            aHLinkItem.SetURL(sTmp);
                        }
                    }
                    // Target
                    OUString sPropTargetFrame( "TargetFrame" );
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

void ScDrawShell::ExecuteHLink( const SfxRequest& rReq )
{
    const SfxItemSet* pReqArgs = rReq.GetArgs();

    sal_uInt16 nSlot = rReq.GetSlot();
    switch ( nSlot )
    {
        case SID_HYPERLINK_SETLINK:
            if( pReqArgs )
            {
                const SfxPoolItem* pItem;
                if ( pReqArgs->GetItemState( SID_HYPERLINK_SETLINK, true, &pItem ) == SfxItemState::SET )
                {
                    const SvxHyperlinkItem* pHyper = static_cast<const SvxHyperlinkItem*>(pItem);
                    const OUString& rName     = pHyper->GetName();
                    const OUString& rURL      = pHyper->GetURL();
                    const OUString& rTarget   = pHyper->GetTargetFrame();
                    SvxLinkInsertMode eMode = pHyper->GetInsertMode();

                    bool bDone = false;
                    if ( eMode == HLINK_FIELD || eMode == HLINK_BUTTON )
                    {
                        ScDrawView* pView = rViewData.GetScDrawView();
                        const SdrMarkList& rMarkList = pView->GetMarkedObjectList();
                        if ( rMarkList.GetMarkCount() == 1 )
                        {
                            SdrObject* pObj = rMarkList.GetMark(0)->GetMarkedSdrObj();
                            SdrUnoObj* pUnoCtrl = dynamic_cast<SdrUnoObj*>( pObj  );
                            if (pUnoCtrl && SdrInventor::FmForm == pUnoCtrl->GetObjInventor())
                            {
                                const uno::Reference<awt::XControlModel>& xControlModel =
                                                        pUnoCtrl->GetUnoControlModel();
                                OSL_ENSURE( xControlModel.is(), "UNO-Control without model" );
                                if( !xControlModel.is() )
                                    return;

                                uno::Reference< beans::XPropertySet > xPropSet( xControlModel, uno::UNO_QUERY );
                                uno::Reference< beans::XPropertySetInfo > xInfo = xPropSet->getPropertySetInfo();

                                OUString sPropTargetURL( "TargetURL" );

                                // Is it possible to set a URL in the object?
                                if (xInfo->hasPropertyByName( sPropTargetURL ))
                                {

                                    OUString sPropButtonType( "ButtonType");
                                    OUString sPropTargetFrame( "TargetFrame" );
                                    OUString sPropLabel( "Label" );

                                    if ( xInfo->hasPropertyByName( sPropLabel ) )
                                    {
                                        xPropSet->setPropertyValue( sPropLabel, uno::Any(rName) );
                                    }

                                    OUString aTmp = INetURLObject::GetAbsURL( rViewData.GetDocShell()->GetMedium()->GetBaseURL(), rURL );
                                    xPropSet->setPropertyValue( sPropTargetURL, uno::Any(aTmp) );

                                    if( !rTarget.isEmpty() && xInfo->hasPropertyByName( sPropTargetFrame ) )
                                    {
                                        xPropSet->setPropertyValue( sPropTargetFrame, uno::Any(rTarget) );
                                    }

                                    if ( xInfo->hasPropertyByName( sPropButtonType ) )
                                    {
                                        xPropSet->setPropertyValue( sPropButtonType, uno::Any(form::FormButtonType_URL) );
                                    }

                                    //! Undo ???
                                    rViewData.GetDocShell()->SetDocumentModified();
                                    bDone = true;
                                }
                            }
                            else
                            {
                                SetHlinkForObject( pObj, rURL );
                                bDone = true;
                            }
                        }
                    }

                    if (!bDone)
                        rViewData.GetViewShell()->
                            InsertURL( rName, rURL, rTarget, static_cast<sal_uInt16>(eMode) );

                    //  If "text" is received by InsertURL of ViewShell, then the DrawShell is turned off !!!
                }
            }
            break;
        default:
            OSL_FAIL("wrong slot");
    }
}

//          Functions on Drawing-Objects

void ScDrawShell::ExecDrawFunc( SfxRequest& rReq )
{
    SfxBindings& rBindings = rViewData.GetBindings();
    ScTabView*   pTabView  = rViewData.GetView();
    ScDrawView*  pView     = pTabView->GetScDrawView();
    sal_uInt16 nSlotId = rReq.GetSlot();

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
            rViewData.GetViewShell()->UpdateDrawShell();
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
        case SID_FLIP_HORIZONTAL:
            pView->MirrorAllMarkedHorizontal();
            rBindings.Invalidate( SID_ATTR_TRANSFORM_ANGLE );
            break;
        case SID_MIRROR_VERTICAL:
        case SID_FLIP_VERTICAL:
            pView->MirrorAllMarkedVertical();
            rBindings.Invalidate( SID_ATTR_TRANSFORM_ANGLE );
            break;

        case SID_OBJECT_ALIGN_LEFT:
        case SID_ALIGN_ANY_LEFT:
            if (pView->IsAlignPossible())
                pView->AlignMarkedObjects(SdrHorAlign::Left, SdrVertAlign::NONE);
            break;
        case SID_OBJECT_ALIGN_CENTER:
        case SID_ALIGN_ANY_HCENTER:
            if (pView->IsAlignPossible())
                pView->AlignMarkedObjects(SdrHorAlign::Center, SdrVertAlign::NONE);
            break;
        case SID_OBJECT_ALIGN_RIGHT:
        case SID_ALIGN_ANY_RIGHT:
            if (pView->IsAlignPossible())
                pView->AlignMarkedObjects(SdrHorAlign::Right, SdrVertAlign::NONE);
            break;
        case SID_OBJECT_ALIGN_UP:
        case SID_ALIGN_ANY_TOP:
            if (pView->IsAlignPossible())
                pView->AlignMarkedObjects(SdrHorAlign::NONE, SdrVertAlign::Top);
            break;
        case SID_OBJECT_ALIGN_MIDDLE:
        case SID_ALIGN_ANY_VCENTER:
            if (pView->IsAlignPossible())
                pView->AlignMarkedObjects(SdrHorAlign::NONE, SdrVertAlign::Center);
            break;
        case SID_OBJECT_ALIGN_DOWN:
        case SID_ALIGN_ANY_BOTTOM:
            if (pView->IsAlignPossible())
                pView->AlignMarkedObjects(SdrHorAlign::NONE, SdrVertAlign::Bottom);
            break;

        case SID_DELETE:
        case SID_DELETE_CONTENTS:
            pView->DeleteMarked();
            rViewData.GetViewShell()->UpdateDrawShell();
        break;

        case SID_CUT:
            pView->DoCut();
            rViewData.GetViewShell()->UpdateDrawShell();
            break;

        case SID_COPY:
            pView->DoCopy();
            break;

        case SID_PASTE:
            ScClipUtil::PasteFromClipboard(&GetViewData(), GetViewData().GetViewShell(), true);
            break;

        case SID_SELECTALL:
            pView->MarkAll();
            break;

        case SID_ANCHOR_PAGE:
            pView->SetPageAnchored();
            rBindings.Invalidate( SID_ANCHOR_PAGE );
            rBindings.Invalidate( SID_ANCHOR_CELL );
            rBindings.Invalidate( SID_ANCHOR_CELL_RESIZE );
            break;

        case SID_ANCHOR_CELL:
            pView->SetCellAnchored(false);
            rBindings.Invalidate( SID_ANCHOR_PAGE );
            rBindings.Invalidate( SID_ANCHOR_CELL );
            rBindings.Invalidate( SID_ANCHOR_CELL_RESIZE );
            break;

        case SID_ANCHOR_CELL_RESIZE:
            pView->SetCellAnchored(true);
            rBindings.Invalidate( SID_ANCHOR_PAGE );
            rBindings.Invalidate( SID_ANCHOR_CELL );
            rBindings.Invalidate( SID_ANCHOR_CELL_RESIZE );
            break;

        case SID_ANCHOR_TOGGLE:
            {
                switch( pView->GetAnchorType() )
                {
                    case SCA_CELL:
                    case SCA_CELL_RESIZE:
                    pView->SetPageAnchored();
                    break;
                    default:
                    pView->SetCellAnchored(false);
                    break;
                }
            }
            rBindings.Invalidate( SID_ANCHOR_PAGE );
            rBindings.Invalidate( SID_ANCHOR_CELL );
            rBindings.Invalidate( SID_ANCHOR_CELL_RESIZE );
            break;

        case SID_OBJECT_ROTATE:
            {
                SdrDragMode eMode;
                if (pView->GetDragMode() == SdrDragMode::Rotate)
                    eMode = SdrDragMode::Move;
                else
                    eMode = SdrDragMode::Rotate;
                pView->SetDragMode( eMode );
                rBindings.Invalidate( SID_OBJECT_ROTATE );
                rBindings.Invalidate( SID_OBJECT_MIRROR );
                if (eMode == SdrDragMode::Rotate && !pView->IsFrameDragSingles())
                {
                    pView->SetFrameDragSingles();
                    rBindings.Invalidate( SID_BEZIER_EDIT );
                }
            }
            break;
        case SID_OBJECT_MIRROR:
            {
                SdrDragMode eMode;
                if (pView->GetDragMode() == SdrDragMode::Mirror)
                    eMode = SdrDragMode::Move;
                else
                    eMode = SdrDragMode::Mirror;
                pView->SetDragMode( eMode );
                rBindings.Invalidate( SID_OBJECT_ROTATE );
                rBindings.Invalidate( SID_OBJECT_MIRROR );
                if (eMode == SdrDragMode::Mirror && !pView->IsFrameDragSingles())
                {
                    pView->SetFrameDragSingles();
                    rBindings.Invalidate( SID_BEZIER_EDIT );
                }
            }
            break;
        case SID_BEZIER_EDIT:
            {
                bool bOld = pView->IsFrameDragSingles();
                pView->SetFrameDragSingles( !bOld );
                rBindings.Invalidate( SID_BEZIER_EDIT );
                if (bOld && pView->GetDragMode() != SdrDragMode::Move)
                {
                    pView->SetDragMode( SdrDragMode::Move );
                    rBindings.Invalidate( SID_OBJECT_ROTATE );
                    rBindings.Invalidate( SID_OBJECT_MIRROR );
                }
            }
            break;

        case SID_FONTWORK:
        {
            sal_uInt16 nId = ScGetFontWorkId();
            SfxViewFrame* pViewFrm = rViewData.GetViewShell()->GetViewFrame();

            if ( rReq.GetArgs() )
                pViewFrm->SetChildWindow( nId,
                                           static_cast<const SfxBoolItem&>(
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

        case SID_FITCELLSIZE:
            pView->FitToCellSize();
            break;

        case SID_ENABLE_HYPHENATION:
            {
                const SfxBoolItem* pItem = rReq.GetArg<SfxBoolItem>(SID_ENABLE_HYPHENATION);
                if( pItem )
                {
                    SfxItemSet aSet( GetPool(), svl::Items<EE_PARA_HYPHENATE, EE_PARA_HYPHENATE>{} );
                    bool bValue = pItem->GetValue();
                    aSet.Put( SfxBoolItem( EE_PARA_HYPHENATE, bValue ) );
                    pView->SetAttributes( aSet );
                }
                rReq.Done();
            }
            break;

        case SID_RENAME_OBJECT:
            {
                if(1 == pView->GetMarkedObjectCount())
                {
                    // #i68101#
                    SdrObject* pSelected = pView->GetMarkedObjectByIndex(0);
                    OSL_ENSURE(pSelected, "ScDrawShell::ExecDrawFunc: nMarkCount, but no object (!)");

                    if(SC_LAYER_INTERN != pSelected->GetLayer())
                    {
                        OUString aName = pSelected->GetName();

                        SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
                        vcl::Window* pWin = rViewData.GetActiveWin();
                        ScopedVclPtr<AbstractSvxObjectNameDialog> pDlg(pFact->CreateSvxObjectNameDialog(pWin ? pWin->GetFrameWeld() : nullptr, aName));

                        pDlg->SetCheckNameHdl(LINK(this, ScDrawShell, NameObjectHdl));

                        if(RET_OK == pDlg->Execute())
                        {
                            ScDocShell* pDocSh = rViewData.GetDocShell();
                            pDlg->GetName(aName);

                            if (aName != pSelected->GetName())
                            {
                                // handle name change
                                const sal_uInt16 nObjType(pSelected->GetObjIdentifier());

                                if (OBJ_GRAF == nObjType && aName.isEmpty())
                                {
                                    //  graphics objects must have names
                                    //  (all graphics are supposed to be in the navigator)
                                    ScDrawLayer* pModel = rViewData.GetDocument().GetDrawLayer();

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
                                    const OUString aPersistName = static_cast<SdrOle2Obj*>(pSelected)->GetPersistName();

                                    if(!aPersistName.isEmpty())
                                    {
                                        pDocSh->GetUndoManager()->AddUndoAction(
                                            std::make_unique<ScUndoRenameObject>(pDocSh, aPersistName, pSelected->GetName(), aName));
                                    }
                                }

                                // set new name
                                pSelected->SetName(aName);
                            }

                            // ChartListenerCollectionNeedsUpdate is needed for Navigator update
                            pDocSh->GetDocument().SetChartListenerCollectionNeedsUpdate( true );
                            pDocSh->SetDrawModified();
                        }
                    }
                }
                break;
            }

        // #i68101#
        case SID_TITLE_DESCRIPTION_OBJECT:
            {
                if(1 == pView->GetMarkedObjectCount())
                {
                    SdrObject* pSelected = pView->GetMarkedObjectByIndex(0);
                    OSL_ENSURE(pSelected, "ScDrawShell::ExecDrawFunc: nMarkCount, but no object (!)");

                    if(SC_LAYER_INTERN != pSelected->GetLayer())
                    {
                        OUString aTitle(pSelected->GetTitle());
                        OUString aDescription(pSelected->GetDescription());

                        SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
                        vcl::Window* pWin = rViewData.GetActiveWin();
                        ScopedVclPtr<AbstractSvxObjectTitleDescDialog> pDlg(pFact->CreateSvxObjectTitleDescDialog(
                                    pWin ? pWin->GetFrameWeld() : nullptr, aTitle, aDescription));

                        if(RET_OK == pDlg->Execute())
                        {
                            ScDocShell* pDocSh = rViewData.GetDocShell();

                            // handle Title and Description
                            pDlg->GetTitle(aTitle);
                            pDlg->GetDescription(aDescription);
                            pSelected->SetTitle(aTitle);
                            pSelected->SetDescription(aDescription);

                            // ChartListenerCollectionNeedsUpdate is needed for Navigator update
                            pDocSh->GetDocument().SetChartListenerCollectionNeedsUpdate( true );
                            pDocSh->SetDrawModified();
                        }
                    }
                }
                break;
            }

        case SID_EXTRUSION_TOGGLE:
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
            svx::FontworkBar::execute( *pView, rReq, rBindings );
            rReq.Ignore ();
            break;

        default:
            break;
    }
}

IMPL_LINK( ScDrawShell, NameObjectHdl, AbstractSvxObjectNameDialog&, rDialog, bool )
{
    OUString aName;
    rDialog.GetName( aName );

    ScDrawLayer* pModel = rViewData.GetDocument().GetDrawLayer();
    if ( !aName.isEmpty() && pModel )
    {
        SCTAB nDummyTab;
        if ( pModel->GetNamedObject( aName, 0, nDummyTab ) )
        {
            // existing object found -> name invalid
            return false;
        }
    }

    return true;   // name is valid
}

void ScDrawShell::ExecFormText(const SfxRequest& rReq)
{
    ScDrawView*         pDrView     = rViewData.GetScDrawView();
    const SdrMarkList&  rMarkList   = pDrView->GetMarkedObjectList();

    if ( rMarkList.GetMarkCount() == 1 && rReq.GetArgs() )
    {
        const SfxItemSet& rSet = *rReq.GetArgs();

        if ( pDrView->IsTextEdit() )
            pDrView->ScEndTextEdit();

        pDrView->SetAttributes(rSet);
    }
}

void ScDrawShell::ExecFormatPaintbrush( const SfxRequest& rReq )
{
    ScViewFunc* pView = rViewData.GetView();
    if ( pView->HasPaintBrush() )
    {
        // cancel paintbrush mode
        pView->ResetBrushDocument();
    }
    else
    {
        bool bLock = false;
        const SfxItemSet *pArgs = rReq.GetArgs();
        if( pArgs && pArgs->Count() >= 1 )
            bLock = pArgs->Get(SID_FORMATPAINTBRUSH).GetValue();

        ScDrawView* pDrawView = rViewData.GetScDrawView();
        if ( pDrawView && pDrawView->AreObjectsMarked() )
        {
            std::unique_ptr<SfxItemSet> pItemSet(new SfxItemSet( pDrawView->GetAttrFromMarked(true/*bOnlyHardAttr*/) ));
            pView->SetDrawBrushSet( std::move(pItemSet), bLock );
        }
    }
}

void ScDrawShell::StateFormatPaintbrush( SfxItemSet& rSet )
{
    ScDrawView* pDrawView = rViewData.GetScDrawView();
    bool bSelection = pDrawView && pDrawView->AreObjectsMarked();
    bool bHasPaintBrush = rViewData.GetView()->HasPaintBrush();

    if ( !bHasPaintBrush && !bSelection )
        rSet.DisableItem( SID_FORMATPAINTBRUSH );
    else
        rSet.Put( SfxBoolItem( SID_FORMATPAINTBRUSH, bHasPaintBrush ) );
}

ScDrawView* ScDrawShell::GetDrawView()
{
    return rViewData.GetView()->GetScDrawView();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
