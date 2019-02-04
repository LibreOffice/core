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

#include <svx/svdview.hxx>
#include <svx/svdotext.hxx>
#include <svl/whiter.hxx>
#include <svx/fontwork.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/request.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/objface.hxx>
#include <svl/itemiter.hxx>
#include <svl/srchitem.hxx>
#include <svx/extrusionbar.hxx>
#include <svx/fontworkbar.hxx>
#include <uitool.hxx>
#include <dcontact.hxx>
#include <textboxhelper.hxx>
#include <wview.hxx>
#include <swmodule.hxx>
#include <swwait.hxx>
#include <doc.hxx>
#include <docsh.hxx>
#include <docstat.hxx>
#include <IDocumentStatistics.hxx>
#include <tools/diagnose_ex.h>

#include <svx/svdoashp.hxx>
#include <svx/xtable.hxx>
#include <vcl/EnumContext.hxx>
#include <svx/svdoole2.hxx>
#include <sfx2/opengrf.hxx>
#include <svx/svdograf.hxx>
#include <svx/svdundo.hxx>
#include <svx/xbtmpit.hxx>

#include <swundo.hxx>
#include <wrtsh.hxx>
#include <cmdid.h>
#include <globals.hrc>
#include <strings.hrc>
#include <drwbassh.hxx>
#include <drawsh.hxx>

#define ShellClass_SwDrawShell
#include <sfx2/msg.hxx>
#include <swslots.hxx>
#include <swabstdlg.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

SFX_IMPL_INTERFACE(SwDrawShell, SwDrawBaseShell)

void SwDrawShell::InitInterface_Impl()
{
    GetStaticInterface()->RegisterPopupMenu("draw");

    GetStaticInterface()->RegisterObjectBar(SFX_OBJECTBAR_OBJECT, SfxVisibilityFlags::Invisible, ToolbarId::Draw_Toolbox_Sw);

    GetStaticInterface()->RegisterChildWindow(SvxFontWorkChildWindow::GetChildWindowId());
}


// #i123922# check as the name implies
SdrObject* SwDrawShell::IsSingleFillableNonOLESelected()
{
    SwWrtShell &rSh = GetShell();
    SdrView* pSdrView = rSh.GetDrawView();

    if(!pSdrView)
    {
        return nullptr;
    }

    if(1 != pSdrView->GetMarkedObjectCount())
    {
        return nullptr;
    }

    SdrObject* pPickObj = pSdrView->GetMarkedObjectByIndex(0);

    if(!pPickObj)
    {
        return nullptr;
    }

    if(!pPickObj->IsClosedObj())
    {
        return nullptr;
    }

    if(dynamic_cast< SdrOle2Obj* >(pPickObj))
    {
        return nullptr;
    }

    return pPickObj;
}

