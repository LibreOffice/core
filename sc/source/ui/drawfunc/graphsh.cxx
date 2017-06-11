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

#include <o3tl/make_unique.hxx>
#include <sfx2/app.hxx>
#include <sfx2/objface.hxx>
#include <sfx2/request.hxx>
#include <vcl/EnumContext.hxx>
#include <sfx2/opengrf.hxx>
#include <svl/whiter.hxx>
#include <svx/svdograf.hxx>
#include <svx/grfflt.hxx>
#include <svx/grafctrl.hxx>
#include <svx/compressgraphicdialog.hxx>
#include <svx/graphichelper.hxx>
#include <vcl/msgbox.hxx>

#include "graphsh.hxx"
#include "strings.hrc"
#include "viewdata.hxx"
#include "drawview.hxx"
#include "scresid.hxx"
#include <svx/extedit.hxx>
#include "tabvwsh.hxx"

#define ScGraphicShell
#include "scslots.hxx"

SFX_IMPL_INTERFACE(ScGraphicShell, ScDrawShell)

void ScGraphicShell::InitInterface_Impl()
{
    GetStaticInterface()->RegisterObjectBar(SFX_OBJECTBAR_OBJECT,
                                            SfxVisibilityFlags::Standard | SfxVisibilityFlags::Server,
                                            ToolbarId::Graphic_Objectbar);

    GetStaticInterface()->RegisterPopupMenu("graphic");
}


ScGraphicShell::ScGraphicShell(ScViewData* pData) :
    ScDrawShell(pData)
{
    SetName("GraphicObject");
        SfxShell::SetContextName(vcl::EnumContext::GetContextName(vcl::EnumContext::Context::Graphic));
}

ScGraphicShell::~ScGraphicShell()
{
}

void ScGraphicShell::GetAttrState( SfxItemSet& rSet )
{
    ScDrawView* pView = GetViewData()->GetScDrawView();

    if( pView )
        SvxGrafAttrHelper::GetGrafAttrState( rSet, *pView );
}

void ScGraphicShell::Execute( SfxRequest& rReq )
{
    ScDrawView* pView = GetViewData()->GetScDrawView();

    if( pView )
    {
        SvxGrafAttrHelper::ExecuteGrafAttr( rReq, *pView );
        Invalidate();
    }
}

void ScGraphicShell::GetFilterState( SfxItemSet& rSet )
{
    ScDrawView* pView = GetViewData()->GetScDrawView();
    const SdrMarkList& rMarkList = pView->GetMarkedObjectList();
    bool bEnable = false;

    if( rMarkList.GetMarkCount() == 1 )
    {
        SdrObject* pObj = rMarkList.GetMark( 0 )->GetMarkedSdrObj();

        if( pObj && dynamic_cast<const SdrGrafObj*>( pObj) != nullptr && ( static_cast<SdrGrafObj*>(pObj)->GetGraphicType() == GraphicType::Bitmap ) )
            bEnable = true;
    }

    if( !bEnable )
        SvxGraphicFilter::DisableGraphicFilterSlots( rSet );
}

void ScGraphicShell::ExecuteFilter( SfxRequest& rReq )
{
    ScDrawView* pView = GetViewData()->GetScDrawView();
    const SdrMarkList& rMarkList = pView->GetMarkedObjectList();

    if( rMarkList.GetMarkCount() == 1 )
    {
        SdrObject* pObj = rMarkList.GetMark( 0 )->GetMarkedSdrObj();

        if( pObj &&  dynamic_cast<const SdrGrafObj*>( pObj)  != nullptr && static_cast<SdrGrafObj*>(pObj)->GetGraphicType() == GraphicType::Bitmap )
        {
            GraphicObject aFilterObj( static_cast<SdrGrafObj*>(pObj)->GetGraphicObject() );

            if( SVX_GRAPHICFILTER_ERRCODE_NONE ==
                SvxGraphicFilter::ExecuteGrfFilterSlot( rReq, aFilterObj ) )
            {
                SdrPageView* pPageView = pView->GetSdrPageView();

                if( pPageView )
                {
                    SdrGrafObj* pFilteredObj = static_cast<SdrGrafObj*>(pObj->Clone());
                    OUString    aStr = pView->GetDescriptionOfMarkedObjects() + " " + ScResId(SCSTR_UNDO_GRAFFILTER);
                    pView->BegUndo( aStr );
                    pFilteredObj->SetGraphicObject( aFilterObj );
                    pView->ReplaceObjectAtView( pObj, *pPageView, pFilteredObj );
                    pView->EndUndo();
                }
            }
        }
    }

    Invalidate();
}

