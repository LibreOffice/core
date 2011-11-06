/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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

