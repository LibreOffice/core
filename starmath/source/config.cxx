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

#include <svl/itemset.hxx>
#include <svl/hint.hxx>
#include <svl/smplhint.hxx>
#include <unotools/pathoptions.hxx>
#include <sfx2/sfxsids.hrc>
#include <svl/eitem.hxx>
#include <svl/itempool.hxx>
#include <svl/stritem.hxx>
#include <svl/intitem.hxx>
#include <tools/stream.hxx>


#include "config.hxx"
#include "format.hxx"
#include "smmod.hxx"
#include "starmath.hrc"

/////////////////////////////////////////////////////////////////

SmConfig::SmConfig()
{
}


SmConfig::~SmConfig()
{
}


void SmConfig::ItemSetToConfig(const SfxItemSet &rSet)
{
    const SfxPoolItem *pItem     = NULL;

    sal_uInt16 nU16;
    bool bVal;
    if (rSet.GetItemState(SID_PRINTSIZE, sal_True, &pItem) == SFX_ITEM_SET)
    {   nU16 = ((const SfxUInt16Item *) pItem)->GetValue();
        SetPrintSize( (SmPrintSize) nU16 );
    }
    if (rSet.GetItemState(SID_PRINTZOOM, sal_True, &pItem) == SFX_ITEM_SET)
    {   nU16 = ((const SfxUInt16Item *) pItem)->GetValue();
        SetPrintZoomFactor( nU16 );
    }
    if (rSet.GetItemState(SID_PRINTTITLE, sal_True, &pItem) == SFX_ITEM_SET)
    {   bVal = ((const SfxBoolItem *) pItem)->GetValue();
        SetPrintTitle( bVal );
    }
    if (rSet.GetItemState(SID_PRINTTEXT, sal_True, &pItem) == SFX_ITEM_SET)
    {   bVal = ((const SfxBoolItem *) pItem)->GetValue();
        SetPrintFormulaText( bVal );
    }
    if (rSet.GetItemState(SID_PRINTFRAME, sal_True, &pItem) == SFX_ITEM_SET)
    {   bVal = ((const SfxBoolItem *) pItem)->GetValue();
        SetPrintFrame( bVal );
    }
    if (rSet.GetItemState(SID_AUTOREDRAW, sal_True, &pItem) == SFX_ITEM_SET)
    {   bVal = ((const SfxBoolItem *) pItem)->GetValue();
        SetAutoRedraw( bVal );
    }
    if (rSet.GetItemState(SID_NO_RIGHT_SPACES, sal_True, &pItem) == SFX_ITEM_SET)
    {   bVal = ((const SfxBoolItem *) pItem)->GetValue();
        if (IsIgnoreSpacesRight() != bVal)
        {
            SetIgnoreSpacesRight( bVal );

            // reformat (displayed) formulas accordingly
            Broadcast(SfxSimpleHint(HINT_FORMATCHANGED));
        }
    }
    if (rSet.GetItemState(SID_SAVE_ONLY_USED_SYMBOLS, sal_True, &pItem) == SFX_ITEM_SET)
    {   bVal = ((const SfxBoolItem *) pItem)->GetValue();
        SetSaveOnlyUsedSymbols( bVal );
    }

    SaveOther();
}


void SmConfig::ConfigToItemSet(SfxItemSet &rSet) const
{
    const SfxItemPool *pPool = rSet.GetPool();

    rSet.Put(SfxUInt16Item(pPool->GetWhich(SID_PRINTSIZE),
                           (sal_uInt16) GetPrintSize()));
    rSet.Put(SfxUInt16Item(pPool->GetWhich(SID_PRINTZOOM),
                           (sal_uInt16) GetPrintZoomFactor()));

    rSet.Put(SfxBoolItem(pPool->GetWhich(SID_PRINTTITLE), IsPrintTitle()));
    rSet.Put(SfxBoolItem(pPool->GetWhich(SID_PRINTTEXT),  IsPrintFormulaText()));
    rSet.Put(SfxBoolItem(pPool->GetWhich(SID_PRINTFRAME), IsPrintFrame()));
    rSet.Put(SfxBoolItem(pPool->GetWhich(SID_AUTOREDRAW), IsAutoRedraw()));
    rSet.Put(SfxBoolItem(pPool->GetWhich(SID_NO_RIGHT_SPACES), IsIgnoreSpacesRight()));
    rSet.Put(SfxBoolItem(pPool->GetWhich(SID_SAVE_ONLY_USED_SYMBOLS), IsSaveOnlyUsedSymbols()));
}


/////////////////////////////////////////////////////////////////


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
