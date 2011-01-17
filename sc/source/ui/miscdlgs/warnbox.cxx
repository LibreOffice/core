/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"
#include "warnbox.hxx"

#include "scmod.hxx"
#include "inputopt.hxx"
#include "scresid.hxx"
#include "sc.hrc"


// ============================================================================

ScCbWarningBox::ScCbWarningBox( Window* pParent, const String& rMsgStr, bool bDefYes ) :
    WarningBox( pParent, WB_YES_NO | (bDefYes ? WB_DEF_YES : WB_DEF_NO), rMsgStr )
{
    SetDefaultCheckBoxText();
}

sal_Int16 ScCbWarningBox::Execute()
{
    sal_Int16 nRet = (GetStyle() & WB_DEF_YES) ? RET_YES : RET_NO;
    if( IsDialogEnabled() )
    {
        nRet = WarningBox::Execute();
        if( GetCheckBoxState() )
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


// ----------------------------------------------------------------------------

ScReplaceWarnBox::ScReplaceWarnBox( Window* pParent ) :
    ScCbWarningBox( pParent, String( ScResId( STR_REPLCELLSWARN ) ), true )
{
    SetHelpId( HID_SC_REPLCELLSWARN );
}

bool ScReplaceWarnBox::IsDialogEnabled()
{
    return SC_MOD()->GetInputOptions().GetReplaceCellsWarn() == sal_True;
}

void ScReplaceWarnBox::DisableDialog()
{
    ScModule* pScMod = SC_MOD();
    ScInputOptions aInputOpt( pScMod->GetInputOptions() );
    aInputOpt.SetReplaceCellsWarn( sal_False );
    pScMod->SetInputOptions( aInputOpt );
}


// ============================================================================

