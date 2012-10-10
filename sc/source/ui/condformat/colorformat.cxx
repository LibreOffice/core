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
#include "colorscale.hxx"

#include "colorformat.hrc"
#include "document.hxx"

#include <svx/xtable.hxx>
#include <svx/drawitem.hxx>
#include <vcl/msgbox.hxx>

ScDataBarSettingsDlg::ScDataBarSettingsDlg(Window* pWindow, ScDocument* pDoc):
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
    maEdMax( this, ScResId( ED_MAX ) ),
    mpNumberFormatter( pDoc->GetFormatTable() )
{
    Init();
    FreeResource();

    maLbTypeMin.SelectEntryPos(0);
    maLbTypeMax.SelectEntryPos(0);
    maLbAxisPos.SelectEntryPos(0);
}

namespace {

void SetType(const ScColorScaleEntry* pEntry, ListBox& rLstBox)
{
    rLstBox.SelectEntryPos(pEntry->GetType());
}

void GetType(const ListBox& rLstBox, const Edit& rEd, ScColorScaleEntry* pEntry, SvNumberFormatter* pNumberFormatter )
{
    double nVal = 0;
    sal_uInt32 nIndex = 0;
    pEntry->SetType(static_cast<ScColorScaleEntryType>(rLstBox.GetSelectEntryPos()));
    switch(rLstBox.GetSelectEntryPos())
    {
        case COLORSCALE_AUTO:
        case COLORSCALE_MIN:
        case COLORSCALE_MAX:
            break;
        case COLORSCALE_PERCENTILE:
        case COLORSCALE_VALUE:
        case COLORSCALE_PERCENT:
            pNumberFormatter->IsNumberFormat( rEd.GetText(), nIndex, nVal );
            pEntry->SetValue(nVal);
            break;
        case 6:
            //TODO: moggi
            break;
    }
}

void SetValue( ScColorScaleEntry* pEntry, Edit& aEdit)
{
    if(pEntry->GetType() == COLORSCALE_FORMULA)
        aEdit.SetText(pEntry->GetFormula(formula::FormulaGrammar::GRAM_DEFAULT));
    else if(pEntry->GetType() != COLORSCALE_MIN && pEntry->GetType() != COLORSCALE_MAX)
        aEdit.SetText(rtl::OUString::valueOf(pEntry->GetValue()));
    else
        aEdit.Disable();
}

}

ScDataBarSettingsDlg::ScDataBarSettingsDlg(Window* pWindow, const ScDataBarFormatData& rData, ScDocument* pDoc):
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
    maEdMax( this, ScResId( ED_MAX ) ),
    maStrWarnSameValue( SC_RESSTR( STR_WARN_SAME_VALUE ) ),
    mpNumberFormatter( pDoc->GetFormatTable() )
{
    Init();
    FreeResource();

    maLbPos.SelectEntry( rData.maPositiveColor );
    if(rData.mpNegativeColor)
        maLbNeg.SelectEntry( *rData.mpNegativeColor );

    switch (rData.meAxisPosition)
    {
        case databar::NONE:
            maLbAxisPos.SelectEntryPos(2);
            break;
        case databar::AUTOMATIC:
            maLbAxisPos.SelectEntryPos(0);
            break;
        case databar::MIDDLE:
            maLbAxisPos.SelectEntryPos(1);
            break;
    }
    ::SetType(rData.mpLowerLimit.get(), maLbTypeMin);
    ::SetType(rData.mpUpperLimit.get(), maLbTypeMax);
    SetValue(rData.mpLowerLimit.get(), maEdMin);
    SetValue(rData.mpUpperLimit.get(), maEdMax);
    maLbAxisCol.SelectEntry(rData.maAxisColor);

    TypeSelectHdl(NULL);
}

void ScDataBarSettingsDlg::Init()
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
    maBtnOk.SetClickHdl( LINK( this, ScDataBarSettingsDlg, OkBtnHdl ) );

    Point aPoint(maLbTypeMax.GetPosPixel().X(), maFtMax.GetPosPixel().Y());
    maLbTypeMax.SetPosPixel(aPoint);

    maLbTypeMin.SetSelectHdl( LINK( this, ScDataBarSettingsDlg, TypeSelectHdl ) );
    maLbTypeMax.SetSelectHdl( LINK( this, ScDataBarSettingsDlg, TypeSelectHdl ) );

}

