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

#include "cmdid.h"
#include "uiitems.hxx"
#include <vcl/window.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/request.hxx>
#include <sfx2/viewfrm.hxx>
#include <svl/stritem.hxx>
#include <rsc/rscsfx.hxx>

#include "view.hxx"
#include "wrtsh.hxx"
#include "basesh.hxx"

void SwView::ExecColl(SfxRequest &rReq)
{
    const SfxItemSet* pArgs = rReq.GetArgs();
    const SfxPoolItem* pItem = nullptr;
    sal_uInt16 nWhich = rReq.GetSlot();
    switch( nWhich )
    {
        case FN_SET_PAGE:
        {
            OSL_ENSURE(false, "Not implemented");
        }
        break;
        case FN_SET_PAGE_STYLE:
        {
            if( pArgs )
            {
                if (pArgs &&
                    SfxItemState::SET == pArgs->GetItemState( nWhich , true, &pItem ))
                {
                    if( static_cast<const SfxStringItem*>(pItem)->GetValue() !=
                                            GetWrtShell().GetCurPageStyle(false) )
                    {
                        SfxStringItem aName(SID_STYLE_APPLY,
                                   static_cast<const SfxStringItem*>(pItem)->GetValue());
                        SfxUInt16Item aFamItem( SID_STYLE_FAMILY,
                                            SFX_STYLE_FAMILY_PAGE);
                        SwPtrItem aShell(FN_PARAM_WRTSHELL, GetWrtShellPtr());
                        SfxRequest aReq(SID_STYLE_APPLY, SfxCallMode::SLOT, GetPool());
                        aReq.AppendItem(aName);
                        aReq.AppendItem(aFamItem);
                        aReq.AppendItem(aShell);
                        GetCurShell()->ExecuteSlot(aReq);
                    }
                }
            }
            else
            {
                SfxRequest aReq(FN_FORMAT_PAGE_DLG, SfxCallMode::SLOT, GetPool());
                GetCurShell()->ExecuteSlot(aReq);
            }
        }
        break;
        default:
            OSL_FAIL("wrong CommandProcessor for Dispatch");
            return;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
