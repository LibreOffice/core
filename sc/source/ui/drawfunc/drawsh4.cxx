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

#include "scitems.hxx"
#include <svx/drawitem.hxx>
#include <svx/fontwork.hxx>
#include <svx/svdotext.hxx>
#include <svx/xdef.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/viewfrm.hxx>
#include <svx/svdoashp.hxx>
#include "drawsh.hxx"
#include "drawview.hxx"
#include "viewdata.hxx"
#include "tabvwsh.hxx"
#include "sc.hrc"

void ScDrawShell::GetFormTextState(SfxItemSet& rSet)
{
    const SdrObject*    pObj        = nullptr;
    SvxFontWorkDialog*  pDlg        = nullptr;
    ScDrawView*         pDrView     = pViewData->GetScDrawView();
    const SdrMarkList&  rMarkList   = pDrView->GetMarkedObjectList();
    sal_uInt16              nId = SvxFontWorkChildWindow::GetChildWindowId();

    SfxViewFrame* pViewFrm = pViewData->GetViewShell()->GetViewFrame();
    if (pViewFrm->HasChildWindow(nId))
    {
        SfxChildWindow* pWnd = pViewFrm->GetChildWindow(nId);
        pDlg = pWnd ? static_cast<SvxFontWorkDialog*>(pWnd->GetWindow()) : nullptr;
    }

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
        if ( pDlg )
        {
            SfxObjectShell* pDocSh = SfxObjectShell::Current();

            if ( pDocSh )
            {
                const SfxPoolItem*  pItem = pDocSh->GetItem( SID_COLOR_TABLE );
                XColorListRef pColorList;

                if ( pItem )
                    pColorList = static_cast<const SvxColorListItem*>(pItem)->GetColorList();

                if ( pColorList.is() )
                    pDlg->SetColorList( pColorList );
            }
        }
        SfxItemSet aViewAttr(pDrView->GetModel()->GetItemPool());
        pDrView->GetAttributes(aViewAttr);
        rSet.Set(aViewAttr);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
