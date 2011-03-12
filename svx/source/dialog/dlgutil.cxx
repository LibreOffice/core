/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
#include "precompiled_svx.hxx"

// include ---------------------------------------------------------------

#include "svx/dlgutil.hxx"
#include <svl/itemset.hxx>
#include <sfx2/sfxsids.hrc>
#include <sfx2/module.hxx>
#include <svl/intitem.hxx>
#include <svl/eitem.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/objsh.hxx>

// -----------------------------------------------------------------------

FieldUnit GetModuleFieldUnit( const SfxItemSet& rSet )
{
    FieldUnit eUnit = FUNIT_INCH;
    const SfxPoolItem* pItem = NULL;
    if ( SFX_ITEM_SET == rSet.GetItemState( SID_ATTR_METRIC, sal_False, &pItem ) )
        eUnit = (FieldUnit)( (const SfxUInt16Item*)pItem )->GetValue();
    else
    {
        OSL_FAIL("Using fallback for field unit - field unit should be provided in ItemSet");
        return SfxModule::GetCurrentFieldUnit();
    }

    return eUnit;
}

BOOL GetApplyCharUnit( const SfxItemSet& rSet )
{
    BOOL  bUseCharUnit = FALSE;
    const SfxPoolItem* pItem = NULL;
    if ( SFX_ITEM_SET == rSet.GetItemState( SID_ATTR_APPLYCHARUNIT, FALSE, &pItem ) )
        bUseCharUnit = (BOOL)( (const SfxBoolItem*)pItem )->GetValue();
    else
    {
        // FIXME - this might be wrong, cf. the DEV300 changes in GetModuleFieldUnit()
        SfxViewFrame* pFrame = SfxViewFrame::Current();
        SfxObjectShell* pSh = NULL;
        if ( pFrame )
            pSh = pFrame->GetObjectShell();
        if ( pSh )  // the object shell is not always available during reload
        {
            SfxModule* pModule = pSh->GetModule();
            if ( pModule )
            {
                pItem = pModule->GetItem( SID_ATTR_APPLYCHARUNIT );
                if ( pItem )
                    bUseCharUnit = (BOOL)( (SfxBoolItem*)pItem )->GetValue();
            }
            else
            {
                DBG_ERRORFILE( "GetApplyCharUnit(): no module found" );
            }
        }
    }
    return bUseCharUnit;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
