/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifdef SC_DLLIMPLEMENTATION
#undef SC_DLLIMPLEMENTATION
#endif

#include <dpgroupdlg.hxx>
#include <globstr.hrc>
#include <scresid.hxx>
#include <editfield.hxx>

#include <com/sun/star/sheet/DataPilotFieldGroupBy.hpp>
#include <svtools/ctrlbox.hxx>

namespace {

/** Date part flags in order of the list box entries. */
static const sal_Int32 spnDateParts[] =
{
    css::sheet::DataPilotFieldGroupBy::SECONDS,
    css::sheet::DataPilotFieldGroupBy::MINUTES,
    css::sheet::DataPilotFieldGroupBy::HOURS,
    css::sheet::DataPilotFieldGroupBy::DAYS,
    css::sheet::DataPilotFieldGroupBy::MONTHS,
    css::sheet::DataPilotFieldGroupBy::QUARTERS,
    css::sheet::DataPilotFieldGroupBy::YEARS
};

static const char* aDatePartResIds[] =
{
    STR_DPFIELD_GROUP_BY_SECONDS,
    STR_DPFIELD_GROUP_BY_MINUTES,
    STR_DPFIELD_GROUP_BY_HOURS,
    STR_DPFIELD_GROUP_BY_DAYS,
    STR_DPFIELD_GROUP_BY_MONTHS,
    STR_DPFIELD_GROUP_BY_QUARTERS,
    STR_DPFIELD_GROUP_BY_YEARS
};

} // namespace

ScDPGroupEditHelper::ScDPGroupEditHelper(weld::RadioButton& rRbAuto, weld::RadioButton& rRbMan, weld::Widget& rEdValue)
    : mrRbAuto(rRbAuto)
    , mrRbMan(rRbMan)
    , mrEdValue(rEdValue)
{
    mrRbAuto.connect_clicked( LINK( this, ScDPGroupEditHelper, ClickHdl ) );
    mrRbMan.connect_clicked( LINK( this, ScDPGroupEditHelper, ClickHdl ) );
}

bool ScDPGroupEditHelper::IsAuto() const
{
    return mrRbAuto.get_active();
}

double ScDPGroupEditHelper::GetValue() const
{
    double fValue;
    if( !ImplGetValue( fValue ) )
        fValue = 0.0;
    return fValue;
}

void ScDPGroupEditHelper::SetValue( bool bAuto, double fValue )
{
    if( bAuto )
    {
        mrRbAuto.set_active(true);
        ClickHdl(mrRbAuto);
    }
    else
    {
        mrRbMan.set_active(true);
        ClickHdl(mrRbMan);
    }
    ImplSetValue( fValue );
}

IMPL_LINK(ScDPGroupEditHelper, ClickHdl, weld::Button&, rButton, void)
{
    if (&rButton == &mrRbAuto)
    {
        // disable edit field on clicking "automatic" radio button
        mrEdValue.set_sensitive(false);
    }
    else if (&rButton == &mrRbMan)
    {
        // enable and set focus to edit field on clicking "manual" radio button
        mrEdValue.set_sensitive(true);
        mrEdValue.grab_focus();
    }
}

ScDPNumGroupEditHelper::ScDPNumGroupEditHelper(weld::RadioButton& rRbAuto,
    weld::RadioButton& rRbMan, DoubleField& rEdValue)
    : ScDPGroupEditHelper(rRbAuto, rRbMan, rEdValue.get_widget())
    , mrEdValue(rEdValue)
{
}

bool ScDPNumGroupEditHelper::ImplGetValue( double& rfValue ) const
{
    return mrEdValue.GetValue(rfValue);
}

void ScDPNumGroupEditHelper::ImplSetValue( double fValue )
{
    mrEdValue.SetValue(fValue);
}

ScDPDateGroupEditHelper::ScDPDateGroupEditHelper(weld::RadioButton& rRbAuto, weld::RadioButton& rRbMan,
                                                 SvtCalendarBox& rEdValue, const Date& rNullDate)
    : ScDPGroupEditHelper(rRbAuto, rRbMan, rEdValue.get_button())
    , mrEdValue(rEdValue)
    , maNullDate(rNullDate)
{
}

bool ScDPDateGroupEditHelper::ImplGetValue( double& rfValue ) const
{
    rfValue = mrEdValue.get_date() - maNullDate;
    return true;
}

void ScDPDateGroupEditHelper::ImplSetValue( double fValue )
{
    Date aDate( maNullDate );
    aDate.AddDays( fValue );
    mrEdValue.set_date( aDate );
}

