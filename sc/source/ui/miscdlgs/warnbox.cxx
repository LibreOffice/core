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
#include "strings.hrc"
#include "helpids.h"

ScReplaceWarnBox::ScReplaceWarnBox( vcl::Window* pParent ) :
    WarningBox( pParent, WB_YES_NO | WB_DEF_YES, ScResId( STR_REPLCELLSWARN ) )
{
    // By default, the check box is ON, and the user needs to un-check it to
    // disable all future warnings.
    SetCheckBoxState(true);
    SetCheckBoxText(ScResId(SCSTR_WARN_ME_IN_FUTURE_CHECK));
    SetHelpId( HID_SC_REPLCELLSWARN );
}

sal_Int16 ScReplaceWarnBox::Execute()
{
    sal_Int16 nRet = RET_YES;
    if( SC_MOD()->GetInputOptions().GetReplaceCellsWarn() )
    {
        nRet = WarningBox::Execute();
        if (!GetCheckBoxState())
        {
            ScModule* pScMod = SC_MOD();
            ScInputOptions aInputOpt( pScMod->GetInputOptions() );
            aInputOpt.SetReplaceCellsWarn( false );
            pScMod->SetInputOptions( aInputOpt );
        }
    }
    return nRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
