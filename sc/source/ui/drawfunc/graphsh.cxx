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
#include <svl/whiter.hxx>
#include <svx/svdograf.hxx>
#include <svx/grfflt.hxx>
#include <svx/grafctrl.hxx>
#include <svx/compressgraphicdialog.hxx>
#include <vcl/msgbox.hxx>

#include "graphsh.hxx"
#include "sc.hrc"
#include "viewdata.hxx"
#include "drawview.hxx"
#include "scresid.hxx"
#include <svx/extedit.hxx>

#define ScGraphicShell
#include "scslots.hxx"

class ScExternalToolEdit : public ExternalToolEdit
{
    FmFormView* m_pView;
    SdrObject*  m_pObj;

public:
    ScExternalToolEdit ( FmFormView* pView, SdrObject* pObj ) :
        m_pView   (pView),
        m_pObj (pObj)
    {}

    virtual void Update( Graphic& aGraphic )
    {
        SdrPageView* pPageView = m_pView->GetSdrPageView();
        if( pPageView )
        {
            SdrGrafObj* pNewObj = (SdrGrafObj*) m_pObj->Clone();
            String      aStr( m_pView->GetDescriptionOfMarkedObjects() );
            aStr.Append( sal_Unicode(' ') );
            aStr.Append( String( "External Edit" ) );
            m_pView->BegUndo( aStr );
            pNewObj->SetGraphicObject( aGraphic );
            m_pView->ReplaceObjectAtView( m_pObj, *pPageView, pNewObj );
            m_pView->EndUndo();
        }
    }
};

SFX_IMPL_INTERFACE(ScGraphicShell, ScDrawShell, ScResId(SCSTR_GRAPHICSHELL) )
{
    SFX_OBJECTBAR_REGISTRATION( SFX_OBJECTBAR_OBJECT|SFX_VISIBILITY_STANDARD|SFX_VISIBILITY_SERVER,
                                ScResId(RID_GRAPHIC_OBJECTBAR) );
    SFX_POPUPMENU_REGISTRATION( ScResId(RID_POPUP_GRAPHIC) );
}

TYPEINIT1( ScGraphicShell, ScDrawShell );

ScGraphicShell::ScGraphicShell(ScViewData* pData) :
    ScDrawShell(pData)
{
    SetHelpId(HID_SCSHELL_GRAPHIC);
    SetName(OUString("GraphicObject"));
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
    sal_Bool bEnable = false;

    if( rMarkList.GetMarkCount() == 1 )
    {
        SdrObject* pObj = rMarkList.GetMark( 0 )->GetMarkedSdrObj();

        if( pObj && pObj->ISA( SdrGrafObj ) && ( ( (SdrGrafObj*) pObj )->GetGraphicType() == GRAPHIC_BITMAP ) )
            bEnable = sal_True;
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

        if( pObj && pObj->ISA( SdrGrafObj ) && ( (SdrGrafObj*) pObj )->GetGraphicType() == GRAPHIC_BITMAP )
        {
            GraphicObject aFilterObj( ( (SdrGrafObj*) pObj )->GetGraphicObject() );

            if( SVX_GRAPHICFILTER_ERRCODE_NONE ==
                SvxGraphicFilter::ExecuteGrfFilterSlot( rReq, aFilterObj ) )
            {
                SdrPageView* pPageView = pView->GetSdrPageView();

                if( pPageView )
                {
                    SdrGrafObj* pFilteredObj = (SdrGrafObj*) pObj->Clone();
                    String      aStr( pView->GetDescriptionOfMarkedObjects() );

                    aStr.Append( sal_Unicode(' ') );
                    aStr.Append( String( ScResId( SCSTR_UNDO_GRAFFILTER ) ) );
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

        if( pObj && pObj->ISA( SdrGrafObj ) && ( ( (SdrGrafObj*) pObj )->GetGraphicType() == GRAPHIC_BITMAP ) )
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

        if( pObj && pObj->ISA( SdrGrafObj ) && ( (SdrGrafObj*) pObj )->GetGraphicType() == GRAPHIC_BITMAP )
        {
            GraphicObject aGraphicObject( ( (SdrGrafObj*) pObj )->GetGraphicObject() );
            ScExternalToolEdit* aExternalToolEdit = new ScExternalToolEdit( pView, pObj );
            aExternalToolEdit->Edit( &aGraphicObject );
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

        if( pObj && pObj->ISA( SdrGrafObj ) && ( ( (SdrGrafObj*) pObj )->GetGraphicType() == GRAPHIC_BITMAP ) )
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

        if( pObj && pObj->ISA( SdrGrafObj ) && ( (SdrGrafObj*) pObj )->GetGraphicType() == GRAPHIC_BITMAP )
        {
            SdrGrafObj* pGraphicObj = (SdrGrafObj*) pObj;
            CompressGraphicsDialog dialog( GetViewData()->GetDialogParent(), pGraphicObj, GetViewData()->GetBindings() );
            if ( dialog.Execute() == RET_OK )
            {
                SdrGrafObj* pNewObject = dialog.GetCompressedSdrGrafObj();
                SdrPageView* pPageView = pView->GetSdrPageView();
                String aUndoString( pView->GetDescriptionOfMarkedObjects() );
                aUndoString += String( " Compress" );
                pView->BegUndo( aUndoString );
                pView->ReplaceObjectAtView( pObj, *pPageView, pNewObject );
                pView->EndUndo();
            }
        }
    }

    Invalidate();
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