void ScGraphicShell::GetExternalEditState( SfxItemSet& rSet )
{
    ScDrawView* pView = GetViewData()->GetScDrawView();
    const SdrMarkList& rMarkList = pView->GetMarkedObjectList();
    bool bEnable = false;
    if( rMarkList.GetMarkCount() == 1 )
    {
        SdrObject* pObj = rMarkList.GetMark( 0 )->GetMarkedSdrObj();

        if( pObj && dynamic_cast<const SdrGrafObj*>( pObj) != nullptr  && ( static_cast<SdrGrafObj*>(pObj)->GetGraphicType() == GraphicType::Bitmap ) )
            bEnable = true;
    }

    if( !bEnable )
        rSet.DisableItem( SID_EXTERNAL_EDIT );
}

void ScGraphicShell::ExecuteExternalEdit( SAL_UNUSED_PARAMETER SfxRequest& )
{
    ScDrawView* pView = GetViewData()->GetScDrawView();
    const SdrMarkList& rMarkList = pView->GetMarkedObjectList();

    if( rMarkList.GetMarkCount() == 1 )
    {
        SdrObject* pObj = rMarkList.GetMark( 0 )->GetMarkedSdrObj();

        if( pObj && dynamic_cast<const SdrGrafObj*>( pObj) != nullptr && static_cast<SdrGrafObj*>(pObj)->GetGraphicType() == GraphicType::Bitmap )
        {
            GraphicObject aGraphicObject( static_cast<SdrGrafObj*>(pObj)->GetGraphicObject() );
            m_ExternalEdits.push_back( o3tl::make_unique<SdrExternalToolEdit>(
                        pView, pObj));
            m_ExternalEdits.back()->Edit( &aGraphicObject );
        }
    }

    Invalidate();
}

void ScGraphicShell::GetCompressGraphicState( SfxItemSet& rSet )
{
    ScDrawView* pView = GetViewData()->GetScDrawView();
    const SdrMarkList& rMarkList = pView->GetMarkedObjectList();
    bool bEnable = false;
    if( rMarkList.GetMarkCount() == 1 )
    {
        SdrObject* pObj = rMarkList.GetMark( 0 )->GetMarkedSdrObj();

        if( pObj && dynamic_cast<const SdrGrafObj*>( pObj) != nullptr && ( static_cast<SdrGrafObj*>(pObj)->GetGraphicType() == GraphicType::Bitmap ) )
            bEnable = true;
    }

    if( !bEnable )
        rSet.DisableItem( SID_COMPRESS_GRAPHIC );
}

void ScGraphicShell::ExecuteCompressGraphic( SAL_UNUSED_PARAMETER SfxRequest& )
{
    ScDrawView* pView = GetViewData()->GetScDrawView();
    const SdrMarkList& rMarkList = pView->GetMarkedObjectList();

    if( rMarkList.GetMarkCount() == 1 )
    {
        SdrObject* pObj = rMarkList.GetMark( 0 )->GetMarkedSdrObj();

        if( pObj && dynamic_cast<const SdrGrafObj*>( pObj)  != nullptr && static_cast<SdrGrafObj*>(pObj)->GetGraphicType() == GraphicType::Bitmap )
        {
            SdrGrafObj* pGraphicObj = static_cast<SdrGrafObj*>(pObj);
            ScopedVclPtrInstance< CompressGraphicsDialog > dialog( GetViewData()->GetDialogParent(), pGraphicObj, GetViewData()->GetBindings() );
            if ( dialog->Execute() == RET_OK )
            {
                SdrGrafObj* pNewObject = dialog->GetCompressedSdrGrafObj();
                SdrPageView* pPageView = pView->GetSdrPageView();
                OUString aUndoString = pView->GetDescriptionOfMarkedObjects() + " Compress";
                pView->BegUndo( aUndoString );
                pView->ReplaceObjectAtView( pObj, *pPageView, pNewObject );
                pView->EndUndo();
            }
        }
    }

    Invalidate();
}

void ScGraphicShell::GetCropGraphicState( SfxItemSet& rSet )
{
    ScDrawView* pView = GetViewData()->GetScDrawView();
    const SdrMarkList& rMarkList = pView->GetMarkedObjectList();
    bool bEnable = false;
    if( rMarkList.GetMarkCount() == 1 )
    {
        SdrObject* pObj = rMarkList.GetMark( 0 )->GetMarkedSdrObj();

        if( pObj && nullptr != dynamic_cast<const SdrGrafObj*>( pObj) && ( static_cast<SdrGrafObj*>(pObj)->GetGraphicType() == GraphicType::Bitmap ) )
            bEnable = true;
    }

    if( !bEnable )
        rSet.DisableItem( SID_OBJECT_CROP );
}

void ScGraphicShell::ExecuteCropGraphic( SAL_UNUSED_PARAMETER SfxRequest& )
{
    ScDrawView* pView = GetViewData()->GetScDrawView();
    const SdrMarkList& rMarkList = pView->GetMarkedObjectList();

    if( rMarkList.GetMarkCount() == 1 )
    {
        SdrObject* pObj = rMarkList.GetMark( 0 )->GetMarkedSdrObj();

        if( pObj && dynamic_cast<const SdrGrafObj*>( pObj) != nullptr && static_cast<SdrGrafObj*>(pObj)->GetGraphicType() == GraphicType::Bitmap )
        {
            pView->SetEditMode(SdrViewEditMode::Edit);
            pView->SetDragMode(SdrDragMode::Crop);
        }
    }

    Invalidate();
}