// #i123922# insert given graphic data dependent of the object type in focus
void SwDrawShell::InsertPictureFromFile(SdrObject& rObject)
{
    SwWrtShell &rSh = GetShell();
    SdrView* pSdrView = rSh.GetDrawView();

    if(pSdrView)
    {
        SvxOpenGraphicDialog aDlg(SwResId(STR_INSERT_GRAPHIC), GetView().GetViewFrame()->GetWindow().GetFrameWeld());

        if (ERRCODE_NONE == aDlg.Execute())
        {
            Graphic aGraphic;
            ErrCode nError = aDlg.GetGraphic(aGraphic);

            if(ERRCODE_NONE == nError)
            {
                const bool bAsLink(aDlg.IsAsLink());
                SdrObject* pResult = &rObject;

                rSh.StartUndo(SwUndoId::PASTE_CLIPBOARD);

                if (SdrGrafObj* pSdrGrafObj = dynamic_cast<SdrGrafObj*>(&rObject))
                {
                    SdrGrafObj* pNewGrafObj(pSdrGrafObj->CloneSdrObject(pSdrGrafObj->getSdrModelFromSdrObject()));

                    pNewGrafObj->SetGraphic(aGraphic);

                    // #i123922#  for handling MasterObject and virtual ones correctly, SW
                    // wants us to call ReplaceObject at the page, but that also
                    // triggers the same assertion (I tried it), so stay at the view method
                    pSdrView->ReplaceObjectAtView(&rObject, *pSdrView->GetSdrPageView(), pNewGrafObj);

                    OUString aReferer;
                    SwDocShell *pDocShell = rSh.GetDoc()->GetDocShell();
                    if (pDocShell->HasName()) {
                        aReferer = pDocShell->GetMedium()->GetName();
                    }

                    // set in all cases - the Clone() will have copied an existing link (!)
                    pNewGrafObj->SetGraphicLink(
                        bAsLink ? aDlg.GetPath() : OUString(),
                        aReferer,
                        bAsLink ? aDlg.GetDetectedFilter() : OUString());

                    pResult = pNewGrafObj;
                }
                else // if(rObject.IsClosedObj() && !dynamic_cast< SdrOle2Obj* >(&rObject))
                {
                    pSdrView->AddUndo(std::make_unique<SdrUndoAttrObj>(rObject));

                    SfxItemSet aSet(pSdrView->GetModel()->GetItemPool(), svl::Items<XATTR_FILLSTYLE, XATTR_FILLBITMAP>{});

                    aSet.Put(XFillStyleItem(drawing::FillStyle_BITMAP));
                    aSet.Put(XFillBitmapItem(OUString(), aGraphic));
                    rObject.SetMergedItemSetAndBroadcast(aSet);
                }

                rSh.EndUndo( SwUndoId::END );

                if(pResult)
                {
                    // we are done; mark the modified/new object
                    pSdrView->MarkObj(pResult, pSdrView->GetSdrPageView());
                }
            }
        }
    }
}

