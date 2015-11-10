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

#include "svx/dlgutil.hxx"
#include <svl/itemset.hxx>
#include <sfx2/sfxsids.hrc>
#include <sfx2/module.hxx>
#include <svl/intitem.hxx>
#include <svl/eitem.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/objsh.hxx>



FieldUnit GetModuleFieldUnit( const SfxItemSet& rSet )
{
    FieldUnit eUnit = FUNIT_INCH;
    const SfxPoolItem* pItem = nullptr;
    if ( SfxItemState::SET == rSet.GetItemState( SID_ATTR_METRIC, false, &pItem ) )
        eUnit = (FieldUnit) static_cast<const SfxUInt16Item*>(pItem)->GetValue();
    else
    {
        return SfxModule::GetCurrentFieldUnit();
    }

    return eUnit;
}

bool GetApplyCharUnit( const SfxItemSet& rSet )
{
    bool  bUseCharUnit = false;
    const SfxPoolItem* pItem = nullptr;
    if ( SfxItemState::SET == rSet.GetItemState( SID_ATTR_APPLYCHARUNIT, false, &pItem ) )
        bUseCharUnit =  static_cast<const SfxBoolItem*>(pItem)->GetValue();
    else
    {
        // FIXME - this might be wrong, cf. the DEV300 changes in GetModuleFieldUnit()
        SfxViewFrame* pFrame = SfxViewFrame::Current();
        SfxObjectShell* pSh = nullptr;
        if ( pFrame )
            pSh = pFrame->GetObjectShell();
        if ( pSh )  // the object shell is not always available during reload
        {
            SfxModule* pModule = pSh->GetModule();
            if ( pModule )
            {
                pItem = pModule->GetItem( SID_ATTR_APPLYCHARUNIT );
                if ( pItem )
                    bUseCharUnit = static_cast<const SfxBoolItem*>(pItem)->GetValue();
            }
            else
            {
                SAL_WARN( "svx.dialog", "GetApplyCharUnit(): no module found" );
            }
        }
    }
    return bUseCharUnit;
}

FieldUnit GetModuleFieldUnit()
{
    return SfxModule::GetCurrentFieldUnit();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