ScDPNumGroupDlg::ScDPNumGroupDlg(weld::Window* pParent, const ScDPNumGroupInfo& rInfo)
    : GenericDialogController(pParent, "modules/scalc/ui/groupbynumber.ui", "PivotTableGroupByNumber")
    , mxRbAutoStart(m_xBuilder->weld_radio_button("auto_start"))
    , mxRbManStart(m_xBuilder->weld_radio_button("manual_start"))
    , mxEdStart(new DoubleField(m_xBuilder->weld_entry("edit_start")))
    , mxRbAutoEnd(m_xBuilder->weld_radio_button("auto_end"))
    , mxRbManEnd(m_xBuilder->weld_radio_button("manual_end"))
    , mxEdEnd(new DoubleField(m_xBuilder->weld_entry("edit_end")))
    , mxEdBy(new DoubleField(m_xBuilder->weld_entry("edit_by")))
    , maStartHelper(*mxRbAutoStart, *mxRbManStart, *mxEdStart)
    , maEndHelper(*mxRbAutoEnd, *mxRbManEnd, *mxEdEnd)
{
    maStartHelper.SetValue( rInfo.mbAutoStart, rInfo.mfStart );
    maEndHelper.SetValue( rInfo.mbAutoEnd, rInfo.mfEnd );
    mxEdBy->SetValue( (rInfo.mfStep <= 0.0) ? 1.0 : rInfo.mfStep );

    /*  Set the initial focus, currently it is somewhere after calling all the radio
        button click handlers. Now the first enabled editable control is focused. */
    if (mxEdStart->get_sensitive())
        mxEdStart->grab_focus();
    else if (mxEdEnd->get_sensitive())
        mxEdEnd->grab_focus();
    else
        mxEdBy->grab_focus();
}

ScDPNumGroupDlg::~ScDPNumGroupDlg()
{
}

ScDPNumGroupInfo ScDPNumGroupDlg::GetGroupInfo() const
{
    ScDPNumGroupInfo aInfo;
    aInfo.mbEnable = true;
    aInfo.mbDateValues = false;
    aInfo.mbAutoStart = maStartHelper.IsAuto();
    aInfo.mbAutoEnd = maEndHelper.IsAuto();

    // get values and silently auto-correct them, if they are not valid
    // TODO: error messages in OK event?
    aInfo.mfStart = maStartHelper.GetValue();
    aInfo.mfEnd = maEndHelper.GetValue();
    if( !mxEdBy->GetValue( aInfo.mfStep ) || (aInfo.mfStep <= 0.0) )
        aInfo.mfStep = 1.0;
    if( aInfo.mfEnd <= aInfo.mfStart )
        aInfo.mfEnd = aInfo.mfStart + aInfo.mfStep;

    return aInfo;
}

ScDPDateGroupDlg::ScDPDateGroupDlg(weld::Window* pParent,
        const ScDPNumGroupInfo& rInfo, sal_Int32 nDatePart, const Date& rNullDate)
    : GenericDialogController(pParent, "modules/scalc/ui/groupbydate.ui", "PivotTableGroupByDate")
    , mxRbAutoStart(m_xBuilder->weld_radio_button("auto_start"))
    , mxRbManStart(m_xBuilder->weld_radio_button("manual_start"))
    , mxEdStart(new SvtCalendarBox(m_xBuilder->weld_menu_button("start_date")))
    , mxRbAutoEnd(m_xBuilder->weld_radio_button("auto_end"))
    , mxRbManEnd(m_xBuilder->weld_radio_button("manual_end"))
    , mxEdEnd(new SvtCalendarBox(m_xBuilder->weld_menu_button("end_date")))
    , mxRbNumDays(m_xBuilder->weld_radio_button("days"))
    , mxRbUnits(m_xBuilder->weld_radio_button("intervals"))
    , mxEdNumDays(m_xBuilder->weld_spin_button("days_value"))
    , mxLbUnits(m_xBuilder->weld_tree_view("interval_list"))
    , mxBtnOk(m_xBuilder->weld_button("ok"))
    , maStartHelper(*mxRbAutoStart, *mxRbManStart, *mxEdStart, rNullDate)
    , maEndHelper(*mxRbAutoEnd, *mxRbManEnd, *mxEdEnd, rNullDate)
{
    maStartHelper.SetValue( rInfo.mbAutoStart, rInfo.mfStart );
    maEndHelper.SetValue( rInfo.mbAutoEnd, rInfo.mfEnd );

    std::vector<int> aWidths;
    aWidths.push_back(mxLbUnits->get_checkbox_column_width());
    mxLbUnits->set_column_fixed_widths(aWidths);

    if( nDatePart == 0 )
        nDatePart = css::sheet::DataPilotFieldGroupBy::MONTHS;
    for (size_t nIdx = 0; nIdx < SAL_N_ELEMENTS(aDatePartResIds); ++nIdx)
    {
        mxLbUnits->append();
        mxLbUnits->set_toggle(nIdx, (nDatePart & spnDateParts[ nIdx ]) != 0, 0);
        mxLbUnits->set_text(nIdx, ScResId(aDatePartResIds[nIdx]), 1);
    }

    if( rInfo.mbDateValues )
    {
        mxRbNumDays->set_active(true);
        ClickHdl(*mxRbNumDays );

        double fNumDays = rInfo.mfStep;
        if( fNumDays < 1.0 )
            fNumDays = 1.0;
        else if( fNumDays > 32767.0 )
            fNumDays = 32767.0;
        mxEdNumDays->set_value(fNumDays);
    }
    else
    {
        mxRbUnits->set_active(true);
        ClickHdl(*mxRbUnits);
    }

    /*  Set the initial focus, currently it is somewhere after calling all the radio
        button click handlers. Now the first enabled editable control is focused. */
    if( mxEdStart->get_sensitive() )
        mxEdStart->grab_focus();
    else if( mxEdEnd->get_sensitive() )
        mxEdEnd->grab_focus();
    else if( mxEdNumDays->get_sensitive() )
        mxEdNumDays->grab_focus();
    else if( mxLbUnits->get_sensitive() )
        mxLbUnits->grab_focus();

    mxRbNumDays->connect_clicked( LINK( this, ScDPDateGroupDlg, ClickHdl ) );
    mxRbUnits->connect_clicked( LINK( this, ScDPDateGroupDlg, ClickHdl ) );
    mxLbUnits->connect_toggled( LINK( this, ScDPDateGroupDlg, CheckHdl ) );
}

