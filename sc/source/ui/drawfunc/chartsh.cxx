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

#include <sfx2/objface.hxx>
#include <vcl/EnumContext.hxx>

#include <chartsh.hxx>
#include <sc.hrc>
#include <viewdata.hxx>
#include <drawview.hxx>
#include <gridwin.hxx>

#define ShellClass_ScChartShell
#include <scslots.hxx>

using namespace css::uno;

namespace drawing = com::sun::star::drawing;

SFX_IMPL_INTERFACE(ScChartShell, ScDrawShell)

void ScChartShell::InitInterface_Impl()
{
    GetStaticInterface()->RegisterObjectBar(SFX_OBJECTBAR_OBJECT,
                                            SfxVisibilityFlags::Standard | SfxVisibilityFlags::Server,
                                            ToolbarId::Draw_Objectbar);

    GetStaticInterface()->RegisterPopupMenu("oleobject");
}


ScChartShell::ScChartShell(ScViewData* pData) :
    ScDrawShell(pData)
{
    SetName( "ChartObject" );
    SfxShell::SetContextName(vcl::EnumContext::GetContextName(vcl::EnumContext::Context::Chart));
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

        if( dynamic_cast<const SdrOle2Obj*>( pObj) )
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

        if( dynamic_cast<const SdrOle2Obj*>( pObject) )
        {
            vcl::Window* pWin = GetViewData()->GetActiveWin();
            Reference< drawing::XShape > xSourceDoc( pObject->getUnoShape(), UNO_QUERY_THROW );
            GraphicHelper::SaveShapeAsGraphic(pWin ? pWin->GetFrameWeld() : nullptr, xSourceDoc);
        }
    }

    Invalidate();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
