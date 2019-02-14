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

#include <DrawViewShell.hxx>
#include <svx/xtable.hxx>
#include <sdattr.hxx>
#include <svl/aeitem.hxx>
#include <editeng/eeitem.hxx>
#include <sfx2/request.hxx>
#include <svx/svditer.hxx>
#include <editeng/colritem.hxx>
#include <sfx2/viewfrm.hxx>
#include <svx/svxids.hrc>
#include <svx/svdundo.hxx>
#include <svx/view3d.hxx>
#include <sfx2/dispatch.hxx>
#include <svx/float3d.hxx>
#include <svx/f3dchild.hxx>
#include <svx/dialogs.hrc>
#include <vcl/weld.hxx>

#include <app.hrc>
#include <strings.hrc>

#include <drawdoc.hxx>
#include <DrawDocShell.hxx>
#include <anminfo.hxx>
#include <unoaprms.hxx>
#include <sdundogr.hxx>
#include <drawview.hxx>
#include <Window.hxx>
#include <sdresid.hxx>

using namespace ::com::sun::star;

namespace sd {

/**
 * Handle SfxRequests for EffekteWindow
 */
void DrawViewShell::ExecEffectWin( SfxRequest& rReq )
{
    CheckLineTo (rReq);

    sal_uInt16 nSId = rReq.GetSlot();

    switch( nSId )
    {
        case SID_3D_INIT:
        {
            sal_uInt16 nId = Svx3DChildWindow::GetChildWindowId();
            SfxChildWindow* pWindow = GetViewFrame()->GetChildWindow( nId );
            if( pWindow )
            {
                Svx3DWin* p3DWin = static_cast<Svx3DWin*>( pWindow->GetWindow() );
                if( p3DWin )
                    p3DWin->InitColorLB();
            }
        }
        break;

        case SID_3D_STATE:
        {
            Update3DWindow();
        }
        break;

        case SID_3D_ASSIGN:
        {
            AssignFrom3DWindow();
        }
        break;

    }
}

void DrawViewShell::Update3DWindow()
{
    sal_uInt16 nId = Svx3DChildWindow::GetChildWindowId();
    SfxChildWindow* pWindow = GetViewFrame()->GetChildWindow( nId );
    if( pWindow )
    {
        Svx3DWin* p3DWin = static_cast<Svx3DWin*>( pWindow->GetWindow() );
        if( p3DWin && p3DWin->IsUpdateMode() )
        {
            SfxItemSet aTmpItemSet = GetView()->Get3DAttributes();
            p3DWin->Update( aTmpItemSet );
        }
    }
}

/*----------------------------------------------------------------------------*/

void DrawViewShell::AssignFrom3DWindow()
{
    sal_uInt16 nId = Svx3DChildWindow::GetChildWindowId();
    SfxChildWindow* pWin = GetViewFrame()->GetChildWindow( nId );
    if( !pWin )
        return;

    Svx3DWin* p3DWin = static_cast<Svx3DWin*>( pWin->GetWindow() );
    if( !(p3DWin && GetView()) )
        return;

    if(!GetView()->IsPresObjSelected())
    {
        SfxItemSet aSet( GetDoc()->GetPool(),
            svl::Items<SDRATTR_START,  SDRATTR_END>{});
        p3DWin->GetAttr( aSet );

        // own UNDO-compounding also around transformation in 3D
        GetView()->BegUndo(SdResId(STR_UNDO_APPLY_3D_FAVOURITE));

        if(GetView()->IsConvertTo3DObjPossible())
        {
            // assign only text-attribute
            SfxItemSet aTextSet( GetDoc()->GetPool(),
                svl::Items<EE_ITEMS_START, EE_ITEMS_END>{} );
            aTextSet.Put( aSet, false );
            GetView()->SetAttributes( aTextSet );

            // transform text into 3D
            sal_uInt16 nSId = SID_CONVERT_TO_3D;
            SfxBoolItem aItem( nSId, true );
            GetViewFrame()->GetDispatcher()->ExecuteList(
                nSId, SfxCallMode::SYNCHRON | SfxCallMode::RECORD,
                { &aItem });

            // Determine if a FILL attribute is set.
            // If not, hard set a fill attribute
            drawing::FillStyle eFillStyle = aSet.Get(XATTR_FILLSTYLE).GetValue();
            if(eFillStyle == drawing::FillStyle_NONE)
                aSet.Put(XFillStyleItem (drawing::FillStyle_SOLID));

            // remove some 3DSCENE attributes since these were
            // created by convert to 3D and may not be changed
            // to the defaults again.
            aSet.ClearItem(SDRATTR_3DSCENE_DISTANCE);
            aSet.ClearItem(SDRATTR_3DSCENE_FOCAL_LENGTH);
            aSet.ClearItem(SDRATTR_3DOBJ_DEPTH);
        }

        // assign attribute
        GetView()->Set3DAttributes( aSet );

        // end UNDO
        GetView()->EndUndo();
    }
    else
    {
        vcl::Window* pWindow = GetActiveWindow();
        std::unique_ptr<weld::MessageDialog> xInfoBox(Application::CreateMessageDialog(pWindow ? pWindow->GetFrameWeld() : nullptr,
                                                      VclMessageType::Info, VclButtonsType::Ok,
                                                      SdResId(STR_ACTION_NOTPOSSIBLE)));
        xInfoBox->run();
    }

    // get focus back
    GetActiveWindow()->GrabFocus();
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
