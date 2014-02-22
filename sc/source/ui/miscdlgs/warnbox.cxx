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

#include "warnbox.hxx"

#include "scmod.hxx"
#include "inputopt.hxx"
#include "scresid.hxx"
#include "sc.hrc"


// ============================================================================

ScCbWarningBox::ScCbWarningBox( Window* pParent, const OUString& rMsgStr, bool bDefYes ) :
    WarningBox( pParent, WB_YES_NO | (bDefYes ? WB_DEF_YES : WB_DEF_NO), rMsgStr )
{
    // By default, the check box is ON, and the user needs to un-check it to
    // disable all future warnings.
    SetCheckBoxState(true);
    SetCheckBoxText(ScResId(SCSTR_WARN_ME_IN_FUTURE_CHECK));
}

sal_Int16 ScCbWarningBox::Execute()
{
    sal_Int16 nRet = (GetStyle() & WB_DEF_YES) ? RET_YES : RET_NO;
    if( IsDialogEnabled() )
    {
        nRet = WarningBox::Execute();
        if (!GetCheckBoxState())
            DisableDialog();
    }
    return nRet;
}

bool ScCbWarningBox::IsDialogEnabled()
{
    return true;
}

void ScCbWarningBox::DisableDialog()
{
}




ScReplaceWarnBox::ScReplaceWarnBox( Window* pParent ) :
    ScCbWarningBox( pParent, OUString( ScResId( STR_REPLCELLSWARN ) ), true )
{
    SetHelpId( HID_SC_REPLCELLSWARN );
}

bool ScReplaceWarnBox::IsDialogEnabled()
{
    return ((bool) SC_MOD()->GetInputOptions().GetReplaceCellsWarn()) == true;
}

void ScReplaceWarnBox::DisableDialog()
{
    ScModule* pScMod = SC_MOD();
    ScInputOptions aInputOpt( pScMod->GetInputOptions() );
    aInputOpt.SetReplaceCellsWarn( false );
    pScMod->SetInputOptions( aInputOpt );
}


// ============================================================================

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
