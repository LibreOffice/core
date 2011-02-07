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
#include "precompiled_starmath.hxx"


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
#ifndef _STARMATH_HRC
#include "starmath.hrc"
#endif

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
    sal_Bool bVal;
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

            // (angezeigte) Formeln muessen entsprechen neu formatiert werden.
            // Das erreichen wir mit:
            Broadcast(SfxSimpleHint(HINT_FORMATCHANGED));
        }
    }
    if (rSet.GetItemState(SID_SAVE_ONLY_USED_SYMBOLS, TRUE, &pItem) == SFX_ITEM_SET)
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