namespace {

void GetAxesPosition(ScDataBarFormatData* pData, const ListBox& rLbox)
{
    switch(rLbox.GetSelectEntryPos())
    {
        case 0:
            pData->meAxisPosition = databar::AUTOMATIC;
            break;
        case 1:
            pData->meAxisPosition = databar::MIDDLE;
            break;
        case 2:
            pData->meAxisPosition = databar::NONE;
            break;
    }
}

}

ScDataBarFormatData* ScDataBarSettingsDlg::GetData()
{
    ScDataBarFormatData* pData = new ScDataBarFormatData();
    pData->maPositiveColor = maLbPos.GetSelectEntryColor();
    pData->mpNegativeColor.reset(new Color(maLbNeg.GetSelectEntryColor()));
    pData->mbGradient = true; //FIXME
    pData->mpUpperLimit.reset(new ScColorScaleEntry());
    pData->mpLowerLimit.reset(new ScColorScaleEntry());
    pData->maAxisColor = maLbAxisCol.GetSelectEntryColor();

    ::GetType(maLbTypeMin, maEdMin, pData->mpLowerLimit.get(), mpNumberFormatter);
    ::GetType(maLbTypeMax, maEdMax, pData->mpUpperLimit.get(), mpNumberFormatter);
    GetAxesPosition(pData, maLbAxisPos);

    return pData;
}

IMPL_LINK_NOARG( ScDataBarSettingsDlg, OkBtnHdl )
{
    //check that min < max
    bool bWarn = false;
    sal_Int32 nSelectMin = maLbTypeMin.GetSelectEntryPos();
    if( nSelectMin == COLORSCALE_MAX )
        bWarn = true;
    sal_Int32 nSelectMax = maLbTypeMax.GetSelectEntryPos();
    if( nSelectMax == COLORSCALE_MIN )
        bWarn = true;

    if(!bWarn && maLbTypeMin.GetSelectEntryPos() == maLbTypeMax.GetSelectEntryPos())
    {
        if(maLbTypeMax.GetSelectEntryPos() != COLORSCALE_FORMULA)
        {
            rtl::OUString aMinString = maEdMin.GetText();
            rtl::OUString aMaxString = maEdMax.GetText();
            double nMinValue = 0;
            sal_uInt32 nIndex = 0;
            mpNumberFormatter->IsNumberFormat(aMinString, nIndex, nMinValue);
            nIndex = 0;
            double nMaxValue = 0;
            mpNumberFormatter->IsNumberFormat(aMaxString, nIndex, nMaxValue);
            if(rtl::math::approxEqual(nMinValue, nMaxValue) || nMinValue > nMaxValue)
                bWarn = true;
        }
    }

    if(bWarn)
    {
        //show warning message and don't close
        WarningBox aWarn(this, WB_OK, maStrWarnSameValue );
        aWarn.Execute();
    }
    else
    {
        EndDialog(RET_OK);
    }
    return 0;
}

IMPL_LINK_NOARG( ScDataBarSettingsDlg, TypeSelectHdl )
{
    sal_Int32 nSelectMin = maLbTypeMin.GetSelectEntryPos();
    if( nSelectMin <= COLORSCALE_MAX)
        maEdMin.Disable();
    else
    {
        maEdMin.Enable();
        if(!maEdMin.GetText().Len())
        {
            if(nSelectMin == COLORSCALE_PERCENTILE || nSelectMin == COLORSCALE_PERCENT)
                maEdMin.SetText(rtl::OUString::valueOf(static_cast<sal_Int32>(50)));
            else
                maEdMin.SetText(rtl::OUString::valueOf(static_cast<sal_Int32>(0)));
        }
    }

    sal_Int32 nSelectMax = maLbTypeMax.GetSelectEntryPos();
    if(nSelectMax <= COLORSCALE_MAX)
        maEdMax.Disable();
    else
    {
        maEdMax.Enable();
        if(!maEdMax.GetText().Len())
        {
            if(nSelectMax == COLORSCALE_PERCENTILE || nSelectMax == COLORSCALE_PERCENT)
                maEdMax.SetText(rtl::OUString::valueOf(static_cast<sal_Int32>(50)));
            else
                maEdMax.SetText(rtl::OUString::valueOf(static_cast<sal_Int32>(0)));
        }
    }
    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