void SwDrawShell::Execute(SfxRequest &rReq)
{
    SwWrtShell          &rSh = GetShell();
    SdrView             *pSdrView = rSh.GetDrawView();
    const SfxItemSet    *pArgs = rReq.GetArgs();
    SfxBindings         &rBnd  = GetView().GetViewFrame()->GetBindings();
    sal_uInt16               nSlotId = rReq.GetSlot();
    bool                 bChanged = pSdrView->GetModel()->IsChanged();

    pSdrView->GetModel()->SetChanged(false);

    const SfxPoolItem* pItem;
    if(pArgs)
        pArgs->GetItemState(nSlotId, false, &pItem);

    bool bMirror = true;

    switch (nSlotId)
    {
        case SID_OBJECT_ROTATE:
            if (rSh.IsObjSelected() && pSdrView->IsRotateAllowed())
            {
                if (GetView().IsDrawRotate())
                    rSh.SetDragMode(SdrDragMode::Move);
                else
                    rSh.SetDragMode(SdrDragMode::Rotate);

                GetView().FlipDrawRotate();
            }
            break;

        case SID_BEZIER_EDIT:
            if (GetView().IsDrawRotate())
            {
                rSh.SetDragMode(SdrDragMode::Move);
                GetView().FlipDrawRotate();
            }
            GetView().FlipDrawSelMode();
            pSdrView->SetFrameDragSingles(GetView().IsDrawSelMode());
            GetView().AttrChangedNotify(&rSh); // Shell switch
            break;

        case SID_OBJECT_HELL:
            if (rSh.IsObjSelected())
            {
                rSh.StartUndo( SwUndoId::START );
                SetWrapMode(FN_FRAME_WRAPTHRU_TRANSP);
                rSh.SelectionToHell();
                rSh.EndUndo( SwUndoId::END );
                rBnd.Invalidate(SID_OBJECT_HEAVEN);
            }
            break;

        case SID_OBJECT_HEAVEN:
            if (rSh.IsObjSelected())
            {
                rSh.StartUndo( SwUndoId::START );
                SetWrapMode(FN_FRAME_WRAPTHRU);
                rSh.SelectionToHeaven();
                rSh.EndUndo( SwUndoId::END );
                rBnd.Invalidate(SID_OBJECT_HELL);
            }
            break;

        case FN_TOOL_HIERARCHIE:
            if (rSh.IsObjSelected())
            {
                rSh.StartUndo( SwUndoId::START );
                if (rSh.GetLayerId() == SdrLayerID(0))
                {
                    SetWrapMode(FN_FRAME_WRAPTHRU);
                    rSh.SelectionToHeaven();
                }
                else
                {
                    SetWrapMode(FN_FRAME_WRAPTHRU_TRANSP);
                    rSh.SelectionToHell();
                }
                rSh.EndUndo( SwUndoId::END );
                rBnd.Invalidate( SID_OBJECT_HELL );
                rBnd.Invalidate( SID_OBJECT_HEAVEN );
            }
            break;

        case SID_FLIP_VERTICAL:
            bMirror = false;
            [[fallthrough]];
        case SID_FLIP_HORIZONTAL:
            rSh.MirrorSelection( bMirror );
            break;

        case SID_FONTWORK:
        {
            FieldUnit eMetric = ::GetDfltMetric( dynamic_cast<SwWebView*>( &rSh.GetView()) != nullptr );
            SW_MOD()->PutItem(SfxUInt16Item(SID_ATTR_METRIC, static_cast< sal_uInt16 >(eMetric)) );
            SfxViewFrame* pVFrame = GetView().GetViewFrame();
            if (pArgs)
            {
                pVFrame->SetChildWindow(SvxFontWorkChildWindow::GetChildWindowId(),
                    static_cast<const SfxBoolItem&>((pArgs->Get(SID_FONTWORK))).GetValue());
            }
            else
                pVFrame->ToggleChildWindow( SvxFontWorkChildWindow::GetChildWindowId() );
            pVFrame->GetBindings().Invalidate(SID_FONTWORK);
        }
        break;
        case FN_FORMAT_FOOTNOTE_DLG:
        {
            GetView().ExecFormatFootnote();
            break;
        }
        case FN_NUMBERING_OUTLINE_DLG:
        {
            GetView().ExecNumberingOutline(GetPool());
            rReq.Done();
        }
        break;
        case SID_OPEN_XML_FILTERSETTINGS:
        {
            HandleOpenXmlFilterSettings(rReq);
        }
        break;
        case FN_WORDCOUNT_DIALOG:
        {
            GetView().UpdateWordCount(this, nSlotId);
        }
        break;
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
            svx::ExtrusionBar::execute( pSdrView, rReq, rBnd );
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
            svx::FontworkBar::execute(*pSdrView, rReq, rBnd);
            rReq.Ignore ();
            break;

        case SID_INSERT_GRAPHIC:
        {
            // #i123922# check if we can do something
            SdrObject* pObj = IsSingleFillableNonOLESelected();

            if(pObj)
            {
                // ...and if yes, do something
                InsertPictureFromFile(*pObj);
            }

            break;
        }

        case FN_ADD_TEXT_BOX:
        {
            if (SdrObject* pObj = IsSingleFillableNonOLESelected())
            {
                SwFrameFormat* pFrameFormat = ::FindFrameFormat(pObj);
                if (pFrameFormat)
                    SwTextBoxHelper::create(pFrameFormat);
            }
            break;
        }
        case FN_REMOVE_TEXT_BOX:
        {
            if (SdrObject* pObj = IsSingleFillableNonOLESelected())
            {
                SwFrameFormat* pFrameFormat = ::FindFrameFormat(pObj);
                if (pFrameFormat)
                    SwTextBoxHelper::destroy(pFrameFormat);
            }
            break;
        }
        default:
            OSL_ENSURE(false, "wrong dispatcher");
            return;
    }
    if (pSdrView->GetModel()->IsChanged())
        rSh.SetModified();
    else if (bChanged)
        pSdrView->GetModel()->SetChanged();
}

