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

#include <sfx2/app.hxx>
#include <sfx2/objface.hxx>
#include <sfx2/request.hxx>
#include <sfx2/sidebar/EnumContext.hxx>
#include <sfx2/opengrf.hxx>
#include <svl/whiter.hxx>
#include <svx/svdograf.hxx>
#include <svx/grfflt.hxx>
#include <svx/grafctrl.hxx>
#include <svx/compressgraphicdialog.hxx>
#include <svx/graphichelper.hxx>
#include <vcl/msgbox.hxx>

#include "graphsh.hxx"
#include "sc.hrc"
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
    GetStaticInterface()->RegisterObjectBar(SFX_OBJECTBAR_OBJECT|SFX_VISIBILITY_STANDARD|SFX_VISIBILITY_SERVER,
                                            RID_GRAPHIC_OBJECTBAR);

    GetStaticInterface()->RegisterPopupMenu(ScResId(RID_POPUP_GRAPHIC));
}

TYPEINIT1( ScGraphicShell, ScDrawShell );

ScGraphicShell::ScGraphicShell(ScViewData* pData) :
    ScDrawShell(pData)
{
    SetHelpId(HID_SCSHELL_GRAPHIC);
    SetName("GraphicObject");
        SfxShell::SetContextName(sfx2::sidebar::EnumContext::GetContextName(sfx2::sidebar::EnumContext::Context_Graphic));
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

        if( pObj && dynamic_cast<const SdrGrafObj*>( pObj) != nullptr && ( static_cast<SdrGrafObj*>(pObj)->GetGraphicType() == GRAPHIC_BITMAP ) )
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

        if( pObj &&  dynamic_cast<const SdrGrafObj*>( pObj)  != nullptr && static_cast<SdrGrafObj*>(pObj)->GetGraphicType() == GRAPHIC_BITMAP )
        {
            GraphicObject aFilterObj( static_cast<SdrGrafObj*>(pObj)->GetGraphicObject() );

            if( SVX_GRAPHICFILTER_ERRCODE_NONE ==
                SvxGraphicFilter::ExecuteGrfFilterSlot( rReq, aFilterObj ) )
            {
                SdrPageView* pPageView = pView->GetSdrPageView();

                if( pPageView )
                {
                    SdrGrafObj* pFilteredObj = static_cast<SdrGrafObj*>(pObj->Clone());
                    OUString    aStr = pView->GetDescriptionOfMarkedObjects() + " " + OUString(ScResId( SCSTR_UNDO_GRAFFILTER ));
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

        if( pObj && dynamic_cast<const SdrGrafObj*>( pObj) != nullptr  && ( static_cast<SdrGrafObj*>(pObj)->GetGraphicType() == GRAPHIC_BITMAP ) )
            bEnable = true;
    }

    if( !bEnable )
        rSet.DisableItem( SID_EXTERNAL_EDIT );
}

void ScGraphicShell::ExecuteExternalEdit( SfxRequest& )
{
    ScDrawView* pView = GetViewData()->GetScDrawView();
    const SdrMarkList& rMarkList = pView->GetMarkedObjectList();

    if( rMarkList.GetMarkCount() == 1 )
    {
        SdrObject* pObj = rMarkList.GetMark( 0 )->GetMarkedSdrObj();

        if( pObj && dynamic_cast<const SdrGrafObj*>( pObj) != nullptr && static_cast<SdrGrafObj*>(pObj)->GetGraphicType() == GRAPHIC_BITMAP )
        {
            GraphicObject aGraphicObject( static_cast<SdrGrafObj*>(pObj)->GetGraphicObject() );
            m_ExternalEdits.push_back( std::unique_ptr<SdrExternalToolEdit>(
                        new SdrExternalToolEdit(pView, pObj)));
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

        if( pObj && dynamic_cast<const SdrGrafObj*>( pObj) != nullptr && ( static_cast<SdrGrafObj*>(pObj)->GetGraphicType() == GRAPHIC_BITMAP ) )
            bEnable = true;
    }

    if( !bEnable )
        rSet.DisableItem( SID_COMPRESS_GRAPHIC );
}

void ScGraphicShell::ExecuteCompressGraphic( SfxRequest& )
{
    ScDrawView* pView = GetViewData()->GetScDrawView();
    const SdrMarkList& rMarkList = pView->GetMarkedObjectList();

    if( rMarkList.GetMarkCount() == 1 )
    {
        SdrObject* pObj = rMarkList.GetMark( 0 )->GetMarkedSdrObj();

        if( pObj && dynamic_cast<const SdrGrafObj*>( pObj)  != nullptr && static_cast<SdrGrafObj*>(pObj)->GetGraphicType() == GRAPHIC_BITMAP )
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

        if( pObj && nullptr != dynamic_cast<const SdrGrafObj*>( pObj) && ( static_cast<SdrGrafObj*>(pObj)->GetGraphicType() == GRAPHIC_BITMAP ) )
            bEnable = true;
    }

    if( !bEnable )
        rSet.DisableItem( SID_OBJECT_CROP );
}

void ScGraphicShell::ExecuteCropGraphic( SfxRequest& )
{
    ScDrawView* pView = GetViewData()->GetScDrawView();
    const SdrMarkList& rMarkList = pView->GetMarkedObjectList();

    if( rMarkList.GetMarkCount() == 1 )
    {
        SdrObject* pObj = rMarkList.GetMark( 0 )->GetMarkedSdrObj();

        if( pObj && dynamic_cast<const SdrGrafObj*>( pObj) != nullptr && static_cast<SdrGrafObj*>(pObj)->GetGraphicType() == GRAPHIC_BITMAP )
        {
            pView->SetEditMode(SDREDITMODE_EDIT);
            pView->SetDragMode(SDRDRAG_CROP);
        }
    }

    Invalidate();
}

void ScGraphicShell::ExecuteSaveGraphic(SfxRequest& /*rReq*/)
{
    ScDrawView* pView = GetViewData()->GetScDrawView();
    const SdrMarkList& rMarkList = pView->GetMarkedObjectList();
    if( rMarkList.GetMarkCount() == 1 )
    {
        SdrObject* pObj = rMarkList.GetMark( 0 )->GetMarkedSdrObj();
        if( pObj && dynamic_cast<const SdrGrafObj*>( pObj) != nullptr && static_cast<SdrGrafObj*>(pObj)->GetGraphicType() == GRAPHIC_BITMAP )
        {
            GraphicObject aGraphicObject( static_cast<SdrGrafObj*>( pObj )->GetGraphicObject() );
            {
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

        if( pObj && dynamic_cast<const SdrGrafObj*>( pObj) != nullptr && ( static_cast<SdrGrafObj*>(pObj)->GetGraphicType() == GRAPHIC_BITMAP ) )
            bEnable = true;
    }

    if( !bEnable )
        rSet.DisableItem( SID_SAVE_GRAPHIC );
}

void ScGraphicShell::ExecuteChangePicture(SfxRequest& /*rReq*/)
{
    ScDrawView* pView = GetViewData()->GetScDrawView();
    const SdrMarkList& rMarkList = pView->GetMarkedObjectList();

    if( rMarkList.GetMarkCount() == 1 )
    {
        SdrObject* pObj = rMarkList.GetMark( 0 )->GetMarkedSdrObj();

        if( pObj && nullptr != dynamic_cast<const SdrGrafObj*>( pObj) && static_cast<SdrGrafObj*>(pObj)->GetGraphicType() == GRAPHIC_BITMAP )
        {
            SdrGrafObj* pGraphicObj = static_cast<SdrGrafObj*>(pObj);
            SvxOpenGraphicDialog aDlg(ScResId(STR_INSERTGRAPHIC));

            if( aDlg.Execute() == GRFILTER_OK )
            {
                Graphic aGraphic;
                int nError = aDlg.GetGraphic(aGraphic);
                if( nError == GRFILTER_OK )
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

        if( pObj && dynamic_cast<const SdrGrafObj*>( pObj) != nullptr && ( static_cast<SdrGrafObj*>(pObj)->GetGraphicType() == GRAPHIC_BITMAP ) )
            bEnable = true;
    }

    if( !bEnable )
        rSet.DisableItem( SID_CHANGE_PICTURE );
}



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
