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
            (void)pNumberFormatter->IsNumberFormat( rEd.GetText(), nIndex, nVal );
            pEntry->SetValue(nVal);
            break;
        case COLORSCALE_FORMULA:
            pEntry->SetFormula(rEd.GetText(), pDoc, rPos);
            break;
    }
}

OUString convertNumberToString(double nVal, ScDocument* pDoc)
{
    SvNumberFormatter* pNumberFormatter = pDoc->GetFormatTable();
    OUString aText;
    pNumberFormatter->GetInputLineString(nVal, 0, aText);
    return aText;
}

void SetValue( ScDocument* pDoc, ScColorScaleEntry* pEntry, Edit& aEdit)
{
    if(pEntry->GetType() == COLORSCALE_FORMULA)
        aEdit.SetText(pEntry->GetFormula(formula::FormulaGrammar::GRAM_DEFAULT));
    else if(pEntry->GetType() != COLORSCALE_MIN && pEntry->GetType() != COLORSCALE_MAX)
        aEdit.SetText(convertNumberToString(pEntry->GetValue(), pDoc));
    else
        aEdit.Disable();
}

void SelectColor(const Color& aColor, const OUString & aCustomName, ColorListBox& rLstBox)
{
    rLstBox.SelectEntry( aColor );
    if ( rLstBox.GetSelectEntryColor() != aColor )
    {
        rLstBox.InsertEntry( aColor, aCustomName );
        rLstBox.SelectEntry( aColor );
    }
}

}

ScDataBarSettingsDlg::ScDataBarSettingsDlg(vcl::Window* pWindow, const ScDataBarFormatData& rData, ScDocument* pDoc, const ScAddress& rPos):
    ModalDialog( pWindow, "DataBarOptions", "modules/scalc/ui/databaroptions.ui" ),
    mpNumberFormatter( pDoc->GetFormatTable() ),
    mpDoc(pDoc),
    maPos(rPos)
{
    get( mpBtnOk, "ok");
    get( mpBtnCancel, "cancel" );
    get( mpLbPos, "positive_colour" );
    get( mpLbNeg, "negative_colour" );
    get( mpLbFillType, "fill_type" );
    get( mpLbTypeMin, "min" );
    get( mpLbTypeMax, "max" );
    get( mpLbAxisPos, "axis_pos" );
    get( mpLbAxisCol, "axis_colour" );
    get( mpEdMin, "min_value" );
    get( mpEdMax, "max_value" );
    get( mpLenMin, "min_length" );
    get( mpLenMax, "max_length" );
    get( mpCbOnlyBar, "only_bar");

    maStrWarnSameValue = get<FixedText>("str_same_value")->GetText();
    maCustomColor = get<FixedText>("custom_color")->GetText();

    Init();

    ::SelectColor( rData.maPositiveColor, maCustomColor, *mpLbPos);
    mpLbFillType->SelectEntryPos( rData.mbGradient ? 1 : 0 );
    if(rData.mpNegativeColor)
        ::SelectColor( *rData.mpNegativeColor, maCustomColor, *mpLbNeg );

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
    SetValue(mpDoc, rData.mpLowerLimit.get(), *mpEdMin);
    SetValue(mpDoc, rData.mpUpperLimit.get(), *mpEdMax);
    mpLenMin->SetText(convertNumberToString(rData.mnMinLength, mpDoc));
    mpLenMax->SetText(convertNumberToString(rData.mnMaxLength, mpDoc));
    mpLbAxisCol->SelectEntry(rData.maAxisColor);
    mpCbOnlyBar->Check(rData.mbOnlyBar);

    TypeSelectHdl(NULL);
    PosSelectHdl(NULL);
}

ScDataBarSettingsDlg::~ScDataBarSettingsDlg()
{
    disposeOnce();
}