void SwDrawShell::GetState(SfxItemSet& rSet)
{
    SwWrtShell &rSh = GetShell();
    SdrView* pSdrView = rSh.GetDrawViewWithValidMarkList();
    SfxWhichIter aIter( rSet );
    sal_uInt16 nWhich = aIter.FirstWhich();
    bool bProtected = rSh.IsSelObjProtected(FlyProtectFlags::Content) != FlyProtectFlags::NONE;

    if (!bProtected)    // Check the parent
        bProtected |= rSh.IsSelObjProtected( FlyProtectFlags::Content|FlyProtectFlags::Parent ) != FlyProtectFlags::NONE;

    while( nWhich )
    {
        switch( nWhich )
        {
            case SID_OBJECT_HELL:
                if ( !rSh.IsObjSelected() || rSh.GetLayerId() == SdrLayerID(0) || bProtected )
                    rSet.DisableItem( nWhich );
                break;

            case SID_OBJECT_HEAVEN:
                if ( !rSh.IsObjSelected() || rSh.GetLayerId() == SdrLayerID(1) || bProtected )
                    rSet.DisableItem( nWhich );
                break;

            case FN_TOOL_HIERARCHIE:
                if ( !rSh.IsObjSelected() || bProtected )
                    rSet.DisableItem( nWhich );
                break;

            case SID_OBJECT_ROTATE:
            {
                const bool bIsRotate = GetView().IsDrawRotate();
                if ( (!bIsRotate && !pSdrView->IsRotateAllowed()) || bProtected )
                    rSet.DisableItem( nWhich );
                else
                    rSet.Put( SfxBoolItem( nWhich, bIsRotate ) );
            }
            break;

            case SID_BEZIER_EDIT:
                if (!Disable(rSet, nWhich))
                    rSet.Put( SfxBoolItem( nWhich, !GetView().IsDrawSelMode()));
            break;

            case SID_FLIP_VERTICAL:
                if ( !pSdrView->IsMirrorAllowed() || bProtected )
                {
                    rSet.DisableItem( nWhich );
                }
                else
                {
                    // TTTT - needs to be adapted in aw080:
                    // state is not kept for drawing objects --> provide not flipped state
                    rSet.Put( SfxBoolItem( nWhich, false ) );
                }
                break;

            case SID_FLIP_HORIZONTAL:
                if ( !pSdrView->IsMirrorAllowed() || bProtected )
                {
                    rSet.DisableItem( nWhich );
                }
                else
                {
                    // TTTT - needs to be adapted in aw080:
                    // state is not kept for drawing objects --> provide not flipped state
                    rSet.Put( SfxBoolItem( nWhich, false ) );
                }
                break;

            case SID_FONTWORK:
            {
                if (bProtected)
                    rSet.DisableItem( nWhich );
                else
                {
                    const sal_uInt16 nId = SvxFontWorkChildWindow::GetChildWindowId();
                    rSet.Put(SfxBoolItem( nWhich , GetView().GetViewFrame()->HasChildWindow(nId)));
                }
            }
            break;

            case SID_INSERT_GRAPHIC:
            {
                // #i123922# check if we can do something
                SdrObject* pObj = IsSingleFillableNonOLESelected();

                if(!pObj)
                {
                    rSet.DisableItem(nWhich);
                }

                break;
            }
            case FN_ADD_TEXT_BOX:
            {
                bool bDisable = true;
                if (SdrObject* pObj = IsSingleFillableNonOLESelected())
                {
                    SwFrameFormat* pFrameFormat = ::FindFrameFormat(pObj);
                    // Allow creating a TextBox only in case this is a draw format without a TextBox so far.
                    if (pFrameFormat && pFrameFormat->Which() == RES_DRAWFRMFMT && !SwTextBoxHelper::isTextBox(pFrameFormat, RES_DRAWFRMFMT))
                    {
                        if (SdrObjCustomShape* pCustomShape = dynamic_cast<SdrObjCustomShape*>( pObj) )
                        {
                            const SdrCustomShapeGeometryItem& rGeometryItem = pCustomShape->GetMergedItem(SDRATTR_CUSTOMSHAPE_GEOMETRY);
                            if (const uno::Any* pAny = rGeometryItem.GetPropertyValueByName("Type"))
                                // But still disallow fontwork shapes.
                                bDisable = pAny->get<OUString>().startsWith("fontwork-");
                        }
                    }
                }

                if (bDisable)
                    rSet.DisableItem(nWhich);
                break;
            }
            case FN_REMOVE_TEXT_BOX:
            {
                bool bDisable = true;
                if (SdrObject* pObj = IsSingleFillableNonOLESelected())
                {
                    SwFrameFormat* pFrameFormat = ::FindFrameFormat(pObj);
                    // Allow removing a TextBox only in case it has one.
                    if (pFrameFormat && SwTextBoxHelper::isTextBox(pFrameFormat, RES_DRAWFRMFMT))
                        bDisable = false;
                }

                if (bDisable)
                    rSet.DisableItem(nWhich);
                break;
            }
        }
        nWhich = aIter.NextWhich();
    }
    svx::ExtrusionBar::getState( pSdrView, rSet );
    svx::FontworkBar::getState( pSdrView, rSet );
}

