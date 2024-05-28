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
const sal_Int32 spnDateParts[] =
{
    css::sheet::DataPilotFieldGroupBy::SECONDS,
    css::sheet::DataPilotFieldGroupBy::MINUTES,
    css::sheet::DataPilotFieldGroupBy::HOURS,
    css::sheet::DataPilotFieldGroupBy::DAYS,
    css::sheet::DataPilotFieldGroupBy::MONTHS,
    css::sheet::DataPilotFieldGroupBy::QUARTERS,
    css::sheet::DataPilotFieldGroupBy::YEARS
};

const TranslateId aDatePartResIds[] =
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
    mrRbAuto.connect_toggled( LINK( this, ScDPGroupEditHelper, ToggleHdl ) );
    mrRbMan.connect_toggled( LINK( this, ScDPGroupEditHelper, ToggleHdl ) );
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
        ToggleHdl(mrRbAuto);
    }
    else
    {
        mrRbMan.set_active(true);
        ToggleHdl(mrRbMan);
    }
    ImplSetValue( fValue );
}

IMPL_LINK(ScDPGroupEditHelper, ToggleHdl, weld::Toggleable&, rButton, void)
{
    if (!rButton.get_active())
        return;

    if (mrRbAuto.get_active())
    {
        // disable edit field on clicking "automatic" radio button
        mrEdValue.set_sensitive(false);
    }
    else if (mrRbMan.get_active())
    {
        // enable and set focus to edit field on clicking "manual" radio button
        mrEdValue.set_sensitive(true);
        mrEdValue.grab_focus();
    }
}

ScDPNumGroupEditHelper::ScDPNumGroupEditHelper(weld::RadioButton& rRbAuto,
    weld::RadioButton& rRbMan, ScDoubleField& rEdValue)
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
    : GenericDialogController(pParent, u"modules/scalc/ui/groupbynumber.ui"_ustr, u"PivotTableGroupByNumber"_ustr)
    , mxRbAutoStart(m_xBuilder->weld_radio_button(u"auto_start"_ustr))
    , mxRbManStart(m_xBuilder->weld_radio_button(u"manual_start"_ustr))
    , mxEdStart(new ScDoubleField(m_xBuilder->weld_entry(u"edit_start"_ustr)))
    , mxRbAutoEnd(m_xBuilder->weld_radio_button(u"auto_end"_ustr))
    , mxRbManEnd(m_xBuilder->weld_radio_button(u"manual_end"_ustr))
    , mxEdEnd(new ScDoubleField(m_xBuilder->weld_entry(u"edit_end"_ustr)))
    , mxEdBy(new ScDoubleField(m_xBuilder->weld_entry(u"edit_by"_ustr)))
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
    : GenericDialogController(pParent, u"modules/scalc/ui/groupbydate.ui"_ustr, u"PivotTableGroupByDate"_ustr)
    , mxRbAutoStart(m_xBuilder->weld_radio_button(u"auto_start"_ustr))
    , mxRbManStart(m_xBuilder->weld_radio_button(u"manual_start"_ustr))
    , mxEdStart(new SvtCalendarBox(m_xBuilder->weld_menu_button(u"start_date"_ustr)))
    , mxRbAutoEnd(m_xBuilder->weld_radio_button(u"auto_end"_ustr))
    , mxRbManEnd(m_xBuilder->weld_radio_button(u"manual_end"_ustr))
    , mxEdEnd(new SvtCalendarBox(m_xBuilder->weld_menu_button(u"end_date"_ustr)))
    , mxRbNumDays(m_xBuilder->weld_radio_button(u"days"_ustr))
    , mxRbUnits(m_xBuilder->weld_radio_button(u"intervals"_ustr))
    , mxEdNumDays(m_xBuilder->weld_spin_button(u"days_value"_ustr))
    , mxLbUnits(m_xBuilder->weld_tree_view(u"interval_list"_ustr))
    , mxBtnOk(m_xBuilder->weld_button(u"ok"_ustr))
    , maStartHelper(*mxRbAutoStart, *mxRbManStart, *mxEdStart, rNullDate)
    , maEndHelper(*mxRbAutoEnd, *mxRbManEnd, *mxEdEnd, rNullDate)
{
    maStartHelper.SetValue( rInfo.mbAutoStart, rInfo.mfStart );
    maEndHelper.SetValue( rInfo.mbAutoEnd, rInfo.mfEnd );

    mxLbUnits->enable_toggle_buttons(weld::ColumnToggleType::Check);

    if( nDatePart == 0 )
        nDatePart = css::sheet::DataPilotFieldGroupBy::MONTHS;
    for (size_t nIdx = 0; nIdx < SAL_N_ELEMENTS(aDatePartResIds); ++nIdx)
    {
        mxLbUnits->append();
        mxLbUnits->set_toggle(nIdx, (nDatePart & spnDateParts[ nIdx ]) ? TRISTATE_TRUE : TRISTATE_FALSE);
        mxLbUnits->set_text(nIdx, ScResId(aDatePartResIds[nIdx]), 0);
    }

    if( rInfo.mbDateValues )
    {
        mxRbNumDays->set_active(true);
        ToggleHdl(*mxRbNumDays );

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
        ToggleHdl(*mxRbUnits);
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

    mxRbNumDays->connect_toggled( LINK( this, ScDPDateGroupDlg, ToggleHdl ) );
    mxRbUnits->connect_toggled( LINK( this, ScDPDateGroupDlg, ToggleHdl ) );
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
        if (mxLbUnits->get_toggle(nIdx) == TRISTATE_TRUE)
            nDatePart |= spnDateParts[ nIdx ];
    return nDatePart;
}

IMPL_LINK(ScDPDateGroupDlg, ToggleHdl, weld::Toggleable&, rButton, void)
{
    if (!rButton.get_active())
        return;
    if (mxRbNumDays->get_active())
    {
        mxLbUnits->set_sensitive(false);
        // enable and set focus to edit field on clicking "num of days" radio button
        mxEdNumDays->set_sensitive(true);
        mxEdNumDays->grab_focus();
        mxBtnOk->set_sensitive(true);
    }
    else if (mxRbUnits->get_active())
    {
        mxEdNumDays->set_sensitive(false);
        // enable and set focus to listbox on clicking "units" radio button
        mxLbUnits->set_sensitive(true);
        mxLbUnits->grab_focus();
        // disable OK button if no date part selected
        Check();
    }
}

namespace
{
    bool HasCheckedEntryCount(const weld::TreeView& rView)
    {
        for (int i = 0; i < rView.n_children(); ++i)
        {
            if (rView.get_toggle(i) == TRISTATE_TRUE)
                return true;
        }
        return false;
    }
}

IMPL_LINK_NOARG(ScDPDateGroupDlg, CheckHdl, const weld::TreeView::iter_col&, void)
{
    Check();
}

void ScDPDateGroupDlg::Check()
{
    // enable/disable OK button on modifying check list box
    mxBtnOk->set_sensitive(HasCheckedEntryCount(*mxLbUnits));
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