void ScDataBarSettingsDlg::dispose()
{
    mpBtnOk.clear();
    mpBtnCancel.clear();
    mpLbPos.clear();
    mpLbNeg.clear();
    mpLbAxisCol.clear();
    mpLbTypeMin.clear();
    mpLbTypeMax.clear();
    mpLbFillType.clear();
    mpLbAxisPos.clear();
    mpEdMin.clear();
    mpEdMax.clear();
    mpLenMin.clear();
    mpLenMax.clear();
    mpCbOnlyBar.clear();
    ModalDialog::dispose();
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
            pColorTable = static_cast<const SvxColorListItem*>(pItem)->GetColorList();
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
    mpLbAxisPos->SetSelectHdl( LINK( this, ScDataBarSettingsDlg, PosSelectHdl ) );

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

void SetBarLength(ScDataBarFormatData* pData, const OUString& minStr, const OUString& maxStr, SvNumberFormatter* mpNumberFormatter)
{
    double nMinValue = 0;
    sal_uInt32 nIndex = 0;
    (void)mpNumberFormatter->IsNumberFormat(minStr, nIndex, nMinValue);
    nIndex = 0;
    double nMaxValue = 0;
    (void)mpNumberFormatter->IsNumberFormat(maxStr, nIndex, nMaxValue);
    pData->mnMinLength = nMinValue;
    pData->mnMaxLength = nMaxValue;
}

}

ScDataBarFormatData* ScDataBarSettingsDlg::GetData()
{
    ScDataBarFormatData* pData = new ScDataBarFormatData();
    pData->maPositiveColor = mpLbPos->GetSelectEntryColor();
    pData->mpNegativeColor.reset(new Color(mpLbNeg->GetSelectEntryColor()));
    pData->mbGradient = ( mpLbFillType->GetSelectEntryPos() == 1 );
    pData->mpUpperLimit.reset(new ScColorScaleEntry());
    pData->mpLowerLimit.reset(new ScColorScaleEntry());
    pData->maAxisColor = mpLbAxisCol->GetSelectEntryColor();
    pData->mbOnlyBar = mpCbOnlyBar->IsChecked();

    ::GetType(*mpLbTypeMin, *mpEdMin, pData->mpLowerLimit.get(), mpNumberFormatter, mpDoc, maPos);
    ::GetType(*mpLbTypeMax, *mpEdMax, pData->mpUpperLimit.get(), mpNumberFormatter, mpDoc, maPos);
    GetAxesPosition(pData, mpLbAxisPos);
    SetBarLength(pData, mpLenMin->GetText(), mpLenMax->GetText(), mpNumberFormatter);

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
    if(!bWarn) // databar length checks
    {
        OUString aMinString = mpLenMin->GetText();
        OUString aMaxString = mpLenMax->GetText();
        double nMinValue = 0;
        sal_uInt32 nIndex = 0;
        (void)mpNumberFormatter->IsNumberFormat(aMinString, nIndex, nMinValue);
        nIndex = 0;
        double nMaxValue = 0;
        (void)mpNumberFormatter->IsNumberFormat(aMaxString, nIndex, nMaxValue);
        if(rtl::math::approxEqual(nMinValue, nMaxValue) || nMinValue > nMaxValue || nMaxValue > 100 || nMinValue < 0)
            bWarn = true;
    }
    if(!bWarn && mpLbTypeMin->GetSelectEntryPos() == mpLbTypeMax->GetSelectEntryPos())
    {

        if(nSelectMax != COLORSCALE_FORMULA && nSelectMax != COLORSCALE_AUTO)
        {
            OUString aMinString = mpEdMin->GetText();
            OUString aMaxString = mpEdMax->GetText();
            double nMinValue = 0;
            sal_uInt32 nIndex = 0;
            (void)mpNumberFormatter->IsNumberFormat(aMinString, nIndex, nMinValue);
            nIndex = 0;
            double nMaxValue = 0;
            (void)mpNumberFormatter->IsNumberFormat(aMaxString, nIndex, nMaxValue);
            if(rtl::math::approxEqual(nMinValue, nMaxValue) || nMinValue > nMaxValue)
                bWarn = true;
        }
    }

    if(bWarn)
    {
        //show warning message and don't close
        ScopedVclPtrInstance< WarningBox > aWarn(this, WB_OK, maStrWarnSameValue );
        aWarn->Execute();
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

IMPL_LINK_NOARG( ScDataBarSettingsDlg, PosSelectHdl )
{
    sal_Int32 axisPos = mpLbAxisPos->GetSelectEntryPos();
    if(axisPos != 2 && axisPos != 1) // disable if axis vertical position is automatic
    {
        mpLenMin->Disable();
        mpLenMax->Disable();
    }
    else
    {
        mpLenMin->Enable();
        mpLenMax->Enable();
        if(mpLenMin->GetText().isEmpty())
        {
            mpLenMin->SetText(OUString::number(0));
            mpLenMax->SetText(OUString::number(100));
        }
    }
    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