void ScGraphicShell::ExecuteSaveGraphic( SAL_UNUSED_PARAMETER SfxRequest& /*rReq*/)
{
    ScDrawView* pView = GetViewData()->GetScDrawView();
    const SdrMarkList& rMarkList = pView->GetMarkedObjectList();
    if( rMarkList.GetMarkCount() == 1 )
    {
        const SdrGrafObj* pObj = dynamic_cast<const SdrGrafObj*>(rMarkList.GetMark( 0 )->GetMarkedSdrObj());
        if( pObj && pObj->GetGraphicType() == GraphicType::Bitmap )
        {
            GraphicAttr aGraphicAttr = pObj->GetGraphicAttr();
            short nState = RET_CANCEL;
            if (aGraphicAttr != GraphicAttr()) // the image has been modified
            {
                vcl::Window* pWin = GetViewData()->GetActiveWin();
                if (pWin)
                {
                    nState = GraphicHelper::HasToSaveTransformedImage(pWin);
                }
            }
            else
            {
                nState = RET_NO;
            }

            if (nState == RET_YES)
            {
                GraphicHelper::ExportGraphic( pObj->GetTransformedGraphic(), "" );
            }
            else if (nState == RET_NO)
            {
                GraphicObject aGraphicObject(pObj->GetGraphicObject());
                GraphicHelper::ExportGraphic( aGraphicObject.GetGraphic(), "" );
            }
        }
    }

    Invalidate();
}

void ScGraphicShell::GetSaveGraphicState(SfxItemSet &rSet)
{
    ScDrawView* pView = GetViewData()->GetScDrawView();
    const SdrMarkList& rMarkList = pView->GetMarkedObjectList();
    bool bEnable = false;
    if( rMarkList.GetMarkCount() == 1 )
    {
        SdrObject* pObj = rMarkList.GetMark( 0 )->GetMarkedSdrObj();

        if( pObj && dynamic_cast<const SdrGrafObj*>( pObj) != nullptr && ( static_cast<SdrGrafObj*>(pObj)->GetGraphicType() == GraphicType::Bitmap ) )
            bEnable = true;
    }

    if( !bEnable )
        rSet.DisableItem( SID_SAVE_GRAPHIC );
}

void ScGraphicShell::ExecuteChangePicture( SAL_UNUSED_PARAMETER SfxRequest& /*rReq*/)
{
    ScDrawView* pView = GetViewData()->GetScDrawView();
    const SdrMarkList& rMarkList = pView->GetMarkedObjectList();

    if( rMarkList.GetMarkCount() == 1 )
    {
        SdrObject* pObj = rMarkList.GetMark( 0 )->GetMarkedSdrObj();

        if( pObj && nullptr != dynamic_cast<const SdrGrafObj*>( pObj) && static_cast<SdrGrafObj*>(pObj)->GetGraphicType() == GraphicType::Bitmap )
        {
            SdrGrafObj* pGraphicObj = static_cast<SdrGrafObj*>(pObj);
            SvxOpenGraphicDialog aDlg(ScResId(STR_INSERTGRAPHIC));

            if( aDlg.Execute() == ERRCODE_NONE )
            {
                Graphic aGraphic;
                ErrCode nError = aDlg.GetGraphic(aGraphic);
                if( nError == ERRCODE_NONE )
                {
                    SdrGrafObj* pNewObject = pGraphicObj->Clone();
                    pNewObject->SetGraphic( aGraphic );
                    SdrPageView* pPageView = pView->GetSdrPageView();
                    OUString aUndoString = pView->GetDescriptionOfMarkedObjects() + " Change";
                    pView->BegUndo( aUndoString );
                    pView->ReplaceObjectAtView( pObj, *pPageView, pNewObject );
                    pView->EndUndo();
                }
            }
        }
    }

    Invalidate();
}

void ScGraphicShell::GetChangePictureState(SfxItemSet &rSet)
{
    ScDrawView* pView = GetViewData()->GetScDrawView();
    const SdrMarkList& rMarkList = pView->GetMarkedObjectList();
    bool bEnable = false;
    if( rMarkList.GetMarkCount() == 1 )
    {
        SdrObject* pObj = rMarkList.GetMark( 0 )->GetMarkedSdrObj();

        if( pObj && dynamic_cast<const SdrGrafObj*>( pObj) != nullptr && ( static_cast<SdrGrafObj*>(pObj)->GetGraphicType() == GraphicType::Bitmap ) )
            bEnable = true;
    }

    if( !bEnable )
        rSet.DisableItem( SID_CHANGE_PICTURE );
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
