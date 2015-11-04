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
#include <sfx2/sidebar/EnumContext.hxx>
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

using namespace css::uno;

namespace drawing = com::sun::star::drawing;

SFX_IMPL_INTERFACE(ScChartShell, ScDrawShell)

void ScChartShell::InitInterface_Impl()
{
    GetStaticInterface()->RegisterObjectBar(SFX_OBJECTBAR_OBJECT | SFX_VISIBILITY_STANDARD | SFX_VISIBILITY_SERVER,
                                            RID_DRAW_OBJECTBAR);

    GetStaticInterface()->RegisterPopupMenu(ScResId(RID_POPUP_CHART));
}

TYPEINIT1( ScChartShell, ScDrawShell );

ScChartShell::ScChartShell(ScViewData* pData) :
    ScDrawShell(pData)
{
    SetHelpId( HID_SCSHELL_CHARTSH );
    SetName( OUString("ChartObject") );
    SfxShell::SetContextName(sfx2::sidebar::EnumContext::GetContextName(sfx2::sidebar::EnumContext::Context_Chart));
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

        if( pObj && dynamic_cast<const SdrOle2Obj*>( pObj) !=  nullptr )
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

        if( pObject && dynamic_cast<const SdrOle2Obj*>( pObject) !=  nullptr )
        {
            Reference< drawing::XShape > xSourceDoc( pObject->getUnoShape(), UNO_QUERY_THROW );
            GraphicHelper::SaveShapeAsGraphic( xSourceDoc );
        }
    }

    Invalidate();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
