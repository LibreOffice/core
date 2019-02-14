/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <colorformat.hxx>
#include <colorscale.hxx>

#include <document.hxx>
#include <sc.hrc>

#include <svx/colorbox.hxx>
#include <svx/xtable.hxx>
#include <svx/drawitem.hxx>
#include <vcl/svapp.hxx>
#include <vcl/weld.hxx>

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
    pEntry->SetType(static_cast<ScColorScaleEntryType>(rLstBox.GetSelectedEntryPos()));
    switch(rLstBox.GetSelectedEntryPos())
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

OUString convertNumberToString(double nVal, const ScDocument* pDoc)
{
    SvNumberFormatter* pNumberFormatter = pDoc->GetFormatTable();
    OUString aText;
    pNumberFormatter->GetInputLineString(nVal, 0, aText);
    return aText;
}

void SetValue( const ScDocument* pDoc, const ScColorScaleEntry* pEntry, Edit& aEdit)
{
    if(pEntry->GetType() == COLORSCALE_FORMULA)
        aEdit.SetText(pEntry->GetFormula(formula::FormulaGrammar::GRAM_DEFAULT));
    else if(pEntry->GetType() != COLORSCALE_MIN && pEntry->GetType() != COLORSCALE_MAX)
        aEdit.SetText(convertNumberToString(pEntry->GetValue(), pDoc));
    else
        aEdit.Disable();
}

}

ScDataBarSettingsDlg::ScDataBarSettingsDlg(weld::Window* pParent, const ScDataBarFormatData& rData, ScDocument* pDoc, const ScAddress& rPos)
    : GenericDialogController(pParent, "modules/scalc/ui/databaroptions.ui", "DataBarOptions")
    , mpNumberFormatter(pDoc->GetFormatTable())
    , mpDoc(pDoc)
    , maPos(rPos)
    , mxBtnOk(m_xBuilder->weld_button("ok"))
    , mxBtnCancel(m_xBuilder->weld_button("cancel"))
    , mxLbPos(new ColorListBox(m_xBuilder->weld_menu_button("positive_colour"), pParent))
    , mxLbNeg(new ColorListBox(m_xBuilder->weld_menu_button("negative_colour"), pParent))
    , mxLbFillType(m_xBuilder->weld_combo_box("fill_type"))
    , mxLbTypeMin(m_xBuilder->weld_combo_box("min"))
    , mxLbTypeMax(m_xBuilder->weld_combo_box("max"))
    , mxLbAxisPos(m_xBuilder->weld_combo_box("axis_pos"))
    , mxLbAxisCol(new ColorListBox(m_xBuilder->weld_menu_button("axis_colour"), pParent))
    , mxEdMin(m_xBuilder->weld_entry("min_value"))
    , mxEdMax(m_xBuilder->weld_entry("max_value"))
    , mxLenMin(m_xBuilder->weld_entry("min_length"))
    , mxLenMax(m_xBuilder->weld_entry("max_length"))
    , mxCbOnlyBar(m_xBuilder->weld_check_button("only_bar"))
    , mxStrSameValueFT(m_xBuilder->weld_label("str_same_value"))
{
    maStrWarnSameValue = mxStrSameValueFT->get_label();

    Init();

    mpLbPos->SelectEntry(rData.maPositiveColor);
    mpLbFillType->SelectEntryPos( rData.mbGradient ? 1 : 0 );
    if (rData.mpNegativeColor)
        mpLbNeg->SelectEntry(*rData.mpNegativeColor);

    switch (rData.meAxisPosition)
    {
        case databar::NONE:
            mxLbAxisPos->SelectEntryPos(2);
            break;
        case databar::AUTOMATIC:
            mxLbAxisPos->SelectEntryPos(0);
            break;
        case databar::MIDDLE:
            mxLbAxisPos->SelectEntryPos(1);
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

    TypeSelectHdl(*mpLbTypeMin);
    PosSelectHdl(*mpLbTypeMin);
}

ScDataBarSettingsDlg::~ScDataBarSettingsDlg()
{
}

void ScDataBarSettingsDlg::Init()
{
    mxLbNeg->SelectEntry(COL_LIGHTRED);
    mxLbAxisCol->SelectEntry(COL_BLACK);
    mxLbPos->SelectEntry(COL_LIGHTBLUE);
    mxBtnOk->connect_clicked( LINK( this, ScDataBarSettingsDlg, OkBtnHdl ) );

    mxLbTypeMin->SetSelectHdl( LINK( this, ScDataBarSettingsDlg, TypeSelectHdl ) );
    mxLbTypeMax->SetSelectHdl( LINK( this, ScDataBarSettingsDlg, TypeSelectHdl ) );
    mxLbAxisPos->SetSelectHdl( LINK( this, ScDataBarSettingsDlg, PosSelectHdl ) );

}

namespace {

void GetAxesPosition(ScDataBarFormatData* pData, const ListBox* rLbox)
{
    switch(rLbox->GetSelectedEntryPos())
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
    pData->mbGradient = ( mpLbFillType->GetSelectedEntryPos() == 1 );
    pData->mpUpperLimit.reset(new ScColorScaleEntry());
    pData->mpLowerLimit.reset(new ScColorScaleEntry());
    pData->maAxisColor = mpLbAxisCol->GetSelectEntryColor();
    pData->mbOnlyBar = mpCbOnlyBar->IsChecked();

    ::GetType(*mpLbTypeMin, *mpEdMin, pData->mpLowerLimit.get(), mpNumberFormatter, mpDoc, maPos);
    ::GetType(*mpLbTypeMax, *mpEdMax, pData->mpUpperLimit.get(), mpNumberFormatter, mpDoc, maPos);
    GetAxesPosition(pData, mxLbAxisPos);
    SetBarLength(pData, mpLenMin->GetText(), mpLenMax->GetText(), mpNumberFormatter);

    return pData;
}

IMPL_LINK_NOARG( ScDataBarSettingsDlg, OkBtnHdl, Button*, void )
{
    //check that min < max
    bool bWarn = false;
    sal_Int32 nSelectMin = mpLbTypeMin->GetSelectedEntryPos();
    if( nSelectMin == COLORSCALE_MAX )
        bWarn = true;
    sal_Int32 nSelectMax = mpLbTypeMax->GetSelectedEntryPos();
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
    if(!bWarn && mpLbTypeMin->GetSelectedEntryPos() == mpLbTypeMax->GetSelectedEntryPos())
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
        std::unique_ptr<weld::MessageDialog> xWarn(Application::CreateMessageDialog(GetFrameWeld(),
                                                   VclMessageType::Warning, VclButtonsType::Ok,
                                                   maStrWarnSameValue));
        xWarn->run();
    }
    else
    {
        EndDialog(RET_OK);
    }
}

IMPL_LINK_NOARG( ScDataBarSettingsDlg, TypeSelectHdl, ListBox&, void )
{
    sal_Int32 nSelectMin = mpLbTypeMin->GetSelectedEntryPos();
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

    sal_Int32 nSelectMax = mpLbTypeMax->GetSelectedEntryPos();
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
}

IMPL_LINK_NOARG( ScDataBarSettingsDlg, PosSelectHdl, ListBox&, void )
{
    sal_Int32 axisPos = mxLbAxisPos->GetSelectedEntryPos();
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
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

