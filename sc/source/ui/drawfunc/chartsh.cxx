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

#include <svx/svdoole2.hxx>
#include <svx/svdobj.hxx>
#include <svx/graphichelper.hxx>

#include <svl/srchitem.hxx>
#include <sfx2/app.hxx>
#include <sfx2/objface.hxx>
#include <sfx2/request.hxx>
#include <svl/whiter.hxx>
#include <vcl/msgbox.hxx>

#include "chartsh.hxx"
#include "drwlayer.hxx"
#include "sc.hrc"
#include "viewdata.hxx"
#include "document.hxx"
#include "docpool.hxx"
#include "drawview.hxx"
#include "scresid.hxx"

#define ScChartShell
#include "scslots.hxx"

SFX_IMPL_INTERFACE(ScChartShell, ScDrawShell, ScResId(SCSTR_CHARTSHELL) )
{
    SFX_OBJECTBAR_REGISTRATION( SFX_OBJECTBAR_OBJECT | SFX_VISIBILITY_STANDARD | SFX_VISIBILITY_SERVER,
                                ScResId(RID_DRAW_OBJECTBAR) );
    SFX_POPUPMENU_REGISTRATION( ScResId(RID_POPUP_CHART) );
}

TYPEINIT1( ScChartShell, ScDrawShell );

ScChartShell::ScChartShell(ScViewData* pData) :
    ScDrawShell(pData)
{
    SetHelpId( HID_SCSHELL_CHARTSH );
    SetName( OUString("ChartObject") );
}

ScChartShell::~ScChartShell()
{
}

void ScChartShell::GetExportAsGraphicState( SfxItemSet& rSet )
{
    ScDrawView* pView = GetViewData()->GetScDrawView();
    const SdrMarkList& rMarkList = pView->GetMarkedObjectList();
    bool bEnable = false;
    if( rMarkList.GetMarkCount() == 1 )
    {
        SdrObject* pObj = rMarkList.GetMark( 0 )->GetMarkedSdrObj();

        if( pObj && pObj->ISA( SdrOle2Obj ) )
            bEnable = true;
    }

    if( !bEnable )
        rSet.DisableItem( SID_EXPORT_AS_GRAPHIC );
}

void ScChartShell::ExecuteExportAsGraphic( SfxRequest& )
{
    ScDrawView* pView = GetViewData()->GetScDrawView();
    const SdrMarkList& rMarkList = pView->GetMarkedObjectList();

    if( rMarkList.GetMarkCount() == 1 )
    {
        SdrObject* pObject = rMarkList.GetMark( 0 )->GetMarkedSdrObj();

        if( pObject && pObject->ISA( SdrOle2Obj ) )
        {
            SdrOle2Obj* aOle2Object = ((SdrOle2Obj*) pObject)->Clone();
            aOle2Object->NbcResize(Point(), Fraction(1,1), Fraction(1,1));
            Graphic* pGraphic = aOle2Object->GetGraphic();
            if( pGraphic != NULL )
            {
                String sGrfNm, sFilterNm;
                GraphicHelper::ExportGraphic( *pGraphic,  String("") );
            }
        }
    }

    Invalidate();
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
