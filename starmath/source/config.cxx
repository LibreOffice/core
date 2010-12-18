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

    UINT16 nU16;
    BOOL bVal;
    if (rSet.GetItemState(SID_PRINTSIZE, TRUE, &pItem) == SFX_ITEM_SET)
    {   nU16 = ((const SfxUInt16Item *) pItem)->GetValue();
        SetPrintSize( (SmPrintSize) nU16 );
    }
    if (rSet.GetItemState(SID_PRINTZOOM, TRUE, &pItem) == SFX_ITEM_SET)
    {   nU16 = ((const SfxUInt16Item *) pItem)->GetValue();
        SetPrintZoomFactor( nU16 );
    }
    if (rSet.GetItemState(SID_PRINTTITLE, TRUE, &pItem) == SFX_ITEM_SET)
    {   bVal = ((const SfxBoolItem *) pItem)->GetValue();
        SetPrintTitle( bVal );
    }
    if (rSet.GetItemState(SID_PRINTTEXT, TRUE, &pItem) == SFX_ITEM_SET)
    {   bVal = ((const SfxBoolItem *) pItem)->GetValue();
        SetPrintFormulaText( bVal );
    }
    if (rSet.GetItemState(SID_PRINTFRAME, TRUE, &pItem) == SFX_ITEM_SET)
    {   bVal = ((const SfxBoolItem *) pItem)->GetValue();
        SetPrintFrame( bVal );
    }
    if (rSet.GetItemState(SID_AUTOREDRAW, TRUE, &pItem) == SFX_ITEM_SET)
    {   bVal = ((const SfxBoolItem *) pItem)->GetValue();
        SetAutoRedraw( bVal );
    }
    if (rSet.GetItemState(SID_NO_RIGHT_SPACES, TRUE, &pItem) == SFX_ITEM_SET)
    {   bVal = ((const SfxBoolItem *) pItem)->GetValue();
        if (IsIgnoreSpacesRight() != bVal)
        {
            SetIgnoreSpacesRight( bVal );

            // (angezeigte) Formeln muessen entsprechen neu formatiert werden.
            // Das erreichen wir mit:
            Broadcast(SfxSimpleHint(HINT_FORMATCHANGED));
        }
    }

    SaveOther();
}


void SmConfig::ConfigToItemSet(SfxItemSet &rSet) const
{
    const SfxItemPool *pPool = rSet.GetPool();

    rSet.Put(SfxUInt16Item(pPool->GetWhich(SID_PRINTSIZE),
                           (UINT16) GetPrintSize()));
    rSet.Put(SfxUInt16Item(pPool->GetWhich(SID_PRINTZOOM),
                           (UINT16) GetPrintZoomFactor()));

    rSet.Put(SfxBoolItem(pPool->GetWhich(SID_PRINTTITLE), IsPrintTitle()));
    rSet.Put(SfxBoolItem(pPool->GetWhich(SID_PRINTTEXT),  IsPrintFormulaText()));
    rSet.Put(SfxBoolItem(pPool->GetWhich(SID_PRINTFRAME), IsPrintFrame()));
    rSet.Put(SfxBoolItem(pPool->GetWhich(SID_AUTOREDRAW), IsAutoRedraw()));
    rSet.Put(SfxBoolItem(pPool->GetWhich(SID_NO_RIGHT_SPACES), IsIgnoreSpacesRight()));
}


/////////////////////////////////////////////////////////////////


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
