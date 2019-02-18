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

#include <svx/colorbox.hxx>
#include <vcl/svapp.hxx>
#include <vcl/weld.hxx>

namespace {

void SetType(const ScColorScaleEntry* pEntry, weld::ComboBox& rLstBox)
{
    rLstBox.set_active(pEntry->GetType());
}

void GetType(const weld::ComboBox& rLstBox, const weld::Entry& rEd, ScColorScaleEntry* pEntry, SvNumberFormatter* pNumberFormatter,
        ScDocument* pDoc, const ScAddress& rPos )
{
    double nVal = 0;
    sal_uInt32 nIndex = 0;
    pEntry->SetType(static_cast<ScColorScaleEntryType>(rLstBox.get_active()));
    switch (rLstBox.get_active())
    {
        case COLORSCALE_AUTO:
        case COLORSCALE_MIN:
        case COLORSCALE_MAX:
            break;
        case COLORSCALE_PERCENTILE:
        case COLORSCALE_VALUE:
        case COLORSCALE_PERCENT:
            (void)pNumberFormatter->IsNumberFormat( rEd.get_text(), nIndex, nVal );
            pEntry->SetValue(nVal);
            break;
        case COLORSCALE_FORMULA:
            pEntry->SetFormula(rEd.get_text(), pDoc, rPos);
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

void SetValue( const ScDocument* pDoc, const ScColorScaleEntry* pEntry, weld::Entry& rEdit)
{
    if(pEntry->GetType() == COLORSCALE_FORMULA)
        rEdit.set_text(pEntry->GetFormula(formula::FormulaGrammar::GRAM_DEFAULT));
    else if(pEntry->GetType() != COLORSCALE_MIN && pEntry->GetType() != COLORSCALE_MAX)
        rEdit.set_text(convertNumberToString(pEntry->GetValue(), pDoc));
    else
        rEdit.set_sensitive(false);
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
    , mxLbAxisCol(new ColorListBox(m_xBuilder->weld_menu_button("axis_colour"), pParent))
    , mxLbFillType(m_xBuilder->weld_combo_box("fill_type"))
    , mxLbTypeMin(m_xBuilder->weld_combo_box("min"))
    , mxLbTypeMax(m_xBuilder->weld_combo_box("max"))
    , mxLbAxisPos(m_xBuilder->weld_combo_box("axis_pos"))
    , mxEdMin(m_xBuilder->weld_entry("min_value"))
    , mxEdMax(m_xBuilder->weld_entry("max_value"))
    , mxLenMin(m_xBuilder->weld_entry("min_length"))
    , mxLenMax(m_xBuilder->weld_entry("max_length"))
    , mxCbOnlyBar(m_xBuilder->weld_check_button("only_bar"))
    , mxStrSameValueFT(m_xBuilder->weld_label("str_same_value"))
{
    maStrWarnSameValue = mxStrSameValueFT->get_label();

    Init();

    mxLbPos->SelectEntry(rData.maPositiveColor);
    mxLbFillType->set_active( rData.mbGradient ? 1 : 0 );
    if (rData.mpNegativeColor)
        mxLbNeg->SelectEntry(*rData.mpNegativeColor);

    switch (rData.meAxisPosition)
    {
        case databar::NONE:
            mxLbAxisPos->set_active(2);
            break;
        case databar::AUTOMATIC:
            mxLbAxisPos->set_active(0);
            break;
        case databar::MIDDLE:
            mxLbAxisPos->set_active(1);
            break;
    }
    ::SetType(rData.mpLowerLimit.get(), *mxLbTypeMin);
    ::SetType(rData.mpUpperLimit.get(), *mxLbTypeMax);
    SetValue(mpDoc, rData.mpLowerLimit.get(), *mxEdMin);
    SetValue(mpDoc, rData.mpUpperLimit.get(), *mxEdMax);
    mxLenMin->set_text(convertNumberToString(rData.mnMinLength, mpDoc));
    mxLenMax->set_text(convertNumberToString(rData.mnMaxLength, mpDoc));
    mxLbAxisCol->SelectEntry(rData.maAxisColor);
    mxCbOnlyBar->set_active(rData.mbOnlyBar);

    TypeSelectHdl(*mxLbTypeMin);
    PosSelectHdl(*mxLbTypeMin);
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

    mxLbTypeMin->connect_changed( LINK( this, ScDataBarSettingsDlg, TypeSelectHdl ) );
    mxLbTypeMax->connect_changed( LINK( this, ScDataBarSettingsDlg, TypeSelectHdl ) );
    mxLbAxisPos->connect_changed( LINK( this, ScDataBarSettingsDlg, PosSelectHdl ) );

}

namespace {

void GetAxesPosition(ScDataBarFormatData* pData, const weld::ComboBox& rLbox)
{
    switch (rLbox.get_active())
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
    pData->maPositiveColor = mxLbPos->GetSelectEntryColor();
    pData->mpNegativeColor.reset(new Color(mxLbNeg->GetSelectEntryColor()));
    pData->mbGradient = ( mxLbFillType->get_active() == 1 );
    pData->mpUpperLimit.reset(new ScColorScaleEntry());
    pData->mpLowerLimit.reset(new ScColorScaleEntry());
    pData->maAxisColor = mxLbAxisCol->GetSelectEntryColor();
    pData->mbOnlyBar = mxCbOnlyBar->get_active();

    ::GetType(*mxLbTypeMin, *mxEdMin, pData->mpLowerLimit.get(), mpNumberFormatter, mpDoc, maPos);
    ::GetType(*mxLbTypeMax, *mxEdMax, pData->mpUpperLimit.get(), mpNumberFormatter, mpDoc, maPos);
    GetAxesPosition(pData, *mxLbAxisPos);
    SetBarLength(pData, mxLenMin->get_text(), mxLenMax->get_text(), mpNumberFormatter);

    return pData;
}

IMPL_LINK_NOARG(ScDataBarSettingsDlg, OkBtnHdl, weld::Button&, void)
{
    //check that min < max
    bool bWarn = false;
    int nSelectMin = mxLbTypeMin->get_active();
    if( nSelectMin == COLORSCALE_MAX )
        bWarn = true;
    int nSelectMax = mxLbTypeMax->get_active();
    if( nSelectMax == COLORSCALE_MIN )
        bWarn = true;
    if(!bWarn) // databar length checks
    {
        OUString aMinString = mxLenMin->get_text();
        OUString aMaxString = mxLenMax->get_text();
        double nMinValue = 0;
        sal_uInt32 nIndex = 0;
        (void)mpNumberFormatter->IsNumberFormat(aMinString, nIndex, nMinValue);
        nIndex = 0;
        double nMaxValue = 0;
        (void)mpNumberFormatter->IsNumberFormat(aMaxString, nIndex, nMaxValue);
        if(rtl::math::approxEqual(nMinValue, nMaxValue) || nMinValue > nMaxValue || nMaxValue > 100 || nMinValue < 0)
            bWarn = true;
    }
    if (!bWarn && mxLbTypeMin->get_active() == mxLbTypeMax->get_active())
    {

        if(nSelectMax != COLORSCALE_FORMULA && nSelectMax != COLORSCALE_AUTO)
        {
            OUString aMinString = mxEdMin->get_text();
            OUString aMaxString = mxEdMax->get_text();
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
        std::unique_ptr<weld::MessageDialog> xWarn(Application::CreateMessageDialog(m_xDialog.get(),
                                                   VclMessageType::Warning, VclButtonsType::Ok,
                                                   maStrWarnSameValue));
        xWarn->run();
    }
    else
    {
        m_xDialog->response(RET_OK);
    }
}

IMPL_LINK_NOARG(ScDataBarSettingsDlg, TypeSelectHdl, weld::ComboBox&, void)
{
    int nSelectMin = mxLbTypeMin->get_active();
    if( nSelectMin <= COLORSCALE_MAX)
        mxEdMin->set_sensitive(false);
    else
    {
        mxEdMin->set_sensitive(true);
        if(mxEdMin->get_text().isEmpty())
        {
            if(nSelectMin == COLORSCALE_PERCENTILE || nSelectMin == COLORSCALE_PERCENT)
                mxEdMin->set_text(OUString::number(50));
            else
                mxEdMin->set_text(OUString::number(0));
        }
    }

    int nSelectMax = mxLbTypeMax->get_active();
    if (nSelectMax <= COLORSCALE_MAX)
        mxEdMax->set_sensitive(false);
    else
    {
        mxEdMax->set_sensitive(true);
        if (mxEdMax->get_text().isEmpty())
        {
            if(nSelectMax == COLORSCALE_PERCENTILE || nSelectMax == COLORSCALE_PERCENT)
                mxEdMax->set_text(OUString::number(50));
            else
                mxEdMax->set_text(OUString::number(0));
        }
    }
}

IMPL_LINK_NOARG(ScDataBarSettingsDlg, PosSelectHdl, weld::ComboBox&, void)
{
    int axisPos = mxLbAxisPos->get_active();
    if(axisPos != 2 && axisPos != 1) // disable if axis vertical position is automatic
    {
        mxLenMin->set_sensitive(false);
        mxLenMax->set_sensitive(false);
    }
    else
    {
        mxLenMin->set_sensitive(true);
        mxLenMax->set_sensitive(true);
        if(mxLenMin->get_text().isEmpty())
        {
            mxLenMin->set_text(OUString::number(0));
            mxLenMax->set_text(OUString::number(100));
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

