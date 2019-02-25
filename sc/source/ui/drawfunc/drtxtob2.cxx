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

#include <editeng/adjustitem.hxx>
#include <svx/fontwork.hxx>
#include <editeng/frmdiritem.hxx>
#include <editeng/writingmodeitem.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/request.hxx>
#include <svl/whiter.hxx>
#include <svx/svdoashp.hxx>
#include <sc.hrc>
#include <drtxtob.hxx>
#include <viewdata.hxx>
#include <drawview.hxx>
#include <tabvwsh.hxx>
#include <drwlayer.hxx>

sal_uInt16 ScGetFontWorkId()
{
    return SvxFontWorkChildWindow::GetChildWindowId();
}

bool ScDrawTextObjectBar::IsNoteEdit()
{
    return ScDrawLayer::IsNoteCaption( pViewData->GetView()->GetSdrView()->GetTextEditObject() );
}

//  if no text edited, functions like in drawsh

void ScDrawTextObjectBar::ExecuteGlobal( SfxRequest &rReq )
{
    ScTabView*   pTabView  = pViewData->GetView();
    ScDrawView*  pView     = pTabView->GetScDrawView();

    sal_uInt16 nSlot = rReq.GetSlot();
    switch ( nSlot )
    {
        case SID_COPY:
            pView->DoCopy();
            break;

        case SID_CUT:
            pView->DoCut();
            pViewData->GetViewShell()->UpdateDrawShell();
            break;

        case SID_PASTE:
        case SID_PASTE_SPECIAL:
        case SID_PASTE_UNFORMATTED:
        case SID_CLIPBOARD_FORMAT_ITEMS:
        case SID_HYPERLINK_SETLINK:
            {
                //  cell methods are at cell shell, which is not available if
                //  ScDrawTextObjectBar is active
                //! move paste etc. to view shell?
            }
            break;

        case SID_SELECTALL:
            pView->MarkAll();
            break;

        case SID_TEXTDIRECTION_LEFT_TO_RIGHT:
        case SID_TEXTDIRECTION_TOP_TO_BOTTOM:
            {
                SfxItemSet aAttr( pView->GetModel()->GetItemPool(), svl::Items<SDRATTR_TEXTDIRECTION, SDRATTR_TEXTDIRECTION>{} );
                aAttr.Put( SvxWritingModeItem(
                    nSlot == SID_TEXTDIRECTION_LEFT_TO_RIGHT ?
                        css::text::WritingMode_LR_TB : css::text::WritingMode_TB_RL,
                        SDRATTR_TEXTDIRECTION ) );
                pView->SetAttributes( aAttr );
                pViewData->GetScDrawView()->InvalidateDrawTextAttrs();  // Bidi slots may be disabled
                rReq.Done( aAttr );
            }
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
    }
}

void ScDrawTextObjectBar::GetGlobalClipState( SfxItemSet& rSet )
{
    //  cell methods are at cell shell, which is not available if
    //  ScDrawTextObjectBar is active -> disable everything
    //! move paste etc. to view shell?

    SfxWhichIter aIter(rSet);
    sal_uInt16 nWhich = aIter.FirstWhich();
    while (nWhich)
    {
        rSet.DisableItem( nWhich );
        nWhich = aIter.NextWhich();
    }
}

void ScDrawTextObjectBar::ExecuteExtra( SfxRequest &rReq )
{
    ScTabView*   pTabView  = pViewData->GetView();
    ScDrawView*  pView     = pTabView->GetScDrawView();

    sal_uInt16 nSlot = rReq.GetSlot();
    switch ( nSlot )
    {
        case SID_FONTWORK:
            {
                sal_uInt16 nId = SvxFontWorkChildWindow::GetChildWindowId();
                SfxViewFrame* pViewFrm = pViewData->GetViewShell()->GetViewFrame();

                if ( rReq.GetArgs() )
                    pViewFrm->SetChildWindow( nId,
                                               static_cast<const SfxBoolItem&>(
                                                (rReq.GetArgs()->Get(SID_FONTWORK))).
                                                    GetValue() );
                else
                    pViewFrm->ToggleChildWindow( nId );

                pViewFrm->GetBindings().Invalidate( SID_FONTWORK );
                rReq.Done();
            }
            break;

        case SID_ATTR_PARA_LEFT_TO_RIGHT:
        case SID_ATTR_PARA_RIGHT_TO_LEFT:
            {
                SfxItemSet aAttr( pView->GetModel()->GetItemPool(),
                                    svl::Items<EE_PARA_WRITINGDIR, EE_PARA_WRITINGDIR,
                                    EE_PARA_JUST, EE_PARA_JUST>{} );
                bool bLeft = ( nSlot == SID_ATTR_PARA_LEFT_TO_RIGHT );
                aAttr.Put( SvxFrameDirectionItem(
                                bLeft ? SvxFrameDirection::Horizontal_LR_TB : SvxFrameDirection::Horizontal_RL_TB,
                                EE_PARA_WRITINGDIR ) );
                aAttr.Put( SvxAdjustItem(
                                bLeft ? SvxAdjust::Left : SvxAdjust::Right,
                                EE_PARA_JUST ) );
                pView->SetAttributes( aAttr );
                pViewData->GetScDrawView()->InvalidateDrawTextAttrs();
                rReq.Done();        //! Done(aAttr) ?

            }
            break;
    }
}

void ScDrawTextObjectBar::ExecFormText(const SfxRequest& rReq)
{
    ScTabView*          pTabView    = pViewData->GetView();
    ScDrawView*         pDrView     = pTabView->GetScDrawView();
    const SdrMarkList&  rMarkList   = pDrView->GetMarkedObjectList();

    if ( rMarkList.GetMarkCount() == 1 && rReq.GetArgs() )
    {
        const SfxItemSet& rSet = *rReq.GetArgs();

        if ( pDrView->IsTextEdit() )
            pDrView->ScEndTextEdit();

        pDrView->SetAttributes(rSet);
    }
}

void ScDrawTextObjectBar::GetFormTextState(SfxItemSet& rSet)
{
    const SdrObject*    pObj        = nullptr;
    ScDrawView*         pDrView     = pViewData->GetView()->GetScDrawView();
    const SdrMarkList&  rMarkList   = pDrView->GetMarkedObjectList();

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
        SfxItemSet aViewAttr(pDrView->GetModel()->GetItemPool());
        pDrView->GetAttributes(aViewAttr);
        rSet.Set(aViewAttr);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
