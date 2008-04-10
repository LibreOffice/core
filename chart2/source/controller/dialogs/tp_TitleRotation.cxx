/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: tp_TitleRotation.cxx,v $
 * $Revision: 1.11 $
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
#include "precompiled_chart2.hxx"
#include "tp_TitleRotation.hxx"

#include "ResId.hxx"
#include "TabPages.hrc"
#include "chartview/ChartSfxItemIds.hxx"

// header for class SfxInt32Item
#include <svtools/intitem.hxx>

//.............................................................................
namespace chart
{
//.............................................................................

SchAlignmentTabPage::SchAlignmentTabPage(Window* pWindow,
                                         const SfxItemSet& rInAttrs) :
    SfxTabPage(pWindow, SchResId(TP_ALIGNMENT), rInAttrs),
    aFlAlign        ( this, SchResId( FL_ALIGN ) ),
    aCtrlDial       ( this, SchResId( CTR_DIAL ) ),
    aFtRotate       ( this, SchResId( FT_DEGREES ) ),
    aNfRotate       ( this, SchResId( NF_ORIENT ) ),
    aCbStacked      ( this, SchResId( BTN_TXTSTACKED ) ),
    aOrientHlp      ( this, aCtrlDial, aNfRotate, aCbStacked )
{
    FreeResource();

    aCbStacked.EnableTriState( FALSE );
    aOrientHlp.AddDependentWindow( aFtRotate, STATE_CHECK );
}

SchAlignmentTabPage::~SchAlignmentTabPage()
{
}

SfxTabPage* SchAlignmentTabPage::Create(Window* pWindow,
                                        const SfxItemSet& rOutAttrs)
{
    return new SchAlignmentTabPage(pWindow, rOutAttrs);
}

BOOL SchAlignmentTabPage::FillItemSet(SfxItemSet& rOutAttrs)
{
    //Seit 4/1998 koennen Texte frei gedreht werden: SCHATTR_TEXT_DEGREES
    bool bStacked = aOrientHlp.GetStackedState() == STATE_CHECK;
    rOutAttrs.Put( SfxBoolItem( SCHATTR_TEXT_STACKED, bStacked ) );

    sal_Int32 nDegrees = bStacked ? 0 : aCtrlDial.GetRotation();
    rOutAttrs.Put( SfxInt32Item( SCHATTR_TEXT_DEGREES, nDegrees ) );
    return TRUE;
}

void SchAlignmentTabPage::Reset(const SfxItemSet& rInAttrs)
{
    const SfxPoolItem* pItem = GetItem( rInAttrs, SCHATTR_TEXT_DEGREES );

    sal_Int32 nDegrees = pItem ? ((const SfxInt32Item*)pItem)->GetValue() : 0;
    aCtrlDial.SetRotation( nDegrees );

    pItem = GetItem( rInAttrs, SCHATTR_TEXT_STACKED );
    bool bStacked = pItem && ((const SfxBoolItem*)pItem)->GetValue();
    aOrientHlp.SetStackedState( bStacked ? STATE_CHECK : STATE_NOCHECK );
}

//.............................................................................
} //namespace chart
//.............................................................................


