/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: config.cxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 15:05:21 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#pragma hdrstop

#ifndef _SFXITEMSET_HXX //autogen
#include <svtools/itemset.hxx>
#endif
#ifndef _SFXHINT_HXX //autogen
#include <svtools/hint.hxx>
#endif
#ifndef _SFXSMPLHINT_HXX //autogen
#include <svtools/smplhint.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_PATHOPTIONS_HXX
#include <svtools/pathoptions.hxx>
#endif
#ifndef _SFXSIDS_HRC //autogen
#include <sfx2/sfxsids.hrc>
#endif
#ifndef _SFXENUMITEM_HXX //autogen
#include <svtools/eitem.hxx>
#endif
#ifndef _SFXITEMPOOL_HXX //autogen
#include <svtools/itempool.hxx>
#endif
#ifndef _SFXSTRITEM_HXX //autogen
#include <svtools/stritem.hxx>
#endif
#ifndef _SFXINTITEM_HXX //autogen
#include <svtools/intitem.hxx>
#endif
#ifndef _STREAM_HXX
#include <tools/stream.hxx>
#endif


#ifndef CONFIG_HXX
#include "config.hxx"
#endif
#ifndef FORMAT_HXX
#include "format.hxx"
#endif
#ifndef _SMMOD_HXX
#include "smmod.hxx"
#endif
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