SwDrawShell::SwDrawShell(SwView &_rView) :
    SwDrawBaseShell(_rView)
{
    SetName("Draw");

    SfxShell::SetContextName(vcl::EnumContext::GetContextName(vcl::EnumContext::Context::Draw));
}

// Edit SfxRequests for FontWork

void SwDrawShell::ExecFormText(SfxRequest const & rReq)
{
    SwWrtShell &rSh = GetShell();
    SdrView*    pDrView = rSh.GetDrawView();
    bool        bChanged = pDrView->GetModel()->IsChanged();
    pDrView->GetModel()->SetChanged(false);

    const SdrMarkList& rMarkList = pDrView->GetMarkedObjectList();

    if ( rMarkList.GetMarkCount() == 1 && rReq.GetArgs() )
    {
        const SfxItemSet& rSet = *rReq.GetArgs();

        if ( pDrView->IsTextEdit() )
        {
            pDrView->SdrEndTextEdit( true );
            GetView().AttrChangedNotify(&rSh);
        }

        pDrView->SetAttributes(rSet);
    }
    if (pDrView->GetModel()->IsChanged())
        rSh.SetModified();
    else
        if (bChanged)
            pDrView->GetModel()->SetChanged();
}

//Return status values for FontWork

void SwDrawShell::GetFormTextState(SfxItemSet& rSet)
{
    SwWrtShell &rSh = GetShell();
    SdrView* pDrView = rSh.GetDrawView();
    const SdrMarkList& rMarkList = pDrView->GetMarkedObjectList();
    const SdrObject* pObj = nullptr;

    if ( rMarkList.GetMarkCount() == 1 )
        pObj = rMarkList.GetMark(0)->GetMarkedSdrObj();

    const SdrTextObj* pTextObj = dynamic_cast< const SdrTextObj* >(pObj);
    const bool bDeactivate(
        !pObj ||
        !pTextObj ||
        !pTextObj->HasText() ||
        dynamic_cast< const SdrObjCustomShape* >(pObj)); // #121538# no FontWork for CustomShapes

    if(bDeactivate)
    {
        rSet.DisableItem(XATTR_FORMTXTSTYLE);
        rSet.DisableItem(XATTR_FORMTXTADJUST);
        rSet.DisableItem(XATTR_FORMTXTDISTANCE);
        rSet.DisableItem(XATTR_FORMTXTSTART);
        rSet.DisableItem(XATTR_FORMTXTMIRROR);
        rSet.DisableItem(XATTR_FORMTXTHIDEFORM);
        rSet.DisableItem(XATTR_FORMTXTOUTLINE);
        rSet.DisableItem(XATTR_FORMTXTSHADOW);
        rSet.DisableItem(XATTR_FORMTXTSHDWCOLOR);
        rSet.DisableItem(XATTR_FORMTXTSHDWXVAL);
        rSet.DisableItem(XATTR_FORMTXTSHDWYVAL);
    }
    else
    {
        pDrView->GetAttributes( rSet );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