ScDPDateGroupDlg::~ScDPDateGroupDlg()
{
}

ScDPNumGroupInfo ScDPDateGroupDlg::GetGroupInfo() const
{
    ScDPNumGroupInfo aInfo;
    aInfo.mbEnable = true;
    aInfo.mbDateValues = mxRbNumDays->get_active();
    aInfo.mbAutoStart = maStartHelper.IsAuto();
    aInfo.mbAutoEnd = maEndHelper.IsAuto();

    // get values and silently auto-correct them, if they are not valid
    // TODO: error messages in OK event?
    aInfo.mfStart = maStartHelper.GetValue();
    aInfo.mfEnd = maEndHelper.GetValue();
    sal_Int64 nNumDays = mxEdNumDays->get_value();
    aInfo.mfStep = static_cast<double>( aInfo.mbDateValues ? nNumDays : 0L );
    if( aInfo.mfEnd <= aInfo.mfStart )
        aInfo.mfEnd = aInfo.mfStart + nNumDays;

    return aInfo;
}

sal_Int32 ScDPDateGroupDlg::GetDatePart() const
{
    // return DAYS for special "number of days" mode
    if( mxRbNumDays->get_active() )
        return css::sheet::DataPilotFieldGroupBy::DAYS;

    // return listbox contents for "units" mode
    sal_Int32 nDatePart = 0;
    for (int nIdx = 0, nCount = mxLbUnits->n_children(); nIdx < nCount; ++nIdx )
        if (mxLbUnits->get_toggle(nIdx, 0))
            nDatePart |= spnDateParts[ nIdx ];
    return nDatePart;
}

IMPL_LINK(ScDPDateGroupDlg, ClickHdl, weld::Button&, rButton, void)
{
    if (&rButton == mxRbNumDays.get())
    {
        mxLbUnits->set_sensitive(false);
        // enable and set focus to edit field on clicking "num of days" radio button
        mxEdNumDays->set_sensitive(true);
        mxEdNumDays->grab_focus();
        mxBtnOk->set_sensitive(true);
    }
    else if (&rButton == mxRbUnits.get())
    {
        mxEdNumDays->set_sensitive(false);
        // enable and set focus to listbox on clicking "units" radio button
        mxLbUnits->set_sensitive(true);
        mxLbUnits->grab_focus();
        // disable OK button if no date part selected
        CheckHdl(row_col(0, 0));
    }
}

namespace
{
    bool HasCheckedEntryCount(const weld::TreeView& rView)
    {
        for (int i = 0; i < rView.n_children(); ++i)
        {
            if (rView.get_toggle(i, 0))
                return true;
        }
        return false;
    }
}

IMPL_LINK_NOARG(ScDPDateGroupDlg, CheckHdl, const row_col&, void)
{
    // enable/disable OK button on modifying check list box
    mxBtnOk->set_sensitive(HasCheckedEntryCount(*mxLbUnits));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
