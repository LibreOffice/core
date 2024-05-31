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
#include <sfx2/sidebar/SidebarController.hxx>
#include <tabvwsh.hxx>

#define ShellClass_ScChartShell
#include <scslots.hxx>

using namespace css::uno;
using namespace sfx2::sidebar;

namespace drawing = com::sun::star::drawing;

namespace {

bool inChartOrMathContext(const ScTabViewShell* pViewShell)
{
    SidebarController* pSidebar = SidebarController::GetSidebarControllerForView(pViewShell);
    if (pSidebar)
        return pSidebar->hasChartOrMathContextCurrently();

    return false;
}

} // anonymous namespace

SFX_IMPL_INTERFACE(ScChartShell, ScDrawShell)

void ScChartShell::InitInterface_Impl()
{
    GetStaticInterface()->RegisterObjectBar(SFX_OBJECTBAR_OBJECT,
                                            SfxVisibilityFlags::Standard | SfxVisibilityFlags::Server,
                                            ToolbarId::Draw_Objectbar);

    GetStaticInterface()->RegisterPopupMenu(u"oleobject"_ustr);
}

void ScChartShell::Activate(bool bMDI)
{
    if(!inChartOrMathContext(GetViewData().GetViewShell()))
        ScDrawShell::Activate(bMDI);
    else
    {
        // Avoid context changes for chart/math during activation / deactivation.
        const bool bIsContextBroadcasterEnabled (SfxShell::SetContextBroadcasterEnabled(false));

        SfxShell::Activate(bMDI);

        SfxShell::SetContextBroadcasterEnabled(bIsContextBroadcasterEnabled);
    }
}

void ScChartShell::Deactivate(bool bMDI)
{
    if(!inChartOrMathContext(GetViewData().GetViewShell()))
        ScDrawShell::Deactivate(bMDI);
    else
    {
        // Avoid context changes for chart/math during activation / deactivation.
        const bool bIsContextBroadcasterEnabled (SfxShell::SetContextBroadcasterEnabled(false));

        SfxShell::Deactivate(bMDI);

        SfxShell::SetContextBroadcasterEnabled(bIsContextBroadcasterEnabled);
    }
}

ScChartShell::ScChartShell(ScViewData& rData) :
    ScDrawShell(rData)
{
    SetName( u"ChartObject"_ustr );
    SfxShell::SetContextName(vcl::EnumContext::GetContextName(vcl::EnumContext::Context::Chart));
}

ScChartShell::~ScChartShell()
{
}

void ScChartShell::GetExportAsGraphicState( SfxItemSet& rSet )
{
    ScDrawView* pView = GetViewData().GetScDrawView();
    const SdrMarkList& rMarkList = pView->GetMarkedObjectList();
    bool bEnable = false;
    if( rMarkList.GetMarkCount() == 1 )
    {
        SdrObject* pObj = rMarkList.GetMark( 0 )->GetMarkedSdrObj();

        if( dynamic_cast<const SdrOle2Obj*>( pObj) )
            bEnable = true;
    }

    if (GetObjectShell()->isExportLocked())
        bEnable = false;

    if( !bEnable )
        rSet.DisableItem( SID_EXPORT_AS_GRAPHIC );
}

void ScChartShell::ExecuteExportAsGraphic( SfxRequest& )
{
    ScDrawView* pView = GetViewData().GetScDrawView();
    const SdrMarkList& rMarkList = pView->GetMarkedObjectList();

    if( rMarkList.GetMarkCount() == 1 )
    {
        SdrObject* pObject = rMarkList.GetMark( 0 )->GetMarkedSdrObj();

        if( dynamic_cast<const SdrOle2Obj*>( pObject) )
        {
            vcl::Window* pWin = GetViewData().GetActiveWin();
            css::uno::Reference<css::lang::XComponent> xComponent;
            const SfxObjectShell* pShell = GetObjectShell();
            if (pShell)
            {
                xComponent = pShell->GetModel();
            }
            Reference< drawing::XShape > xSourceDoc( pObject->getUnoShape() );
            GraphicHelper::SaveShapeAsGraphic(pWin ? pWin->GetFrameWeld() : nullptr, xComponent,
                                              xSourceDoc);
        }
    }

    Invalidate();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
