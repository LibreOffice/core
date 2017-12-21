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

#include <config_features.h>

#include <basic/sberrors.hxx>
#include <svx/svdmark.hxx>
#include <svx/svdoole2.hxx>
#include <svx/svdview.hxx>
#include <sfx2/app.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/request.hxx>
#include <basic/sbxcore.hxx>
#include <svl/whiter.hxx>
#include <vcl/msgbox.hxx>

#include <tabvwsh.hxx>
#include <client.hxx>
#include <document.hxx>
#include <docsh.hxx>
#include <sc.hrc>
#include <drwlayer.hxx>
#include <retypepassdlg.hxx>
#include <tabprotection.hxx>

using namespace com::sun::star;

void ScTabViewShell::ExecuteObject( const SfxRequest& /*rReq*/ )
{
}

void ScTabViewShell::GetObjectState( SfxItemSet& /*rSet*/ )
{
}

void ScTabViewShell::AddAccessibilityObject( SfxListener& rObject )
{
    if (!pAccessibilityBroadcaster)
        pAccessibilityBroadcaster = new SfxBroadcaster;

    rObject.StartListening( *pAccessibilityBroadcaster );
    ScDocument* pDoc = GetViewData().GetDocument();
    if (pDoc)
        pDoc->AddUnoObject(rObject);
}

void ScTabViewShell::RemoveAccessibilityObject( SfxListener& rObject )
{
    SolarMutexGuard g;

    if (pAccessibilityBroadcaster)
    {
        rObject.EndListening( *pAccessibilityBroadcaster );
        ScDocument* pDoc = GetViewData().GetDocument();
        if (pDoc)
            pDoc->RemoveUnoObject(rObject);
    }
    else
    {
        OSL_FAIL("no accessibility broadcaster?");
    }
}

void ScTabViewShell::BroadcastAccessibility( const SfxHint &rHint )
{
    if (pAccessibilityBroadcaster)
        pAccessibilityBroadcaster->Broadcast( rHint );
}

bool ScTabViewShell::HasAccessibilityObjects()
{
    return pAccessibilityBroadcaster != nullptr;
}

bool ScTabViewShell::ExecuteRetypePassDlg(ScPasswordHash eDesiredHash)
{
    ScDocument* pDoc = GetViewData().GetDocument();

    VclPtrInstance< ScRetypePassDlg > pDlg(GetDialogParent());
    pDlg->SetDataFromDocument(*pDoc);
    pDlg->SetDesiredHash(eDesiredHash);
    if (pDlg->Execute() != RET_OK)
        return false;

    pDlg->WriteNewDataToDocument(*pDoc);
    return true;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
