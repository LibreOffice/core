/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Version: MPL 1.1 / GPLv3+ / LGPLv3+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License or as specified alternatively below. You may obtain a copy of
 * the License at http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * Major Contributor(s):
 * Copyright (C) 2012 Markus Mohrhard <markus.mohrhard@googlemail.com> (initial developer)
 *
 * All Rights Reserved.
 *
 * For minor contributions see the git repository.
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
 */

#include "colorformat.hxx"

#include "colorformat.hrc"

#include <svx/xtable.hxx>
#include <svx/drawitem.hxx>

ScDataBarSettingsDlg::ScDataBarSettingsDlg(Window* pWindow):
    ModalDialog( pWindow, ScResId( RID_SCDLG_DATABAR ) ),
    maBtnOk( this, ScResId( BTN_OK ) ),
    maBtnCancel( this, ScResId( BTN_CANCEL ) ),
    maFlBarColors( this, ScResId( FL_BAR_COLORS ) ),
    maFlAxes( this, ScResId( FL_AXIS ) ),
    maFlValues( this, ScResId( FL_VALUES ) ),
    maFtMin( this, ScResId( FT_MINIMUM ) ),
    maFtMax( this, ScResId( FT_MAXIMUM ) ),
    maFtPositive( this, ScResId( FT_POSITIVE ) ),
    maFtNegative( this, ScResId( FT_NEGATIVE ) ),
    maFtPosition( this, ScResId( FT_POSITION ) ),
    maFtAxisColor( this, ScResId( FT_COLOR_AXIS ) ),
    maLbPos( this, ScResId( LB_POS ) ),
    maLbNeg( this, ScResId( LB_NEG ) ),
    maLbAxisCol( this, ScResId( LB_COL_AXIS ) ),
    maLbTypeMin( this, ScResId( LB_TYPE ) ),
    maLbTypeMax( this, ScResId( LB_TYPE ) ),
    maLbAxisPos( this, ScResId( LB_AXIS_POSITION ) ),
    maEdMin( this, ScResId( ED_MIN ) ),
    maEdMax( this, ScResId( ED_MAX ) )
{
    SfxObjectShell*     pDocSh      = SfxObjectShell::Current();
    const SfxPoolItem*  pItem       = NULL;
    XColorListRef       pColorTable;

    DBG_ASSERT( pDocSh, "DocShell not found!" );

    if ( pDocSh )
    {
        pItem = pDocSh->GetItem( SID_COLOR_TABLE );
        if ( pItem != NULL )
            pColorTable = ( (SvxColorListItem*)pItem )->GetColorList();
    }

    DBG_ASSERT( pColorTable.is(), "ColorTable not found!" );

    if ( pColorTable.is() )
    {
        // filling the line color box
        maLbPos.SetUpdateMode( false );
        maLbNeg.SetUpdateMode( false );
        maLbAxisCol.SetUpdateMode( false );

        for ( long i = 0; i < pColorTable->Count(); ++i )
        {
            XColorEntry* pEntry = pColorTable->GetColor(i);
            maLbPos.InsertEntry( pEntry->GetColor(), pEntry->GetName() );
            maLbNeg.InsertEntry( pEntry->GetColor(), pEntry->GetName() );
            maLbAxisCol.InsertEntry( pEntry->GetColor(), pEntry->GetName() );

            if(pEntry->GetColor() == Color(COL_LIGHTRED))
                maLbNeg.SelectEntryPos(i);
            if(pEntry->GetColor() == Color(COL_BLACK))
                maLbAxisCol.SelectEntryPos(i);
            if(pEntry->GetColor() == Color(COL_LIGHTBLUE))
                maLbPos.SelectEntryPos(i);
        }
        maLbPos.SetUpdateMode( sal_True );
        maLbNeg.SetUpdateMode( sal_True );
        maLbAxisCol.SetUpdateMode( sal_True );
    }
    FreeResource();

    maLbTypeMin.SelectEntryPos(0);
    maLbTypeMax.SelectEntryPos(1);
    maLbAxisPos.SelectEntryPos(0);
    maBtnOk.SetClickHdl( LINK( this, ScDataBarSettingsDlg, OkBtnHdl ) );

    Point aPoint = maLbTypeMax.GetPosPixel();
    aPoint.Y() += 50;
    maLbTypeMax.SetPosPixel(aPoint);
}

IMPL_LINK_NOARG( ScDataBarSettingsDlg, OkBtnHdl )
{
    //check that min < max
    bool bWarn = false;
    if(maLbTypeMin.GetSelectEntryPos() == 1)
        bWarn = true;
    if(maLbTypeMax.GetSelectEntryPos() == 0)
        bWarn = true;

    if(!bWarn && maLbTypeMin.GetSelectEntryPos() == maLbTypeMax.GetSelectEntryPos())
    {
        if(maLbTypeMax.GetSelectEntryPos() != 5)
        {
            rtl::OUString aMinString = maEdMin.GetText();
            rtl::OUString aMaxString = maEdMax.GetText();
            double nMinValue = rtl::math::stringToDouble(aMinString, '.', ',');
            double nMaxValue = rtl::math::stringToDouble(aMaxString, '.', ',');
            if(rtl::math::approxEqual(nMinValue, nMaxValue) || nMinValue > nMaxValue)
                bWarn = true;
        }
    }

    if(bWarn)
    {
        //show warning message and don't close
    }
    else
    {
        Close();
    }
    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
