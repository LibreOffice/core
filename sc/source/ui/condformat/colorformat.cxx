/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "colorformat.hxx"
#include "colorscale.hxx"

#include "document.hxx"
#include "sc.hrc"

#include <svx/xtable.hxx>
#include <svx/drawitem.hxx>
#include <vcl/msgbox.hxx>

namespace {

void SetType(const ScColorScaleEntry* pEntry, ListBox& rLstBox)
{
    rLstBox.SelectEntryPos(pEntry->GetType());
}

void GetType(const ListBox& rLstBox, const Edit& rEd, ScColorScaleEntry* pEntry, SvNumberFormatter* pNumberFormatter,
        ScDocument* pDoc, const ScAddress& rPos )
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
        case COLORSCALE_FORMULA:
            pEntry->SetFormula(rEd.GetText(), pDoc, rPos);
            break;
    }
}

void SetValue( ScColorScaleEntry* pEntry, Edit& aEdit)
{
    if(pEntry->GetType() == COLORSCALE_FORMULA)
        aEdit.SetText(pEntry->GetFormula(formula::FormulaGrammar::GRAM_DEFAULT));
    else if(pEntry->GetType() != COLORSCALE_MIN && pEntry->GetType() != COLORSCALE_MAX)
        aEdit.SetText(OUString::number(pEntry->GetValue()));
    else
        aEdit.Disable();
}

}

ScDataBarSettingsDlg::ScDataBarSettingsDlg(Window* pWindow, const ScDataBarFormatData& rData, ScDocument* pDoc, const ScAddress& rPos):
    ModalDialog( pWindow, "DataBarOptions", "modules/scalc/ui/databaroptions.ui" ),
    mpNumberFormatter( pDoc->GetFormatTable() ),
    mpDoc(pDoc),
    maPos(rPos)
{
    get( mpBtnOk, "ok");
    get( mpBtnCancel, "cancel" );
    get( mpLbPos, "positive_colour" );
    get( mpLbNeg, "negative_colour" );
    get( mpLbTypeMin, "min" );
    get( mpLbTypeMax, "max" );
    get( mpLbAxisPos, "axis_pos" );
    get( mpLbAxisCol, "axis_colour" );
    get( mpEdMin, "min_value" );
    get( mpEdMax, "max_value" );

    maStrWarnSameValue = get<FixedText>("str_same_value")->GetText();

    Init();

    mpLbPos->SelectEntry( rData.maPositiveColor );
    if(rData.mpNegativeColor)
        mpLbNeg->SelectEntry( *rData.mpNegativeColor );

    switch (rData.meAxisPosition)
    {
        case databar::NONE:
            mpLbAxisPos->SelectEntryPos(2);
            break;
        case databar::AUTOMATIC:
            mpLbAxisPos->SelectEntryPos(0);
            break;
        case databar::MIDDLE:
            mpLbAxisPos->SelectEntryPos(1);
            break;
    }
    ::SetType(rData.mpLowerLimit.get(), *mpLbTypeMin);
    ::SetType(rData.mpUpperLimit.get(), *mpLbTypeMax);
    SetValue(rData.mpLowerLimit.get(), *mpEdMin);
    SetValue(rData.mpUpperLimit.get(), *mpEdMax);
    mpLbAxisCol->SelectEntry(rData.maAxisColor);

    TypeSelectHdl(NULL);
}

void ScDataBarSettingsDlg::Init()
{
    SfxObjectShell*     pDocSh      = SfxObjectShell::Current();
    XColorListRef       pColorTable;

    DBG_ASSERT( pDocSh, "DocShell not found!" );

    if ( pDocSh )
    {
        const SfxPoolItem*  pItem = pDocSh->GetItem( SID_COLOR_TABLE );
        if ( pItem != NULL )
            pColorTable = ( (SvxColorListItem*)pItem )->GetColorList();
    }
    if ( pColorTable.is() )
    {
        // filling the line color box
        mpLbPos->SetUpdateMode( false );
        mpLbNeg->SetUpdateMode( false );
        mpLbAxisCol->SetUpdateMode( false );

        for ( long i = 0; i < pColorTable->Count(); ++i )
        {
            XColorEntry* pEntry = pColorTable->GetColor(i);
            mpLbPos->InsertEntry( pEntry->GetColor(), pEntry->GetName() );
            mpLbNeg->InsertEntry( pEntry->GetColor(), pEntry->GetName() );
            mpLbAxisCol->InsertEntry( pEntry->GetColor(), pEntry->GetName() );

            if(pEntry->GetColor() == Color(COL_LIGHTRED))
                mpLbNeg->SelectEntryPos(i);
            if(pEntry->GetColor() == Color(COL_BLACK))
                mpLbAxisCol->SelectEntryPos(i);
            if(pEntry->GetColor() == Color(COL_LIGHTBLUE))
                mpLbPos->SelectEntryPos(i);
        }
        mpLbPos->SetUpdateMode( true );
        mpLbNeg->SetUpdateMode( true );
        mpLbAxisCol->SetUpdateMode( true );
    }
    mpBtnOk->SetClickHdl( LINK( this, ScDataBarSettingsDlg, OkBtnHdl ) );

    mpLbTypeMin->SetSelectHdl( LINK( this, ScDataBarSettingsDlg, TypeSelectHdl ) );
    mpLbTypeMax->SetSelectHdl( LINK( this, ScDataBarSettingsDlg, TypeSelectHdl ) );

}

namespace {

void GetAxesPosition(ScDataBarFormatData* pData, const ListBox* rLbox)
{
    switch(rLbox->GetSelectEntryPos())
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
    pData->maPositiveColor = mpLbPos->GetSelectEntryColor();
    pData->mpNegativeColor.reset(new Color(mpLbNeg->GetSelectEntryColor()));
    pData->mbGradient = true; //FIXME
    pData->mpUpperLimit.reset(new ScColorScaleEntry());
    pData->mpLowerLimit.reset(new ScColorScaleEntry());
    pData->maAxisColor = mpLbAxisCol->GetSelectEntryColor();

    ::GetType(*mpLbTypeMin, *mpEdMin, pData->mpLowerLimit.get(), mpNumberFormatter, mpDoc, maPos);
    ::GetType(*mpLbTypeMax, *mpEdMax, pData->mpUpperLimit.get(), mpNumberFormatter, mpDoc, maPos);
    GetAxesPosition(pData, mpLbAxisPos);

    return pData;
}

IMPL_LINK_NOARG( ScDataBarSettingsDlg, OkBtnHdl )
{
    //check that min < max
    bool bWarn = false;
    sal_Int32 nSelectMin = mpLbTypeMin->GetSelectEntryPos();
    if( nSelectMin == COLORSCALE_MAX )
        bWarn = true;
    sal_Int32 nSelectMax = mpLbTypeMax->GetSelectEntryPos();
    if( nSelectMax == COLORSCALE_MIN )
        bWarn = true;

    if(!bWarn && mpLbTypeMin->GetSelectEntryPos() == mpLbTypeMax->GetSelectEntryPos())
    {

        if(nSelectMax != COLORSCALE_FORMULA && nSelectMax != COLORSCALE_AUTO)
        {
            OUString aMinString = mpEdMin->GetText();
            OUString aMaxString = mpEdMax->GetText();
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
    sal_Int32 nSelectMin = mpLbTypeMin->GetSelectEntryPos();
    if( nSelectMin <= COLORSCALE_MAX)
        mpEdMin->Disable();
    else
    {
        mpEdMin->Enable();
        if(mpEdMin->GetText().isEmpty())
        {
            if(nSelectMin == COLORSCALE_PERCENTILE || nSelectMin == COLORSCALE_PERCENT)
                mpEdMin->SetText(OUString::number(50));
            else
                mpEdMin->SetText(OUString::number(0));
        }
    }

    sal_Int32 nSelectMax = mpLbTypeMax->GetSelectEntryPos();
    if(nSelectMax <= COLORSCALE_MAX)
        mpEdMax->Disable();
    else
    {
        mpEdMax->Enable();
        if(mpEdMax->GetText().isEmpty())
        {
            if(nSelectMax == COLORSCALE_PERCENTILE || nSelectMax == COLORSCALE_PERCENT)
                mpEdMax->SetText(OUString::number(50));
            else
                mpEdMax->SetText(OUString::number(0));
        }
    }
    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
